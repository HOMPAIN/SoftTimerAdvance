#include "SoftTimerAdvanceTask.h"

//запуск задания
void STTask::Start(PrmFuncST _Func, SoftTimerManager* _STManager, STTask *_ParentTask)
{
    if(Line!=0)//таск уже запущен
    {
        //ErrorBrakepoint();
    }
    Line=1;
    TimerManager = _STManager;
    TaskParent = _ParentTask;

    Timer = (SDelay*)TimerManager->AddDelayCall(_Func, this, 0);
    Timer->SetDeleteProtection(1);
}
//запустить дочерную задачу из текущей
void STTask::StartChild(PrmFuncST _Func, STTask *_ChildTask)
{
    TaskChild = _ChildTask;

    _ChildTask->Start(_Func, TimerManager, this);

}
//завершить таск если он работает
void STTask::Abort()
{
   if (Line!=0)
    {
        //останавливаем таймер
        if (Timer != 0)
        {
            Timer->Delete();
        }

        //сбрасываем таск
        Line = 0;

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
    return Line!=0;
}
