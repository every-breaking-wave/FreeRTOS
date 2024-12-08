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
#include "queue.h"
#include "timers.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue.  The times are converted from
 * milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS pdMS_TO_TICKS(200UL)
#define mainTIMER_SEND_FREQUENCY_MS pdMS_TO_TICKS(2000UL)

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH (10)

/* The values sent to the queue receive task from the queue send task and the
 * queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK (100UL)
#define mainVALUE_SENT_FROM_TIMER (200UL)

static uint64_t xTaskStatusBitMap = NULL;

#define mainGET_TASK_STATUS(bitmap, i) (((bitmap) >> (i)) & 1)

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
//  */
// static void prvQueueReceiveTask( void * pvParameters );
// static void prvQueueSendTask( void * pvParameters );

/*-----------------------------------------------------------*/

typedef struct tskParam
{
    TickType_t periodExecute;
    TickType_t priority;
    TickType_t deadline;
} tskParam_t;

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* The timer used to create tasks periodically. */
static TimerHandle_t xTimer = NULL;

/* Timer callback used to create new tasks */
static void taskCreate(void *pvParameters);

static char bitmap = 0;

/* Test functions */
unsigned long ulTaskNumber[configEXPECTED_NO_RUNNING_TASKS];

TaskHandle_t xTskCreate, xT1, xT2, xT3, xT4;

// static List_t tskSets
static int stage = 0;

const unsigned long dT1 = 100;
const unsigned long dT2 = 70;
const unsigned long dT3 = 50;
const unsigned long dT4 = 400;

static void T1(void *pvParameters);
static void T2(void *pvParameters);
static void T3(void *pvParameters);
static void T4(void *pvParameters);

/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/

void main_test(void)
{

    /* Create the queue. */
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

    if (xQueue != NULL)
    {
        printf("starting test scheduler\n");
        xTaskCreate(T1, (signed char *)"T1", configMINIMAL_STACK_SIZE, (void *)&dT1, 1, &xT1);
        xTaskCreate(T2, (signed char *)"T2", configMINIMAL_STACK_SIZE, (void *)&dT2, 1, &xT2);
        xTaskCreate(T3, (signed char *)"T3", configMINIMAL_STACK_SIZE, (void *)&dT3, 1, &xT3);
        vTaskStartScheduler();
    }

    for (;;)
    {
    }
}

/** */
static void taskCreate(void *pvParameters)
{
    printf("creating task...");
    for (;;)
    {

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // xTaskCreate(T4, (signed char *)"T4", 1000, &wT4, 1, &xT4);
}

static void T1(void *pvParameters)
{
    printf("T1 Executing\n");
    // while (1)
    // {
    //     // i = 0xFFFFFFFE + 0xA;
    //     // printf("%x\n", i);

    //     // for (int i = 0; i < 1000000; i++);
    //     // vTaskDelay(pdMS_TO_TICKS(100));
    // }
}

static void T2(void *pvParameters)
{
    printf("T2 executing\n");
    // while (1)
    // {

    //     // for (int i = 0; i < 1000000; i++);
    //     // vTaskDelay(pdMS_TO_TICKS(100));
    // }
}

static void T3(void *pvParameters)
{
    printf("T3 Executing\n");
    // while (1)
    // {
    //     printf("T3 Executing\n");
    //     // for (int i = 0; i < 1000000; i++);
    //     // vTaskDelay(pdMS_TO_TICKS(100));
    // }
}

static void T4(void *pvParameters)
{
    while (1)
    {
        printf("T4 executing\n");
        // for (int i = 0; i < 1000000; i++);
        // vTaskDelay(pdMS_TO_TICKS(100));
    }
}
