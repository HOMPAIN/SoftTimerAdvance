//потоки на базе таймеров
#pragma once

#include "SoftTimerAdvanceManager.h"
#include "SoftTimerAdvanceInterface.h"

//результат запуска задачи
enum class STTaskResult : uint16_t {
    Ok = 0,                 //задача запущена
    AlreadyRunning = 1,     //задача уже работает, повторный запуск возможен после её завершения или Abort
    NoTimers = 2,           //в пуле менеджера нет места для таймера задачи
    BadParams = 3           //не задана функция, менеджер таймеров или дочерняя задача
};

typedef class STTask STTask;
class STTask
{
public:
    uint16_t Line;      //текущая страка таска
    uint16_t Running;   //1 - задача запущена и ещё не завершилась
    SDelay *Timer;   //таймер в котором крутится таск, 0 если задача не запущена
    struct STTask *TaskParent;  //родительский таск для текущего, 0 если его нет
    struct STTask *TaskChild;   //дочерний таск вызываемый из текущего, 0 если нет. Необходим для корректной работы команды TaskAbort

    STTask()
    {
        Line = 0;
        Running = 0;
        Timer = 0;
        TaskParent = 0;
        TaskChild = 0;
        TimerManager = 0;
    }

    //запустить задачу, возвращает STTaskResult::Ok или код ошибки
    STTaskResult Start(PrmFuncST _Func, SoftTimerManager* _STManager, STTask *_ParentTask = 0);
    //запустить дочерную задачу из текущей, возвращает STTaskResult::Ok или код ошибки
    STTaskResult StartChild(PrmFuncST _Func, STTask *_ChildTask);
    //завершить задачу
    void Abort();
    //1 - задача запущена и ещё не завершилась (в том числе пока выполняется её функция)
    uint16_t IsRunning();

    //пауза задачи, используется макросом TaskDelay
    void Delay(uint32_t _Time, uint16_t _Line);
    //завершение задачи, используется макросом TaskEnd
    void End();
private:
    SoftTimerManager* TimerManager;//то куда будет добавляться таймер
};

//макросы, добавляются внутрь заданий, после каждого ставится точка с запятой
// Begin в начале функции таска
#define TaskBegin()     switch(Task->Line){case 0: return;case 1: Task->Line=0
//задержка миллисекунды, обёрнута в do-while, чтобы работать как один оператор (например под if без скобок)
#define TaskDelay(X)    do{Task->Delay((X),(__LINE__));return;case (__LINE__): Task->Line=0;}while(0)
// END в конце функции таска
#define TaskEnd()       }Task->End()
