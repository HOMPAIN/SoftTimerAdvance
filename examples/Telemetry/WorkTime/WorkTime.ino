// Пример демонстрирует измерение времени работы функции таймера
// Два таймера работают около 2 мс и около 20 мс, раз в 2 секунды время их работы выводится в порт
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейсы таймеров, через них можно узнать время работы каждого таймера
STimer *Timer1 = 0;
STimer *Timer2 = 0;

// Функция таймера 1, работает около 2 мс
void Work2ms()
{
  delay(2);
}

// Функция таймера 2, работает около 20 мс
void Work20ms()
{
  delay(20);
}

// Функция таймера, раз в 2 секунды выводит время работы таймеров
void PrintTime()
{
  // GetWorkTime возвращает время работы функции таймера в микросекундах
  Serial.print("Timer 1, us: ");
  Serial.print(Timer1->GetWorkTime());
  Serial.print("  Timer 2, us: ");
  Serial.println(Timer2->GetWorkTime());
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Телеметрии нужно время в микросекундах, указываем функцию micros. Выполняется после Init
  ST.Telemetry.GetMicros = micros;

  // Инициализируем таймеры
  Timer1 = ST.AddTimer(Work2ms, 100);// Период 100 мс
  Timer2 = ST.AddTimer(Work20ms, 500);// Период 500 мс
  ST.AddTimer(PrintTime, 2, STUnits::Seconds);// Период 2 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
