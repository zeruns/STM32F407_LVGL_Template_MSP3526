#include "LCD.h"

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
	return rxData;
}

/**
 * @brief 通过SPI接口发送一个16位数据。
 * 将16位数据分割成两个8位数据，然后通过SPI接口发送。
 * @param Data: 要发送的16位数据
 * @return 没有返回值
 */
void SPI_Send2Byte(uint16_t Data)
{
	uint8_t data[2];
	data[0] = Data >> 8;
	data[1] = (uint8_t)Data;
	HAL_SPI_Transmit(LCD_SPI, data, 2, HAL_MAX_DELAY);
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
		;
	LCD_CS_CLR;
	LCD_RS_CLR;
	SPI_SwapByte(data);
	LCD_CS_SET;
}

/*****************************************************************************
 * @name       :void LCD_WR_DATA(uint8_t data)
 * @date       :2018-08-09
 * @function   :Write an 8-bit data to the LCD screen
 * @parameters :data:data value to be written
 * @retvalue   :None
 ******************************************************************************/
void LCD_WR_DATA(uint8_t data)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	LCD_RS_SET;
	SPI_SwapByte(data);
	LCD_CS_SET;
}

uint8_t LCD_RD_DATA(void)
{
	uint8_t data;
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	LCD_RS_SET;
	SPI1_SetSpeed(0);
	data = SPI_SwapByte(0xFF);
	SPI1_SetSpeed(1);
	LCD_CS_SET;
	return data;
}

/*****************************************************************************
 * @name       :void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
 * @date       :2018-08-09
 * @function   :Write data into registers
 * @parameters :LCD_Reg:Register address
				LCD_RegValue:Data to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}

uint8_t LCD_ReadReg(uint8_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);
	return LCD_RD_DATA();
}

/*****************************************************************************
 * @name       :void LCD_WriteRAM_Prepare(void)
 * @date       :2018-08-09
 * @function   :Write GRAM
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}

void LCD_ReadRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.rramcmd);
}

/*****************************************************************************
 * @name       :void Lcd_WriteData_16Bit(uint16_t Data)
 * @date       :2018-08-09
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :Data:Data to be written
 * @retvalue   :None
 ******************************************************************************/
void Lcd_WriteData_16Bit(uint16_t Data)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	LCD_RS_SET;
	SPI_Send2Byte(Data);
	LCD_CS_SET;
}

void Lcd_WriteData(uint8_t *Data, uint32_t Size)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	LCD_RS_SET;
	HAL_SPI_Transmit(LCD_SPI, Data, Size, HAL_MAX_DELAY);
	LCD_CS_SET;
}

CCMRAM void Lcd_WriteData_DMA(uint8_t *Data, uint32_t Size)
{
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	LCD_RS_SET;
	HAL_SPI_Transmit_DMA(LCD_SPI, Data, Size);
}

uint16_t Lcd_ReadData_16Bit(void)
{
	uint16_t r, g;
	while (HAL_SPI_GetState(LCD_SPI) != HAL_SPI_STATE_READY)
		;
	LCD_CS_CLR;
	LCD_RS_CLR;
	SPI_SwapByte(lcddev.rramcmd);
	SPI1_SetSpeed(0);
	LCD_RS_SET;
	SPI_SwapByte(0xFF);
	r = SPI_SwapByte(0xFF);
	g = SPI_SwapByte(0xFF);
	SPI1_SetSpeed(1);
	LCD_CS_SET;
	r <<= 8;
	r |= g;
	return r;
}

/*****************************************************************************
 * @name       :void LCD_DrawPoint(uint16_t x,uint16_t y)
 * @date       :2018-08-09
 * @function   :Write a pixel data at a specified location
 * @parameters :x:the x coordinate of the pixel
				y:the y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/
void LCD_DrawPoint(uint16_t x, uint16_t y)
{
	LCD_SetCursor(x, y); // 设置光标位置
	Lcd_WriteData_16Bit(POINT_COLOR);
}

uint16_t LCD_ReadPoint(uint16_t x, uint16_t y)
{
	uint16_t color;
	LCD_SetCursor(x, y); // 设置光标位置
	color = Lcd_ReadData_16Bit();
	return color;
}

/*******************************************************************
 * @name       :void GUI_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
 * @date       :2018-08-09
 * @function   :draw a point in LCD screen
 * @parameters :x:the x coordinate of the point
				y:the y coordinate of the point
								color:the color value of the point
 * @retvalue   :None
********************************************************************/
void LCDDrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_SetCursor(x, y); // 设置光标位置
	Lcd_WriteData_16Bit(color);
}

/*******************************************************************
 * @name       :void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
 * @date       :2018-08-09
 * @function   :fill the specified area
 * @parameters :sx:the bebinning x coordinate of the specified area
				sy:the bebinning y coordinate of the specified area
								ex:the ending x coordinate of the specified area
								ey:the ending y coordinate of the specified area
								color:the filled color value
 * @retvalue   :None
********************************************************************/
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

CCMRAM void LCD_Fill_LVGL(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, lv_color_t *color_p)
{
	uint32_t i, j;
	uint16_t width = ex - sx + 1;  // 得到填充的宽度
	uint16_t height = ey - sy + 1; // 高度
	uint32_t Pixel = width * height;
	LCD_SetWindows(sx, sy, ex, ey); // 设置显示窗口
									//	for (i = 0; i < height; i++)
									//	{
									//		for (j = 0; j < width; j++){
									//			Lcd_WriteData_16Bit(color_p->full); // 写入数据
									//		}
									//	}

	uint8_t data[Pixel * 2];
	for (i = 0; i < Pixel; i++)
	{
		// 将16位的颜色数据拆分成两个字节的数据
		data[i * 2] = (color_p->full) >> 8;
		data[i * 2 + 1] = (uint8_t)(color_p->full);
		color_p++;
		if ((Pixel * 2 >= 6000) && (i == (Pixel / 2)))
		{
			// 要发送的数据字节数大于6000时分开两次发送
			if (Pixel % 2 == 1)
			{
				// 奇数时发送数据+1，凑个偶数，因为颜色数据是两个字节
				Lcd_WriteData_DMA(data, Pixel + 1);
			}
			else
			{
				Lcd_WriteData_DMA(data, Pixel);
			}
		}
	}
	if (width * height * 2 >= 6000)
	{
		// 发送剩余数据
		if ((width * height) % 2 == 1)
		{
			uint8_t *temp = &data[Pixel + 1];
			Lcd_WriteData_DMA(temp, Pixel - 1);
		}
		else
		{
			uint8_t *temp = &data[Pixel];
			Lcd_WriteData_DMA(temp, Pixel);
		}
	}
	else
	{
		// 要发送的数据小于6000字节时一次全部发送
		Lcd_WriteData_DMA(data, Pixel * 2);
	}

	LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1); // 恢复窗口设置为全屏
}

/*****************************************************************************
 * @name       :void LCD_Clear(uint16_t Color)
 * @date       :2018-08-09
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
 ******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	unsigned int i, m;
	LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
	uint8_t data[lcddev.width * 2];
	uint16_t n = 0;
	for (i = 0; i < lcddev.height; i++)
	{
		for (m = 0; m < lcddev.width; m++)
		{
			// Lcd_WriteData_16Bit(Color);
			data[n] = Color >> 8;
			data[n + 1] = (uint8_t)Color;
			n = n + 2;
		}
		Lcd_WriteData_DMA(data, lcddev.width * 2);
		n = 0;
	}
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09
 * @function   :Reset LCD screen
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_RESET(void)
{
	LCD_RST_CLR;
	HAL_Delay(80);
	LCD_RST_SET;
	HAL_Delay(30);
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09
 * @function   :Initialization LCD screen
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_Init(void)
{

	LCD_RESET(); // LCD 复位
	//*************3.5 ST7796S IPS初始化**********//
	LCD_WR_REG(0x11);

	HAL_Delay(80); // Delay 120ms

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

	HAL_Delay(80);

	LCD_WR_REG(0x21);

	LCD_WR_REG(0x29);

	LCD_direction(USE_HORIZONTAL); // 设置LCD显示方向
	LCD_Clear(WHITE);			   // 清全屏白色
}

/*****************************************************************************
 * @name       :void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
 * @date       :2018-08-09
 * @function   :Setting LCD display window
 * @parameters :xStar:the bebinning x coordinate of the LCD display window
								yStar:the bebinning y coordinate of the LCD display window
								xEnd:the endning x coordinate of the LCD display window
								yEnd:the endning y coordinate of the LCD display window
 * @retvalue   :None
******************************************************************************/
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd)
{
	LCD_WR_REG(lcddev.setxcmd);
	LCD_WR_DATA(xStar >> 8);
	LCD_WR_DATA(0x00FF & xStar);
	LCD_WR_DATA(xEnd >> 8);
	LCD_WR_DATA(0x00FF & xEnd);

	LCD_WR_REG(lcddev.setycmd);
	LCD_WR_DATA(yStar >> 8);
	LCD_WR_DATA(0x00FF & yStar);
	LCD_WR_DATA(yEnd >> 8);
	LCD_WR_DATA(0x00FF & yEnd);

	LCD_WriteRAM_Prepare(); // 开始写入GRAM
}

/*****************************************************************************
 * @name       :void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
 * @date       :2018-08-09
 * @function   :Set coordinate value
 * @parameters :Xpos:the  x coordinate of the pixel
								Ypos:the  y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_SetWindows(Xpos, Ypos, Xpos, Ypos);
}

/*****************************************************************************
 * @name       :void LCD_direction(uint8_t direction)
 * @date       :2018-08-09
 * @function   :Setting the display direction of LCD screen
 * @parameters :direction:0-0 degree
						  1-90 degree
													2-180 degree
													3-270 degree
 * @retvalue   :None
******************************************************************************/
void LCD_direction(uint8_t direction)
{
	lcddev.setxcmd = 0x2A;
	lcddev.setycmd = 0x2B;
	lcddev.wramcmd = 0x2C;
	lcddev.rramcmd = 0x2E;
	lcddev.dir = direction % 4;
	switch (lcddev.dir)
	{
	case 0:
		lcddev.width = LCD_W;
		lcddev.height = LCD_H;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 6));
		break;
	case 1:
		lcddev.width = LCD_H;
		lcddev.height = LCD_W;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 5));
		break;
	case 2:
		lcddev.width = LCD_W;
		lcddev.height = LCD_H;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 7));
		break;
	case 3:
		lcddev.width = LCD_H;
		lcddev.height = LCD_W;
		LCD_WriteReg(0x36, (1 << 3) | (1 << 7) | (1 << 6) | (1 << 5));
		break;
	default:
		break;
	}
}

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
