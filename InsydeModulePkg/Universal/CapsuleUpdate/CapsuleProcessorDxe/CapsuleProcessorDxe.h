/** @file
  Type definitions for Capsule Processor Dxe module

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CAPSULE_PROCESSOR_DXE_H_
#define _CAPSULE_PROCESSOR_DXE_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/VariableLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Library/SeamlessRecoveryLib.h>
#include <Library/CapsuleUpdateCriteriaLib.h>
#include <Library/OemGraphicsLib.h>
#include <Library/BvdtLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/FirmwareManagement.h>
#include <Protocol/SimpleTextOut.h>
#include <Guid/EfiSystemResourceTable.h>
#include <Guid/FmpCapsule.h>
#include <Guid/ImageAuthentication.h>
#include <SecureFlash.h>

#define CAPSULE_FMP_SIGNATURE                   SIGNATURE_32('C','F','M','P')
#define CAPSULE_FMP_INSTANCE_FROM_THIS(a)       CR (a, CAPSULE_FMP_INSTANCE, Fmp, CAPSULE_FMP_SIGNATURE)

typedef struct {
  LIST_ENTRY                                    Link;
  UINT8                                         ImageIndex;
  EFI_GUID                                      ImageTypeGuid;
  UINT64                                        Attributes;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL_SET_IMAGE    SetImage;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL_CHECK_IMAGE  CheckImage;
} CAPSULE_FMP_ENTRY;

typedef struct {
  UINTN                                         Signature;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL              Fmp;
  UINTN                                         CapsuleProcessorCount;
  LIST_ENTRY                                    CapsuleProcessorListHead;
} CAPSULE_FMP_INSTANCE;

extern CAPSULE_FMP_INSTANCE                     mCapsuleFmp;

EFI_STATUS
InstallCapsuleInWindowsUx (
  VOID
  );

EFI_STATUS
InstallCapsuleInUefiImage (
  VOID
  );

EFI_STATUS
InstallCapsuleInUefiFmp (
  VOID
  );

#endif