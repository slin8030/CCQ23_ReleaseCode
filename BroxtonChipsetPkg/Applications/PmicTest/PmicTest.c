/** @file
  Clovertrail I2C test application

  This application tests "UEFI Entropy-Gathering Protocol" 

Copyright (c) 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/Pmiclib.h>


//usage
void usage(void)
{
  Print(L"Usage: \n");
  Print(L"   PmicTest.efi read <offset> \n");
  Print(L"   PmicTest.efi write <offset> <value>\n");
  Print(L"   PmicTest.efi page0_read <offset> \n");
  Print(L"   PmicTest.efi page0_write <offset> <value>\n");
  Print(L"   PmicTest.efi dump \n");
  Print(L"   PmicTest.efi show <interrupt|gpio> \n");
  Print(L"   PmicTest.efi init \n");
  return;
}

//register dump
void dump(void)
{
  UINT8 reg= 0;

  for (; reg <= 0xC6; reg++)    {
     Print (L"reg:0x%x:0x%x\n", reg, PmicRead8(reg));
  }
  return ;
}
//gpio table

//init debug
void init(void)
{
  UINT8 DevId, RevId;

  PmicGetDevID(&DevId, &RevId);

  Print (L"PMIC Dev:0x%x Rev:0x%x\n", DevId, RevId);

  Print(L"   PmicTest.efi init start\n");
  PmicThermInit ();

  //SV Fix later  PmicGpioInit (NULL);
#if defined (TABLET_PF_ENABLE) && (TABLET_PF_ENABLE == 1)
  PmicGpioInit();
#endif

  PmicIntrInit ();

  PmicBcuInit ();

  PmicMiscInit ();
  Print(L"   PmicTest.efi init done\n");
}

//interrupt status



/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  //EFI_STATUS			            Status;
  UINTN  offset;
  UINTN  val=0;
  CHAR8 AsciiStr[20]={0};

  if (Argc < 2) {
    usage();
    return EFI_INVALID_PARAMETER;
  }

  UnicodeStrToAsciiStr(Argv[1], AsciiStr);

  if(0 == AsciiStrCmp(AsciiStr, "read")) {
    if (Argc != 3) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset    = StrHexToUintn (Argv[2]);
    val = PmicRead8((UINT8)offset) ;
    Print(L"PMIC read offset 0x%x value:0x%x\n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "write"))   {
    if (Argc != 4) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset  = StrHexToUintn (Argv[2]);
    val     = StrHexToUintn (Argv[3]);
    PmicWrite8 ((UINT8)offset, (UINT8)val) ;
    Print(L"PMIC write offset 0x%x value:0x%x \n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "page0_read")) {
    if (Argc != 3) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset    = StrHexToUintn (Argv[2]);
    val = PmicRead8_page0((UINT8)offset) ;
    Print(L"PMIC page0 read offset 0x%x value:0x%x\n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "page0_write"))   {
    if (Argc != 4) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset  = StrHexToUintn (Argv[2]);
    val     = StrHexToUintn (Argv[3]);
    PmicWrite8_page0 ((UINT8)offset, (UINT8)val) ;
    Print(L"PMIC page0 write offset 0x%x value:0x%x \n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "init"))   {
    init();
  }else if(0 == AsciiStrCmp(AsciiStr, "dump"))   {
    dump();
  }
  return EFI_SUCCESS;
}
