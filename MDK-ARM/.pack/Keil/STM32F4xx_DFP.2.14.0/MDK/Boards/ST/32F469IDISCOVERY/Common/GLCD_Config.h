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
 * Name:    GLCD_Config.h
 * Purpose: Graphic LCD interface configuration file for 32F469IDISCOVERY Kit
 * Rev.:    1.0.0
 *----------------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"
#include "stm32469i_discovery_lcd.h"

#ifndef __GLCD_CONFIG_H
#define __GLCD_CONFIG_H
/*---------------------- Graphic LCD orientation configuration ---------------*/
#ifndef GLCD_LANDSCAPE
#define GLCD_LANDSCAPE  0
#endif

/*---------------------- Graphic LCD physical definitions --------------------*/
#define GLCD_SIZE_X     480             /* Screen size X (in pixels) */
#define GLCD_SIZE_Y     800             /* Screen size Y (in pixels) */
#define GLCD_BPP        32              /* Bits per pixel            */

#if    (GLCD_LANDSCAPE == 0)
#define GLCD_WIDTH      GLCD_SIZE_X     /* Screen Width  (in pixels) */
#define GLCD_HEIGHT     GLCD_SIZE_Y     /* Screen Height (in pixels) */
#else
#define GLCD_WIDTH      GLCD_SIZE_Y     /* Screen Width  (in pixels) */
#define GLCD_HEIGHT     GLCD_SIZE_X     /* Screen Height (in pixels) */
#endif

/*---------------------- Graphic LCD color definitions -----------------------*/
/*
  Color coding: ARGB8888
*/

/* GLCD color definitions                            */
#define GLCD_COLOR_BLACK        LCD_COLOR_BLACK
#define GLCD_COLOR_NAVY         0xFF000080
#define GLCD_COLOR_DARK_GREEN   LCD_COLOR_DARKGREEN
#define GLCD_COLOR_DARK_CYAN    LCD_COLOR_DARKCYAN
#define GLCD_COLOR_MAROON       0xFF800000
#define GLCD_COLOR_PURPLE       0xFF800080
#define GLCD_COLOR_OLIVE        0xFF808000
#define GLCD_COLOR_LIGHT_GREY   LCD_COLOR_LIGHTGRAY
#define GLCD_COLOR_DARK_GREY    LCD_COLOR_DARKGRAY
#define GLCD_COLOR_BLUE         LCD_COLOR_BLUE
#define GLCD_COLOR_GREEN        LCD_COLOR_GREEN
#define GLCD_COLOR_CYAN         LCD_COLOR_CYAN
#define GLCD_COLOR_RED          LCD_COLOR_RED
#define GLCD_COLOR_MAGENTA      LCD_COLOR_MAGENTA
#define GLCD_COLOR_YELLOW       LCD_COLOR_YELLOW
#define GLCD_COLOR_WHITE        LCD_COLOR_WHITE

#endif /* __GLCD_CONFIG_H */
