//в данном файле описаны функции добавления таймеров

#include "SoftTimerAdvanceManger.h"
#include "SoftTimerAdvanceStructures.h"
#include "SoftTimerAdvanceInterface.h"

//поиск свободной ячейки (удалённого таймера) среди занятых ячеек заданного разрешения, 0 если такой нет
SoftTimerBase* SoftTimerManager::FindFree(STResolution _Res)
{
    if (_Res == STResolution::Bits16)
    {
        for (uint16_t i = 0; i < Count16; i++)
        {
            if (Timers16[i].Func == 0)
                return &Timers16[i];
        }
    }
    else
    {
        for (uint16_t i = 0; i < Count32; i++)
        {
            SoftTimer32* timer = Timers32End - 1 - i;
            if (timer->Func == 0)
                return timer;
        }
    }
    return 0;
}

//универсальаная функция добавления таймера
void* SoftTimerManager::AddManual(SoftTimerBase _Timer, uint32_t _Delay, STResolution _Res)
{
    //разрешение не задано, выбираем по периоду
    //микросекундные таймеры всегда 32 бита: 16-битный счётчик теряет время, если между вызовами Update проходит больше 65 мс
    if (_Res != STResolution::Bits16 && _Res != STResolution::Bits32)
        _Res = (_Delay > 0xFFFF || _Timer.Config.Units == STUnits::Microseconds) ? STResolution::Bits32 : STResolution::Bits16;

    uint16_t lock_write = 0;
    if (Lock) //уже идёт запись в другом месте, можно добавить таймер только в конец(возможно при создании таймера из прерывания)
        lock_write = 1;

    //активировать блокировку добавления
    Lock = 1;

    SoftTimerBase* ptr = 0;
    //ищу свободный таймер, возможно был удалён какой-нибудь таймер
    if (!lock_write)
        ptr = FindFree(_Res);

    //свободных нет, запись в конец: 16-битные растут от начала пула, 32-битные от конца
    if (ptr == 0)
    {
        uint32_t used = (uint32_t)Count16 * sizeof(SoftTimer16) + (uint32_t)Count32 * sizeof(SoftTimer32);
        if (_Res == STResolution::Bits16)
        {
            if (Count16 < 0xFFFF && used + sizeof(SoftTimer16) <= PoolSize)
                ptr = &Timers16[Count16++];
            //места нет, но 16-битный таймер можно положить в свободную 32-битную ячейку
            else if (!lock_write && (ptr = FindFree(STResolution::Bits32)) != 0)
                _Res = STResolution::Bits32;
        }
        else
        {
            if (Count32 < 0xFFFF && used + sizeof(SoftTimer32) <= PoolSize)
                ptr = Timers32End - 1 - Count32++;
        }
    }

    //больше нет доступных таймеров
    if (ptr == 0)
    {
        //ErrorBrakepoint();
        Lock = 0;              //выключить блокировку добавления
        return 0;
    }

    //функцию записываем последней, до этого ячейка считается свободной
    _Timer.Config.Resolution = _Res;
    ptr->Params = _Timer.Params;
    ptr->Config = _Timer.Config;
    ptr->MaxWorkTime = 0;
    ptr->DelaySet(_Delay, _Timer.Config.Units);
    ptr->Func = _Timer.Func;

    //выключить блокировку добавления
    Lock = 0;
    return ptr;
}

//добавление таймера в микро, милли или секундах
STimer* SoftTimerManager::AddTimer(VoidFuncST _Func, uint32_t _Period, STUnits _Units, STResolution _Res)
{
    SoftTimerBase timer;
    timer.Func = _Func;
    timer.MaxWorkTime = 0;
    timer.Params = 0;
    timer.Config.Type = 1;//timer
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 0;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimer*)AddManual(timer, _Period, _Res);
}
STimer* SoftTimerManager::AddTimer(PrmFuncST _Func, void* _Prm, uint32_t _Period, STUnits _Units, STResolution _Res)
{
    SoftTimerBase timer;
    timer.Func = (VoidFuncST)_Func;
    timer.MaxWorkTime = 0;
    timer.Params = _Prm;
    timer.Config.Type = 1;//timer
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 1;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimer*)AddManual(timer, _Period, _Res);
}

//добавление задачи отложенного запуска в микро, милли или секундах
SDelay* SoftTimerManager::AddDelayCall(VoidFuncST _Func, uint32_t _Period, STUnits _Units, STResolution _Res)
{
    SoftTimerBase timer;
    timer.Func = _Func;
    timer.MaxWorkTime = 0;
    timer.Params = 0;
    timer.Config.Type = 2;//delay
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 0;
    timer.Config.PrmEnable = 0;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (SDelay*)AddManual(timer, _Period, _Res);
}
SDelay* SoftTimerManager::AddDelayCall(PrmFuncST _Func, void* _Prm, uint32_t _Period, STUnits _Units, STResolution _Res)
{
    SoftTimerBase timer;
    timer.Func = (VoidFuncST)_Func;
    timer.MaxWorkTime = 0;
    timer.Params = _Prm;
    timer.Config.Type = 2;//delay
    timer.Config.Freeze = 0;
    timer.Config.DeleteProtection = 0;
    timer.Config.PrmEnable = 1;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (SDelay*)AddManual(timer, _Period, _Res);
}
STimeout* SoftTimerManager::AddTimeout(VoidFuncST _Func, uint32_t _Timeout, STUnits _Units, STResolution _Res)
{
    SoftTimerBase timer;
    timer.Func = _Func;
    timer.MaxWorkTime = 0;
    timer.Params = 0;
    timer.Config.Type = 3;//timeout
    timer.Config.Freeze = 1;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 0;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimeout*)AddManual(timer, _Timeout, _Res);
}
STimeout* SoftTimerManager::AddTimeout(PrmFuncST _Func, void* _Prm, uint32_t _Timeout, STUnits _Units, STResolution _Res)
{
    SoftTimerBase timer;
    timer.Func = (VoidFuncST)_Func;
    timer.MaxWorkTime = 0;
    timer.Params = _Prm;
    timer.Config.Type = 3;//timeout
    timer.Config.Freeze = 1;
    timer.Config.DeleteProtection = 1;
    timer.Config.PrmEnable = 1;
    timer.Config.StrictMode = 0;
    timer.Config.Units = (uint16_t)_Units;
    return (STimeout*)AddManual(timer, _Timeout, _Res);
}
