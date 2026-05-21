#include "main.h"
#include "game1.h"
#include "draw.h"      // 接入我们的绘图引擎
#include "resources.h" // 接入小球位图
#include "cmsis_os.h"  // 接入 FreeRTOS 的延时函数
#include <stdio.h>     // 用于 sprintf 打印分数

// 定义你遥控器对应的按键码 
#define IR_CODE_LEFT  0xe0 
#define IR_CODE_RIGHT 0x90
#define BLOCK_COUNT 10

// 游戏状态变量
static byte ballX, ballY; // 小球坐标
static int paddleX = 58,paddleY = 56; // 挡板位置
static char scoreStr[10]; // 分数缓存
static byte blocks[BLOCK_COUNT]; // 0表示存在，1表示消失

//增加状态机
typedef enum { STATE_READY, STATE_PLAYING, STATE_GAMEOVER } GameState;
static GameState currentState = STATE_READY;

// 游戏主任务实现
void game1_task(void *params)
{
    uint8_t dev, data;
    // 1. 初始化绘图引擎
    draw_init();
    // 2. 初始位置
		static int ballX = 64, ballY = 32;
    // 3. 速度变量
		static int velX = 2, velY = 1; 
		for(;;)
    {
			// --- 1. 读取红外数据 ---
			// IRReceiver_Read 返回 0 表示读取到了有效按键
			if (IRReceiver_Read(&dev, &data) == 0)
			{
					if (data == IR_CODE_LEFT && paddleX > 0)
					{
							paddleX -= 8; // 挡板左移
					}
					else if (data == IR_CODE_RIGHT && paddleX < (128 - 12))
					{
							paddleX += 8; // 挡板右移
					}
			}
			// --- A. 逻辑计算 ---
			ballX += velX;
			ballY += velY;

			// 撞击左右墙壁
			if (ballX <= 0 || ballX >= 126) velX = -velX;
			// 撞击天花板
			if (ballY <= 0) velY = -velY;
			// 掉落地面（暂时反弹，后期可以减命）
			if (ballY >= 62) velY = -velY; 

			// 如果球的 Y 坐标到达挡板高度，且 X 坐标在挡板范围内
			if (ballY >= paddleY && ballX >= paddleX && ballX <= (paddleX + 12)) 
			{
					velY = -velY; // 向上弹回
					ballY = paddleY;   // 防止卡在挡板里
			}
			// --- B. 绘图开始 ---
			draw_clearArea(0, 0, 128); // 清空上一帧
			
			// 画出小球 (参数：x, y, 图片, 宽, 高, 不反色, 无偏移)
			draw_bitmap(ballX, ballY, ballImg, 2, 2, false, 0);
			draw_bitmap(38, 36, block, 3, 8, false, 0);
			draw_bitmap(paddleX, paddleY, paddleImg, 12, 8, false, 0);
			
			// --- C. 刷新到硬件 ---
			draw_flushArea(0, 0, 128, 64);
			
			// --- D. 维持帧率 (约 30fps) ---
			vTaskDelay(33); 
    }
}

