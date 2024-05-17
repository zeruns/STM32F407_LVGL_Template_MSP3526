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
 * Name:    Gyroscope_L3G4200D.h
 * Purpose: Gyroscope L3G4200D & L3GD20 interface
 * Rev.:    1.0.1
 *----------------------------------------------------------------------------*/

#include "Driver_I2C.h"
#include "Board_Gyroscope.h"

#ifndef GYROSCOPE_I2C_PORT
#define GYROSCOPE_I2C_PORT      1       /* I2C Port number                    */
#endif

#define GYROSCOPE_I2C_ADDR      0x68    /* Gyroscope 7-bit I2C address        */
#define GYROSCOPE_ID            0xD3    /* Gyroscope ID (Who am I value)      */

#define GYROSCOPE_I2C_ADDR_2    0x6A    /* Gyroscope 7-bit I2C address 2      */
#define GYROSCOPE_ID_2          0xD4    /* This driver also works with L3GD20 */

/* I2C Driver */
#define _I2C_Driver_(n)  Driver_I2C##n
#define  I2C_Driver_(n) _I2C_Driver_(n)
extern ARM_DRIVER_I2C    I2C_Driver_(GYROSCOPE_I2C_PORT);
#define ptrI2C         (&I2C_Driver_(GYROSCOPE_I2C_PORT))

/* Gyroscope register addresses */
#define WHO_AM_I                0x0F
#define CTRL_REG1               0x20
#define CTRL_REG4               0x23
#define CTRL_REG5               0x24
#define OUT_X_L                 0x28
#define OUT_X_H                 0x29
#define OUT_Y_L                 0x2A
#define OUT_Y_H                 0x2B
#define OUT_Z_L                 0x2C
#define OUT_Z_H                 0x2D

static uint8_t DeviceAddr;


/**
  \fn          int32_t Gyroscope_Read (uint8_t reg, uint8_t *val)
  \brief       Read value from Gyroscope register
  \param[in]   reg    Register address
  \param[out]  val    Pointer where data will be read from register
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
static int32_t Gyroscope_Read (uint8_t reg, uint8_t *val) {
  uint8_t data[1];

  data[0] = reg;
  ptrI2C->MasterTransmit(DeviceAddr, data, 1, true);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 1) return -1;
  ptrI2C->MasterReceive (DeviceAddr, val, 1, false);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 1) return -1;

  return 0;
}

/**
  \fn          int32_t Gyroscope_WriteData (uint8_t reg, const uint8_t *val)
  \brief       Write value to Gyroscope register
  \param[in]   reg    Register address
  \param[in]   val    Pointer with data to write to register
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
static int32_t Gyroscope_Write (uint8_t reg, uint8_t val) {
  uint8_t data[2];

  data[0] = reg;
  data[1] = val;
  ptrI2C->MasterTransmit(DeviceAddr, data, 2, false);
  while (ptrI2C->GetStatus().busy);
  if (ptrI2C->GetDataCount() != 2) return -1;

  return 0;
}


/**
  \fn          int32_t Gyroscope_Initialize (void)
  \brief       Initialize gyroscope
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Gyroscope_Initialize (void) {
  uint8_t who = 0;
  int32_t res;

  ptrI2C->Initialize  (NULL);
  ptrI2C->PowerControl(ARM_POWER_FULL);
  ptrI2C->Control     (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  ptrI2C->Control     (ARM_I2C_BUS_CLEAR, 0);

  /* Init L3G4200D device */
  DeviceAddr = GYROSCOPE_I2C_ADDR;
  res = Gyroscope_Read(WHO_AM_I, &who);
  if (res == 0) {
    if (who != GYROSCOPE_ID) {
      return -1;                        /* Wrong ID                           */
    }
  }
  else {
    /* Check if this is L3GD20 device (L3G4200D compatible) */
    DeviceAddr = GYROSCOPE_I2C_ADDR_2;
    res = Gyroscope_Read(WHO_AM_I, &who);
    if (res == 0) {
      if (who != GYROSCOPE_ID_2) {
        return -1;                      /* Wrong ID                           */
      }
    }
    else {
      return -1;                        /* Device not responding              */
    }
  }

  Gyroscope_Write(CTRL_REG1, 0x0F);     /* ODR: 100Hz, Cut-Off: 12.5,         */
  Gyroscope_Write(CTRL_REG4, 0x00);     /* Continuous update,full scale 250dps*/
  Gyroscope_Write(CTRL_REG5, 0x40);     /* FIFO Enable                        */

  return 0;
}


/**
  \fn          int32_t Gyroscope_Uninitialize (void)
  \brief       De-initialize gyroscope
  \returns
   - \b 0: function succeeded
   - \b -1: function failed
*/
int32_t Gyroscope_Uninitialize (void) {

  /* Put gyroscope into power-down mode */
  Gyroscope_Write(CTRL_REG1, 0x00);

  return 0;
}

/**
  \fn          int32_t Gyroscope_GetState (GYROSCOPE_STATE *pState)
  \brief       Get gyroscope state
  \param[out]  pState  pointer to GYROSCOPE_STATE structure
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Gyroscope_GetState (GYROSCOPE_STATE *pState) {
  uint8_t valL, valH;
  int16_t mg;

  Gyroscope_Read(OUT_X_L, &valL);
  Gyroscope_Read(OUT_X_H, &valH);
  mg = (int16_t)((valH << 8) | valL);
  pState->x = (mg*2500) >> 16;

  Gyroscope_Read(OUT_Y_L, &valL);
  Gyroscope_Read(OUT_Y_H, &valH);
  mg = (int16_t)((valH << 8) | valL);
  pState->y = (mg*2500) >> 16;

  Gyroscope_Read(OUT_Z_L, &valL);
  Gyroscope_Read(OUT_Z_H, &valH);  
  mg = (int16_t)((valH << 8) | valL);
  pState->z = (mg*2500) >> 16;

  return 0;
}
