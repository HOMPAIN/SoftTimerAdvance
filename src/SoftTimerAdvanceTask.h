//потоки на базе таймеров
#pragma once

#include "SoftTimerAdvanceManger.h"
#include "SoftTimerAdvanceInterface.h"


typedef class STTask STTask;
class STTask
{
public:
    uint16_t Line;      //текущая страка таска
    SDelay *Timer;   //таймер в котором крутится таск
    struct STTask *TaskParent;  //родительский таск для текущего, 0 если его нет
    struct STTask *TaskChild;   //дочерний таск вызываемый из текущего, 0 если нет. Необходим для корректной работы команды TaskAbort

    STTask()
    {
        Line = 0;
        Timer = 0;
        TaskParent = 0;
        TaskChild = 0;
    }

    //запустить задачу
    void Start(PrmFuncST _Func, SoftTimerManager* _STManager, STTask *_ParentTask = 0);
    //запустить дочерную задачу из текущей
    void StartChild(PrmFuncST _Func, STTask *_ChildTask);
    //завершить задачу
    void Abort();
    uint16_t IsRunning();
private:
    SoftTimerManager* TimerManager;//то куда будет добавляться таймер
};

//макросы, добавляются внутрь заданий
// Begin в начале функции таска
#define TaskBegin();    switch(Task->Line){case 0: return;case 1: Task->Line=0;
//задержка миллисекунды
#define TaskDelay(X);   Task->Timer->Reset(X);Task->Line=(__LINE__);return;case (__LINE__): Task->Line=0;
// END в конце функции таска
#define TaskEnd();      }Task->Timer->Delete();
