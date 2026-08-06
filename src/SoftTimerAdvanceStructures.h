#pragma once
#include <stdint.h>                 //для определения типов uint16_t...

//структура телеметрии таймера
/*typedef struct
{
    uint32_t uSeconds;              //микросекунды для работы телеметрии, нужно добавить обновление в обычном таймер(не обязательно)
    int16_t  MaxWorkTime;           //время долгих функций в микро сек
    uint16_t CPULoad;               //загрузка процессора % 0-100, для работы необходимо обновлять STuSeconds
    uint16_t WatchDogTime;          //максимально допустимое время одной функции

    void * LastFanc;                //последняя выполненая функция
    uint32_t LastFancTime;          //врем работы последней функции
}SoftTimerTelemetry;
*/

enum STResolution : uint16_t {
    Bits16 = 0,
    Bits32 = 1,
    Bits64 = 2
};

enum STUnits : uint16_t {
    Microseconds = 0,
    Milliseconds = 1,
    Seconds = 2
};

//шаблоны указателей на функцию
typedef void(*VoidFuncST)(void); //простая функция без параметра void func();
typedef void(*PrmFuncST)(void*);//функция с параметром          void func(void* prm);

//конфигурация объекта тамйера
class SoftTimerConfig
{
public:
    uint16_t Type : 2;                //тип таймера 0 - не используется, 1 - обычный, 2 - разовый (Invoke), 3 - таймаут
    uint16_t Freeze : 1;              //1 - заморозить таймер, счётчик не будет работать и он не будет вызываться
    uint16_t DeleteProtection : 1;    //1 - защита разового таймера от удаления, при даходе счётчика до 0 таймер будет заморожен
    uint16_t PrmEnable : 1;           //0 - таймер без параметров, 1 с параметрами
    uint16_t StrictMode : 2;          //0 - строгий режим(без пропусков), 1 пытается сохранить частоту, но допускает пропуски выполнеия, 2 - сбрасывает счётчик при каждом запуске
    uint16_t Resolution : 2;        //разрешение таймера , 0 - 16 бит, 1 - 32 биты, 2 - 64 бита
    uint16_t Units : 2;             //0 микро, 1- милли, 2 - секунды. Единицы измерения таймера
    uint16_t res : 5;//резерв
};

//общие параметры таймера
class SoftTimerBase
{
public:
    VoidFuncST Func;                //функция вызываемая по таймеру, 0 означает, что таймер свободен
    void* Params;                   //параметры для передачи в функцию таймера
    SoftTimerConfig Config;      //конфигурация таймера
    uint16_t MaxWorkTime;           //максимальное время работы функции (для телеметрии)
private:
};

//класс описания таймера с разрешением 16 бит в пуле
class SoftTimer16 : public SoftTimerBase
{
public:
    uint16_t Counter;               //счётчик, при доходе до 0 функция таймера вызывается
    uint16_t Delay;                 //периуд таймера мс, если 0 то таймер вызывается 1 раз и удалёется

    //заполнить из универсального таймера
    void FromTimer(SoftTimerBase _Timer, uint16_t _Counter, uint16_t _Delay)
    {
        Func = _Timer.Func;
        Params = _Timer.Params;
        Config = _Timer.Config;
        MaxWorkTime = 0;
        Counter = _Counter;
        Delay = _Delay;
    }
private:
};

//класс описания таймера с разрешением 32 бит в пуле
class SoftTimer32 : public SoftTimerBase
{
public:
    uint32_t Counter;               //счётчик, при доходе до 0 функция таймера вызывается
    uint32_t Delay;                 //периуд таймера мс, если 0 то таймер вызывается 1 раз и удалёется

    //заполнить из универсального таймера
    void FromTimer(SoftTimerBase _Timer, uint32_t _Counter, uint32_t _Delay)
    {
        Func = _Timer.Func;
        Params = _Timer.Params;
        Config = _Timer.Config;
        MaxWorkTime = 0;
        Counter = _Counter;
        Delay = _Delay;
    }
private:
};

//класс описания таймера с разрешением 64 бит в пуле
class SoftTimer64 : public SoftTimerBase
{
public:
    uint64_t Counter;               //счётчик, при доходе до 0 функция таймера вызывается
    uint64_t Delay;                 //периуд таймера мс, если 0 то таймер вызывается 1 раз и удалёется

    //заполнить из универсального таймера
    void FromTimer(SoftTimerBase _Timer, uint64_t _Counter, uint64_t _Delay)
    {
        Func = _Timer.Func;
        Params = _Timer.Params;
        Config = _Timer.Config;
        MaxWorkTime = 0;
        Counter = _Counter;
        Delay = _Delay;
    }
private:
};
