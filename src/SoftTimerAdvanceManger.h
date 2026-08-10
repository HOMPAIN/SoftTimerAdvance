#pragma once

#include <stdint.h>                 //для определения типов uint16_t...
#include <string.h>                 //для работы memset
#include "SoftTimerAdvanceStructures.h"
#include "SoftTimerAdvanceInterface.h"







class SoftTimerTelemetry_pp
{
public:
    int16_t  MaxWorkTime;           //время долгих функций в микро сек
    uint16_t CPULoad;               //загрузка процессора % 0-100, для работы необходимо обновлять STuSeconds
    int16_t  LedGPIO;               //светодиод загрузки процессора
    uint16_t WatchDogTime;          //максимально допустимое время одной функции

    VoidFuncST LastFanc;                //последняя выполненая функция
    uint32_t LastFancTime;          //врем работы последней функции



    void Tick(uint32_t _Dt);
    void AddWorkTime(uint32_t _WorkTime);
private:
    //uint32_t TimeCounter;//вспомогательный счик для расчёта максимального времени работы

    //время замера и время работы во время замера для расчёта загрузки ЦП
    uint32_t WorkTime;
    uint32_t MeasTime;
};

class SoftTimerManager
{
public:
    //время работы таймера
    uint64_t uSeconds;      //микросекунды
    uint32_t mSeconds;      //миллисекунды
    uint32_t Seconds;       //секунды

    //время работы таймера с предыдущего шага
    uint64_t uSecondsLast;  //микросекунды
    uint32_t mSecondsLast;  //миллисекунды
    uint32_t SecondsLast;   //секунды

    uint16_t Size;    //максимальное количество таймеров
    uint16_t Count;   //количество занятых таймеров
    uint16_t Iterator;//счётчик для перебора таймеров для запуска по обному за проход

    STResolution Resolution; //разрешение таймеров, 0 - 16 бит, 1 - 32 биты, 2 - 64 бита

    //телеметрия таймера
    SoftTimerTelemetry_pp Telemetry;


    SoftTimerManager()
    {
        memset(this, 0,sizeof(SoftTimerManager));
    }

    //инициализация парамтетров мэнеджера, возращает количество таймеров доступных в пуле
    //_Res - разрешение таймеров, 0 - 16 бит, 1 - 32 биты, 2 - 64 бита
    //_Buff - буффер для пула таймеров, _BuffSize - размер буффера в байтах
    uint16_t Init(uint8_t * _Buff, uint32_t _BuffSize, STResolution _Res = STResolution::Bits32);

    //работа таймера, передаётся текущее время, можно вызывать любую функцию
    void Update(uint64_t _Time, STUnits _Units = STUnits::Milliseconds);
    void Update_us(uint64_t _uSeconds);
    void Update_ms(uint64_t _mSeconds);
    void Update_s(uint64_t _Seconds);

    //добавление таймера в микро, милли или секундах
    STimer* AddTimer(VoidFuncST _Func, uint64_t _Period, STUnits _Units = STUnits::Milliseconds);
    STimer* AddTimer(PrmFuncST _Func, void * _Prm, uint64_t _Period, STUnits _Units = STUnits::Milliseconds);
    //добавление задачи отложенного запуска в микро, милли или секундах
    SDelay* AddDelayCall(VoidFuncST _Func, uint64_t _Delay, STUnits _Units = STUnits::Milliseconds);
    SDelay* AddDelayCall(PrmFuncST _Func, void* _Prm, uint64_t _Delay, STUnits _Units = STUnits::Milliseconds);
    //добавление таймаута
    STimeout* AddTimeout(VoidFuncST _Func, uint64_t _Timeout, STUnits _Units = STUnits::Milliseconds);
    STimeout* AddTimeout(PrmFuncST _Func, void* _Prm, uint64_t _Timeout, STUnits _Units = STUnits::Milliseconds);
private:

    //пул таймеров с различным разрешением, используется только 1
    SoftTimer16* Timers16;
    SoftTimer32* Timers32;
    SoftTimer64* Timers64;

    //универсальаная функция добавления таймера
    void* AddManual(SoftTimerBase _Timer, uint64_t _Counter, uint64_t _Delay);
    //цикл прохода по таймерам
    void Working();
    //счётчик времени таймеров
    void Tick16(uint16_t _Dt_us, uint16_t _Dt_ms, uint16_t _Dt_s);
    //счётчик времени таймеров
    void Tick32(uint16_t _Dt_us, uint16_t _Dt_ms, uint16_t _Dt_s);
    //счётчик времени таймеров
    void Tick64(uint16_t _Dt_us, uint16_t _Dt_ms, uint16_t _Dt_s);

    uint16_t Lock;//блокировка одновременного доступа
};

