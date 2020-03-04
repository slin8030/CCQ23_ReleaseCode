/** @file
 This function offers an interface to Hook IHISI Sub function AH=20h,function "FetsEcFlash"
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmOemSvcKernelLib.h>
#include <Library/EcSpiLib.h>
#define END_FLASH     0xEF

/**
 This function offers an interface to Hook IHISI Sub function AH=20h,function "FetsEcFlash"
 Function uses to flash EC part.

 @param[in]         FlashingDataBuffer  Double pointer to data buffer.
 @param[in]         SizeToFlash         Data size by bytes want to flash.
 @param[in]         DestBlockNo         Dsstination block number.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS20HookFetsEcFlash (
  IN UINT8          *FlashingDataBuffer,
  IN UINTN          SizeToFlash,
  IN UINT8          DestBlockNo
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/
  UINTN                                   FlashImageSize;
  UINTN                                   Address;
  UINTN                                   i;

  FlashImageSize = 0;
  Address = 0;


  if (DestBlockNo == END_FLASH) {
    return EFI_SUCCESS;
  }

/*
  0x1F000 ~ 0x1FFFF (EC EEPROM region)
  Total protect size = 4K (0x1000)
*/
  FlashImageSize = SizeToFlash;
  Address = 0;

  if (SizeToFlash != 0x10000) {
    FlashImageSize = SizeToFlash - 0x1000;
    Address = 0;
  } else {
    if (DestBlockNo == 0) {
      FlashImageSize = SizeToFlash;
      Address = 0;
    } else if (DestBlockNo == 1) {
      FlashImageSize = SizeToFlash - 0x1000;
      Address = DestBlockNo*0x10000;
    } else {
      return EFI_SUCCESS;
    }
  }

  Start_SPI();
  
//DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Address = %x ,  FlashImageSize = %x \n", Address, FlashImageSize));
  for (i=0; i<((FlashImageSize/1024)*8) ; i++) {
    WriteEcRom(Address, FlashingDataBuffer, 128);
    Address += 128;
    FlashingDataBuffer += 128;
  }
  Stop_SPI();

  return EFI_SUCCESS;
}
