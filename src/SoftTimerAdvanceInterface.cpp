#include "SoftTimerAdvanceInterface.h"

//вызвать как можно быстрее
void STBase::ForceCell()
{
    switch (Config.Resolution)
    {
    case 0://16 бит
        ((SoftTimer16*)this)->Counter = ((SoftTimer16*)this)->Delay;
        break;
    case 1://32 бит
        ((SoftTimer32*)this)->Counter = ((SoftTimer32*)this)->Delay;
        break;
    case 2://64 бит
        ((SoftTimer64*)this)->Counter = ((SoftTimer64*)this)->Delay;
        break;
    }
}
//возвраает время до следующего запуска
uint64_t STBase::GetDelay(STUnits _Units)
{
    uint64_t delay = 0;
    switch (Config.Resolution)
    {
    case 0://16 бит
        delay = (((SoftTimer16*)this)->Delay > ((SoftTimer16*)this)->Counter) ? (((SoftTimer16*)this)->Delay - ((SoftTimer16*)this)->Counter) : 0;
        break;
    case 1://32 бит
        delay = (((SoftTimer32*)this)->Delay > ((SoftTimer32*)this)->Counter) ? (((SoftTimer32*)this)->Delay - ((SoftTimer32*)this)->Counter) : 0;
        break;
    case 2://64 бит
        delay = (((SoftTimer64*)this)->Delay > ((SoftTimer64*)this)->Counter) ? (((SoftTimer64*)this)->Delay - ((SoftTimer64*)this)->Counter) : 0;
        break;
    }
    for (int i = Config.Units; i < _Units; i++)
        delay /= 1000;
    for (int i = _Units; i < Config.Units; i++)
        delay *= 1000;
    return delay;
}

//удалить таймер
void STBase::Delete()
{
	Func = 0;
}
//перезагрузить с новой задержкой
void SDelay::Reset(uint64_t _Delay, STUnits _Units)
{
	Config.Freeze = 0;
    Config.Units = _Units;
    switch (Config.Resolution)
    {
    case 0://16 бит
        ((SoftTimer16*)this)->Counter = 0; ((SoftTimer16*)this)->Delay = _Delay;
        break;
    case 1://32 бит
        ((SoftTimer32*)this)->Counter = 0; ((SoftTimer32*)this)->Delay = _Delay;
        break;
    case 2://64 бит
        ((SoftTimer64*)this)->Counter = 0; ((SoftTimer64*)this)->Delay = _Delay;
        break;
    }
}
//установить защиту от удаления
void SDelay::SetDeleteProtection(uint16_t _Enable)
{
    Config.DeleteProtection = _Enable;
}
//прочитать, установлина ли защита от удаления
uint16_t SDelay::GetDeleteProtection()
{
    return Config.DeleteProtection;
}
//0 - работает, 1 - остановлен
uint16_t SDelay::GetStatus()
{
    return Config.Freeze;
}
//задать новый период работы таймера, _Units задаёт размерность времени, по умолчанию миллисек
void STimer::SetPeriod(uint64_t _Period, STUnits _Units)
{
    Config.Units = _Units;
    switch (Config.Resolution)
    {
    case 0://16 бит
        ((SoftTimer16*)this)->Counter = 0; ((SoftTimer16*)this)->Delay = _Period;
        break;
    case 1://32 бит
        ((SoftTimer32*)this)->Counter = 0; ((SoftTimer32*)this)->Delay = _Period;
        break;
    case 2://64 бит
        ((SoftTimer64*)this)->Counter = 0; ((SoftTimer64*)this)->Delay = _Period;
        break;
    }
}
//получить текущий период, _Units задаёт размерность времени
uint64_t STimer::GetPeriod(STUnits _Units)
{
    uint64_t period=0;
    switch (Config.Resolution)
    {
    case 0://16 бит
        period = ((SoftTimer16*)this)->Delay;
        break;
    case 1://32 бит
        period = ((SoftTimer32*)this)->Delay;
        break;
    case 2://64 бит
        period = ((SoftTimer64*)this)->Delay;
        break;
    }
    for (int i = Config.Units; i < _Units; i++)
        period /= 1000;
    for (int i = _Units; i < Config.Units; i++)
        period *= 1000;
    return period;
}
//сбросить таймер, будет вызван через период
void STimer::Reset()
{
    switch (Config.Resolution)
    {
    case 0://16 бит
        ((SoftTimer16*)this)->Counter = 0;
        break;
    case 1://32 бит
        ((SoftTimer32*)this)->Counter = 0;
        break;
    case 2://64 бит
        ((SoftTimer64*)this)->Counter = 0;
        break;
    }
}
//заморозить таймер
void STimer::Freeze()
{
    Config.Freeze = 1;
}
//разморозить таймер
void STimer::UnFreeze()
{
    Config.Freeze = 0;
}
//1 - таймер заморожен, 0 - нет
uint16_t STimer::GetFreezeStatus()
{
    return Config.Freeze;
}
//сбросить таймаут
void STimeout::Reset()
{
    Config.Freeze = 0;
    switch (Config.Resolution)
    {
    case 0://16 бит
        ((SoftTimer16*)this)->Counter = 0;
        break;
    case 1://32 бит
        ((SoftTimer32*)this)->Counter = 0;
        break;
    case 2://64 бит
        ((SoftTimer64*)this)->Counter = 0;
        break;
    }
}
//установить новое время таймаута
void STimeout::SetTimeout(uint64_t _Timeout, STUnits _Units)
{
    Config.Units = _Units;
    switch (Config.Resolution)
    {
    case 0://16 бит
        ((SoftTimer16*)this)->Counter = 0; ((SoftTimer16*)this)->Delay = _Timeout;
        break;
    case 1://32 бит
        ((SoftTimer32*)this)->Counter = 0; ((SoftTimer32*)this)->Delay = _Timeout;
        break;
    case 2://64 бит
        ((SoftTimer64*)this)->Counter = 0; ((SoftTimer64*)this)->Delay = _Timeout;
        break;
    }
}
//получить текущее время таймаута
uint64_t STimeout::GetTimeout(STUnits _Units)
{
    uint64_t timeout = 0;
    switch (Config.Resolution)
    {
    case 0://16 бит
        timeout = ((SoftTimer16*)this)->Delay;
        break;
    case 1://32 бит
        timeout = ((SoftTimer32*)this)->Delay;
        break;
    case 2://64 бит
        timeout = ((SoftTimer64*)this)->Delay;
        break;
    }
    for (int i = Config.Units; i < _Units; i++)
        timeout /= 1000;
    for (int i = _Units; i < Config.Units; i++)
        timeout *= 1000;
    return timeout;
}