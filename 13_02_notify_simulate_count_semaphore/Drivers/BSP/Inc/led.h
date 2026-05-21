#ifndef _LED_H
#define _LED_H

#include <stdint.h>


struct RGBCtrl { 
		uint8_t r;
		uint8_t g;
		uint8_t b;
};
extern struct RGBCtrl g_RGBCtrl;


void LED0_ON(void);
void LED0_OFF(void);
void LED0_Toggle(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED1_Toggle(void);



void ColorLED_Init(void);
void ColorLED_Set(uint32_t color);
void ColorLED_Test(void);
void RGB_Set(uint8_t r,uint8_t g,uint8_t b);

#endif
