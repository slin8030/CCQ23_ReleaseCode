/** @file
  1. Access CMOS bank 2 (I/O 72:73) fill offset 0x17 with 0x00
  2. Issue 0x0E to I/O 0xCF9 to perform cold reset

;******************************************************************************
;* Copyright (c) 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/CmosLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <ChipsetCmos.h>
#include <Library/UefiRuntimeServicesTableLib.h>

/**

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
TriggerRecoveryEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS             Status;
  EFI_INPUT_KEY          Key;
  UINT8                  CMosData = 0;
  UINT32                 TempAttribute;
  
  TempAttribute = gST->ConOut->Mode->Attribute;
  Status = EFI_SUCCESS;
  gST->ConOut->ClearScreen (gST->ConOut);
  CMosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, 0x17);

  Print (L"**************************************************************************************************\n");
  Print (L"**                         Insyde Recovery test Tool     Version: 1.0                           **\n");
  Print (L"**                                                                                              **\n");
  Print (L"**   Support platform: Apollo Lake                                                              **\n");
  Print (L"**   This tool will fill value 0x01 to CMOS bank 2: then next boot, bios will run recovery flow **\n");
  Print (L"**   Value of CMOS bank 2 offset 0x17 before update:");
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW);
  Print (L" 0x%2x", CMosData);
  gST->ConOut->SetAttribute (gST->ConOut, TempAttribute);
  Print (L"                                       **\n");
  Print (L"**************************************************************************************************\n");
  Print (L"\n\nPress any key to start...\n");

  //
  // Print message and wait for key stoke
  //
  do {
    gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  } while (Key.UnicodeChar == 0x00);


  CMosData = 0x1;

  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, 0x17, CMosData);

  //Full reset
  gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  
  Print (L"System reseting...\n");
  Print (L"   System reseting...\n");
  Print (L"      System reseting...\n");

  return Status;
}



