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
Purpose     : Port routines for STM32F4xx 16-bit Interface
----------------------------------------------------------------------
*/

#include "GUI.h"
#include "LCD_X.h"

/*********************************************************************
*
*       Hardware configuration
*
**********************************************************************
*/

#include "stm32f4xx_hal.h"

#ifndef FSMC_Bank1_NORSRAM4     /* defined in ST Peripheral Library */
  #define FSMC_Bank1_NORSRAM4      ((uint32_t)0x00000006)
#endif

/* LCD /CS is NE4 - Bank 4 of NOR/SRAM Bank 1~4                     */
#define LCD_BASE   (0x60000000UL | 0x0C000000UL)

#define LCD_REG16  (*((volatile U16 *)(LCD_BASE  ))) 
#define LCD_DAT16  (*((volatile U16 *)(LCD_BASE+2)))

static SRAM_HandleTypeDef hsram4;

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
  GPIO_InitTypeDef GPIO_InitStruct;
  FSMC_NORSRAM_TimingTypeDef Timing;

  /* GPIO Ports Clock Enable */
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();

  /* FSMC GPIO Configuration:
     PF0  -> FSMC_A0
     PE7  -> FSMC_D4
     PE8  -> FSMC_D5
     PE9  -> FSMC_D6
     PE10 -> FSMC_D7
     PE11 -> FSMC_D8
     PE12 -> FSMC_D9
     PE13 -> FSMC_D10
     PE14 -> FSMC_D11
     PE15 -> FSMC_D12
     PD8  -> FSMC_D13
     PD9  -> FSMC_D14
     PD10 -> FSMC_D15
     PD14 -> FSMC_D0
     PD15 -> FSMC_D1
     PD0  -> FSMC_D2
     PD1  -> FSMC_D3
     PD4  -> FSMC_NOE
     PD5  -> FSMC_NWE
     PG12 -> FSMC_NE4
  */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                        GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                        GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_14 |
                        GPIO_PIN_15 | GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4  |
                        GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* LCD Backlight: PC07 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* Enable FSMC peripheral clock */
  __FSMC_CLK_ENABLE();

  /* SRAM4 memory initialization */
  hsram4.Instance = FSMC_NORSRAM_DEVICE;
  hsram4.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

  hsram4.Init.NSBank = FSMC_NORSRAM_BANK4;
  hsram4.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram4.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram4.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram4.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram4.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram4.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram4.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram4.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram4.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram4.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram4.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram4.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

  Timing.AddressSetupTime = 1;
  Timing.AddressHoldTime = 0;
  Timing.DataSetupTime = 11;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 0;
  Timing.DataLatency = 0;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;

  HAL_SRAM_Init(&hsram4, &Timing, NULL);

  /* Turn LCD Backlight On */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
}

/*********************************************************************
*
*       LCD_X_Write0_16
*
* Purpose:
*   Write to controller, with A0 = 0
*/
void LCD_X_Write0_16(U16 c) {
  LCD_REG16 = c;
}

/*********************************************************************
*
*       LCD_X_Read0_16
*
* Purpose:
*   Read from controller, with A0 = 0
*/
U16 LCD_X_Read0_16(void) {
  return (LCD_REG16);
}

/*********************************************************************
*
*       LCD_X_Write1_16
*
* Purpose:
*   Write to controller, with A0 = 1
*/
void LCD_X_Write1_16(U16 c) {
  LCD_DAT16 = c;
}

/*********************************************************************
*
*       LCD_X_Read1_16
*
* Purpose:
*   Read from controller, with A0 = 1
*/
U16 LCD_X_Read1_16(void) {
  return (LCD_DAT16);
}

/*********************************************************************
*
*       LCD_X_WriteM1_16
*
* Purpose:
*   Write multiple bytes to controller, with A0 = 1
*/
void LCD_X_WriteM1_16(U16 * pData, int NumItems) {
  for (; NumItems; NumItems--) {
    LCD_DAT16 = *pData++;
  }
}

/*********************************************************************
*
*       LCD_X_ReadM1_16
*
* Purpose:
*   Read multiple bytes from controller, with A0 = 1
*/
void LCD_X_ReadM1_16(U16 * pData, int NumItems) {
  for (; NumItems; NumItems--) {
    *pData++ = LCD_DAT16;
  }
}

/*************************** End of file ****************************/
