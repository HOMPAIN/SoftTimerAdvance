#include "SoftTimerAdvanceTask.h"

//запуск задания
STTaskResult STTask::Start(PrmFuncST _Func, SoftTimerManager* _STManager, STTask *_ParentTask)
{
    if (_Func == 0 || _STManager == 0)
        return STTaskResult::BadParams;
    if (Running)//таск уже запущен
        return STTaskResult::AlreadyRunning;

    //32 бита, чтобы TaskDelay не был ограничен 65535 мс
    SDelay* timer = _STManager->AddDelayCall(_Func, this, 0, STUnits::Milliseconds, STResolution::Bits32);
    if (timer == 0)//в пуле нет места
        return STTaskResult::NoTimers;
    timer->SetDeleteProtection(1);

    Timer = timer;
    TimerManager = _STManager;
    TaskParent = _ParentTask;
    TaskChild = 0;
    Line = 1;
    Running = 1;
    return STTaskResult::Ok;
}
//запустить дочерную задачу из текущей
STTaskResult STTask::StartChild(PrmFuncST _Func, STTask *_ChildTask)
{
    if (_ChildTask == 0 || _ChildTask == this || TimerManager == 0)
        return STTaskResult::BadParams;

    STTaskResult res = _ChildTask->Start(_Func, TimerManager, this);
    if (res == STTaskResult::Ok)
        TaskChild = _ChildTask;
    return res;
}
//завершить таск если он работает
void STTask::Abort()
{
    if (Running)
    {
        //останавливаем таймер и сбрасываем таск
        End();

        //останавливаем дочерние таски, если они были
        if(TaskChild!=0)
        {
            TaskChild->Abort();
            TaskChild = 0;
        }
    }
}
//запущен ли таск
uint16_t STTask::IsRunning()
{
    return Running;
}
//пауза задачи, используется макросом TaskDelay
void STTask::Delay(uint32_t _Time, uint16_t _Line)
{
    //задача прервана через Abort из собственной функции, продолжать её не нужно
    if (!Running || Timer == 0)
        return;
    Timer->Reset(_Time);
    Line = _Line;
}
//завершение задачи, используется макросом TaskEnd
void STTask::End()
{
    if (Timer != 0)
        Timer->Delete();
    Timer = 0;
    Line = 0;
    Running = 0;
}
