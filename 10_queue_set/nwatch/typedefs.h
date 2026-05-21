#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_

#include <stdbool.h>
#include <stdint.h>

// 1. 定义基本类型（韦老师代码里大量使用这些简写）
typedef uint8_t  byte;
typedef uint16_t uint;
typedef uint32_t ulong;

#define UPT_MOVE_NONE	0
#define UPT_MOVE_RIGHT	1
#define UPT_MOVE_LEFT	2


// 2. 定义 PROGMEM 为空 
// 原代码是在 AVR 单片机上写的，PROGMEM 用来把数据存在 Flash 里
// 在 STM32 上我们不需要这个关键字，所以把它定义为空，保证编译通过
#define PROGMEM

// 3. 快速循环宏（游戏逻辑里经常看到 LOOP(128, x) 这种写法）
#define LOOP(count, var) for(byte var=0; var<count; var++)

#endif /* TYPEDEFS_H_ */
