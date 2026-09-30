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

    uint16_t Size;    //количество 32-битных таймеров, которое помещается в пустой пул (16-битных помещается больше)
    uint16_t Count16; //количество занятых ячеек 16-битных таймеров, они заполняют пул с начала
    uint16_t Count32; //количество занятых ячеек 32-битных таймеров, они заполняют пул с конца
    uint16_t Iterator;//счётчик для перебора таймеров для запуска по обному за проход

    //телеметрия таймера
    SoftTimerTelemetry_pp Telemetry;


    SoftTimerManager()
    {
        memset(this, 0,sizeof(SoftTimerManager));
    }

    //инициализация парамтетров мэнеджера, возращает количество таймеров доступных в пуле
    //(считается по 32-битным таймерам, 16-битных помещается больше)
    //_Buff - буффер для пула таймеров, _BuffSize - размер буффера в байтах
    uint16_t Init(uint8_t * _Buff, uint32_t _BuffSize);

    //работа таймера, передаётся текущее время, можно вызывать любую функцию
    //источник времени должен быть 32- или 64-битным счётчиком (millis(), micros()), его переполнение обрабатывается
    //возвращает время до следующего срабатывания таймера в микросекундах (см. GetTimeToNext), его можно использовать для сна
    uint32_t Update(uint64_t _Time, STUnits _Units = STUnits::Milliseconds);
    uint32_t Update_us(uint64_t _uSeconds);
    uint32_t Update_ms(uint64_t _mSeconds);
    uint32_t Update_s(uint64_t _Seconds);

    //время до ближайшего срабатывания таймера в микросекундах
    //0 - есть таймер, готовый к запуску, нужно снова вызвать Update
    //0xFFFFFFFF - работающих таймеров нет (или ждать дольше 71 минуты)
    uint32_t GetTimeToNext();

    //добавление таймера в микро, милли или секундах
    //_Res - разрешение таймера. По умолчанию выбирается по периоду: до 65535 - 16 бит (занимает меньше памяти), больше - 32 бита
    //Таймеры в микросекундах по умолчанию всегда 32 бита
    //Если период позже будет увеличен выше 65535, стоит сразу указать STResolution::Bits32
    STimer* AddTimer(VoidFuncST _Func, uint32_t _Period, STUnits _Units = STUnits::Milliseconds, STResolution _Res = STResolution::BitsAuto);
    STimer* AddTimer(PrmFuncST _Func, void * _Prm, uint32_t _Period, STUnits _Units = STUnits::Milliseconds, STResolution _Res = STResolution::BitsAuto);
    //добавление задачи отложенного запуска в микро, милли или секундах
    SDelay* AddDelayCall(VoidFuncST _Func, uint32_t _Delay, STUnits _Units = STUnits::Milliseconds, STResolution _Res = STResolution::BitsAuto);
    SDelay* AddDelayCall(PrmFuncST _Func, void* _Prm, uint32_t _Delay, STUnits _Units = STUnits::Milliseconds, STResolution _Res = STResolution::BitsAuto);
    //добавление таймаута
    STimeout* AddTimeout(VoidFuncST _Func, uint32_t _Timeout, STUnits _Units = STUnits::Milliseconds, STResolution _Res = STResolution::BitsAuto);
    STimeout* AddTimeout(PrmFuncST _Func, void* _Prm, uint32_t _Timeout, STUnits _Units = STUnits::Milliseconds, STResolution _Res = STResolution::BitsAuto);
private:

    //общий пул таймеров: 16-битные заполняют его с начала, 32-битные с конца навстречу им
    SoftTimer16* Timers16;      //начало пула, 16-битный таймер n лежит в Timers16[n]
    SoftTimer32* Timers32End;   //конец пула, 32-битный таймер n лежит в Timers32End[-1 - n]
    uint32_t PoolSize;          //размер пула в байтах

    //универсальаная функция добавления таймера
    void* AddManual(SoftTimerBase _Timer, uint32_t _Delay, STResolution _Res);
    //поиск свободной ячейки (удалённого таймера) среди занятых ячеек заданного разрешения, 0 если такой нет
    SoftTimerBase* FindFree(STResolution _Res);
    //цикл прохода по таймерам, передаётся время, прошедшее с предыдущего вызова
    void Working(uint64_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s);
    //счётчик времени таймеров
    void Tick(uint32_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s);

    //время, прошедшее с предыдущего вызова Update, в единицах источника времени
    uint32_t TimeDelta(uint64_t _Time, STUnits _Units);
    //перевод приращения времени в более крупные единицы (делитель 1000), остаток накапливается в _Rem
    uint32_t DtDiv1000(uint32_t _Dt, uint16_t* _Rem);

    //состояние источника времени, для расчёта приращения между вызовами Update
    uint32_t TimeLast;      //предыдущее значение времени, переданное в Update
    uint16_t TimeUnits;     //единицы, в которых оно было передано
    uint16_t TimeStarted;   //1 - первое значение времени уже получено
    uint16_t RemUs;         //остаток микросекунд, не вошедший в mSeconds
    uint16_t RemMs;         //остаток миллисекунд, не вошедший в Seconds

    uint16_t Lock;//блокировка одновременного доступа
};

