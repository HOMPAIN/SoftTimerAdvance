// Пример демонстрирует многократное использование одного отложенного запуска
// Любой символ, отправленный в монитор порта, включает светодиод, через секунду светодиод гаснет
// Отложенный запуск создаётся один раз и каждый раз перезапускается функцией Reset
#include "SoftTimerAdvance.h"

// Пин светодиода, при необходимости замените на свой
#ifdef LED_BUILTIN
const int LED_PIN = LED_BUILTIN;
#else
const int LED_PIN = 2;
#endif

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления отложенным запуском
SDelay *LedOffDelay = 0;

// Функция отложенного запуска, выключает светодиод
void LedOff()
{
  digitalWrite(LED_PIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем отложенный запуск
  LedOffDelay = ST.AddDelayCall(LedOff, 1000);
  // Включаем защиту от удаления: после срабатывания отложенный запуск останется в менеджере
  // и его можно будет запустить снова
  LedOffDelay->SetDeleteProtection(1);
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает отложенные запуски
  ST.Update(millis());

  // Принят символ, включаем светодиод и перезапускаем отложенный запуск на 1000 мс
  if (Serial.available())
  {
    Serial.read();
    digitalWrite(LED_PIN, HIGH);
    LedOffDelay->Reset(1000);
  }
}
