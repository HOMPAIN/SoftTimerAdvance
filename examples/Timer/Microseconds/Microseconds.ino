// Пример демонстрирует таймер с периодом в микросекундах
// Для таких таймеров время в менеджер тоже нужно передавать в микросекундах
// Первый таймер срабатывает раз в 2500 мкс и считает вызовы, второй раз в секунду выводит их количество (около 400)
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Счётчик вызовов быстрого таймера
int counter = 0;

// Функция таймера 1, вызывается раз в 2500 микросек.
void Timer2500us()
{
  counter++;
}

// Функция таймера 2, вызывается раз в секунду
void Timer1sec()
{
  Serial.print("Calls per second: ");
  Serial.println(counter);
  counter = 0;
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем таймеры
  ST.AddTimer(Timer2500us, 2500, STUnits::Microseconds);// Период 2500 мкс
  ST.AddTimer(Timer1sec, 1, STUnits::Seconds);// Период 1 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле
  // Время передаётся в микросекундах, вторым параметром указываются единицы
  ST.Update(micros(), STUnits::Microseconds);
}
