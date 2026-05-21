/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#if 1	//开启的任务 - 结构体定义
/* 使用同一函数创建不同任务OLEDPrint ---------------------------------------------------------*/
struct TaskPrintInfo{
	uint8_t x; /* 列 */
	uint8_t y; /* 行 */
	char name[16];
};

#endif
#if 0	//关闭的任务 - 结构体定义
/* 使用同一函数创建不同任务OLEDPrint ---------------------------------------------------------*/
struct TaskPrintInfo{
	uint8_t x; /* 列 */
	uint8_t y; /* 行 */
	char name[16];
};

#endif

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
#if 1	//开启的任务 - 句柄与变量定义
/* 静态任务 - 句柄与变量定义 ---------------------------------------------------------*/
	
/* 动态任务 - 句柄与变量定义 ---------------------------------------------------------*/
/* 使用同一函数创建不同任务OLEDPrint ---------------------------------------------------------*/
	TaskHandle_t xOLEDPrintTask2Handle = NULL;
	static struct TaskPrintInfo g_Task2Info = {1,2,"Task2"};
	static int g_OLEDEnable = 1;
	
#endif
#if 0	//关闭的任务 - 句柄与变量定义
/* 静态任务 - 句柄与变量定义 ---------------------------------------------------------*/
	StaticTask_t xLEDTaskTCB;	//OLED静态任务控制块（TCB）
	StackType_t xLEDTaskStack[128];	//任务栈空间数组
	TaskHandle_t xLEDTaskHandle = NULL;	// LED任务句柄

	StaticTask_t xOLEDTaskTCB;	//OLED静态任务控制块（TCB）
	StackType_t xOLEDTaskStack[128];	//任务栈空间数组
	TaskHandle_t xOLEDTaskHandle = NULL;	// OLED任务句柄

	StaticTask_t xIRReceiverTaskTCB;	//IRReceiver静态任务控制块（TCB）
	StackType_t xIRReceiverTaskStack[256];	//任务栈空间数组
	TaskHandle_t xIRReceiverTaskHandle = NULL;	// IRReceiver任务句柄

/* 动态任务 - 句柄与变量定义 ---------------------------------------------------------*/
/* 使用同一函数创建不同任务OLEDPrint ---------------------------------------------------------*/
	TaskHandle_t xOLEDPrintTask1Handle = NULL;
	TaskHandle_t xOLEDPrintTask2Handle = NULL;
	TaskHandle_t xOLEDPrintTask3Handle = NULL;
	static struct TaskPrintInfo g_Task1Info = {1,1,"Task1"};//静态的全局变量只能在本文件中使用
	static struct TaskPrintInfo g_Task2Info = {1,2,"Task2"};
	static struct TaskPrintInfo g_Task3Info = {1,3,"Task3"};
	static int g_OLEDEnable = 1;
	
#endif

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

//freertos文件的任务 - 函数原型
/* 使用同一函数创建不同任务OLEDPrint ---------------------------------------------------------*/
void vOLEDPrintTaskFuction(void* params)
{
	struct TaskPrintInfo* Info = params;
	uint16_t cnt = 0;
	for(;;)
	{
		if(g_OLEDEnable)
		{
			g_OLEDEnable = 0;
			OLED_ClearLine(Info->y);
			OLED_ShowString(Info->y ,Info->x,Info->name);
			OLED_ShowChar(Info->y,6,':');
			OLED_ShowNumNoLength(Info->y,7,cnt++);
			g_OLEDEnable = 1;
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	OLED_Init();
	OLED_Clear();
	IRReceiver_Init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
#if 1	//开启的任务 - 任务创建
			
#endif
#if 0	//关闭的任务 - 任务创建
  /*创建任务：LED*/
  xLEDTaskHandle = xTaskCreateStatic(vLEDTaskFunction,"LEDTask",128,NULL,osPriorityNormal,
		  xLEDTaskStack,&xLEDTaskTCB);
  /*创建任务：OLED*/
  xOLEDTaskHandle = xTaskCreateStatic(vOLEDTaskFunction,"OLEDTask",128,NULL,osPriorityNormal,
		  xOLEDTaskStack,&xOLEDTaskTCB);
  /*创建任务：IRReceiver*/
  xIRReceiverTaskHandle = xTaskCreateStatic(vIRReceiverTaskFunction,"IRReceiverTask",256,NULL,osPriorityNormal,
		  xIRReceiverTaskStack,&xIRReceiverTaskTCB);
  /*使用同一函数创建不同任务OLEDPrint*/
	xTaskCreate(vOLEDPrintTaskFuction,"OLEDPrintTask1",128,
							&g_Task1Info,osPriorityNormal,&xOLEDPrintTask1Handle);
	xTaskCreate(vOLEDPrintTaskFuction,"OLEDPrintTask2",128,
							&g_Task2Info,osPriorityNormal,&xOLEDPrintTask2Handle);
	xTaskCreate(vOLEDPrintTaskFuction,"OLEDPrintTask3",128,
							&g_Task3Info,osPriorityNormal,&xOLEDPrintTask3Handle);

#endif
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	
  uint8_t dev, data;
	int bRuning;	
	
  /* Infinite loop */
  for(;;)
  {
		if (!IRReceiver_Read(&dev, &data))
		{
			if(data == 0xa8)	/* play */
			{
				/* 创建显示任务 */
				if(xOLEDPrintTask2Handle == NULL)
				{
					OLED_ClearLine(1);
					OLED_ShowString(1,1,"Creat Task");
					xTaskCreate(vOLEDPrintTaskFuction,"OLEDPrintTask2",128,
											&g_Task2Info,osPriorityNormal,&xOLEDPrintTask2Handle);	
					bRuning = 1;
				}
				else
				{
					if(bRuning == 1)
					{
						OLED_ClearLine(1);
						OLED_ShowString(1,1,"Suspend Task");
						vTaskSuspend(xOLEDPrintTask2Handle);
						bRuning = 0;
					}
					else
					{
						OLED_ClearLine(1);
						OLED_ShowString(1,1,"Resume Task");
						vTaskResume(xOLEDPrintTask2Handle);
						bRuning = 1;
					}
				}
			}
			else if(data == 0xa2)	/* power */
			{
				/* 删除显示任务 */
				if(xOLEDPrintTask2Handle != NULL)
				{
					OLED_Clear();
					OLED_ShowString(1,1,"Delete Task");
					vTaskDelete(xOLEDPrintTask2Handle);
					xOLEDPrintTask2Handle = NULL;
					bRuning = 0;
				}					
			}
		}
    osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

