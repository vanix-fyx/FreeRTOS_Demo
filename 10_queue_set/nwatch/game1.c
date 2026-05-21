#include "main.h"
#include "game1.h"
#include "draw.h"      // 接入我们的绘图引擎
#include "resources.h" // 接入小球位图
#include "typedefs.h"
#include "cmsis_os.h"  // 接入 FreeRTOS 的延时函数
#include <stdio.h>     // 用于 sprintf 打印分数

#include "ir_receiver.h"
#include "rotary_encoder.h"


/* 砖块 */
#define BLOCK_ROWS    4    // 4排
#define BLOCK_COLS    32   // 每排32个（128宽 / 间距4 = 32）
#define TOTAL_BLOCKS (BLOCK_ROWS * BLOCK_COLS)

/* 坐标&区域变量 */
static float ballX=40, ballY=40; // 小球坐标
static int platformX = 58,platformY = 56; // 挡板位置
static byte blocks[TOTAL_BLOCKS]; // 砖块区域：0表示存在，1表示消失
static float velX = 0.5;      // 小球水平速度
static float velY = 0.5;      // 小球垂直速度

/* 游戏状态变量 */
static char scoreStr[10]; // 分数缓存
static int g_score = 0; // 全局分数变量
static int g_lives = 3;  // 初始生命值为3

/* 定义挡板移动状态 */
static uint8_t g_platformMoveDir = UPT_MOVE_NONE; // 挡板当前移动意图

/* 增加状态机 */
GameState currentState = STATE_PLAYING;

/* 队列集输入队列 & 队列集元素 */
static QueueSetHandle_t g_xQueueSetInput;	//输入队列集
static QueueHandle_t g_xQueueIRReceiver;	//红外队列
static QueueHandle_t g_xQueueRotary;			//旋转编码器队列
/* 挡球板队列 */
QueueHandle_t g_xQueuePlatform;



//给砖块一个初始状态（0 表示存在）
void game_init_objects(void) 
{
    for (int i = 0; i < TOTAL_BLOCKS; i++) 
		{
        blocks[i] = 0; // 0 表示砖块还在
    }
    g_score = 0;
		g_lives = 3;
		ballX=40;
		ballY=40;
		velX = 0.5;      // 小球水平速度
		velY = 0.5;      // 小球垂直速度
		g_platformMoveDir = UPT_MOVE_NONE;// 挡板当前移动意图
}
//挡球板任务
void Platform_task(void *params)
{
    while (1)
    {
				/* 读挡球板队列 */
				if(pdPASS == xQueueReceive(g_xQueuePlatform,&g_platformMoveDir,portMAX_DELAY))
						if (currentState == STATE_PLAYING)
						{
							if(g_platformMoveDir == UPT_MOVE_LEFT)
							{
                if (platformX > 8) platformX -= 4;
								else platformX = 0;
							}
							else if(g_platformMoveDir == UPT_MOVE_RIGHT)
							{
                if (platformX < (128 - 20)) platformX += 4;
								else platformX = 128 - 12;
							}
						}
    }
}
void IRReceiverProcess(void)
{
		IRReceiver_Data_Struct rdata;
		static uint8_t platform_idata;
	
		// 1. 读取红外数据
		if(pdPASS == xQueueReceive(g_xQueueIRReceiver,&rdata,0))
		{				
				// 2. 根据按键值更新坐标
				switch(rdata.data)
				{
					case 0x00:	// 不变
											break;
					case 0xe0:	// 左移
											platform_idata = UPT_MOVE_LEFT;
//											xQueueSend(g_xQueuePlatform,&platform_idata,0);
											break;
					case 0x90:	// 右移
											platform_idata = UPT_MOVE_RIGHT;
//											xQueueSend(g_xQueuePlatform,&platform_idata,0);
											break;
					case 0xa8:	// 暂停Play
											if(currentState == STATE_PLAYING)
											{
													rdata.data = 0;
													platform_idata = UPT_MOVE_NONE;
//													xQueueSend(g_xQueuePlatform,&platform_idata,0);
													currentState = STATE_READY;
											}
											else if(currentState == STATE_READY)
											{
													rdata.data = 0;
													platform_idata = UPT_MOVE_NONE;
//													xQueueSend(g_xQueuePlatform,&platform_idata,0);
													currentState = STATE_PLAYING;
											}
											break;
					case 0x22:	 // 重置Test	
											platform_idata = UPT_MOVE_NONE;
											currentState = STATE_PLAYING;
											draw_init();
											game_init_objects();
											break;
					default:	
											platform_idata = UPT_MOVE_NONE;
//										xQueueSend(g_xQueuePlatform,&platform_idata,0);
										break;
			}
			/* 写挡球板队列 */
			xQueueSend(g_xQueuePlatform,&platform_idata,0);				
		}
}
void ProcessRotaryData(void)
{
		int cnt,left;
		Rotary_Data rdata;
		uint8_t idata;
	
		/* 读旋转编码器队列 */
		if(pdPASS == xQueueReceive(g_xQueueRotary,&rdata,0))
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
			xQueueSend(g_xQueuePlatform,&idata,0);
		}
}
//输入任务
void Input_task(void *params)
{
		QueueSetMemberHandle_t xQueueHandle;
    while (1)
    {
			/* 读取队列集，获得队列句柄 */
			xQueueHandle = xQueueSelectFromSet(g_xQueueSetInput,portMAX_DELAY);
			
			/* 读取队列，获得队列数据 */
			if(xQueueHandle)
			{
				if(xQueueHandle == g_xQueueIRReceiver)
				{
					/* 处理红外数据，写入挡球板队列 */
					IRReceiverProcess();
				}
				else if(xQueueHandle == g_xQueueRotary)
				{
					/* 处理旋转编码器数据，写入挡球板队列 */
					ProcessRotaryData();
				}
			}			
    }
}
//STATE_GAMEOVER状态执行函数
void game1_over_function(void)
{
	if (currentState == STATE_GAMEOVER)
	{
		// --- 渲染结束画面 ---
		draw_clearArea(0, 0, 128);
		
		draw_string(2, 4, "Game Over"); 
		sprintf(scoreStr, "Final Score:%d", g_score);
		draw_string(4, 2, scoreStr); 
		
		draw_flushArea(0, 0, 128, 64);
		
	}
}
//STATE_PLAYING状态执行函数
void game1_play_function(void)
{
	if (currentState == STATE_PLAYING)
	{
						/* --- A. 边框判定 --- */
		ballX += velX;
		ballY += velY;

		// 1. 左右墙壁反弹 (保持现状)
		if (ballX <= 0 || ballX >= 126) velX = -velX;

		// 2. 天花板反弹 (保持现状)
		if (ballY <= 17) velY = -velY;

		// 3. 底部反弹 (这是你要修的地方)
		if (ballY >= 62) 
		{
			velY = -velY;  // 直接反弹，不重置位置
			ballY = 62;    // 坐标纠偏，防止粘连
			
			if (g_lives > 0) 
			{
				g_lives--; // 扣除一颗心
			}
			
			if (g_lives <= 0)
			{
				currentState = STATE_GAMEOVER; // 没命了，切换到结束状态
			}
		 }				

					/* --- B. 碰撞逻辑：检查小球是否碰到 platformX --- */
		if (ballY >= platformY && ballY <= platformY+8)
		{
				if (ballX >= platformX && ballX <= (platformX + 12))
				{
					velY = -velY; // 反弹
					ballY = platformY-1;
				}
		}
		
		/* --- 死亡判定 --- */
		if (ballY > 64) // 小球掉出屏幕底端
		{
				g_lives--; // 生命值减 1
				
				if (g_lives > 0)
				{
						// 还有命：重置球的位置到挡板上方，继续游戏
						ballX = platformX + 4; 
						ballY = 52;
						velY = -1; // 让球向上弹起
				}
				else
				{
						// 没命了：进入游戏结束状态
						currentState = STATE_GAMEOVER;
				}
		}
		
		/* --- 砖块碰撞检测 (适配 3x3 砖块, 间隔1) --- */
		// 1. 判定是否进入砖块总区域 (y 从 17 到 17 + 4*4 = 33)
		if (ballY >= 17 && ballY <= 33) 
		{
				// 2. 计算小球撞到了哪一行和哪一列
				int row = (ballY - 17) / 4;
				int col = ballX / 4;

				// 3. 安全检查
				if (row >= 0 && row < BLOCK_ROWS && col >= 0 && col < BLOCK_COLS) 
				{
						int idx = row * BLOCK_COLS + col;
						if (blocks[idx] == 0) // 如果砖块还在
						{
								blocks[idx] = 1;  // 砖块消失
								velY = -velY;     // 反弹
								g_score += 1;    // 加分

								// 4. 坐标纠偏：立刻将球移出当前砖块行，防止“粘连”
								if (velY > 0) 
										ballY = 17 + (row + 1) * 4; // 向下弹，移到下一行起始位置
								else 
										ballY = 17 + row * 4 - 3;   // 向上弹，移到当前行上方
						}
				}
		}	
				        /* --- C. 渲染逻辑：把画面刷出来 --- */
		draw_clearArea(0, 0, 128);
		
		// 1. 绘制爱心生命值 (放在右上角)
		int heartStartX = 128 - (g_lives * 8) - 2; // 计算起始X坐标：总宽 - (生命*间隔) - 右边距
		int heartY = 2; // 顶部边距

		for (int i = 0; i < g_lives; i++)
		{
				// 如果你选了 7x6 的 heartImg：
				// x = 起始X + i*间距8 (7像素宽+1间隔)
				draw_bitmap(heartStartX + i * 8, heartY, heartImg, 7, 6, false, 0); 
		}

		// 绘制分数
		sprintf(scoreStr, "%d", g_score);
		// 注意：这里需要调用你驱动里的字符串显示函数，例如：
		draw_string(0, 0, scoreStr); 
		
		// 2. 绘制砖块 (在屏幕顶端排列)
		for (int row = 0; row < BLOCK_ROWS; row++) 
		{
			for (int col = 0; col < BLOCK_COLS; col++) 
			{
				int idx = row * BLOCK_COLS + col;
				if (blocks[idx] == 0) 
				{
						// x = 列 * 4
						// y = 17 + 行 * 4 (每排纵向间隔4像素)
						draw_bitmap(col * 4, 17 + row * 4, blockImg, 3, 8, false, 0); 
				}
			}
		}
		
		// 画球
		draw_bitmap(ballX, ballY, ballImg, 2, 2, false, 0);
		// 画挡板 (注意：这里的 platformX 会被另一个任务自动更新)
		draw_bitmap(platformX, platformY, platformImg, 12, 8, false, 0);
		
		
		// 局部或全局刷新
		draw_flushArea(0, 0, 128, 64);
	}
}
// 游戏主任务实现
void game1_task(void *params)
{	
    draw_init();
		game_init_objects();
	
		g_xQueueSetInput = xQueueCreateSet(IRReceverQueueLength + RotaryQueueLength);
		g_xQueueIRReceiver = GetQueueIR();
		g_xQueueRotary = GetQueueRotary();
		
		xQueueAddToSet(g_xQueueIRReceiver,g_xQueueSetInput);
		xQueueAddToSet(g_xQueueRotary,g_xQueueSetInput);
		
		g_xQueuePlatform = xQueueCreate(10,sizeof(uint8_t));
	
		xTaskCreate(Platform_task, "PlatformTask", 128, NULL, osPriorityNormal, NULL);
		xTaskCreate(Input_task, "InputTask", 128, NULL, osPriorityNormal, NULL);
	
    while (1)
    {
			game1_play_function();
			game1_over_function();
			vTaskDelay(50); // 维持约 33 帧
    }
}




