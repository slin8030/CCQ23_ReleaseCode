/*++

Copyright (c)  2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  BoardUsbLegacy.h
    
Abstract:

  The Usb Legacy Protocol is used provide a standard interface to the
  UsbLegacy code for platform modifications of operation.
  
  
Revision History

--*/

//
// Local function prototypes
//

// These codes should be ASCII for MonitorKeyFilter
#define ASCII_ESC_KEY   0x01
#define ASCII_DEL_KEY   0x53
#define ASCII_F1_KEY    0x3b
#define ASCII_F2_KEY    0x3c
#define ASCII_F3_KEY    0x3d
#define ASCII_F4_KEY    0x3e
#define ASCII_F5_KEY    0x3f
#define ASCII_F6_KEY    0x40
#define ASCII_F7_KEY    0x41
#define ASCII_F8_KEY    0x42
#define ASCII_F9_KEY    0x43
#define ASCII_F10_KEY   0x44
#define ASCII_F11_KEY   0x85
#define ASCII_F12_KEY   0x86
#define ASCII_UP_ARROW_KEY   0x18

//
// These must reflect order in KEY_ELEMENT
//
#define F1_KEY_BIT   0x01
#define F2_KEY_BIT   0x02
#define DEL_KEY_BIT  0x04
#define F10_KEY_BIT  0x08
#define F12_KEY_BIT  0x10
#define ESC_KEY_BIT  0x20
#define UP_ARROW_KEY_BIT 0x40
#define F4_KEY_BIT   0x80
#define F7_KEY_BIT   0x100

//
// Order is bit position returned by MonitorKeyFilter protocol.
//
#define PLATFORM_KEY_LIST { \
   {ASCII_F1_KEY,0},  \
   {ASCII_F2_KEY,0},  \
   {ASCII_DEL_KEY,0}, \
   {ASCII_F10_KEY,0}, \
   {ASCII_F12_KEY,0}, \
   {ASCII_ESC_KEY,0},  \
   {ASCII_UP_ARROW_KEY,0},  \
   {ASCII_F4_KEY,0}, \
   {ASCII_F7_KEY,0} \
}
