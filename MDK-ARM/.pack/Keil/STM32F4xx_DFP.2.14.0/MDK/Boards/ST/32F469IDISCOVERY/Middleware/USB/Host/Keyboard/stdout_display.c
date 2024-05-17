/*-----------------------------------------------------------------------------
 * Copyright (c) 2013-2018 Arm Limited (or its affiliates). All
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
 * Name:    stdout_display.c
 * Purpose: STDOUT Display Template
 * Rev.:    1.0.0
   ---------------------------------------------------------------------------*/
 
#include "Board_GLCD.h"
#include "GLCD_Config.h"

extern GLCD_FONT GLCD_Font_16x24;

extern int stdout_init    (void);
extern int stdout_putchar (int ch);

#define GLCD_COLUMNS           (GLCD_WIDTH  / GLCD_Font_16x24.width)
#define GLCD_ROWS              (GLCD_HEIGHT / GLCD_Font_16x24.height)
 
 
/**
  Initialize stdout
 
  \return          0 on success, or -1 on error.
*/
int stdout_init (void) {
 
  GLCD_Initialize         ();
  GLCD_SetBackgroundColor (GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor (GLCD_COLOR_BLUE);
  GLCD_ClearScreen        ();
  GLCD_SetFont            (&GLCD_Font_16x24);
 
  return (0);
}
 
 
/**
  Put a character to the stdout
 
  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stdout_putchar (int ch) {
  static uint8_t row = 0U;
  static uint8_t col = 0U;

  switch (ch) {
    case 0x08:                          // Backspace
      if (col != 0U) {                  // If column is not first
        col--;                          // Move cursor one column to left
      } else if (row != 0U) {           // Else if it is not first row
        row--;                          // Move cursor one row up and to
        col=(uint8_t)(GLCD_COLUMNS-1U); // last column on the right
      }
      // Clear character at current cursor position
      GLCD_DrawChar (col * GLCD_Font_16x24.width, row * GLCD_Font_16x24.height, ' ');
      break;
    case 0x0A:                          // Line Feed
    case 0x0D:                          // Carriage Return
      row++;                            // Move cursor one row down and to
      col = 0U;                         // first column
      if (row == GLCD_ROWS) {           // If bottom of display was overstepped
        row = 0U;                       // Move cursor to first row and to
        col = 0U;                       // first column
      }
      break;
    default:
      // Display character at current cursor position
      GLCD_DrawChar (col * GLCD_Font_16x24.width, row * GLCD_Font_16x24.height, ch);
      col++;                            // Move cursor one column to right
      if (col == GLCD_COLUMNS) {        // If last column was overstepped
        row++;                          // Move cursor one row down and to
        col = 0U;                       // first column
      }
      break;
  }

  return (ch);
}
 
