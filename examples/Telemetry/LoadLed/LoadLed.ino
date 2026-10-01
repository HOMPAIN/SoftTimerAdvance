// Пример демонстрирует светодиод загрузки процессора
// Светодиод горит, пока работает функция любого таймера. Чем ярче светодиод, тем больше загрузка
// В примере таймер раз в 500 мс работает около 100 мс
#include "SoftTimerAdvance.h"

// Пин светодиода, при необходимости замените на свой
#ifdef LED_BUILTIN
const int LED_PIN = LED_BUILTIN;
#else
const int LED_PIN = 2;
#endif

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Функция включения светодиода, вызывается перед функцией таймера
void LedOn()
{
  digitalWrite(LED_PIN, HIGH);
}

// Функция выключения светодиода, вызывается после функции таймера
void LedOff()
{
  digitalWrite(LED_PIN, LOW);
}

// Функция таймера, вызывается раз в 500 миллисек. и работает около 100 мс
void Work()
{
  delay(100);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Указываем функции включения и выключения светодиода. Выполняется после Init
  ST.Telemetry.LedOn = LedOn;
  ST.Telemetry.LedOff = LedOff;

  // Инициализируем таймер
  ST.AddTimer(Work, 500);// Период 500 мс
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
