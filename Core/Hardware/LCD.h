#ifndef __LCD_H
#define __LCD_H

#include "stdlib.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "tim.h"

// LCD重要参数集
typedef struct
{
	uint16_t width;	  // LCD 宽度
	uint16_t height;  // LCD 高度
	uint16_t id;	  // LCD ID
	uint8_t dir;	  // 横屏还是竖屏控制：0，竖屏；1，横屏。
	uint16_t wramcmd; // 开始写gram指令
	uint16_t rramcmd; // 开始读gram指令
	uint16_t setxcmd; // 设置x坐标指令
	uint16_t setycmd; // 设置y坐标指令
} _lcd_dev;

// LCD参数
extern _lcd_dev lcddev; // 管理LCD重要参数
/////////////////////////////////////用户配置区///////////////////////////////////
#define USE_HORIZONTAL 0 // 定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转

//////////////////////////////////////////////////////////////////////////////////
// 定义LCD的尺寸
#define LCD_W 320
#define LCD_H 480

// TFTLCD部分外要调用的函数
extern uint16_t POINT_COLOR; // 默认红色
extern uint16_t BACK_COLOR;	 // 背景颜色.默认为白色

////////////////////////////////////////////////////////////////////
//-----------------LCD端口定义----------------

#define LED 13 // 背光控制引脚

// QDtech全系列模块采用了三极管控制背光亮灭，用户也可以接PWM调节背光亮度
#define LCD_LED PBout(LED) // LCD背光
#define GPIO_TYPE GPIOB
#define LCD_CS 15
#define LCD_RS 14
#define LCD_RST 12
// 如果使用官方库函数定义下列底层，速度将会下降到14帧每秒，建议采用我司推荐方法
// 以下IO定义直接操作寄存器，快速IO操作，刷屏速率可以达到28帧每秒！
/*
#define LCD_CS_SET GPIO_TYPE->BSRRL = 1 << LCD_CS	// 片选端口  	PB11
#define LCD_RS_SET GPIO_TYPE->BSRRL = 1 << LCD_RS	// 数据/命令  PB10
#define LCD_RST_SET GPIO_TYPE->BSRRL = 1 << LCD_RST // 复位			PB12

#define LCD_CS_CLR GPIO_TYPE->BSRRH = 1 << LCD_CS	// 片选端口  	PB11
#define LCD_RS_CLR GPIO_TYPE->BSRRH = 1 << LCD_RS	// 数据/命令  PB10
#define LCD_RST_CLR GPIO_TYPE->BSRRH = 1 << LCD_RST // 复位			  PB12
*/

#define LCD_CS_SET HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)	// 片选端口
#define LCD_RS_SET HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET)	// 数据/命令
#define LCD_RST_SET HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET) // 复位

#define LCD_CS_CLR HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)	  // 片选端口
#define LCD_RS_CLR HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET)	  // 数据/命令
#define LCD_RST_CLR HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET) // 复位

// 画笔颜色
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 // 棕色
#define BRRED 0XFC07 // 棕红色
#define GRAY 0X8430	 // 灰色
// GUI颜色

#define DARKBLUE 0X01CF	 // 深蓝色
#define LIGHTBLUE 0X7D7C // 浅蓝色
#define GRAYBLUE 0X5458	 // 灰蓝色
// 以上三色为PANEL的颜色

#define LIGHTGREEN 0X841F // 浅绿色
#define LIGHTGRAY 0XEF5B  // 浅灰色(PANNEL)
#define LGRAY 0XC618	  // 浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE 0XA651 // 浅灰蓝色(中间层颜色)
#define LBBLUE 0X2B12	 // 浅棕蓝色(选择条目的反色)

void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(uint16_t Color);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawPoint(uint16_t x, uint16_t y);		// 画点
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y); // 读点
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);

uint8_t LCD_RD_DATA(void); // 读取LCD数据
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
void LCD_WR_DATA(uint8_t data);
uint8_t LCD_ReadReg(uint8_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
uint16_t LCD_ReadRAM(void);
uint16_t LCD_BGR2RGB(uint16_t c);
void LCD_SetParam(void);
void Lcd_WriteData_16Bit(uint16_t Data);
void LCD_direction(uint8_t direction);
uint16_t LCD_Read_ID(void);

#endif
