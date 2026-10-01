// Пример демонстрирует сброс таймаута из прерывания
// Каждое нажатие кнопки вызывает прерывание, в котором сбрасывается таймаут
// Если кнопку не нажимали 3 секунды, вызывается функция таймаута
// Обычные методы таймеров вызывать из прерываний нельзя, для этого есть методы с окончанием FromISR
#include "SoftTimerAdvance.h"

// Пин кнопки, кнопка замыкает его на землю. Пин должен поддерживать прерывания (на Arduino Uno это 2 и 3)
#ifdef ESP32
const int BUTTON_PIN = 4;
#else
const int BUTTON_PIN = 2;
#endif

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;
// Интерфейс управления таймаутом
STimeout *Timeout1 = 0;

// Функция будет вызвана, если кнопку не нажимали 3 секунды
void NoButtonTimeout()
{
  Serial.println("Timeout: no button press for 3 sec");
}

// Обработчик прерывания, вызывается при нажатии кнопки
void ButtonISR()
{
  // Сбрасываем и запускаем таймаут. Сам сброс произойдёт при ближайшем вызове ST.Update
  Timeout1->ResetFromISR();
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Создаём таймаут 3 сек и запускаем отсчёт
  Timeout1 = ST.AddTimeout(NoButtonTimeout, 3, STUnits::Seconds);
  Timeout1->Reset();

  // Подключаем прерывание по нажатию кнопки. Таймаут к этому моменту уже должен быть создан
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ButtonISR, FALLING);
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, здесь же выполняются действия, запрошенные из прерываний
  ST.Update(millis());
}
