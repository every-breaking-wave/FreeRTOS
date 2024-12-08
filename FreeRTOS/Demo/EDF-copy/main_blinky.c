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

TaskHandle_t xT1;
TaskHandle_t xT2, xT3, xT4;
TaskHandle_t xT0;



static char taskStatusMap = 0; // bit for task == 0 indicates task not running.

static void T1(void *pvParameters);
static void T2(void *pvParameters);
static void T3(void *pvParameters);
static void T4(void *pvParameters);

const unsigned long dT1 = 100;
const unsigned long dT2 = 70;
const unsigned long dT3 = 50;
const unsigned long dT4 = 400;

static void taskBatchCreate(void* pvParameters);

/* This defines priority of the 4 tasks in 4 iteration, that is, in the 1st iteration, the 4 tasks have 1,1,1,1 as deadline respectively */
const int priority_list[4][4] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};

const int hard_deadline_list[4][4] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};

const int soft_deadline_list[4][4] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};

const int weight_list[4][4] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};

const int runtime_list_tick[4][4] = {{20, 20, 20, 20}, {20, 20, 20, 20}, {20, 20, 20, 20}, {20, 20, 20, 20}};

/* This list defines whether a task should start after an assigned timrout from the start of a new iteration and the timeout value, e.g. 0 denotes no delay */
const int delay_list[4][4] = {{20, 0, 10, 30}, {20, 0, 10, 30}, {20, 0, 10, 30}, {20, 0, 10, 30}};

// TODO:
/* Maybe we alse need a list defining time of execution of the tasks? */

static int iter_num = 0;
/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_blinky(void)
{
    const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

    /* Create the queue. */
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

    if (xQueue != NULL) {
        // Create an eternal task evoked periodically for creating new tasks
        xTaskCreate(taskBatchCreate, "task-create", configMINIMAL_STACK_SIZE, 0, configMAX_PRIORITIES-1, &xT0);
    }

    // xTimer = xTimerCreate("timer", pdMS_TO_TICKS(5000), pdTRUE, NULL, timerCallback);
    // xTimerStart(xTimer, 0);
    /* Do not create tasks in software interrupt callback! Too long interrupt handle incurs context error! */

    vTaskStartScheduler();
    // if (xQueue != NULL)
    // {
    //     /* Start the two tasks as described in the comments at the top of this
    //      * file. */
    //     // xTaskCreate( prvQueueReceiveTask,             /* The function that implements the task. */
    //     //              "Rx",                            /* The text name assigned to the task - for debug only as it is not used by the kernel. */
    //     //              configMINIMAL_STACK_SIZE,        /* The size of the stack to allocate to the task. */
    //     //              NULL,                            /* The parameter passed to the task - not used in this simple case. */
    //     //              mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority assigned to the task. */
    //     //              NULL );                          /* The task handle is not required, so NULL is passed. */

    //     // xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

    //     // /* Create the software timer, but don't start it yet. */
    //     // xTimer = xTimerCreate( "Timer",                     /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
    //     //                        xTimerPeriod,                /* The period of the software timer in ticks. */
    //     //                        pdTRUE,                      /* xAutoReload is set to pdTRUE, so this is an auto-reload timer. */
    //     //                        NULL,                        /* The timer's ID is not used. */
    //     //                        prvQueueSendTimerCallback ); /* The function executed when the timer expires. */

    //     // xTimerStart( xTimer, 0 );                           /* The scheduler has not started so use a block time of 0. */

    //     // /* Start the tasks and timer running. */
    //     // vTaskStartScheduler();
    //     printf("Starting EDF Scheduler\n");
    //     xTaskCreate(T1, (signed char *)"T1", configMINIMAL_STACK_SIZE, (void *)&dT1, 1, &xT1);
    //     xTaskCreate(T2, (signed char *)"T2", configMINIMAL_STACK_SIZE, (void *)&dT2, 1, &xT2);
    //     xTaskCreate(T3, (signed char *)"T3", configMINIMAL_STACK_SIZE, (void *)&dT3, 1, &xT3);
    //     xTaskCreate(T4, (signed char *)"T4", configMINIMAL_STACK_SIZE, (void *)&dT4, 1, &xT4);
    //     /* Start the tasks running. */
    //     vTaskStartScheduler();
    // }

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
/*-----------------------------------------------------------*/

#ifdef configUSE_DEFAULT_SCHEDULING
    static void taskBatchCreate(void* pvParameters)
    {
        (void*) pvParameters;
        for(;iter_num<4;iter_num++) {
            if (taskStatusMap == 0) {
                printf("taskBatchCreate called when: %d\n", xTaskGetTickCount());
                xTaskCreate(T1, (signed char *)"T1", configMINIMAL_STACK_SIZE, NULL, priority_list[iter_num][0], &xT1);
                mainSET_TASK_STATUS(taskStatusMap, 0, 1);
                xTaskCreate(T2, (signed char *)"T2", configMINIMAL_STACK_SIZE, NULL, priority_list[iter_num][1], &xT2);
                mainSET_TASK_STATUS(taskStatusMap, 1, 1);
                xTaskCreate(T3, (signed char *)"T3", configMINIMAL_STACK_SIZE, NULL, priority_list[iter_num][2], &xT3);
                mainSET_TASK_STATUS(taskStatusMap, 2, 1);
                xTaskCreate(T4, (signed char *)"T4", configMINIMAL_STACK_SIZE, NULL, priority_list[iter_num][3], &xT4);
                mainSET_TASK_STATUS(taskStatusMap, 3, 1);
                printf("iter_num: %d\n", iter_num);
            }

            vTaskDelay(100);
        }
        vTaskDelete(NULL);
    }
#endif

#ifdef configUSE_EDF_SCHEDULING
    static void taskBatchCreate(void* pvParameters)
    {
        (void*) pvParameters;
        for(;iter_num<4;iter_num++) {
            if (taskStatusMap == 0) {
                printf("taskBatchCreate called when: %d\n", xTaskGetTickCount());
                xTaskCreate(T1, (signed char *)"T1", configMINIMAL_STACK_SIZE, (void *)&hard_deadline_list[iter_num][0], 1, &xT1);
                mainSET_TASK_STATUS(taskStatusMap, 0, 1);
                xTaskCreate(T2, (signed char *)"T2", configMINIMAL_STACK_SIZE, (void *)&hard_deadline_list[iter_num][1], 1, &xT2);
                mainSET_TASK_STATUS(taskStatusMap, 1, 1);
                xTaskCreate(T3, (signed char *)"T3", configMINIMAL_STACK_SIZE, (void *)&hard_deadline_list[iter_num][2], 1, &xT3);
                mainSET_TASK_STATUS(taskStatusMap, 2, 1);
                xTaskCreate(T4, (signed char *)"T4", configMINIMAL_STACK_SIZE, (void *)&hard_deadline_list[iter_num][3], 1, &xT4);
                mainSET_TASK_STATUS(taskStatusMap, 3, 1);
                printf("iter_num: %d\n", iter_num);
            }

            vTaskDelay(100);
        }
        vTaskDelete(NULL);
    }
#endif



/*-----------------------------------------------------------*/

static void T1(void *pvParameters)
{
    int timeDelay = delay_list[iter_num][0];
    if(timeDelay > 0)
        vTaskDelay(timeDelay);
    BaseType_t taskStartTime = xTaskGetTickCount();
    printf("T1 executed when: %d\n", taskStartTime);
    for(;;) {
        if(xTaskGetTickCount() - taskStartTime >= runtime_list_tick[iter_num][0])
            break;
    }
    mainSET_TASK_STATUS(taskStatusMap, 0, 0);
    printf("T1 ended when: %d\n", xTaskGetTickCount());
    vTaskDelete(NULL);
}

static void T2(void *pvParameters)
{
    int timeDelay = delay_list[iter_num][1];
    if(timeDelay > 0)
        vTaskDelay(timeDelay);
    BaseType_t taskStartTime = xTaskGetTickCount();
    printf("T2 executed when: %d\n", taskStartTime);
    for(;;) {
        if(xTaskGetTickCount() - taskStartTime >= runtime_list_tick[iter_num][1])
            break;
    }
    mainSET_TASK_STATUS(taskStatusMap, 1, 0);
    printf("T2 ended when: %d\n", xTaskGetTickCount());
    vTaskDelete(NULL);

}

static void T3(void *pvParameters)
{
    int timeDelay = delay_list[iter_num][2];
    if(timeDelay > 0)
        vTaskDelay(timeDelay);
    BaseType_t taskStartTime = xTaskGetTickCount();
    printf("T3 executed when: %d\n", taskStartTime);
    for(;;) {
        if(xTaskGetTickCount() - taskStartTime >= runtime_list_tick[iter_num][2])
            break;
    }
    mainSET_TASK_STATUS(taskStatusMap, 2, 0);
    printf("T3 ended when: %d\n", xTaskGetTickCount());
    vTaskDelete(NULL);

}

static void T4(void *pvParameters)
{
    int timeDelay = delay_list[iter_num][3];
    if(timeDelay > 0)
        vTaskDelay(timeDelay);
    BaseType_t taskStartTime = xTaskGetTickCount();
    printf("T4 executed when: %d\n", xTaskGetTickCount());
    for(;;) {
        if(xTaskGetTickCount() - taskStartTime >= runtime_list_tick[iter_num][3])
            break;
    }
    mainSET_TASK_STATUS(taskStatusMap, 3, 0);
    printf("T4 ended when: %d\n", xTaskGetTickCount());
    vTaskDelete(NULL);
}

