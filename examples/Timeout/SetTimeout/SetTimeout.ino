// Пример демонстрирует изменение времени таймаута
// Цифра от 1 до 9 в мониторе порта задаёт время таймаута в секундах и запускает его
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймаутом
STimeout *Timeout1 = 0;

// Функция таймаута
void OnTimeout()
{
  Serial.println("Timeout");
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Создаём таймаут 1 сек. После создания таймаут остановлен
  Timeout1 = ST.AddTimeout(OnTimeout, 1, STUnits::Seconds);
  Serial.println("Send digit 1..9 to set timeout in seconds");
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймаут
  ST.Update(millis());

  if (Serial.available())
  {
    char c = Serial.read();
    if (c >= '1' && c <= '9')
    {
      // Задаём новое время таймаута в секундах и запускаем отсчёт
      Timeout1->SetTimeout(c - '0', STUnits::Seconds);
      Timeout1->Reset();

      // GetTimeout возвращает текущее время таймаута, по умолчанию в миллисекундах
      Serial.print("Timeout, ms: ");
      Serial.println(Timeout1->GetTimeout());
    }
  }
}
