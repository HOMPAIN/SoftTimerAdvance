//в данном файле описаны функции добавления таймеров

#include "SoftTimerAdvanceManger.h"
#include "SoftTimerAdvanceStructures.h"
#include "SoftTimerAdvanceInterface.h"

//универсальаная функция добавления таймера
void* SoftTimerManager::AddManual(SoftTimerBase _Timer, uint64_t _Counter, uint64_t _Delay)
{
    _Timer.Config.Resolution = Resolution;

    uint16_t lock_write = 0;
    uint16_t write_to_end = 1;
    if (Lock) //уже идёт запись в другом месте, можно добавить таймер только в конец(возможно при создании таймера из прерывания)
        lock_write = 1;

    //активировать блокировку добавления
    Lock = 1; 

    int n = Count;
    if (!lock_write)
    {
        //ищу свободный таймер, возможно был удалён какой-нибудь таймер
        for (int i = 0; i < Count; i++)
        {
            VoidFuncST func = 0;
            switch (Resolution)
            {
            case 0://16 бит
                func = Timers16[i].Func;
                break;
            case 1://32 бит
                func = Timers32[i].Func;
                break;
            case 2://64 бит
                func = Timers64[i].Func;
                break;
            }
            if (func == 0)
            {
                n = i;
                write_to_end = 0;
                break;
            }
        }
    }
    //больше нет доступных таймеров
    if (n >= Size)
    {
        //ErrorBrakepoint();
        Lock = 0;              //выключить блокировку добавления
        return 0;
    }

    if (write_to_end)                   //запись в конец
        n = Count++;

    void* ptr = 0;
    switch (Resolution)
    {
    case 0://16 бит
        Timers16[n].FromTimer(_Timer, _Counter, _Delay);
        ptr = &Timers16[n];
        break;
    case 1://32 бит
        Timers32[n].FromTimer(_Timer, _Counter, _Delay);
        ptr = &Timers32[n];
        break;
    case 2://64 бит
        Timers64[n].FromTimer(_Timer, _Counter, _Delay);
        ptr = &Timers64[n];
        break;
    }

    //выключить блокировку добавления
    Lock = 0;                  
    return ptr;
}

//добавление таймера в микро, милли или секундах
STimer* SoftTimerManager::AddTimer(VoidFuncST _Func, uint64_t _Period, STUnits _Units)
{
    SoftTimerBase timer;
    timer.Func = _Func;
    //timer.Counter = 0;
    //timer.Delay = _Period;
    timer.MaxWorkTime = 0;
    timer.Params = 0;
    timer.Config.Type = 1;//timer
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 0;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimer*)AddManual(timer, 0, _Period);
}
STimer* SoftTimerManager::AddTimer(PrmFuncST _Func, void* _Prm, uint64_t _Period, STUnits _Units)
{
    SoftTimerBase timer;
    timer.Func = (VoidFuncST)_Func;
    //timer.Counter = 0;
    //timer.Delay = _Period;
    timer.MaxWorkTime = 0;
    timer.Params = _Prm;
    timer.Config.Type = 1;//timer
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 1;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimer*)AddManual(timer, 0, _Period);
}

//добавление задачи отложенного запуска в микро, милли или секундах
SDelay* SoftTimerManager::AddDelayCall(VoidFuncST _Func, uint64_t _Period, STUnits _Units)
{
    SoftTimerBase timer;
    timer.Func = _Func;
    //timer.Counter = 0;
    //timer.Delay = _Period;
    timer.MaxWorkTime = 0;
    timer.Params = 0;
    timer.Config.Type = 2;//delay
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 0;
    timer.Config.PrmEnable = 0;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (SDelay*)AddManual(timer, 0, _Period);
}
SDelay* SoftTimerManager::AddDelayCall(PrmFuncST _Func, void* _Prm, uint64_t _Period, STUnits _Units)
{
    SoftTimerBase timer;
    timer.Func = (VoidFuncST)_Func;
    //timer.Counter = 0;
    //timer.Delay = _Period;
    timer.MaxWorkTime = 0;
    timer.Params = _Prm;
    timer.Config.Type = 2;//delay
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 0;
    timer.Config.PrmEnable = 1;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (SDelay*)AddManual(timer, 0, _Period);
}
STimeout* SoftTimerManager::AddTimeout(VoidFuncST _Func, uint64_t _Timeout, STUnits _Units)
{
    SoftTimerBase timer;
    timer.Func = _Func;
    //timer.Counter = 0;
    //timer.Delay = _Period;
    timer.MaxWorkTime = 0;
    timer.Params = 0;
    timer.Config.Type = 3;//timeout
    timer.Config.Freeze = 1;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 0;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimeout*)AddManual(timer, 0, _Timeout);
}
STimeout* SoftTimerManager::AddTimeout(PrmFuncST _Func, void* _Prm, uint64_t _Timeout, STUnits _Units)
{
    SoftTimerBase timer;
    timer.Func = (VoidFuncST)_Func;
    //timer.Counter = 0;
    //timer.Delay = _Period;
    timer.MaxWorkTime = 0;
    timer.Params = _Prm;
    timer.Config.Type = 3;//timeout
    timer.Config.Freeze = 1;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 1;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimeout*)AddManual(timer, 0, _Timeout);
}