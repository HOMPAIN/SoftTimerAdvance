// Пример демонстрирует одновременную работу двух тасков
// Каждый таск в бесконечном цикле выводит сообщение со своей паузой: 1 секунда и 300 мс
// Таски не мешают друг другу, хотя в каждом есть пауза
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Переменные для хранения тасков, у каждого таска своя
STTask Task1;
STTask Task2;

// Функция первого таска, должна обязательно принимать указатель с именем "Task"
void Task1Func(STTask* Task)
{
  TaskBegin();
  for (;;)
  {
    Serial.println("Task 1");
    TaskDelay(1000);
  }
  TaskEnd();
}

// Функция второго таска
void Task2Func(STTask* Task)
{
  TaskBegin();
  for (;;)
  {
    Serial.println("  Task 2");
    TaskDelay(300);
  }
  TaskEnd();
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Запуск тасков
  Task1.Start((PrmFuncST)Task1Func, &ST);
  Task2.Start((PrmFuncST)Task2Func, &ST);
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таски
  ST.Update(millis());
}
