// Пример демонстрирует контроль слишком долгих функций таймеров
// Таймер раз в секунду работает то 5 мс, то 50 мс. Допустимое время работы - 20 мс
// При превышении вызывается функция пользователя
#include "SoftTimerAdvance.h"

// Буфер для хранения таймеров
uint8_t t_buff[1024];
// Менеджер таймеров
SoftTimerManager ST;

// Функция таймера, вызывается раз в секунду. Каждый второй раз работает слишком долго
void Work()
{
  static bool slow = false;
  if (slow)
    delay(50);
  else
    delay(5);
  slow = !slow;
}

// Функция вызывается, когда функция таймера работала дольше допустимого
// _Func - функция таймера, _Time - время её работы в микросекундах
void TooLong(VoidFuncST _Func, uint32_t _Time)
{
  Serial.print("Too long, us: ");
  Serial.println(_Time);
}

void setup() {
  Serial.begin(115200);

  // Инициализация менеджера таймеров
  ST.Init(t_buff, sizeof(t_buff));

  // Телеметрии нужно время в микросекундах, указываем функцию micros. Выполняется после Init
  ST.Telemetry.GetMicros = micros;
  // Допустимое время работы функции таймера 20000 мкс и функция, вызываемая при превышении
  ST.Telemetry.WatchDogTime = 20000;
  ST.Telemetry.OnWatchDog = TooLong;

  // Инициализируем таймер
  ST.AddTimer(Work, 1000);// Период 1000 мс
}

void loop() {
  // Работа менеджера таймеров в бесконечном цикле, в ней он вызывает таймеры
  ST.Update(millis());
}
