#include "OLED.h"
#include "OLED_Font.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"

/*引脚配置*/
#define OLED_W_SCL(x)    HAL_GPIO_WritePin(OLED_GPIOx, OLED_PIN_SCL, (GPIO_PinState)(x))
#define OLED_W_SDA(x)    HAL_GPIO_WritePin(OLED_GPIOx, OLED_PIN_SDA, (GPIO_PinState)(x))

#if OLEDTaskFunction_ENABLE
/* 任务句柄创建 */
StaticTask_t xOLEDTaskTCB;	//OLED静态任务控制块（TCB）
StackType_t xOLEDTaskStack[128];	//任务栈空间数组
TaskHandle_t xOLEDTask_Handle = NULL;	// OLED任务句柄
#endif 

void vOLEDTaskFunction(void* pvParameters)
{
	OLED_Init();
//	OLED_Clear();
	for(;;)
	{
//		OLED_ShowString(1,1,"25261225918 fyx");
	}
}



/*引脚初始化*/
void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};  // HAL库建议初始化结构体

    OLED_GPIO_CLK_ENABLE();  // HAL库时钟使能（替代RCC_APB2PeriphClockCmd）

    // 配置SCL引脚
    GPIO_InitStructure.Pin = OLED_PIN_SCL;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出（替代GPIO_Mode_Out_OD）
    GPIO_InitStructure.Pull = GPIO_NOPULL;          // 无上下拉（HAL库新增）
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; // 高速（替代GPIO_Speed_50MHz）
    HAL_GPIO_Init(OLED_GPIOx, &GPIO_InitStructure);

    // 配置SDA引脚
    GPIO_InitStructure.Pin = OLED_PIN_SDA;
    HAL_GPIO_Init(OLED_GPIOx, &GPIO_InitStructure);

    // 初始电平置高
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置低4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置高4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
 * @brief  OLED 清除指定行
 * @param  line: 要清除的行（页地址），范围 0~7
 * @retval 无
 */
void OLED_ClearLine(uint8_t line)
{
	uint8_t i, j;
	for (j = (line-1)*2; j < (line-1)*2+2; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column,char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}
/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNumNoLength(uint8_t Line, uint8_t Column, uint32_t Number)
{
	char buf[16];

	// sprintf 的返回值就是这个数字转成字符串后的长度,buf 里面存的是数字Number转化的字符串
	sprintf(buf, "%d", Number);
	
	OLED_ShowString(Line,Column,buf);
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
	
#if OLEDTaskFunction_ENABLE
  /*创建任务：OLED*/
  xOLEDTask_Handle = xTaskCreateStatic(vOLEDTaskFunction,"OLEDTask",128,NULL,osPriorityNormal,
		  xOLEDTaskStack,&xOLEDTaskTCB);
#endif
}

/**
 * @brief  全屏更新函数：将内存中的显存数组一次性刷入OLED硬件
 * @param  Buffer: 指向1024字节显存数组的指针 (128x64像素 / 8)
 * @retval 无
 */
void OLED_Update(uint8_t *Buffer)
{
	uint8_t i, j;
	for (i = 0; i < 8; i++) // OLED分为8页（每页8行像素）
	{
		/* 1. 设置页地址 */
		OLED_WriteCommand(0xB0 + i); 
		/* 2. 设置列地址起始为0 */
		OLED_WriteCommand(0x00);     // 设置起始列低4位
		OLED_WriteCommand(0x10);     // 设置起始列高4位
		
		/* 3. 连续写入128列的像素数据 */
		for (j = 0; j < 128; j++)
		{
			// 发送 Buffer 中对应位置的一个字节（8个垂直像素点）
			OLED_WriteData(Buffer[j + i * 128]); 
		}
	}
}

/**
  * @brief  将字符点阵写入指定的显存缓冲区
  * @param  Buffer: 指向目标显存缓冲区的指针 (1024字节)
  * @param  x: 缓冲区横坐标 (0~127)
  * @param  y: 缓冲区页坐标 (0~7，对应垂直方向每8像素一页)
  * @param  Char: 要写入的 ASCII 字符
  */
void OLED_DrawBuffChar(uint8_t *Buffer, uint8_t x, uint8_t y, char Char)
{
    uint8_t i;
    
    // 1. 字符上半部分处理 (8像素高)
    for (i = 0; i < 8; i++)
    {
        // 确保 x 坐标不越界，且当前页 y 在有效范围内
        if (x + i < 128 && y < 8)
        {
            // 将字库上半部分数据写入传入的 Buffer
            Buffer[x + i + (y * 128)] = OLED_F8x16[Char - ' '][i];
        }
    }
    
    // 2. 字符下半部分处理 (接下来的8像素高)
    for (i = 0; i < 8; i++)
    {
        // 字符高16像素，下半部分在 y+1 页
        if (x + i < 128 && (y + 1) < 8)
        {
            // 将字库下半部分数据写入传入的 Buffer
            Buffer[x + i + ((y + 1) * 128)] = OLED_F8x16[Char - ' '][i + 8];
        }
    }
}

/**
  * @brief  将字符串写入指定的显存缓冲区
  * @param  Buffer: 指向目标显存缓冲区的指针
  * @param  x: 起始横坐标
  * @param  y: 起始页坐标
  * @param  String: 要显示的字符串
  */
void OLED_DrawBuffString(uint8_t *Buffer, uint8_t x, uint8_t y, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        // 每个字符宽8像素，传入同一个 Buffer 指针
        OLED_DrawBuffChar(Buffer, x + (i * 8), y, String[i]);
    }
}

