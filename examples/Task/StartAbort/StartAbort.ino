// Пример демонстрирует остановку и повторный запуск таска
// Таск считает от нуля с паузой 500 мс. Любой символ, отправленный в монитор порта,
// останавливает работающий таск или запускает остановленный
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Переменная для хранения таска
STTask Task1;
// Функция таска, должна обязательно принимать указатель с именем "Task"
void TaskFunc(STTask* Task)
{
  // Переменные внутри таска должны быть статическими
  static int i = 0;
  TaskBegin();
  for (i = 0; ; i++)
  {
    Serial.println(i);
    TaskDelay(500);
  }
  TaskEnd();
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Запуск таска
  Task1.Start((PrmFuncST)TaskFunc, &ST);
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таск
  ST.Update(millis());

  if (Serial.available())
  {
    Serial.read();
    // IsRunning возвращает 1, если таск запущен и ещё не завершился
    if (Task1.IsRunning())
    {
      Serial.println("Abort");
      Task1.Abort();// Останавливает таск, он больше не вызывается
    }
    else
    {
      Serial.println("Start");
      Task1.Start((PrmFuncST)TaskFunc, &ST);// Таск начинается с начала
    }
  }
}
