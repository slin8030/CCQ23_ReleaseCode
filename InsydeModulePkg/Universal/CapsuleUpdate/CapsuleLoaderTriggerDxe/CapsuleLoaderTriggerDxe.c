/** @file
  Driver to trigger CapsuleUpdate Flash Loader and Firmware Update

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

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/DevicePathLib.h>
#include <Library/VariableLib.h>
#include <Library/CapsuleLib.h>
#include <Library/CapsuleUpdateCriteriaLib.h>
#include <Library/SeamlessRecoveryLib.h>
#include <Library/BdsCpLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>
#include <Guid/FileInfo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/EfiSystemResourceTable.h>
#include <Guid/H2OBdsCheckPoint.h>
#include <Guid/DebugMask.h>
#include <StdLib/stdlib.h>
#include <SecureFlash.h>

#define MAX_STRING_LENGTH       128

/**
  Get the system drive information through the boot loader file path under system partition

  @param[out] SysRootDevicePath    The pointer of system root device path pointer
  @param[out] SysRoorDir           The pointer of EFI_FILE_HANDLE of system drive

  @retval EFI_SUCCESS              The system drive information is correctly get
  @return others                   Unable to get system drive information

**/
EFI_STATUS
EFIAPI
GetSystemRootInfo (
  OUT    EFI_DEVICE_PATH        **SysRootDevicePath,
  OUT    EFI_FILE_HANDLE        *SysRootHandle
  );

/**
  File name comparsion for qsort

  @param[in] FileInfo1          Pointer of first file info for comparsion
  @param[in] FileInfo2          Pointer of second file info for comparsion

**/
STATIC
int
FileNameComparsion (
  IN     CONST VOID*    FileInfo1,
  IN     CONST VOID*    FileInfo2
  )
{
  EFI_FILE_INFO *FileInfoPtr1 = *(EFI_FILE_INFO**)FileInfo1;
  EFI_FILE_INFO *FileInfoPtr2 = *(EFI_FILE_INFO**)FileInfo2;

  return ((int)StrCmp (FileInfoPtr1->FileName, FileInfoPtr2->FileName));
}

/**
  ReadyToBoot notification event handler for capsule image update.

  @param[in] Event              Event whose notification function is being invoked.
  @param[in] Context            Pointer to the notification function's context.

  @retval    None
**/
VOID
EFIAPI
TriggerCapsuleUpdate (
  IN     EFI_EVENT      Event,
  IN     VOID           *Context
  )
{
  EFI_STATUS                Status;
  VOID                      *Interface;
  EFI_DEVICE_PATH_PROTOCOL  *SysRootDevicePath;
  EFI_FILE_HANDLE           SysRootHandle;
  EFI_FILE_HANDLE           CapsuleHandle;
  UINTN                     Size;
  UINT64                    OsIndications;
  UINTN                     Index;
  UINTN                     FileSize;
  UINTN                     FileInfoSize;
  UINTN                     TotalCapsules;
  EFI_CAPSULE_HEADER        *Capsule;
  UINT8                     *FileInfo;
  UINT8                     *FileInfoPtr;
  EFI_FILE_INFO             **CapsuleFileInfo;
  CHAR16                    CapsuleImagePath[MAX_STRING_LENGTH];
  BOOLEAN                   ResetNeeded;

  SysRootHandle     = NULL;
  CapsuleHandle     = NULL;
  FileInfo          = NULL;
  CapsuleFileInfo   = NULL;
  ResetNeeded       = FALSE;
  //
  // Double check the interface installation
  //
  Status = gBS->LocateProtocol (
                  &gH2OBdsCpReadyToBootBeforeProtocolGuid,
                  NULL,
                  (VOID**)&Interface
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  gBS->CloseEvent(Event);
  //
  // Enumerate whole capsule files under
  //
  Status = GetSystemRootInfo (&SysRootDevicePath, &SysRootHandle);
  if (EFI_ERROR (Status) || SysRootHandle == NULL) {
    return;
  }
  Status = SysRootHandle->Open (
                            SysRootHandle,
                            &CapsuleHandle,
                            EFI_CAPSULE_FILE_PATH,
                            EFI_FILE_MODE_READ,
                            EFI_FILE_DIRECTORY
                            );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }
  TotalCapsules = 0;
  FileInfoSize  = 0;
  Status = CapsuleHandle->GetInfo (
                            CapsuleHandle,
                            &gEfiFileInfoGuid,
                            &FileInfoSize,
                            NULL
                            );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // error is expected.  getting size to allocate
    //
    FileInfo = AllocatePool (FileInfoSize);
    if (!FileInfo) {
      goto Exit;
    }
    Status = CapsuleHandle->GetInfo (
                              CapsuleHandle,
                              &gEfiFileInfoGuid,
                              &FileInfoSize,
                              FileInfo
                              );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    Size = (UINTN)((EFI_FILE_INFO*)FileInfo)->FileSize;
    FreePool (FileInfo);
    if (Size == 0) {
      goto Exit;
    }
    FileInfo = AllocatePool (Size);
    if (!FileInfo) {
      goto Exit;
    }
    TotalCapsules = 0;
    FileInfoPtr   = FileInfo;
    do {
      FileInfoSize = Size;
      Status = CapsuleHandle->Read (
                                CapsuleHandle,
                                &FileInfoSize,
                                FileInfoPtr
                                );
      if (!EFI_ERROR (Status) && FileInfoSize > 0) {
        if (!(((EFI_FILE_INFO*)FileInfoPtr)->Attribute & EFI_FILE_DIRECTORY) && ((EFI_FILE_INFO*)FileInfoPtr)->FileSize > 0) {
          if (CapsuleFileInfo == NULL) {
            CapsuleFileInfo = AllocatePool (sizeof (EFI_FILE_INFO*) * (UINTN)DivU64x32((UINT64)Size, SIZE_OF_EFI_FILE_INFO));
            if (CapsuleFileInfo == NULL) {
              goto Exit;
            }
          }
          CapsuleFileInfo[TotalCapsules ++] = (EFI_FILE_INFO*)FileInfoPtr;
          FileInfoPtr += FileInfoSize;
        }
      }
    } while (FileInfoSize != 0);
    CapsuleHandle->Close (CapsuleHandle);
    CapsuleHandle = NULL;
  }
  if (!FileInfo) {
    goto Exit;
  }
  //
  // Count total capsule images according to file info record
  //
  if (TotalCapsules == 0) {
    //
    // No any capsule file in the EFI_CAPSULE_FILE_PATH directory
    //
    goto Exit;
  }
  //
  // Sort the capsule file list
  //
  qsort (CapsuleFileInfo, TotalCapsules, sizeof (EFI_FILE_INFO*), FileNameComparsion);
  //
  // Erase the EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED bit in
  // OsIndications variable before flash no matter the flash is successful or not
  //
  Size          = sizeof(UINT64);
  OsIndications = 0;
  Status = CommonGetVariable (
             L"OsIndications",
             &gEfiGlobalVariableGuid,
             &Size,
             &OsIndications
             );
  OsIndications &= ~(UINTN)EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED;
  Status = CommonSetVariable (
             L"OsIndications",
             &gEfiGlobalVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             sizeof(UINT64),
             &OsIndications
             );
  //
  // Loop for cpasule update processing
  //
  for (Index = 0, CapsuleHandle = NULL; Index < TotalCapsules; Index ++) {
    UnicodeSPrint (CapsuleImagePath, MAX_STRING_LENGTH, L"%s\\%s", EFI_CAPSULE_FILE_PATH, CapsuleFileInfo[Index]->FileName);
    Status = SysRootHandle->Open (
                              SysRootHandle,
                              &CapsuleHandle,
                              CapsuleImagePath,
                              EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
                              EFI_FILE_ARCHIVE
                              );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    FileSize = (UINTN)CapsuleFileInfo[Index]->FileSize;
    Capsule = AllocatePool (FileSize);
    if (Capsule == NULL) {
      goto Exit;
    }
    Status = CapsuleHandle->Read (
                              CapsuleHandle,
                              &FileSize,
                              (VOID*)Capsule
                              );
    if (!EFI_ERROR (Status)) {
      //
      // Clear the Flags in capsule header as no flags allowed in file capsule according to UEFI 2.4B spec 7.5.5
      //
      Capsule->Flags = 0;
      //
      // Process capsule image
      //
      Status = ProcessCapsuleImage (Capsule);
      //
      // Delete the file after process capsule
      //
      CapsuleHandle->Delete (CapsuleHandle);
      //
      // Always reset system whether process capsule success or not
      //
      ResetNeeded = TRUE;
    } else {
      //
      // Close the file when read failed
      //
      CapsuleHandle->Close (CapsuleHandle);
    }
    FreePool (Capsule);
    CapsuleHandle = NULL;
  }
Exit:
  if (FileInfo) {
    FreePool (FileInfo);
  }
  if (CapsuleFileInfo) {
    FreePool (CapsuleFileInfo);
  }
  if (SysRootHandle) {
    SysRootHandle->Close (SysRootHandle);
  }
  if (CapsuleHandle) {
    CapsuleHandle->Close (CapsuleHandle);
  }
  if (ResetNeeded) {
    gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  }
}

/**
  CapsuleLoaderTrigger DXE driver entry point


  @param ImageHandle     A handle for the image that is initializing this driver
  @param SystemTable     A pointer to the EFI system table

  @retval EFI_SUCCESS:   Module initialized successfully
**/
EFI_STATUS
EFIAPI
CapsuleLoaderTriggerEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS           Status;
  UINTN                Size;
  UINT64               OsIndications;
  H2O_BDS_CP_HANDLE    CpHandle;
  //
  // Inspect the OsIndications variable to startup file capsule process
  //
  Size          = sizeof(UINT64);
  OsIndications = 0;
  Status = CommonGetVariable (
             L"OsIndications",
             &gEfiGlobalVariableGuid,
             &Size,
             &OsIndications
             );
  if (!EFI_ERROR (Status) && (OsIndications & EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED)) {
    //
    // Setup the capsule update procedure right before ready to boot event
    // Ues of TPL_CALLBACK-1 to make the USB hot-plug can work properly as it
    // is using TPL_CALLBACK level
    //
    BdsCpRegisterHandler (
      &gH2OBdsCpReadyToBootBeforeProtocolGuid,
      TriggerCapsuleUpdate,
      TPL_CALLBACK - 1,
      &CpHandle
      );
  }
  return EFI_SUCCESS;
}