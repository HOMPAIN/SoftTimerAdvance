// Пример для ESP32 демонстрирует сон процессора между срабатываниями таймеров
// В примере два светодиода мигают с разным периодом, а всё остальное время процессор спит (light sleep)
// Update возвращает время до следующего срабатывания таймера в микросекундах, на это время процессор и засыпает
// Раз в 5 секунд в порт выводится, какую долю времени процессор провёл во сне
#if !defined(ESP32)
#error "Этот пример только для ESP32"
#endif
#include "SoftTimerAdvance.h"
#include "esp_sleep.h"

// Пины светодиодов, при необходимости замените на свои
const int LED1_PIN = 2; // встроенный светодиод на большинстве плат ESP32
const int LED2_PIN = 4;

// Если до следующего таймера осталось меньше, засыпать нет смысла: вход в сон и выход из него занимают время
const uint32_t MIN_SLEEP_US = 2000;

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Суммарное время сна в микросекундах, для статистики
uint64_t sleep_time_us = 0;

// Функция таймера 1, переключает первый светодиод раз в 300 мс
void Led1Blink()
{
  digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
}

// Функция таймера 2, переключает второй светодиод раз в секунду
void Led2Blink()
{
  digitalWrite(LED2_PIN, !digitalRead(LED2_PIN));
}

// Функция таймера 3, раз в 5 секунд выводит долю времени, проведённую во сне
void PrintStat()
{
  static uint64_t sleep_last = 0;
  Serial.print("Sleep: ");
  Serial.print((uint32_t)((sleep_time_us - sleep_last) / 50000)); // 5 сек = 100%
  Serial.println("%");
  sleep_last = sleep_time_us;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Инициализация менеджера таймеров, возвращает количество таймеров, которое помещается в буфер
  int max_timers_count = ST.Init(t_buff, sizeof(t_buff));
  Serial.print("Timers manager init complite. Timer available: ");
  Serial.println(max_timers_count);

  // Инициализируем таймеры
  ST.AddTimer(Led1Blink, 300);// Период 300 мс
  ST.AddTimer(Led2Blink, 1000);// Период 1000 мс
  ST.AddTimer(PrintStat, 5, STUnits::Seconds);// Период 5 сек
}

void loop() {
  // Работа менеджера таймеров. Возвращает время до следующего срабатывания таймера в микросекундах
  // 0 означает, что есть таймер, готовый к запуску, и спать нельзя
  // Время передаём в микросекундах, счётчик micros() во время light sleep продолжает идти
  uint32_t time_to_next = ST.Update(micros(), STUnits::Microseconds);

  if (time_to_next > MIN_SLEEP_US)
  {
    // Перед сном дожидаемся отправки данных в порт, иначе вывод оборвётся
    Serial.flush();
    // Засыпаем до следующего таймера. Состояние выходов во время light sleep сохраняется
    esp_sleep_enable_timer_wakeup(time_to_next);
    esp_light_sleep_start();
    sleep_time_us += time_to_next;
  }
}
