/** @file

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

#ifndef _OEM_ACPI_PLATFORM_H_
#define _OEM_ACPI_PLATFORM_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/Uefilib.h>
#include <IndustryStandard/Acpi.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/AcpiSupport.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <Library/PcdLib.h>

#ifndef EFI_SIGNATURE_16
#define EFI_SIGNATURE_16(A, B)        ((A) | (B << 8))
#endif
#ifndef EFI_SIGNATURE_32
#define EFI_SIGNATURE_32(A, B, C, D)  (EFI_SIGNATURE_16 (A, B) | (EFI_SIGNATURE_16 (C, D) << 16))
#endif



extern EFI_ACPI_SUPPORT_PROTOCOL                 *mAcpiSupport;
EFI_STATUS
OemUpdateOemTableID (
  VOID
  );
#endif
