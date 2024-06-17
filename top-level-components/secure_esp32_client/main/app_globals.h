/*
app_globals.h
*/
#ifndef _APP_GLOBALS_H_
#define _APP_GLOBALS_H_

#include "freertos/task.h"


// #ifdef APP_DEBUG
// # define DEBUG_TOUCH_PAD_NUMBER 1
// #endif


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    TaskHandle_t taskToNotify;
    UBaseType_t indexToNotify;
} globalTaskNotifyParams;


#ifdef __cplusplus
}
#endif


#endif // _APP_GLOBALS_H_
