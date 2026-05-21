#include "demo_freertos.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include "ir_receiver.h"
#include "OLED.h"
#include "uart.h"
#include "timer.h"



TaskHandle_t start_task_handle;
/* 队列 */
static QueueHandle_t g_xQueueIRReceiver;

/* 信号量 */
static SemaphoreHandle_t	g_xSemaphoreCountHandle;

/* 函数声明 */
void start_task(void* pvParameters);
void high_task1(void* pvParameters);
void medium_task2(void* pvParameters);
void low_task3(void* pvParameters);

void demo_freertos_init(void)
{
	xTaskCreate(start_task,"start_task",128,NULL,osPriorityNormal+2,&start_task_handle);
	g_xQueueIRReceiver = GetQueueIR();
}

void start_task(void* pvParameters)
{
	while(1)
	{
		g_xSemaphoreCountHandle = xSemaphoreCreateCounting(1,1);
		xTaskCreate(high_task1,"task1",128,NULL,osPriorityNormal+1,NULL);
		xTaskCreate(medium_task2,"task2",128,NULL,osPriorityNormal ,NULL);
		xTaskCreate(low_task3,"task3",128,NULL,osPriorityNormal-1,NULL);
		vTaskDelete(start_task_handle);
	}
}

void high_task1(void* pvParameters)
{
	vTaskDelay(500);
	while(1)
	{		
		printf("high_task1 ready to take semaphore\r\n");
		xSemaphoreTake(g_xSemaphoreCountHandle,portMAX_DELAY);
		printf("high_task1 has taked semaphore\r\n");
		printf("high_task1 running\r\n");
		printf("high_task1 give semaphore\r\n");
		xSemaphoreGive(g_xSemaphoreCountHandle);
		vTaskDelay(100);
	}
}
void medium_task2(void* pvParameters)
{
	int task2_num = 0;
	vTaskDelay(200);
	while(1)
	{
		for(task2_num=0;task2_num<5;task2_num++)
		{
			printf("medium_task2 running\r\n");
			mdelay(100);
		}
		vTaskDelay(1000);
	}
}
void low_task3(void* pvParameters)
{
	int task3_num = 0;
	while(1)
	{
		printf("low_task3 ready to take semaphore\r\n");
		xSemaphoreTake(g_xSemaphoreCountHandle,portMAX_DELAY);
		printf("low_task3 has taked semaphore\r\n");
		for (task3_num=0; task3_num<5; task3_num++)
		{
		printf("low_task3 running\r\n");
		mdelay(100); /* 模拟运行， 不触发任务调度 */
		}		
		printf("low_task3 give semaphore\r\n");
		xSemaphoreGive(g_xSemaphoreCountHandle);
		vTaskDelay(1000);
	}
}















