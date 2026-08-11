// Пример демонстрирует динамическое добавление и удаление таймеров
// В примере запускаются 2 программных таймера
// Первый с периудом 3 сек, он динамически добавляет и удаляет второй таймер
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления вторым таймером
STimer *Timer2=0;

// Функция таймера 2, вызывается раз в 500 миллисек.
void Timer500ms()
{
  Serial.println("500ms");
}

// Функция таймера 1, вызывается раз в 3 сек.
void Timer3sec()
{
    Serial.println("3sec");
  //если втрого таймера нет создаём его, если есть , наоборот удалёем
  if(Timer2==0)
  {
    Timer2 = ST.AddTimer(Timer500ms, 500);// Период 500 мс (по умолчанию миллисекунды)
  }else
  {
    Timer2->Delete();
    Timer2=0;
  }
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров, возвращает количество таймеров, которое помещается в буфер
  int max_timers_count = ST.Init(t_buff, sizeof(t_buff));
  Serial.print("Timers manager init complite. Timer available: ");
  Serial.println(max_timers_count);

  // Инициализируем таймер
  ST.AddTimer(Timer3sec, 3, STUnits::Seconds);// Период 3 сек
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}