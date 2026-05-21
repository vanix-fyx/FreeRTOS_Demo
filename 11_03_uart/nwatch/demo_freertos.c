#include "demo_freertos.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include "ir_receiver.h"
#include "OLED.h"


TaskHandle_t start_task_handle;
/* 队列 */
static QueueHandle_t g_xQueueIRReceiver;

/* 信号量 */
static SemaphoreHandle_t	g_xSemaphoreCountHandle;


void demo_freertos_init(void)
{
	xTaskCreate(start_task,"start_task",128,NULL,osPriorityNormal+2,&start_task_handle);
	g_xQueueIRReceiver = GetQueueIR();
}

void start_task(void* pvParameters)
{
	while(1)
	{
		g_xSemaphoreCountHandle = xSemaphoreCreateCounting(100,0);
		xTaskCreate(task1,"task1",128,NULL,osPriorityNormal-1,NULL);
		xTaskCreate(task2,"task2",128,NULL,osPriorityNormal + 1,NULL);
		vTaskDelete(start_task_handle);
	}
}

void task1(void* pvParameters)
{
	IRReceiver_Data_Struct rdata;
	int cnt = 0;
	while(1)
	{
		if(pdPASS == xQueueReceive(g_xQueueIRReceiver,&rdata,portMAX_DELAY))
		{
			switch(rdata.data)
			{
				case IR_KEY_1:	xSemaphoreGive(g_xSemaphoreCountHandle);
												cnt = uxSemaphoreGetCount(g_xSemaphoreCountHandle);
												break;
			}
		}
		
		vTaskDelay(10);
		
	}
}
void task2(void* pvParameters)
{
	int cnt = 0;
	while(1)
	{
		xSemaphoreTake(g_xSemaphoreCountHandle,portMAX_DELAY);
		cnt = uxSemaphoreGetCount(g_xSemaphoreCountHandle);
		OLED_ClearLine(1);
		OLED_ShowNumNoLength(1,1,cnt);
		vTaskDelay(1000);
	}
}















