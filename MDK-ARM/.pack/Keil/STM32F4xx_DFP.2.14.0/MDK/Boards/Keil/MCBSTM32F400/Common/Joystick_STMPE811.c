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
 * Name:    Joystick_STMPE811.c
 * Purpose: Joystick STMPE811 interface
 * Rev.:    1.0.1
 *----------------------------------------------------------------------------*/

#include "STMPE811.h"
#include "Driver_I2C.h"
#include "Board_Joystick.h"
#include "RTE_Components.h"             // Component selection

#ifdef RTE_CMSIS_RTOS2
#include "cmsis_os2.h"
#endif

#ifdef RTE_CMSIS_RTOS
#include "cmsis_os.h"
#endif

#ifndef TSC_I2C_PORT
#define TSC_I2C_PORT    1               /* I2C Port number                    */
#endif

#define TSC_I2C_ADDR    0x44            /* 7-bit I2C address                  */

/* I2C Driver */
#define _I2C_Driver_(n)  Driver_I2C##n
#define  I2C_Driver_(n) _I2C_Driver_(n)
extern ARM_DRIVER_I2C    I2C_Driver_(TSC_I2C_PORT);
#define ptrI2C         (&I2C_Driver_(TSC_I2C_PORT))


/**
  \fn          int32_t Touch_ReadRegister (uint8_t reg, uint8_t *val)
  \brief       Read register value from Touchscreen controller
  \param[in]   reg    Register address
  \param[out]  val    Pointer where data will be read from register
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
static int32_t Touch_Read (uint8_t reg, uint8_t *val) {
  uint8_t data[1];

  data[0] = reg;
  ptrI2C->MasterTransmit(TSC_I2C_ADDR, data, 1, true);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 1) return -1;
  ptrI2C->MasterReceive (TSC_I2C_ADDR, val, 1, false);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 1) return -1;

  return 0;
}

/**
  \fn          int32_t Touch_WriteData (uint8_t reg, const uint8_t *val)
  \brief       Write value to Touchscreen controller register
  \param[in]   reg    Register address
  \param[in]   val    Pointer with data to write to register
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
static int32_t Touch_Write (uint8_t reg, uint8_t val) {
  uint8_t data[2];

  data[0] = reg;
  data[1] = val;
  ptrI2C->MasterTransmit(TSC_I2C_ADDR, data, 2, false);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 2) return -1;

  return 0;
}


/**
  \fn          int32_t Joystick_Initialize (void)
  \brief       Initialize joystick
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Joystick_Initialize (void) {
  ptrI2C->Initialize  (NULL);
  ptrI2C->PowerControl(ARM_POWER_FULL);
  ptrI2C->Control     (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  ptrI2C->Control     (ARM_I2C_BUS_CLEAR, 0);

  Touch_Write(STMPE811_SYS_CTRL1, 0x02);  /* Reset Touch-screen controller      */
  osDelay(10);                              /* Wait 10ms                     */

  Touch_Write(STMPE811_SYS_CTRL2, 0x03);  /* Enable only GPIO and temp. sensor  */
  Touch_Write(STMPE811_INT_CTRL,  0x03);  /* INT is active low edge interrupt   */
  Touch_Write(STMPE811_INT_EN,    0xA0);  /* GPIO and temp. sens. int enabled   */
  Touch_Write(STMPE811_GPIO_EN,   0xFF);  /* All GPIOs are generating interrupt */
  Touch_Write(STMPE811_GPIO_DIR,  0x00);  /* GPIOs direction to input state     */
  Touch_Write(STMPE811_GPIO_FE,   0xFF);  /* Detect falling edge on all GPIOs   */
  Touch_Write(STMPE811_GPIO_AF,   0xFF);  /* Configure all pins as GPIO         */

  return 0;
}

/**
  \fn          int32_t Joystick_Uninitialize (void)
  \brief       De-initialize joystick
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Joystick_Uninitialize (void) {
  Touch_Write(STMPE811_SYS_CTRL1, 0x02);  /* Reset Touch-screen controller      */
  return 0;
}

/**
  \fn          uint32_t Joystick_GetState (void)
  \brief       Get joystick state
  \returns     Joystick state
*/
uint32_t Joystick_GetState (void) {
  uint32_t state = 0;
  uint8_t  val;

  /* Read Joystick inputs */
  if (Touch_Read(STMPE811_GPIO_MP_STA, &val) == 0) {
    if ((val & (1 << 3)) == 0) state |= JOYSTICK_UP;
    if ((val & (1 << 4)) == 0) state |= JOYSTICK_RIGHT;
    if ((val & (1 << 5)) == 0) state |= JOYSTICK_LEFT;
    if ((val & (1 << 6)) == 0) state |= JOYSTICK_DOWN;
    if ((val & (1 << 7)) == 0) state |= JOYSTICK_CENTER;
  }
  return state;
}
