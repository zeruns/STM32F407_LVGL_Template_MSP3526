/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    WinUSB_Echo.c
 * Purpose: USB Device WinUSB Device Echo Bulk Out to Bulk In
 *----------------------------------------------------------------------------*/

#include "main.h"
#include "rl_usb.h"                     // Keil.MDK-Pro::USB:CORE

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *argument) {
  (void)argument;


  USBD_Initialize(0U);                  // USB Device 0 Initialization
  USBD_Connect   (0U);                  // USB Device 0 Connect

  osThreadExit();
}
