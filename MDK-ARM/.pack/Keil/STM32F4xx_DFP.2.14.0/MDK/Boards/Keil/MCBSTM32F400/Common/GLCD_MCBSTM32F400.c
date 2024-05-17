/*-----------------------------------------------------------------------------
 * Copyright (c) 2013 - 2018 Arm Limited (or its affiliates). All
 * rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   3.Neither the name of Arm nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * Name:    GLCD_MCBSTM32F400.c
 * Purpose: Graphic LCD interface (240x320 pixels) for Graphic LCD with
 *          16-bit interface for MCBSTM32F400 evaluation board
 * Rev.:    1.0.1
 *----------------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"
#include "GLCD_Config.h"
#include "Board_GLCD.h"

/*********************** Hardware specific configuration **********************/

/*--------------- Graphic LCD interface hardware definitions -----------------*/

/* LCD /CS is NE4 - Bank 4 of NOR/SRAM Bank 1~4                               */
#define LCD_BASE   (0x60000000UL | 0x0C000000UL)          // LCD base address

#define LCD_REG16  (*((volatile uint16_t *)(LCD_BASE  ))) // LCD register address
#define LCD_DAT16  (*((volatile uint16_t *)(LCD_BASE+2))) // LCD data address

/************************ Global variables ************************************/

static uint16_t   foreground_color = GLCD_COLOR_BLACK;
static uint16_t   background_color = GLCD_COLOR_WHITE;
static GLCD_FONT *active_font      = NULL;

static uint8_t    Himax;
#if (GLCD_SWAP_XY == 0)
static uint32_t   Scroll;
#endif

static SRAM_HandleTypeDef hsram4;

/************************ Local auxiliary functions ***************************/

#define delay_ms HAL_Delay

/**
  \fn          void wr_cmd (uint8_t cmd)
  \brief       Write a command to the LCD controller
  \param[in]   cmd  Command to write
*/
static __inline void wr_cmd (uint8_t cmd) {
  LCD_REG16 = cmd;
}

/**
  \fn          void wr_dat (uint16_t dat)
  \brief       Write data to the LCD controller
  \param[in]   dat  Data to write
*/
static __inline void wr_dat (uint16_t dat) {
  LCD_DAT16 = dat;
}

/**
  \fn          void wr_dat_start (void)
  \brief       Start writing data to LCD controller
*/
static __inline void wr_dat_start (void) {
  /* only used for SPI interface */
}

/**
  \fn          void wr_dat_stop (void)
  \brief       Stop writing data to LCD controller
*/
static __inline void wr_dat_stop (void) {
  /* only used for SPI interface */
}

/**
  \fn          void wr_dat_only (uint16_t dat)
  \brief       Write data to the LCD controller (without start/stop sequence)
  \param[in]   dat  Data to write
*/
static __inline void wr_dat_only (uint16_t dat) {
  LCD_DAT16 = dat;
}

/**
  \fn          uint16_t rd_dat (void)
  \brief       Read data from the LCD controller
  \returns     Data read
*/
static __inline uint16_t rd_dat (void) {
  return LCD_DAT16;
}

/**
  \fn          void wr_reg (uint8_t reg, uint16_t val)
  \brief       Write a value to the LCD register
  \param[in]   reg  Register to be written
  \param[in]   val  Value to write to the register
*/
static __inline void wr_reg (uint8_t reg, uint16_t val) {
  wr_cmd(reg);
  wr_dat(val);
}

/**
  \fn          uint16_t rd_reg (uint8_t reg)
  \brief       Read a value from the LCD register
  \param[in]   reg  Register to be read
  \returns     Value read from the register
*/
static __inline uint16_t rd_reg (uint8_t reg) {
  wr_cmd(reg);
  return (rd_dat());
}


/**
  \fn          void GLCD_SetWindow (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
  \brief       Set draw window region
  \param[in]   x  Horizontal position in pixel
  \param[in]   y  Vertical position in pixel
  \param[in]   w  Window width in pixel
  \param[in]   h  Window height in pixel
*/
static void GLCD_SetWindow (uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  uint32_t xe, ye;

  if (Himax) {
    xe = x+w-1;
    ye = y+h-1;

    wr_reg(0x02U, (uint16_t)(x  >>    8));  /* Column address start MSB           */
    wr_reg(0x03U, (uint16_t)(x  &  0xFF));  /* Column address start LSB           */
    wr_reg(0x04U, (uint16_t)(xe >>    8));  /* Column address end MSB             */
    wr_reg(0x05U, (uint16_t)(xe &  0xFF));  /* Column address end LSB             */
  
    wr_reg(0x06U, (uint16_t)(y  >>    8));  /* Row address start MSB              */
    wr_reg(0x07U, (uint16_t)(y  &  0xFF));  /* Row address start LSB              */
    wr_reg(0x08U, (uint16_t)(ye >>    8));  /* Row address end MSB                */
    wr_reg(0x09U, (uint16_t)(ye &  0xFF));  /* Row address end LSB                */
  }
  else {
  #if (GLCD_SWAP_XY == 1)
    wr_reg(0x50U, (uint16_t)(y));           /* Vertical   GRAM Start Address      */
    wr_reg(0x51U, (uint16_t)(y+h-1));       /* Vertical   GRAM End   Address (-1) */
    wr_reg(0x52U, (uint16_t)(x));           /* Horizontal GRAM Start Address      */
    wr_reg(0x53U, (uint16_t)(x+w-1));       /* Horizontal GRAM End   Address (-1) */
    wr_reg(0x20U, (uint16_t)(y));
    wr_reg(0x21U, (uint16_t)(x));
  #else
    wr_reg(0x50U, (uint16_t)(x));           /* Horizontal GRAM Start Address      */
    wr_reg(0x51U, (uint16_t)(x+w-1));       /* Horizontal GRAM End   Address (-1) */
    wr_reg(0x52U, (uint16_t)(y));           /* Vertical   GRAM Start Address      */
    wr_reg(0x53U, (uint16_t)(y+h-1));       /* Vertical   GRAM End   Address (-1) */
    wr_reg(0x20U, (uint16_t)(x));
    wr_reg(0x21U, (uint16_t)(y));
  #endif
  }
}


/************************ Exported functions **********************************/

/**
  \fn          int32_t GLCD_Initialize (void)
  \brief       Initialize Graphic LCD
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_Initialize (void) {
  GPIO_InitTypeDef GPIO_InitStruct;
  FSMC_NORSRAM_TimingTypeDef Timing;
  uint16_t driverCode;

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

  delay_ms(50);                         /* Delay 50 ms                        */
  driverCode = rd_reg(0x00);

  if (driverCode == 0x47) {             /* LCD with HX8347-D LCD Controller   */
    Himax = 1;                          /* Set Himax LCD controller flag      */
    /* Driving ability settings ----------------------------------------------*/
    wr_reg(0xEA, 0x00);                 /* Power control internal used (1)    */
    wr_reg(0xEB, 0x20);                 /* Power control internal used (2)    */
    wr_reg(0xEC, 0x0C);                 /* Source control internal used (1)   */
    wr_reg(0xED, 0xC7);                 /* Source control internal used (2)   */
    wr_reg(0xE8, 0x38);                 /* Source output period Normal mode   */
    wr_reg(0xE9, 0x10);                 /* Source output period Idle mode     */
    wr_reg(0xF1, 0x01);                 /* RGB 18-bit interface ;0x0110       */
    wr_reg(0xF2, 0x10);

    /* Adjust the Gamma Curve ------------------------------------------------*/
    wr_reg(0x40, 0x01);
    wr_reg(0x41, 0x00);
    wr_reg(0x42, 0x00);
    wr_reg(0x43, 0x10);
    wr_reg(0x44, 0x0E);
    wr_reg(0x45, 0x24);
    wr_reg(0x46, 0x04);
    wr_reg(0x47, 0x50);
    wr_reg(0x48, 0x02);
    wr_reg(0x49, 0x13);
    wr_reg(0x4A, 0x19);
    wr_reg(0x4B, 0x19);
    wr_reg(0x4C, 0x16);

    wr_reg(0x50, 0x1B);
    wr_reg(0x51, 0x31);
    wr_reg(0x52, 0x2F);
    wr_reg(0x53, 0x3F);
    wr_reg(0x54, 0x3F);
    wr_reg(0x55, 0x3E);
    wr_reg(0x56, 0x2F);
    wr_reg(0x57, 0x7B);
    wr_reg(0x58, 0x09);
    wr_reg(0x59, 0x06);
    wr_reg(0x5A, 0x06);
    wr_reg(0x5B, 0x0C);
    wr_reg(0x5C, 0x1D);
    wr_reg(0x5D, 0xCC);

    /* Power voltage setting -------------------------------------------------*/
    wr_reg(0x1B, 0x1B);
    wr_reg(0x1A, 0x01);
    wr_reg(0x24, 0x2F);
    wr_reg(0x25, 0x57);
    wr_reg(0x23, 0x88);

    /* Power on setting ------------------------------------------------------*/
    wr_reg(0x18, 0x36);                 /* Internal oscillator frequency adj  */
    wr_reg(0x19, 0x01);                 /* Enable internal oscillator         */
    wr_reg(0x01, 0x00);                 /* Normal mode, no scrool             */
    wr_reg(0x1F, 0x88);                 /* Power control 6 - DDVDH Off        */
    delay_ms(200);
    wr_reg(0x1F, 0x82);                 /* Power control 6 - Step-up: 3 x VCI */
    delay_ms(50);
    wr_reg(0x1F, 0x92);                 /* Power control 6 - Step-up: On      */
    delay_ms(50);
    wr_reg(0x1F, 0xD2);                 /* Power control 6 - VCOML active     */
    delay_ms(50);

    /* Color selection -------------------------------------------------------*/
    wr_reg(0x17, 0x55);                 /* RGB, System interface: 16 Bit/Pixel*/
    wr_reg(0x00, 0x00);                 /* Scrolling off, no standby          */

    /* Interface config ------------------------------------------------------*/
    wr_reg(0x2F, 0x11);                 /* LCD Drive: 1-line inversion        */
    wr_reg(0x31, 0x00);
    wr_reg(0x32, 0x00);                 /* DPL=0, HSPL=0, VSPL=0, EPL=0       */

    /* Display on setting ----------------------------------------------------*/
    wr_reg(0x28, 0x38);                 /* PT(0,0) active, VGL/VGL            */
    delay_ms(200);
    wr_reg(0x28, 0x3C);                 /* Display active, VGL/VGL            */

    /* Miror and Swap                                                         */
    wr_reg (0x16, ((GLCD_MIRROR_Y << 7) |   /* MY                             */
                   (GLCD_MIRROR_X << 6) |   /* MX                             */
                   (GLCD_SWAP_XY  << 5) |   /* MV                             */
                   (0 << 4) |               /* ML                             */
                   (1 << 3)));              /* BGR                            */

    /* Display scrolling settings --------------------------------------------*/
    wr_reg(0x0E, 0x00);                 /* TFA MSB                            */
    wr_reg(0x0F, 0x00);                 /* TFA LSB                            */
    wr_reg(0x10, 320 >> 8);             /* VSA MSB                            */
    wr_reg(0x11, 320 &  0xFF);          /* VSA LSB                            */
    wr_reg(0x12, 0x00);                 /* BFA MSB                            */
    wr_reg(0x13, 0x00);                 /* BFA LSB                            */
  }
  else {
    Himax = 0;                          /* This is not Himax LCD controller   */
    /* Start Initial Sequence ------------------------------------------------*/
    #if (GLCD_MIRROR_X)
    wr_reg(0x01, 0x0000);               /* Clear SS bit                       */
    #else
    wr_reg(0x01, 0x0100);               /* Set SS bit                         */
    #endif
    wr_reg(0x02, 0x0700);               /* Set 1 line inversion               */
    wr_reg(0x04, 0x0000);               /* Resize register                    */
    wr_reg(0x08, 0x0207);               /* 2 lines front, 7 back porch        */
    wr_reg(0x09, 0x0000);               /* Set non-disp area refresh cyc ISC  */
    wr_reg(0x0A, 0x0000);               /* FMARK function                     */
    wr_reg(0x0C, 0x0000);               /* RGB interface setting              */
    wr_reg(0x0D, 0x0000);               /* Frame marker Position              */
    wr_reg(0x0F, 0x0000);               /* RGB interface polarity             */

    /* Power On sequence -----------------------------------------------------*/
    wr_reg(0x10, 0x0000);               /* Reset Power Control 1              */
    wr_reg(0x11, 0x0000);               /* Reset Power Control 2              */
    wr_reg(0x12, 0x0000);               /* Reset Power Control 3              */
    wr_reg(0x13, 0x0000);               /* Reset Power Control 4              */
    delay_ms(200);                      /* Discharge cap power voltage (200ms)*/
    wr_reg(0x10, 0x12B0);               /* SAP, BT[3:0], AP, DSTB, SLP, STB   */
    wr_reg(0x11, 0x0007);               /* DC1[2:0], DC0[2:0], VC[2:0]        */
    delay_ms(50);                       /* Delay 50 ms                        */
    wr_reg(0x12, 0x01BD);               /* VREG1OUT voltage                   */
    delay_ms(50);                       /* Delay 50 ms                        */
    wr_reg(0x13, 0x1400);               /* VDV[4:0] for VCOM amplitude        */
    wr_reg(0x29, 0x000E);               /* VCM[4:0] for VCOMH                 */
    delay_ms(50);                       /* Delay 50 ms                        */
    wr_reg(0x20, 0x0000);               /* GRAM horizontal Address            */
    wr_reg(0x21, 0x0000);               /* GRAM Vertical Address              */

    /* Adjust the Gamma Curve ------------------------------------------------*/
    switch (driverCode) {
      case 0x5408:                      /* LCD with SPFD5408 LCD Controller   */
        wr_reg(0x30, 0x0B0D);
        wr_reg(0x31, 0x1923);
        wr_reg(0x32, 0x1C26);
        wr_reg(0x33, 0x261C);
        wr_reg(0x34, 0x2419);
        wr_reg(0x35, 0x0D0B);
        wr_reg(0x36, 0x1006);
        wr_reg(0x37, 0x0610);
        wr_reg(0x38, 0x0706);
        wr_reg(0x39, 0x0304);
        wr_reg(0x3A, 0x0E05);
        wr_reg(0x3B, 0x0E01);
        wr_reg(0x3C, 0x010E);
        wr_reg(0x3D, 0x050E);
        wr_reg(0x3E, 0x0403);
        wr_reg(0x3F, 0x0607);
        break;

      case 0x9325:                      /* LCD with RM68050 LCD Controller    */
        wr_reg(0x0030,0x0000);
        wr_reg(0x0031,0x0607);
        wr_reg(0x0032,0x0305);
        wr_reg(0x0035,0x0000);
        wr_reg(0x0036,0x1604);
        wr_reg(0x0037,0x0204);
        wr_reg(0x0038,0x0001);
        wr_reg(0x0039,0x0707);
        wr_reg(0x003C,0x0000);
        wr_reg(0x003D,0x000F);
        break;

      case 0x9320:                      /* LCD with ILI9320 LCD Controller    */
      default:                          /* LCD with other LCD Controller      */
        wr_reg(0x30, 0x0006);
        wr_reg(0x31, 0x0101);
        wr_reg(0x32, 0x0003);
        wr_reg(0x35, 0x0106);
        wr_reg(0x36, 0x0B02);
        wr_reg(0x37, 0x0302);
        wr_reg(0x38, 0x0707);
        wr_reg(0x39, 0x0007);
        wr_reg(0x3C, 0x0600);
        wr_reg(0x3D, 0x020B);
        break;
    }

    /* Set GRAM area ---------------------------------------------------------*/
    wr_reg(0x50, 0x0000);               /* Horizontal GRAM Start Address      */
    wr_reg(0x51, (GLCD_HEIGHT-1));      /* Horizontal GRAM End   Address      */
    wr_reg(0x52, 0x0000);               /* Vertical   GRAM Start Address      */
    wr_reg(0x53, (GLCD_WIDTH-1));       /* Vertical   GRAM End   Address      */

    /* Set Gate Scan Line ----------------------------------------------------*/
    switch (driverCode) {
      case 0x5408:                      /* LCD with SPFD5408 LCD Controller   */
      case 0x9325:                      /* LCD with RM68050 LCD Controller    */
      #if (GLCD_MIRROR_Y)
        wr_reg(0x60, 0x2700);
      #else
        wr_reg(0x60, 0xA700);
      #endif
        break;

      case 0x9320:                      /* LCD with ILI9320 LCD Controller    */
      default:                          /* LCD with other LCD Controller      */
      #if (GLCD_MIRROR_Y)
        wr_reg(0x60, 0xA700);
      #else
        wr_reg(0x60, 0x2700);
      #endif
        break;
    }
    wr_reg(0x61, 0x0001);               /* NDL,VLE, REV                       */
    wr_reg(0x6A, 0x0000);               /* Set scrolling line                 */

    /* Partial Display Control -----------------------------------------------*/
    wr_reg(0x80, 0x0000);
    wr_reg(0x81, 0x0000);
    wr_reg(0x82, 0x0000);
    wr_reg(0x83, 0x0000);
    wr_reg(0x84, 0x0000);
    wr_reg(0x85, 0x0000);

    /* Panel Control ---------------------------------------------------------*/
    wr_reg(0x90, 0x0010);
    wr_reg(0x92, 0x0000);
    wr_reg(0x93, 0x0003);
    wr_reg(0x95, 0x0110);
    wr_reg(0x97, 0x0000);
    wr_reg(0x98, 0x0000);

    /* Set GRAM write direction
       I/D=11 (Horizontal : increment, Vertical : increment)                  */
  #if (GLCD_SWAP_XY == 1)
    /* AM=1   (address is updated in vertical writing direction)              */
    wr_reg(0x03, 0x1038);
  #else
    /* AM=0   (address is updated in horizontal writing direction)            */
    wr_reg(0x03, 0x1030);
  #endif

    wr_reg(0x07, 0x0137);               /* 262K color and display ON          */
  }

  /* Turn LCD Backlight On */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

  return 0;
}

/**
  \fn          int32_t GLCD_Uninitialize (void)
  \brief       De-initialize Graphic LCD
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_Uninitialize (void) {

  HAL_SRAM_DeInit(&hsram4);

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
  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0);
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                         GPIO_PIN_15);
  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_14 |
                         GPIO_PIN_15 | GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4  |
                         GPIO_PIN_5);
  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_12);

  /* LCD Backlight: PC07 */
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7);

  return 0;
}

/**
  \fn          int32_t GLCD_SetForegroundColor (uint32_t color)
  \brief       Set foreground color
  \param[in]   color  Color value
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_SetForegroundColor (uint32_t color) {
  foreground_color = (uint16_t)color;
  return 0;
}

/**
  \fn          int32_t GLCD_SetBackgroundColor (uint32_t color)
  \brief       Set background color
  \param[in]   color  Color value
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_SetBackgroundColor (uint32_t color) {
  background_color = (uint16_t)color;
  return 0;
}

/**
  \fn          int32_t GLCD_ClearScreen (void)
  \brief       Clear screen (with active background color)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_ClearScreen (void) {
  uint32_t i;

  GLCD_SetWindow(0U, 0U, GLCD_WIDTH, GLCD_HEIGHT);

  wr_cmd(0x22U);
  wr_dat_start();
  for (i = 0U; i < (GLCD_WIDTH * GLCD_HEIGHT); i++) {
    wr_dat_only(background_color);
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_SetFont (GLCD_FONT *font)
  \brief       Set active font
  \param[in]   font   Pointer to font structure
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_SetFont (GLCD_FONT *font) {
  active_font = font;
  return 0;
}

/**
  \fn          int32_t GLCD_DrawPixel (uint32_t x, uint32_t y)
  \brief       Draw pixel (in active foreground color)
  \param[in]   x  x position in pixels (0 = left corner)
  \param[in]   y  y position in pixels (0 = upper corner)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawPixel (uint32_t x, uint32_t y) {
#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  if (Himax) {
    wr_reg(0x02U, (uint16_t)(x >>    8));   /* Column address start MSB           */
    wr_reg(0x03U, (uint16_t)(x &  0xFF));   /* Column address start LSB           */
    wr_reg(0x04U, (uint16_t)(x >>    8));   /* Column address end MSB             */
    wr_reg(0x05U, (uint16_t)(x &  0xFF));   /* Column address end LSB             */
  
    wr_reg(0x06U, (uint16_t)(y >>    8));   /* Row address start MSB              */
    wr_reg(0x07U, (uint16_t)(y &  0xFF));   /* Row address start LSB              */
    wr_reg(0x08U, (uint16_t)(y >>    8));   /* Row address end MSB                */
    wr_reg(0x09U, (uint16_t)(y &  0xFF));   /* Row address end LSB                */
  }
  else {
  #if (GLCD_SWAP_XY == 1)
    wr_reg(0x20U, (uint16_t)(y));
    wr_reg(0x21U, (uint16_t)(x));
  #else
    wr_reg(0x20u, (uint16_t)(x));
    wr_reg(0x21U, (uint16_t)(y));
  #endif
  }

  wr_cmd(0x22U);
  wr_dat(foreground_color);

  return 0;
}

/**
  \fn          int32_t GLCD_DrawHLine (uint32_t x, uint32_t y, uint32_t length)
  \brief       Draw horizontal line (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   length Line length
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawHLine (uint32_t x, uint32_t y, uint32_t length) {
#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, length, 1);
  wr_cmd(0x22);
  wr_dat_start();

  while (length--) { 
    wr_dat_only(foreground_color);
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_DrawVLine (uint32_t x, uint32_t y, uint32_t length)
  \brief       Draw vertical line (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   length Line length in pixels
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawVLine (uint32_t x, uint32_t y, uint32_t length) {
#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, 1, length);
  wr_cmd(0x22);
  wr_dat_start();

  while (length--) { 
    wr_dat_only(foreground_color);
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_DrawRectangle (uint32_t x, uint32_t y, uint32_t width, uint32_t height)
  \brief       Draw rectangle (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   width  Rectangle width in pixels
  \param[in]   height Rectangle height in pixels
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawRectangle (uint32_t x, uint32_t y, uint32_t width, uint32_t height) {

  GLCD_DrawHLine (x,         y,          width);
  GLCD_DrawHLine (x,         y + height, width);
  GLCD_DrawVLine (x,         y,          height);
  GLCD_DrawVLine (x + width, y,          height);

  return 0;
}

/**
  \fn          int32_t GLCD_DrawChar (uint32_t x, uint32_t y, int32_t ch)
  \brief       Draw character (in active foreground color)
  \param[in]   x      x position in pixels (0 = left corner)
  \param[in]   y      y position in pixels (0 = upper corner)
  \param[in]   ch     Character
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawChar (uint32_t x, uint32_t y, int32_t ch) {
  uint32_t i, j;
  uint32_t wb;
  uint8_t *ptr_ch_bmp;

#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT; 
#endif

  if (active_font == NULL) return -1;

  ch        -= active_font->offset;
  wb         = (active_font->width + 7)/8;
  ptr_ch_bmp = (uint8_t *)((uint32_t)active_font->bitmap + ((uint32_t)ch * wb * active_font->height));

  GLCD_SetWindow(x, y, active_font->width, active_font->height);

  wr_cmd(0x22);
  wr_dat_start();
  
  for (i = 0; i < active_font->height; i++) {
    for (j = 0; j < active_font->width; j++) {
      wr_dat_only(((*ptr_ch_bmp >> (j & 7)) & 1) ? foreground_color : background_color);
      if (((j & 7) == 7) && (j != (active_font->width - 1))) ptr_ch_bmp++;
    }
    ptr_ch_bmp++;
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_DrawString (uint32_t x, uint32_t y, const char *str)
  \brief       Draw string (in active foreground color)
  \param[in]   x      x position in pixels (0 = left corner)
  \param[in]   y      y position in pixels (0 = upper corner)
  \param[in]   str    Null-terminated String
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawString (uint32_t x, uint32_t y, const char *str) {

  while (*str) { GLCD_DrawChar(x, y, *str++); x += active_font->width; }

  return 0;
}


/**
  \fn          int32_t GLCD_DrawBargraph (uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t val)
  \brief       Draw bargraph (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   width  Full bargraph width in pixels
  \param[in]   height Full bargraph height in pixels
  \param[in]   val    Active bargraph value (maximum value is 100)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawBargraph (uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t val) {
  uint32_t i, j;

  val = (val * width) / 100;            /* Scale value                        */

#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, width, height);

  wr_cmd(0x22);
  wr_dat_start();
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      if(j >= val) {
        wr_dat_only(background_color);
      } else {
        wr_dat_only(foreground_color);
      }
    }
  }
  wr_dat_stop();

  return 0;
}

/**
  \fn          int32_t GLCD_DrawBitmap (uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap)
  \brief       Draw bitmap (bitmap from BMP file without header)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   width  Bitmap width in pixels
  \param[in]   height Bitmap height in pixels
  \param[in]   bitmap Bitmap data
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawBitmap (uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap) {
  int32_t   i;
  uint32_t  j;
  uint16_t *ptr_bmp;

  ptr_bmp = (uint16_t *)((uint32_t)bitmap);

#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, width, height);

  wr_cmd(0x22);
  wr_dat_start();
  for (i = (int32_t)((height-1)*width); i > -1; i -= width) {
    for (j = 0; j < width; j++) {
      wr_dat_only(ptr_bmp[(uint32_t)i+j]);
    }
  }
  wr_dat_stop();

  return 0;
}

/**
  \fn          int32_t GLCD_VScroll (uint32_t dy)
  \brief       Scroll picture on display vertically
  \param[in]   dy  Scroll size in pixels
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_VScroll (uint32_t dy) {
#if (GLCD_SWAP_XY == 0)
  uint32_t i;

  Scroll = (Scroll + dy) % GLCD_HEIGHT;

  if (Himax) {
    wr_reg(0x01, 0x08);
    wr_reg(0x14, (uint16_t)(Scroll >> 8));      /* VSP MSB                    */
    wr_reg(0x15, (uint16_t)(Scroll & 0xFF));    /* VSP LSB                    */
  }
  else {
    wr_reg(0x6A, (uint16_t)Scroll);
    wr_reg(0x61,  3);
  }

  /* Clear new space */
  GLCD_SetWindow(0, Scroll, GLCD_WIDTH, dy);

  wr_cmd(0x22);
  wr_dat_start();
  for(i = 0; i < (GLCD_WIDTH * dy); i++) {
    wr_dat_only(background_color);
  }
  wr_dat_stop();
  return (0);
#else
  (void)dy;
  return -1;
#endif
}

/**
  \fn          int32_t GLCD_FrameBufferAccess (bool enable)
  \brief       Enable or disable direct access to FrameBuffer
  \param[in]   enable Enable or disable request
                - true (1): enable
                - false(0): disable
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t  GLCD_FrameBufferAccess (bool enable) {
  (void)enable;

  GLCD_SetWindow(0, 0, GLCD_WIDTH, GLCD_HEIGHT);

  wr_reg (0x20, 0x0000);
  wr_reg (0x21, 0x0000);
  wr_cmd (0x22);
  return 0;
}

/**
  \fn          uint32_t GLCD_FrameBufferAddress (void)
  \brief       Retrieve FrameBuffer address
  \returns     FrameBuffer address
*/
uint32_t GLCD_FrameBufferAddress (void) {
  return (uint32_t)&LCD_DAT16;
}
