// emulated_system_calls.hpp

#ifndef _EMULATED_SYSTEM_CALLS_HPP_
#define _EMULATED_SYSTEM_CALLS_HPP_


#include <stdint.h>
#include "lightweight_semaphore.hpp"



#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------


//----------------------
// FreeRTOS
//----------------------
typedef int BaseType_t;
typedef unsigned UBaseType_t;
typedef unsigned TickType_t;

#define portMAX_DELAY    ( TickType_t ) 0xffffffffUL


//------------------
// from: projdefs.h
//------------------
/*
 * Defines the prototype to which task functions must conform.  Defined in this
 * file to ensure the type is known before portable.h is included.
 */
typedef void (* TaskFunction_t)( void * arg );

#define pdFALSE                                  ( ( BaseType_t ) 0 )
#define pdTRUE                                   ( ( BaseType_t ) 1 )
#define pdFALSE_SIGNED                           ( ( BaseType_t ) 0 )
#define pdTRUE_SIGNED                            ( ( BaseType_t ) 1 )
#define pdFALSE_UNSIGNED                         ( ( UBaseType_t ) 0 )
#define pdTRUE_UNSIGNED                          ( ( UBaseType_t ) 1 )

#define pdPASS                                   ( pdTRUE )
#define pdFAIL                                   ( pdFALSE )
#define errQUEUE_EMPTY                           ( ( BaseType_t ) 0 )
#define errQUEUE_FULL                            ( ( BaseType_t ) 0 )

/* FreeRTOS error definitions. */
#define errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY    ( -1 )
#define errQUEUE_BLOCKED                         ( -4 )
#define errQUEUE_YIELD                           ( -5 )



//--------------
// from: task.h
//--------------
/**
 * task. h
 * Type by which tasks are referenced.  For example, a call to xTaskCreate
 * returns (via a pointer parameter) an TaskHandle_t variable that can then
 * be used as a parameter to vTaskDelete to delete the task.
 */
typedef struct tskTaskControlBlock  /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    tskTaskControlBlock(unsigned intialSemephoreCount, std::string semaphoreName) :
        semaphore(intialSemephoreCount, semaphoreName)
    { }

    LightweightSemaphore semaphore;
    UBaseType_t indexToNotify;
} tskTCB;

typedef struct tskTaskControlBlock         * TaskHandle_t;
typedef const struct tskTaskControlBlock   * ConstTaskHandle_t;


/*
 * Defines the prototype to which the application task hook function must
 * conform.
 */
typedef BaseType_t (* TaskHookFunction_t)( void * arg );

/* Task states returned by eTaskGetState. */
typedef enum
{
    eRunning = 0, /* A task is querying the state of itself, so must be running. */
    eReady,       /* The task being queried is in a ready or pending ready list. */
    eBlocked,     /* The task being queried is in the Blocked state. */
    eSuspended,   /* The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
    eDeleted,     /* The task being queried has been deleted, but its TCB has not yet been freed. */
    eInvalid      /* Used as an 'invalid state' value. */
} eTaskState;

/* Actions that can be performed when vTaskNotify() is called. */
typedef enum
{
    eNoAction = 0,            /* Notify the task without updating its notify value. */
    eSetBits,                 /* Set bits in the task's notification value. */
    eIncrement,               /* Increment the task's notification value. */
    eSetValueWithOverwrite,   /* Set the task's notification value to a specific value even if the previous value has not yet been read by the task. */
    eSetValueWithoutOverwrite /* Set the task's notification value if the previous value has been read by the task. */
} eNotifyAction;

//...

extern void setTaskControlBlock (TaskHandle_t taskControlBlock);
extern TaskHandle_t getThreadTaskHandle();

extern uint32_t ulTaskNotifyTakeIndexed( UBaseType_t uxIndexToWaitOn, BaseType_t xClearCountOnExit, TickType_t xTicksToWait );
extern BaseType_t xTaskNotifyGiveIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify );
extern BaseType_t xTaskNotifyIndexed( TaskHandle_t xTaskToNotify, UBaseType_t uxIndexToNotify, uint32_t ulValue, eNotifyAction eAction );


//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif


#endif // _EMULATED_SYSTEM_CALLS_HPP_
