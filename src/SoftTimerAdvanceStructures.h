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
    BitsAuto = 2                    //разрешение выбирается автоматически по периоду таймера
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
    uint16_t Resolution : 2;        //разрешение таймера , 0 - 16 бит, 1 - 32 бита
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

    //доступ к счётчику и периоду таймера с учётом его разрешения
    uint32_t CounterGet();
    void CounterSet(uint32_t _Counter);
    uint32_t DelayGet();
    //задать период и единицы измерения, счётчик сбрасывается
    //если значение не помещается в 16-битный таймер, он переходит на более крупные единицы с округлением
    void DelaySet(uint32_t _Delay, uint16_t _Units);
private:
};

//класс описания таймера с разрешением 16 бит в пуле
class SoftTimer16 : public SoftTimerBase
{
public:
    uint16_t Counter;               //счётчик, при доходе до 0 функция таймера вызывается
    uint16_t Delay;                 //периуд таймера мс, если 0 то таймер вызывается 1 раз и удалёется
private:
};

//класс описания таймера с разрешением 32 бит в пуле
class SoftTimer32 : public SoftTimerBase
{
public:
    uint32_t Counter;               //счётчик, при доходе до 0 функция таймера вызывается
    uint32_t Delay;                 //периуд таймера мс, если 0 то таймер вызывается 1 раз и удалёется
private:
};

inline uint32_t SoftTimerBase::CounterGet()
{
    if (Config.Resolution == STResolution::Bits16)
        return ((SoftTimer16*)this)->Counter;
    return ((SoftTimer32*)this)->Counter;
}
inline void SoftTimerBase::CounterSet(uint32_t _Counter)
{
    if (Config.Resolution == STResolution::Bits16)
        ((SoftTimer16*)this)->Counter = (_Counter > 0xFFFF) ? 0xFFFF : (uint16_t)_Counter;
    else
        ((SoftTimer32*)this)->Counter = _Counter;
}
inline uint32_t SoftTimerBase::DelayGet()
{
    if (Config.Resolution == STResolution::Bits16)
        return ((SoftTimer16*)this)->Delay;
    return ((SoftTimer32*)this)->Delay;
}
inline void SoftTimerBase::DelaySet(uint32_t _Delay, uint16_t _Units)
{
    if (Config.Resolution == STResolution::Bits16)
    {
        //значение не помещается в 16 бит, переходим на более крупные единицы с округлением
        while (_Delay > 0xFFFF && _Units < STUnits::Seconds)
        {
            _Delay = _Delay / 1000 + ((_Delay % 1000) >= 500);
            _Units++;
        }
        if (_Delay > 0xFFFF)
            _Delay = 0xFFFF;
        Config.Units = _Units;
        ((SoftTimer16*)this)->Counter = 0;
        ((SoftTimer16*)this)->Delay = (uint16_t)_Delay;
    }
    else
    {
        Config.Units = _Units;
        ((SoftTimer32*)this)->Counter = 0;
        ((SoftTimer32*)this)->Delay = _Delay;
    }
}
