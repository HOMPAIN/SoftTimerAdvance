// Пример демонстрирует базовую работу с задержкой вызова
// В примере создаётся задержка после старта программы 1000 мс и 5 сек, а также 1000 мс после пятисекундной задержки
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Функция будет вызвана через 1000 миллисекунд
void Delay1000ms()
{
  Serial.println("Delay 1000ms");
}

// Функция будет вызвана через 5 секунд
void Delay5sec()
{
  Serial.println("Delay 5sec");
  // Ещё раз вызовем отложенный запуск через 1000 мс от этого момента
  ST.AddDelayCall(Delay1000ms, 1000);
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров, возвращает количество таймеров, которое помещается в буфер
  int max_timers_count = ST.Init(t_buff, sizeof(t_buff));
  Serial.print("Timers manager init complite. Timer available: ");
  Serial.println(max_timers_count);

  // Инициализируем отложенные запуски после старта программы
  ST.AddDelayCall(Delay1000ms, 1000);// Задержка 1000 мс (по умолчанию миллисекунды)
  ST.AddDelayCall(Delay5sec, 5, STUnits::Seconds);// Задержка 5 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает отложенные запуски
  ST.Update(millis());
}