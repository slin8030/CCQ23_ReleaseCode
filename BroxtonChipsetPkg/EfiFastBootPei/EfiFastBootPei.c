/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/**

Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/          
#include <PiPei.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UsbDeviceModeLib.h>
#include <Library/HobLib.h>
#include <Ppi/UsbDeviceMode.h>
#include <Library/UefiBootServicesTableLib.h>
#include <EfiFastBootPei.h>
#include <ScAccess.h>
#include <Guid/PlatformInfo.h>
#include <Library/HobLib.h>
#include <Library/PmicLib/PmicWhiskeyCove.h>
#include <Library/I2CLib.h>

EFI_STATUS
PeiFastBoot(
  IN EFI_PEI_SERVICES             **PeiServices
  );

CONST EFI_PEI_FAST_BOOT_PPI mPeiFastbootPpi = {
  PeiFastBoot
};

CONST EFI_PEI_PPI_DESCRIPTOR mFastbootPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gPeiFastbootPpiGuid,
    (VOID *) &mPeiFastbootPpi
  }
};

EFI_STATUS
PeiFastBoot(
  IN EFI_PEI_SERVICES       **PeiServices
  )
{
  EFI_PEI_HOB_POINTERS   GuidHob;
  UINT8         Value = 0;
  EFI_STATUS    Status = EFI_SUCCESS;
  EFI_PLATFORM_INFO_HOB  *mPlatformInfo=NULL;

  DEBUG ((DEBUG_INFO, "FASTBOOT: Entry FastBoot App\n"));

  GuidHob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (GuidHob.Raw != NULL);
  mPlatformInfo = GET_GUID_HOB_DATA(GuidHob.Raw);

  FastBootDataInit(PeiServices);
  if (mPlatformInfo != NULL) {
    if (mPlatformInfo->PmicId == PMIC_TYPE_WC) {
      //
      // [Cherryview HSD][4799387]BIOS support for the USB detection on CHTT FFD with Whiskey Cove
      //
      Value = 0;
      ByteReadI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_USB_USBPHYCTRL_REG, 1, &Value);
      if(EFI_ERROR(Status)){
        DEBUG ((DEBUG_INFO, "WC: ByteReadI2C() failed: Offset = 0x%x, Value = 0x%x, Status = %r\n", DEV3_USB_USBPHYCTRL_REG, Value, Status));
      }
      Value &= ~BIT0;
      Value |= BIT0;
      
      ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_USB_USBPHYCTRL_REG, 1, &Value);
      if(EFI_ERROR(Status)){
        DEBUG ((DEBUG_INFO, "WC: ByteWriteI2C() failed: Offset = 0x%x, Value = 0x%x, Status = %r\n", DEV3_USB_USBPHYCTRL_REG, Value, Status));
      }
    }
  }
  FastBootStart((CONST EFI_PEI_SERVICES **)PeiServices);
  
  DEBUG ((DEBUG_INFO, "FASTBOOT: Exit FastBoot App\n"));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FastBootPeiEntry (
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  DEBUG ((DEBUG_INFO, "FASTBOOT: Install FastBoot App\n"));

  (**PeiServices).InstallPpi (PeiServices, mFastbootPpiList);

  return EFI_SUCCESS;
}
