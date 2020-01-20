//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#include <Iosf.h>

VOID
DoShowMsgData(
  IN  UINT8    OPCode,
  IN  UINT8    PortID,
  IN  UINT32   m_Reg,
  OUT UINT32  *Data
  )
{
  UEFIMsgBus32Read(OPCode, PortID, m_Reg, *Data);  
}

VOID
DoWriteMsgData(
  IN UINT8   OPCode,
  IN UINT8   PortID,
  IN UINT32  m_Reg,
  IN UINT32  Data
  )
{
  UEFIMsgBus32Write(OPCode, PortID, m_Reg, Data);   
}


VOID
ShowHelp(
  VOID
)
{
  Print(L"IOSF-SB Read/Write    Version V0.0.1\n");
  Print(L"Please Input Command: Iosf.efi <-R|W> <OpCode> <PortID> <Register> [Data]\n");
  Print(L"           R             Read Message Bus \n");
  Print(L"           W             Write Message Bus \n");
  Print(L"           Eg:           Iosf -R 6 43 4100\n");
  Print(L"                         Iosf -W 7 43 4100 4B209\n");
  Print(L"           NOTE: All the parameters is in Hex Decimal format.\n");
  Print(L"           NOTE: Opcode varies for different IOSF-SB port. \n");
  Print(L"                 Common read opcode is 0x06,write opcode is 0x07.\n");
  Print(L"                 Please check the relevant spec for the accurate opcode on the given port\n");
  Print(L"          \n");
}

UINT32
StringToHex(
  IN CHAR16 *String
)
{
  UINT8   Index = 0;
  UINT32 Value = 0;
  
  while (String[Index] != 0){
  
    if ((String[Index] >= '0') && (String[Index] <= '9')){		
      Value = Value * 16 + (String[Index] - '0');
    }

    if ((String[Index] >= 'A') && (String[Index] <= 'F')){		
      Value = Value * 16 + (String[Index] - 'A' + 10);
    }

    if ((String[Index] >= 'a') && (String[Index] <= 'f')){		
      Value = Value * 16 + (String[Index] - 'a' + 10);
    }

    Index++;
  }
  return Value;
}

UINT8
AnalyzeArgument(
  IN CHAR16     **Argv,
  IN UINTN        Argc,
  OUT UINT8     *OpCode,
  OUT UINT8     *PortID,
  OUT UINT32   *mReg,
  OUT UINT32   *Data
)
{
  UINT8 Action = 0xFE;

  *OpCode = 0;
  *PortID   = 0;
  *mReg = 0;
  *Data = 0;        
  
  switch(Argv[1][1])
  {
    case 'R':
    case 'r':
      Action = 0;
      break;
  
    case 'W':
    case 'w':
      Action = 1;
      break;
  }

  *OpCode = (UINT8)StringToHex(Argv[2]);
  *PortID   = (UINT8)StringToHex(Argv[3]);
  *mReg    = StringToHex(Argv[4]);
  if (Argc == 6) {
    *Data = StringToHex(Argv[5]);
  }
  return Action;
}

EFI_STATUS
EFIAPI
IOSFEntry(
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
 )
{
  EFI_STATUS                                    Status;
  
  UINTN                                              Argc;
  UINT8                                              Type;
  UINT8                                              OpCode;
  UINT8                                              PortID;
  UINT32                                            mRegister;
  UINT32                                            Data;
  
  CHAR16                                           **Argv;
  EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol;
  EFI_SHELL_INTERFACE                      *EfiShellInterface;

  Status = gBS->OpenProtocol(ImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **)&EfiShellParametersProtocol,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );


  //Step 1: Get Argument 

  if (!EFI_ERROR(Status)) {
    //
    // use shell 2.0 interface
    //
    Argc = EfiShellParametersProtocol->Argc;
    Argv = EfiShellParametersProtocol->Argv;
  } else {
    //
    // try to get shell 1.0 interface instead.
    //
    Status = gBS->OpenProtocol(ImageHandle,
                    &gEfiShellInterfaceGuid,
                    (VOID **)&EfiShellInterface,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (!EFI_ERROR(Status)) {
      //
      // use shell 1.0 interface
      //
      Argc = EfiShellInterface->Argc;
      Argv = EfiShellInterface->Argv;
    } else {
      Print(L"Can't Find Shell Interface, Return!\n");
      return EFI_NOT_FOUND;
    }
  }

  //Step 2:  Analyze Argument 
  if (Argc < 5) {
    ShowHelp();
    return EFI_NOT_FOUND;
  }

  Type = AnalyzeArgument(Argv, Argc, &OpCode, &PortID, &mRegister, &Data);

  if (Type == 0){
    DoShowMsgData(OpCode, PortID, mRegister, &Data);
    Print(L" OpCode[%02x]    PortID:[%02x]    Register:[%08x]    Data:[%08x]\n", OpCode, PortID, mRegister, Data);
  } else if(Type == 1) {
    DoWriteMsgData(OpCode, PortID, mRegister, Data);
  }
   
  return EFI_SUCCESS;
}
