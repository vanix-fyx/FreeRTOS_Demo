#include "led.h"
#include "main.h"
#include "timer.h"


struct RGBCtrl g_RGBCtrl={0,0,0};

void vLEDTaskFunction(void* pvParameters)
{
    ColorLED_Init();

	for(;;)
	{
		g_RGBCtrl.b = 100;
    RGB_Set(g_RGBCtrl.r,g_RGBCtrl.g,g_RGBCtrl.b);
    vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


#if 1	//正点原子精英板两个LED的相关函数
void LED0_ON(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
}

void LED0_OFF(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

void LED0_Toggle(void)
{
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
}

void LED1_ON(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
}

void LED1_OFF(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
}

void LED1_Toggle(void)
{
	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
}


void vLED0TaskFunction(void* pvParameters)
{

	for(;;)
	{
		LED0_Toggle();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void vLED1TaskFunction(void* pvParameters)
{

	for(;;)
	{
		LED1_Toggle();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
#endif
//


#define CHANNEL_RED   TIM_CHANNEL_3
#define CHANNEL_GREEN TIM_CHANNEL_1
#define CHANNEL_BLUE  TIM_CHANNEL_2

static TIM_HandleTypeDef *g_HPWM_ColorLED = &htim2;

/*********************************************************************
 * 函数名称： ColorLED_Init
 * 功能描述： 全彩LED的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 **********************************************************************/
void ColorLED_Init(void)
{
//    MX_TIM2_Init();
	/*定时器配置，采用mx时参考
	 *

	    TIM_OC_InitTypeDef sConfigR;
	    TIM_OC_InitTypeDef sConfigG;
	    TIM_OC_InitTypeDef sConfigB;

	    sConfigR.OCMode = TIM_OCMODE_PWM1;        // PWM 输出的两种模式:PWM1 当极性为低,CCR<CNT,输出低电平,反之高电平
	    sConfigR.OCPolarity = TIM_OCPOLARITY_LOW; // 设置极性为低(硬件上低电平亮灯)
	    sConfigR.OCFastMode = TIM_OCFAST_DISABLE; // 输出比较快速使能禁止(仅在 PWM1 和 PWM2 可设置)
	    sConfigR.Pulse = r*2000/255;              // 在 PWM1 模式下,通道 3(RLED)占空比

	    sConfigG.OCMode = TIM_OCMODE_PWM1;        // PWM 输出的两种模式:PWM1 当极性为低,CCR<CNT,输出低电平,反之高电平
	    sConfigG.OCPolarity = TIM_OCPOLARITY_LOW; // 设置极性为低(硬件上低电平亮灯)
	    sConfigG.OCFastMode = TIM_OCFAST_DISABLE; // 输出比较快速使能禁止(仅在 PWM1 和 PWM2 可设置)
	    sConfigG.Pulse = g*2000/255;              // 在 PWM1 模式下,通道 3(RLED)占空比

	    sConfigB.OCMode = TIM_OCMODE_PWM1;        // PWM 输出的两种模式:PWM1 当极性为低,CCR<CNT,输出低电平,反之高电平
	    sConfigB.OCPolarity = TIM_OCPOLARITY_LOW; // 设置极性为低(硬件上低电平亮灯)
	    sConfigB.OCFastMode = TIM_OCFAST_DISABLE; // 输出比较快速使能禁止(仅在 PWM1 和 PWM2 可设置)
	    sConfigB.Pulse = b*2000/255;              // 在 PWM1 模式下,通道 3(RLED)占空比

	    HAL_TIM_PWM_Stop(g_HPWM_ColorLED, CHANNEL_RED);
	    HAL_TIM_PWM_Stop(g_HPWM_ColorLED, CHANNEL_GREEN);
	    HAL_TIM_PWM_Stop(g_HPWM_ColorLED, CHANNEL_BLUE);

	    HAL_TIM_PWM_ConfigChannel(g_HPWM_ColorLED, &sConfigR, CHANNEL_RED);
	    HAL_TIM_PWM_ConfigChannel(g_HPWM_ColorLED, &sConfigG, CHANNEL_GREEN);
	    HAL_TIM_PWM_ConfigChannel(g_HPWM_ColorLED, &sConfigB, CHANNEL_BLUE);

	    HAL_TIM_PWM_Start(g_HPWM_ColorLED, CHANNEL_RED);
	    HAL_TIM_PWM_Start(g_HPWM_ColorLED, CHANNEL_GREEN);
	    HAL_TIM_PWM_Start(g_HPWM_ColorLED, CHANNEL_BLUE);

	*
	*/
		HAL_TIM_PWM_Start(g_HPWM_ColorLED, CHANNEL_RED);
    HAL_TIM_PWM_Start(g_HPWM_ColorLED, CHANNEL_GREEN);
    HAL_TIM_PWM_Start(g_HPWM_ColorLED, CHANNEL_BLUE);

}

/*********************************************************************
 * 函数名称： ColorLED_Set
 * 功能描述： 全彩LED设置颜色函数
 * 输入参数： color - 24bit颜色,格式为0x00RRGGBB
 * 输出参数： 无
 * 返 回 值： 无
 **********************************************************************/
void ColorLED_Set(uint32_t color)
{

    int r,g,b;

    r = (color >> 16) & 0xff;
    g = (color >> 8) & 0xff;
    b = (color >> 0) & 0xff;

    // 直接操作 CCR 寄存器
    __HAL_TIM_SET_COMPARE(g_HPWM_ColorLED, CHANNEL_RED,   r*2000/255);
    __HAL_TIM_SET_COMPARE(g_HPWM_ColorLED, CHANNEL_GREEN, g*2000/255);
    __HAL_TIM_SET_COMPARE(g_HPWM_ColorLED, CHANNEL_BLUE,  b*2000/255);

}
/*********************************************************************
 * 函数名称： RGB_Set
 * 功能描述： 全彩LED设置颜色函数
 * 输入参数： r,g,b - 范围 0~100
						（共阳极，0最亮，这里进行了转换，变成了100最亮）
 * 输出参数： 无
 * 返 回 值： 无
 **********************************************************************/
void RGB_Set(uint8_t r,uint8_t g,uint8_t b)
{
		if(r > 100)r=0;
		else r = 100 - r;
	
		if(g > 100)g=0;
		else g = 100 - g;
	
		if(b > 100)r=0;
		else b = 100 - b;
	
    // 直接操作 CCR 寄存器
    __HAL_TIM_SET_COMPARE(g_HPWM_ColorLED, CHANNEL_RED,   r*2000/100);
    __HAL_TIM_SET_COMPARE(g_HPWM_ColorLED, CHANNEL_GREEN, g*2000/100);
    __HAL_TIM_SET_COMPARE(g_HPWM_ColorLED, CHANNEL_BLUE,  b*2000/100);

}

/*********************************************************************
 * 函数名称： ColorLED_Test
 * 功能描述： 全彩LED测试程序
 * 输入参数： 无
 * 输出参数： 无
 *            无
 * 返 回 值： 无
 **********************************************************************/
void ColorLED_Test(void)
{
    uint32_t color = 0;

    ColorLED_Init();

    while (1)
    {

        ColorLED_Set(color);

        color += 200000;
        color &= 0x00ffffff;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


