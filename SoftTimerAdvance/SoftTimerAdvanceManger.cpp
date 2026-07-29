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
uint16_t SoftTimerManager::Init(uint16_t _Res, uint8_t* _Buff, uint32_t _BuffSize)
{
    memset(this,0,sizeof(SoftTimerManager));

    Resolution = _Res;
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
void SoftTimerManager::Update_us(uint64_t _uSeconds)
{
    uSeconds = _uSeconds;
    mSeconds = uSeconds / 1000;
    Seconds = mSeconds / 1000;
    Working();
}
void SoftTimerManager::Update_ms(uint64_t _mSeconds)
{
    mSeconds = _mSeconds;
    uSeconds = mSeconds * 1000;
    Seconds = mSeconds / 1000;
    Working();
}
void SoftTimerManager::Update_s(uint64_t _Seconds)
{
    Seconds = _Seconds;
    mSeconds = Seconds * 1000;
    uSeconds = mSeconds * 1000;
    Working();
}

//цикл прохода по таймерам
void SoftTimerManager::Working()
{

    mSeconds = uSeconds/1000;
    Seconds = mSeconds/1000;

    //первый запуск, обнуляем время
    if (uSecondsLast == 0)
    {
        uSecondsLast = uSeconds;
        mSecondsLast = mSeconds;
        SecondsLast = Seconds;
        return;
    }
    //счётчик таймеров
    uint16_t dt_us = uSeconds - uSecondsLast;
    uint16_t dt_ms = mSeconds - mSecondsLast;
    uint16_t dt_s = Seconds - SecondsLast;
    uSecondsLast = uSeconds;
    mSecondsLast = mSeconds;
    SecondsLast = Seconds;
    if(dt_us !=0)
    {
        switch (Resolution)
        {
        case 0://16 бит
            Tick16(dt_us, dt_ms, dt_s);
            break;
        case 1://32 бит
            Tick32(dt_us, dt_ms, dt_s);
            break;
        case 2://64 бит
            Tick64(dt_us, dt_ms, dt_s);
            break;
        }

        //Telemetry.Tick(dt);

    }

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
            else if (Resolution == 2)
            {
                Timers32[Iterator].Counter -= Timers32[Iterator].Delay;
                if (Timers32[Iterator].Counter >= Timers32[Iterator].Delay)
                    Timers32[Iterator].Counter = 0; //таймер не успевает, пропуск
            }
            else if (Resolution == 3)
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
void SoftTimerManager::Tick16(uint16_t _Dt_us, uint16_t _Dt_ms, uint16_t _Dt_s)
{
    for (int i = 0; i < Count; i++)
    {
        if (Timers16[i].Func == 0)
            continue;
        if (Timers16[i].Config.Freeze == 1)
            continue;
        uint16_t counter_old = Timers16[i].Counter;
        switch (Timers16[i].Config.Units)
        {
        case STUnits::Microseconds:
            Timers16[i].Counter += _Dt_us;
            break;
        case STUnits::Milliseconds:
            Timers16[i].Counter += _Dt_ms;
            break;
        case STUnits::Seconds:
            Timers16[i].Counter += _Dt_s;
            break;
        }
        //защита от переполнения
        if (counter_old > Timers16[i].Counter)
            Timers16[i].Counter = 0xFFFF;
    }
}

void SoftTimerManager::Tick32(uint16_t _Dt_us, uint16_t _Dt_ms, uint16_t _Dt_s)
{
    for (int i = 0; i < Count; i++)
    {
        if (Timers32[i].Func == 0)
            continue;
        if (Timers32[i].Config.Freeze == 1)
            continue;
        uint32_t counter_old = Timers32[i].Counter;
        switch (Timers32[i].Config.Units)
        {
        case STUnits::Microseconds:
            Timers32[i].Counter += _Dt_us;
            break;
        case STUnits::Milliseconds:
            Timers32[i].Counter += _Dt_ms;
            break;
        case STUnits::Seconds:
            Timers32[i].Counter += _Dt_s;
            break;
        }
        //защита от переполнения
        if (counter_old > Timers32[i].Counter)
            Timers32[i].Counter = 0xFFFF;
    }
}

void SoftTimerManager::Tick64(uint16_t _Dt_us, uint16_t _Dt_ms, uint16_t _Dt_s)
{
    for (int i = 0; i < Count; i++)
    {
        if (Timers64[i].Func == 0)
            continue;
        if (Timers64[i].Config.Freeze == 1)
            continue;
        uint64_t counter_old = Timers64[i].Counter;
        switch (Timers64[i].Config.Units)
        {
        case STUnits::Microseconds:
            Timers64[i].Counter += _Dt_us;
            break;
        case STUnits::Milliseconds:
            Timers64[i].Counter += _Dt_ms;
            break;
        case STUnits::Seconds:
            Timers64[i].Counter += _Dt_s;
            break;
        }
        //защита от переполнения
        if (counter_old > Timers64[i].Counter)
            Timers64[i].Counter = 0xFFFF;
    }
}