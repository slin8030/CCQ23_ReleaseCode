/** @file
For H2O Setup Link protocol

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "InternalH2OFormBrowser.h"

extern H2O_FORM_BROWSER_PRIVATE_DATA mFBPrivate;

STATIC
BOOLEAN
ChkHasFormSet (
  IN CONST EFI_HII_HANDLE         HiiHandle,
  IN CONST EFI_GUID               *FormsetGuid
  )
{
  EFI_STATUS                      Status;
  EFI_HII_DATABASE_PROTOCOL       *HiiDatabase;
  UINTN                           BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER     *HiiPackageList;
  UINT8                           *Package;
  UINT8                           *OpCodeData;
  UINT32                          Offset;
  UINT32                          PackageOffset;
  UINT32                          PackageListLength;
  EFI_HII_PACKAGE_HEADER          PackageHeader;

  if (HiiHandle == NULL || FormsetGuid == NULL) {
    return FALSE;
  }

  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // Get HII package list
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return FALSE;
  }
  HiiPackageList = AllocatePool (BufferSize);
  if (HiiPackageList == NULL) {
    return FALSE;
  }
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (EFI_ERROR (Status)) {
    FreePool (HiiPackageList);
    return FALSE;
  }

  //
  // Get Form package and check form set class GUID in this HII package list
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);
  while (Offset < PackageListLength) {
    Package = (UINT8 *) HiiPackageList + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      PackageOffset = sizeof (EFI_HII_PACKAGE_HEADER);
      while (PackageOffset < PackageHeader.Length) {
        OpCodeData = Package + PackageOffset;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP &&
            CompareGuid ((EFI_GUID *) (VOID *) (&((EFI_IFR_FORM_SET *) OpCodeData)->Guid), FormsetGuid)) {
          FreePool (HiiPackageList);
          return TRUE;
        }
        PackageOffset += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
      }
    }
    Offset += PackageHeader.Length;
  }

  FreePool (HiiPackageList);

  return FALSE;
}

STATIC
BOOLEAN
ChkHasClassGuid (
  IN CONST EFI_HII_HANDLE         HiiHandle,
  IN CONST EFI_GUID               *ClassGuid,
  OUT EFI_GUID                    *FormsetGuid
  )
{
  EFI_STATUS                      Status;
  EFI_HII_DATABASE_PROTOCOL       *HiiDatabase;
  UINTN                           BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER     *HiiPackageList;
  UINT8                           *Package;
  UINT8                           *OpCodeData;
  UINT32                          Offset;
  UINT32                          PackageOffset;
  UINT32                          PackageListLength;
  EFI_HII_PACKAGE_HEADER          PackageHeader;
  EFI_IFR_FORM_SET                *FormSet;
  UINT32                          Index;
  UINT32                          ClassGuidCount;
  EFI_GUID                        *ClassGuidList;

  if (HiiHandle == NULL || ClassGuid == NULL || FormsetGuid == NULL) {
    return FALSE;
  }

  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // Get HII package list
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return FALSE;
  }
  HiiPackageList = AllocatePool (BufferSize);
  if (HiiPackageList == NULL) {
    return FALSE;
  }
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (EFI_ERROR (Status)) {
    FreePool (HiiPackageList);
    return FALSE;
  }

  //
  // Get Form package and check form set class GUID in this HII package list
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);
  while (Offset < PackageListLength) {
    Package = (UINT8 *) HiiPackageList + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      PackageOffset = sizeof (EFI_HII_PACKAGE_HEADER);
      while (PackageOffset < PackageHeader.Length) {
        OpCodeData = Package + PackageOffset;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          FormSet = (EFI_IFR_FORM_SET *) OpCodeData;
          ClassGuidCount = (FormSet->Header.Length - sizeof (EFI_IFR_FORM_SET)) / sizeof (EFI_GUID);
          ClassGuidList = (EFI_GUID *)(FormSet + 1);
          for (Index = 0; Index < ClassGuidCount; Index ++) {
            if (CompareGuid (&ClassGuidList[Index], ClassGuid)) {
              CopyGuid (FormsetGuid, (EFI_GUID *)(VOID *)&FormSet->Guid);
              FreePool (HiiPackageList);
              return TRUE;
            }
          }
        }
        PackageOffset += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
      }
    }
    Offset += PackageHeader.Length;
  }

  FreePool (HiiPackageList);

  return FALSE;
}

STATIC
EFI_STATUS
GetDevicePathStrByHiiHandle (
  IN CONST EFI_HII_HANDLE           HiiHandle,
  OUT    CHAR16                     **DevicePathStr
  )
{
  EFI_STATUS                        Status;
  EFI_HII_DATABASE_PROTOCOL         *HiiDatabase;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL  *DevicePathToText;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_HANDLE                        DriverHandle;

  if (HiiHandle == NULL || DevicePathStr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HiiDatabase->GetPackageListHandle (HiiDatabase, HiiHandle, &DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->HandleProtocol (DriverHandle, &gEfiDevicePathProtocolGuid, (VOID**) &DevicePath);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gEfiDevicePathToTextProtocolGuid, NULL, (VOID **) &DevicePathToText);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *DevicePathStr = DevicePathToText->ConvertDevicePathToText (DevicePath, FALSE, TRUE);
  if (*DevicePathStr == NULL) {
    EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetFirstFormId (
  IN CONST EFI_HII_HANDLE           HiiHandle,
  IN CONST EFI_GUID                 *FormSetGuid,
  OUT    EFI_FORM_ID                *FirstFormId
  )
{
  EFI_HII_DATABASE_PROTOCOL         *HiiDatabase;
  EFI_STATUS                        Status;
  UINTN                             BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER       *HiiPackageList;
  UINT8                             *Package;
  UINT8                             *OpCodeData;
  UINT32                            Offset;
  UINT32                            Offset2;
  UINT32                            PackageListLength;
  EFI_HII_PACKAGE_HEADER            PackageHeader;
  BOOLEAN                           Found;
  EFI_IFR_FORM_SET                  *FormSet;
  EFI_GUID                          ZeroGuid;

  if (HiiHandle == NULL || FormSetGuid == NULL || FirstFormId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get HII PackageList
  //
  BufferSize     = 0;
  HiiPackageList = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return EFI_NOT_FOUND;
  }

  HiiPackageList = AllocatePool (BufferSize);
  if (HiiPackageList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get Form package from this HII package List
  //
  Offset  = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);
  if (PackageListLength > BufferSize) {
    PackageListLength = (UINT32)BufferSize;
  }

  ZeroMem (&ZeroGuid, sizeof(EFI_GUID));
  FormSet = NULL;
  Found   = FALSE;
  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet Opcode in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          FormSet = (EFI_IFR_FORM_SET *) OpCodeData;

          if (!CompareGuid (FormSetGuid, &ZeroGuid) &&
            !CompareGuid (FormSetGuid, (EFI_GUID *)(VOID *)&FormSet->Guid)) {
            FormSet = NULL;
          }
        }

        if (FormSet != NULL && (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_OP)) {
          *FirstFormId = ((EFI_IFR_FORM *) OpCodeData)->FormId;
          Found = TRUE;
          break;
        }

        //
        // Go to next opcode
        //
        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
        if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length < sizeof (EFI_IFR_OP_HEADER)) {
          break;
        }
      }
    }

    if (Found) {
      break;
    }

    //
    // Go to next package
    //
    Offset += PackageHeader.Length;
    if (PackageHeader.Length == 0) {
      break;
    }
  }

  FreePool (HiiPackageList);

  return Found ? EFI_SUCCESS : EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetAllFirstFormId (
  IN CONST UINT32                     FormSetCount,
  IN CONST EFI_HII_HANDLE             *HiiHandleList,
  IN CONST EFI_GUID                   *FormSetGuidList,
  OUT EFI_FORM_ID                     **FirstFormIdList
  )
{
  EFI_STATUS                          Status;
  UINT32                              Index;
  EFI_FORM_ID                         *ResultFormIdList;

  if (FormSetCount == 0 || HiiHandleList == NULL || FormSetGuidList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ResultFormIdList = AllocateZeroPool (sizeof (EFI_FORM_ID) * FormSetCount);
  if (ResultFormIdList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < FormSetCount; Index ++) {
    Status = GetFirstFormId (
               HiiHandleList[Index],
               &FormSetGuidList[Index],
               &ResultFormIdList[Index]
               );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  *FirstFormIdList = ResultFormIdList;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetFormSetInfo (
  IN CONST EFI_HII_HANDLE           HiiHandle,
  IN OUT EFI_GUID                   *FormSetGuid,
  OUT    EFI_STRING_ID              *FormSetTitleStrId,
  OUT    EFI_STRING_ID              *FormSetHelpStrId,
  OUT    EFI_FORM_ID                *RootFormId
  )
{
  EFI_HII_DATABASE_PROTOCOL         *HiiDatabase;
  EFI_STATUS                        Status;
  UINTN                             BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER       *HiiPackageList;
  UINT8                             *Package;
  UINT8                             *OpCodeData;
  UINT32                            Offset;
  UINT32                            Offset2;
  UINT32                            PackageListLength;
  EFI_HII_PACKAGE_HEADER            PackageHeader;
  BOOLEAN                           Found;
  EFI_IFR_FORM_SET                  *FormSet;
  EFI_GUID                          ZeroGuid;

  if (HiiHandle == NULL || FormSetGuid == NULL || FormSetTitleStrId == NULL || FormSetHelpStrId == NULL || RootFormId == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get HII PackageList
  //
  BufferSize     = 0;
  HiiPackageList = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return EFI_NOT_FOUND;
  }

  HiiPackageList = AllocatePool (BufferSize);
  if (HiiPackageList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get Form package from this HII package List
  //
  Offset  = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);
  if (PackageListLength > BufferSize) {
    PackageListLength = (UINT32)BufferSize;
  }

  ZeroMem (&ZeroGuid, sizeof(EFI_GUID));
  FormSet = NULL;
  Found   = FALSE;
  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORMS) {
      //
      // Search FormSet Opcode in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          FormSet = (EFI_IFR_FORM_SET *) OpCodeData;

          if (CompareGuid (FormSetGuid, &ZeroGuid) || CompareGuid (FormSetGuid, (EFI_GUID *)(VOID *)&FormSet->Guid)) {
            CopyGuid (FormSetGuid, (EFI_GUID *)(VOID *)&FormSet->Guid);
            *FormSetTitleStrId = FormSet->FormSetTitle;
            *FormSetHelpStrId  = FormSet->Help;
          } else {
            FormSet = NULL;
          }
        }

        if (FormSet != NULL && (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_OP)) {
          *RootFormId = ((EFI_IFR_FORM *) OpCodeData)->FormId;
          Found = TRUE;
          break;
        }

        //
        // Go to next opcode
        //
        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
        if (((EFI_IFR_OP_HEADER *) OpCodeData)->Length < sizeof (EFI_IFR_OP_HEADER)) {
          break;
        }
      }
    }

    if (Found) {
      break;
    }

    //
    // Go to next package
    //
    Offset += PackageHeader.Length;
    if (PackageHeader.Length == 0) {
      break;
    }
  }

  FreePool (HiiPackageList);

  return Found ? EFI_SUCCESS : EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetHiiHandleByFormSetGuid (
  IN CONST EFI_GUID               *FormsetGuid,
  OUT UINT32                      *FormsetCount,
  OUT EFI_HII_HANDLE              *HiiHandleList,
  OUT EFI_GUID                    *FormsetGuidList
  )
{
  EFI_HII_HANDLE                  *HiiHandles;
  UINTN                           Index;

  if (FormsetGuid == NULL || FormsetCount == NULL || HiiHandleList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiHandles = HiiGetHiiHandles (NULL);
  if (HiiHandles == NULL) {
    return EFI_NOT_FOUND;
  }

  (*FormsetCount) = 0;
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    if (ChkHasFormSet (HiiHandles[Index], FormsetGuid)) {
      HiiHandleList[(*FormsetCount)] = HiiHandles[Index];
      CopyGuid (&FormsetGuidList[(*FormsetCount)], FormsetGuid);
      (*FormsetCount)++;
    }
  }

  FreePool (HiiHandles);

  if ((*FormsetCount) != 0) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetFormsetsByClassGuid (
  IN CONST EFI_GUID               *ClassGuid,
  OUT UINT32                      *FormsetCount,
  OUT EFI_HII_HANDLE              *HiiHandleList,
  OUT EFI_GUID                    *FormsetGuidList
  )
{
  EFI_HII_HANDLE                  *HiiHandles;
  UINT32                          Index;

  if (ClassGuid == NULL || FormsetCount == NULL || HiiHandleList == NULL || FormsetGuidList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get all HiiHandles in current hii database
  //
  HiiHandles = HiiGetHiiHandles (NULL);
  if (HiiHandles == NULL) {
    return EFI_NOT_FOUND;
  }

  (*FormsetCount) = 0;
  for (Index = 0; HiiHandles[Index] != NULL; Index++) {
    //
    // Get HiiFormPkg by HiiHandle, and check this ClassGuid in its formset
    // If true, return FormsetGuid
    //
    if (ChkHasClassGuid (HiiHandles[Index], ClassGuid, &FormsetGuidList[(*FormsetCount)])) {
      HiiHandleList[(*FormsetCount)] = HiiHandles[Index];
      (*FormsetCount) ++;
    }
  }

  FreePool (HiiHandles);

  if ((*FormsetCount) != 0) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetFormsetsByLink (
  IN CONST EFI_GUID                   *FormsetGuid,
  IN CONST UINT32                     IncludeCount,
  IN CONST UINT32                     ExcludeCount,
  IN CONST EFI_GUID                   *IncludeList,
  IN CONST EFI_GUID                   *ExcludeList,
  OUT UINT32                          *FormsetCount,
  OUT EFI_HII_HANDLE                  **HiiHandleList,
  OUT EFI_GUID                        **FormsetGuidList
  )
{
  EFI_STATUS                          Status;
  UINT32                              IncludeIndex;
  UINT32                              ExcludeIndex;
  UINT32                              Count;
  UINT32                              IncludeFormsetCount;
  EFI_HII_HANDLE                      *IncludeHiiHandleList;
  EFI_GUID                            *IncludeFormsetGuidList;
  UINT32                              ExcludeFormsetCount;
  EFI_HII_HANDLE                      *ExcludeHiiHandleList;
  EFI_GUID                            *ExcludeFormsetGuidList;
  UINT32                              ResultFormsetCount;
  EFI_HII_HANDLE                      *ResultHiiHandleList;
  EFI_GUID                            *ResultFormsetGuidList;
  BOOLEAN                             Skip;

  IncludeHiiHandleList   = (EFI_HII_HANDLE *)AllocateZeroPool (sizeof (EFI_HII_HANDLE) * 100);
  IncludeFormsetGuidList = (EFI_GUID *)AllocateZeroPool (sizeof (EFI_GUID) * 100);
  ExcludeHiiHandleList   = (EFI_HII_HANDLE *)AllocateZeroPool (sizeof (EFI_HII_HANDLE) * 100);
  ExcludeFormsetGuidList = (EFI_GUID *)AllocateZeroPool (sizeof (EFI_GUID) * 100);
  ResultHiiHandleList    = (EFI_HII_HANDLE *)AllocateZeroPool (sizeof (EFI_HII_HANDLE) * 100);
  ResultFormsetGuidList  = (EFI_GUID *)AllocateZeroPool (sizeof (EFI_GUID) * 100);
  if (IncludeHiiHandleList == NULL || IncludeFormsetGuidList == NULL ||
    ExcludeHiiHandleList == NULL || ExcludeFormsetGuidList == NULL ||
    ResultHiiHandleList == NULL || ResultFormsetGuidList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get Include Formsets
  //
  IncludeFormsetCount = 0;
  for (IncludeIndex = 0; IncludeIndex < IncludeCount; IncludeIndex ++) {

    if (!CompareGuid (&IncludeList[IncludeIndex], FormsetGuid)) {
      //
      // Formset Guid
      //
      Count = 0;
      Status = GetHiiHandleByFormSetGuid (
                 &IncludeList[IncludeIndex],
                 &Count,
                 &IncludeHiiHandleList[IncludeFormsetCount],
                 &IncludeFormsetGuidList[IncludeFormsetCount]
                 );
      if (Status == EFI_SUCCESS) {
        IncludeFormsetCount += Count;
        continue;
      }
    }
    //
    // Class Guid
    //
    Count = 0;
    Status = GetFormsetsByClassGuid (
               &IncludeList[IncludeIndex],
               &Count,
               &IncludeHiiHandleList[IncludeFormsetCount],
               &IncludeFormsetGuidList[IncludeFormsetCount]
               );
    if (Status == EFI_SUCCESS) {
      IncludeFormsetCount += Count;
    }
  }

  //
  // Get Exclude Formsets
  //
  ExcludeFormsetCount = 0;
  for (ExcludeIndex = 0; ExcludeIndex < ExcludeCount; ExcludeIndex ++) {
    //
    // Formset Guid
    //
    Count = 0;
    Status = GetHiiHandleByFormSetGuid (
               &ExcludeList[ExcludeIndex],
               &Count,
               &ExcludeHiiHandleList[ExcludeFormsetCount],
               &ExcludeFormsetGuidList[ExcludeFormsetCount]
               );
    if (Status == EFI_SUCCESS) {
      ExcludeFormsetCount += Count;
      continue;
    }
    //
    // Class Guid
    //
    Count = 0;
    Status = GetFormsetsByClassGuid (
               &ExcludeList[ExcludeIndex],
               &Count,
               &ExcludeHiiHandleList[ExcludeFormsetCount],
               &ExcludeFormsetGuidList[ExcludeFormsetCount]
               );
    if (Status == EFI_SUCCESS) {
      ExcludeFormsetCount += Count;
    }
  }

  //
  // Merge Formsets
  //
  ResultFormsetCount = 0;
  for (IncludeIndex = 0; IncludeIndex < IncludeFormsetCount; IncludeIndex ++) {
    Skip = FALSE;
    for (ExcludeIndex = 0; ExcludeIndex < ExcludeFormsetCount; ExcludeIndex ++) {
      if ((IncludeHiiHandleList[IncludeIndex] == ExcludeHiiHandleList[ExcludeIndex]) ||
        CompareGuid (&IncludeFormsetGuidList[IncludeIndex], &ExcludeFormsetGuidList[ExcludeIndex])) {
        Skip = TRUE;
        break;
      }
    }
    if (!Skip) {
      ResultHiiHandleList[ResultFormsetCount] = IncludeHiiHandleList[IncludeIndex];
      CopyGuid (&ResultFormsetGuidList[ResultFormsetCount], (EFI_GUID *)(VOID *)&IncludeFormsetGuidList[IncludeIndex]);
      ResultFormsetCount ++;
    }
  }

  *FormsetCount = ResultFormsetCount;
  *HiiHandleList = (EFI_HII_HANDLE *)AllocateCopyPool (sizeof (EFI_HII_HANDLE) * ResultFormsetCount, ResultHiiHandleList);
  *FormsetGuidList = (EFI_GUID *)AllocateCopyPool (sizeof (EFI_GUID) * ResultFormsetCount, ResultFormsetGuidList);

  FreePool (IncludeHiiHandleList);
  FreePool (IncludeFormsetGuidList);
  FreePool (ExcludeHiiHandleList);
  FreePool (ExcludeFormsetGuidList);
  FreePool (ResultHiiHandleList);
  FreePool (ResultFormsetGuidList);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetIncludeByPcd (
  IN CONST EFI_GUID                   *FormsetGuid,
  OUT UINT32                          *IncludeCount,
  OUT EFI_GUID                        **IncludeList
  )
{
  UINTN                               TokenNum;
  EFI_GUID                            *Guid;
  EFI_GUID                            *TempGuidList;
  UINT32                              Count;

  if (FormsetGuid == NULL || IncludeCount == NULL || IncludeList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TempGuidList = (EFI_GUID *)AllocateZeroPool (sizeof (EFI_GUID) * 100);
  if (TempGuidList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Count = 0;
  TokenNum = LibPcdGetNextToken(FormsetGuid, 0);
  while (TokenNum != 0) {
    Guid = (EFI_GUID *) LibPcdGetExPtr(FormsetGuid, TokenNum);
    CopyGuid (&TempGuidList[Count], Guid);
    Count ++;
    TokenNum = LibPcdGetNextToken(FormsetGuid, TokenNum);
  }

  if (Count == 0) {
    FreePool (TempGuidList);
    return EFI_NOT_FOUND;
  }

  *IncludeCount = Count;
  *IncludeList = (EFI_GUID *)AllocateCopyPool (sizeof (EFI_GUID) * Count, TempGuidList);
  FreePool (TempGuidList);

  return EFI_SUCCESS;
}

//
// Form IfrParse.c
//
EFI_STATUS
InitializeRequestElement (
  IN OUT FORM_BROWSER_FORMSET     *FormSet,
  IN OUT FORM_BROWSER_STATEMENT   *Question,
  IN OUT FORM_BROWSER_FORM        *Form
  );

FORM_BROWSER_STATEMENT *
CreateQuestion (
  IN UINT8                        *OpCodeData,
  IN OUT FORM_BROWSER_FORMSET     *FormSet,
  IN OUT FORM_BROWSER_FORM        *Form
  );

STATIC
FORM_BROWSER_STATEMENT *
CreateGotoFormSetQuestion (
  IN CONST EFI_HII_HANDLE         RefHiiHandle,
  IN CONST EFI_GUID               *RefFormsetGuid,
  IN CONST EFI_FORM_ID            RefFormId,
  IN OUT FORM_BROWSER_FORMSET     *FormSet,
  IN OUT FORM_BROWSER_FORM        *Form,
  IN OUT UINT8                    *OpCodeData
  )
{
  EFI_STATUS                      Status;
  EFI_GUID                        FormsetGuid;
  EFI_STRING_ID                   FormSetTitleStrId;
  EFI_STRING_ID                   FormSetHelpStrId;
  EFI_STRING_ID                   Prompt;
  EFI_STRING_ID                   Help;
  EFI_STRING_ID                   DevicePath;
  EFI_FORM_ID                     EntryFormId;
  CHAR16                          *String;
  EFI_IFR_REF4                    *GotoFormSetOp;
  FORM_BROWSER_STATEMENT          *Statement;
  EFI_HII_VALUE                   *Value;

  if (RefHiiHandle == 0 || RefFormsetGuid == NULL || FormSet == NULL || Form == NULL || OpCodeData == NULL) {
    return NULL;
  }

  CopyMem (&FormsetGuid, RefFormsetGuid, sizeof (EFI_GUID));
  GetFormSetInfo (RefHiiHandle, &FormsetGuid, &FormSetTitleStrId, &FormSetHelpStrId, &EntryFormId);
  if (RefFormId != 0) {
    EntryFormId = RefFormId;
  } else {
    Status = GetFirstFormId (RefHiiHandle, RefFormsetGuid, &EntryFormId);
    if (EFI_ERROR (Status)) {
      return NULL;
    }
  }
  String = HiiGetString (RefHiiHandle, FormSetTitleStrId, NULL);
  if (String != NULL) {
    Prompt = HiiSetString (FormSet->HiiHandle, 0, String, NULL);
    FreePool (String);
  } else {
    Prompt = 0;
  }

  String = HiiGetString (RefHiiHandle, FormSetHelpStrId, NULL);
  if (String != NULL) {
    Help = HiiSetString (FormSet->HiiHandle, 0, String, NULL);
    FreePool (String);
  } else {
    Help = 0;
  }

  Status = GetDevicePathStrByHiiHandle (RefHiiHandle, &String);
  if (!EFI_ERROR (Status)) {
    DevicePath = HiiSetString (FormSet->HiiHandle, 0, String, NULL);
    FreePool (String);
  } else {
    DevicePath = 0;
  }

  GotoFormSetOp = (EFI_IFR_REF4 *)OpCodeData;
  GotoFormSetOp->Header.OpCode                   = EFI_IFR_REF_OP;
  GotoFormSetOp->Header.Length                   = sizeof (EFI_IFR_REF4);
  GotoFormSetOp->Question.Header.Prompt          = Prompt;
  GotoFormSetOp->Question.Header.Help            = Help;
  GotoFormSetOp->Question.QuestionId             = H2O_QUESTION_ID_INVALID;
  GotoFormSetOp->Question.VarStoreId             = 0;
  GotoFormSetOp->Question.VarStoreInfo.VarOffset = 0;
  GotoFormSetOp->Question.Flags                  = 0;
  GotoFormSetOp->FormId                          = EntryFormId;
  GotoFormSetOp->QuestionId                      = 0;
  GotoFormSetOp->DevicePath                      = DevicePath;
  CopyMem (&GotoFormSetOp->FormSetId, &FormsetGuid, sizeof (EFI_GUID));

  Statement = CreateQuestion (OpCodeData, FormSet, Form);
  if (Statement == NULL) {
    return NULL;
  }

  Value = &Statement->HiiValue;
  Value->Type = EFI_IFR_TYPE_REF;
  CopyMem (&Value->Value.ref.FormId     , &GotoFormSetOp->FormId    , sizeof (EFI_FORM_ID));
  CopyMem (&Value->Value.ref.QuestionId , &GotoFormSetOp->QuestionId, sizeof (EFI_QUESTION_ID));
  CopyMem (&Value->Value.ref.FormSetGuid, &GotoFormSetOp->FormSetId , sizeof (EFI_GUID));
  CopyMem (&Value->Value.ref.DevicePath , &GotoFormSetOp->DevicePath, sizeof (EFI_STRING_ID));
  Statement->StorageWidth = (UINT16) sizeof (EFI_HII_REF);
  InitializeRequestElement (FormSet, Statement, Form);

  return Statement;
}

//
// Form OemSvcUpdateLinkFormSetIds.c
//
EFI_STATUS
OemSvcUpdateLinkFormSetIds (
  IN CONST EFI_GUID               *FormsetGuid,
  IN OUT UINT32                   *FormsetCount,
  IN OUT EFI_HII_HANDLE           **HiiHandleList,
  IN OUT EFI_GUID                 **FormsetGuidList,
  IN OUT UINT32                   **EntryFormIdList OPTIONAL
  );

BOOLEAN
ChkInHiiHandleList (
  IN EFI_HII_HANDLE               HiiHandle
  )
{
  UINT32                          FBHiiHandleIndex;

  for (FBHiiHandleIndex = 0; FBHiiHandleIndex < mFBPrivate.HiiHandleCount; FBHiiHandleIndex ++) {
    if (mFBPrivate.HiiHandleList[FBHiiHandleIndex] == HiiHandle) {
      return TRUE;
    }
  }

  return FALSE;
}

STATIC
BOOLEAN
ChkInNeedAddHiiHandleList (
  IN OUT EFI_HII_HANDLE           HiiHandle
  )
{
  LIST_ENTRY                      *Link;
  HII_HANDLE_ENTRY                *HiiHandleEntry;

  Link = GetFirstNode (&mFBPrivate.NeedAddHiiHandleList);
  while (!IsNull (&mFBPrivate.NeedAddHiiHandleList, Link)) {
    HiiHandleEntry = (HII_HANDLE_ENTRY *) Link;
    Link = GetNextNode (&mFBPrivate.NeedAddHiiHandleList, Link);
    if (HiiHandleEntry->HiiHandle == HiiHandle) {
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
InsertHiiHandle (
  IN EFI_HII_HANDLE               HiiHandle
  )
{
  EFI_HII_HANDLE                  *NewHiiHandleList;

  if (HiiHandle == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Update HiiHandleList
  //
  NewHiiHandleList = AllocateZeroPool ((mFBPrivate.HiiHandleCount + 1) * sizeof (EFI_HII_HANDLE));
  if (NewHiiHandleList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (NewHiiHandleList, mFBPrivate.HiiHandleList, mFBPrivate.HiiHandleCount * sizeof (EFI_HII_HANDLE));
  NewHiiHandleList[mFBPrivate.HiiHandleCount] = HiiHandle;

  if (mFBPrivate.HiiHandleList != NULL && mFBPrivate.HiiHandleCount > 1) {
    FreePool (mFBPrivate.HiiHandleList);
  }

  mFBPrivate.HiiHandleList = NewHiiHandleList;
  mFBPrivate.HiiHandleCount++;

  return EFI_SUCCESS;
}

EFI_STATUS
RemoveHiiHandle (
  IN EFI_HII_HANDLE               HiiHandle
  )
{
  UINT32                          Index;
  LIST_ENTRY                      *Link;
  FORM_BROWSER_FORMSET            *FormSet;

  if (HiiHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Link = GetFirstNode (&mFBPrivate.FormSetList);
  while (!IsNull (&mFBPrivate.FormSetList, Link)) {
    FormSet = FORM_BROWSER_FORMSET_FROM_DISPLAY_LINK (Link);
    Link    = GetNextNode (&mFBPrivate.FormSetList, Link);

    if (FormSet->HiiHandle == HiiHandle) {
      return EFI_ABORTED;
    }
  }


  for (Index = 0; Index < mFBPrivate.HiiHandleCount; Index++) {
    if (mFBPrivate.HiiHandleList[Index] == HiiHandle) {
      CopyMem (
        &mFBPrivate.HiiHandleList[Index],
        &mFBPrivate.HiiHandleList[Index + 1],
        sizeof (EFI_HII_HANDLE) * (mFBPrivate.HiiHandleCount - Index - 1)
        );
      mFBPrivate.HiiHandleCount--;
      break;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InsertNeedAddHiiHandle (
  IN OUT EFI_HII_HANDLE           HiiHandle
  )
{
  HII_HANDLE_ENTRY                *HiiHandleEntry;

  if (HiiHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiHandleEntry = AllocateZeroPool (sizeof (HII_HANDLE_ENTRY));
  if (HiiHandleEntry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  HiiHandleEntry->HiiHandle = HiiHandle;

  InsertTailList (&mFBPrivate.NeedAddHiiHandleList, &HiiHandleEntry->Link);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EnsureAllFormsetsInFB (
  IN OUT UINT32                   FormsetCount,
  IN OUT EFI_HII_HANDLE           *HiiHandleList
  )
{
  UINT32                          Index;
  EFI_HII_HANDLE                  HiiHandle;


  if (FormsetCount == 0 || HiiHandleList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < FormsetCount; Index++) {
    HiiHandle = HiiHandleList[Index];
    if (!ChkInHiiHandleList (HiiHandle) &&
        !ChkInNeedAddHiiHandleList (HiiHandle)) {
      InsertHiiHandle (HiiHandle);
      InsertNeedAddHiiHandle (HiiHandle);
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetAllFormsets (
  IN LINK_DATA                    *LinkData,
  IN FORM_BROWSER_FORMSET         *FormSet,
  OUT UINT32                      *FormsetCount,
  OUT EFI_HII_HANDLE              **HiiHandleList,
  OUT EFI_GUID                    **FormsetGuidList
  )
{
  EFI_STATUS                      Status;
  UINT32                          IncludeCount;
  UINT32                          ExcludeCount;
  EFI_GUID                        *IncludeList;
  EFI_GUID                        *ExcludeList;
  EFI_GUID                        *TempIncludeList;

  if (LinkData == NULL || FormSet == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IncludeCount = LinkData->CurrentIncludeItemCount;
  ExcludeCount = LinkData->CurrentExcludeItemCount;
  if (IncludeCount == 0) {
    //
    // Empty include guid of link
    //
    // 1. Get formset guids from pcd
    //
    GetIncludeByPcd (&FormSet->Guid, &IncludeCount, &IncludeList);
    //
    // 2. Add this formset guid into include guid
    //
    TempIncludeList = AllocateZeroPool (sizeof (EFI_GUID) * (IncludeCount + 1));
    if (IncludeCount != 0) {
      CopyMem (TempIncludeList, IncludeList, sizeof (EFI_GUID) * IncludeCount);
      FreePool (IncludeList);
    }
    IncludeList = TempIncludeList;
    CopyGuid (&IncludeList[IncludeCount], &FormSet->Guid);
    IncludeCount ++;
  } else {
    IncludeList = LinkData->IncludeGuidArray;
  }

  ExcludeList = LinkData->ExcludeGuidArray;

  Status = GetFormsetsByLink (
             &FormSet->Guid,
             IncludeCount,
             ExcludeCount,
             IncludeList,
             ExcludeList,
             FormsetCount,
             HiiHandleList,
             FormsetGuidList
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (IncludeList != NULL) {
    FreePool (IncludeList);
  }
  if (ExcludeList != NULL) {
    FreePool (ExcludeList);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ParseLinkOpCodeInsertGuid (
  IN UINT16                       Function,
  IN EFI_GUID                     *Guid,
  IN OUT LINK_DATA                *LinkData
  )
{
  UINT32                          *ItemCount;
  EFI_GUID                        **GuidArray;
  EFI_GUID                        *TempGuidArray;

  if (Function == H2O_IFR_EXT_INCLUDE_FORM_SET) {
    ItemCount = &LinkData->CurrentIncludeItemCount;
    GuidArray = &LinkData->IncludeGuidArray;
  } else {
    ItemCount = &LinkData->CurrentExcludeItemCount;
    GuidArray = &LinkData->ExcludeGuidArray;
  }

  TempGuidArray = AllocateZeroPool (sizeof (EFI_GUID) * ((*ItemCount) + 1));
  if (TempGuidArray == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (TempGuidArray + (*ItemCount), Guid, sizeof (EFI_GUID));

  if ((*ItemCount) != 0) {
    CopyMem (TempGuidArray, (*GuidArray), sizeof (EFI_GUID) * (*ItemCount));
    FreePool (*GuidArray);
  }

  *GuidArray = TempGuidArray;
  (*ItemCount) ++;

  return EFI_SUCCESS;
}

EFI_STATUS
ParseLinkOpCodeEnd (
  IN LINK_DATA                    *LinkData,
  IN OUT FORM_BROWSER_FORMSET     *FormSet,
  IN OUT FORM_BROWSER_FORM        *Form
  )
{
  EFI_STATUS                      Status;
  UINT32                          FormsetCount;
  EFI_HII_HANDLE                  *HiiHandleList;
  EFI_GUID                        *FormsetGuidList;
  EFI_FORM_ID                     *EntryFormIdList;
  UINT32                          *FormIdList;
  UINT32                          Index;
  LINK_STATEMENT_NODE             *LinkStatementNode;
  LIST_ENTRY                      *Link;

  if (LinkData == NULL || FormSet == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Form == NULL) {
    //
    // compatiable link opcode in formset
    //
    Link = GetFirstNode (&FormSet->FormListHead);
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);
  }

  Status = GetAllFormsets (
             LinkData,
             FormSet,
             &FormsetCount,
             &HiiHandleList,
             &FormsetGuidList
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (FormsetCount != 0) {
    Status = GetAllFirstFormId (FormsetCount, HiiHandleList, FormsetGuidList, &EntryFormIdList);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    FormIdList = (UINT32 *)(VOID *)EntryFormIdList;
  } else {
    FormIdList = NULL;
  }

  //
  // Oem update formset list
  //
  OemSvcUpdateLinkFormSetIds (&FormSet->Guid, &FormsetCount, &HiiHandleList, &FormsetGuidList, &FormIdList);

  if (FormsetCount == 0 || HiiHandleList == NULL || FormsetGuidList == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = EnsureAllFormsetsInFB (FormsetCount, HiiHandleList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (FormIdList == NULL) {
    EntryFormIdList = (EFI_FORM_ID *)AllocateZeroPool (sizeof (EFI_FORM_ID) * FormsetCount);
    if (EntryFormIdList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    EntryFormIdList = (EFI_FORM_ID *)(VOID *)FormIdList;
  }

  //
  // Add REF4 questions in this formset's root form
  //
  for (Index = 0; Index < FormsetCount; Index ++) {

    LinkStatementNode = (LINK_STATEMENT_NODE *)AllocateZeroPool (sizeof (LINK_STATEMENT_NODE));
    if (LinkStatementNode == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    LinkStatementNode->Statement = CreateGotoFormSetQuestion (
                                     HiiHandleList[Index],
                                     &FormsetGuidList[Index],
                                     EntryFormIdList[Index],
                                     FormSet,
                                     Form,
                                     (UINT8 *)&LinkStatementNode->Ref4Opcode
                                     );

    if (LinkStatementNode->Statement != NULL) {
      InsertTailList (&FormSet->LinkStatementListHead, &LinkStatementNode->Link);
    } else {
      FreePool (LinkStatementNode);
    }
  }

  FreePool (HiiHandleList);
  FreePool (FormsetGuidList);
  FreePool (EntryFormIdList);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SetUsedQuestionId (
  IN UINT8                        *UsedQuestionIdMap,
  IN EFI_QUESTION_ID              QuestionId
  )
{
  if (UsedQuestionIdMap == NULL || QuestionId == H2O_QUESTION_ID_INVALID) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IS_MASK(UsedQuestionIdMap, QuestionId)) {
    SET_MASK (UsedQuestionIdMap, QuestionId);
    return EFI_SUCCESS;
  }

  //
  // QuestionId Redefined
  //
  ASSERT (TRUE);
  return EFI_INVALID_PARAMETER;
}

STATIC
EFI_STATUS
SetUsedQuestionIdInForm (
  IN UINT8                        *UsedQuestionIdMap,
  IN CONST FORM_BROWSER_FORM      *Form
  )
{
  LIST_ENTRY                      *Link;
  FORM_BROWSER_STATEMENT          *CurrentStatement;

  if (Form == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    CurrentStatement = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);
    SetUsedQuestionId (UsedQuestionIdMap, CurrentStatement->QuestionId);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SetUsedQuestionIdInFormset (
  IN UINT8                        *UsedQuestionIdMap,
  IN CONST FORM_BROWSER_FORMSET   *Formset
  )
{
  LIST_ENTRY                      *Link;
  FORM_BROWSER_FORM               *CurrentForm;

  if (Formset == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Link = GetFirstNode (&Formset->FormListHead);
  while (!IsNull (&Formset->FormListHead, Link)) {
    CurrentForm = FORM_BROWSER_FORM_FROM_LINK (Link);
    Link = GetNextNode (&Formset->FormListHead, Link);
    SetUsedQuestionIdInForm (UsedQuestionIdMap, CurrentForm);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_QUESTION_ID
GetUnusedQuestionId (
  IN UINT8                        *UsedQuestionIdMap
  )
{
  EFI_QUESTION_ID                 QuestionId;

  if (UsedQuestionIdMap == NULL) {
    return H2O_QUESTION_ID_INVALID;
  }

  for (QuestionId = 1; QuestionId < H2O_QUESTION_ID_MAX; QuestionId++) {
    if (!IS_MASK(UsedQuestionIdMap, QuestionId)) {
      SET_MASK (UsedQuestionIdMap, QuestionId);
      return QuestionId;
    }
  }

  return H2O_QUESTION_ID_INVALID;
}

EFI_STATUS
SetAllQuestionIdValid (
  IN FORM_BROWSER_FORMSET         *FormSet
  )
{
  LIST_ENTRY                      *Link;
  LINK_STATEMENT_NODE             *LinkStatementNode;
  UINT8                           *UsedQuestionIdMap;

  if (FormSet == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UsedQuestionIdMap = (UINT8 *)AllocateZeroPool (H2O_QUESTION_ID_MAX / 8);
  if (UsedQuestionIdMap == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetUsedQuestionIdInFormset (UsedQuestionIdMap, FormSet);

  Link = GetFirstNode (&FormSet->LinkStatementListHead);
  while (!IsNull (&FormSet->LinkStatementListHead, Link)) {
    LinkStatementNode = LINK_STATEMENT_NODE_FROM_LINK (Link);
    Link = GetNextNode (&FormSet->LinkStatementListHead, Link);
    LinkStatementNode->Statement->QuestionId = GetUnusedQuestionId (UsedQuestionIdMap);
  }

  FreePool (UsedQuestionIdMap);

  return EFI_SUCCESS;
}

