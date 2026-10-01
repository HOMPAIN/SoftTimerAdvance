// Пример демонстрирует, как узнать, сколько памяти буфера занимают таймеры
// Это помогает подобрать размер буфера: его можно уменьшить до реально используемого
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Функция таймеров
void Tick()
{
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Инициализируем таймеры
  ST.AddTimer(Tick, 500);// Период до 65535 - таймер занимает меньше памяти
  ST.AddTimer(Tick, 1000);
  ST.AddTimer(Tick, 100000);// Период больше 65535 - таймер занимает больше памяти

  // PoolUsedMax - наибольшее количество байт буфера, которое было занято таймерами
  Serial.print("Pool used, bytes: ");
  Serial.print(ST.Telemetry.PoolUsedMax);
  Serial.print(" of ");
  Serial.println(sizeof(t_buff));
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
