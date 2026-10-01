// Пример демонстрирует базовую работу с таймаутом
// Таймаут срабатывает, если в течение заданного времени его ни разу не сбросили
// В примере отслеживается приём данных по Serial: каждый принятый символ сбрасывает таймаут,
// если данных нет дольше 3 сек, вызывается функция таймаута и гаснет светодиод
// Для проверки откройте монитор порта и отправляйте любые символы
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
// Интерфейс управления таймаутом
STimeout *RxTimeout = 0;

// Функция будет вызвана, если данные не приходили 3 секунды
void NoDataTimeout()
{
  Serial.println("Timeout: no data for 3 sec");
  digitalWrite(LED_PIN, LOW);
  // После срабатывания таймаут останавливается и не удаляется,
  // повторно он запустится при следующем вызове Reset()
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Инициализация менеджера таймеров, возвращает количество таймеров, которое помещается в буфер
  int max_timers_count = ST.Init(t_buff, sizeof(t_buff));
  Serial.print("Timers manager init complite. Timer available: ");
  Serial.println(max_timers_count);

  // Создаём таймаут 3 сек. После создания таймаут остановлен, отсчёт начнётся после Reset()
  RxTimeout = ST.AddTimeout(NoDataTimeout, 3, STUnits::Seconds);
  // Если в буфере нет места, функции добавления возвращают 0. Пользоваться таким указателем нельзя
  if (RxTimeout == 0)
  {
    Serial.println("Error: no free timers");
    while (true) {}
  }
  // Запускаем отсчёт сразу, чтобы таймаут сработал, если данные не придут совсем
  RxTimeout->Reset();
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймаут
  ST.Update(millis());

  // Каждый принятый символ сбрасывает таймаут, отсчёт 3 сек начинается заново
  if (Serial.available())
  {
    char c = Serial.read();
    Serial.print("Received: ");
    Serial.println(c);
    digitalWrite(LED_PIN, HIGH);
    RxTimeout->Reset();
  }
}
