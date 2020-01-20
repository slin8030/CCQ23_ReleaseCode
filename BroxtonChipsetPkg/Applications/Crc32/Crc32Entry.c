/** @file
  Crc32 shell tool.

  Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
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
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellParameters.h>
 
#include "Crc32Func.h"

 EFI_BOOT_SERVICES     *gBS;
 EFI_RUNTIME_SERVICES  *gRT;
 EFI_SYSTEM_TABLE      *gST;
 SHELL_FILE_HANDLE     FileHandle;
 
/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI	
Crc32Entry(
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  EFI_STATUS         Status;
  CHAR16             Name[32];
  UINTN              Argc;
  CHAR16             **Argv;
  UINT32             Crc;
  UINT64             Size;
  VOID               *StartAddr;
  UINTN              Length;

  EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol;
  EFI_SHELL_INTERFACE           *EfiShellInterface;  

  gBS = SystemTable->BootServices;
  gRT = SystemTable->RuntimeServices;
  gST = SystemTable;
  
  gBS->SetMem (Name,sizeof(Name), 0);
 
  //
  // Try to get shell 2.0 interface instead.
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **)&EfiShellParametersProtocol,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (!EFI_ERROR (Status)) {
    Argc = EfiShellParametersProtocol->Argc;
    Argv = EfiShellParametersProtocol->Argv;
  } else {
    //
    // Try to get shell 1.0 interface instead.
    //
    Status = gBS->OpenProtocol (
                    ImageHandle,
                    &gEfiShellInterfaceGuid,
                    (VOID **) &EfiShellInterface,
                    ImageHandle,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {
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
  if(Argc == 3) {	//File Crc32

  if(StrCmp (Argv[1], L"-f") == 0){

    StrCpy (Name, Argv[2]);

    Status = ShellFileExists(Name);
    if(EFI_ERROR(Status)){
      Print(L"File %s Is Not Exist\n",Name);
      goto ERROR;
    }

    Status = ShellOpenFileByName(Name, &FileHandle, EFI_FILE_MODE_READ, 0);
    if(EFI_ERROR(Status)){
      if(Status == EFI_NOT_FOUND){
        Print(L"File Not Found !\n");
        goto ERROR;
      }
    }

    Status = ShellGetFileSize(FileHandle,&Size);
    if(EFI_ERROR(Status)){
      Print(L"ShellGetFileSize Error! \n");
      goto ERROR;
    }

    Crc32File(Size,&Crc);

    ShellCloseFile(&FileHandle);

    } else {
    Usage();
    return EFI_SUCCESS;
  }

  }
  else if (Argc == 5){//Memory Crc32

    if(StrCmp (Argv[1], L"-s") == 0){
      StartAddr = (VOID*)StrHexToUintn(Argv[2]);	
      Print(L"StartAddr = 0x%02x \n",StartAddr);	
    }else {
      Usage();
      return EFI_SUCCESS;
    }

    if(StrCmp (Argv[3], L"-l") == 0){
      Length = StrHexToUintn(Argv[4]);
      Print(L"Length = 0x%02x \n",Length);	
    }else {
      Usage();
      return EFI_SUCCESS;
    }

  if(Length == 0x00)
    return EFI_SUCCESS;

  Crc32Mem(StartAddr,Length,&Crc);

  }else{
    Usage();
    return EFI_SUCCESS;
  }

  Print(L"Crc32 Value Is: ",Name);

  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_RED, EFI_BLACK));	
  Print(L"[0x%02x]",Crc);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLACK));	
 
  return EFI_SUCCESS;

ERROR:
  return Status;
  
}

