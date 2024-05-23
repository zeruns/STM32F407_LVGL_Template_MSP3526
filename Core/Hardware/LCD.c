#include "LCD.h"

/**
 * @brief ST7796 LCD驱动程序。
 * @作者个人博客： https://blog.zeruns.tech
 * @B站空间： https://space.bilibili.com/8320520
 */

// LCD屏使用的SPI接口
#define LCD_SPI &hspi1

// 管理LCD重要参数
// 默认为竖屏
volatile _lcd_dev lcddev;

// 画笔颜色,背景颜色
uint16_t POINT_COLOR = 0x0000, BACK_COLOR = 0xFFFF;
uint16_t DeviceCode;

/**
 * @brief SPI传输完成回调函数。
 * 当通过SPI1接口发送数据完成时，此回调函数被调用。
 * @param hspi: 指向SPI句柄的指针，包含SPI状态信息。
 * @retval None
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1) // 检查完成传输的SPI外设是否是SPI1
	{
		LCD_CS_SET; // LCD片选引脚置高电平，停止发送
	}
}

/**
 * @brief SPI 交换字节。
 * 通过 SPI 接口发送一个字节数据，并接收返回的数据。
 * @param ByteSend 要发送的字节数据
 * @return 接收到的字节数据
 */
uint8_t SPI_SwapByte(uint8_t ByteSend)
{
	uint8_t rxData = 0;														// 用于接收数据的变量
	HAL_SPI_TransmitReceive(LCD_SPI, &ByteSend, &rxData, 1, HAL_MAX_DELAY); // SPI发送数据并接收数据
	return rxData;															// 返回接收到的数据
}

/**
 * @brief 通过SPI接口发送一个16位数据。
 * 将16位数据分割成两个8位数据，然后通过SPI接口发送。
 * @param Data: 要发送的16位数据
 * @return 没有返回值
 */
void SPI_Send2Byte(uint16_t Data)
{
	uint8_t data[2];								   // 用于发送数据的数组
	data[0] = Data >> 8;							   // 高8位数据
	data[1] = (uint8_t)Data;						   // 低8位数据
	HAL_SPI_Transmit(LCD_SPI, data, 2, HAL_MAX_DELAY); // 发送数据
}

/**
 * @brief 向LCD屏幕写入一个8位命令。
 * 向LCD屏幕发送一个8位的命令值。
 * @param data: 要写入的命令值
 * @return 没有返回值
 */
void LCD_WR_REG(uint8_t data)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;				// 等待SPI总线空闲
	LCD_CS_CLR;			// 选中LCD
	LCD_RS_CLR;			// 设置为命令模式
	SPI_SwapByte(data); // 发送命令字节
	LCD_CS_SET;			// 取消选中LCD
}

/**
 * @brief 向LCD写入数据
 * 通过SPI接口向LCD写入一个字节的数据。
 * @param data 要写入的数据
 */
void LCD_WR_DATA(uint8_t data)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;				// 等待SPI总线空闲
	LCD_CS_CLR;			// 选中LCD
	LCD_RS_SET;			// 设置为数据模式
	SPI_SwapByte(data); // 发送数据
	LCD_CS_SET;			// 取消选中LCD
}

/**
 * @brief 读取LCD数据
 * 从LCD中读取一个字节的数据，并通过SPI接口返回。
 * @return 读取到的LCD数据
 */
uint8_t LCD_RD_DATA(void)
{
	uint8_t data;
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;					   // 等待SPI总线空闲
	LCD_CS_CLR;				   // 选中LCD
	LCD_RS_SET;				   // 设置为数据模式
	SPI1_SetSpeed(0);		   // 设置SPI速度为低速
	data = SPI_SwapByte(0xFF); // 发送命令字节，并接收返回数据
	SPI1_SetSpeed(1);		   // 设置SPI速度为高速
	LCD_CS_SET;				   // 取消选中LCD
	return data;			   // 返回读取到的数据
}

/**
 * @brief 向LCD寄存器写入数据
 * 向指定的LCD寄存器写入给定的数据值。
 * @param LCD_Reg LCD寄存器地址
 * @param LCD_RegValue 要写入的数据值
 */
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}

/**
 * @brief 读取 LCD 寄存器值
 * 通过给定的 LCD 寄存器地址，读取对应的寄存器值并返回。
 * @param LCD_Reg LCD 寄存器地址
 * @return 读取到的 LCD 寄存器值
 */
uint8_t LCD_ReadReg(uint8_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);
	return LCD_RD_DATA();
}

/**
 * @brief 准备向LCD的GRAM写入数据
 * 该函数用于准备向LCD的GRAM写入数据。通过发送写GRAM命令到LCD，为后续的写入操作做准备。
 */
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd); // 发送写GRAM命令
}

/**
 * @brief 准备读取LCD的GRAM
 * 此函数用于准备读取LCD GRAM。
 * 在调用此函数后，可以通过其他函数来读取 LCD GRAM 中的数据。
 */
void LCD_ReadRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.rramcmd); // 发送读GRAM命令
}

/**
 * @brief 向LCD写入16位的数据
 * 使用SPI接口向LCD写入16位的数据。
 * @param Data 要写入的数据
 */
void Lcd_WriteData_16Bit(uint16_t Data)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;				 // 等待SPI总线空闲
	LCD_CS_CLR;			 // 选中LCD
	LCD_RS_SET;			 // 设置为数据模式
	SPI_Send2Byte(Data); // 发送数据
	LCD_CS_SET;			 // 取消选中LCD
}

/**
 * @brief  向LCD写入多个字节数据。
 * @param  Data: 指向要写入LCD的数据的指针。
 * @param  Size: 要写入的数据大小，以字节为单位。
 * @retval 无
 */
void Lcd_WriteData(uint8_t *Data, uint32_t Size)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;												  // 等待SPI总线空闲
	LCD_CS_CLR;											  // LCD片选引脚置低电平，选中LCD
	LCD_RS_SET;											  // LCD数据/命令引脚置高电平，设置为数据模式
	HAL_SPI_Transmit(LCD_SPI, Data, Size, HAL_MAX_DELAY); // 发送数据
	LCD_CS_SET;											  // LCD片选引脚置高电平，取消选中LCD
}

/**
 * @brief 使用DMA方式向LCD写入多个字节的数据
 * 使用DMA（直接内存访问）方式将指定的数据写入LCD屏幕。
 * @param Data 指向待写入数据的指针
 * @param Size 待写入数据的长度
 */
CCMRAM void Lcd_WriteData_DMA(uint8_t *Data, uint32_t Size)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;									   // 等待SPI总线空闲
	LCD_CS_CLR;								   // LCD片选引脚置低电平，选中LCD
	LCD_RS_SET;								   // LCD数据/命令引脚置高电平，设置为数据模式
	HAL_SPI_Transmit_DMA(LCD_SPI, Data, Size); // 发送数据
}

/**
 * @brief 从LCD读取16位的数据
 * 从LCD中读取16位数据并返回。
 * @return 读取到的16位数据
 */
uint16_t Lcd_ReadData_16Bit(void)
{
	uint16_t r, g;
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;						  // 等待SPI总线空闲
	LCD_CS_CLR;					  // LCD片选引脚置低电平，选中LCD
	LCD_RS_CLR;					  // LCD数据/命令引脚置低电平，设置为命令模式
	SPI_SwapByte(lcddev.rramcmd); // 发送读GRAM命令
	SPI1_SetSpeed(0);			  // 设置SPI速度为低速
	LCD_RS_SET;					  // LCD数据/命令引脚置高电平，设置为数据模式
	SPI_SwapByte(0xFF);			  // 发送数据
	r = SPI_SwapByte(0xFF);		  // 接收高8位数据
	g = SPI_SwapByte(0xFF);		  // 接收低8位数据
	SPI1_SetSpeed(1);			  // 设置SPI速度为高速
	LCD_CS_SET;					  // LCD片选引脚置高电平，取消选中LCD
	r <<= 8;					  // 合并高8位数据
	r |= g;						  // 合并低8位数据
	return r;					  // 返回数据
}

/**
 * @brief 在LCD上绘制一个点
 * 在指定的坐标位置 `(x, y)` 上绘制一个点，点的颜色由 `POINT_COLOR` 定义。
 * @param x 点的x坐标
 * @param y 点的y坐标
 */
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
	LCD_SetCursor(x, y);			  // 设置光标位置
	Lcd_WriteData_16Bit(POINT_COLOR); // 写入16位数据
}

/**
 * @brief 读取LCD屏幕指定点的颜色值
 * 在LCD屏幕上，根据给定的x和y坐标，读取指定点的颜色值。
 * @param x x坐标
 * @param y y坐标
 * @return 返回指定点的颜色值
 */
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
	uint16_t color;
	LCD_SetCursor(x, y);		  // 设置光标位置
	color = Lcd_ReadData_16Bit(); // 读取16位数据
	return color;				  // 返回颜色值
}

/**
 * @brief 在LCD上绘制一个点
 * 在LCD上指定位置绘制一个点，并设置该点的颜色。
 * @param x 点的横坐标
 * @param y 点的纵坐标
 * @param color 点的颜色
 */
void LCDDrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_SetCursor(x, y);		// 设置光标位置
	Lcd_WriteData_16Bit(color); // 写入16位数据
}

/**
 * @brief 填充LCD屏幕指定矩形区域
 * 在LCD屏幕上填充指定矩形区域的颜色。
 * @param sx 起始点的横坐标
 * @param sy 起始点的纵坐标
 * @param ex 结束点的横坐标
 * @param ey 结束点的纵坐标
 * @param color 填充颜色
 */
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
	uint16_t i, j;
	uint16_t width = ex - sx + 1;	// 得到填充的宽度
	uint16_t height = ey - sy + 1;	// 高度
	LCD_SetWindows(sx, sy, ex, ey); // 设置显示窗口
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
			Lcd_WriteData_16Bit(color); // 写入数据
	}
	LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1); // 恢复窗口设置为全屏
}

/**
 * @brief 填充LCD屏幕指定矩形区域
 * 使用LVGL库的颜色值填充LCD指定区域。
 * @param sx 起始X坐标
 * @param sy 起始Y坐标
 * @param ex 结束X坐标
 * @param ey 结束Y坐标
 * @param color_p 指向要填充的颜色的指针
 */
CCMRAM void LCD_Fill_LVGL(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, lv_color_t *color_p)
{
	uint32_t i, j;
	uint16_t width = ex - sx + 1;	 // 计算填充区域的宽度
	uint16_t height = ey - sy + 1;	 // 计算填充区域的高度
	uint32_t Pixel = width * height; // 计算填充区域像素个数
	LCD_SetWindows(sx, sy, ex, ey);	 // 设置LCD的显示窗口为指定的区域

//	for (i = 0; i < height; i++)
//	{
//		for (j = 0; j < width; j++){
//			Lcd_WriteData_16Bit(color_p->full); // 写入数据
//		}
//	}

// 数据分割值, 用于分批发送数据
#define data_split 3000

	uint8_t data[Pixel * 2]; // 创建一个数组用于存储颜色数据

	for (i = 0; i < Pixel; i++)
	{
		// 将颜色数据从16位转换为两个8位的数据
		data[i * 2] = (color_p->full) >> 8;			// 获取高8位数据
		data[i * 2 + 1] = (uint8_t)(color_p->full); // 获取低8位数据
		color_p++;									// 指向下一个颜色值

		// 判断数据量是否大于10000，如果大于则分批发送数据
		if (Pixel > 10000)
		{
			if ((i + 1) % data_split == 0)
			{
				if ((i + 1) == data_split)
				{
					Lcd_WriteData_DMA(data, data_split * 2); // 以DMA方式发送数据
				}
				else
				{
					while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY);						// 等待SPI总线空闲
					uint8_t *temp = &data[((uint16_t)((i + 1) / data_split) - 1) * data_split * 2]; // 获取剩余数据
					Lcd_WriteData_DMA(temp, data_split * 2);										// 以DMA方式发送数据
				}
			}
			else if (((i + 1) % data_split > 0) && ((i + 1) > data_split) && (i == (Pixel - 1)))
			{
				if ((uint16_t)((i + 1) / data_split) == 1)
				{
					while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
						;												 // 等待SPI总线空闲
					uint8_t *temp = &data[data_split * 2];				 // 获取剩余数据
					Lcd_WriteData_DMA(temp, ((i + 1) % data_split) * 2); // 以DMA方式发送数据
				}
				else
				{
					while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
						;																	  // 等待SPI总线空闲
					uint8_t *temp = &data[(uint16_t)((i + 1) / data_split) * data_split * 2]; // 获取剩余数据
					Lcd_WriteData_DMA(temp, ((i + 1) % data_split) * 2);
				}
			}
		}
	}

	if (Pixel <= 10000)
	{
		// 要发送的数据小于10000*2字节时一次全部发送
		Lcd_WriteData_DMA(data, Pixel * 2);
	}

	LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1); // 恢复窗口设置为全屏
}

/**
 * @brief LCD清屏
 * 将LCD屏幕的内容全部设置为指定颜色，实现屏幕清空效果。
 * @param Color 颜色值
 */
void LCD_Clear(uint16_t Color)
{
	unsigned int i, m;
	LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1); // 设置显示窗口为全屏
	uint8_t data[lcddev.width * 2];							   // 创建一个数组用于存储颜色数据
	uint16_t n = 0;
	for (i = 0; i < lcddev.height; i++)
	{
		for (m = 0; m < lcddev.width; m++)
		{
			// Lcd_WriteData_16Bit(Color);
			data[n] = Color >> 8;		  // 获取高8位数据
			data[n + 1] = (uint8_t)Color; // 获取低8位数据
			n = n + 2;
		}
		Lcd_WriteData_DMA(data, lcddev.width * 2); // 以DMA方式发送数据
		n = 0;
	}
}

/**
 * @brief LCD屏复位
 * 该函数用于复位LCD屏。
 * 通过控制LCD_RST引脚，使LCD屏进入复位状态，并等待一段时间后释放复位。
 */
void LCD_RESET(void)
{
	LCD_RST_CLR;
	osDelay(80);
	LCD_RST_SET;
	osDelay(30);
}

/**
 * @brief 初始化LCD显示屏
 * 该函数用于初始化LCD显示屏，并设置相关的寄存器。
 * @note 初始化完成后，LCD显示屏将处于可用状态，并设置显示方向为垂直方向。
 */
void LCD_Init(void)
{

	LCD_RESET(); // LCD 复位
	//*************3.5 ST7796S IPS初始化**********//
	LCD_WR_REG(0x11);

	osDelay(80); // Delay 120ms

	LCD_WR_REG(0x36); // Memory Data Access Control MY,MX~~
	LCD_WR_DATA(0x48);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA(0x55); // LCD_WR_DATA(0x66);

	LCD_WR_REG(0xF0); // Command Set Control
	LCD_WR_DATA(0xC3);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x96);

	LCD_WR_REG(0xB4);
	LCD_WR_DATA(0x01);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA(0xC6);

	/*LCD_WR_REG(0xB9);
	LCD_WR_DATA(0x02);
	LCD_WR_DATA(0xE0);*/

	LCD_WR_REG(0xC0);
	LCD_WR_DATA(0x80);
	LCD_WR_DATA(0x45);

	LCD_WR_REG(0xC1);
	LCD_WR_DATA(0x13); // 18  //00

	LCD_WR_REG(0xC2);
	LCD_WR_DATA(0xA7);

	LCD_WR_REG(0xC5);
	LCD_WR_DATA(0x0A);

	LCD_WR_REG(0xE8);
	LCD_WR_DATA(0x40);
	LCD_WR_DATA(0x8A);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x29);
	LCD_WR_DATA(0x19);
	LCD_WR_DATA(0xA5);
	LCD_WR_DATA(0x33);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA(0xD0);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x33);
	LCD_WR_DATA(0x30);
	LCD_WR_DATA(0x33);
	LCD_WR_DATA(0x47);
	LCD_WR_DATA(0x17);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x13);
	LCD_WR_DATA(0x2B);
	LCD_WR_DATA(0x31);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA(0xD0);
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x0B);
	LCD_WR_DATA(0x09);
	LCD_WR_DATA(0x07);
	LCD_WR_DATA(0x2F);
	LCD_WR_DATA(0x33);
	LCD_WR_DATA(0x47);
	LCD_WR_DATA(0x38);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x16);
	LCD_WR_DATA(0x2C);
	LCD_WR_DATA(0x32);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x3C);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x69);

	osDelay(80);

	LCD_WR_REG(0x21);

	LCD_WR_REG(0x29);

	LCD_direction(USE_HORIZONTAL); // 设置LCD显示方向
	LCD_Clear(WHITE);			   // 清全屏白色
}

/**
 * @brief 设置LCD显示窗口区域
 * 将指定的起始坐标和结束坐标作为LCD的显示窗口区域进行设置，用于后续文本或图形的绘制。
 * @param xStar 窗口起始点的X坐标
 * @param yStar 窗口起始点的Y坐标
 * @param xEnd 窗口结束点的X坐标
 * @param yEnd 窗口结束点的Y坐标
 */
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd)
{
	LCD_WR_REG(lcddev.setxcmd); // 设置X坐标
	/*LCD_WR_DATA(xStar >> 8);
	LCD_WR_DATA(0x00FF & xStar);
	LCD_WR_DATA(xEnd >> 8);
	LCD_WR_DATA(0x00FF & xEnd);*/
	Lcd_WriteData_16Bit(xStar); // 设置X坐标起始点
	Lcd_WriteData_16Bit(xEnd);	// 设置X坐标结束点

	LCD_WR_REG(lcddev.setycmd); // 设置Y坐标
	/*LCD_WR_DATA(yStar >> 8);
	LCD_WR_DATA(0x00FF & yStar);
	LCD_WR_DATA(yEnd >> 8);
	LCD_WR_DATA(0x00FF & yEnd);*/
	Lcd_WriteData_16Bit(yStar); // 设置Y坐标起始点
	Lcd_WriteData_16Bit(yEnd);	// 设置Y坐标结束点

	LCD_WriteRAM_Prepare(); // 开始写入GRAM
}

/**
 * @brief 设置LCD光标位置
 * 在LCD屏幕上设置光标的位置，用于后续文本或图形的绘制。
 * @param Xpos X坐标位置
 * @param Ypos Y坐标位置
 */
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_SetWindows(Xpos, Ypos, Xpos, Ypos);
}

/**
 * @brief 设置LCD屏幕的显示方向。
 * 根据传入的方向参数，配置LCD的显示方向。
 * @param direction 要设置的LCD显示方向：
 *                   0 - 0度（默认方向）
 *                   1 - 90度（顺时针旋转）
 *                   2 - 180度（旋转半圈）
 *                   3 - 270度（逆时针旋转）
 * @return None
 * @note 该函数通过写入特定的寄存器值来改变LCD的显示方向。
 */
void LCD_direction(uint8_t direction)
{
	// 定义设置X坐标、Y坐标、写入RAM和读取RAM的命令代码
	lcddev.setxcmd = 0x2A;
	lcddev.setycmd = 0x2B;
	lcddev.wramcmd = 0x2C;
	lcddev.rramcmd = 0x2E;

	// 确保方向值在0到3之间
	lcddev.dir = direction % 4;

	// 根据方向值，设置LCD的宽度、高度和写入寄存器0x36来改变显示方向
	switch (lcddev.dir)
	{
	case 0: // 0度方向，宽度和高度保持默认值
		lcddev.width = LCD_W;
		lcddev.height = LCD_H;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 6)); // 写入寄存器0x36，设置扫描方向
		break;
	case 1: // 90度方向，交换宽度和高度的值
		lcddev.width = LCD_H;
		lcddev.height = LCD_W;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 5)); // 写入寄存器0x36，设置扫描方向
		break;
	case 2: // 180度方向，宽度和高度保持默认值，但寄存器0x36的值不同
		lcddev.width = LCD_W;
		lcddev.height = LCD_H;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 7)); // 写入寄存器0x36，设置扫描方向
		break;
	case 3: // 270度方向，交换宽度和高度的值，并设置额外的扫描方向位
		lcddev.width = LCD_H;
		lcddev.height = LCD_W;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 7) | (1 << 6) | (1 << 5)); // 写入寄存器0x36，设置扫描方向
		break;
	default: // 默认值，不进行任何操作
		break;
	}
}

/**
 * @brief 读取LCD屏幕ID
 * 通过向LCD屏幕发送指令并读取返回的数据，获取LCD屏幕的ID信息。
 * @return 返回LCD屏幕的ID值，该ID值是一个16位无符号整数。
 */
uint16_t LCD_Read_ID(void)
{
	uint8_t i, val[3] = {0};
	LCD_WR_REG(0xF0); // Command Set Control
	LCD_WR_DATA(0xC3);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x96);
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	for (i = 1; i < 4; i++)
	{
		LCD_RS_CLR;
		SPI_SwapByte(0xFB);
		LCD_RS_SET;
		SPI_SwapByte(0x10 + i);
		LCD_RS_CLR;
		SPI_SwapByte(0xD3);
		SPI1_SetSpeed(0);
		LCD_RS_SET;
		val[i - 1] = SPI_SwapByte(0xFF);
		SPI1_SetSpeed(1);
		LCD_RS_CLR;
		SPI_SwapByte(0xFB);
		LCD_RS_SET;
		SPI_SwapByte(0x00);
	}
	LCD_CS_SET;
	LCD_WR_REG(0xF0); // Command Set Control
	LCD_WR_DATA(0x3C);
	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x69);
	lcddev.id = val[1];
	lcddev.id <<= 8;
	lcddev.id |= val[2];
	return lcddev.id;
}

/**
 * @brief 切换屏幕方向
 * @param direction 要设置的LCD显示方向：
 *                   0 - 0度（默认方向）
 *                   1 - 90度（顺时针旋转）
 *                   2 - 180度（旋转半圈）
 *                   3 - 270度（逆时针旋转）
 * @return None
 */
void LCD_Switch_Dir(uint8_t direction)
{
	LCD_direction(direction);			   // 设置屏幕旋转
	lv_disp_set_rotation(NULL, direction); // 设置lvgl屏幕旋转
	if (direction == 0 || direction == 2)
	{
		lcddev.dir = 0;
	}
	else{
		lcddev.dir = 4;
	}
}