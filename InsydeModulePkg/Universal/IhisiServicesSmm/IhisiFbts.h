/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _IHISI_FBTS_H_
#define _IHISI_FBTS_H_

#include "IhisiServicesSmm.h"

typedef struct {
  UINT8      IhisiAreaType;
  UINT8      FlashAreaType;
} FLASH_MAP_MAPPING_TABLE;

/**
  Get IHISI status.translated from EFI status

  @param[in] Status  EFI_STATUS

  @return UINT32     IHISI status
**/
UINT32
GetFbtsIhisiStatus (
  EFI_STATUS                             Status
  );

/**
  AH=10h, Get permission,hook OemSvcIhisiS10HookGetPermission.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
OemFbtsGetPermission (
  VOID
  );

/**
  AH=10h, Get Ac status,hook OemSvcIhisiS10HookGetAcStatus


  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
OemFbtsGetAcStatus (
  VOID
  );

/**
  AH=10h, Get battery life,hook OemSvcIhisiS10HookGetBatterylife.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
OemFbtsGetBatteryLife (
  VOID
  );

/**
  AH=11h, AP check ,hook OemSvcIhisiS11HookFbtsApCheck.

@retval
  BIT 0: Model name check
  BIT 1: Model version check
  BIT 2: Allow same version file.
  BIT 3: Verify file checksum
  BIT 4: Disable display model name
  BIT 5: Disable display model version
  BIT 6: Disable read comparison while flash process
  BIT 7: Enable ECX to return extended flag.
**/
EFI_STATUS
EFIAPI
OemFbtsApCheck (
  VOID
  );

/**
  AH=12h(FbtsGetPlatformRomMap), Get Oem define flash map.

  @retval EFI_SUCCESS       Get OEM flash map successful.
  @retval EFI_UNSUPPORTED   FBTS_OEM_ROM_MAP_COUNT is 0 or module rom map buffer is full.
**/
EFI_STATUS
EFIAPI
OemGetPlatformRomMap (
  VOID
  );

/**
  AH=1Eh,Get whole BIOS ROM map, hook OemSvcIhisiGetWholeBiosRomMap;

  @retval EFI_SUCCESS                 FBTS get BIOS ROM map success.
  @return Others                      FBTS get BIOS ROM map failed.
**/
EFI_STATUS
EFIAPI
OemFbtsGetWholeBiosRomMap (
  VOID
  );

/**
  AH=1Fh, AP Hook Point for BIOS

  @retval EFI_SUCCESS                 FBTS get BIOS ROM map success.
  @return Others                      FBTS get BIOS ROM map failed.
**/
EFI_STATUS
EFIAPI
OemFbtsApHookForBios (
  VOID
  );

/**
  AH=10h,Get Permission, hook SmmCsSvcIhisiFbtsGetPermission.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsGetPermission (
  VOID
  );

/**
  AH=14h ,FbtsRead before process , hook SmmCsSvcIhisiFbtsDoBeforeReadProcess

  @retval EFI_SUCCESS            Chipset FBTS read success.
  @return Others                 FBTS read failed.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsDoBeforeReadProcess (
  VOID
  );

/**
  AH=14h ,FbtsRead after process , hook SmmCsSvcIhisiFbtsDoAfterReadProcess
**/
EFI_STATUS
EFIAPI
ChipsetFbtsDoAfterReadProcess (
  VOID
  );

/**
  AH=15h ,FbtsWrite before process , hook SmmCsSvcIhisiFbtsDoBeforeWriteProcess

  @retval EFI_SUCCESS            Chipset FBTS read success.
  @return Others                 FBTS read failed.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsDoBeforeWriteProcess (
  VOID
  );

/**
  AH=15h ,FbtsWrite after process , hook SmmCsSvcIhisiFbtsDoAfterWriteProcess
**/
EFI_STATUS
EFIAPI
ChipsetFbtsDoAfterWriteProcess (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsApTerminated.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsCompleteApTerminated (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsNormalFlash.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsCompleteNormalFlash (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsPartialFlash.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsCompletePartialFlash (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsOemComplete.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsComplete (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsReboot.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsReboot (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsShutDown.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsShutDown (
  VOID
  );

/**
  AH=16h, FbtsComplete, hook SmmCsSvcIhisiFbtsApRequestDoNothing.

  @retval EFI_SUCCESS        Function succeeded.
**/
EFI_STATUS
EFIAPI
ChipsetFbtsDoNothing (
  VOID
  );

/**
  AH=12h, Get Platform ROM map protection, hook SmmCsSvcIhisiFbtsGetOemFlashMap

  @retval EFI_SUCCESS        Get Platform ROM map protection successful.
**/

EFI_STATUS
EFIAPI
ChipsetFbtsGetPlatformRomMap (
  VOID
  );

/**
  AH=10h, Get IHISI version

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetVersion (
  VOID
  );

/**
  AH=10h, Init OemHelp1/2.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
KernelFbtsInitOemHelp (
  VOID
  );

/**
  AH=10h, Get IHISI VendorId.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetVendorId (
  VOID
  );

/**
  AH=10h, Get Battery low bound.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetBatteryLoawBound (
  VOID
  );

/**
  AH=11h, Get platform information. Get Model Name.

  @retval EFI_SUCCESS        Get platform information successful.
  @return Other              Get platform information failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetModelName (
  VOID
  );

/**
  AH=11h, Get platform information. Get Model Version

  @retval EFI_SUCCESS        Get platform information successful.
  @return Other              Get platform information failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetModelVersion (
  VOID
  );

/**
  AH=11h, Get platform information.Update Extend Platform

  @retval EFI_SUCCESS        Get platform information successful.
  @return Other              Get platform information failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsUpdateExtendPlatform (
  VOID
  );

/**
  AH=12h, Get Platform ROM map protection.

  @retval EFI_SUCCESS        Get Platform ROM map protection successful.
**/
EFI_STATUS
EFIAPI
KernelFbtsUpdateProtectRomMap (
  VOID
  );

/**
  AH=12h, Get Platform ROM map protection.

  @retval EFI_SUCCESS        Get Platform ROM map protection successful.
**/
EFI_STATUS
EFIAPI
KernelFbtsUpadtePrivateRomMap (
  VOID
  );


/**
   AH=13h, Get Flash part information.

  @retval EFI_SUCCESS        Get Flash part information successful.
  @return Other              Get Flash part information failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetFlashPartInfo (
  VOID
  );

/**
  AH=1Bh,Skip module check allows and binary file transmissions.

  @retval EFI_SUCCESS   Success returns.
**/
EFI_STATUS
EFIAPI
KernelSkipMcCheckAndBinaryTrans (
  VOID
  );

/**
  AH=1Eh, Get whole BIOS ROM map.

  @retval EFI_SUCCESS                 FBTS get BIOS ROM map success.
  @return Others                      FBTS get BIOS ROM map failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsGetWholeBiosRomMap (
  VOID
  );

/**
  AH=14h, FBTS Read.

  @retval EFI_SUCCESS            FBTS read success.
  @return Others                 FBTS read failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsRead (
  VOID
  );

/**
  AH=15h,FBTS write.

  @retval EFI_SUCCESS            FBTS write success.
  @return Others                 FBTS write failed.
**/
EFI_STATUS
EFIAPI
KernelFbtsWrite (
  VOID
  );
/**
  AH=16h, This function uses to execute some specific action after the flash process is completed.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
KernelFbtsPurifyVariable (
  VOID
  );
#endif
