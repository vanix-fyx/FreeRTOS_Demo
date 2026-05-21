#include "main.h"
#include "resources.h"


// PROGMEM 在 typedefs.h 里被我们定义为空了，所以这里不会报错
// 这是一个 2x2 像素的小球数据
const byte ballImg[] PROGMEM = {
	0x03,0x03,
};

// 这是一个 3x8 像素的砖块数据
const byte block[] PROGMEM = {
    0x07,0x07,0x07,
};

// 这是一个 12x2 像素的挡板数据（按线性流计算，24个点需要3个字节全亮）
const byte paddleImg[] PROGMEM = { 
	0x60,0x70,0x50,0x10,0x30,0xF0,0xF0,0x30,0x10,0x50,0x70,0x60,
};
