//;******************************************************************************
//;* Copyright (c) 1983-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
/*++ 
  Module Name:
  
    OEMBadgingString.c
  
  Abstract:
  
    OEM can define badging string in this file. 
    
--*/
#include <OEMBadgingString.h>
#include <Protocol/SetupUtility.h>
#include <Library/HiiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/printlib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

OEM_BADGING_STRING mOemBadgingString[] = {
  //
  // OEM can modify the background and foreground color of the OEM dadging string through through the below data
  // for example:
  //   { 50, 280, { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_CPUID ), GetCpuId },
  //     { 0x00, 0x00, 0x00, 0x00 } indicate the foreground color { Blue, Green, Red, Reserved }
  //     { 0xFF, 0xFF, 0xFF, 0x00 } indicate the background color { Blue, Green, Red, Reserved }
  //
  { OEM_STRING_GRAPHICAL_LOCATION_X,   OEM_STRING_GRAPHICAL_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_ESC   ), NULL },
};
OEM_BADGING_STRING mOemBadgingStringInTextMode[] = {
  {       OEM_STRING_LOCATION_X,       OEM_STRING1_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_CPUID ), GetCpuId },
  {       OEM_STRING_LOCATION_X,       OEM_STRING2_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_ESC   ), NULL}
};

OEM_BADGING_STRING mOemBadgingStringAfterSelect[4][BADGING_STRINGS_COUNT] = {
 {
  { OEM_STRING_GRAPHICAL_ESC_SELECT_LOCATION_X, OEM_STRING_GRAPHICAL_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_ESC_SELECT ), NULL}
 },
 {
  { OEM_STRING_GRAPHICAL_LOCATION_X, OEM_STRING_GRAPHICAL_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_F2_SELECT ), NULL}
 },
 {
  { OEM_STRING_GRAPHICAL_LOCATION_X, OEM_STRING_GRAPHICAL_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_F12_SELECT ), NULL}
 },
 {
  {       OEM_STRING_LOCATION_X,       OEM_STRING2_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_F12_SELECT    ), NULL     }
 }
};


OEM_BADGING_STRING mOemBadgingStringAfterSelectInTextMode[4][BADGING_STRINGS_COUNT_TEXT_MODE] = {
  {
    { OEM_STRING_LOCATION_X, OEM_STRING1_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_CPUID ), GetCpuId },
    { OEM_STRING_LOCATION_X, OEM_STRING2_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_ESC_SELECT), NULL}
  },
  {
    { OEM_STRING_LOCATION_X, OEM_STRING1_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_CPUID  ), GetCpuId },
    { OEM_STRING_LOCATION_X, OEM_STRING2_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_F2_SELECT), NULL}
  },
  {
    { OEM_STRING_LOCATION_X, OEM_STRING1_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_CPUID ), GetCpuId },
    { OEM_STRING_LOCATION_X, OEM_STRING2_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_F12_SELECT), NULL}
  },
  {
    { OEM_STRING_LOCATION_X, OEM_STRING1_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_CPUID  ), GetCpuId },
    { OEM_STRING_LOCATION_X, OEM_STRING2_LOCATION_Y, { 0xFF, 0xFF, 0xFF, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, STRING_TOKEN ( STR_OEM_BADGING_STR_F12_SELECT), NULL}
  }
};

EFI_STATUS
GetId(
  IN OUT UINTN    *CpuId
)
{
  UINT32 RegEax;
  AsmCpuid (0x01, &RegEax, NULL, NULL, NULL);

  *CpuId = RegEax;

  return EFI_SUCCESS;
}

BOOLEAN
GetCpuId(
  IN OEM_BADGING_STRING  *Structure,
  OUT CHAR16             **StringData
)
{
//[-start-170206-IB07400838-modify]//
  CHAR16                                *Str2;
  UINTN                                 CpuId=0;
  EFI_STATUS                            Status;
  UINTN                                 StringDataSize;
  CHAR16                                *Str3;

  Str3 = HiiGetPackageString (&gEfiCallerIdGuid, Structure->StringToken, NULL);
  if (Str3 == NULL) {
    return FALSE;
  }

  Str2 = AllocateZeroPool (100);
  if (Str2 == NULL) {
    return FALSE;
  }

  Status = GetId(&CpuId);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  UnicodeValueToString (Str2, RADIX_HEX, CpuId, 0);      

  StringDataSize = StrSize (Str3) + StrSize (Str2);
  *StringData = AllocateZeroPool (StringDataSize);
  if (*StringData == NULL) {
    return FALSE;
  }
  StrCpy (*StringData, Str3);
  StrCat (*StringData, Str2);

  FreePool (Str2);
  FreePool (Str3);

  return TRUE;
//[-end-170206-IB07400838-modify]//
}
