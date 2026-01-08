#include "i2c_peripheral.h"

/**
 *@brief 初始化I2C1外设
 */
void initI2c1(void)
{
	// 1. 开启I2C1和对应的GPIO的时钟(SCL-PB6, SDA-PB7)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	
	// 2. 复用GPIO功能
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
	
	// 3. 配置GPIO工作模式
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// 4. 配置I2C1工作模式
	I2C_InitTypeDef I2C_InitStruct;
	I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = 0;
	I2C_Init(I2C1, &I2C_InitStruct);
	
	I2C_Cmd(I2C1, ENABLE);
}

/**
 *@brief 指定地址写一个字节
 *@param reg_addr 一个16位的寄存器地址
 *@param data 一个8位的数据
 */
void writeOneByteByI2c1(uint16_t reg_addr, uint8_t data)
{
	// 1. 产生起始条件
	I2C_GenerateSTART(I2C1, ENABLE);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	// 2. 发送从机地址和写位
	I2C_Send7bitAddress(I2C1, BL24C512A_ADDR, I2C_Direction_Transmitter);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
	
	// 3. 写入数据——寄存器地址
	I2C_SendData(I2C1, (reg_addr >> 8));
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	I2C_SendData(I2C1, reg_addr);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	
	// 4. 写入数据——具体数据
	I2C_SendData(I2C1, data);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS);
	
	// 5. 终止时序
	I2C_GenerateSTOP(I2C1, ENABLE);
}

/**
 *@brief 指定地址写如多个字节
 *@param reg_addr 一个16位的寄存器地址
 *@param data 要写入的8位数据
 *@param len 要写入的字节数
 */
void writeMulByteByI2c1(uint16_t reg_addr, uint8_t data[], uint8_t len)
{
	uint8_t i = 0;
	// 1. 产生起始条件
	I2C_GenerateSTART(I2C1, ENABLE);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	// 2. 发送从机地址和写位
	I2C_Send7bitAddress(I2C1, BL24C512A_ADDR, I2C_Direction_Transmitter);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
	
	// 3. 写入数据——寄存器地址
	I2C_SendData(I2C1, (reg_addr >> 8));
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	I2C_SendData(I2C1, reg_addr);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	
	// 4. 写入数据——具体数据
	for(i=0; i<len; i++)
	{
		I2C_SendData(I2C1, data[i]);
		while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS);
	}
		
	// 5. 终止时序
	I2C_GenerateSTOP(I2C1, ENABLE);
}

/**
 *@brief 指定地址读一个字节
 *@param reg_addr 一个16位的寄存器地址
 *@return 读取的字节
 */
uint8_t readOneByteByI2c1(uint16_t reg_addr)
{
	uint8_t rx_data;
	
	// 1. 产生起始条件
	I2C_GenerateSTART(I2C1, ENABLE);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	// 2. 发送从机地址和写位
	I2C_Send7bitAddress(I2C1, BL24C512A_ADDR, I2C_Direction_Transmitter);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
	
	// 3. 写入数据——寄存器地址
	I2C_SendData(I2C1, (reg_addr >> 8));
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	I2C_SendData(I2C1, reg_addr);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	
	// 4. 生成重复起始条件
	I2C_GenerateSTART(I2C1, ENABLE);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	// 5. 发送从机地址和读位
	I2C_Send7bitAddress(I2C1, BL24C512A_ADDR, I2C_Direction_Receiver);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);
	
	// 6. 设置ACK=0不给应答，因为这只是一个字节
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	
	// 7.终止时序
	I2C_GenerateSTOP(I2C1, ENABLE);
	
	// 8. 等待EV7事件
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
	
	// 9. 读取接收的数据
	rx_data = I2C_ReceiveData(I2C1);
	
	return rx_data;
}

/**
 *@brief 指定地址读多个字节
 *@param reg_addr 一个16位的寄存器地址
 *@param data 传出参数，读取到的多个字节
 *@param len 读取的字节数
 */
void readMulByteByI2c1(uint16_t reg_addr, uint8_t data[], uint8_t len)
{
	uint8_t i = 0;
	
	// 1. 产生起始条件
	I2C_GenerateSTART(I2C1, ENABLE);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	// 2. 发送从机地址和写位
	I2C_Send7bitAddress(I2C1, BL24C512A_ADDR, I2C_Direction_Transmitter);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) != SUCCESS);
	
	// 3. 写入数据——寄存器地址
	I2C_SendData(I2C1, (reg_addr >> 8));
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	I2C_SendData(I2C1, reg_addr);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING) != SUCCESS);
	
	// 4. 生成重复起始条件
	I2C_GenerateSTART(I2C1, ENABLE);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS);
	
	// 5. 发送从机地址和读位
	I2C_Send7bitAddress(I2C1, BL24C512A_ADDR, I2C_Direction_Receiver);
	while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) != SUCCESS);
	
	// 6. 读取数据
	for(i=0; i<len; i++)
	{
		if(i == len-1)
		{
			// 设置ACK=0不给应答，因为这只是一个字节
			I2C_AcknowledgeConfig(I2C1, DISABLE);
		}
		// 等待EV7事件
		while(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS);
		// 读取接收的数据
		data[i] = I2C_ReceiveData(I2C1);
	}
	
	// 7.终止时序
	I2C_GenerateSTOP(I2C1, ENABLE);
	
}
	

