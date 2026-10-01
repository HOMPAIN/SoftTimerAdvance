// Пример демонстрирует работу телеметрии
// Телеметрия измеряет время работы функций таймеров, загрузку процессора и следит за слишком долгими функциями
// В примере работают 2 таймера: быстрый и медленный, раз в 3 секунды данные телеметрии выводятся в порт
#include "SoftTimerAdvance.h"

// Пин светодиода загрузки процессора, при необходимости замените на свой
#ifdef LED_BUILTIN
const int LED_PIN = LED_BUILTIN;
#else
const int LED_PIN = 2;
#endif

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейсы таймеров, через них можно узнать время работы каждого таймера
STimer *FastTimer = 0;
STimer *SlowTimer = 0;

// Функция быстрого таймера, вызывается раз в 100 миллисекунд и работает около 2 мс
void Fast()
{
  delay(2);
}

// Функция медленного таймера, вызывается раз в секунду и работает около 30 мс
void Slow()
{
  delay(30);
}

// Функция вызывается телеметрией, если какой-то таймер работал дольше разрешённого времени
// _Func - функция таймера, _Time - время её работы в микросекундах
void TooLong(VoidFuncST _Func, uint32_t _Time)
{
  Serial.print("Too long: ");
  Serial.print(_Func == Slow ? "Slow" : "other");
  Serial.print(", us: ");
  Serial.println(_Time);
}

// Функции светодиода загрузки процессора: он горит, пока работает функция любого таймера
void LedOn()
{
  digitalWrite(LED_PIN, HIGH);
}
void LedOff()
{
  digitalWrite(LED_PIN, LOW);
}

// Функция таймера, раз в 3 секунды выводит данные телеметрии
void PrintTelemetry()
{
  Serial.print("CPU load, %: ");
  Serial.print(ST.Telemetry.CPULoad);
  Serial.print("  Max work time, us: ");
  Serial.print(ST.Telemetry.MaxWorkTime);
  Serial.print("  Fast, us: ");
  Serial.print(FastTimer->GetWorkTime());
  Serial.print("  Slow, us: ");
  Serial.print(SlowTimer->GetWorkTime());
  Serial.print("  Pool used, bytes: ");
  Serial.println(ST.Telemetry.PoolUsedMax);
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Настройка телеметрии, выполняется после Init
  // Телеметрии нужно время в микросекундах, которое идёт во время работы функций таймеров
  // В ардуино для этого достаточно указать функцию micros
  ST.Telemetry.GetMicros = micros;
  // Вместо функции можно увеличивать переменную из своего прерывания, например раз в 50 мкс:
  // ST.Telemetry.uSeconds += 50;

  // Следим, чтобы функции таймеров работали не дольше 10 мс
  ST.Telemetry.WatchDogTime = 10000;
  ST.Telemetry.OnWatchDog = TooLong;

  // Светодиод загрузки процессора, функции включения и выключения задавать не обязательно
  pinMode(LED_PIN, OUTPUT);
  ST.Telemetry.LedOn = LedOn;
  ST.Telemetry.LedOff = LedOff;

  // Инициализируем таймеры
  FastTimer = ST.AddTimer(Fast, 100);
  SlowTimer = ST.AddTimer(Slow, 1000);
  ST.AddTimer(PrintTelemetry, 3, STUnits::Seconds);
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
