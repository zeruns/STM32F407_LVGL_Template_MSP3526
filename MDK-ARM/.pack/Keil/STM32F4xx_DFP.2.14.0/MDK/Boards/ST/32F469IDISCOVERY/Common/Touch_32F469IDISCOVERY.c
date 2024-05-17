/*-----------------------------------------------------------------------------
 * Copyright (c) 2018 Arm Limited (or its affiliates). All
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
 * Name:    Touch_32F469IDISCOVERY.c
 * Purpose: Touchscreen interface for 32F469IDISCOVERY Kit
 * Rev.:    1.0.0
 *----------------------------------------------------------------------------*/

#include "stm32469i_discovery_ts.h"
#include "Board_Touch.h"

/*---------------------- Touch Screen orientation configuration --------------*/
#ifndef TS_LANDSCAPE
#define TS_LANDSCAPE    0
#endif

/*---------------------- Touch Screen physical definitions--------------------*/
#define TS_X_MAX_VAL    480             /* Max X size of the TS area on LCD   */
#define TS_Y_MAX_VAL    800             /* Max Y size of the TS area on LCD   */

#if    (TS_LANDSCAPE == 0)
#define TS_X            TS_X_MAX_VAL
#define TS_Y            TS_Y_MAX_VAL
#else
#define TS_X            TS_Y_MAX_VAL
#define TS_Y            TS_X_MAX_VAL
#endif
/**
  \fn          int32_t Touch_Initialize (void)
  \brief       Initialize touchscreen
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Touch_Initialize (void) {
  uint8_t ret;

  ret = BSP_TS_Init(TS_X, TS_Y);

  if (ret != TS_OK) {
    return -1;
  }

  return 0;
}

/**
  \fn          int32_t Touch_Uninitialize (void)
  \brief       De-initialize touchscreen
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Touch_Uninitialize (void) {
  return TS_OK;
}

/**
  \fn          int32_t Touch_GetState (TOUCH_STATE *pState)
  \brief       Get touchscreen state
  \param[out]  pState  pointer to TOUCH_STATE structure
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t Touch_GetState (TOUCH_STATE *pState) {
  TS_StateTypeDef state;
  uint8_t ret;

  ret = BSP_TS_GetState(&state);

  if (ret != TS_OK) {
    return -1;
  }

  if (state.touchDetected) {
    pState->pressed = 1;
    pState->x = (int16_t)state.touchX[0];
    pState->y = (int16_t)state.touchY[0];
  } else {
    pState->pressed = 0;
  }

  return 0;
}
