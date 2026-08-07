// Пример демонстрирует базовую работу с таймерами
// В примере запускаются 2 программных таймера с периодом работы 500 мс и 1 сек
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Функция таймера, вызывается раз в 500 миллисекунд
void Timer500ms()
{
  Serial.println("500ms");
}

// Функция таймера, вызывается раз в секунду
void Timer1sec()
{
  Serial.println("1sec");
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров, возвращает количество таймеров, которое помещается в буфер
  int max_timers_count = ST.Init(t_buff, sizeof(t_buff));
  Serial.print("Timers manager init complite. Timer available: ");
  Serial.println(max_timers_count);

  // Инициализируем таймеры
  ST.AddTimer(Timer500ms, 500);// Период 500 мс (по умолчанию миллисекунды)
  ST.AddTimer(Timer1sec, 1, STUnits::Seconds);// Период 1 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}