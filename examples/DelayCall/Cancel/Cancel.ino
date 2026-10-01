// Пример демонстрирует отмену отложенного запуска
// Через 5 секунд после старта должна быть вызвана функция. Если до этого отправить
// любой символ в монитор порта, запуск отменяется
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления отложенным запуском
SDelay *Delay1 = 0;

// Функция будет вызвана через 5 секунд
void Delay5sec()
{
  Serial.println("Delay 5sec");
  // После срабатывания отложенный запуск удаляется сам, пользоваться указателем на него больше нельзя
  Delay1 = 0;
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем отложенный запуск
  Delay1 = ST.AddDelayCall(Delay5sec, 5, STUnits::Seconds);// Задержка 5 сек
  Serial.println("Send any symbol to cancel");
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает отложенные запуски
  ST.Update(millis());

  // Принят символ, отменяем отложенный запуск, если он ещё не сработал
  if (Serial.available())
  {
    Serial.read();
    if (Delay1 != 0)
    {
      Delay1->Delete();
      Delay1 = 0;
      Serial.println("Canceled");
    }
  }
}
