// Пример демонстрирует остановку (заморозку) и продолжение работы таймера
// Первый таймер выводит сообщение раз в 500 мс, второй раз в 3 секунды останавливает его или запускает снова
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймером
STimer *Timer1 = 0;

// Функция таймера 1, вызывается раз в 500 миллисек.
void Tick()
{
  Serial.println("tick");
}

// Функция таймера 2, вызывается раз в 3 сек.
void FreezeSwitch()
{
  // GetFreezeStatus возвращает 1, если таймер заморожен
  if (Timer1->GetFreezeStatus())
  {
    Serial.println("UnFreeze");
    Timer1->UnFreeze();// Таймер продолжает работу
  }
  else
  {
    Serial.println("Freeze");
    Timer1->Freeze();// Таймер остановлен, его функция не вызывается
  }
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем таймеры
  Timer1 = ST.AddTimer(Tick, 500);// Период 500 мс
  ST.AddTimer(FreezeSwitch, 3, STUnits::Seconds);// Период 3 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
