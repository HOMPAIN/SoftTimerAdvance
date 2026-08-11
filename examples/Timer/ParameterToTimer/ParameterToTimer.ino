// Пример демонстрирует передачу параметра в таймер
// В примере запускаются 3 таймера в которые передаются различные параметры
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
//параметры для передачи в таймер
int p1 = 1;
int p2 = 2;
int p3 = 3;

// Функция таймера 1, вызывается раз в 3 сек.
void Timer3sec(void *_Prm)
{
  //конвертируем указать в int и получаем значение по адресу
  int p = *((int *)_Prm);
  //выводим значение параметра
  Serial.println(p);
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров, возвращает количество таймеров, которое помещается в буфер
  int max_timers_count = ST.Init(t_buff, sizeof(t_buff));
  Serial.print("Timers manager init complite. Timer available: ");
  Serial.println(max_timers_count);

  // Инициализируем таймерs, период 3 сек
  ST.AddTimer(Timer3sec, &p1, 3, STUnits::Seconds);// передаётся параметр p1
  ST.AddTimer(Timer3sec, &p2, 3, STUnits::Seconds);// передаётся параметр p2
  ST.AddTimer(Timer3sec, &p3, 3, STUnits::Seconds);// передаётся параметр p3
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}