#include "LCD.h"

// 管理LCD重要参数
// 默认为竖屏
_lcd_dev lcddev;

// 画笔颜色,背景颜色
uint16_t POINT_COLOR = 0x0000, BACK_COLOR = 0xFFFF;
uint16_t DeviceCode;

/**
 * @brief SPI 交换字节。
 * 通过 SPI 接口发送一个字节数据，并接收返回的数据。
 * @param ByteSend 要发送的字节数据
 * @return 接收到的字节数据
 */
uint8_t SPI_SwapByte(uint8_t ByteSend)
{
	uint8_t rxData = 0;													   // 用于接收数据的变量
	HAL_SPI_TransmitReceive(&hspi2, &ByteSend, &rxData, 1, HAL_MAX_DELAY); // SPI发送数据并接收数据
	return rxData;
}


/*****************************************************************************
 * @name       :void LCD_WR_REG(uint8_t data)
 * @date       :2018-08-09
 * @function   :Write an 8-bit command to the LCD screen
 * @parameters :data:Command value to be written
 * @retvalue   :None
 ******************************************************************************/
void LCD_WR_REG(uint8_t data)
{
	LCD_CS_CLR;
	LCD_RS_CLR;
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY); // SPI发送一个字节数据
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
	LCD_CS_CLR;
	LCD_RS_SET;
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	LCD_CS_SET;
}

uint8_t LCD_RD_DATA(void)
{
	uint8_t data;
	LCD_CS_CLR;
	LCD_RS_SET;
	SPI2_SetSpeed(0);
	data = SPI_SwapByte(0xFF);
	SPI2_SetSpeed(1);
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
	uint8_t data[2];
	LCD_CS_CLR;
	LCD_RS_SET;
	data[0] = Data >> 8;
	data[1] = (uint8_t)Data;
	HAL_SPI_Transmit(&hspi2, data, 2, HAL_MAX_DELAY);
	/*SPI_SwapByte(Data >> 8);
	SPI_SwapByte(Data);*/
	LCD_CS_SET;
}

uint16_t Lcd_ReadData_16Bit(void)
{
	uint16_t r, g;
	LCD_CS_CLR;
	LCD_RS_CLR;
	SPI_SwapByte(lcddev.rramcmd);
	SPI2_SetSpeed(0);
	LCD_RS_SET;
	SPI_SwapByte(0xFF);
	r = SPI_SwapByte(0xFF);
	g = SPI_SwapByte(0xFF);
	SPI2_SetSpeed(1);
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
void DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	LCD_SetCursor(x, y); // 设置光标位置
	Lcd_WriteData_16Bit(color);
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
	LCD_CS_CLR;
	LCD_RS_SET;
	for (i = 0; i < lcddev.height; i++)
	{
		for (m = 0; m < lcddev.width; m++)
		{
			SPI_SwapByte(Color >> 8);
			SPI_SwapByte(Color);
		}
	}
	LCD_CS_SET;
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
	HAL_Delay(150);
	LCD_RST_SET;
	HAL_Delay(100);
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

	HAL_Delay(120); // Delay 120ms

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

	HAL_Delay(120);

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
	LCD_CS_CLR;
	for (i = 1; i < 4; i++)
	{
		LCD_RS_CLR;
		SPI_SwapByte(0xFB);
		LCD_RS_SET;
		SPI_SwapByte(0x10 + i);
		LCD_RS_CLR;
		SPI_SwapByte(0xD3);
		SPI2_SetSpeed(0);
		LCD_RS_SET;
		val[i - 1] = SPI_SwapByte(0xFF);
		SPI2_SetSpeed(1);
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
