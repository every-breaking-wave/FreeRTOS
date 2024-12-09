/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky version.
 *
 * This file only contains the source code that is specific to the basic demo.
 * Generic functions, such FreeRTOS hook functions, are defined in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, one software timer, and two tasks.  It then
 * starts the scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
 * the value 100 to the queue every 200 (simulated) milliseconds.
 *
 * The Queue Send Software Timer:
 * The timer is an auto-reload timer with a period of two (simulated) seconds.
 * Its callback function writes the value 200 to the queue.  The callback
 * function is implemented by prvQueueSendTimerCallback() within this file.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
 * When data is received, the task checks the value of the data, then outputs a
 * message to indicate if the data came from the queue send task or the queue
 * send software timer.
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS pdMS_TO_TICKS(200UL)
#define mainTIMER_SEND_FREQUENCY_MS pdMS_TO_TICKS(2000UL)

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH (2)

/* The values sent to the queue receive task from the queue send task and the
 * queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK (100UL)
#define mainVALUE_SENT_FROM_TIMER (200UL)

#define mainGET_TASK_STATUS(bitmap, i) (((bitmap) >> (i)) & 1)
#define mainSET_TASK_STATUS(bitmap, i, bit) \
    do                                      \
    {                                       \
        if (bit)                            \
            (bitmap) |= (1 << (i));         \
        else                                \
            (bitmap) &= ~(1 << (i));        \
    } while (0)

/*-----------------------------------------------------------*/

static BaseType_t durationToLoop(TickType_t duration)
{
    return duration * (1000000000 / 1295) ;
}
/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask(void *pvParameters);
static void prvQueueSendTask(void *pvParameters);

/*
 * The callback function executed when the software timer expires.
 */
static void prvQueueSendTimerCallback(TimerHandle_t xTimerHandle);

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;

static BaseType_t isTimeout = pdFALSE;

/* Test functions */
unsigned long ulTaskNumber[configEXPECTED_NO_RUNNING_TASKS];

static void T1(void *pvParameters);
static void T2(void *pvParameters);
static void T3(void *pvParameters);
static void T4(void *pvParameters);

static TaskHandle_t xT[4];

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_blinky(void)
{
    const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

    /* Create the queue. */
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

    pvParameter_t param[4];
    for(int i = 0; i < 4; i++)
        param[i].uxWeight = 0;
    param[0].uxDeadLine = 100;
    param[1].uxDeadLine = 1400;
    param[2].uxDeadLine = 1000;
    param[3].uxDeadLine = 2000;
    
    if (xQueue != NULL) {
        // Create an eternal task evoked periodically for creating new tasks
        xTaskCreate(T1, "task1", configMINIMAL_STACK_SIZE, &param[0], configMAX_PRIORITIES-1, &(xT[0]));
        xTaskCreate(T2, "task2", configMINIMAL_STACK_SIZE, &param[1], configMAX_PRIORITIES-2, &(xT[1]));
        xTaskCreate(T3, "task3", configMINIMAL_STACK_SIZE, &param[2], configMAX_PRIORITIES-3, &(xT[2]));
        xTaskCreate(T4, "task4", configMINIMAL_STACK_SIZE, &param[3], configMAX_PRIORITIES-4, &(xT[3]));
    }

    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks	to be created.  See the memory management section on the
     * FreeRTOS web site for more details.  NOTE: This demo uses static allocation
     * for the idle and timer tasks so this line should never execute. */
    for (;;)
    {
    }
}

static void T1(void *pvParameters)
{
    TickType_t previousTime = xTaskGetTickCount();
    TickType_t runtime = 100, period = 500;
    int iterNum = 0;
    BaseType_t loopCount = durationToLoop(runtime);
    for(;;) {
        TickType_t startTime = xTaskGetTickCount();
        printf("[iter:%d] task1 in starts at %d\n", iterNum, startTime);
        for(int i = 0; i < loopCount; i++);
        printf("[iter:%d] task1 ends at %d\n", iterNum, xTaskGetTickCount());
        xTaskDelayUntil(&previousTime, period);
    }
}

static void T2(void *pvParameters)
{
    TickType_t previousTime = xTaskGetTickCount();
    TickType_t runtime = 1000, period = 1500;
    int iterNum = 0;
    BaseType_t loopCount = durationToLoop(runtime);
    for(;;) {
        TickType_t startTime = xTaskGetTickCount();
        printf("[iter:%d] task2 in starts at %d\n", iterNum, startTime);
        for(int i = 0; i < loopCount; i++);
        printf("[iter:%d] task2 ends at %d\n", iterNum, xTaskGetTickCount());
        iterNum++;
        xTaskDelayUntil(&previousTime, period);
    }
}


static void T3(void *pvParameters)
{
    TickType_t previousTime = xTaskGetTickCount();
    TickType_t runtime = 300, period = 2000;
    int iterNum = 0;
    BaseType_t loopCount = durationToLoop(runtime);
    for(;;) {
        TickType_t startTime = xTaskGetTickCount();
        printf("[iter:%d] task3 in starts at %d\n", iterNum, startTime);
        for(int i = 0; i < loopCount; i++);
        printf("[iter:%d] task3 ends at %d\n", iterNum, xTaskGetTickCount());
        iterNum++;
        xTaskDelayUntil(&previousTime, period);
    }
}

static void T4(void *pvParameters)
{
    TickType_t previousTime = xTaskGetTickCount();
    TickType_t runtime = 500, period = 4000;
    int iterNum = 0;
    BaseType_t loopCount = durationToLoop(runtime);
    for(;;) {
        TickType_t startTime = xTaskGetTickCount();
        printf("[iter:%d] task4 in starts at %d\n", iterNum, startTime);
        for(int i = 0; i < loopCount; i++);
        printf("[iter:%d] task4 ends at %d\n", iterNum, xTaskGetTickCount());
        iterNum++;
        xTaskDelayUntil(&previousTime, period);
    }
}
