#include "touch.h"

volatile _m_tp_dev tp_dev =
	{
		TP_Init,
		NULL,
		0,
		0,
		0,
};
// 默认为touchtype=0的数据.

/*****************************************************************************
 * @name       :uint8_t TP_Init(void)
 * @date       :2018-08-09
 * @function   :Initialization touch screen
 * @parameters :None
 * @retvalue   :0-no calibration
								1-Has been calibrated
******************************************************************************/
uint8_t TP_Init(void)
{
	if (FT6336_Init())
	{
		return 1;
	}
	tp_dev.init = FT6336_Init; // 初始化函数指向触摸屏初始化
	tp_dev.scan = FT6336_Scan; // 扫描函数指向GT911触摸屏扫描
	return 0;
}

/**
 * @brief GPIO外部中断回调函数。
 * 当指定的 GPIO 引脚发生外部中断事件时，该函数将被调用。
 * @param GPIO_Pin GPIO 引脚号
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CTP_INT_Pin)
	{
		tp_dev.scan();
		USART1_Printf("x:%d,y:%d\r\n", tp_dev.x, tp_dev.y);
	}
}
