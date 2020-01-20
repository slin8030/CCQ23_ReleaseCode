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
#include <Library/i2clib.h>


//usage
void usage(void)
{
  Print(L"Usage: \n");
  Print(L"   I2cTest.efi read  <busno> <devaddr> <offset>\n");
  Print(L"   I2cTest.efi write <busno> <devaddr> <offset> <value>\n");
  Print(L"   I2cTest.efi dump  <busno> <devaddr>\n");
  return;
}

void dump(UINT8 i2cbus, UINT8 i2caddress)
{
  UINT32  reg  =0;
  UINT8  value =0;  
  EFI_STATUS rc=0;

  for (; reg <= 0xFF; reg++)    {
    rc = ByteReadI2C((UINT8)i2cbus, (UINT8)i2caddress, (UINT8)reg, 1, &value);
    if (EFI_SUCCESS != rc)  {
       DEBUG ((DEBUG_INFO, "ByteReadI2C failed :0x%x\n", rc));
       return ;
    }
     Print (L"0x%x:0x%x\n", reg, value);
  }
  return ;
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
  UINTN  offset;
  UINTN  val=0;
  UINTN  i2cAddr=0;
  UINTN  BusNo=0;
  UINT8  value=0;
  EFI_STATUS rc=0;
      
  CHAR8 AsciiStr[20]={0};
  
  if (Argc < 2) {
    usage();
    return EFI_INVALID_PARAMETER;
  }
 
  UnicodeStrToAsciiStr(Argv[1], AsciiStr);

  if(0 == AsciiStrCmp(AsciiStr, "read")) {
    if (Argc != 5) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    
    BusNo   = StrHexToUintn (Argv[2]);
    i2cAddr = StrHexToUintn (Argv[3]);
    offset  = StrHexToUintn (Argv[4]);
    
    rc = ByteReadI2C((UINT8)BusNo, (UINT8)i2cAddr, (UINT8)offset, 1, &value);
    if (EFI_SUCCESS != rc)  {
       DEBUG ((DEBUG_INFO, "ByteReadI2C failed :0x%x\n", rc));
       return 0xff;
    }

    Print(L"i2c read bus 0x%X dev:0x%x offset 0x%x value:0x%x \n", BusNo, i2cAddr, offset, value);    
  }else if(0 == AsciiStrCmp(AsciiStr, "dump")) {
    if (Argc != 4) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    
    BusNo   = StrHexToUintn (Argv[2]);
    i2cAddr = StrHexToUintn (Argv[3]);
    Print(L"i2c dump bus 0x%X dev:0x%x \n", BusNo, i2cAddr);    
    dump((UINT8)BusNo, (UINT8)i2cAddr);
      
  }else if(0 == AsciiStrCmp(AsciiStr, "write"))   {
    if (Argc != 6) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    
    BusNo  = StrHexToUintn (Argv[2]);
    i2cAddr = StrHexToUintn (Argv[3]);
    offset  = StrHexToUintn (Argv[4]);
    val     = StrHexToUintn (Argv[5]);
    value   = (UINT8)val;
    
    rc = ByteWriteI2C((UINT8)BusNo, (UINT8)i2cAddr, (UINT8)offset, 1, &value);
    if (EFI_SUCCESS != rc)  {
       DEBUG ((DEBUG_INFO, "ByteWriteI2C failed :0x%x\n", rc));
       return 0xff;
    }

    Print(L"i2c write bus 0x%X dev:0x%x offset 0x%x value:0x%x \n", BusNo, i2cAddr, offset, value);
  }
  
  return EFI_SUCCESS;
}
