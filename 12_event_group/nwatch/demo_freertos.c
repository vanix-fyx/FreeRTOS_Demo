#include "demo_freertos.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include "ir_receiver.h"
#include "OLED.h"
#include "uart.h"
#include "timer.h"
#include "event_groups.h"

#define EVENTBIT_0 1 << 0
#define EVENTBIT_1 1 << 1
#define EVENTBIT_2 1 << 2

TaskHandle_t start_task_handle;
/* 队列 */
static QueueHandle_t g_xQueueIRReceiver_Handle;

/* 信号量 */
static SemaphoreHandle_t	g_xSemaphoreMutex;

/* 事件标志组 */
static EventGroupHandle_t	g_xEventGroup_Handle;

/* 函数声明 */
void start_task(void* pvParameters);
void high_task1(void* pvParameters);
void medium_task2(void* pvParameters);
void low_task3(void* pvParameters);

void demo_freertos_init(void)
{
	xTaskCreate(start_task,"start_task",128,NULL,osPriorityNormal+2,&start_task_handle);
	g_xQueueIRReceiver_Handle = GetQueueIR();
}

void start_task(void* pvParameters)
{
	while(1)
	{
		g_xEventGroup_Handle = xEventGroupCreate();
		if(g_xEventGroup_Handle != NULL)
			printf("事件标志组创建成功\r\n");
		else
			printf("事件标志组创建失败\r\n");
		xTaskCreate(high_task1,"task1",128,NULL,osPriorityNormal+1,NULL);
		xTaskCreate(medium_task2,"task2",128,NULL,osPriorityNormal ,NULL);
		xTaskCreate(low_task3,"task3",128,NULL,osPriorityNormal-1,NULL);
		vTaskDelete(start_task_handle);
	}
}

void high_task1(void* pvParameters)
{
	IRReceiver_Data_Struct rdata;
	while(1)
	{		
		if(pdTRUE == xQueueReceive(g_xQueueIRReceiver_Handle,&rdata,portMAX_DELAY))
		{
			switch(rdata.data)
			{
				case IR_KEY_0:
					xEventGroupSetBits(g_xEventGroup_Handle,EVENTBIT_0);
					break;
				case IR_KEY_1:
					xEventGroupSetBits(g_xEventGroup_Handle,EVENTBIT_1);
					break;
				case IR_KEY_2:
					xEventGroupSetBits(g_xEventGroup_Handle,EVENTBIT_2);
					break;
				case IR_KEY_4:
					xEventGroupClearBits(g_xEventGroup_Handle,EVENTBIT_0);
					break;
				case IR_KEY_5:
					xEventGroupClearBits(g_xEventGroup_Handle,EVENTBIT_1);
					break;
				case IR_KEY_6:
					xEventGroupClearBits(g_xEventGroup_Handle,EVENTBIT_2);
					break;
			}
			vTaskDelay(10);
		}
	}
}
void medium_task2(void* pvParameters)
{
	EventBits_t event_bit_val = 0;
	while(1)
	{
		event_bit_val = xEventGroupWaitBits(g_xEventGroup_Handle,		/* 等待的事件标志组句柄 */
																				EVENTBIT_0 | EVENTBIT_1| EVENTBIT_2,	/* 等待的事件标志位， 可以用逻辑或等待多个事件标志位 */
																				pdFALSE,		/* 成功等待到事件标志位后， 清除事件组中对应的事件标志位 */
																				pdTRUE,		/* 等待 uxBitsToWaitFor 中的所有事件标志位（逻辑与） */
																				portMAX_DELAY);			/* 获取等待的阻塞时间 */
		printf("0x%x\r\n",event_bit_val);
		vTaskDelay(10);
	}
}
void low_task3(void* pvParameters)
{
	int event_val = 0;
	while(1)
	{
		event_val = xEventGroupGetBits(g_xEventGroup_Handle);
		OLED_ShowBinNum(1,1,event_val,5);
		vTaskDelay(10);
	}
}















