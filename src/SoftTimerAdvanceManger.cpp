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
//_Res - разрешение таймеров, 0 - 16 бит, 1 - 32 биты, 2 - 64 бита
//_Buff - буффер для пула таймеров, _BuffSize - размер буффера в байтах
uint16_t SoftTimerManager::Init(uint8_t* _Buff, uint32_t _BuffSize, STResolution _Res)
{
    memset(this,0,sizeof(SoftTimerManager));

    Resolution = _Res;
    if (_Buff == 0)
        return 0;
    //очищаем пул, в буфере может быть мусор
    memset(_Buff, 0, _BuffSize);
    switch (_Res)
    {
    case 0://16 бит
        Size = _BuffSize / sizeof(SoftTimer16);
        Timers16 = (SoftTimer16 *)_Buff;
        break;
    case 1://32 бит
        Size = _BuffSize / sizeof(SoftTimer32);
        Timers32 = (SoftTimer32*)_Buff;
        break;
    case 2://64 бит
        Size = _BuffSize / sizeof(SoftTimer64);
        Timers64 = (SoftTimer64*)_Buff;
        break;
    }
    return Size;
}
//работа таймера, передаётся текущее время, можно вызывать любую функцию
void SoftTimerManager::Update(uint64_t _Time, STUnits _Units)
{
    switch (_Units)
    {
    case STUnits::Microseconds:
        Update_us(_Time);
        break;
    case STUnits::Milliseconds:
        Update_ms(_Time);
        break;
    case STUnits::Seconds:
        Update_s(_Time);
        break;
    }
}
void SoftTimerManager::Update_us(uint64_t _uSeconds)
{
    uint32_t dt_us = TimeDelta(_uSeconds, STUnits::Microseconds);
    uint32_t dt_ms = DtDiv1000(dt_us, &RemUs);
    uint32_t dt_s = DtDiv1000(dt_ms, &RemMs);
    Working(dt_us, dt_ms, dt_s);
}
void SoftTimerManager::Update_ms(uint64_t _mSeconds)
{
    uint32_t dt_ms = TimeDelta(_mSeconds, STUnits::Milliseconds);
    uint32_t dt_s = DtDiv1000(dt_ms, &RemMs);
    Working((uint64_t)dt_ms * 1000, dt_ms, dt_s);
}
void SoftTimerManager::Update_s(uint64_t _Seconds)
{
    uint32_t dt_s = TimeDelta(_Seconds, STUnits::Seconds);
    uint32_t dt_ms = (dt_s > 0xFFFFFFFFUL / 1000) ? 0xFFFFFFFFUL : dt_s * 1000;
    Working((uint64_t)dt_s * 1000000, dt_ms, dt_s);
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
        switch (Resolution)
        {
        case 0://16 бит
            Tick16(dt_us, _Dt_ms, _Dt_s);
            break;
        case 1://32 бит
            Tick32(dt_us, _Dt_ms, _Dt_s);
            break;
        case 2://64 бит
            Tick64(dt_us, _Dt_ms, _Dt_s);
            break;
        }

        //Telemetry.Tick(dt);

    }

    //таймеров нет, в пуле нечего проверять
    if (Count == 0)
        return;

    //счётчик(номер таймера) выполнения теймеров, по одному за запуск
    Iterator++;
    if (Iterator >= Count)
        Iterator = 0;

    SoftTimerBase* timer_base =0;

    switch (Resolution)
    {
    case 0://16 бит
        timer_base = (SoftTimerBase*)&Timers16[Iterator];
        break;
    case 1://32 бит
        timer_base = (SoftTimerBase*)&Timers32[Iterator];
        break;
    case 2://64 бит
        timer_base = (SoftTimerBase*)&Timers64[Iterator];
        break;
    default:
        return;
    }

    //таймер не занят, нужно удалить если он в конце
    if (timer_base->Func == 0)
    {
        //таймер находится в конце?
        if ((Iterator + 1) == Count && Count != 0) 
        {
            Count--;
            //кто-то добавил таймер из прерывания, вернём обратно
            if (timer_base->Func != 0) 
                Count++;
        }
        return;
    }

    //таймер заморожен, пропускаем
    if (timer_base->Config.Freeze) 
        return;

    //проверка готовности таймера, если таймер не готов, выходим
    switch (Resolution)
    {
    case 0://16 бит
        if (Timers16[Iterator].Counter < Timers16[Iterator].Delay)
            return;
        break;
    case 1://32 бит
        if (Timers32[Iterator].Counter < Timers32[Iterator].Delay)
            return;
        break;
    case 2://64 бит
        if (Timers64[Iterator].Counter < Timers64[Iterator].Delay)
            return;
        break;
    }

    //запамянам функцию, для телеметрии
    Telemetry.LastFanc = (VoidFuncST)timer_base->Func;

    //обработка счётчика таймера
    if (timer_base->Config.Type == 1)
    {
        //режим отработки времени
        switch (timer_base->Config.StrictMode)
        {
        case 0://строгий режим
            if(Resolution==0)
                Timers16[Iterator].Counter -= Timers16[Iterator].Delay;
            else if (Resolution == 1)
                Timers32[Iterator].Counter -= Timers32[Iterator].Delay;
            else if (Resolution == 2)
                Timers64[Iterator].Counter -= Timers64[Iterator].Delay;
            break;
        case 1://пытается сохранить частоту, но допускает пропуски выполнеия
            if (Resolution == 0)
            {
                Timers16[Iterator].Counter -= Timers16[Iterator].Delay;
                if (Timers16[Iterator].Counter >= Timers16[Iterator].Delay)
                    Timers16[Iterator].Counter = 0; //таймер не успевает, пропуск
            }
            else if (Resolution == 1)
            {
                Timers32[Iterator].Counter -= Timers32[Iterator].Delay;
                if (Timers32[Iterator].Counter >= Timers32[Iterator].Delay)
                    Timers32[Iterator].Counter = 0; //таймер не успевает, пропуск
            }
            else if (Resolution == 2)
            {
                Timers64[Iterator].Counter -= Timers64[Iterator].Delay;
                if (Timers64[Iterator].Counter >= Timers64[Iterator].Delay)
                    Timers64[Iterator].Counter = 0; //таймер не успевает, пропуск
            }
            break;
        case 2://сбрасывает счётчик при каждом запуске
            if (Resolution == 0)
                Timers16[Iterator].Counter = 0;
            else if (Resolution == 1)
                Timers32[Iterator].Counter = 0;
            else if (Resolution == 2)
                Timers64[Iterator].Counter = 0;
            break;
        default://по умолчанию строгий
            if (Resolution == 0)
                Timers16[Iterator].Counter -= Timers16[Iterator].Delay;
            else if (Resolution == 1)
                Timers32[Iterator].Counter -= Timers32[Iterator].Delay;
            else if (Resolution == 2)
                Timers64[Iterator].Counter -= Timers64[Iterator].Delay;
            break;
        }
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
        uint64_t delay = 0;
        switch (Resolution)
        {
        case 0://16 бит
            delay = Timers16[Iterator].Delay;
            break;
        case 1://32 бит
            delay = Timers32[Iterator].Delay;
            break;
        case 2://64 бит
            delay = Timers64[Iterator].Delay;
            break;
        }
        if (Telemetry.LastFancTime >= timer_base->MaxWorkTime || delay >= 1000)
            timer_base->MaxWorkTime = Telemetry.LastFancTime;
        else if (timer_base->MaxWorkTime > 0)
            timer_base->MaxWorkTime--;
    }else
        timer_base->MaxWorkTime = Telemetry.LastFancTime;


    //удаляем объект таймера после запуска, если он не защищён от удаления
    if (!timer_base->Config.DeleteProtection)
    {
        timer_base->Func = 0;
        if ((Iterator + 1) == Count && Count != 0)
        {
            Count--;
            if (timer_base->Func != 0) //кто-то добавил таймер из прерывания, вернём обратно
                Count++;
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
void SoftTimerManager::Tick16(uint32_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s)
{
    for (int i = 0; i < Count; i++)
    {
        if (Timers16[i].Func == 0)
            continue;
        if (Timers16[i].Config.Freeze == 1)
            continue;
        uint32_t dt = 0;
        switch (Timers16[i].Config.Units)
        {
        case STUnits::Microseconds:
            dt = _Dt_us;
            break;
        case STUnits::Milliseconds:
            dt = _Dt_ms;
            break;
        case STUnits::Seconds:
            dt = _Dt_s;
            break;
        }
        //защита от переполнения
        if (dt > (uint16_t)(0xFFFF - Timers16[i].Counter))
            Timers16[i].Counter = 0xFFFF;
        else
            Timers16[i].Counter += (uint16_t)dt;
    }
}

void SoftTimerManager::Tick32(uint32_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s)
{
    for (int i = 0; i < Count; i++)
    {
        if (Timers32[i].Func == 0)
            continue;
        if (Timers32[i].Config.Freeze == 1)
            continue;
        uint32_t dt = 0;
        switch (Timers32[i].Config.Units)
        {
        case STUnits::Microseconds:
            dt = _Dt_us;
            break;
        case STUnits::Milliseconds:
            dt = _Dt_ms;
            break;
        case STUnits::Seconds:
            dt = _Dt_s;
            break;
        }
        //защита от переполнения
        if (dt > 0xFFFFFFFFUL - Timers32[i].Counter)
            Timers32[i].Counter = 0xFFFFFFFFUL;
        else
            Timers32[i].Counter += dt;
    }
}

void SoftTimerManager::Tick64(uint32_t _Dt_us, uint32_t _Dt_ms, uint32_t _Dt_s)
{
    for (int i = 0; i < Count; i++)
    {
        if (Timers64[i].Func == 0)
            continue;
        if (Timers64[i].Config.Freeze == 1)
            continue;
        uint32_t dt = 0;
        switch (Timers64[i].Config.Units)
        {
        case STUnits::Microseconds:
            dt = _Dt_us;
            break;
        case STUnits::Milliseconds:
            dt = _Dt_ms;
            break;
        case STUnits::Seconds:
            dt = _Dt_s;
            break;
        }
        //защита от переполнения
        if (dt > 0xFFFFFFFFFFFFFFFFULL - Timers64[i].Counter)
            Timers64[i].Counter = 0xFFFFFFFFFFFFFFFFULL;
        else
            Timers64[i].Counter += dt;
    }
}