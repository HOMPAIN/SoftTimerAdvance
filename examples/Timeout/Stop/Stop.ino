// Пример демонстрирует запуск и остановку таймаута
// Символ 's' в мониторе порта запускает таймаут на 5 секунд, символ 'x' останавливает его
// Если таймаут не остановить, через 5 секунд будет вызвана его функция
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймаутом
STimeout *Timeout1 = 0;

// Функция будет вызвана, если таймаут не был остановлен или сброшен за 5 секунд
void Timeout5sec()
{
  Serial.println("Timeout");
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Создаём таймаут 5 сек. После создания таймаут остановлен
  Timeout1 = ST.AddTimeout(Timeout5sec, 5, STUnits::Seconds);
  Serial.println("Send 's' to start, 'x' to stop");
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймаут
  ST.Update(millis());

  if (Serial.available())
  {
    char c = Serial.read();
    if (c == 's')
    {
      Serial.println("Start");
      Timeout1->Reset();// Запускает отсчёт с начала
    }
    if (c == 'x')
    {
      Serial.println("Stop");
      Timeout1->Stop();// Останавливает отсчёт, функция таймаута не будет вызвана
    }
  }
}
