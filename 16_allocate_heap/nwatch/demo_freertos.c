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
static QueueHandle_t g_xQueueMem_Handle;

static SemaphoreHandle_t g_xSemaphoreCountMemory_Handle;

/* 函数声明 */
void start_task(void* pvParameters);
void high_task1(void* pvParameters);
//void medium_task2(void* pvParameters);
//void low_task3(void* pvParameters);

void demo_freertos_init(void)
{
	xTaskCreate(start_task,"start_task",128,NULL,osPriorityNormal+2,&start_task_handle);
	g_xQueueIRReceiver_Handle = GetQueueIR();
}

void start_task(void* pvParameters)
{
		/* 创建任务 */
		xTaskCreate(high_task1,"task1",256,NULL,osPriorityNormal+1,&task1_handle);
//		xTaskCreate(medium_task2,"task2",128,NULL,osPriorityNormal ,&task2_handle);
//		xTaskCreate(low_task3,"task3",128,NULL,osPriorityNormal-1,&task1_handl3);
	
		/* 创建队列 */
		g_xQueueMem_Handle = xQueueCreate(10,sizeof(uint8_t *));
	
		/* 创建信号量 */
		g_xSemaphoreCountMemory_Handle = xSemaphoreCreateCounting(10,0);
			
		/* 删除任务 */
		vTaskDelete(start_task_handle);
		printf("start_task删除失败\r\n");
}

void high_task1(void* pvParameters)
{
	IRReceiver_Data_Struct rdata;
	
	uint8_t *buf = NULL,*freebuf = NULL;
	BaseType_t return_result;
	int memory_count;
	size_t free_size;
	int cnt = 0;
	while(1)
	{		
		if(pdTRUE == xQueueReceive(g_xQueueIRReceiver_Handle,&rdata,0))
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
						buf = pvPortMalloc(30);
						if(buf != NULL)
						{
							return_result = xQueueSend(g_xQueueMem_Handle,&buf,0);
							if(return_result != pdTRUE)
							{
								vPortFree(buf);
								printf("申请内存失败\r\n");
								memory_count = uxSemaphoreGetCount(g_xSemaphoreCountMemory_Handle);
								printf("已申请内存次数:%d\r\n",memory_count);
							}
							else
							{
								printf("申请内存成功\r\n");
								xSemaphoreGive(g_xSemaphoreCountMemory_Handle);
								memory_count = uxSemaphoreGetCount(g_xSemaphoreCountMemory_Handle);
								printf("已申请内存次数:%d\r\n",memory_count);
							}
						}
						break;
					}
				case IR_KEY_2:
					{
						return_result = xQueueReceive(g_xQueueMem_Handle,&freebuf,0);
						if(pdTRUE == return_result)
						{
							vPortFree(freebuf);
							printf("释放内存成功\r\n");
							xSemaphoreTake(g_xSemaphoreCountMemory_Handle,0);
							memory_count = uxSemaphoreGetCount(g_xSemaphoreCountMemory_Handle);
							printf("已申请内存次数:%d\r\n",memory_count);
						}
						else
						{
							printf("释放内存失败\r\n");
							memory_count = uxSemaphoreGetCount(g_xSemaphoreCountMemory_Handle);
							printf("已申请内存次数:%d\r\n",memory_count);
						}
						break;
					}
				case IR_KEY_3:
					{
						free_size =  xPortGetFreeHeapSize();
						memory_count = uxSemaphoreGetCount(g_xSemaphoreCountMemory_Handle);
						printf("内存堆中未分配的内存总量:%d\r\n已申请内存次数:%d\r\n",free_size,memory_count);
						break;
					}
				case IR_KEY_4:
					{
						break;
					}
				case IR_KEY_5:
					{
						break;
					}
			}
		}
		free_size =  xPortGetFreeHeapSize();
		if(cnt >= 100)
		{
			printf("内存堆中未分配的内存总量:%d\r\n",free_size);
			cnt = 0;
		}
		++cnt;
		vTaskDelay(10);
	}
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











