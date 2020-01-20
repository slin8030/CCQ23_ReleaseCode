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
#include <Library/I2clib.h>

#define ULPMC_I2C_BUSNO   0x0
#define ULPMC_I2C_ADDR    0x78

//#define ULPMC_I2C_BUSNO   0x6
//#define ULPMC_I2C_ADDR    0x6E

//usage
void usage(void)
{
  Print(L"Usage: \n");
  Print(L"   UlpmcDbg.efi read <offset> \n");
  Print(L"   UlpmcDbg.efi write <offset> <value>\n");
//  Print(L"   UlpmcDbg.efi dump \n");
//  Print(L"   UlpmcDbg.efi show <interrupt|gpio> \n");
//  Print(L"   UlpmcDbg.efi init \n");
  return;
}


UINT8
EFIAPI
UlpmcRead8 (
  IN      UINT8                     Register
  )
{
  UINT8 value=0;
  EFI_STATUS rc=0;
  
  rc = ByteReadI2C(ULPMC_I2C_BUSNO, ULPMC_I2C_ADDR, Register, 1, &value);
  if (EFI_SUCCESS != rc)  {
     DEBUG ((DEBUG_INFO, "UlpmcRead8 failed :0x%x\n", rc));
     return 0xff;
  }
  return value;
}


EFI_STATUS
EFIAPI
UlpmcWrite8 (
  IN      UINT8                     Register,
  IN      UINT8                     Value
  )
{
  //UINT8 value=0;
  EFI_STATUS rc=0;

  DEBUG ((DEBUG_INFO, "UlpmcWrite8:---------------0x%x,0x%x\n", &Value,Value));
  rc = ByteWriteI2C(ULPMC_I2C_BUSNO, ULPMC_I2C_ADDR, Register, 1, &Value);
  if (EFI_SUCCESS != rc)  {
    DEBUG ((DEBUG_INFO, "UlpmcWrite8 failed :0x%x\n", rc));
    return rc;
  }
  return EFI_SUCCESS;
}



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
    val = UlpmcRead8((UINT8)offset) ;
    Print(L"Ulpmc read offset 0x%x value:0x%x\n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "write"))   {
    if (Argc != 4) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset  = StrHexToUintn (Argv[2]);
    val     = StrHexToUintn (Argv[3]);
    UlpmcWrite8 ((UINT8)offset, (UINT8)val) ;
    Print(L"Ulpmc write offset 0x%x value:0x%x \n", offset, val);
  }
  return EFI_SUCCESS;
}
