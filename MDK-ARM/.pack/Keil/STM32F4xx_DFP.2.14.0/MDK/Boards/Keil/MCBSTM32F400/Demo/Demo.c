/*-----------------------------------------------------------------------------
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All
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
 * Name:    Demo.c
 * Purpose: Demo example for MCBSTM32F400
 * Note(s):
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"
#include "GLCD_Config.h"                // Keil.MCBSTM32F400::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_ADC.h"                  // ::Board Support:A/D Converter
#include "Board_Buttons.h"              // ::Board Support:Buttons
#include "Board_Joystick.h"             // ::Board Support:Joystick
#include "Board_Camera.h"               // ::Board Support:Camera
#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "Board_Accelerometer.h"        // ::Board Support:Accelerometer
#include "Board_Gyroscope.h"            // ::Board Support:Gyroscope

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

/* Ruler position definitions */
#define RUL_2M      ( 8U * 6U + 3U)
#define RUL_1M      (17U * 6U + 3U)
#define RUL_0       (26U * 6U + 3U)
#define RUL_1P      (35U * 6U + 3U)
#define RUL_2P      (44U * 6U + 3U)
#define RUL_HGT     (5U)
#define RUL_LN(ln) ((ln) * 8U - 2U)

/* Bargraph position definitions */
#define BAR_ST      (8U * 6U + 3U)
#define BAR_LEN     (217U)
#define BAR_HGT     (20U)
#define BAR_LN(ln)  ((ln) * 24U)

/* Joystick position definitions */
#define JOY_X       (13U * 16U)
#define JOY_Y       ( 5U * 24U + 6U)

/* Button position definitions */
#define BTN_X       (9U)
#define BTN_Y       (190U)
#define BTN_X_EVAL  (20U)
#define BTN_Y_EVAL  (182U)

/* Bulb position definition */
#define BULB_X       (209U)
#define BULB_Y       (190U)
#define BULB_X_EVAL  (9U * 16U)
#define BULB_Y_EVAL  (8U * 24U)

/* Keys bit masks */
#define KEY_WAKEUP  4
#define KEY_TAMPER  2
#define KEY_USER    1

/* Mode definitions */
enum {
  M_INIT = 0,
  M_STD,
  M_MOTION,
  M_CAM,
};

/* Periodic timer definition */
static void Timer_Callback (void *arg);
static osTimerId_t TimerId;

/* Mode switch timeout variable */
static uint32_t KeyTick;

/* Extern variables */
extern unsigned char Button_16bpp[];
extern unsigned char Bulb_16bpp[];
extern GLCD_FONT     GLCD_Font_6x8;
extern GLCD_FONT     GLCD_Font_16x24;

/*-----------------------------------------------------------------------------
  Periodic timer callback
 *----------------------------------------------------------------------------*/
static void Timer_Callback (void *arg) {
  static uint32_t ticks;

  (void)arg;

  if (KeyTick == 0) {
    if (++ticks == 50) {
      KeyTick = 1;
      ticks = 0;
    }
  }
  else ticks = 0;
}

/*-----------------------------------------------------------------------------
  Draw vertical line at specified coordinates (x,y) with specified height (h)
 *----------------------------------------------------------------------------*/
static void DrawRuler (uint32_t x, uint32_t y, uint32_t h) {
  uint32_t i;

  for (i = 0; i < h; i++) {
    GLCD_DrawPixel (x, y - i);
  }
}

/*-----------------------------------------------------------------------------
  Switch between modes
 *----------------------------------------------------------------------------*/
static uint32_t SwitchMode (uint32_t mode) {

  GLCD_SetBackgroundColor (GLCD_COLOR_WHITE);
  GLCD_ClearScreen ();

  switch (mode) {
    case M_INIT:
      /* Prepare display for ADC, Keys, Joystick */
      GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
      GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
      GLCD_SetFont            (&GLCD_Font_16x24);
      GLCD_DrawString (0, 0*24, "    MCBSTM32F400    ");
      GLCD_DrawString (0, 1*24, "    Demo Example    ");
      GLCD_SetBackgroundColor (GLCD_COLOR_WHITE);
      GLCD_SetForegroundColor (GLCD_COLOR_BLUE);
      GLCD_DrawString (0, 3*24, "AD value:           ");
      GLCD_DrawString (0, 4*24, "Buttons :           ");
      GLCD_DrawString (0, 6*24, "Joystick:           ");

      mode = M_STD;
      break;

    case M_STD:
      /* Prepare display for accelerator and gyroscope */
      GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
      GLCD_SetBackgroundColor (GLCD_COLOR_NAVY);
      GLCD_ClearScreen ();

      /* Draw rulers */
      DrawRuler (RUL_2M, RUL_LN(6), RUL_HGT);
      DrawRuler (RUL_1M, RUL_LN(6), RUL_HGT);
      DrawRuler (RUL_0,  RUL_LN(6), RUL_HGT);
      DrawRuler (RUL_1P, RUL_LN(6), RUL_HGT);
      DrawRuler (RUL_2P, RUL_LN(6), RUL_HGT);

      DrawRuler (RUL_2M, RUL_LN(21), RUL_HGT);
      DrawRuler (RUL_1M, RUL_LN(21), RUL_HGT);
      DrawRuler (RUL_0,  RUL_LN(21), RUL_HGT);
      DrawRuler (RUL_1P, RUL_LN(21), RUL_HGT);
      DrawRuler (RUL_2P, RUL_LN(21), RUL_HGT);
      
      /* Accelerator graphics */
      GLCD_DrawString (0, 0*24, "  Acceleration [g]  ");
      
      GLCD_SetFont    (&GLCD_Font_6x8);
      GLCD_DrawString ( 7*6,  4*8, "-2");
      GLCD_DrawString (16*6,  4*8, "-1");
      GLCD_DrawString (26*6,  4*8, "0");
      GLCD_DrawString (35*6,  4*8, "1");
      GLCD_DrawString (44*6,  4*8, "2");
      
      GLCD_SetFont    (&GLCD_Font_16x24);
      GLCD_DrawString (0, 2*24, "X:");
      GLCD_DrawString (0, 3*24, "Y:");
      GLCD_DrawString (0, 4*24, "Z:");
      
      /* Gyroscope graphics */
      GLCD_DrawString (0,5*24, " Angular rate [d/s] ");

      GLCD_SetFont    (&GLCD_Font_6x8);      
      GLCD_DrawString ( 6*6, 19*8, "-250");
      GLCD_DrawString (15*6, 19*8, "-125");
      GLCD_DrawString (26*6, 19*8, "0");
      GLCD_DrawString (34*6, 19*8, "125");
      GLCD_DrawString (43*6, 19*8, "250");
      
      GLCD_SetFont    (&GLCD_Font_16x24);
      GLCD_DrawString (0, 7*24, "X:");
      GLCD_DrawString (0, 8*24, "Y:");
      GLCD_DrawString (0, 9*24, "Z:");

      GLCD_SetBackgroundColor (GLCD_COLOR_DARK_GREY);
      mode = M_MOTION;
      break;

    case M_MOTION:
      /* Prepare display for video stream from digital camera */
      GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
      GLCD_ClearScreen ();
      GLCD_FrameBufferAccess (true);
      /* Turn camera on */
      Camera_On ();
      mode = M_CAM;
      break;

    case M_CAM:
      /* Turn camera off */
      Camera_Off ();
      GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
      GLCD_ClearScreen ();
      mode = M_INIT;
      break;

    default:
      mode = M_INIT;
      break;
  }
  return (mode);
}

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *argument) {
  (void)argument;

  TOUCH_STATE tsc_state;
  ACCELEROMETER_STATE acc;
  GYROSCOPE_STATE ang;
  uint32_t mode, addr;
  uint32_t keyMsk, joyMsk;
  uint32_t key  = 0xFFFFFFFFU;
  uint32_t joy  = 0xFFFFFFFFU;
  int32_t  adcVal;
  int32_t  adc  = -1;

  TimerId = osTimerNew (Timer_Callback, osTimerPeriodic, 0, NULL);

  if (TimerId) {
    osTimerStart (TimerId, 10);
  }

  LED_Initialize();                     /* LED Init                           */
  ADC_Initialize();                     /* A/D Converter Init                 */
  Buttons_Initialize();                 /* Buttons Init                       */
  Joystick_Initialize();                /* Joystick Init                      */
  Touch_Initialize();                   /* Touchscreen Controller Init        */
  Accelerometer_Initialize();           /* Accelerometer Init                 */
  Gyroscope_Initialize();               /* Gyroscope Init                     */
  GLCD_Initialize();                    /* Graphical Display Init             */

  addr = GLCD_FrameBufferAddress();     /* Retrieve fremebuffer address       */
  Camera_Initialize(addr);              /* Camera Init                        */

  GLCD_SetBackgroundColor (GLCD_COLOR_WHITE);
  GLCD_ClearScreen ();
  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
  GLCD_SetFont            (&GLCD_Font_16x24);

  GLCD_DrawString (0, 0*24, "    MCBSTM32F400    ");
  GLCD_DrawString (0, 1*24, "    Demo Example    ");
  GLCD_SetBackgroundColor (GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor (GLCD_COLOR_BLUE);

  GLCD_DrawString (0, 3*24, " Use WAKEUP key to  ");
  GLCD_DrawString (0, 4*24, "   switch example   ");
  GLCD_DrawString (0, 5*24, "        mode        ");

  GLCD_DrawString (0, 8*24, "  Press any key to  ");
  GLCD_DrawString (0, 9*24, "      continue      ");

  /* Wait for user to press the key */
  while (!Buttons_GetState() && !Joystick_GetState());
  mode = SwitchMode (M_INIT);
  KeyTick = 0;

  while (1) {    
    if (KeyTick && (Buttons_GetState() & KEY_WAKEUP)) {
      mode = SwitchMode (mode);
      KeyTick = 0;
      /* Force refresh */
      key = 0xFFFFFFFFU;
      joy = 0xFFFFFFFFU;
      adc = -1;
    }

    switch (mode) {
      /* Show potentiometer, keys, joystick */
      case M_STD:
        keyMsk = Buttons_GetState();    /* Show buttons state                 */
        if (key ^ keyMsk) {          
          GLCD_SetForegroundColor (GLCD_COLOR_BLACK);
          if (keyMsk & KEY_TAMPER)    { GLCD_DrawString (9*16, 4*24, "TAMPER"); }
          if (keyMsk & KEY_USER  )    { GLCD_DrawString (9*16, 5*24, "USER");   }
          
          GLCD_SetForegroundColor (GLCD_COLOR_LIGHT_GREY);
          if (!(keyMsk & KEY_TAMPER)) { GLCD_DrawString (9*16, 4*24, "TAMPER"); }
          if (!(keyMsk & KEY_USER  )) { GLCD_DrawString (9*16, 5*24, "USER");   }
        }

        joyMsk = Joystick_GetState(); /* Show joystick state                  */
        if (joy ^ joyMsk) {
          joy = joyMsk;

          if (joy == 0) {
            GLCD_SetForegroundColor (GLCD_COLOR_LIGHT_GREY);
            GLCD_DrawString (9*16, 6*24, "NOT PUSHED");
          }
          else {
            GLCD_SetForegroundColor (GLCD_COLOR_BLACK);

            if (joy & JOYSTICK_DOWN)   { GLCD_DrawString (9U*16U, 6U*24U, "DOWN      "); }
            if (joy & JOYSTICK_UP)     { GLCD_DrawString (9U*16U, 6U*24U, "UP        "); }
            if (joy & JOYSTICK_CENTER) { GLCD_DrawString (9U*16U, 6U*24U, "CENTER    "); }
            if (joy & JOYSTICK_LEFT)   { GLCD_DrawString (9U*16U, 6U*24U, "LEFT      "); }
            if (joy & JOYSTICK_RIGHT)  { GLCD_DrawString (9U*16U, 6U*24U, "RIGHT     "); }
          }
        }

        ADC_StartConversion();          /* Show A/D conversion bargraph       */
        adcVal = ADC_GetValue();
        if (adcVal != -1) {
          if (adc ^ adcVal) {
            GLCD_SetForegroundColor (GLCD_COLOR_GREEN);
            GLCD_DrawBargraph (144U, 3U*24U, 160U, 20U, (uint32_t)((adcVal * 100) / ((1 << ADC_GetResolution()) - 1)));
          }
        }
        
        Touch_GetState (&tsc_state);    /* Get touch screen state             */

        if (!tsc_state.pressed) {
          tsc_state.x = 0;
          tsc_state.y = 0;
        }

        if (tsc_state.x >= 200 && tsc_state.x <= 1000 &&
            tsc_state.y >= 260 && tsc_state.y <= 1650) {
          GLCD_DrawBitmap(BTN_X,  BTN_Y,  120U, 48U, &Button_16bpp[1*11520]);
          GLCD_DrawBitmap(BULB_X, BULB_Y,  26U, 40U, &Bulb_16bpp[1*2080]);
        }
        else {
          GLCD_DrawBitmap(BTN_X,  BTN_Y,  120, 48U, &Button_16bpp[0*11520]);
          GLCD_DrawBitmap(BULB_X, BULB_Y,  26U, 40U, &Bulb_16bpp[0*2080]);
        }
        break;

      /* Show accelerator and gyroscope outputs */
      case M_MOTION:
        if (Accelerometer_GetState (&acc)) {    /* Get acceleration data      */
          acc.x = 0; acc.y = 0; acc.z = 0;
        }
        GLCD_SetForegroundColor (GLCD_COLOR_RED);
        GLCD_DrawBargraph (BAR_ST, BAR_LN(2U), BAR_LEN, BAR_HGT, 50u + (uint32_t)(acc.x * 25/1000));
        GLCD_DrawBargraph (BAR_ST, BAR_LN(3U), BAR_LEN, BAR_HGT, 50U + (uint32_t)(acc.y * 25/1000));
        GLCD_DrawBargraph (BAR_ST, BAR_LN(4U), BAR_LEN, BAR_HGT, 50U + (uint32_t)(acc.z * 25/1000));

        if (Gyroscope_GetState (&ang)) {        /* Get angular rate data      */
          ang.x = 0; ang.y = 0; ang.z = 0;
        }
        GLCD_SetForegroundColor (GLCD_COLOR_YELLOW);
        GLCD_DrawBargraph (BAR_ST, BAR_LN(7U), BAR_LEN, BAR_HGT, 50U + (uint32_t)(ang.x * 25/1250));
        GLCD_DrawBargraph (BAR_ST, BAR_LN(8U), BAR_LEN, BAR_HGT, 50U + (uint32_t)(ang.y * 25/1250));
        GLCD_DrawBargraph (BAR_ST, BAR_LN(9U), BAR_LEN, BAR_HGT, 50U + (uint32_t)(ang.z * 25/1250));
        break;

      /* Show digital camera output on graphical LCD */
      case M_CAM:
        break;

      default:
        mode = SwitchMode (mode);
        break;
    }
  }
}
