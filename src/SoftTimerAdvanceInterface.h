//тут описаны классы реализующие интерфесы взаимодействия с таймерами
//время в 16-битном таймере ограничено 65535: если новое значение не помещается,
//таймер переходит на более крупные единицы измерения с округлением (например 90500 мс -> 91 сек)
//
//Работа из прерываний:
//обычные методы вызывать из прерываний нельзя, для этого есть методы с окончанием FromISR.
//Такой метод сам таймер не меняет, а оставляет заявку, которую менеджер выполнит при ближайшем вызове Update.
//Если до Update пришло несколько заявок, выполняются все (из одинаковых - последняя).
//Добавлять и удалять таймеры, а также менять период из прерываний нельзя.
//Одним таймером нельзя управлять из двух прерываний, если одно из них может прервать другое.
#pragma once
#include <stdint.h>                 //для определения типов uint16_t...
#include "SoftTimerAdvanceStructures.h"

//базовый интерфейс
class STBase :protected SoftTimerBase
{
public:
    //вызвать как можно быстрее
    void ForceCall();
    //возвраает время до следующего запуска
    uint32_t GetDelay(STUnits _Units = STUnits::Milliseconds);
    //удалить таймер
    void Delete();
    //время работы функции таймера в микросекундах (не больше 65535), нужна настроенная телеметрия менеджера
    uint16_t GetWorkTime();

    //вызвать как можно быстрее, для вызова из прерывания
    void ForceCallFromISR() { RequestSet(ST_REQ_COUNTER | ST_REQ_FORCE, ST_REQ_COUNTER | ST_REQ_FORCE); }
};
//интерфейс таймера
class STimer :public STBase
{
public:
    //задать новый период работы таймера, _Units задаёт размерность времени, по умолчанию миллисек
    void SetPeriod(uint32_t _Period, STUnits _Units = STUnits::Milliseconds);
    //получить текущий период, _Units задаёт размерность времени
    uint32_t GetPeriod(STUnits _Units = STUnits::Milliseconds);
    //сбросить таймер, будет вызван через период
    void Reset();
    //заморозить таймер
    void Freeze();
    //разморозить таймер
    void UnFreeze();
    //1 - таймер заморожен, 0 - нет
    uint16_t GetFreezeStatus();
    //задать режим работы таймера: что делать с вызовами, которые не успели выполниться вовремя (см. STMode)
    //по умолчанию STMode::Skip - пропущенные вызовы не копятся
    void SetMode(STMode _Mode);
    //получить режим работы таймера
    STMode GetMode();

    //сбросить таймер, для вызова из прерывания
    void ResetFromISR() { RequestSet(ST_REQ_COUNTER | ST_REQ_FORCE, ST_REQ_COUNTER); }
    //заморозить таймер, для вызова из прерывания
    void FreezeFromISR() { RequestSet(ST_REQ_FREEZE | ST_REQ_FROZEN, ST_REQ_FREEZE | ST_REQ_FROZEN); }
    //разморозить таймер, для вызова из прерывания
    void UnFreezeFromISR() { RequestSet(ST_REQ_FREEZE | ST_REQ_FROZEN, ST_REQ_FREEZE); }
};
//интерфейс отложенный запуск
class SDelay :public STBase
{
public:
    //перезагрузить с новой задержкой
    void Reset(uint32_t _Delay, STUnits _Units = STUnits::Milliseconds);
    //установить защиту от удаления
    void SetDeleteProtection(uint16_t _Enable);
    //прочитать, установлина ли защита от удаления
    uint16_t GetDeleteProtection();
    //0 - работает, 1 - остановлен
    uint16_t GetStatus();

    //перезагрузить с прежней задержкой, для вызова из прерывания
    //отложенный запуск должен быть защищён от удаления (SetDeleteProtection), иначе после срабатывания его уже нет
    void ResetFromISR() { RequestSet(0x0F, ST_REQ_COUNTER | ST_REQ_FREEZE); }
};
//интерфейс таймаута
class STimeout :public STBase
{
public:
    //сбросить таймаут
    void Reset();
    //остановить отсчёт таймаута без удаления, повторный запуск через Reset()
    void Stop();
    //установить новое время таймаута
    void SetTimeout(uint32_t _Timeout, STUnits _Units = STUnits::Milliseconds);
    //получить текущее время таймаута
    uint32_t GetTimeout(STUnits _Units = STUnits::Milliseconds);

    //сбросить таймаут, для вызова из прерывания
    void ResetFromISR() { RequestSet(0x0F, ST_REQ_COUNTER | ST_REQ_FREEZE); }
    //остановить отсчёт таймаута, для вызова из прерывания
    void StopFromISR() { RequestSet(ST_REQ_FREEZE | ST_REQ_FROZEN, ST_REQ_FREEZE | ST_REQ_FROZEN); }
};
