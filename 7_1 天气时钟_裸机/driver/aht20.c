#include "aht20.h"
#include "systick_delay.h"


void initI2c2(void);
uint8_t checkI2cEvent(uint32_t I2C_EVENT, uint8_t time_out_ms);
uint8_t writeMulByteByI2c2(uint8_t data[], uint8_t len);
uint8_t readMulByteByI2c2(uint8_t data[], uint8_t len);
uint8_t readStateOfAht20(uint8_t* p_state);
uint8_t isCalibrateOfAht20(void);
uint8_t isBusyOfAht20(void);
uint8_t calibrateAht20(uint8_t time_out_ms);


/**
 *@brief 初始化AHT20温湿度器
 *@return 1-初始化成功，0-初始化失败
 */
uint8_t initAht20(void)
{
	// 1. 初始化I2C2外设，用于MCU与AHT20通信
	initI2c2();
	
	// 2. 上电后要等待40ms
	delayMsUseSystick(40);
	
	// 3. 读取状态字，看状态字的校准使能位Bit[3]是否为1
	if(isCalibrateOfAht20() == 0)
	{
		// 校准AHT20(超时时间50ms)
		if(calibrateAht20(50) == 0)	// 校准失败
			return 0;
	}
	
	return 1;
}

/**
 *@brief 初始化I2C2外设
 */
void initI2c2(void)
{
	// 1. 开启I2C2和对应的GPIO的时钟(SCL-PB10, SDA-PB11)（board.c文件）
	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	// RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	
	// 2. 复用GPIO功能
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);
	
	// 3. 配置GPIO工作模式
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// 4. 配置I2C2工作模式
	I2C_InitTypeDef I2C_InitStruct;
	I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = 0;
	I2C_Init(I2C2, &I2C_InitStruct);
	
	I2C_Cmd(I2C2, ENABLE);
}

/**
 *@brief I2C时序超时校验事件函数
 *@param I2C_EVENT 要写入的8位数据
 *@param time_out_ms 超时退出的时间（单位：ms）
 *@return 1-成功检测到事件，0-失败并超时退出
 */
uint8_t checkI2cEvent(uint32_t I2C_EVENT, uint8_t time_out_ms)
{
	while((I2C_CheckEvent(I2C2, I2C_EVENT) != SUCCESS) && (time_out_ms > 0))
	{
		time_out_ms--;
		delayMsUseSystick(1);
	}
	return (time_out_ms > 0) ? 1 : 0;
}

/**
 *@brief 指定地址写入多个字节
 *@param data 要写入的8位数据
 *@param len 要写入的字节数
 *@return 1-写入成功，0-写入失败
 */
uint8_t writeMulByteByI2c2(uint8_t data[], uint8_t len)
{
	uint8_t i = 0;
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	// 1. 产生起始条件
	I2C_GenerateSTART(I2C2, ENABLE);
	if(checkI2cEvent(I2C_EVENT_MASTER_MODE_SELECT, 10) == 0) return 0;
	
	// 2. 发送从机地址和写位
	I2C_Send7bitAddress(I2C2, AHT20_ADDR, I2C_Direction_Transmitter);
	if(checkI2cEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, 10) == 0) return 0;
	
	// 4. 写入数据——具体数据
	for(i=0; i<len; i++)
	{
		I2C_SendData(I2C2, data[i]);
		if(checkI2cEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING, 10) == 0) return 0;
	}
		
	// 5. 终止时序
	I2C_GenerateSTOP(I2C2, ENABLE);
	
	return 1;
}

/**
 *@brief 指定地址读多个字节
 *@param data 传出参数，读取到的多个字节
 *@param len 读取的字节数
 *@return 1-读取成功，0-读取失败
 */
uint8_t readMulByteByI2c2(uint8_t data[], uint8_t len)
{
	uint8_t i = 0;
	
	// 1. 生成起始条件
	I2C_GenerateSTART(I2C2, ENABLE);
	if(checkI2cEvent(I2C_EVENT_MASTER_MODE_SELECT, 10) == 0) return 0;
	
	// 2. 发送从机地址和读位
	I2C_Send7bitAddress(I2C2, AHT20_ADDR, I2C_Direction_Receiver);
	if(checkI2cEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, 10) == 0) return 0;
	
	// 3. 读取数据
	for(i=0; i<len; i++)
	{
		I2C_AcknowledgeConfig(I2C2, (i != len-1) ? ENABLE : DISABLE);
		// 等待EV7事件
		if(checkI2cEvent(I2C_EVENT_MASTER_BYTE_RECEIVED, 10) == 0) return 0;
		// 读取接收的数据
		data[i] = I2C_ReceiveData(I2C2);
	}
	
	// 4.终止时序
	I2C_GenerateSTOP(I2C2, ENABLE);
	
	return 1;
}

/**
 *@brief 读取状态寄存器
 *@note1 通过发送0x71可以获取一个字节的状态字
 *@param p_state 读取到的状态寄存器
 *@return 1-读取成功，0-读取失败
 */
uint8_t readStateOfAht20(uint8_t* p_state)
{
	uint8_t cmd = 0x71;
	// 首先发送指令0x71，告诉AHT20我接下来要读取状态寄存器了
	if(writeMulByteByI2c2(&cmd, 1) == 0)
		return 0;
	
	// 接着，开始读取AHT20通过I2C2传过来的状态寄存器值
	if(readMulByteByI2c2(p_state, 1) == 0)
		return 0;
	else
		return 1;
}

/**
 *@brief 读取状态寄存器，查看是否校验
 *@return 1-已校准，0-未校准
 */
uint8_t isCalibrateOfAht20(void)
{
	uint8_t state;
	// 读取状态寄存器
	if(readStateOfAht20(&state) == 0)
		return 0;
	// 判断是否校准
	return (state & 0x08) > 0 ? 1 : 0;
}

/**
 *@brief 读取状态寄存器，查看是否处于忙状态
 *@return 1-设备忙，处于测量状态，0-设备闲，处于休眠状态
 */
uint8_t isBusyOfAht20(void)
{
	uint8_t state;
	// 读取状态寄存器
	if(readStateOfAht20(&state) == 0)
		return 0;
	// 判断AHT20状态（忙状态标志位Bit[7]）
	return (state & 0x80) > 0 ? 1 : 0;
}

/**
 *@brief 校准AHT20
 *@note1 发送0xBE命令,此命令参数有两个字节，第一个字节为0x08，第二个字节为0x00。
 *@param time_out_ms 超时退出的时间（单位：ms）
 *@return 1-校准成功，0-校准失败
 */
uint8_t calibrateAht20(uint8_t time_out_ms)
{
	// 发送校准命令
	if(writeMulByteByI2c2((uint8_t[]){0xBE, 0x08, 0x00}, 3) == 0)
		return 0;
	// 等待校准完成
	while((isCalibrateOfAht20() == 0) && (time_out_ms > 0))
	{
		time_out_ms--;
		delayMsUseSystick(1);
	}
	return (time_out_ms > 0) ? 1 : 0;
}

/**
 *@brief 触发AHT20测量
 *@note1 直接发送 0xAC命令(触发测量)，此命令参数有两个字节，第一个字节为 0x33，第二个字节为0x00
 *@param time_out_ms 超时退出的时间（单位：ms）
 *@return 1-触发成功，0-触发失败
 */
uint8_t startMeasureOfAht20(uint8_t time_out_ms)
{
	// 发送测量命令
	if(writeMulByteByI2c2((uint8_t[]){0xAC, 0x33, 0x00}, 3) == 0)
		return 0;
	// 等待75ms待测量完成，等价于等待忙状态标志位Bit[7]从1变为0
	while((isBusyOfAht20() == 1) && (time_out_ms > 0))
	{
		time_out_ms--;
		delayMsUseSystick(1);
	}
	return (time_out_ms > 0) ? 1 : 0;
}

/**
 *@brief 读取数据并解析
 *@param temperature 测量的温度
 *@param humidity 测量的湿度
 *@return 1-解析成功，0-解析失败
 */
uint8_t readMeasureData(float* temperature, float* humidity)
{
	uint8_t measure_data[6];
	// 发0X71命令，读取测量数据
	if(readMulByteByI2c2(measure_data, 6) == 0)
		return 0;
	
	// 解析湿度数据
	uint32_t raw_humidity = 0;
	raw_humidity = ((uint32_t)measure_data[1] << 12) + ((uint32_t)measure_data[2] << 4) + ((uint32_t)measure_data[3] >> 4);
	*humidity = ((float)raw_humidity/(float)0x100000) * 100.0f;
	
	// 解析温度数据
	uint32_t raw_temperature = 0;
	raw_temperature = ((uint32_t)(measure_data[3] & 0x0F) << 16) + ((uint32_t)measure_data[4] << 8) + (uint32_t)measure_data[5];
	*temperature = ((float)raw_temperature/(float)0x100000) * 200.0f - 50.0f;
	
	return 1;
}

