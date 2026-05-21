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

/* 任务句柄 */
TaskHandle_t start_task_handle;
TaskHandle_t task1_handle;
TaskHandle_t task2_handle;
TaskHandle_t task3_handle;
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
		xTaskCreate(high_task1,"task1",128,NULL,osPriorityNormal+1,&task1_handle);
		xTaskCreate(medium_task2,"task2",128,NULL,osPriorityNormal ,&task2_handle);
//		xTaskCreate(low_task3,"task3",128,NULL,osPriorityNormal-1,&task1_handl3);
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
					 if(pdPASS == xTaskNotify(task2_handle, 0, eSetValueWithoutOverwrite) )
						 printf("发送任务通知0成功\r\n");
					 else
						 printf("发送任务通知0失败\r\n");
					break;
				case IR_KEY_1:
					 if(pdPASS == xTaskNotify(task2_handle, 1, eSetValueWithOverwrite) )
						 printf("发送任务通知1成功\r\n");
					 else
						 printf("发送任务通知1失败\r\n");
					break;
				case IR_KEY_2:
					 if(pdPASS == xTaskNotify(task2_handle, 2, eSetValueWithOverwrite) )
						 printf("发送任务通知2成功\r\n");
					 else
						 printf("发送任务通知2失败\r\n");
					break;
				case IR_KEY_4:
					break;
				case IR_KEY_5:
					break;
				case IR_KEY_6:
					break;
			}
			vTaskDelay(10);
		}
	}
}
void medium_task2(void* pvParameters)
{
	uint32_t notify_val = 0;
	while(1)
	{
		xTaskNotifyWait(0,0xffffffff,&notify_val,portMAX_DELAY);
		printf("任务通知值为:%d\r\n",notify_val);
		vTaskDelay(1000);
	}
}
void low_task3(void* pvParameters)
{
	int event_val = 0;
	while(1)
	{
		vTaskDelay(10);
	}
}















