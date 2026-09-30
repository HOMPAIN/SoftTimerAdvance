// Тестовый проект для отладки в Windows (Visual Studio Community)
#include <iostream>
#include "../../src/SoftTimerAdvance.h"
#include <chrono>
long micros();
long millis();



STTask Task1;
STimeout* Timeout1=0;



void test_print()
{
    static int counter = 1;
    std::cout << counter << "\n";
    counter++;
    Timeout1->Reset();
}

void test_print2()
{
    std::cout << "hello!\n";
}

void TaskFunc(STTask* Task)
{
    static int i = 0;
    TaskBegin();
    for (i = 0; i < 10; i++)
    {
        std::cout << "hi!\n";
        TaskDelay(500);
    };
    TaskEnd();
}


uint8_t t_buff[1024];
SoftTimerManager ST1;

void Setup()
{
    ST1.Init(t_buff, sizeof(t_buff));

    ST1.AddTimer(test_print, 3, STUnits::Seconds);
    /*ST1.AddDelayCall(test_print2, 3000);
    ST1.AddDelayCall(test_print2, 10000 - 10);
    ST1.AddDelayCall(test_print2, 10000 + 10);*/
    Timeout1 = (STimeout*)ST1.AddTimeout(test_print2,2500);

    //Task1.Start((PrmFuncST)TaskFunc, &ST1);
}
void Loop()
{
    ST1.Update(millis());
}


//------------------------------------------------------------------------------------------------------
//вспромогательные переменный для имитации функций времени и среды ардуино
long long start_time;//начальное время, микросекунды
long micros()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    // Превращаем прошедшее время в миллисекунды
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    micros -= start_time;

    return micros;
}

long millis()
{
    return micros() / 1000;
}

int main()
{
    auto now_start = std::chrono::system_clock::now();
    auto duration_start = now_start.time_since_epoch();

    // Превращаем прошедшее время в миллисекунды
    start_time = std::chrono::duration_cast<std::chrono::microseconds>(duration_start).count();

    Setup();

    while (1)
    {
        Loop();
    };
}
