#ifndef DRAW_H_
#define DRAW_H_

#include "typedefs.h"

// --- 基础控制函数 ---
void draw_init(void);    // 初始化显存和屏幕
void draw_end(void);     // 结束绘图（通常留空）

// --- 核心绘图函数 ---
// 参数：x坐标, y坐标, 位图数据, 宽, 高, 是否反色, Y轴偏移
void draw_bitmap(byte x, byte yy, const byte* bitmap, byte w, byte h, bool invert, byte offsetY);

// --- 刷新与清除 ---
// 局部刷新：将Buffer中特定区域刷向硬件
void draw_flushArea(byte x, byte y, byte w, byte h);
// 清除区域：用于擦除旧的图像轨迹
void draw_clearArea(byte x, byte y, byte w);

// --- 文字显示（后续可选） ---
void draw_string(uint8_t x, uint8_t y, char *str);

#endif /* DRAW_H_ */
