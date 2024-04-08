// emulated_system_calls.cpp


#include "emulated_system_calls.hpp" 

static const unsigned char QUEUE_SIZE = 4;

// #include <semaphore> // C++20
//
// C++20
//static std::counting_semaphore<QUEUE_SIZE> producerSemaphore{QUEUE_SIZE}, consumerSemaphore{0};

// #include <chrono>
#include <iostream>
#include <thread>
using namespace std;


thread_local TaskHandle_t threadTaskControlBlock = nullptr;


void setTaskControlBlock (TaskHandle_t taskControlBlock) {
    // 'threadTaskControlBlock' is thread_local.
    threadTaskControlBlock = taskControlBlock;
    //threadTaskControlBlock.indexToNotify = taskControlBlock.indexToNotify;
}

TaskHandle_t getThreadTaskHandle() {
    // 'threadTaskControlBlock' is thread_local.
    return threadTaskControlBlock;
}



//--------------
// from: task.h
//--------------
uint32_t ulTaskNotifyTakeIndexed( UBaseType_t uxIndexToWaitOn, BaseType_t xClearCountOnExit, TickType_t xTicksToWait )
{
    TaskHandle_t threadTaskHandle = getThreadTaskHandle();
    cout << "ulTaskNotifyTakeIndexed(...): " << threadTaskHandle->semaphore.debug_str() << endl;
    return threadTaskHandle->semaphore.take();
}



BaseType_t xTaskNotifyGiveIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify )
{
    cout << "xTaskNotifyGiveIndexed(...): " << xTaskToNotify->semaphore.debug_str() << endl;
    xTaskToNotify->semaphore.give();
    return pdPASS;
}



BaseType_t xTaskNotifyIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify, uint32_t ulValue, eNotifyAction eAction )
{
    cout << "xTaskNotifyIndexed(...): " << xTaskToNotify->semaphore.debug_str() << endl;
    xTaskToNotify->semaphore.give(ulValue);
    return pdPASS;
}
