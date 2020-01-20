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

  UsbLegacy.h
    
Abstract:

  The Usb Legacy Protocol is used provide a standard interface to the
  UsbLegacy code for platform modifications of operation.
  
  
Revision History

--*/
//
// Local function prototypes
//
#define ESC_KEY   0x01
#define DEL_KEY   0x53
#define F1_KEY    0x3b
#define F2_KEY    0x3c
#define F3_KEY    0x3d
#define F4_KEY    0x3e
#define F5_KEY    0x3f
#define F6_KEY    0x40
#define F7_KEY    0x41
#define F8_KEY    0x42
#define F9_KEY    0x43
#define F10_KEY   0x44
#define F11_KEY   0x57
#define F12_KEY   0x58

#define USB_LEGACY_PLATFORM_INSTANCE_SIGNATURE   EFI_SIGNATURE_32('U','L','P','L')
