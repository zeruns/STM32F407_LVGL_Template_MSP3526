/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HID.c
 * Purpose: USB Device - Human Interface Device example
 *----------------------------------------------------------------------------*/

#include "main.h"
#include "rl_usb.h"                     // Keil.MDK-Pro::USB:CORE

#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"              // ::Board Support:Buttons

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

/*------------------------------------------------------------------------------
 *        Application
 *----------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  uint8_t but, but_last;

  (void)arg;

  LED_Initialize();                     /* LED Initialization                 */
  Buttons_Initialize();                 /* Buttons Initialization             */

  USBD_Initialize(0U);                  /* USB Device 0 Initialization        */
  USBD_Connect   (0U);                  /* USB Device 0 Connect               */

  but = but_last = 0U;
  for (;;) {                            /* Loop forever                       */
    but = (uint8_t)(Buttons_GetState ());
    if ((but ^ but_last) & 0x1FU) {
      but_last = but & 0x1FU;
      if (USBD_Configured (0)) { USBD_HID_GetReportTrigger(0U, 0U, &but, 1U); }
    }
    osDelay(100U);                      /* 100 ms delay for sampling buttons  */
  }
}
