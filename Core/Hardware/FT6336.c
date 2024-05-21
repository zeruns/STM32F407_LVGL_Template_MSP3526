#include "FT6336.h"

/*FT6336从机地址*/
#define FT6336_ADDRESS 0x38 << 1 // 0x38是FT6336的7位地址，左移1位最后位做读写位变成0x70

/*I2C超时时间*/
#define FT6336_I2C_TIMEOUT 10

/*I2C接口，定义FT6336使用哪个I2C接口*/
#define FT6336_I2C &hi2c1

extern volatile uint8_t touch_flag;

/*****************************************************************************
 * @name       :uint8_t FT5426_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
 * @date       :2020-05-13
 * @function   :Write data to ft5426 once
 * @parameters :reg:Start register address for written
								buf:the buffer of data written
								len:Length of data written
 * @retvalue   :0-Write succeeded
								1-Write failed
******************************************************************************/
uint8_t FT6336_WR_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
	uint8_t i;

	uint8_t TxData[len + 1];
	TxData[0] = (reg & 0XFF); // 低8位地址

	for (i = 0; i < len; i++)
	{
		TxData[i + 1] = buf[i];
	}
	HAL_I2C_Master_Transmit(FT6336_I2C, FT6336_ADDRESS, (uint8_t *)TxData, len + 1, FT6336_I2C_TIMEOUT);

	return 0;
}

/*****************************************************************************
 * @name       :void FT5426_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
 * @date       :2020-05-13
 * @function   :Read data to ft5426 once
 * @parameters :reg:Start register address for read
								buf:the buffer of data read
								len:Length of data read
 * @retvalue   :none
******************************************************************************/
void FT6336_RD_Reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
	uint8_t TxData[1] = {(reg & 0XFF)}; // 低8位地址
	HAL_I2C_Master_Transmit(FT6336_I2C, FT6336_ADDRESS, (uint8_t *)TxData, 1, FT6336_I2C_TIMEOUT);
	HAL_I2C_Master_Receive(FT6336_I2C, FT6336_ADDRESS, (uint8_t *)buf, len, FT6336_I2C_TIMEOUT);
}

/*****************************************************************************
 * @name       :uint8_t FT5426_Init(void)
 * @date       :2020-05-13
 * @function   :Initialize the ft5426 touch screen
 * @parameters :none
 * @retvalue   :0-Initialization successful
								1-initialization failed
******************************************************************************/
uint8_t FT6336_Init(void)
{
	uint8_t temp[2];

	FT_RST(GPIO_PIN_RESET); // 复位
	HAL_Delay(10);
	FT_RST(GPIO_PIN_SET); // 释放复位
	HAL_Delay(500);
	//	temp[0]=0;
	//	FT6336_WR_Reg(FT_DEVIDE_MODE,temp,1);	//进入正常操作模式
	//	FT6336_WR_Reg(FT_ID_G_MODE,temp,1);		//查询模式
	// temp[0]=40;								//触摸有效值，22，越小越灵敏
	// FT6336_WR_Reg(FT_ID_G_THGROUP,temp,1);	//设置触摸有效值
	FT6336_RD_Reg(FT_ID_G_FOCALTECH_ID, &temp[0], 1);
	if (temp[0] != 0x11)
	{
		return 1;
	}
	FT6336_RD_Reg(FT_ID_G_CIPHER_MID, &temp[0], 2);
	if (temp[0] != 0x26)
	{
		return 1;
	}
	if ((temp[1] != 0x00) && (temp[1] != 0x01) && (temp[1] != 0x02))
	{
		return 1;
	}
	FT6336_RD_Reg(FT_ID_G_CIPHER_HIGH, &temp[0], 1);
	if (temp[0] != 0x64)
	{
		return 1;
	}
	//	temp[0]=12;								//激活周期，不能小于12，最大14
	//	FT6336_WR_Reg(FT_ID_G_PERIODACTIVE,temp,1);
	// 读取版本号，参考值：0x3003
	//	FT6336_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);
	//	if(temp[0]==0X10&&temp[1]==0X01)//版本:0X3003
	//	{
	//		printf("CTP ID:%x\r\n",((uint16_t)temp[0]<<8)+temp[1]);
	//		return 0;
	//	}
	return 0;
}

const uint16_t FT6336_TPX_TBL[2] = {FT_TP1_REG, FT_TP2_REG};

/*****************************************************************************
 * @name       :uint8_t FT5426_Scan(void)
 * @date       :2020-05-13
 * @function   :Scan touch screen (query mode)
 * @parameters :none
 * @retvalue   :Current touch screen status
								0-No touch
								1-With touch
******************************************************************************/
uint8_t FT6336_Scan(void)
{
	uint8_t buf[4];
	uint8_t i = 0;
	uint8_t res = 0;
	uint8_t temp;
	uint8_t mode;
	static uint8_t t = 0; // 控制查询间隔,从而降低CPU占用率
	t++;
	if ((t % 10) == 0 || t < 10) // 每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
		FT6336_RD_Reg(FT_REG_NUM_FINGER, &mode, 1); // 读取触摸点的状态
		if (mode && (mode < 3))
		{
			temp = 0XFF << mode; // 将点的个数转换为1的位数,匹配tp_dev.sta定义
			tp_dev.sta = (~temp) | TP_PRES_DOWN | TP_CATH_PRES;
			for (i = 0; i < CTP_MAX_TOUCH; i++)
			{
				FT6336_RD_Reg(FT6336_TPX_TBL[i], buf, 4); // 读取XY坐标值
				if (tp_dev.sta & (1 << i))				  // 触摸有效?
				{
					switch (lcddev.dir)
					{
					case 0:
						tp_dev.x[i] = ((uint16_t)(buf[0] & 0X0F) << 8) + buf[1];
						tp_dev.y[i] = ((uint16_t)(buf[2] & 0X0F) << 8) + buf[3];
						break;
					case 1:
						tp_dev.y[i] = lcddev.height - (((uint16_t)(buf[0] & 0X0F) << 8) + buf[1]);
						tp_dev.x[i] = ((uint16_t)(buf[2] & 0X0F) << 8) + buf[3];
						break;
					case 2:
						tp_dev.x[i] = lcddev.width - (((uint16_t)(buf[0] & 0X0F) << 8) + buf[1]);
						tp_dev.y[i] = lcddev.height - (((uint16_t)(buf[2] & 0X0F) << 8) + buf[3]);
						break;
					case 3:
						tp_dev.y[i] = ((uint16_t)(buf[0] & 0X0F) << 8) + buf[1];
						tp_dev.x[i] = lcddev.width - (((uint16_t)(buf[2] & 0X0F) << 8) + buf[3]);
						break;
					case 4:
						tp_dev.x[i] = lcddev.height - (((uint16_t)(buf[0] & 0X0F) << 8) + buf[1]);
						tp_dev.y[i] = lcddev.width - (((uint16_t)(buf[2] & 0X0F) << 8) + buf[3]);
						break;
					}
					// if((buf[0]&0XF0)!=0X80)tp_dev.x[i]=tp_dev.y[i]=0;//必须是contact事件，才认为有效
					// USART1_Printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}
			}
			res = 1;
			if (tp_dev.x[0] == 0 && tp_dev.y[0] == 0)
				mode = 0; // 读到的数据都是0,则忽略此次数据
			t = 0;		  // 触发一次,则会最少连续监测10次,从而提高命中率
		}
	}
	if (mode == 0) // 无触摸点按下
	{
		if (tp_dev.sta & TP_PRES_DOWN) // 之前是被按下的
		{
			tp_dev.sta &= ~(1 << 7); // 标记按键松开
		}
		else // 之前就没有被按下
		{
			tp_dev.x[0] = 0xffff;
			tp_dev.y[0] = 0xffff;
			tp_dev.sta &= 0XE0; // 清除点有效标记
		}
	}
	if (t > 240)
		t = 10; // 重新从10开始计数
	return res;
}