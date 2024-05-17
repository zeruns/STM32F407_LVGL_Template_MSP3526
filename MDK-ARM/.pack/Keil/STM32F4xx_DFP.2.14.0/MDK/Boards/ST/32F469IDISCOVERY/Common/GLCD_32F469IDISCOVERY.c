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
 * Name:    GLCD_32F469IDISCOVERY.c
 * Purpose: Graphic LCD interface for GLCD_32F469IDISCOVERY Kit
 * Rev.:    1.0.0
 *----------------------------------------------------------------------------*/

#include "GLCD_Config.h"
#include "Board_GLCD.h"

#ifndef SDRAM_BASE_ADDR
#define SDRAM_BASE_ADDR       0xC0000000
#endif

#ifndef      __MEMORY_AT
  #if     (defined (__CC_ARM))
    #define  __MEMORY_AT(x)     __attribute__((at(x)))
  #elif   (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
    #define  __MEMORY_AT__(x)   __attribute__((section(".ARM.__AT_"#x)))
    #define  __MEMORY_AT(x)     __MEMORY_AT__(x)
  #else
    #define  __MEMORY_AT(x)
    #warning Position memory containing __MEMORY_AT macro at absolute address!
  #endif
#endif


/*---------------------------- Global variables ------------------------------*/

static uint32_t frame_buf[GLCD_WIDTH*GLCD_HEIGHT] __MEMORY_AT(SDRAM_BASE_ADDR);
static uint32_t foreground_color = GLCD_COLOR_BLACK;
static uint32_t background_color = GLCD_COLOR_WHITE;
static GLCD_FONT *active_font    = NULL;


/**
  \fn          int32_t GLCD_Initialize (void)
  \brief       Initialize Graphic LCD
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_Initialize (void) {

  // Initialize DSI LCD
#if (GLCD_LANDSCAPE != 0)
  BSP_LCD_InitEx (LCD_ORIENTATION_LANDSCAPE);
#else
  BSP_LCD_InitEx (LCD_ORIENTATION_PORTRAIT);
#endif

  // Set frame buffer address
  BSP_LCD_LayerDefaultInit(0, SDRAM_BASE_ADDR);
  BSP_LCD_LayerDefaultInit(1, SDRAM_BASE_ADDR);
  BSP_LCD_SelectLayer(0);

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
  BSP_LCD_MspDeInit ();
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
  foreground_color = color;
  BSP_LCD_SetTextColor (color);
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
  background_color = color;
  BSP_LCD_SetBackColor (color);
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
  BSP_LCD_Clear (background_color);
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
  BSP_LCD_DrawPixel((uint16_t)x, (uint16_t)y, foreground_color);
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
  BSP_LCD_DrawHLine ((uint16_t)x, (uint16_t)y, (uint16_t)length);
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
  BSP_LCD_DrawVLine ((uint16_t)x, (uint16_t)y, (uint16_t)length);
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
  BSP_LCD_DrawRect ((uint16_t)x, (uint16_t)y, (uint16_t)width, (uint16_t)height);
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
  uint32_t       i, j;
  uint32_t       wb;
  const uint8_t *ptr_ch_bmp;

  if (active_font == NULL) return -1;

  ch        -= active_font->offset;
  wb         = (active_font->width + 7)/8;
  ptr_ch_bmp = active_font->bitmap + ((uint32_t)ch * wb * active_font->height);

  for (i = 0; i < active_font->height; i++) {
    for (j = 0; j < active_font->width; j++) {
      BSP_LCD_DrawPixel((uint16_t)(x+j), (uint16_t)(y+i), (((*ptr_ch_bmp >> (j & 7)) & 1) ? foreground_color : background_color));
      if (((j & 7) == 7) && (j != (active_font->width - 1))) ptr_ch_bmp++;
    }
    ptr_ch_bmp++;
  }

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
  val = (val * width) / 100;            /* Scale value                        */

  BSP_LCD_FillRect((uint16_t)x, (uint16_t)y, (uint16_t)val, (uint16_t)height);
  BSP_LCD_SetTextColor (background_color);
  BSP_LCD_FillRect((uint16_t)(x+val), (uint16_t)y, (uint16_t)(width-val), (uint16_t)height);
  BSP_LCD_SetTextColor (foreground_color);

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
  uint32_t        i, j;
  const uint32_t *ptr_bmp;

  ptr_bmp = (const uint32_t *)((uint32_t)bitmap);

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      BSP_LCD_DrawPixel ((uint16_t)(x + j), (uint16_t)y, *ptr_bmp++);
    }
    y++;
  }

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
  uint32_t i, len;

  dy *= GLCD_WIDTH;
  len = GLCD_WIDTH * GLCD_HEIGHT - dy;
  for (i = 0; i < len; i++) {
    frame_buf[i] = frame_buf[i+dy];
  }
  for (; i < GLCD_WIDTH * GLCD_HEIGHT; i++) {
    frame_buf[i] = background_color;
  }

  return 0;
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
int32_t  GLCD_FrameBufferAccess   (bool enable) {
  /* Direct data access is used with direct access to frame buffer            */
  (void)enable;
  return 0;
}

/**
  \fn          uint32_t GLCD_FrameBufferAddress (void)
  \brief       Retrieve FrameBuffer address
  \returns     FrameBuffer address
*/
uint32_t GLCD_FrameBufferAddress (void) {
  return ((uint32_t)frame_buf);
}
