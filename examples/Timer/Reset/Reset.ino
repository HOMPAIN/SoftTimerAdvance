// Пример демонстрирует сброс таймера
// Таймер выводит сообщение раз в 3 секунды. Любой символ, отправленный в монитор порта,
// сбрасывает таймер, и 3 секунды отсчитываются заново
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймером
STimer *Timer1 = 0;

// Функция таймера, вызывается раз в 3 сек.
void Timer3sec()
{
  Serial.println("3sec");
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем таймер
  Timer1 = ST.AddTimer(Timer3sec, 3, STUnits::Seconds);// Период 3 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());

  // Принят символ, сбрасываем таймер: он сработает через полный период от этого момента
  if (Serial.available())
  {
    Serial.read();
    Serial.println("Reset");
    Timer1->Reset();
  }
}
