#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* 定义任务句柄 */
TaskHandle_t xSensorTaskHandle = NULL;
TaskHandle_t xCommunicationTaskHandle = NULL;
TaskHandle_t xProcessingTaskHandle = NULL;

/* 定义任务的截止时间 */
TickType_t sensorTaskDeadline = pdMS_TO_TICKS(3000);
TickType_t communicationTaskDeadline = pdMS_TO_TICKS(4000);
TickType_t processingTaskDeadline = pdMS_TO_TICKS(2000);

/* 模拟的传感器数据 */
int sensorData = 0;

/* 任务函数原型 */
void vSensorTask(void *pvParameters);
void vCommunicationTask(void *pvParameters);
void vProcessingTask(void *pvParameters);

/* 设置任务的截止时间 */
void vSetTaskDeadline(TaskHandle_t xTask, TickType_t deadline);

/* 应用程序入口 */
int main(void)
{
    /* 创建处理任务 */
    xTaskCreate(vProcessingTask, "ProcessingTask", configMINIMAL_STACK_SIZE, NULL, 3, &xProcessingTaskHandle);
    vSetTaskDeadline(xProcessingTaskHandle, processingTaskDeadline);

    /* 延时 1 秒后创建传感器任务 */
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreate(vSensorTask, "SensorTask", configMINIMAL_STACK_SIZE, NULL, 2, &xSensorTaskHandle);
    vSetTaskDeadline(xSensorTaskHandle, sensorTaskDeadline);

    /* 延时 2 秒后创建通信任务 */
    vTaskDelay(pdMS_TO_TICKS(2000));
    xTaskCreate(vCommunicationTask, "CommunicationTask", configMINIMAL_STACK_SIZE, NULL, 1, &xCommunicationTaskHandle);
    vSetTaskDeadline(xCommunicationTaskHandle, communicationTaskDeadline);

    /* 启动调度器 */
    vTaskStartScheduler();

    for(;;);
}

/* 传感器任务：采集数据 */
void vSensorTask(void *pvParameters)
{
    for (;;)
    {
        /* 模拟数据采集 */
        sensorData++;
        printf("Sensor Task: Data collected = %d\n", sensorData);

        /* 模拟任务执行时间 */
        vTaskDelay(pdMS_TO_TICKS(500));

        /* 延时到下一个周期 */
        vTaskDelayUntil(NULL, sensorTaskDeadline);
    }
}

/* 通信任务：发送数据 */
void vCommunicationTask(void *pvParameters)
{
    for (;;)
    {
        printf("Communication Task: Sending data = %d\n", sensorData);

        /* 模拟任务执行时间 */
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* 延时到下一个周期 */
        vTaskDelayUntil(NULL, communicationTaskDeadline);
    }
}

/* 处理任务：对数据进行计算 */
void vProcessingTask(void *pvParameters)
{
    for (;;)
    {
        /* 模拟数据处理 */
        int processedData = sensorData * 2;
        printf("Processing Task: Processed data = %d\n", processedData);

        /* 模拟任务执行时间 */
        vTaskDelay(pdMS_TO_TICKS(500));

        /* 延时到下一个周期 */
        vTaskDelayUntil(NULL, processingTaskDeadline);
    }
}

