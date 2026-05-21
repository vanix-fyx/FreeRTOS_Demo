#include "iic.h"


// ------------------- GPIO模式切换函数（仅方案2需要） -------------------
// void IIC_SetSDA_Mode(uint32_t Mode)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};
//     GPIO_InitStruct.Pin = GPIO_PIN_SDA;
//     GPIO_InitStruct.Mode = Mode;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     HAL_GPIO_Init(IIC_GPIOx, &GPIO_InitStruct);
// }

// ------------------- IIC初始化（替换标准库GPIO初始化） -------------------
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 使能GPIOB时钟（HAL库方式）
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. 配置PB6(SCL)、PB7(SDA)为推挽输出
    GPIO_InitStruct.Pin = GPIO_PIN_SCL | GPIO_PIN_SDA;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;    // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;            // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // 高速
    HAL_GPIO_Init(IIC_GPIOx, &GPIO_InitStruct);

    // 3. 初始化为高电平
    IIC_SCL_H();
    IIC_SDA_H();
}

// ------------------- IIC起始信号（仅延时替换为udelay） -------------------
void IIC_Start(void)
{
    SDA_OUT();     // sda线输出
    IIC_SDA_H();
    IIC_SCL_H();
    udelay(4);
    IIC_SDA_L();   // START:当CLK是高电平时，DATA从高变低
    udelay(4);
    IIC_SCL_L();   // 钳住I2C总线，准备发送或接收数据
}

// ------------------- IIC停止信号（仅延时替换） -------------------
void IIC_Stop(void)
{
    SDA_OUT();     // sda线输出
    IIC_SCL_L();
    IIC_SDA_L();   // STOP:当CLK是高电平时，DATA从低变高
    udelay(4);
    IIC_SCL_H();
    IIC_SDA_H();   // 发送I2C总线结束信号
    udelay(4);
}

// ------------------- 等待应答信号（仅延时替换） -------------------
uint8_t IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    SDA_IN();      // SDA设置为输入
    IIC_SDA_H();
    udelay(1);
    IIC_SCL_H();
    udelay(1);
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_L();   // 时钟输出0
    return 0;
}

// ------------------- 产生ACK应答（仅延时替换） -------------------
void IIC_Ack(void)
{
    IIC_SCL_L();
    SDA_OUT();
    IIC_SDA_L();
    udelay(2);
    IIC_SCL_H();
    udelay(2);
    IIC_SCL_L();
}

// ------------------- 不产生ACK应答（仅延时替换） -------------------
void IIC_NAck(void)
{
    IIC_SCL_L();
    SDA_OUT();
    IIC_SDA_H();
    udelay(2);
    IIC_SCL_H();
    udelay(2);
    IIC_SCL_L();
}

// ------------------- IIC发送一个字节（仅延时替换） -------------------
void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    SDA_OUT();
    IIC_SCL_L();  // 拉低时钟开始数据传输
    for(t = 0; t < 8; t++)
    {
        // 输出当前位（最高位先送）
        (txd & 0x80) ? IIC_SDA_H() : IIC_SDA_L();
        txd <<= 1;
        udelay(2);  // 延时保留（对TEA5767等器件必须）
        IIC_SCL_H();
        udelay(2);
        IIC_SCL_L();
        udelay(2);
    }
}

// ------------------- 读1个字节（仅延时替换） -------------------
uint8_t IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN();  // SDA设置为输入
    for(i = 0; i < 8; i++)
    {
        IIC_SCL_L();
        udelay(2);
        IIC_SCL_H();
        receive <<= 1;
        if(READ_SDA) receive++;
        udelay(1);
    }
    if (!ack)
        IIC_NAck();  // 发送nACK
    else
        IIC_Ack();   // 发送ACK
    return receive;
}

// 扩展函数（如需实现，逻辑与标准库一致，仅IO/延时替换）
void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data)
{
    IIC_Start();
    IIC_Send_Byte(daddr);   // 发送器件写地址
    IIC_Wait_Ack();
    IIC_Send_Byte(addr);    // 发送寄存器地址
    IIC_Wait_Ack();
    IIC_Send_Byte(data);    // 发送数据
    IIC_Wait_Ack();
    IIC_Stop();
    udelay(200);
}

uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr)
{
    uint8_t res = 0;
    IIC_Start();
    IIC_Send_Byte(daddr);   // 发送器件写地址
    IIC_Wait_Ack();
    IIC_Send_Byte(addr);    // 发送寄存器地址
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(daddr+1); // 发送器件读地址
    IIC_Wait_Ack();
    res = IIC_Read_Byte(0); // 读取数据，发送nACK
    IIC_Stop();
    return res;
}
