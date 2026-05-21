#include "main.h"
#include <string.h>
#include <stdlib.h>
#include "draw.h"
#include "resources.h"
#include "OLED.h" // 包含你自己的底层驱动

// 1. 定义私有全局变量
static uint8_t g_oledBuffer[1024]; 
static uint8_t *oledBuffer = g_oledBuffer;

static uint32_t g_xres = 128;      // 屏幕宽度
static uint32_t g_yres = 64;       // 屏幕高度

// 2. 初始化函数
void draw_init(void)
{
    memset(oledBuffer, 0, 1024);
    
    // 调用你自己的硬件初始化
    OLED_Init(); 
}

// 3. 核心刷新函数：对接你的 OLED_Update
void draw_flushArea(byte x, byte y, byte w, byte h)
{
    if (oledBuffer != NULL)
    {
        // 调用你第一步写的那个底层函数
        OLED_Update(oledBuffer);
    }
}

// 4. 清除显存函数
void draw_clearArea(byte x, byte y, byte w)
{
    if (oledBuffer != NULL)
    {
        // 游戏简单化处理：每次擦除都清空整个 Buffer
        // 这样可以有效防止上一帧的残影
        memset(oledBuffer, 0, 1024);
    }
}

// 5. 结束绘图
void draw_end(void)
{
    // 暂时不需要处理
}
/**
 * @brief  在显存Buffer里画出一张位图
 * @param  x: 目标X坐标
 * @param  yy: 目标Y坐标
 * @param  bitmap: 指向位图数据的指针 (在resources.c定义)
 * @param  w: 图片宽度
 * @param  h: 图片高度
 * @param  invert: 是否反色显示
 * @param  offsetY: Y轴偏移量 (通常设为0)
 */
/**
 * @brief  适配 WDS 列取模资源的绘图函数
 * @note   外层循环扫列 (width)，内层循环扫行 (height/bit)
 */
void draw_bitmap(byte x, byte y, const byte* bitmap, byte w, byte h, bool invert, byte offsetY)
{
    // 0. 内存安全检查
    if (oledBuffer == NULL) return;

    // 1. 外层循环：遍历图片的每一列 (WDS 原版资源的排列方式)
    for (byte i = 0; i < w; i++) // 0 到 11 (如果是12像素宽)
    {
        // 每次取出一整列 (8像素) 的数据
        byte colData = bitmap[i]; 
        
        // 2. 内层循环：遍历这一列里的每一个像素位
        for (byte j = 0; j < 8; j++) // 0 到 7 位
        {
            // 3. 检查这一列的第 j 位是否为 1 (1亮, 0灭)
            bool pixel = (colData & (1 << j)) ? 1 : 0;
            if (invert) pixel = !pixel; // 应用反色逻辑

            if (pixel)
            {
                // 4. 计算资源坐标对应的实际显示的 Page 显存坐标
                uint16_t actualY = y + j + offsetY;
                
                // 5. 坐标边界检查 (128x64)
                if ((x + i) < 128 && actualY < 64)
                {
                    // 6. 将像素点写入 OLED 的 Page 模式显存 Buffer
                    oledBuffer[(x + i) + (actualY / 8) * 128] |= (1 << (actualY % 8));
                }
            }
        }
    }
}
