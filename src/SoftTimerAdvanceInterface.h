//тут описаны классы реализующие интерфесы взаимодействия с таймерами
#pragma once
#include <stdint.h>                 //для определения типов uint16_t...
#include "SoftTimerAdvanceStructures.h"

//базовый интерфейс
class STBase :protected SoftTimerBase
{
public:
    //вызвать как можно быстрее
    void ForceCell();
    //возвраает время до следующего запуска
    uint64_t GetDelay(STUnits _Units = STUnits::Milliseconds);
    //удалить таймер
    void Delete();
};
//интерфейс таймера
class STimer :public STBase
{
public:
    //задать новый период работы таймера, _Units задаёт размерность времени, по умолчанию миллисек
    void SetPeriod(uint64_t _Period, STUnits _Units = STUnits::Milliseconds);
    //получить текущий период, _Units задаёт размерность времени
    uint64_t GetPeriod(STUnits _Units = STUnits::Milliseconds);
    //сбросить таймер, будет вызван через период
    void Reset();
    //заморозить таймер
    void Freeze();
    //разморозить таймер
    void UnFreeze();
    //1 - таймер заморожен, 0 - нет
    uint16_t GetFreezeStatus();
};
//интерфейс отложенный запуск
class SDelay :public STBase
{
public:
    //перезагрузить с новой задержкой
    void Reset(uint64_t _Delay, STUnits _Units = STUnits::Milliseconds);
    //установить защиту от удаления
    void SetDeleteProtection(uint16_t _Enable);
    //прочитать, установлина ли защита от удаления
    uint16_t GetDeleteProtection();
    //0 - работает, 1 - остановлен
    uint16_t GetStatus();
};
//интерфейс таймаута
class STimeout :public STBase
{
public:
    //сбросить таймаут
    void Reset();
    //установить новое время таймаута
    void SetTimeout(uint64_t _Timeout, STUnits _Units = STUnits::Milliseconds);
    //получить текущее время таймаута
    uint64_t GetTimeout(STUnits _Units = STUnits::Milliseconds);
};
