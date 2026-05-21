#include "demo_freertos.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"

#include "ir_receiver.h"
#include "OLED.h"
#include "uart.h"

#define EVENTBIT_0 1 << 0
#define EVENTBIT_1 1 << 1
#define EVENTBIT_2 1 << 2

/* 任务句柄 */
TaskHandle_t start_task_handle;
TaskHandle_t task1_handle;
//TaskHandle_t task2_handle;
//TaskHandle_t task3_handle;
/* 队列 */
static QueueHandle_t g_xQueueIRReceiver_Handle;

/* 信号量 */
static SemaphoreHandle_t	g_xSemaphoreMutex;

/* 事件标志组 */
static EventGroupHandle_t	g_xEventGroup_Handle;

/* 软件定时器 */
static TimerHandle_t	Timer1_Handle;
static TimerHandle_t	Timer2_Handle;

/* 函数声明 */
void start_task(void* pvParameters);
void high_task1(void* pvParameters);
//void medium_task2(void* pvParameters);
//void low_task3(void* pvParameters);
void Timer1_Callback( TimerHandle_t xTimer );
void Timer2_Callback( TimerHandle_t xTimer );

void demo_freertos_init(void)
{
	xTaskCreate(start_task,"start_task",128,NULL,osPriorityNormal+2,&start_task_handle);
	g_xQueueIRReceiver_Handle = GetQueueIR();
}

void start_task(void* pvParameters)
{
		/* 创建任务 */
		xTaskCreate(high_task1,"task1",128,NULL,osPriorityNormal+1,&task1_handle);
//		xTaskCreate(medium_task2,"task2",128,NULL,osPriorityNormal ,&task2_handle);
//		xTaskCreate(low_task3,"task3",128,NULL,osPriorityNormal-1,&task1_handl3);
		
		/* 创建软件定时器 */
		Timer1_Handle = xTimerCreate("timer1",1000,pdFALSE,(void*) 1,Timer1_Callback);	/* 单次定时器 */
		Timer2_Handle = xTimerCreate("timer2",1000,pdTRUE,(void*) 2,Timer2_Callback);	/* 周期定时器 */
		
		vTaskDelete(start_task_handle);
		printf("start_task删除失败\r\n");
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
					{
						printf("IR_KEY_0\r\n");
						break;
					}
				case IR_KEY_1:
					{
						xTimerStart(Timer1_Handle,portMAX_DELAY);
						xTimerStart(Timer2_Handle,portMAX_DELAY);
						break;
					}
				case IR_KEY_2:
					{
						xTimerStop(Timer1_Handle,portMAX_DELAY);
						xTimerStop(Timer2_Handle,portMAX_DELAY);
						break;
					}
				case IR_KEY_3:
					{
						xTimerReset(Timer1_Handle,portMAX_DELAY);
						xTimerReset(Timer2_Handle,portMAX_DELAY);
						break;
					}
				case IR_KEY_4:
					{
						xTimerChangePeriod(Timer1_Handle,500,portMAX_DELAY);
						break;
					}
				case IR_KEY_5:
					{
						xTimerChangePeriod(Timer2_Handle,2000,portMAX_DELAY);
						break;
					}
			}
			vTaskDelay(10);
		}
	}
}
void Timer1_Callback( TimerHandle_t xTimer  )
{
	static int cnt = 0;
	printf("timer1运行次数：%d\r\n",++cnt);
}
void Timer2_Callback( TimerHandle_t xTimer )
{
	static int cnt = 0;
	printf("timer2运行次数：%d\r\n",++cnt);
}
void PRE_SLEEP_PROCESSING(void)
{
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOD_CLK_DISABLE();
	__HAL_RCC_GPIOE_CLK_DISABLE();
}
void POST_SLEEP_PROCESSING(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
}












