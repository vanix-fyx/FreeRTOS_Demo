
#ifndef __IIC_H
#define __IIC_H

#include "main.h"




// ------------------- IO方向设置（两种方案可选，推荐方案1：寄存器操作（简洁）） -------------------
// 方案1：直接操作寄存器（兼容标准库逻辑，简洁高效）
#define SDA_IN()  {GPIOB->CRL &= 0X0FFFFFFF; GPIOB->CRL |= (uint32_t)8<<28;}  // PB7输入模式
#define SDA_OUT() {GPIOB->CRL &= 0X0FFFFFFF; GPIOB->CRL |= (uint32_t)3<<28;}  // PB7输出模式

// 方案2：HAL库标准GPIO模式配置（更规范，需配合函数重初始化）
// #define SDA_IN()  IIC_SetSDA_Mode(GPIO_MODE_INPUT)
// #define SDA_OUT() IIC_SetSDA_Mode(GPIO_MODE_OUTPUT_PP)
// void IIC_SetSDA_Mode(uint32_t Mode);  // 声明模式切换函数

// ------------------- IO操作宏（替换PBout/PBin） -------------------
#define IIC_GPIOx	 GPIOB
#define GPIO_PIN_SCL GPIO_PIN_6
#define GPIO_PIN_SDA GPIO_PIN_7

#define IIC_SCL_H()  HAL_GPIO_WritePin(IIC_GPIOx, GPIO_PIN_SCL, GPIO_PIN_SET)
#define IIC_SCL_L()  HAL_GPIO_WritePin(IIC_GPIOx, GPIO_PIN_SCL, GPIO_PIN_RESET)
#define IIC_SDA_H()  HAL_GPIO_WritePin(IIC_GPIOx, GPIO_PIN_SDA, GPIO_PIN_SET)
#define IIC_SDA_L()  HAL_GPIO_WritePin(IIC_GPIOx, GPIO_PIN_SDA, GPIO_PIN_RESET)
#define READ_SDA     HAL_GPIO_ReadPin(IIC_GPIOx, GPIO_PIN_SDA)

// IIC核心函数声明
void IIC_Init(void);                // 初始化IIC的IO口
void IIC_Start(void);               // 发送IIC开始信号
void IIC_Stop(void);                // 发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);         // IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack);// IIC读取一个字节
uint8_t IIC_Wait_Ack(void);              // IIC等待ACK信号
void IIC_Ack(void);                 // IIC发送ACK信号
void IIC_NAck(void);                // IIC不发送ACK信号

// 扩展函数（原声明保留）
void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);



#endif













