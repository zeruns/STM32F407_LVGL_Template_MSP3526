/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::USB:Device
 * Copyright (c) 2004-2018 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    USBD_User_CustomClass_0.c
 * Purpose: USB Device Custom Class User module
 * Rev.:    V6.7.3
 *----------------------------------------------------------------------------*/
/*
 * USBD_User_CustomClass_0.c is a code template for the Custom Class 0 
 * class request handling. It allows user to handle all Custom Class class 
 * requests.
 *
 * Uncomment "Example code" lines to see example that receives data on 
 * Endpoint 1 OUT and echoes it back on Endpoint 1 IN.
 * To try the example you also have to enable Bulk Endpoint 1 IN/OUT in Custom 
 * Class configuration in USBD_Config_CustomClass_0.h file.
 */

/**
 * \addtogroup usbd_custom_classFunctions
 *
 */
 
 
//! [code_USBD_User_CustomClass]
 
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "rl_usb.h"
#include "Driver_USBD.h"
 
 
// Variable declaration
static bool     data_received, send_active;
static uint32_t data_len_received;
static uint16_t bulk_max_packet_size;
static uint8_t  bulk_out_buf[512];
static uint8_t  bulk_in_buf [512];
 
 
// \brief Callback function called during USBD_Initialize to initialize the USB Custom class instance
void USBD_CustomClass0_Initialize (void) {

  // Clear buffers
  memset((void *)bulk_out_buf, 0, 512U);
  memset((void *)bulk_in_buf,  0, 512U);

  data_received     = false;
  data_len_received = 0U;
  send_active       = false;
}

// \brief Callback function called during USBD_Uninitialize to de-initialize the USB Custom class instance
void USBD_CustomClass0_Uninitialize (void) {

  data_received     = false;
  data_len_received = 0U;
  send_active       = false;
}
 
// \brief Callback function called upon USB Bus Reset signaling
void USBD_CustomClass0_Reset (void) {

  data_received     = false;
  data_len_received = 0U;
  send_active       = false;
}
 
// \brief Callback function called when Endpoint Start was requested (by activating interface or configuration)
// \param[in]     ep_addr       endpoint address.
void USBD_CustomClass0_EndpointStart (uint8_t ep_addr) {

  if (USBD_GetState(0U).speed == USB_SPEED_HIGH) {
    bulk_max_packet_size = 512U;
  } else {
    bulk_max_packet_size = 64U;
  }

  // Start reception on Endpoint 1 OUT
  if (ep_addr == USB_ENDPOINT_OUT(1)) {
    USBD_EndpointRead(0U, USB_ENDPOINT_OUT(1), bulk_out_buf, bulk_max_packet_size);
  }
}
 
// \brief Callback function called when DATA was sent or received on Endpoint 1
// \param[in]     event                   event on Endpoint:
//                                          - ARM_USBD_EVENT_OUT = data OUT received
//                                          - ARM_USBD_EVENT_IN  = data IN  sent
void USBD_CustomClass0_Endpoint1_Event  (uint32_t event) {

  if (event & ARM_USBD_EVENT_IN) {      // IN event
    send_active = false;
  }
  if (event & ARM_USBD_EVENT_OUT) {     // OUT event
    data_len_received = USBD_EndpointReadGetResult(0U, USB_ENDPOINT_OUT(1));
    data_received     = true;
  }
  if ((data_received) && (!send_active)) {
    // Copy received data bytes to transmit buffer
    memcpy((void *)bulk_in_buf, (void *)bulk_out_buf, data_len_received);

    // Transmit back data bytes
    send_active = true;
    USBD_EndpointWrite(0U, USB_ENDPOINT_IN(1) , bulk_in_buf, data_len_received);

    // Restart reception on Endpoint 1 OUT
    data_received = false;
    USBD_EndpointRead (0U, USB_ENDPOINT_OUT(1), bulk_out_buf, bulk_max_packet_size);
  }
}
void USBD_CustomClass0_Endpoint2_Event  (uint32_t event) {
  // Handle Endpoint 2 events
}
void USBD_CustomClass0_Endpoint3_Event  (uint32_t event) {
  // Handle Endpoint 3 events
}
void USBD_CustomClass0_Endpoint4_Event  (uint32_t event) {
  // Handle Endpoint 4 events
}
void USBD_CustomClass0_Endpoint5_Event  (uint32_t event) {
  // Handle Endpoint 5 events
}
void USBD_CustomClass0_Endpoint6_Event  (uint32_t event) {
  // Handle Endpoint 6 events
}
void USBD_CustomClass0_Endpoint7_Event  (uint32_t event) {
  // Handle Endpoint 7 events
}
void USBD_CustomClass0_Endpoint8_Event  (uint32_t event) {
  // Handle Endpoint 8 events
}
void USBD_CustomClass0_Endpoint9_Event  (uint32_t event) {
  // Handle Endpoint 9 events
}
void USBD_CustomClass0_Endpoint10_Event (uint32_t event) {
  // Handle Endpoint 10 events
}
void USBD_CustomClass0_Endpoint11_Event (uint32_t event) {
  // Handle Endpoint 11 events
}
void USBD_CustomClass0_Endpoint12_Event (uint32_t event) {
  // Handle Endpoint 12 events
}
void USBD_CustomClass0_Endpoint13_Event (uint32_t event) {
  // Handle Endpoint 13 events
}
void USBD_CustomClass0_Endpoint14_Event (uint32_t event) {
  // Handle Endpoint 14 events
}
void USBD_CustomClass0_Endpoint15_Event (uint32_t event) {
  // Handle Endpoint 15 events
}
 
//! [code_USBD_User_CustomClass]
