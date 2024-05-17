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
 * Name:    Camera_OVM7690.c
 * Purpose: Digital camera OVM7690 interface
 * Rev.:    1.0.0
 *----------------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"
#include "Board_Camera.h"


//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> Digital Camera Configuration
//   <o0> I2C Port <1=>I2C1 <2=>I2C2 <3=>I2C3
#define CAMERA_I2C_PORT         1

//   <h> DMA
//     <o0> Number <2=>2
//     <i>  Selects DMA Number (only DMA2 can be used)
//     <o1> Stream <1=>1 <7=>7
//     <i>  Selects DMA Stream (only Stream 1 or 7 can be used)
//     <o2> Channel <1=>1
//     <i>  Selects DMA Channel (only Channel 1 can be used)
//     <o3> Priority <0=>Low <1=>Medium <2=>High <3=>Very High
//     <i>  Selects DMA Priority
//   </h>

#define DCMI_DMA_NUMBER         2       ///< Set DMA number
#define DCMI_DMA_STREAM         1       ///< Set DMA stream
#define DCMI_DMA_CHANNEL        1       ///< Set DMA channel
#define DCMI_DMA_PRIORITY       2       ///< Set DMA priority
// </h>


#define CAMERA_I2C_ADDR         0x21    /* 7-bit digital camera I2C address  */

/* I2C Driver */
#define _I2C_Driver_(n)  Driver_I2C##n
#define  I2C_Driver_(n) _I2C_Driver_(n)
extern ARM_DRIVER_I2C    I2C_Driver_(CAMERA_I2C_PORT);
#define ptrI2C         (&I2C_Driver_(CAMERA_I2C_PORT))


/* DMA source address */
#define DCMI_SRC_ADDR   0x50050028      /* DMA source address                 */


/* Camera register structure */
typedef struct { 
  uint8_t addr;                         /* Register address                   */
  uint8_t val;                          /* Register value                     */
} CAMERA_REG;


/* OmniVision recommended settings based on OVM7690 Setting V2.2              */
/* Modified for RGB QVGA settings                                             */
static CAMERA_REG Camera_RegInit[] = {

  {0x0E, 0x00},                         /* Normal operation mode              */
  {0x0C, 0x46},
  {0x81, 0xFF},
  {0x21, 0x44},
  {0x16, 0x03},
  {0x39, 0x80},
  {0x1E, 0xB1},
  
  /* Format */
  {0x12, 0x06},                         /* Output format control: RGB         */
  {0x82, 0x03},
  {0xD0, 0x48},
  {0x80, 0x7F},
  {0x3E, 0x30},
  {0x22, 0x00},
  
  /* Resolution */
  {0x17, 0x69},                         /* Horizontal window start point      */
  {0x18, 0xA4},                         /* Horizontal senzor size             */
  {0x19, 0x0C},                         /* Vertical Window start line         */
  {0x1A, 0xF6},                         /* Vertical sensor size               */
  
  {0xC8, 0x02},
  {0xC9, 0x80},
  {0xCA, 0x01},
  {0xCB, 0xE0},  
  {0xCC, 0x02},
  {0xCD, 0x80},
  {0xCE, 0x01},
  {0xCF, 0xE0},
  
  /* Lens Correction */
  {0x85, 0x90},
  {0x86, 0x00},
  {0x87, 0x00},
  {0x88, 0x10},
  {0x89, 0x30},
  {0x8A, 0x29},
  {0x8B, 0x26},
  
  /* Color Matrix */
  {0xBB, 0x80},
  {0xBC, 0x62},
  {0xBD, 0x1E},
  {0xBE, 0x26},
  {0xBF, 0x7B},
  {0xC0, 0xAC},
  {0xC1, 0x1E},
  
  /* Edge + Denoise */
  {0xB7, 0x05},
  {0xB8, 0x09},
  {0xB9, 0x00},
  {0xBA, 0x18},
  
  /* UVAdjust */
  {0x5A, 0x4A},
  {0x5B, 0x9F},
  {0x5C, 0x48},
  {0x5D, 0x32},
  
  /* AEC/AGC target */
  {0x24, 0x78},
  {0x25, 0x68},
  {0x26, 0xB3},
  
  /* Gamma */
  {0xA3, 0x0B},
  {0xA4, 0x15},
  {0xA5, 0x2A},
  {0xA6, 0x51},
  {0xA7, 0x63},
  {0xA8, 0x74},
  {0xA9, 0x83},
  {0xAA, 0x91},
  {0xAB, 0x9E},
  {0xAC, 0xAA},
  {0xAD, 0xBE},
  {0xAE, 0xCE},
  {0xAF, 0xE5},
  {0xB0, 0xF3},
  {0xB1, 0xFB},
  {0xB2, 0x06},
  
  /* Advance */
  {0x8C, 0x5D},
  {0x8D, 0x11},
  {0x8E, 0x12},
  {0x8F, 0x11},
  {0x90, 0x50},
  {0x91, 0x22},
  {0x92, 0xD1},
  {0x93, 0xA7},
  {0x94, 0x23},
  {0x95, 0x3B},
  {0x96, 0xFF},
  {0x97, 0x00},
  {0x98, 0x4A},
  {0x99, 0x46},
  {0x9A, 0x3D},
  {0x9B, 0x3A},
  {0x9C, 0xF0},
  {0x9D, 0xF0},
  {0x9E, 0xF0},
  {0x9F, 0xFF},
  {0xA0, 0x56},
  {0xA1, 0x55},
  {0xA2, 0x13},
  
  /* General Control */
  {0x50, 0x9A},
  {0x51, 0x80},
  {0x21, 0x23},
  
  {0x14, 0x29},
  {0x13, 0xF7},
  {0x11, 0x01},
  
  {0x0E, 0x00},

  /* QVGA/RGB565 capture */
  {0xC8, 0x02},
  {0xC9, 0x80},
  {0xCA, 0x01},
  {0xCB, 0xE0},
  {0xCC, 0x01},
  {0xCd, 0x40},
  {0xCE, 0x00},
  {0xCF, 0xF0}
};

static uint32_t           FrameBufAddr;
static DMA_HandleTypeDef  hdma_dcmi;
static DCMI_HandleTypeDef hdcmi;


/**
  \fn          int32_t Camera_Read (uint8_t reg, uint8_t *val)
  \brief       Read value from Camera register
  \param[in]   reg    Register address
  \param[out]  val    Pointer where data will be read from register
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
static int32_t Camera_Read (uint8_t reg, uint8_t *val) {
  uint8_t data[1];

  data[0] = reg;
  ptrI2C->MasterTransmit(CAMERA_I2C_ADDR, data, 1, true);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 1) return -1;
  ptrI2C->MasterReceive (CAMERA_I2C_ADDR, val, 1, false);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 1) return -1;

  return 0;
}

/**
  \fn          int32_t Camera_WriteData (uint8_t reg, const uint8_t *val)
  \brief       Write value to Camera register
  \param[in]   reg    Register address
  \param[in]   val    Pointer with data to write to register
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
static int32_t Camera_Write (uint8_t reg, uint8_t val) {
  uint8_t data[2];

  data[0] = reg;
  data[1] = val;
  ptrI2C->MasterTransmit(CAMERA_I2C_ADDR, data, 2, false);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 2) return -1;

  return 0;
}


/**
  \fn          void Camera_SetQVGA (void)
  \brief       Configure display size to QVGA (240*320)
*/
static void Camera_SetQVGA (void) {
  uint32_t i;

  for (i = 0; i < (sizeof(Camera_RegInit)/sizeof(CAMERA_REG)); i++) {
    Camera_Write(Camera_RegInit[i].addr, Camera_RegInit[i].val);
  }
}


/**
  \fn          int32_t Camera_Initialize (uint32_t frame_buf_addr)
  \brief       Initialize digital camera
  \param[in]   frame_buf_addr  Framebuffer address
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Camera_Initialize (uint32_t frame_buf_addr) {
  GPIO_InitTypeDef GPIO_InitStruct;
  uint8_t val;
  
  FrameBufAddr = frame_buf_addr;

  ptrI2C->Initialize  (NULL);
  ptrI2C->PowerControl(ARM_POWER_FULL);
  ptrI2C->Control     (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  ptrI2C->Control     (ARM_I2C_BUS_CLEAR, 0);

  /* Configure camera size */
  Camera_SetQVGA();
  
  Camera_Read (0x6F, &val);
  val &= ~(1 << 7);
  Camera_Write(0x6F,  val);

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();
  __GPIOI_CLK_ENABLE();

  /* DCMI GPIO Configuration:
     PA6  -> DCMI_PIXCK
     PH8  -> DCMI_HSYNC
     PH9  -> DCMI_D0
     PH10 -> DCMI_D1
     PH11 -> DCMI_D2
     PH12 -> DCMI_D3
     PH14 -> DCMI_D4
     PI4  -> DCMI_D5
     PI5  -> DCMI_VSYNC
     PI6  -> DCMI_D6
     PI7  -> DCMI_D7 
  */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                        GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /* Enable DCMI and DMA peripheral clock */
  __DCMI_CLK_ENABLE();
  __DMA2_CLK_ENABLE();

  /* Configure DMA */
  hdma_dcmi.Instance = DMA2_Stream1;
  hdma_dcmi.Init.Channel = DMA_CHANNEL_1;
  hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_dcmi.Init.MemInc = DMA_MINC_DISABLE;
  hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_dcmi.Init.Mode = DMA_CIRCULAR;
  hdma_dcmi.Init.Priority = DMA_PRIORITY_LOW;
  hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_dcmi.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_dcmi.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_dcmi.Init.PeriphBurst = DMA_PBURST_SINGLE;
  HAL_DMA_Init(&hdma_dcmi);

  /* Configure DCMI peripheral */
  hdcmi.Instance = DCMI;
  hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  HAL_DCMI_Init(&hdcmi);

  return 0;
}

/**
  \fn          int32_t Camera_Uninitialize (void)
  \brief       De-initialize digital camera
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Camera_Uninitialize (void) {

  /* DCMI GPIO Configuration:
     PA6  -> DCMI_PIXCK
     PH8  -> DCMI_HSYNC
     PH9  -> DCMI_D0
     PH10 -> DCMI_D1
     PH11 -> DCMI_D2
     PH12 -> DCMI_D3
     PH14 -> DCMI_D4
     PI4  -> DCMI_D5
     PI5  -> DCMI_VSYNC
     PI6  -> DCMI_D6
     PI7  -> DCMI_D7 
  */
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                         GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14);
  HAL_GPIO_DeInit(GPIOI, GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6 | GPIO_PIN_7);

  /* Disable DCMI clock */
  __DCMI_CLK_DISABLE();

  return 0;
}

/**
  \fn          int32_t Camera_On (void)
  \brief       Turn on digital camera
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Camera_On (void) {
  uint8_t val;

  if (Camera_Read(0x0E, &val) == 0) {
    val &= ~(1 << 3);

    if (Camera_Write(0x0E, val) == 0) { /* Put camera into normal mode        */
      HAL_DMA_Start(&hdma_dcmi, DCMI_SRC_ADDR, FrameBufAddr, 1);
      __HAL_DCMI_ENABLE (&hdcmi);
      HAL_DCMI_Start_DMA (&hdcmi, DCMI_MODE_CONTINUOUS, FrameBufAddr, 1);
      return 0;
    }
  }
  return -1;
}

/**
  \fn          int32_t Camera_Off (void)
  \brief       Turn off digital camera
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Camera_Off (void) {
  uint8_t val;

  HAL_DMA_Abort(&hdma_dcmi);
  __HAL_DCMI_DISABLE (&hdcmi);
  HAL_DCMI_Stop (&hdcmi);

  if (Camera_Read(0x0E, &val) == 0) {
    /* Put camera into sleep mode */
    if (Camera_Write(0x0E, val | (1<<3)) == 0) {
      return 0;
    }
  }
  return -1;
}
