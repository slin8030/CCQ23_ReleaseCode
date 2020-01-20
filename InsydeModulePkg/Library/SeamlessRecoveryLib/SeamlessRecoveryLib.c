/** @file
   Seamless Recovery support library for Windows UEFI Firmware Update Platform

;******************************************************************************
;* Copyright (c) 2013 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/FdSupportLib.h>
#include <Library/FlashRegionLib.h>
#include <Protocol/SmmFwBlockService.h>
#include <Library/UefiBootServicesTableLib.h>

/**
  Check whether it is in firmware failure recovery mode

  @param  None

  @retval TRUE      The system is in firmware failure recovery mode
          FALSE     The system is not in firmware failure recovery mode

**/
BOOLEAN
IsFirmwareFailureRecovery (
  VOID
  )
{
  EFI_GUID *RecoverySig;

  RecoverySig = (EFI_GUID *)(UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1);
  if ( CompareGuid( PcdGetPtr(PcdSeamlessRecoverySignature), RecoverySig) ) {
    return TRUE;
  }

  return FALSE;
}

/**
  Set firmware updating in process signature

  @param  None

  @returns None

**/
VOID
SetFirmwareUpdatingFlag (
  BOOLEAN FirmwareUpdatingFlag
  )
{
  EFI_STATUS  Status;
  UINTN       Size;
  EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL     *SmmFwbService;
  UINT32                                BlocksSize;

  if ( (!FirmwareUpdatingFlag) && (!IsFirmwareFailureRecovery()) ) {
    return;
  }

  BlocksSize = GetFlashBlockSize();

  Status = gBS->LocateProtocol (
                  &gEfiSmmFwBlockServiceProtocolGuid,
                  NULL,
                  (VOID **)&SmmFwbService
                  );
  if (EFI_ERROR (Status)) {
    Status = FlashErase( (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1), BlocksSize);
    if (EFI_ERROR(Status)) {
      ASSERT_EFI_ERROR(Status);
      return;
    }
    if (FirmwareUpdatingFlag) {
      Size = sizeof(EFI_GUID);
      FlashProgram (
        (UINT8 *)(UINTN)FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1),
        PcdGetPtr(PcdSeamlessRecoverySignature),
        &Size, 
        (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1)
        );
    }
  } else {
    
    Status = SmmFwbService->EraseBlocks(
                              SmmFwbService,
                              (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1),
                              (UINTN *) &BlocksSize
                              );
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return;
    }
    if (FirmwareUpdatingFlag) {
      Size = sizeof (EFI_GUID);
      Status = SmmFwbService->Write (
                                SmmFwbService,
                                (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1),
                                (UINTN *) &Size,
                                (UINT8 *) PcdGetPtr (PcdSeamlessRecoverySignature)
                                );
    }
  }
}