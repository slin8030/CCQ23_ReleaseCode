/** @file
  Header file for SPI Flash Device Support Library Class

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Library/FdSupportLib.h>

#ifndef _FLASH_DEVICES_LIB_H_
#define _FLASH_DEVICES_LIB_H_
UINTN
EFIAPI
NumberOfSupportedFlashDevices (
  VOID
);

extern FLASH_DEVICE *gSupportedFlashDevices[];  

#endif
