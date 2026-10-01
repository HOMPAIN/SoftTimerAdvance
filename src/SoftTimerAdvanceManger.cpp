#include "SoftTimerAdvanceManger.h"

void SoftTimerTelemetry_pp::Tick(uint32_t _Dt)
{
    //расчёт % загрузки процессора
    MeasTime+=_Dt*1000;
    if(MeasTime>3000000)
    {
        CPULoad = WorkTime*100/MeasTime;
        WorkTime=0;
        MeasTime=0;
    }

    //расчёт времени самого долгого таймера
    uint16_t d = (_Dt/16);
    if(d==0)d=1;
    if (MaxWorkTime > d)
        MaxWorkTime-=d;
    else
        MaxWorkTime=0;
}
void SoftTimerTelemetry_pp::AddWorkTime(uint32_t _WorkTime)
{
    LastFancTime = _WorkTime;
    WorkTime+=_WorkTime;

    if(MaxWorkTime<_WorkTime)
        MaxWorkTime = _WorkTime;
}
//инициализация парамтетров мэнеджера, возращает количество таймеров доступных в пуле
//(считается по 32-битным таймерам, 16-битных помещается больше)
//_Buff - буффер для пула таймеров, _BuffSize - размер буффера в байтах
uint16_t SoftTimerManager::Init(uint8_t* _Buff, uint32_t _BuffSize)
{
    memset(this,0,sizeof(SoftTimerManager));

    if (_Buff == 0)
        return 0;
    //границы пула выравниваем под структуры таймеров
    uintptr_t align = alignof(SoftTimer32);
    uintptr_t begin = ((uintptr_t)_Buff + align - 1) / align * align;
    uintptr_t end = ((uintptr_t)_Buff + _BuffSize) / align * align;
    if (end <= begin)
        return 0;
    //16-битные таймеры заполняют пул с начала, 32-битные с конца
    Timers16 = (SoftTimer16*)begin;
    Timers32End = (SoftTimer32*)end;
    PoolSize = (uint32_t)(end - begin);
    //очищаем пул, в буфере может быть мусор
    memset((void*)begin, 0, PoolSize);

    uint32_t size = PoolSize / sizeof(SoftTimer32);
    Size = (size > 0xFFFF) ? 0xFFFF : (uint16_t)size;
    return Size;
}
//работа таймера, передаётся текущее время, можно вызывать любую функцию
//возвращает время до следующего срабатывания таймера в тех же единицах, в которых передано время
uint32_t SoftTimerManager::Update(uint64_t _Time, STUnits _Units)
{
    switch (_Units)
    {
    case STUnits::Microseconds:
        return Update_us(_Time);
    case STUnits::Seconds:
        return Update_s(_Time);
    default:
        return Update_ms(_Time);
    }
}
uint32_t SoftTimerManager::Update_us(uint64_t _uSeconds)
{
    uint32_t dt_us = TimeDelta(_uSeconds, STUnits::Microseconds);
    uint32_t dt_ms = DtDiv1000(dt_us, &RemUs);
    uint32_t dt_s = DtDiv1000(dt_ms, &RemMs);
    Working(dt_us, dt_ms, dt_s);
    return GetTimeToNext(STUnits::Microseconds);
}
uint32_t SoftTimerManager::Update_ms(uint64_t _mSeconds)
{
    uint32_t dt_ms = TimeDelta(_mSeconds, STUnits::Milliseconds);
    uint32_t dt_s = DtDiv1000(dt_ms, &RemMs);
    Working((uint64_t)dt_ms * 1000, dt_ms, dt_s);
    return GetTimeToNext(STUnits::Milliseconds);
}
uint32_t SoftTimerManager::Update_s(uint64_t _Seconds)
{
    uint32_t dt_s = TimeDelta(_Seconds, STUnits::Seconds);
    uint32_t dt_ms = (dt_s > 0xFFFFFFFFUL / 1000) ? 0xFFFFFFFFUL : dt_s * 1000;
    Working((uint64_t)dt_s * 1000000, dt_ms, dt_s);
    return GetTimeToNext(STUnits::Seconds);
}

//деление с округлением вверх
static uint32_t CeilDiv(uint32_t _Value, uint32_t _Div)
{
    uint32_t res = _Value / _Div;
    if (res * _Div != _Value)
        res++;
    return res;
}

//время до ближайшего срабатывания таймера, _Units задаёт размерность времени (округляется вверх)
//0 - есть таймер, готовый к запуску, 0xFFFFFFFF - работающих таймеров нет (или значение не помещается в 32 бита)
uint32_t SoftTimerManager::GetTimeToNext(STUnits _Units)
{
    //минимальное оставшееся время по единицам измерения таймера (STUnits)
    uint32_t left_units[4] = { 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL };

    //16-битные таймеры
    for (uint16_t i = 0; i < Count16; i++)
    {
        SoftTimer16* timer = &Timers16[i];
        if (timer->Func == 0 || timer->Config.Freeze == 1)
            continue;
        if (timer->Counter >= timer->Delay)
            return 0;
        uint32_t left = timer->Delay - timer->Counter;
        if (left < left_units[timer->Config.Units])
            left_units[timer->Config.Units] = left;
    }

    //32-битные таймеры
    for (uint16_t i = 0; i < Count32; i++)
    {
        SoftTimer32* timer = Timers32End - 1 - i;
        if (timer->Func == 0 || timer->Config.Freeze == 1)
            continue;
        if (timer->Counter >= timer->Delay)
            return 0;
        uint32_t left = timer->Delay - timer->Counter;
        if (left < left_units[timer->Config.Units])
            left_units[timer->Config.Units] = left;
    }

    uint32_t left_us = left_units[STUnits::Microseconds];
    uint32_t left_ms = left_units[STUnits::Milliseconds];
    uint32_t left_s = left_units[STUnits::Seconds];
    const uint32_t none = 0xFFFFFFFFUL;//таймеров с такими единицами нет

    //перевод в запрошенные единицы. Счётчики миллисекунд и секунд увеличиваются, когда накопится
    //целая единица, поэтому вычитаем уже накопленные остатки RemUs и RemMs
    uint32_t next = none;
    uint64_t time;
    switch (_Units)
    {
    case STUnits::Microseconds:
        next = left_us;
        if (left_ms != none)
        {
            time = (uint64_t)left_ms * 1000 - RemUs;
            if (time < next)
                next = (uint32_t)time;
        }
        if (left_s != none)
        {
            time = ((uint64_t)left_s * 1000 - RemMs) * 1000 - RemUs;
            if (time < next)
                next = (uint32_t)time;
        }
        break;
    case STUnits::Seconds:
        next = left_s;
        if (left_ms != none && CeilDiv(left_ms, 1000) < next)
            next = CeilDiv(left_ms, 1000);
        if (left_us != none && CeilDiv(left_us, 1000000UL) < next)
            next = CeilDiv(left_us, 1000000UL);
        break;
    default://миллисекунды
        next = left_ms;
        if (left_s != none && left_s <= none / 1000 && left_s * 1000 - RemMs < next)
            next = left_s * 1000 - RemMs;
        if (left_us != none && CeilDiv(left_us, 1000) < next)
            next = CeilDiv(left_us, 1000);
        break;
    }
    return next;
}

//возвращает время, прошедшее с предыдущего вызова Update, в единицах источника времени
//разность считается в 32 битах, поэтому переполнение 32-битных счётчиков millis()/micros() не мешает
uint32_t SoftTimerManager::TimeDelta(uint64_t _Time, STUnits _Units)
{
    uint32_t time = (uint32_t)_Time;
    uint32_t dt = 0;
    if (TimeStarted && TimeUnits == _Units)
        dt = time - TimeLast;
    else
    {
        //первый запуск, время работы начинаем с переданного значения
        if (!TimeStarted)
        {
            uint64_t ms = _Time;
            uSeconds = _Time;
            if (_Units == STUnits::Microseconds)
            {
                ms = _Time / 1000;
                RemUs = (uint16_t)(_Time - ms * 1000);
            }
            else if (_Units == STUnits::Seconds)
                ms = _Time * 1000;
            if (_Units != STUnits::Microseconds)
                uSeconds = ms * 1000;
            uint64_t s = ms / 1000;
            RemMs = (uint16_t)(ms - s * 1000);
            mSeconds = (uint32_t)ms;
            Seconds = (uint32_t)s;
        }
        //при смене единиц источника времени просто начинаем отсчёт заново
        TimeStarted = 1;
        TimeUnits = _Units;
    }
    TimeLast = time;
    return dt;
}
//перевод приращения времени в более крупные единицы (делитель 1000), остаток накапливается в _Rem
uint32_t SoftTimerManager::DtDiv1000(uint32_t _Dt, uint16_t* _Rem)
{
    uint32_t dt = 0;
    if (_Dt >= 1000)
    {
        dt = _Dt / 1000;
        _Dt -= dt * 1000;
    }
    *_Rem += (uint16_t)_Dt;
    if (*_Rem >= 1000)
    {
        *_Rem -= 1000;
        dt++;
    }
    return dt;
}

//цикл прохода по таймерам, передаётся время, прошедшее с предыдущего вызова
void SoftTimerManager::Working(uint64_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s)
{
    //время работы таймера
    uSecondsLast = uSeconds;
    mSecondsLast = mSeconds;
    SecondsLast = Seconds;
    uSeconds += _Dt_us;
    mSeconds += _Dt_ms;
    Seconds += _Dt_s;

    //счётчик таймеров
    if(_Dt_us !=0)
    {
        uint32_t dt_us = (_Dt_us > 0xFFFFFFFFUL) ? 0xFFFFFFFFUL : (uint32_t)_Dt_us;
        Tick(dt_us, _Dt_ms, _Dt_s);

        //Telemetry.Tick(dt);

    }

    //таймеров нет, в пуле нечего проверять
    uint16_t count = Count16 + Count32;
    if (count == 0)
        return;

    //счётчик(номер таймера) выполнения теймеров, по одному за запуск
    Iterator++;
    if (Iterator >= count)
        Iterator = 0;

    //сначала перебираются 16-битные таймеры, за ними 32-битные
    SoftTimerBase* timer_base = 0;
    uint16_t* pool_count = 0;   //количество занятых ячеек того же разрешения, что и таймер
    uint16_t n = 0;             //номер таймера среди ячеек своего разрешения
    if (Iterator < Count16)
    {
        n = Iterator;
        timer_base = &Timers16[n];
        pool_count = &Count16;
    }
    else
    {
        n = Iterator - Count16;
        timer_base = Timers32End - 1 - n;
        pool_count = &Count32;
    }

    //таймер не занят, нужно удалить если он в конце
    if (timer_base->Func == 0)
    {
        //таймер находится в конце?
        if ((n + 1) == *pool_count)
        {
            (*pool_count)--;
            //кто-то добавил таймер из прерывания, вернём обратно
            if (timer_base->Func != 0)
                (*pool_count)++;
        }
        return;
    }

    //таймер заморожен, пропускаем
    if (timer_base->Config.Freeze)
        return;

    //проверка готовности таймера, если таймер не готов, выходим
    uint32_t counter = timer_base->CounterGet();
    uint32_t delay = timer_base->DelayGet();
    if (counter < delay)
        return;

    //запамянам функцию, для телеметрии
    Telemetry.LastFanc = (VoidFuncST)timer_base->Func;

    //обработка счётчика таймера
    if (timer_base->Config.Type == 1)
    {
        //режим отработки времени
        switch (timer_base->Config.StrictMode)
        {
        case 1://пытается сохранить частоту, но допускает пропуски выполнеия
            counter -= delay;
            if (counter >= delay)
                counter = 0; //таймер не успевает, пропуск
            break;
        case 2://сбрасывает счётчик при каждом запуске
            counter = 0;
            break;
        default://строгий режим, он же по умолчанию
            counter -= delay;
            break;
        }
        timer_base->CounterSet(counter);
    }
    //для единичного запуска и таймаута счётчик не важен
    else
    {
        timer_base->Config.Freeze = 1; //замораживаем таймер
    }

    //запуск телеметрии
    uint32_t time_start = uSeconds;

    //выполнение функии таймера
    //if(Telemetry.LedGPIO>=0)
    //    digitalWrite(Telemetry.LedGPIO, HIGH);//светодиод загрузки процессора
    if (timer_base->Config.PrmEnable == 0)//таймер без параметров
        timer_base->Func();
    else//таймер с параметрами
        ((PrmFuncST)timer_base->Func)(timer_base->Params);
    //if(Telemetry.LedGPIO>=0)
    //    digitalWrite(Telemetry.LedGPIO, LOW);//светодиод загрузки процессора

    //остановка телеметрии
    uint32_t work_time = uSeconds - time_start;
    Telemetry.AddWorkTime(work_time);

    //запись телеметрии в таймер
    if (timer_base->Config.Type == 1)//для простого таймера делаем доп усредение
    {
        if (Telemetry.LastFancTime >= timer_base->MaxWorkTime || timer_base->DelayGet() >= 1000)
            timer_base->MaxWorkTime = Telemetry.LastFancTime;
        else if (timer_base->MaxWorkTime > 0)
            timer_base->MaxWorkTime--;
    }else
        timer_base->MaxWorkTime = Telemetry.LastFancTime;


    //удаляем объект таймера после запуска, если он не защищён от удаления
    if (!timer_base->Config.DeleteProtection)
    {
        timer_base->Func = 0;
        if ((n + 1) == *pool_count)
        {
            (*pool_count)--;
            if (timer_base->Func != 0) //кто-то добавил таймер из прерывания, вернём обратно
                (*pool_count)++;
        }
    }

    //WatchDog
    if(Telemetry.LastFancTime>Telemetry.WatchDogTime && Telemetry.WatchDogTime!=0)
    {
        Telemetry.LastFanc=Telemetry.LastFanc;//эта функция работала дольше чем нужно
        //ErrorBrakepoint();
        Telemetry.LastFancTime=0;
    }
}
//счётчик времени таймеров
void SoftTimerManager::Tick(uint32_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s)
{
    //приращение времени по единицам измерения таймера (STUnits)
    uint32_t dt_units[4] = { _Dt_us, _Dt_ms, _Dt_s, 0 };

    //16-битные таймеры
    for (uint16_t i = 0; i < Count16; i++)
    {
        SoftTimer16* timer = &Timers16[i];
        if (timer->Func == 0)
            continue;
        if (timer->Config.Freeze == 1)
            continue;
        uint32_t dt = dt_units[timer->Config.Units];
        //защита от переполнения
        if (dt > (uint16_t)(0xFFFF - timer->Counter))
            timer->Counter = 0xFFFF;
        else
            timer->Counter += (uint16_t)dt;
    }

    //32-битные таймеры
    for (uint16_t i = 0; i < Count32; i++)
    {
        SoftTimer32* timer = Timers32End - 1 - i;
        if (timer->Func == 0)
            continue;
        if (timer->Config.Freeze == 1)
            continue;
        uint32_t dt = dt_units[timer->Config.Units];
        //защита от переполнения
        if (dt > 0xFFFFFFFFUL - timer->Counter)
            timer->Counter = 0xFFFFFFFFUL;
        else
            timer->Counter += dt;
    }
}
