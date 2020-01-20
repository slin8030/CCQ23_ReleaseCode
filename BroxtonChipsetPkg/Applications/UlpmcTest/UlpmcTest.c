/** @file
  BatteryInfo shell tool.

  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
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

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellParameters.h>
 
#include <KBInput.h>
#include <Screen.h>
#include <WriteFile.h>
#include <Monitor.h>

EFI_BOOT_SERVICES     *gBS;
EFI_RUNTIME_SERVICES  *gRT;
EFI_SYSTEM_TABLE      *gST;

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
ULPMCTestEntry(
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  EFI_INPUT_KEY  Key;
  EFI_STATUS     Status;
  CHAR16         Name[32];
  CHAR8          String[512];  
  EFI_FILE       *FilePtr;
  EFI_FILE       *MonitorFilePtr;
  MONITOR_ITEM   MonitorItem;  
  UINTN          Argc;
  CHAR16         **Argv;
  EFI_SHELL_PARAMETERS_PROTOCOL *EfiShellParametersProtocol;
  EFI_SHELL_INTERFACE           *EfiShellInterface;  
    
  gBS = SystemTable->BootServices;
  gRT = SystemTable->RuntimeServices;
  gST = SystemTable;
  
  CleanScreen ();  

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

  if (Argc == 1) {
    GetFileName (Name);
  } else {
    StrCpy (Name, Argv[1]);
  }

  //
  // Create the Log file.
  //
  Status = InitFileSystem (
             ImageHandle, 
             Name, 
             EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 
             &FilePtr
             );
  if (EFI_ERROR (Status)) {
    Print (L"Create File [%s] Failed\n", Name);
    return EFI_NOT_STARTED;
  }
  
  //
  // Get the monitor device list.(Device needs to be monitored).
  //
  Status = InitFileSystem (
             ImageHandle, 
             L"Monitor.lst", 
             EFI_FILE_MODE_READ, 
             &MonitorFilePtr
             );    
  if (EFI_ERROR (Status)) {
    Print (L"Get Monitor.lst Failed\n");
    return EFI_NOT_STARTED;
  }

  //
  // Analyze the Monitor.lst 
  //
  GetMonitorList (MonitorFilePtr, &MonitorItem);
  MonitorFilePtr->Close(MonitorFilePtr);
  
#if 0
  if(1)
  {
    UINTN Index;
    for(Index = 0; Index < MonitorItem.MonitorNum; Index++)
    {
      Print(L"Index: [%02x]\n", Index);
      Print(L"      Monitor Address:[%02x]\n", MonitorItem.MonitorRegs[Index].MonitorAddress);	
      Print(L"      Monitor Reg   :[%02x]\n", MonitorItem.MonitorRegs[Index].MonitorReg);
      Print(L"      Monitor Start Bit:[%02x]\n", MonitorItem.MonitorRegs[Index].MonitorStartBit);
      Print(L"      Monitor End Bit:[%02x]\n", MonitorItem.MonitorRegs[Index].MonitorEndBit);
      Print(L"      Monitor Value:[%02x]\n", MonitorItem.MonitorRegs[Index].MonitorValue);
      Print(L"      Change Address:[%02x]\n", MonitorItem.MonitorRegs[Index].ChangeAddress);
      Print(L"      Change REG:[%02x]\n", MonitorItem.MonitorRegs[Index].ChangeReg);
      Print(L"      Change Start Bit:[%02x]\n", MonitorItem.MonitorRegs[Index].ChangeStartBit);
      Print(L"      Change End Bit:[%02x]\n", MonitorItem.MonitorRegs[Index].ChangeEndBit);
      Print(L"      Change Value:[%02x]\n", MonitorItem.MonitorRegs[Index].ChangeValue);
    }
  }
#endif

  //
  // Show the Charger Information
  //
  CleanScreen ();  
  ShowChargerInformation (String);
  WriteString2File (FilePtr, String);
  
  do {
    ShowTimeBar (String);	
    WriteString2File (FilePtr, String);	
    
    //
    // Show Battery Information
    //
    ShowBatteryInformation (String);
    WriteString2File (FilePtr, String);

    //
    // Check the Monitor List Register.
    //
    CheckMonitorRegister (MonitorItem, FilePtr);

    //
    // Get key for 1 sec.
    // If user not press any key, continue to dump battery information.
    //
    Key.ScanCode = 0;
    Key.UnicodeChar = 0;	
    GetKey(&Key);
    if((Key.ScanCode == 0x0) && (Key.UnicodeChar == 0x0)) {
      continue;
    }
  } while(Key.ScanCode != SCAN_ESC);
  
  FilePtr->Close(FilePtr);
  gBS->FreePool(MonitorItem.MonitorRegs);
  
  SetFrmMode(0xFF);
  CleanScreen();
 
  return EFI_SUCCESS;
}

