/** @file
  Bds check point library. It provides functionalities to register, unregister
  and trigger check point and also has function to get check point information
  from handle.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/BdsCpLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/DxeServicesLib.h>

#include <Protocol/SmmBase2.h>

#include <Guid/DebugImageInfoTable.h>

#define H2O_BDS_CP_SIGNATURE                    SIGNATURE_32 ('H', 'B', 'C', 'P')

typedef struct {
  UINT32               Signature;
  LIST_ENTRY           Link;
  EFI_GUID             Guid;
  EFI_TPL              Tpl;
  H2O_BDS_CP_HANDLER   Handler;
  EFI_EVENT            Event;
} INTERNAL_BDS_CP_HANDLE;

#define BDS_CP_NODE_FROM_LINK(a)  CR (a, INTERNAL_BDS_CP_HANDLE, Link, H2O_BDS_CP_SIGNATURE)

STATIC LIST_ENTRY      *mCpList;
STATIC EFI_GUID        mInternalBdsCpListHeadGuid = {0xd1d417d5, 0x5264, 0x4369, 0xa3, 0x83, 0xf5, 0xf3, 0xc6, 0x19, 0x49, 0x28};


/**
  Internal function to get EFI_DEBUG_IMAGE_INFO_TABLE_HEADER

  @return Pointer to EFI_DEBUG_IMAGE_INFO_TABLE_HEADER or NULL if cannot find
          EFI_DEBUG_IMAGE_INFO_TABLE_HEADER
**/
STATIC
EFI_DEBUG_IMAGE_INFO_TABLE_HEADER *
GetDebugImageInfoTableHeader (
  VOID
  )
{
  EFI_STATUS                          Status;
  EFI_DEBUG_IMAGE_INFO_TABLE_HEADER   *Header;

  Status = EfiGetSystemConfigurationTable (&gEfiDebugImageInfoTableGuid, (VOID **)&Header);
  return Status == EFI_SUCCESS ? Header : NULL;
}


/**
  Internal function to get image base name from input image.

  @param[in] Image  The pointer to the PE/COFF image that is loaded in system
                    memory.

  @return Image base name ASCII string or NULL if cannot find base name.
**/
STATIC
CHAR8 *
GetBaseNameFromImage (
  IN     VOID    *Image
  )
{
  CHAR8         *PdbPath;
  UINTN         Length;
  CHAR8         *NewStr;
  UINTN         Index;

  PdbPath = PeCoffLoaderGetPdbPointer (Image);
  if (PdbPath == NULL) {
    return NULL;
  }

  NewStr = AllocateCopyPool (AsciiStrSize (PdbPath), PdbPath);
  if (NewStr == NULL) {
    return NULL;
  }
  //
  // Change full path to name path
  // Ex: C:\abc\dsdsa\MyName.pdb to MyName.pdb
  //
  Length = AsciiStrLen (NewStr);
  for (Index = Length; Index > 0; Index--) {
    if (NewStr[Index - 1] == '\\' || NewStr[Index - 1] == '/') {
      CopyMem (NewStr, &NewStr[Index], AsciiStrSize (&NewStr[Index]));
      break;
    }
  }
  //
  // Remove filename extension.
  // Ex: MyName.pdb to MyName
  //
  for (Index = 0; NewStr[Index] != 0; Index++) {
    if (NewStr[Index] == '.') {
      NewStr[Index] = '\0';
      break;
    }
  }
  return NewStr;
}

/**
  Internal function to get image base name from UI section.

  @param[in] LoadedImage   Pointer to EFI_LOADED_IMAGE_PROTOCOL instance.

  @return Image base name ASCII string or NULL if cannot find base name.
**/
STATIC
CHAR8 *
GetBaseNameFromUiSection (
  IN   EFI_LOADED_IMAGE_PROTOCOL    *LoadedImage
  )
{
  CHAR16                                *NameString;
  CHAR8                                 *AsciiNameString;
  UINTN                                 StringSize;
  EFI_STATUS                            Status;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH     *FvFileDevPath;

  if (LoadedImage == NULL || LoadedImage->FilePath == NULL) {
    return NULL;
  }
  if (LoadedImage->FilePath->Type != MEDIA_DEVICE_PATH ||
      LoadedImage->FilePath->SubType != MEDIA_PIWG_FW_FILE_DP) {
    return NULL;
  }

  FvFileDevPath = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) LoadedImage->FilePath;
  Status = GetSectionFromAnyFv (
            &FvFileDevPath->FvFileName,
            EFI_SECTION_USER_INTERFACE,
            0,
            (VOID **) &NameString,
            &StringSize
            );
  if (Status != EFI_SUCCESS) {
    return NULL;
  }

  AsciiNameString = AllocateZeroPool (StrSize (NameString) / sizeof (CHAR16));
  if (AsciiNameString != NULL) {
    UnicodeStrToAsciiStr (NameString, AsciiNameString);
  }
  FreePool (NameString);
  return AsciiNameString;
}

/**
  Internal function to get firmware file name from EFI_LOADED_IMAGE_PROTOCOL instance

  @param[in] LoadedImage   Pointer to EFI_LOADED_IMAGE_PROTOCOL instance.

  @return Firmware file name ASCII string or NULL if cannot find file name.
**/
STATIC
CHAR8 *
GetFvFileName (
  IN   EFI_LOADED_IMAGE_PROTOCOL    *LoadedImage
  )
{
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH     *FvFileDevPath;
  CHAR8                                 *GuidStr;

  if (LoadedImage == NULL || LoadedImage->FilePath == NULL) {
    return NULL;
  }
  if (LoadedImage->FilePath->Type != MEDIA_DEVICE_PATH ||
      LoadedImage->FilePath->SubType != MEDIA_PIWG_FW_FILE_DP) {
    return NULL;
  }
  GuidStr = AllocateZeroPool (0x50);
  if (GuidStr == NULL) {
    return NULL;
  }
  FvFileDevPath = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) LoadedImage->FilePath;
  AsciiSPrint (GuidStr, 0x50, "%g", &FvFileDevPath->FvFileName);
  return GuidStr;
}

/**
  Internal function to useEFI Debug Image Info Table to lookup the FunctionAddress and find
  which PE/COFF image it came from and return image name using ascii string.

  The memory is allocated from EFI boot services memory. It is the responsibility
  of the caller to free the memory allocated.

  @param[in]  FunctionAddress   Callback function address.

  @return Unicoe string for image name or NULL if cannot find image name.
**/
STATIC
CHAR8 *
GetImageName (
  IN  UINTN  FunctionAddress
  )
{
  EFI_DEBUG_IMAGE_INFO_TABLE_HEADER    *TableHeader;
  EFI_DEBUG_IMAGE_INFO                 *DebugTable;
  UINTN                                Index;
  UINTN                                ImageBase;
  UINTN                                ImageSize;
  CHAR8                                *BaseName;

  TableHeader = GetDebugImageInfoTableHeader ();
  if (TableHeader == NULL) {
    return NULL;
  }

  DebugTable = TableHeader->EfiDebugImageInfoTable;
  for (Index = 0; Index < TableHeader->TableSize; Index++) {
    if (DebugTable[Index].NormalImage == NULL ||
        DebugTable[Index].NormalImage->ImageInfoType != EFI_DEBUG_IMAGE_INFO_TYPE_NORMAL ||
        DebugTable[Index].NormalImage->LoadedImageProtocolInstance == NULL) {
      continue;
    }
    ImageBase = (UINTN) DebugTable[Index].NormalImage->LoadedImageProtocolInstance->ImageBase;
    ImageSize = (UINTN) DebugTable[Index].NormalImage->LoadedImageProtocolInstance->ImageSize;
    if (FunctionAddress >= ImageBase && FunctionAddress < ImageBase + ImageSize) {
      BaseName = GetBaseNameFromImage ((VOID *) ImageBase);
      if (BaseName != NULL) {
        return BaseName;
      }
      BaseName = GetBaseNameFromUiSection (DebugTable[Index].NormalImage->LoadedImageProtocolInstance);
      if (BaseName != NULL) {
        return BaseName;
      }
      return GetFvFileName (DebugTable[Index].NormalImage->LoadedImageProtocolInstance);
    }
  }
  return NULL;
}

/**
  Print callback functions which registered to input checkpoint information. The information
  includes function pointer address, callback function priority, Image name for this Callback
  function.

  @param[in] BdsCheckpoint      Pointer to the GUID associated with the BDS checkpoint.
**/
STATIC
VOID
DumpCallbackFunctionInfo (
  IN CONST EFI_GUID *BdsCheckpoint
  )
{
  LIST_ENTRY              *List;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;
  EFI_TPL                 Tpl;
  CHAR8                   *ModuleName;

  DEBUG((DEBUG_INFO, "BDS checkpoint signaled: %g ...\n", BdsCheckpoint));
  if (IsListEmpty (mCpList)) {
    return;
  }

  for (Tpl = TPL_HIGH_LEVEL; Tpl > TPL_APPLICATION; Tpl--) {
    List = GetNextNode (mCpList, mCpList);
    while (List != mCpList) {
      CpHandle = BDS_CP_NODE_FROM_LINK (List);
      if (Tpl == CpHandle->Tpl && CompareGuid (BdsCheckpoint, &CpHandle->Guid)) {
        ModuleName = GetImageName ((UINTN) CpHandle->Handler);
        DEBUG((DEBUG_INFO, "Module name: %a, Priority: 0x%02x, Functoin Address: 0x%p.\n",
                           ModuleName == NULL ? "Unknown" : ModuleName, Tpl, (VOID *) (UINTN) CpHandle->Handler));
        if (ModuleName != NULL) {
          FreePool (ModuleName);
        }
      }
      List = GetNextNode (mCpList, List);
    }
  }
}

/**
  This function registers a handler for the specified checkpoint with the specified priority.

  @param[in]  BdsCheckpoint     Pointer to a GUID that specifies the checkpoint for which the
                                handler is being registered.
  @param[in]  Handler           Pointer to the handler function.
  @param[in]  Priority          Enumerated value that specifies the priority with which the function
                                will be associated.
  @param[out] Handle            Pointer to the returned handle that is associated with the newly
                                registered checkpoint handler.

  @retval EFI_SUCCESS           Register check point handle successfully.
  @retval EFI_INVALID_PARAMETER BdsCheckpoint ,Handler or Handle is NULL.
  @retval EFI_OUT_OF_RESOURCES  Allocate memory for Handle failed.
**/
EFI_STATUS
BdsCpRegisterHandler (
  IN  CONST EFI_GUID      *BdsCheckpoint,
  IN  H2O_BDS_CP_HANDLER  Handler,
  IN  H2O_BDS_CP_PRIORITY Priority,
  OUT H2O_BDS_CP_HANDLE   *Handle
  )
{
  EFI_EVENT               Event;
  EFI_STATUS              Status;
  VOID                    *Registration;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;

  if (BdsCheckpoint == NULL || Handler == NULL || Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CpHandle = AllocateZeroPool (sizeof (INTERNAL_BDS_CP_HANDLE));
  if (CpHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  Priority,
                  Handler,
                  CpHandle,
                  &Event
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->RegisterProtocolNotify (
                  (EFI_GUID *) BdsCheckpoint,
                  Event,
                  &Registration
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CpHandle->Signature = H2O_BDS_CP_SIGNATURE;
  CopyGuid (&CpHandle->Guid, BdsCheckpoint);
  CpHandle->Event     = Event;
  CpHandle->Tpl       = Priority;
  CpHandle->Handler   = Handler;
  InsertTailList (mCpList, &CpHandle->Link);
  *Handle = (VOID *) CpHandle;
  return EFI_SUCCESS;
}

/**
  This function returns the checkpoint data structure that was installed when the checkpoint was
  triggered and, optionally, the GUID that was associated with the checkpoint.

  @param[in]  Handle            The handle associated with a previously registered checkpoint
                                handler.
  @param[out] BdsCheckpointData The pointer to the checkpoint structure that was installed.
  @param[out] BdsCheckpoint     Optional pointer to the returned pointer to the checkpoint GUID.

  @retval EFI_SUCCESS           Get check point data successfully.
  @retval EFI_INVALID_PARAMETER Handle or BdsCheckpointData is NULL or Handle is invalid.
  @retval EFI_INVALID_PARAMETER It does not refer to a previously registered checkpoint handler.
  @return Others                Other error occurred while getting check point information.
**/
EFI_STATUS
BdsCpLookup (
  IN  H2O_BDS_CP_HANDLE Handle,
  OUT VOID              **BdsCheckpointData,
  OUT EFI_GUID          *BdsCheckpoint       OPTIONAL
  )
{
  LIST_ENTRY              *List;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;
  INTERNAL_BDS_CP_HANDLE  *CurrentCpHandle;
  EFI_STATUS              Status;
  UINTN                   BufferSize;

  if (Handle == NULL || BdsCheckpointData == NULL || IsListEmpty (mCpList)) {
    return EFI_INVALID_PARAMETER;
  }

  CpHandle = (INTERNAL_BDS_CP_HANDLE *) Handle;
  if (CpHandle->Signature != H2O_BDS_CP_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  List = GetNextNode (mCpList, mCpList);
  while (List != mCpList) {
    CurrentCpHandle = BDS_CP_NODE_FROM_LINK (List);
    if (CpHandle->Event == CurrentCpHandle->Event) {
      //
      // Should not have multiple protocol instances,
      //
      BufferSize = 0;
      Status = gBS->LocateHandle (
                      ByProtocol,
                      &CpHandle->Guid,
                      NULL,
                      &BufferSize,
                      NULL
                      );
      if (Status != EFI_BUFFER_TOO_SMALL || BufferSize / sizeof (EFI_HANDLE) != 1) {
        return EFI_INVALID_PARAMETER;
      }

      Status = gBS->LocateProtocol (
                      &CpHandle->Guid,
                      NULL,
                      BdsCheckpointData
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }
      if (BdsCheckpoint != NULL) {
        CopyGuid (BdsCheckpoint, &CpHandle->Guid);
      }
      return EFI_SUCCESS;
    }
    List = GetNextNode (mCpList, List);
  }

  return EFI_INVALID_PARAMETER;
}

/**
  This function install checks point data to check point GUID and triggers check point
  according to check point GUID.

  @param[in] BdsCheckpoint      Pointer to the GUID associated with the BDS checkpoint.
  @param[in] BdsCheckpointData  Pointer to the data associated with the BDS checkpoint.


  @retval EFI_SUCCESS           Trigger check point successfully.
  @retval EFI_INVALID_PARAMETER BdsCheckpoint or BdsCheckpointData is NULL.
  @retval Other                 Install BdsCheckpoint protocol failed.
**/
EFI_STATUS
BdsCpTrigger (
  IN CONST EFI_GUID *BdsCheckpoint,
  IN CONST VOID     *BdsCheckpointData
  )
{
  EFI_HANDLE          Handle;

  if (BdsCheckpoint == NULL || BdsCheckpointData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG_CODE (DumpCallbackFunctionInfo (BdsCheckpoint););
  Handle = NULL;
  return gBS->InstallProtocolInterface (
                  &Handle,
                  (EFI_GUID *) BdsCheckpoint,
                  EFI_NATIVE_INTERFACE,
                  (VOID *) BdsCheckpointData
                  );
}

/**
  This function unregisters the handle and frees any associated resources.

  @param[in] Handle             The handle that is associated with the registered checkpoint handler.

  @retval EFI_SUCCESS           The function completed successfully.
  @return EFI_INVALID_PARAMETER Handle is NULL, Handle is invalid or does not refer to a previously
                                registered checkpoint handler.
**/
EFI_STATUS
BdsCpUnregisterHandler (
  IN H2O_BDS_CP_HANDLE Handle
  )
{
  LIST_ENTRY              *List;
  INTERNAL_BDS_CP_HANDLE  *CpHandle;
  INTERNAL_BDS_CP_HANDLE  *CurrentCpHandle;

  if (Handle == NULL || IsListEmpty (mCpList)) {
    return EFI_INVALID_PARAMETER;
  }

  CpHandle = (INTERNAL_BDS_CP_HANDLE *) Handle;
  if (CpHandle->Signature != H2O_BDS_CP_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  List = GetNextNode (mCpList, mCpList);
  while (List != mCpList) {
    CurrentCpHandle = BDS_CP_NODE_FROM_LINK (List);
    if (CpHandle->Event == CurrentCpHandle->Event) {
      gBS->CloseEvent (CpHandle->Event);
      RemoveEntryList (List);
      FreePool (Handle);
      return EFI_SUCCESS;
    }
    List = GetNextNode (mCpList, List);
  }

  return EFI_INVALID_PARAMETER;
}

/**
  Detect whether the system is in SMM mode.

  @retval TRUE                  System is at SMM mode.
  @retval FALSE                 System is not at SMM mode.
**/
STATIC
BOOLEAN
IsInSmm (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_SMM_BASE2_PROTOCOL         *SmmBase;
  BOOLEAN                        InSmm;

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **) &SmmBase
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  InSmm = FALSE;
  SmmBase->InSmm (SmmBase, &InSmm);
  return InSmm;
}

/**
  The constructor function saves the address of check point address in volatile variable.
  And then all of library instances can access the same database.

  It will ASSERT() if that operation fails and it will always return EFI_SUCCESS.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   This function completes  successfully.
  @return Others        Any error occurred while saving or getting check point address from
                        volatile variable.
**/
EFI_STATUS
EFIAPI
BdsCpLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS     Status;
  EFI_HANDLE     Handle;

  if (IsInSmm ()) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (
                  &mInternalBdsCpListHeadGuid,
                  NULL,
                  (VOID **) &mCpList
                  );
  if (EFI_ERROR (Status)) {
    mCpList = AllocatePool (sizeof (LIST_ENTRY));
    ASSERT (mCpList != NULL);
    if (mCpList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    mCpList->ForwardLink = mCpList;
    mCpList->BackLink    = mCpList;
    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &mInternalBdsCpListHeadGuid,
                    EFI_NATIVE_INTERFACE,
                    (VOID *) mCpList
                    );
  }
  return Status;
}
