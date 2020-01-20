/** @file
 Implement the Chipset Servcie IHISI FBTS subfunction for this driver.

***************************************************************************
* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/
#ifndef _CHIPSET_SERVICES_IHISI_FBTS_H_
#define _CHIPSET_SERVICES_IHISI_FBTS_H_
#include <H2OIhisi.h>
#include <Protocol/GlobalNvsArea.h>
#include <Protocol/SmmFwBlockService.h>
#include <Library/SpiAccessLib.h>

extern UINT32                               mIhisiFlash;
extern EFI_GLOBAL_NVS_AREA                 *mGlobalNvsArea;
extern EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL   *mSmmFwBlockService;
//[-start-160810-IB11270161-add]//
typedef enum {
  APL_DESC_REGION              = 0,
  APL_BIOS_REGION,
  APL_ME_REGION,
  APL_GBE_REGION,
  APL_PLATFORM_DATA_REGION,
  APL_DEVICE_EXPANSION,
  APL_MAX_FLASH_REGION,
  APL_ME_SUB_REGION_IGNITION   = 0x20,
  APL_REGION_TYPE_OF_EOS       = 0xff,
} APL_FLASH_REGION_TYPE;
//[-end-160810-IB11270161-add]//
/**
  AH=10h(FbtsGetSupportVersion), Get FBTS supported version and FBTS permission.

  @param[in] VersionStr          Flash tool version
  @param[in, out] Permission     Permission
**/
VOID
EFIAPI
IhisiFbtsGetPermission (
  IN FBTS_TOOLS_VERSION_BUFFER              *VersionPtr,
  IN OUT UINT16                             *Permission
  );

/**
  AH=12h(FbtsGetPlatformRomMap), Get Oem define flash map.

  @param[in, out] RomMapBuffer          Pointer to the returned platform's ROM map protection structure.
                                        After version 1.7.6, InsydeFlash will skip this structure if it found definition in BVDT
  @param[in, out] PrivateRomMapBuffer   Pointer to the returned platform's private map structure.
                                        Flash utility will not flash these areas
                                        (even userenter /all in flashit or all=1 in platform.ini).

  @retval EFI_SUCCESS       Get OEM flash map successful.
  @retval EFI_UNSUPPORTED   FBTS_OEM_ROM_MAP_COUNT is 0 or module rom map buffer is full.
**/
EFI_STATUS
EFIAPI
IhisiFbtsGetOemFlashMap (
  IN OUT   FBTS_PLATFORM_ROM_MAP       **RomMapBuffer,
  IN OUT   FBTS_PLATFORM_PRIVATE_ROM   **PrivateRomMapBuffer
  );

/**
  AH=15h(FBTS write) : Hook function before Write process.

  @param[in, out] WriteDataBufferPtr    Pointer to data buffer for write.
  @param[in, out] WriteSizePtr          Write size.
  @param[in, out] RomBaseAddressPtr     Target linear address to write.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoBeforeWriteProcess (
  IN OUT    UINT8                      *WriteDataBufferPtr,
  IN OUT    UINTN                      *WriteSizePtr,
  IN OUT    UINTN                      *RomBaseAddressPtr
  );

/**
  AH=15h(FBTS write) : Hook function after Write process.

  @param[in]      WriteStatus

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoAfterWriteProcess (
  IN EFI_STATUS        WriteStatus
  );

/**
  AH=14h(FbtsRead) : Hook function before read process

  @param[in, out] ReadAddress           Target linear address to read.
  @param[in, out] ReadSize              Read size.
  @param[in, out] DataBuffer            Pointer to returned data buffer.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoBeforeReadProcess (
  IN OUT UINTN                                   *ReadAddress,
  IN OUT UINTN                                   *ReadSize,
  IN OUT UINT8                                   *DataBuffer
  );

/**
  AH=14h(FbtsRead) : Hook function fter read process

  @param[in]      SmmFwBlockService     SmmFwBlockService protocl.

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoAfterReadProcess (
  IN EFI_STATUS        ReadStatus
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute Ap terminate.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsApTerminated (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute normal flash.
                        (Update whole image or BIOS region by normal or secure flash.)

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
IhisiFbtsNormalFlash (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function uses to execute Partial flash.
                        (Update specific address or update single firmware volume.)

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsPartialFlash (
  VOID
  );

/**
  AH=16h(Fbtscomplete), This function is a hook funciton berfore ApRequest execute.

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
IhisiFbtsOemComplete (
  IN UINT8           ApRequest
  );

/**
  Function to do system shutdown.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsShutDown (
  VOID
  );

/**
  Function to reboot system.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsReboot(
  VOID
  );

/**
  Function to flash complete do nothing.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsApRequestDoNothing (
  VOID
  );

#endif
