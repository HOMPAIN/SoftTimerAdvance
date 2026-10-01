// Пример демонстрирует изменение периода таймера во время работы
// Первый таймер выводит сообщение, второй раз в 5 секунд меняет его период: 200 мс или 1000 мс
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймером
STimer *Timer1 = 0;

// Функция таймера 1
void Tick()
{
  Serial.println("tick");
}

// Функция таймера 2, вызывается раз в 5 сек. и меняет период таймера 1
void ChangePeriod()
{
  // GetPeriod возвращает текущий период в миллисекундах
  if (Timer1->GetPeriod() == 200)
    Timer1->SetPeriod(1000);// Новый период 1000 мс
  else
    Timer1->SetPeriod(200);// Новый период 200 мс

  Serial.print("New period: ");
  Serial.println(Timer1->GetPeriod());
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем таймеры
  Timer1 = ST.AddTimer(Tick, 200);// Период 200 мс
  ST.AddTimer(ChangePeriod, 5, STUnits::Seconds);// Период 5 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
