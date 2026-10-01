#pragma once
#include <stdint.h>                 //для определения типов uint16_t...

//структура телеметрии таймера
/*typedef struct
{
    uint32_t uSeconds;              //микросекунды для работы телеметрии, нужно добавить обновление в обычном таймер(не обязательно)
    int16_t  MaxWorkTime;           //время долгих функций в микро сек
    uint16_t CPULoad;               //загрузка процессора % 0-100, для работы необходимо обновлять STuSeconds
    uint16_t WatchDogTime;          //максимально допустимое время одной функции

    void * LastFunc;                //последняя выполненая функция
    uint32_t LastFuncTime;          //врем работы последней функции
}SoftTimerTelemetry;
*/

//enum class, чтобы имена значений не попадали в глобальную область имён. Использовать как STUnits::Seconds
enum class STResolution : uint16_t {
    Bits16 = 0,
    Bits32 = 1,
    BitsAuto = 2                    //разрешение выбирается автоматически по периоду таймера
};

enum class STUnits : uint16_t {
    Microseconds = 0,
    Milliseconds = 1,
    Seconds = 2
};

//режим работы таймера: что делать, если его функцию не успели вызвать вовремя
//(например, другая функция долго работала или программа была занята)
enum class STMode : uint16_t {
    Strict = 0,                     //строгий: пропущенные вызовы копятся и выполняются подряд, общее число вызовов сохраняется
    Skip = 1,                       //по умолчанию: вызовы не копятся. Небольшое опоздание компенсируется и частота сохраняется,
                                    //при опоздании больше периода пропущенные вызовы отбрасываются
    Restart = 2                     //период отсчитывается от момента вызова: между вызовами проходит не меньше периода
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
    uint16_t StrictMode : 2;          //режим работы таймера (STMode): 0 - строгий режим(без пропусков), 1 пытается сохранить частоту, но допускает пропуски выполнеия, 2 - сбрасывает счётчик при каждом запуске
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

    //заявка из прерывания. Прерывание само таймер не меняет, а только записывает сюда, что нужно сделать,
    //заявку выполняет основной цикл в Update. Request пишется только из прерывания, RequestDone только
    //из основного цикла, каждый занимает один байт, поэтому запрещать прерывания не нужно
    uint8_t Request;                //младшие 4 бита - что сделать (ST_REQ_...), старшие 4 бита - номер заявки
    uint8_t RequestDone;            //последняя выполненная заявка

    //доступ к счётчику и периоду таймера с учётом его разрешения
    uint32_t CounterGet();
    void CounterSet(uint32_t _Counter);
    uint32_t DelayGet();
    //задать период и единицы измерения, счётчик сбрасывается
    //если значение не помещается в 16-битный таймер, он переходит на более крупные единицы с округлением
    void DelaySet(uint32_t _Delay, uint16_t _Units);

    //поставить заявку из прерывания: _Mask - какие биты заявки заменить, _Bits - их новые значения
    void RequestSet(uint8_t _Mask, uint8_t _Bits);
    //выполнить заявку из прерывания, вызывается из основного цикла
    void RequestApply();
    //1 - есть невыполненная заявка из прерывания
    uint16_t RequestPending();
private:
};

//биты заявки из прерывания
const uint8_t ST_REQ_COUNTER = 0x01;    //изменить счётчик
const uint8_t ST_REQ_FORCE = 0x02;      //1 - счётчик равен периоду (вызвать как можно быстрее), 0 - счётчик равен 0 (сброс)
const uint8_t ST_REQ_FREEZE = 0x04;     //изменить заморозку
const uint8_t ST_REQ_FROZEN = 0x08;     //1 - заморозить, 0 - разморозить

//класс описания таймера с разрешением 16 бит в пуле
class SoftTimer16 : public SoftTimerBase
{
public:
    uint16_t Counter;               //счётчик прошедшего времени, когда он доходит до Delay, функция таймера вызывается
    uint16_t Delay;                 //период таймера (задержка, время таймаута) в единицах Config.Units
private:
};

//класс описания таймера с разрешением 32 бит в пуле
class SoftTimer32 : public SoftTimerBase
{
public:
    uint32_t Counter;               //счётчик прошедшего времени, когда он доходит до Delay, функция таймера вызывается
    uint32_t Delay;                 //период таймера (задержка, время таймаута) в единицах Config.Units
private:
};

inline uint32_t SoftTimerBase::CounterGet()
{
    if (Config.Resolution == (uint16_t)STResolution::Bits16)
        return ((SoftTimer16*)this)->Counter;
    return ((SoftTimer32*)this)->Counter;
}
inline void SoftTimerBase::CounterSet(uint32_t _Counter)
{
    if (Config.Resolution == (uint16_t)STResolution::Bits16)
        ((SoftTimer16*)this)->Counter = (_Counter > 0xFFFF) ? 0xFFFF : (uint16_t)_Counter;
    else
        ((SoftTimer32*)this)->Counter = _Counter;
}
inline uint32_t SoftTimerBase::DelayGet()
{
    if (Config.Resolution == (uint16_t)STResolution::Bits16)
        return ((SoftTimer16*)this)->Delay;
    return ((SoftTimer32*)this)->Delay;
}
inline void SoftTimerBase::DelaySet(uint32_t _Delay, uint16_t _Units)
{
    if (Config.Resolution == (uint16_t)STResolution::Bits16)
    {
        //значение не помещается в 16 бит, переходим на более крупные единицы с округлением
        while (_Delay > 0xFFFF && _Units < (uint16_t)STUnits::Seconds)
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
//поставить заявку из прерывания
inline void SoftTimerBase::RequestSet(uint8_t _Mask, uint8_t _Bits)
{
    volatile uint8_t* request = &Request;
    volatile uint8_t* request_done = &RequestDone;
    uint8_t req = *request;
    uint8_t done = *request_done;
    //если предыдущая заявка ещё не выполнена, её биты сохраняются и дополняются новыми
    uint8_t bits = (req == done) ? 0 : (req & 0x0F);
    bits = (uint8_t)((bits & ~_Mask) | _Bits);
    //номер заявки на 1 больше, чем у выполненной, поэтому новая заявка всегда от неё отличается
    *request = (uint8_t)(((done & 0xF0) + 0x10) | bits);
}
//1 - есть невыполненная заявка из прерывания
inline uint16_t SoftTimerBase::RequestPending()
{
    volatile uint8_t* request = &Request;
    return *request != RequestDone;
}
//выполнить заявку из прерывания, вызывается из основного цикла
inline void SoftTimerBase::RequestApply()
{
    volatile uint8_t* request = &Request;
    volatile uint8_t* request_done = &RequestDone;
    uint8_t req = *request;
    if (req == *request_done)
        return;//новых заявок нет
    if (req & ST_REQ_FREEZE)
        Config.Freeze = (req & ST_REQ_FROZEN) ? 1 : 0;
    if (req & ST_REQ_COUNTER)
        CounterSet((req & ST_REQ_FORCE) ? DelayGet() : 0);
    //если прерывание изменило заявку, пока она выполнялась, Request уже не равен req
    //и заявка будет выполнена ещё раз при следующем вызове
    *request_done = req;
}
