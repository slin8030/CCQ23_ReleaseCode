/** @file
  String support

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

/**
  String support

Copyright (c) 2004 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "String.h"

EFI_HII_HANDLE gStringPackHandle;

EFI_GUID mBdsStringPackGuid = {
  0x7bac95d3, 0xddf, 0x42f3, {0x9e, 0x24, 0x7c, 0x64, 0x49, 0x40, 0x37, 0x9a}
};

/**
  Initialize HII global accessor for string support.

**/
VOID
InitializeStringSupport (
  VOID
  )
{
  gStringPackHandle = HiiAddPackages (
                        &mBdsStringPackGuid,
                        gImageHandle,
                        BdsDxeStrings,
                        NULL
                        );
  ASSERT (gStringPackHandle != NULL);
}

/**
  Get string by string id from HII Interface

  @param Id              String ID.

  @retval  CHAR16 *  String from ID.
  @retval  NULL      If error occurs.

**/
CHAR16 *
GetStringById (
  IN  EFI_STRING_ID   Id
  )
{
  return HiiGetString (gStringPackHandle, Id, NULL);
}


/**
  Update the content of STR_BOOT_DEVICE_ERROR_MESSAGE string to CSM16 for boot error message
  if this string isn't an empty string.

  @retval EFI_SUCCESS    Update boot error message to CSM16 successfully.
  @retval EFI_NOT_FOUND  Cannot find default boot error message in CSM16.
  @retval EFI_NOT_READY  STR_BOOT_DEVICE_ERROR_MESSAGE token is empty string.
**/
EFI_STATUS
UpdateBootMessage (
  VOID
  )
{
  EFI_LEGACY_REGION_PROTOCOL        *LegacyRegion;
  EFI_LEGACY_BIOS_PROTOCOL          *LegacyBios;
  EFI_COMPATIBILITY16_TABLE         *Table;
  EFI_IA32_REGISTER_SET             Regs;
  EFI_STATUS                        Status;
  UINT8                             *Ptr;
  BOOT_MESSAGE                      *BootMsg;
  CHAR16                            *NewString;
  UINTN                             StringLen;
  UINT8                             *NewStringAddress;

  //
  // See if the Legacy Region Protocol is available
  //
  Status = gBS->LocateProtocol (&gEfiLegacyRegionProtocolGuid, NULL, (VOID **)&LegacyRegion);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&LegacyBios);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BootMsg = NULL;
  for (Ptr = (UINT8 *) ((UINTN) 0xF0000);
       Ptr < (UINT8 *) ((UINTN) 0x100000);
       Ptr += 0x10) {
    //
    // "boot device error string (BDES)"
    //
    if (*(UINT32 *) Ptr == SIGNATURE_32 ('B', 'D', 'E', 'S')) {
      BootMsg = (BOOT_MESSAGE *) Ptr;
      break;
    }
  }
  if (BootMsg == NULL) {
    return EFI_NOT_FOUND;
  }

  NewString = GetStringById (STRING_TOKEN (STR_BOOT_DEVICE_ERROR_MESSAGE));
  if (NewString == NULL) {
    return EFI_NOT_READY;
  }
  StringLen = StrLen (NewString);
  if (StringLen == 0) {
    return EFI_NOT_READY;
  }

  if (StringLen <= BOOT_FAIL_STRING_LENTH) {
    Status = LegacyRegion->UnLock (LegacyRegion, 0xF0000, 0x10000, NULL);
    ZeroMem (&BootMsg->String, BOOT_FAIL_STRING_LENTH);
    UnicodeStrToAsciiStr (NewString, &BootMsg->String);
    Status = LegacyRegion->Lock (LegacyRegion, 0xF0000, 0x10000, NULL);
    return EFI_SUCCESS;
  }

  Table = NULL;
  for (Ptr = (UINT8 *) ((UINTN) 0xFE000);
       Ptr < (UINT8 *) ((UINTN) 0x100000);
       Ptr += 0x10) {
    if (*(UINT32 *) Ptr == SIGNATURE_32 ('I', 'F', 'E', '$')) {
      Table   = (EFI_COMPATIBILITY16_TABLE *) Ptr;
      break;
    }
  }
  if (Table == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = LegacyRegion->UnLock (LegacyRegion, 0xE0000, 0x10000, NULL);
  ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
  Regs.X.AX = Legacy16GetTableAddress;
  Regs.X.BX = 0x0002;
  Regs.X.CX = (UINT16)StringLen;
  Regs.X.DX = 1;
  LegacyBios->FarCall86 (
                LegacyBios,
                Table->Compatibility16CallSegment,
                Table->Compatibility16CallOffset,
                &Regs,
                NULL,
                0);

  NewStringAddress = (UINT8 *)(UINTN)(Regs.X.DS*16 + Regs.X.BX);

  BootMsg->StrSegment = (UINT16)(((UINTN)NewStringAddress >> 4) & 0xF000);
  BootMsg->StrOffset  = (UINT16)(UINTN)NewStringAddress;

  ZeroMem (NewStringAddress, StringLen);
  UnicodeStrToAsciiStr (NewString, (CHAR8 *) NewStringAddress);
  Status = LegacyRegion->Lock (LegacyRegion, 0xE0000, 0x10000, NULL);

  return EFI_SUCCESS;
}