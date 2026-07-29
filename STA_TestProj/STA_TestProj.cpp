// STA_TestProj.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <iostream>
#include "SoftTimerAdvance.h"
#include <chrono>


SoftTimerManager ST1;
STTask Task1;


void test_print()
{
    static int counter = 1;
    std::cout << counter << "\n";
    counter++;
}

void test_print2()
{
    std::cout << "hello!\n";
}

void TaskFunc(STTask* Task)
{
    static int i = 0;
    TaskBeginPP();
    for (i = 0; i < 10; i++)
    {
        std::cout << "hi!\n";
        TaskDelayPP(500);
    };
    TaskEndPP();
}

uint8_t t_buff[1024];
int main()
{
    ST1.Init(t_buff, sizeof(t_buff));

    auto now_start = std::chrono::system_clock::now();
    auto duration_start = now_start.time_since_epoch();

    // Превращаем прошедшее время в миллисекунды
    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration_start).count();

    std::cout << "Hello World!\n";

    ST1.AddTimer(test_print, 1, STUnits::Seconds);
    ST1.AddDelayCall(test_print2, 3000);
    ST1.AddDelayCall(test_print2, 10000 - 10);
    ST1.AddDelayCall(test_print2, 10000 + 10);

    Task1.Start((PrmFuncST)TaskFunc, &ST1);
    while (1)
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();

        // Превращаем прошедшее время в миллисекунды
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        millis -= start_time;

        ST1.Update(millis);
    };
}
