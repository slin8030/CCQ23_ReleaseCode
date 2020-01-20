/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Uefi.h>

#include <Protocol/H2ORamDiskServices.h>

#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/ShellLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HiiLib.h>

//
// Global Variables
//
EFI_HII_HANDLE      mHiiHandle;

SHELL_PARAM_ITEM    RamDiskParamList[] = {
  {
    L"-c",
    TypeFlag
  },
  {
    L"-d",
    TypeValue
  },
  {
    L"-s",
    TypeValue
  },
  {
    L"-r",
    TypeFlag
  },
  {
    L"-i",
    TypeFlag
  },
  {
    L"-?",
    TypeFlag
  },
  {
    NULL,
    TypeMax
  },
};

/**
  Diplay RAM Disk devices information.

  @param[in] RamDiskServicesProtocol    Indicates a pointer to the H2O_RAM_DISK_SERVICES_PROTOCOL
                                        context.  
**/
VOID
DisplayRamDiskInfo (
  IN H2O_RAM_DISK_SERVICES_PROTOCOL     *RamDiskServicesProtocol
  )
{
  UINT32                 Index;
  UINT32                 RamDiskDeviceNum;
  UINT64                 RamDiskId;
  UINT64                 RamDiskSize;
  BOOLEAN                Removable;
  EFI_STATUS             Status = EFI_NOT_FOUND;

  RamDiskDeviceNum = RamDiskServicesProtocol->GetRamDiskCount ();
  RamDiskId     = 0;
  RamDiskSize   = 0;
  
  Print (L"RAM Disk devices Count = %d\n", RamDiskDeviceNum);
  
  if (RamDiskDeviceNum == 0) {
    return;
  }
  
  Print (L"NO  ID                  Size(MB)  Removable\n");
  for (Index = 0; Index < RamDiskDeviceNum; Index++) {
    Status = RamDiskServicesProtocol->GetRamDiskInfo (
                                        Index,
                                        &RamDiskSize, 
                                        &RamDiskId,
                                        &Removable
                                        );
    if (Status == EFI_SUCCESS) {
      Print (L"%02d  ", Index);
      Print (L"0x%-16lx  ", RamDiskId);
      Print (L"%-8d  ", (RamDiskSize/(1024*1024)));
      if (Removable) {
        Print (L"TRUE");
      } else {
        Print (L"FALSE");
      }
      Print (L"\n");
    }
  }
}

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
RamDiskAppMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                         Status;
  LIST_ENTRY                         *ParamPackage;
  H2O_RAM_DISK_SERVICES_PROTOCOL     *RamDiskServicesProtocol;
  UINT32                             RamDiskSize;
  UINT64                             RamDiskId;
  BOOLEAN                            Removable;
  CONST CHAR16                       *ValueStr;
  CONST CHAR16                       *ValueStrPtr;

  //
  // Register our string package with HII and return the handle to it.
  //
  mHiiHandle = HiiAddPackages (&gEfiCallerIdGuid, ImageHandle, RamDiskAppStrings, NULL);
  ASSERT (mHiiHandle != NULL);

  Status = ShellCommandLineParseEx (RamDiskParamList, &ParamPackage, NULL, TRUE, FALSE);
  if (EFI_ERROR (Status)) {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RAM_DISK_INVALID_INPUT), mHiiHandle);
    goto ON_EXIT;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-?")) {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RAM_DISK_HELP), mHiiHandle);
    goto ON_EXIT;
  }
  
  Status = gBS->LocateProtocol (
                  &gH2ORamDiskServicesProtocolGuid,
                  NULL,
                  (VOID **)&RamDiskServicesProtocol
                  );
  if (EFI_ERROR (Status)) {
    Print (L"RamDiskServicesProtocl not found!!!\n");
    goto ON_EXIT;
  }

  Removable     = FALSE;
  RamDiskSize   = 8;
  RamDiskId     = 0;

  if (ShellCommandLineGetFlag (ParamPackage, L"-i")) {
    DisplayRamDiskInfo (RamDiskServicesProtocol);
    goto ON_EXIT;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-c")) {
    ValueStr = ShellCommandLineGetValue (ParamPackage, L"-s");
    ValueStrPtr = ValueStr;
    if (ValueStr != NULL) {
      RamDiskSize = (UINT32)ShellStrToUintn (ValueStrPtr);
      if (RamDiskSize < 1) {
        Print (L"Minimal size is 1MB, change size to 1MB.\n");
        RamDiskSize = 1;
      }
      if (RamDiskSize > 512) {
        Print (L"Maxmal size is 512MB, change size to 512MB.\n");
        RamDiskSize = 512;
      }
    }
    if (ShellCommandLineGetFlag (ParamPackage, L"-r")) {
      Removable = TRUE;
    }
    Status = RamDiskServicesProtocol->Create (
                                        RamDiskSize,
                                        Removable,
                                        &RamDiskId
                                        );
    if (EFI_ERROR (Status)) {
      Print (L"Status : %r\n", Status);
      goto ON_EXIT;
    } 
    if (Removable) {
      Print (L"Create %d MB removable RAM Disk success.\n", RamDiskSize);
    } else {
      Print (L"Create %d MB RAM Disk success.\n", RamDiskSize);
    }
    Print (L"New RAM Disk ID : 0x%lx\n", RamDiskId);
    goto ON_EXIT;
  }
  
  ValueStr = ShellCommandLineGetValue (ParamPackage, L"-d");
  ValueStrPtr = ValueStr;
  if (ValueStr != NULL) {
    RamDiskId = (UINT64)ShellStrToUintn (ValueStrPtr);
    Status = RamDiskServicesProtocol->Remove (RamDiskId);
    if (EFI_ERROR (Status)) {
      if (Status == EFI_DEVICE_ERROR) {
        Print (L"This RAM Disk is not a removable device.\n");
      } else if (Status == EFI_NOT_FOUND) {
        Print (L"Cannot find specific RAM Disk.\n");
      } else {
        Print (L"Status : %r\n", Status);
      }
      goto ON_EXIT;
    }    
    Print (L"Remove specific RAM Disk success.\n");
    goto ON_EXIT;
  }

ON_EXIT:
  ShellCommandLineFreeVarList (ParamPackage);
  HiiRemovePackages (mHiiHandle);
  
  return Status;
}
