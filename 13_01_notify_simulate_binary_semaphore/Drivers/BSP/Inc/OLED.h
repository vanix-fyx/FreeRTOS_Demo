#ifndef __OLED_H
#define __OLED_H
//#include "main.h"
#include <stdint.h>

#define OLEDTaskFunction_ENABLE 0

#define OLED_GPIOx	 GPIOB
#define OLED_PIN_SCL GPIO_PIN_6
#define OLED_PIN_SDA GPIO_PIN_7
#define OLED_GPIO_CLK_ENABLE()  __HAL_RCC_##GPIOB##_CLK_ENABLE()

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ClearLine(uint8_t line);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowNumNoLength(uint8_t Line, uint8_t Column, uint32_t Number);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

void OLED_Update(uint8_t *Buffer);
void OLED_DrawBuffChar(uint8_t *Buffer, uint8_t x, uint8_t y, char Char);
void OLED_DrawBuffString(uint8_t *Buffer, uint8_t x, uint8_t y, char *String);


void vOLEDTaskFunction(void* pvParameters);

#endif
