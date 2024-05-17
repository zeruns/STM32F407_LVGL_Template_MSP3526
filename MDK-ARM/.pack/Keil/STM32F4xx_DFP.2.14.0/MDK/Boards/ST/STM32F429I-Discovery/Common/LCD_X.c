/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2019  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.50 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              ARM Ltd, 110 Fulbourn Road, CB1 9NJ Cambridge, UK
Licensed SEGGER software: emWin
License number:           GUI-00181
License model:            LES-SLA-20007, Agreement, effective since October 1st 2011 
Licensed product:         MDK-ARM Professional
Licensed platform:        ARM7/9, Cortex-M/R4
Licensed number of seats: -
----------------------------------------------------------------------
File        : LCD_X.c
Purpose     : Port routines for STM32F429/439 RGB with SPI
----------------------------------------------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Hardware configuration
*
**********************************************************************
*/

#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"

/* SPI Interface: SPI5 (configured in RTE_Device.h)
     Pins:
       SCL  = PF7
       MISO = PF8
       MOSI = PF9
*/

/* SPI Driver */
extern ARM_DRIVER_SPI   Driver_SPI5;
#define ptrSPI        (&Driver_SPI5)

/*********************************************************************
*
*       Exported code
*
*********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Init
*
* Purpose:
*   This routine should be called from your application program
*   to set port pins to their initial values
*/
void LCD_X_Init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInitStructure;

  /* Enable GPIOs clock */
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();

  /* GPIOs configuration */
  /*
   +------------------------+------------------------+------------------------+
   +                           LCD pins assignment                            +
   +------------------------+------------------------+------------------------+
   |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06  |  LCD_TFT B2 <-> PD.06  |
   |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10  |  LCD_TFT B3 <-> PG.11  |
   |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10  |  LCD_TFT B4 <-> PG.12  |
   |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11  |  LCD_TFT B5 <-> PA.03  |
   |  LCD_TFT R6 <-> PB.01  |  LCD_TFT G6 <-> PC.07  |  LCD_TFT B6 <-> PB.08  |
   |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03  |  LCD_TFT B7 <-> PB.09  |
   ----------------------------------------------------------------------------
   |  LCD_TFT HSYNC <-> PC.06       |  LCD_TFT VSYNC <->  PA.04       |
   |  LCD_TFT CLK   <-> PG.07       |  LCD_TFT DE    <->  PF.10       |
   --------------------------------------------------------------------
   |  LCD_SCL <-> PF.07 (SPI5 SCL)  |  LCD_SDA <-> PF.09 (SPI5 MOSI)  |
   --------------------------------------------------------------------
   |  LCD_CSX <-> PC.02 (GPIO)      |  LCD_WRX <-> PD.13 (GPIO)       |
   --------------------------------------------------------------------
  */
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3  | GPIO_PIN_4  | GPIO_PIN_6  |
                           GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_8  |
                           GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6  | GPIO_PIN_7  | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3  | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);     

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6  | GPIO_PIN_7  | GPIO_PIN_10 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;

  /* GPIOD PD13 configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOC PC2 configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);


  /* LCD clock configuration 
       PLLSAI_VCO Input = HSE_VALUE / PLL_M = 1MHz
       PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192MHz
       PLLLCDCLK = PLLSAI_VCO Output / PLLSAIR = 192/4 = 48MHz
       LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6MHz
  */
  RCC_PeriphClkInitStructure.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  RCC_PeriphClkInitStructure.PLLSAI.PLLSAIN = 192;
  RCC_PeriphClkInitStructure.PLLSAI.PLLSAIR = 4;
  RCC_PeriphClkInitStructure.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInitStructure); 

  /* Initialize and configure SPI */
  ptrSPI->Initialize  (NULL);
  ptrSPI->PowerControl(ARM_POWER_FULL);
  ptrSPI->Control     (ARM_SPI_MODE_MASTER  |
                       ARM_SPI_CPOL0_CPHA0  |
                       ARM_SPI_MSB_LSB      |
                       ARM_SPI_DATA_BITS(8),
                       10000000);
}

/*********************************************************************
*
*       LCD_CSX
*
* Purpose:
*   Controls CSX pin
*/
__inline static void LCD_CSX(uint32_t val) {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2,  val ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/*********************************************************************
*
*       LCD_WRX
*
* Purpose:
*   Controls WRX pin
*/
__inline static void LCD_WRX(uint32_t val) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


/*********************************************************************
*
*       LCD_WriteM
*
* Purpose:
*   Writes multiple bytes to controller
*/
__inline static void LCD_WriteM(U8 * pData, int NumBytes) {
  ptrSPI->Send(pData, NumBytes);
  while (ptrSPI->GetStatus().busy);
}

/*********************************************************************
*
*       LCD_ReadM
*
* Purpose:
*   Reads multiple bytes from the controller
*/
__inline static void LCD_ReadM(U8 * pData, int NumBytes) {
  ptrSPI->Receive(pData, NumBytes);
  while (ptrSPI->GetStatus().busy);
}

/*********************************************************************
*
*       LCD_X_Write0_8
*
* Purpose:
*   Write to controller, with A0 = 0
*/
void LCD_X_Write0_8(U8 c) {
  LCD_WRX(0);
  LCD_CSX(0);
  LCD_WriteM(&c, 1);
  LCD_CSX(1);
}

/*********************************************************************
*
*       LCD_X_Read0_8
*
* Purpose:
*   Read from controller, with A0 = 0
*/
U8 LCD_X_Read0_8(void) {
  U8 c;

  LCD_WRX(0);
  LCD_CSX(0);
  LCD_ReadM(&c, 1);
  LCD_CSX(1);

  return (c);
}

/*********************************************************************
*
*       LCD_X_Write1_8
*
* Purpose:
*   Write to controller, with A0 = 1
*/
void LCD_X_Write1_8(U8 c) {
  LCD_WRX(1);
  LCD_CSX(0);
  LCD_WriteM(&c, 1);
  LCD_CSX(1);
}

/*********************************************************************
*
*       LCD_X_Read1_8
*
* Purpose:
*   Read from controller, with A0 = 1
*/
U8 LCD_X_Read1_8(void) {
  U8 c;

  LCD_WRX(1);
  LCD_CSX(0);
  LCD_ReadM(&c, 1);
  LCD_CSX(1);

  return (c);
}

/*********************************************************************
*
*       LCD_X_WriteM1_8
*
* Purpose:
*   Write multiple bytes to controller, with A0 = 1
*/
void LCD_X_WriteM1_8(U8 * pData, int NumItems) {
  LCD_WRX(1);
  LCD_CSX(0);
  LCD_WriteM(pData, NumItems);
  LCD_CSX(1);
}

/*********************************************************************
*
*       LCD_X_ReadM1_8
*
* Purpose:
*   Read multiple bytes from controller, with A0 = 1
*/
void LCD_X_ReadM1_8(U8 * pData, int NumItems) {
  LCD_WRX(1);
  LCD_CSX(0);
  LCD_ReadM(pData, NumItems);
  LCD_CSX(1);
}

/*************************** End of file ****************************/
