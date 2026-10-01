#pragma once

#include <stdint.h>                 //для определения типов uint16_t...
#include <string.h>                 //для работы memset
#include "SoftTimerAdvanceStructures.h"
#include "SoftTimerAdvanceInterface.h"







//функция времени в микросекундах для телеметрии, совпадает по типу с micros() в ардуино
typedef unsigned long(*MicrosFuncST)(void);
//функция, вызываемая при превышении WatchDogTime: _Func - функция таймера, _Time - время её работы в микросекундах
typedef void(*WatchDogFuncST)(VoidFuncST _Func, uint32_t _Time);

//телеметрия измеряет время работы функций таймеров. Для этого ей нужно время в микросекундах,
//которое идёт и во время работы функции таймера. Источник времени задаётся одним из двух способов:
//  1. GetMicros - указатель на функцию времени, например ST.Telemetry.GetMicros = micros;
//  2. uSeconds - переменная, которую пользователь увеличивает в прерывании, например ST.Telemetry.uSeconds += 50;
//Если GetMicros не 0, используется функция, иначе переменная. Если не задано ни то, ни другое, время работы равно 0
//Настройки телеметрии нужно задавать после Init менеджера, Init их сбрасывает
class SoftTimerTelemetry_pp
{
public:
    uint32_t uSeconds;              //микросекунды телеметрии, обновляются пользователем в прерывании
    MicrosFuncST GetMicros;         //функция времени в микросекундах, если не 0 - используется вместо uSeconds

    uint32_t MaxWorkTime;           //время самой долгой функции в микро сек, со временем убывает (1 мкс за 1 мс)
    VoidFuncST MaxWorkFunc;         //функция, которая работала MaxWorkTime
    uint16_t CPULoad;               //загрузка процессора функциями таймеров % 0-100, обновляется раз в 3 секунды

    //светодиод загрузки процессора: горит, пока работает функция таймера
    VoidFuncST LedOn;               //функция включения светодиода, вызывается перед функцией таймера, 0 - не вызывать
    VoidFuncST LedOff;              //функция выключения светодиода, вызывается после функции таймера, 0 - не вызывать

    uint32_t WatchDogTime;          //максимально допустимое время одной функции в микро сек, 0 - не проверять
    WatchDogFuncST OnWatchDog;      //вызывается, если функция таймера работала дольше WatchDogTime, 0 - не вызывать
    uint16_t WatchDogCount;         //сколько раз функции таймеров работали дольше WatchDogTime

    VoidFuncST LastFunc;                //последняя выполненая функция
    uint32_t LastFuncTime;          //врем работы последней функции

    uint32_t PoolUsedMax;           //наибольшее количество байт пула, занятое таймерами (для подбора размера буфера)

    //текущее время телеметрии в микросекундах
    uint32_t Now();
    //_Dt - прошедшее время в микросекундах
    void Tick(uint32_t _Dt);
    void AddWorkTime(uint32_t _WorkTime);
private:
    //время замера и время работы во время замера для расчёта загрузки ЦП
    uint32_t WorkTime;
    uint32_t MeasTime;
    uint16_t DecayTime;             //остаток времени для убывания MaxWorkTime
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
    //возвращает время до следующего срабатывания таймера в тех же единицах, в которых передано время (см. GetTimeToNext)
    //его можно использовать для сна. Если в этом вызове выполнялась функция таймера, возвращает 0 без расчёта
    uint32_t Update(uint64_t _Time, STUnits _Units = STUnits::Milliseconds);
    uint32_t Update_us(uint64_t _uSeconds);
    uint32_t Update_ms(uint64_t _mSeconds);
    uint32_t Update_s(uint64_t _Seconds);

    //время до ближайшего срабатывания таймера, _Units задаёт размерность времени (округляется вверх)
    //0 - есть таймер, готовый к запуску, или невыполненная заявка из прерывания, нужно снова вызвать Update
    //0xFFFFFFFF - работающих таймеров нет (или значение не помещается в 32 бита)
    uint32_t GetTimeToNext(STUnits _Units = STUnits::Milliseconds);

    //добавление таймера в микро, милли или секундах
    //Все функции добавления возвращают 0, если в пуле нет места. Перед использованием указателя его нужно проверить
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
    //возвращает 1, если была выполнена функция таймера
    uint16_t Working(uint64_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s);
    //счётчик времени таймеров, в этом же проходе выполняются заявки из прерываний (методы ...FromISR)
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

