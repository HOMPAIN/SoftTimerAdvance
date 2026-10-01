#include "SoftTimerAdvanceInterface.h"

//перевод времени из единиц таймера _From в единицы _To, при переполнении возвращает максимум
static uint32_t ConvertUnits(uint32_t _Time, uint16_t _From, uint16_t _To)
{
    for (uint16_t i = _From; i < _To; i++)
        _Time /= 1000;
    for (uint16_t i = _To; i < _From; i++)
    {
        if (_Time > 0xFFFFFFFFUL / 1000)
            return 0xFFFFFFFFUL;
        _Time *= 1000;
    }
    return _Time;
}

//вызвать как можно быстрее
void STBase::ForceCall()
{
    CounterSet(DelayGet());
}
//возвраает время до следующего запуска
uint32_t STBase::GetDelay(STUnits _Units)
{
    uint32_t counter = CounterGet();
    uint32_t delay = DelayGet();
    delay = (delay > counter) ? (delay - counter) : 0;
    return ConvertUnits(delay, Config.Units, (uint16_t)_Units);
}

//удалить таймер
void STBase::Delete()
{
	Func = 0;
}
//время работы функции таймера в микросекундах
uint16_t STBase::GetWorkTime()
{
    return MaxWorkTime;
}
//перезагрузить с новой задержкой
void SDelay::Reset(uint32_t _Delay, STUnits _Units)
{
	Config.Freeze = 0;
    DelaySet(_Delay, (uint16_t)_Units);
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
void STimer::SetPeriod(uint32_t _Period, STUnits _Units)
{
    DelaySet(_Period, (uint16_t)_Units);
}
//получить текущий период, _Units задаёт размерность времени
uint32_t STimer::GetPeriod(STUnits _Units)
{
    return ConvertUnits(DelayGet(), Config.Units, (uint16_t)_Units);
}
//сбросить таймер, будет вызван через период
void STimer::Reset()
{
    CounterSet(0);
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
    CounterSet(0);
}
//остановить отсчёт таймаута без удаления
void STimeout::Stop()
{
    Config.Freeze = 1;
}
//установить новое время таймаута
void STimeout::SetTimeout(uint32_t _Timeout, STUnits _Units)
{
    DelaySet(_Timeout, (uint16_t)_Units);
}
//получить текущее время таймаута
uint32_t STimeout::GetTimeout(STUnits _Units)
{
    return ConvertUnits(DelayGet(), Config.Units, (uint16_t)_Units);
}
