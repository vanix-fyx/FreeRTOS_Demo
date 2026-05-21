//#include "main.h"
#include "ir_receiver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "timer.h"

#include "OLED.h"

/* 环形缓冲区: 用来保存解析出来的按键,可以防止丢失 */
#define BUF_LEN 128
static unsigned char g_KeysBuf[BUF_LEN];
static int g_KeysBuf_R, g_KeysBuf_W;

static uint64_t g_IRReceiverIRQ_Timers[68];
static int g_IRReceiverIRQ_Cnt = 0;
static uint32_t g_last_val;

/* 队列: 用来保存解析出来的按键,可以防止丢失 */
static QueueHandle_t g_xQueueIRReceiver;
static IRReceiver_Data_Struct IRReceiver_Data; 

#define NEXT_POS(x) ((x+1) % BUF_LEN)

/* 辅助函数 */

/**********************************************************************
 * 函数名称： GetQueueIR
 * 功能描述： 返回红外驱动程序里的队列句柄
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 队列句柄
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/09/02	     V1.0	  韦东山	      创建
 ***********************************************************************/
QueueHandle_t GetQueueIR(void)
{
	return g_xQueueIRReceiver;
}

/**********************************************************************
 * 函数名称： isKeysBufEmpty
 * 功能描述： 环形缓冲区是否空
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 1-空, 0-非空
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int isKeysBufEmpty(void)
{
	return (g_KeysBuf_R == g_KeysBuf_W);
}

/**********************************************************************
 * 函数名称： isKeysBufFull
 * 功能描述： 环形缓冲区是否满
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 1-满, 0-未满
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int isKeysBufFull(void)
{
	return (g_KeysBuf_R == NEXT_POS(g_KeysBuf_W));
}

/**********************************************************************
 * 函数名称： PutKeyToBuf
 * 功能描述： 把按键数据写入环形缓冲区
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static void PutKeyToBuf(unsigned char key)
{
	if (!isKeysBufFull())
	{
		g_KeysBuf[g_KeysBuf_W] = key;
		g_KeysBuf_W = NEXT_POS(g_KeysBuf_W);
	}
}

/**********************************************************************
 * 函数名称： GetKeyFromBuf
 * 功能描述： 从环形缓冲区读取按键数据
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0xff - 读不到数据, 其他值-按键值(device或key)
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static unsigned char GetKeyFromBuf(void)
{
	unsigned char key = 0xff;
	if (!isKeysBufEmpty())
	{
		key = g_KeysBuf[g_KeysBuf_R];
		g_KeysBuf_R = NEXT_POS(g_KeysBuf_R);
	}
	return key;
}

/**********************************************************************
 * 函数名称： IRReceiver_IRQTimes_Parse
 * 功能描述： 解析中断回调函数里记录的时间序列,得到的device和key放入队列
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, (-1) - 失败
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int IRReceiver_IRQTimes_Parse(void)
{
	uint64_t time;
	int i;
	int m, n;
	unsigned char datas[4];
	unsigned char data = 0;
	int bits = 0;
	int byte = 0;
	
	IRReceiver_Data_Struct i_data;
	
	/* 1. 判断前导码 : 9ms的低脉冲, 4.5ms高脉冲  */
	time = g_IRReceiverIRQ_Timers[1] - g_IRReceiverIRQ_Timers[0];
	if (time < 8000000 || time > 10000000)
	{
		return -1;
	}

	time = g_IRReceiverIRQ_Timers[2] - g_IRReceiverIRQ_Timers[1];
	if (time < 3500000 || time > 55000000)
	{
		return -1;
	}

	/* 2. 解析数据 */
	for (i = 0; i < 32; i++)
	{
		m = 3 + i*2;
		n = m+1;
		time = g_IRReceiverIRQ_Timers[n] - g_IRReceiverIRQ_Timers[m];
		data <<= 1;
		bits++;
		if (time > 1000000)
		{
			/* 得到了数据1 */
			data |= 1;
		}

		if (bits == 8)
		{
			datas[byte] = data;
			byte++;
			data = 0;
			bits = 0;
		}
	}

	/* 判断数据正误 */
	datas[1] = ~datas[1];
	datas[3] = ~datas[3];
	
	if ((datas[0] != datas[1]) || (datas[2] != datas[3]))
	{
        g_IRReceiverIRQ_Cnt = 0;
        return -1;
	}

//	PutKeyToBuf(datas[0]);
//	PutKeyToBuf(datas[2]);
	/* 写队列 */
	i_data.dev = datas[0];
	i_data.data = datas[2];
	xQueueSendToBackFromISR(g_xQueueIRReceiver,&i_data,NULL);
	
    return 0;
}

/**********************************************************************
 * 函数名称： isRepeatedKey
 * 功能描述： 解析中断回调函数里记录的时间序列,判断是否重复码
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 1 - 是, (0) - 不是
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int isRepeatedKey(void)
{
	uint64_t time;

	/* 1. 判断重复码 : 9ms的低脉冲, 2.25ms高脉冲  */
	time = g_IRReceiverIRQ_Timers[1] - g_IRReceiverIRQ_Timers[0];
	if (time < 8000000 || time > 10000000)
	{
		return 0;
	}

	time = g_IRReceiverIRQ_Timers[2] - g_IRReceiverIRQ_Timers[1];
	if (time < 2000000 || time > 2500000)
	{
		return 0;
	}	

	return 1;
}

/**********************************************************************
 * 函数名称： IRReceiver_IRQ_Callback
 * 功能描述： 红外接收器的中断回调函数,记录中断时刻
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
void IRReceiver_IRQ_Callback(void)
{
    uint64_t time;
    static uint64_t pre_time = 0;
		IRReceiver_Data_Struct i_data;
        
	/* 1. 记录中断发生的时刻 */	
	time = system_get_ns();
    
    /* 一次按键的最长数据 = 引导码 + 32个数据"1" = 9+4.5+2.25*32 = 85.5ms
     * 如果当前中断的时刻, 举例上次中断的时刻超过这个时间, 以前的数据就抛弃
     */
    if (time - pre_time > 100000000) 
    {
        g_IRReceiverIRQ_Cnt = 0;
    }
    pre_time = time;
    
	g_IRReceiverIRQ_Timers[g_IRReceiverIRQ_Cnt] = time;

	/* 2. 累计中断次数 */
	g_IRReceiverIRQ_Cnt++;

	/* 3. 次数达标后, 解析数据, 放入buffer */
	if (g_IRReceiverIRQ_Cnt == 4)
	{
		/* 是否重复码 */
		if (isRepeatedKey())
		{
			/* device: 0, val: 0, 表示重复码 */
//			PutKeyToBuf(0);
//			PutKeyToBuf(0);
			i_data.dev = 0;
			i_data.data = g_last_val;
			xQueueSendToBackFromISR(g_xQueueIRReceiver,&i_data,NULL);
			g_IRReceiverIRQ_Cnt = 0;
		}
	}
	if (g_IRReceiverIRQ_Cnt == 68)
	{
		IRReceiver_IRQTimes_Parse();
		g_IRReceiverIRQ_Cnt = 0;
	}
}


/**********************************************************************
 * 函数名称： IRReceiver_Init
 * 功能描述： 红外接收器的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
void IRReceiver_Init(void)
{
    /* PB10在MX_GPIO_Init()中已经被配置为双边沿触发, 并使能了中断 */
		/* 需要在cubemx上使能中断，勾选EXTI line [15:10] interrupts	*/
		/* IRReceiver_IRQ_Callback();需要把这个函数放在中断函数EXTI15_10_IRQHandler()里*/
#if 0
    /*Configure GPIO pin : PB10 */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
		g_xQueueIRReceiver = xQueueCreate(IRReceverQueueLength,sizeof(IRReceiver_Data_Struct));
}

/**********************************************************************
 * 函数名称： IRReceiver_Read
 * 功能描述： 红外接收器的读取函数
 * 输入参数： 无
 * 输出参数： pDev  - 用来保存设备ID
 *            pData - 用来保存按键码
 * 返 回 值： 0 - 成功, (-1) - 失败(无数据)
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
int IRReceiver_Read(IRReceiver_Data_Struct *rdata)
{
	if(pdPASS == xQueueReceive(g_xQueueIRReceiver,rdata,0 ))//portMAX_DELAY
		return 0;
	else 
		return -1;
}

/**********************************************************************
 * 函数名称： IRReceiver_CodeToString
 * 功能描述： 把接收到的按键码转换为按键名字
 * 输入参数： code - 按键码
 * 输出参数： 无
 * 返 回 值： NULL - 未识别的按键码; 其他值 - 按键名字
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/04	     V1.0	  韦东山	      创建
 ***********************************************************************/
const char *IRReceiver_CodeToString(uint8_t code)
{
    const uint8_t codes[]= {0xa2, 0xe2, 0x22, 0x02, 0xc2, 0xe0, 0xa8, 0x90, \
                            0x68, 0x98, 0xb0, 0x30, 0x18, 0x7a, 0x10, 0x38, \
                            0x5a, 0x42, 0x4a, 0x52, 0x00};
    const char *names[]= {"Power", "Menu", "Test", "+", "Return", "Left", "Play", "Right", \
                            "0", "-", "C", "1", "2", "3", "4", "5", \
                            "6", "7", "8", "9", "Repeat"};
    int i;
    
    for (i = 0; i < sizeof(codes)/sizeof(codes[0]); i++)
    {
        if (code == codes[i])
        {
            return names[i];
        }
    }
    return "Error";
}


/**********************************************************************
 * 函数名称： IRReceiver_Test
 * 功能描述： 红外接收器测试程序
 * 输入参数： 无
 * 输出参数： 无
 *            无
 * 返 回 值： 无
 * 修改日期        版本号     修改人        修改内容
 * -----------------------------------------------
 * 2023/08/04        V1.0     韦东山       创建
 ***********************************************************************/
void IRReceiver_Test(void)
{
		IRReceiver_Data_Struct rdata;
	
    IRReceiver_Init();

    while (1)
    {
        OLED_ShowString(1, 1, "IR Receiver: ");        
        OLED_ShowString(2, 1, "Device  Data");

        if (!IRReceiver_Read(&rdata))
        {
            OLED_ShowString(3, 1, "                ");
            OLED_ShowHexNum(3, 1, rdata.dev, 1);
            OLED_ShowHexNum(3, 9, rdata.data, 1);
            OLED_ShowString(4, 1, "                ");
            OLED_ShowString(4, 1, "Key name: ");
            OLED_ShowString(4, 11, (char*)IRReceiver_CodeToString(rdata.data));
        }
    }
}

void vIRReceiverTaskFunction(void* pvParameters)
{
    /* PB10在MX_GPIO_Init()中已经被配置为双边沿触发, 并使能了中断 */
		/* 需要在cubemx上使能中断，勾选EXTI line [15:10] interrupts	*/
		/* IRReceiver_IRQ_Callback();需要把这个函数放在中断函数EXTI15_10_IRQHandler()里*/
	
		for(;;)
		{
			
		}
}




