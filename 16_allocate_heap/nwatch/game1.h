#ifndef GAME1_H_
#define GAME1_H_

#include "typedefs.h"
#include "queue.h"


extern QueueHandle_t g_xQueuePlatform;

typedef enum { STATE_READY, STATE_PLAYING, STATE_GAMEOVER } GameState;
extern  GameState currentState;

//game初始化
void game1_init(void);
//给砖块一个初始状态（0 表示存在）
void game_init_objects(void);
// 声明游戏主任务函数，供 FreeRTOS 调用
void game1_task(void *params);

#endif /* GAME1_H_ */
