/*  Copyright (s) 2019 深圳百问网科技有限公司
 *  All rights reserved
 * 
 * 文件名称：driver_rotary_encoder.c
 * 摘要：
 *  
 * 修改历史     版本号        Author       修改内容
 *--------------------------------------------------
 * 2023.08.05      v01         百问科技      创建文件
 *--------------------------------------------------
*/

#include "main.h"
#include "rotary_encoder.h"
#include "OLED.h"
#include "timer.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "typedefs.h"

#include "FreeRTOS.h"
#include "queue.h"

/*
 * PB12 - S1
 * PB0  - S2
 * PB1  - Key
 */
#define ROTARY_ENCODER_S1_GPIO_GROUP GPIOB
#define ROTARY_ENCODER_S1_GPIO_PIN   GPIO_PIN_12

#define ROTARY_ENCODER_S2_GPIO_GROUP GPIOB
#define ROTARY_ENCODER_S2_GPIO_PIN   GPIO_PIN_0

#define ROTARY_ENCODER_KEY_GPIO_GROUP GPIOB
#define ROTARY_ENCODER_KEY_GPIO_PIN   GPIO_PIN_1

/* 全局变量 */
static int32_t g_count = 0;
static int32_t g_speed = 0; /* 速度(正数表示顺时针旋转,负数表示逆时针旋转,单位:每秒转动次数) */

/* 队列 */
static QueueHandle_t g_xQueueRotaryEncoderHandle;


/**********************************************************************
 * 函数名称： RotaryEncoder_Get_Key
 * 功能描述： 读取旋转编码器Key引脚电平
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 1-按键被按下, 0-按键被松开
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int RotaryEncoder_Get_Key(void)
{
    if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(ROTARY_ENCODER_KEY_GPIO_GROUP, ROTARY_ENCODER_KEY_GPIO_PIN))
        return 1;
    else
        return 0;
}

/**********************************************************************
 * 函数名称： RotaryEncoder_Get_S1
 * 功能描述： 读取旋转编码器S1引脚电平
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int RotaryEncoder_Get_S1(void)
{
    if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(ROTARY_ENCODER_S1_GPIO_GROUP, ROTARY_ENCODER_S1_GPIO_PIN))
        return 0;
    else
        return 1;
}

/**********************************************************************
 * 函数名称： RotaryEncoder_Get_S2
 * 功能描述： 读取旋转编码器S2引脚电平
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int RotaryEncoder_Get_S2(void)
{
    if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(ROTARY_ENCODER_S2_GPIO_GROUP, ROTARY_ENCODER_S2_GPIO_PIN))
        return 0;
    else
        return 1;
}


/**********************************************************************
 * 函数名称： RotaryEncoder_IRQ_Callback
 * 功能描述： 旋转编码器的中断回调函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
void RotaryEncoder_IRQ_Callback(void)
{
		RotaryEncoder_Data_Struct rdata;
    uint64_t time;
    static uint64_t pre_time = 0;
        
	/* 1. 记录中断发生的时刻 */	
	time = system_get_ns();

    /* 上升沿触发: 必定是高电平 
     * 防抖
     */
//    mdelay(2);
		if(time - pre_time < 2000000)/* 当前中断的时间 跟 上次中断的时间 < 2ms的话,认为是抖动 */
		{
			pre_time = time;
			return;
		}
	
    if (!RotaryEncoder_Get_S1())
        return;

    /* S1上升沿触发中断
     * S2为0表示逆时针转, 为1表示顺时针转
     */
    g_speed = (uint64_t)1000000000/(time - pre_time);
		if(g_speed == 0)
			g_speed = 1;
    if (RotaryEncoder_Get_S2())
    {
        g_count++;
    }
    else
    {
        g_count--;
        g_speed = 0 - g_speed;
    }
    pre_time = time;
        
		/* 写队列 */
		rdata.cnt = g_count;
		rdata.speed = g_speed;
		xQueueSendFromISR(g_xQueueRotaryEncoderHandle,&rdata,NULL);
}


/**********************************************************************
 * 函数名称： RotaryEncoder_Init
 * 功能描述： 旋转编码器的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/05	     V1.0	  韦东山	      创建
 ***********************************************************************/
void RotaryEncoder_Init(void)
{
    /* PB0,PB1在MX_GPIO_Init中被配置为输入引脚 */
    /* PB12在MX_GPIO_Init中被配置为中断引脚,上升沿触发 */
		g_xQueueRotaryEncoderHandle = xQueueCreate(10,sizeof(RotaryEncoder_Data_Struct));
}

/**********************************************************************
 * 函数名称： RotaryEncoder_Write
 * 功能描述： 旋转编码器的读取函数
 * 输入参数： 无
 * 输出参数： pCnt   - 用于保存计数值 
 *            pSpeed - 用于保存速度(正数表示顺时针旋转,负数表示逆时针旋转,单位:每秒转动次数)
 *            pKey   - 用于保存按键状态(1-按键被按下, 0-按键被松开)
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/05	     V1.0	  韦东山	      创建
 ***********************************************************************/
void RotaryEncoder_Read(int32_t *pCnt, int32_t *pSpeed, int32_t *pKey)
{
    static int32_t pre_count = 0;
    *pCnt = g_count;

    if (g_count == pre_count)
        *pSpeed = 0;
    else
        *pSpeed = g_speed;

    pre_count = g_count;
    
    *pKey = RotaryEncoder_Get_Key();
}


/**********************************************************************
 * 函数名称： RotaryEncoder_Test
 * 功能描述： 旋转编码器测试程序
 * 输入参数： 无
 * 输出参数： 无
 *            无
 * 返 回 值： 无
 * 修改日期        版本号     修改人        修改内容
 * -----------------------------------------------
 * 2023/08/05        V1.0     韦东山       创建
 ***********************************************************************/
void RotaryEncoder_Test(void)
{
    int32_t count, speed, key;
    
    RotaryEncoder_Init();

    while (1)
    {
        OLED_ShowString(1, 1, "RotaryEncoder: ");

        RotaryEncoder_Read(&count, &speed, &key);

        OLED_ClearLine(2);
        OLED_ShowString(2, 1, "Cnt:");
        OLED_ShowNumNoLength(2, 5, count);
        
        OLED_ClearLine(3);
        OLED_ShowString(3, 1, "Speed:");
        OLED_ShowNumNoLength(3, 7, speed);
        
        OLED_ClearLine(4);
        OLED_ShowString(4, 1, "Key:");
        OLED_ShowString(4, 5, key ? "Pressed":"Released");
    }
}
/**********************************************************************
 * 函数名称： vRotaryEncoderTaskFunction
 * 功能描述： 旋转编码器任务功能程序
 * 输入参数： 无
 * 输出参数： 无
 *            无
 * 返 回 值： 无
 * 修改日期        版本号     修改人        修改内容
 * -----------------------------------------------
 * 2023/08/05        V1.0     韦东山       创建
 ***********************************************************************/
void vRotaryEncoderTaskFunction(void* pvParameters)
{
    
		int cnt,left;
		RotaryEncoder_Data_Struct rdata;
		uint8_t idata;
			
    RotaryEncoder_Init();
	
    while (1)
    {
				/* 读旋转编码器队列 */
				if(pdPASS == xQueueReceive(g_xQueueRotaryEncoderHandle,&rdata,portMAX_DELAY))
				{
					/* 处理数据 */
					if(rdata.speed > 0)
					{
						left = 1;
					}
					else 
					{
						rdata.speed = 0 - rdata.speed;
						left = 0;
					}
						
					if(rdata.speed > 100)
						cnt = 4;
					else if(rdata.speed > 50)
						cnt =2;
					else
						cnt = 1;
					/* 写挡球板队列 */
					idata = left ? UPT_MOVE_LEFT : UPT_MOVE_RIGHT;
					for(int i = 0;i < cnt;i++)
					xQueueSend(g_xQueuePlatformHandle,&idata,portMAX_DELAY);
				}
//        OLED_ClearLine(1);
//        OLED_ShowString(1, 1, "Cnt:");
//        OLED_ShowNumNoLength(1, 5, rdata.cnt);
//        
//        OLED_ClearLine(2);
//        OLED_ShowString(2, 1, "Speed:");
//        OLED_ShowNumNoLength(2, 7, rdata.speed);
//					
//        OLED_ClearLine(3);
//        OLED_ShowString(3, 1, "Cnt:");
//        OLED_ShowNumNoLength(3, 5, g_count);
//					
//        OLED_ClearLine(4);
//        OLED_ShowString(4, 1, "Speed:");
//        OLED_ShowNumNoLength(4, 7, g_speed);
        
				vTaskDelay(pdMS_TO_TICKS(20));
    }
}


