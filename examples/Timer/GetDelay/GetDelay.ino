// Пример демонстрирует чтение времени, оставшегося до срабатывания таймера
// Первый таймер срабатывает раз в 5 секунд, второй раз в секунду выводит, сколько осталось до первого
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймером
STimer *Timer1 = 0;

// Функция таймера 1, вызывается раз в 5 сек.
void Timer5sec()
{
  Serial.println("5sec");
}

// Функция таймера 2, вызывается раз в секунду
void PrintDelay()
{
  // GetDelay возвращает время до следующего вызова, по умолчанию в миллисекундах
  Serial.print("Time to 5sec timer, ms: ");
  Serial.println(Timer1->GetDelay());
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем таймеры
  Timer1 = ST.AddTimer(Timer5sec, 5000);// Период 5000 мс
  ST.AddTimer(PrintDelay, 1000);// Период 1000 мс
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
