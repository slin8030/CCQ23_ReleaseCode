/** @file
  Content file contains Implementation for Variable Edit Driver

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#include "VariableEditDxe.h"
#include <Guid/H2OBdsCheckPoint.h>
#include <Library/BdsCpLib.h>

//[-start-170721-IB07400892-add]//
#define H2O_UVE_INITIALIZE_SETUP_UTILITY  0x02
#define H2O_UVE_SHUTDOWN_SETUP_UTILITY    0x03
//[-end-170721-IB07400892-add]//

UVE_DATA_INFO  mUVEDataInfo = {
                 UVE_DATA_INFO_SIGNATURE,
                 0x1,
                 {0},
                 FixedPcdGetBool (PcdH2OFormBrowserSupported),
                 {0},
                 (UINT64)(UINTN)PcdGetPtr (PcdScuFormsetGuidList),
                 (UINT64)(UINTN)PcdGetPtr (PcdScuFormsetFlagList)
                 };

extern EFI_GUID              gVarableEditGuid;

//
//  Module-defined variable
//
EFI_GUID     mVariableEditIdentifyGuid   = VARIABLE_EDIT_IDENTIFY_GUID;

VARIABLE_EDIT_TABLE                  *mTable     = NULL;
BOOLEAN                               mIsPage    = FALSE;
UINTN                                 mPageNo    = 0;
SETUP_UTILITY_BROWSER_DATA           *mSuBrowser = NULL;
VOID                                 *mFEaddress = 0;
EFI_HANDLE                            mSetupUtilityHandle;
EFI_SYS_PASSWORD_SERVICE_PROTOCOL     OldSysPasswordInstance = {0};

/**
  Check supervisor or use password is set or not.

  @param  This                 The EFI_SYS_PASSWORD_SERVICE_PROTOCOL instance.
  @param  SupervisorOrUser     determine to disable supervisor or use password

  @retval EFI_SUCCESS          password is set
  @retval EFI_NOT_FOUND        password isn't set

**/
EFI_STATUS
EFIAPI
GetPasswordStatus (
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL    *This,
  IN  BOOLEAN                              SupervisorOrUser
  )
{
  DEBUG ( (EFI_D_INFO, "[InitializeVariableEditTable] GetPasswordStatus hook call.\n"));
  return EFI_NOT_FOUND;
}

/**
  Free the resource of Variable edit table

**/
VOID
EFIAPI
FreeTable (
  VOID
  )
{

  if (NULL == mTable) {
    return;
  }

  if (mIsPage == TRUE) {
    gBS->FreePages ( (EFI_PHYSICAL_ADDRESS) mTable, mPageNo);

  } else {
    gBS->FreePool (mTable);
  }

  mTable = NULL;
  return;
}

/**
  Free the resource of PackageListsInfo

  @param[in]  PackageListsInfo       PackageListsInfo Structure array

**/
VOID
RelPackagelistInfo (
  IN  PACKAGE_LIST                      *PackageListsInfo
  )
{
  UINTN                                 Index = 0;
  if (PackageListsInfo == NULL) {
    return;
  }

  for (Index = 0x0U; Index < PackageListsInfo->ListCount; ++Index) {
    if (PackageListsInfo->ListInfo[Index].PackageList != NULL) {
      FreePool (PackageListsInfo->ListInfo[Index].PackageList);
      PackageListsInfo->ListInfo[Index].PackageList = NULL;
    }

    ZeroMem (&PackageListsInfo->ListInfo[Index], sizeof (PACKAGE_LIST_INFO));
  }
  if (PackageListsInfo->ListInfo != NULL) {
    FreePool (PackageListsInfo->ListInfo);
    PackageListsInfo->ListInfo = NULL;
  }
  PackageListsInfo->ListCount = 0;
  PackageListsInfo->RootCount = 0;
}

/**
  Get the size and pointer of Hii package list.

  @param[in] HiiDatabase         The interface of Hii database protocal
  @param[in] Handle              The Hii handles
  @param[out] Size               Pointer to the size of package list
  @param[out] PackageList        Pointer to package list

  @retval  EFI_SUCCESS
  @retval  EFI_INVALID_PARAMETER  HiiDatabase is NULL or Handle is 0
**/
EFI_STATUS
GetHiiPackageList (
  IN  EFI_HII_DATABASE_PROTOCOL             *HiiDatabase,
  IN  EFI_HII_HANDLE                        Handle,
  OUT UINTN                                 *Size OPTIONAL,
  OUT VOID                                  **PackageList OPTIONAL

  )
{
  EFI_STATUS                          Status = EFI_SUCCESS;
  UINTN                               BufferSize;
  EFI_HII_PACKAGE_LIST_HEADER         *HiiPackageList;

  if ((HiiDatabase == NULL) || (Handle == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  BufferSize = 0;
  HiiPackageList = NULL;

  Status = HiiDatabase->ExportPackageLists (HiiDatabase, Handle, &BufferSize, HiiPackageList);

  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = AllocatePool (BufferSize);
    if (HiiPackageList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Status = HiiDatabase->ExportPackageLists (HiiDatabase, Handle, &BufferSize, HiiPackageList);
  }
  if (EFI_ERROR (Status)) {
    FreePool (HiiPackageList);
    return Status;
  }

  if (Size != NULL) {
    *Size = BufferSize;
  }

  if (PackageList != NULL) {
    *PackageList = (VOID *)HiiPackageList;
  }

  return EFI_SUCCESS;
}

/**
  Check if input HII package support specific form set GUID or class GUID or not

  @param[in] FormsetGuid         The pointer to the list of form set GUID or class GUID
  @param[in] FormsetGuidFlag     The pointer to the list of form set GUID flag.
  @param[in] FormSetGuidCount    The number of form set GUID or class GUID
  @param[in] Package             Pointer to HII package

  @retval TRUE                   HII package support specific form set class GUID
  @retval FALSE                  HII package does not support specific form set class GUID
**/
BOOLEAN
SupportFormsetGuidByPackage (
  IN EFI_GUID                  *FormsetGuid,
  IN UINT8                     *FormsetGuidFlag,
  IN UINT32                    FormSetGuidCount,
  IN UINT8                     *Package
  )
{
  EFI_HII_PACKAGE_HEADER       *PackageHeader;
  UINT32                       Offset;
  UINT8                        *OpCodeData;
  UINT8                        ClassGuidNum;
  EFI_GUID                     *ClassGuid;
  UINT32                       Index;

  if ((FormsetGuid == NULL) || (FormsetGuidFlag == NULL) || (Package == NULL) || (FormSetGuidCount == 0)) {
    return FALSE;
  }

  PackageHeader = (EFI_HII_PACKAGE_HEADER *) Package;
  if (PackageHeader->Type != EFI_HII_PACKAGE_FORMS) {
    return FALSE;
  }

  Offset = sizeof (EFI_HII_PACKAGE_HEADER);
  while (Offset < PackageHeader->Length) {
    OpCodeData = Package + Offset;

    if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP &&
        ((EFI_IFR_OP_HEADER *) OpCodeData)->Length > OFFSET_OF (EFI_IFR_FORM_SET, Flags)) {
      for (Index = 0; Index < FormSetGuidCount; Index++) {
        if ((FormsetGuidFlag[Index] == FormsetGuidFlagFormsetGuid ||
             FormsetGuidFlag[Index] == FormsetGuidFlagAll) &&
            CompareGuid ((GUID *)(VOID *)&((EFI_IFR_FORM_SET *) OpCodeData)->Guid, &FormsetGuid[Index])) {
          return TRUE;
        }
      }

      ClassGuidNum = (UINT8) (((EFI_IFR_FORM_SET *) OpCodeData)->Flags & 0x3);
      ClassGuid    = (EFI_GUID *) (VOID *) (OpCodeData + sizeof (EFI_IFR_FORM_SET));

      while (ClassGuidNum > 0) {
        for (Index = 0; Index < FormSetGuidCount; Index++) {
          if ((FormsetGuidFlag[Index] == FormsetGuidFlagFormsetClassGuid ||
               FormsetGuidFlag[Index] == FormsetGuidFlagAll) &&
              CompareGuid (ClassGuid, &FormsetGuid[Index])) {
            return TRUE;
          }
        }

        ClassGuidNum--;
        ClassGuid++;
      }
    }

    Offset += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
  }

  return FALSE;
}

/**
  Check if input Hii PackageList support specific form set GUID or class GUID or not.

  @param[in] FormsetGuid         The pointer to the list of form set GUID or class GUID
  @param[in] FormsetGuidFlag     The pointer to the list of form set GUID flag
  @param[in] FormSetGuidCount    The number of form set GUID or class GUID
  @param[in] HiiPackageList      The HII PackageList.

  @retval TRUE                   HII PackageList support specific form set class GUID.
  @retval FALSE                  HII PackageList does not support specific form set class GUID.
**/
BOOLEAN
SupportFormsetGuid (
  IN EFI_GUID                  *FormsetGuid,
  IN UINT8                     *FormsetGuidFlag,
  IN UINT32                    FormSetGuidCount,
  IN EFI_HII_PACKAGE_LIST_HEADER *HiiPackageList
  )
{
  UINT8                        *Package;
  EFI_HII_PACKAGE_HEADER       *PackageHeader;
  UINT32                       Offset;
  UINT32                       PackageListLength;

  if ((HiiPackageList == NULL) || (FormsetGuidFlag == NULL) || (FormsetGuid == NULL) || (FormSetGuidCount == 0)) {
    return FALSE;
  }

  //
  // In HII package list, find the Form package which support specific form set class GUID.
  //
  Offset            = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  PackageListLength = ReadUnaligned32 (&HiiPackageList->PackageLength);
  while (Offset < PackageListLength) {
    Package       = (UINT8 *) HiiPackageList + Offset;
    PackageHeader = (EFI_HII_PACKAGE_HEADER *) Package;

    if (PackageHeader->Type == EFI_HII_PACKAGE_FORMS &&
        SupportFormsetGuidByPackage (FormsetGuid, FormsetGuidFlag, FormSetGuidCount, Package)) {
      return TRUE;
    }

    Offset += PackageHeader->Length;
  }

  return FALSE;
}

/**
  Get the list of form set GUID flag.

  @param[out] FormsetGuidFlag       The pointer to the list of form set GUID flag

  @retval EFI_SUCCESS
  @retval EFI_NOT_FOUND             FormSetGuidCount is zero.
  @retval EFI_OUT_OF_RESOURCES      Allocate memory fail.

**/
STATIC
EFI_STATUS
GetFormsetFlagList (
  OUT UINT8                              **FormsetGuidFlagOut
  )
{
  EFI_GUID                              *PcdFormSetGuid;
  EFI_GUID                              ZeroGuid;
  UINT32                                FormSetGuidCount;
  UINT32                                Index;
  UINT8                                 *FormsetGuidFlag;
  UINT32                                FormsetGuidFlagCount;

  FormSetGuidCount = 0;
  //
  // Get Root Formset order
  //
  ZeroMem (&ZeroGuid, sizeof (EFI_GUID));
  PcdFormSetGuid      = PcdGetPtr(PcdScuFormsetGuidList);
  while (!CompareGuid (&PcdFormSetGuid[FormSetGuidCount], &ZeroGuid)) {
    FormSetGuidCount++;
  }
  if (FormSetGuidCount == 0) {
    DEBUG ( (EFI_D_INFO, "Warning! FormSetGuidCount is zero.\n"));
    return EFI_NOT_FOUND;
  }

  FormsetGuidFlag      = (UINT8 *) PcdGetPtr (PcdScuFormsetFlagList);
  FormsetGuidFlagCount = 0;
  while (FormsetGuidFlag[FormsetGuidFlagCount] != 0xFF) {
    FormsetGuidFlagCount++;
  }

  FormsetGuidFlag      = AllocateZeroPool (FormSetGuidCount + 1);
  if (FormsetGuidFlag == NULL) {
    DEBUG ( (EFI_D_ERROR, "Error: AllocateZeroPool for FormsetGuidFlag failed! Allocate size: 0x%x\n", FormSetGuidCount + 1));
    return EFI_OUT_OF_RESOURCES;
  }

  if (FormsetGuidFlagCount != FormSetGuidCount) {
    for (Index = 0; Index < FormSetGuidCount; Index++) {
      FormsetGuidFlag[Index] = FormsetGuidFlagAll;
    }
    FormsetGuidFlag[Index] = 0xFF;
  } else {
    CopyMem (FormsetGuidFlag, FormsetGuidFlag, FormSetGuidCount + 1);
  }

  *FormsetGuidFlagOut = FormsetGuidFlag;

  return EFI_SUCCESS;
}

/**
  Get the pointer to Hii package header.

  @param[in] PackageList            The pointer to the headre of HII package list.
  @param[in] PackageType            The HII package type.

  @retval EFI_HII_PACKAGE_HEADER *  Success to get the pointer of Hii package header.
  @retval NULL                      Form package not found in this Package List

**/
STATIC
EFI_HII_PACKAGE_HEADER *
GetPackagePtr (
  IN  EFI_HII_PACKAGE_LIST_HEADER      *PackageList,
  IN  UINT8                             PackageType
  )
{
  UINT32                       Offset;
  UINT32                       PackageListLength;
  UINT8                        *Package;
  EFI_HII_PACKAGE_HEADER       PackageHeader;

  Package = NULL;

  //
  // Get Form package from this HII package List
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  CopyMem (&PackageListLength, &PackageList->PackageLength, sizeof (UINT32));

  while (Offset < PackageListLength) {
    Package = ((UINT8 *) PackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == PackageType) {
      break;
    }

    Offset += PackageHeader.Length;
  }

  if (Offset >= PackageListLength) {
    //
    // Form package not found in this Package List
    //
    return NULL;
  }

  return (EFI_HII_PACKAGE_HEADER *)Package;
}

/**
  Initial the package list when PcdH2OFormBrowserSupported is FALSE.

  @param[out] PackageListsInfo      PackageListsInfo Structure array.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER     PackageListsInfo is NULL.
  @retval EFI_OUT_OF_RESOURCES      Allocate memory fail.

**/
STATIC
EFI_STATUS
EFIAPI
InitPkgListInfoInSetupBrowser (
  OUT PACKAGE_LIST                     *PackageListsInfo
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINTN                                 Index = 0;
  PACKAGE_LIST_INFO                    *PackageListsInfoAry;
  UINTN                                 Size = 0;
  UINTN                                 ListCount = 0;
  VOID                                 *PackageList = NULL;
  UINT16                                HiiHandlesList[] =
                                        {
                                          ExitHiiHandle,
                                          BootHiiHandle,
                                          PowerHiiHandle,
                                          SecurityHiiHandle,
                                          AdvanceHiiHandle,
                                          MainHiiHandle
                                        };

  if (NULL == PackageListsInfo) {
    return EFI_INVALID_PARAMETER;
  }

  ListCount = sizeof (HiiHandlesList) / sizeof (UINT16);

  DEBUG ( (EFI_D_INFO, "ListCount:0x%lx\n", ListCount));
  PackageListsInfoAry = AllocateZeroPool (ListCount * sizeof(PACKAGE_LIST_INFO));
  if (PackageListsInfoAry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PackageListsInfo->ListInfo = PackageListsInfoAry;

  //
  // Get All Package lists info
  //
  for (Index = 0x0U, PackageListsInfo->ListCount = 0; Index < ListCount; ++Index) {
    if (mSuBrowser->SUCInfo->MapTable[HiiHandlesList[Index]].HiiHandle == 0) {
      continue;
    }

    Status = GetHiiPackageList (mSuBrowser->HiiDatabase, mSuBrowser->SUCInfo->MapTable[HiiHandlesList[Index]].HiiHandle, &Size, &PackageList);
    if (EFI_ERROR (Status)) {
      RelPackagelistInfo (PackageListsInfo);
      return Status;
    }

    if (Size <= sizeof (EFI_HII_PACKAGE_LIST_HEADER)) {
      continue;
    }

    PackageListsInfoAry[PackageListsInfo->ListCount].pFormsetHdr = GetPackagePtr (PackageList, EFI_HII_PACKAGE_FORMS);
    if (PackageListsInfoAry[PackageListsInfo->ListCount].pFormsetHdr == NULL) {
      DEBUG ( (EFI_D_INFO, "HiiHandlesList[%d]: 0x%x, No Forms type package\n", (UINT16)Index, HiiHandlesList[Index]));
      continue;
    }

    PackageListsInfoAry[PackageListsInfo->ListCount].pStringHdr = GetPackagePtr (PackageList, EFI_HII_PACKAGE_STRINGS);
    if (PackageListsInfoAry[PackageListsInfo->ListCount].pStringHdr == NULL) {
      PackageListsInfoAry[PackageListsInfo->ListCount].pFormsetHdr = NULL;
      DEBUG ( (EFI_D_INFO, "HiiHandlesList[%d]: 0x%x, No Strings type package\n", (UINT16)Index, HiiHandlesList[Index]));
      continue;
    }

    DEBUG ( (EFI_D_INFO, "HiiHandle: %p, Size:0x%lx, PackageList: %p\n", mSuBrowser->SUCInfo->MapTable[HiiHandlesList[Index]].HiiHandle, Size, PackageList));
    PackageListsInfoAry[PackageListsInfo->ListCount].Size = Size;
    PackageListsInfoAry[PackageListsInfo->ListCount].PackageList = PackageList;
    PackageListsInfoAry[PackageListsInfo->ListCount].HiiHandle = mSuBrowser->SUCInfo->MapTable[HiiHandlesList[Index]].HiiHandle;
    PackageListsInfoAry[PackageListsInfo->ListCount].Order = (UINT16)PackageListsInfo->ListCount;
    PackageListsInfo->ListCount++;
  }

  PackageListsInfo->RootCount = PackageListsInfo->ListCount;

  return EFI_SUCCESS;
}

/**
  Compares two PACKAGE_LIST_INFO, and returns the difference
  between S1 and S2.

  @param  S1     A pointer to a PACKAGE_LIST_INFO.
  @param  S2     A pointer to a PACKAGE_LIST_INFO.

  @retval 0      Order of S1 is identical to the order of S2.
  @return others Order of S1 is not identical to the order of S2.

**/
STATIC
int
PackageListsInfoOrderCmp (
  const void *S1,
  const void *S2
  )
{
  PACKAGE_LIST_INFO *S1PkgInfo = (PACKAGE_LIST_INFO *)S1;
  PACKAGE_LIST_INFO *S2PkgInfo = (PACKAGE_LIST_INFO *)S2;

  return ((int)S1PkgInfo->Order - (int)S2PkgInfo->Order);
}

/**
  Initialize the package list when PcdH2OFormBrowserSupported is TRUE.

  @param[out]  PackageListsInfo PackageListsInfo Structure array

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER  when PackageListsInfo is NULL
  @retval EFI_NOT_FOUND          No HII handles were found in the HII database.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory fail
**/
STATIC
EFI_STATUS
EFIAPI
InitPkgListInfoInH2OFormBrowser (
  OUT PACKAGE_LIST                     *PackageListsInfo
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  PACKAGE_LIST_INFO                     *PackageListsInfoAry;
  EFI_HII_HANDLE                        *HiiHandles;
  UINTN                                 Index;
  UINTN                                 ListCount;
  EFI_GUID                              *FormSetGuid;
  UINTN                                 Size = 0;
  VOID                                  *PackageList = NULL;
  UINTN                                 Num;
  UINTN                                 PkgListIndex = 0;
  UINTN                                 HiiHandleCount = 0;
  EFI_GUID                              ZeroGuid;
  UINT8                                 *FormsetGuidFlag;

  if (NULL == PackageListsInfo) {
    return EFI_INVALID_PARAMETER;
  }

  HiiHandles = HiiGetHiiHandles (NULL);
  if (HiiHandles == NULL) {
    return EFI_NOT_FOUND;
  }
  ListCount = 0;
  while (HiiHandles[ListCount] != NULL) {
    ListCount++;
  }

  PackageListsInfoAry = AllocateZeroPool (ListCount * sizeof(PACKAGE_LIST_INFO));
  if (PackageListsInfoAry == NULL) {
    FreePool (HiiHandles);
    return EFI_OUT_OF_RESOURCES;
  }

  PackageListsInfo->ListInfo = PackageListsInfoAry;

  for (Index = 0, Num = 0; Index < ListCount; Index++) {

    DEBUG ( (EFI_D_INFO, "HiiHandle: %p\n", HiiHandles[Index]));

    //
    // Get PackageList
    //
    Status = GetHiiPackageList (mSuBrowser->HiiDatabase, HiiHandles[Index], &Size, &PackageList);
    if (EFI_ERROR (Status)) {
      RelPackagelistInfo (PackageListsInfo);
      FreePool (HiiHandles);
      return Status;
    }

    if (Size <= sizeof (EFI_HII_PACKAGE_LIST_HEADER)) {
      if (PackageList != NULL) {
        FreePool (PackageList);
        PackageList = NULL;
      }
      continue;
    }

    DEBUG ( (EFI_D_INFO, "Size:0x%lx, PackageList: %p\n", Size, PackageList));

    PackageListsInfoAry[Num].pFormsetHdr = GetPackagePtr (PackageList, EFI_HII_PACKAGE_FORMS);
    if (PackageListsInfoAry[Num].pFormsetHdr == NULL) {
      DEBUG ( (EFI_D_INFO, "No Forms type package\n"));
      continue;
    }

    PackageListsInfoAry[Num].pStringHdr = GetPackagePtr (PackageList, EFI_HII_PACKAGE_STRINGS);
    if (PackageListsInfoAry[Num].pStringHdr == NULL) {
      PackageListsInfoAry[Num].pFormsetHdr = NULL;
      DEBUG ( (EFI_D_INFO, "No Strings type package\n"));
      continue;
    }

    PackageListsInfoAry[Num].Size = Size;
    PackageListsInfoAry[Num].PackageList = PackageList;
    PackageListsInfoAry[Num].HiiHandle = HiiHandles[Index];
    PackageListsInfoAry[Num].Order = FORMSET_ORDER_MAX;
    HiiHandles[Index] = NULL;
    Num++;
  }

  //
  // Set list count
  //
  PackageListsInfo->ListCount = Num;

  //
  // Get Root Formset order
  //
  ZeroMem (&ZeroGuid, sizeof (EFI_GUID));
  FormSetGuid = PcdGetPtr(PcdScuFormsetGuidList);

  Status = GetFormsetFlagList (&FormsetGuidFlag);
  if (Status != EFI_SUCCESS) {
    RelPackagelistInfo (PackageListsInfo);
    FreePool (HiiHandles);
    return Status;
  }

  for (Index = 0, HiiHandleCount = 0; (CompareMem (&FormSetGuid[Index], &ZeroGuid, sizeof (EFI_GUID)) != 0); Index++) {
    for (PkgListIndex = 0; PkgListIndex < Num; ++PkgListIndex) {
      if (SupportFormsetGuid (&FormSetGuid[Index], &FormsetGuidFlag[Index], 1, PackageListsInfoAry[PkgListIndex].PackageList)) {
        DEBUG ( (EFI_D_INFO, "Root formset HiiHandle: %p. \n", PackageListsInfoAry[PkgListIndex].HiiHandle));
        HiiHandles[HiiHandleCount] = PackageListsInfoAry[PkgListIndex].HiiHandle;
        HiiHandleCount++;
        break;
      }
    }

    if (PkgListIndex >= Num) {
      DEBUG ( (EFI_D_INFO, "Get HiiHandle with Formset guid(%g) failed. \n", &FormSetGuid[Index]));
    }
  }

  //
  // Call OEM service function to determine setup menu when SCU
  //
  OemSvcAdjustNavigationMenu (HiiHandles, &HiiHandleCount, &ZeroGuid);

  DEBUG ( (EFI_D_INFO, "Formset HiiHandleCount: %ld \n", HiiHandleCount));

  //
  // Set Order PackageListsInfo by Hii Handle list order
  //
  for (Index = 0, PackageListsInfo->RootCount = 0; Index < HiiHandleCount; ++Index) {

    for (PkgListIndex = 0; PkgListIndex < Num; PkgListIndex++) {

      if (HiiHandles[Index] != PackageListsInfoAry[PkgListIndex].HiiHandle) {
        continue;
      }

      PackageListsInfoAry[PkgListIndex].Order = (UINT16)Index;
      PackageListsInfo->RootCount++;
      break;
    }

    if (PkgListIndex >= Num) {
      DEBUG ( (EFI_D_INFO, "Get PackageListsInfoAry with HiiHandle(%p) failed. \n", HiiHandles[Index]));
    }
  }

  DEBUG ( (EFI_D_INFO, "PackageListsInfo RootCount: %ld, ListCount: %ld\n", PackageListsInfo->RootCount, PackageListsInfo->ListCount));

  qsort (PackageListsInfoAry, Num, sizeof (PACKAGE_LIST_INFO), PackageListsInfoOrderCmp);
  FreePool (HiiHandles);
  FreePool (FormsetGuidFlag);
  return EFI_SUCCESS;
}

/**
  Get Package Info.

  @param[in]  PackageListsInfo PackageListsInfo Structure array

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER  when PackageListsInfo is NULL
  @retval EFI_NOT_FOUND          No protocol instances were found that match Protocol or
                                 No HII handles were found in the HII database.
**/
EFI_STATUS
EFIAPI
InitPackageListInfo (
  OUT PACKAGE_LIST                     *PackageListsInfo
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL   *Interface = NULL;

  if (NULL == PackageListsInfo) {
    return EFI_INVALID_PARAMETER;
  }
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityBrowserProtocolGuid,
                  NULL,
                  (VOID **)&Interface
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }

  mSuBrowser = EFI_SETUP_UTILITY_BROWSER_FROM_THIS (Interface);

  if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
    Status = InitPkgListInfoInH2OFormBrowser (PackageListsInfo);
  } else {
    Status = InitPkgListInfoInSetupBrowser (PackageListsInfo);
  }
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
  }

  return Status;
}

/**
  Calculate the total required size of Variable edit table.

 @param [in]   PackageListsInfo      Pointer to the struct of PACKAGE_LIST.
 @param [out]  TotalSize             Pointer to the total size of PackageListsInfo.

 @retval Status                 EFI_SUCCESS

**/

EFI_STATUS
EFIAPI
GetRequiredSize (
  IN  PACKAGE_LIST   *PackageListsInfo,
  OUT UINTN          *TotalSize
  )
{
  UINTN                                 Index = 0;
  UINTN                                 Size = 0;

  if ((NULL == PackageListsInfo) ||(NULL == TotalSize)) {
    return EFI_INVALID_PARAMETER;
  }

  *TotalSize = sizeof (VARIABLE_EDIT_TABLE);

  for (Index = 0x0U; Index < PackageListsInfo->ListCount; ++Index) {

    //
    // Calculating the size of the HiiHandle
    //
    Size = PackageListsInfo->ListInfo[Index].Size - sizeof (EFI_HII_PACKAGE_LIST_HEADER);

    *TotalSize += Size;
  }

  //
  // Reserved Space for the VFR_STR_OFFSET_PAIR
  //
  *TotalSize += PackageListsInfo->ListCount * sizeof (VFR_STR_OFFSET_PAIR);

  return EFI_SUCCESS;
}

/**
  Allocate memory to store Variable Edit Table

  @param[in]   NeedSize      The size of mTable needed.
  @param[out]  AllocateSize  The actual size of mTable used.

  @retval EFI_SUCCESS        Allocate success

**/
EFI_STATUS
EFIAPI
AllocateReservedMemory (
  IN  UINTN          NeedSize,
  OUT UINTN         *AllocateSize
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
  VOID                 *TempAddress = NULL;
  EFI_PHYSICAL_ADDRESS  PhyAddress = 0;

  if (NULL == AllocateSize) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->AllocatePool (
             EfiReservedMemoryType,
             NeedSize,
             (VOID **)&TempAddress
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //  Check whether the address is smaller than the 4G
  //
  if ( (UINTN) TempAddress > VARIABLE_EDIT_TABLE_MAX_ADDRESS) {
    Status = gBS->FreePool (TempAddress);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    PhyAddress = VARIABLE_EDIT_TABLE_MAX_ADDRESS;
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiReservedMemoryType,
                    EFI_SIZE_TO_PAGES (NeedSize),
                    &PhyAddress
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    mIsPage = TRUE;
    mPageNo = EFI_SIZE_TO_PAGES (NeedSize);
    *AllocateSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (NeedSize));
    mTable = (VARIABLE_EDIT_TABLE *) (UINTN) PhyAddress;

  } else {
    mIsPage = FALSE;
    *AllocateSize = NeedSize;
    mTable = (VARIABLE_EDIT_TABLE *) TempAddress;
  }

  gBS->SetMem (mTable, *AllocateSize, 0);

  return EFI_SUCCESS;
}

/**
  Fill data into Variable Edit Table

  @param[in]  PackageListsInfo PackageListsInfo Structure array

  @retval EFI_SUCCESS        Fill success

**/
EFI_STATUS
EFIAPI
FillAllData (
  IN  PACKAGE_LIST  *PackageListsInfo
  )
{
  UINTN                         Index = 0;
  UINTN                         Size = 0;
  VFR_STR_OFFSET_PAIR          *PairPtr = NULL;
  UINT8                        *DataPtr = NULL;
  EFI_HII_PACKAGE_HEADER       *PkgHdr  = NULL;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList  = NULL;

  if (NULL == PackageListsInfo) {
    return EFI_INVALID_PARAMETER;
  }

  PairPtr = (VFR_STR_OFFSET_PAIR *) (mTable + 1);
  DataPtr = (UINT8 *) (PairPtr + PackageListsInfo->ListCount);

  for (Index = 0x0U; Index < PackageListsInfo->ListCount; ++Index) {
    DEBUG ( (EFI_D_INFO, "FillAll PackageListsInfo->ListInfo[%d].Order: 0x%x \n", Index, PackageListsInfo->ListInfo[Index].Order));
    Size = PackageListsInfo->ListInfo[Index].Size - sizeof (EFI_HII_PACKAGE_LIST_HEADER);
    PackageList = (EFI_HII_PACKAGE_LIST_HEADER *)PackageListsInfo->ListInfo[Index].PackageList;
    CopyMem (DataPtr, PackageList + 1, Size);

    PkgHdr = (EFI_HII_PACKAGE_HEADER *) (DataPtr + ((UINTN)PackageListsInfo->ListInfo[Index].pFormsetHdr - (UINTN)(PackageList + 1)));
    PairPtr[Index].VfrBinOffset = (UINT32) ( (UINTN) PkgHdr - (UINTN) mTable);

    PkgHdr = (EFI_HII_PACKAGE_HEADER *) (DataPtr + ((UINTN)PackageListsInfo->ListInfo[Index].pStringHdr - (UINTN)(PackageList + 1)));
    PairPtr[Index].StrPkgOffset = (UINT32) ( (UINTN) PkgHdr - (UINTN) mTable);

    DataPtr += Size;
  }

  mTable->Signature  = VARIABLE_EDIT_TABLE_SIGNATURE;
  mTable->SmiNumber  = 0xFF;
  mTable->BootTypeOrderSize     =       FIELD_SIZE (KERNEL_CONFIGURATION, BootTypeOrder);
  mTable->BootTypeOrderOffset   = EFI_FIELD_OFFSET (KERNEL_CONFIGURATION, BootTypeOrder);
  mTable->PairCount  = (UINT32) PackageListsInfo->ListCount;


  mTable->OemDevOffset           = (UINT32) 0;
  mTable->BootTypeOrderStringID  = (UINT16) 0;
  mTable->VfrStrPkgOffset        = (UINT32) 0;
  mTable->LoadDefaultCallBackID  = (UINT16) KEY_LOAD_OPTIMAL;
  mTable->Flag = 0;
  if (PcdGetBool(PcdH2OFormBrowserSupported)) {
    mTable->Flag |= (UINT16)FLAG_VAR_EDIT_TBL_H2O_FORM_BROWSER_SUPPORTED;
  } else {
    mTable->Flag &= (UINT16)(~FLAG_VAR_EDIT_TBL_H2O_FORM_BROWSER_SUPPORTED);
  }

  mTable->RootFormsetCount = (UINT16)PackageListsInfo->RootCount;

  return EFI_SUCCESS;
}

/**
  Find a suitable memory space to store Variable Edit Ptr
  find the E/F segment first. If there is no memory space, allocate it in high memory then store
  the address in the Variable - Varedit

  @param[in]  AllocateSize       the Variable Edit table size
  @param[in]  FEAddress         the address to store the signature

  @retval TODO

**/
EFI_STATUS
EFIAPI
GetFEsegmentTableAddress (
  IN  UINTN         AllocateSize,
  OUT VOID        **FEAddress
  )
{
  EFI_STATUS                   Status;
  LEGACY_BIOS_INSTANCE        *BiosPrivate   = NULL;
  EFI_LEGACY_BIOS_PROTOCOL    *LegacyBios    = NULL;
  EFI_LEGACY_REGION2_PROTOCOL *LegacyRegion  = NULL;
  EFI_IA32_REGISTER_SET        Regs = {0};
  VARIABLE_EDIT_PTR           *Point = NULL;

  if (NULL == FEAddress) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // LocateProtocol : LegacyBios
  //
  Status = gBS->LocateProtocol (
                  &gEfiLegacyBiosProtocolGuid,
                  NULL,
                  (VOID **)&LegacyBios
                  );
  if (EFI_ERROR (Status)) {
    //
    //  Pure EFI Boot. Allocate high memory to store the signature, and store the address in the variable
    //
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (VARIABLE_EDIT_PTR),
                    (VOID **)&Point
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Point->Signature  = VARIABLE_EDIT_PTR_SIGNATURE;
    Point->Address    = (UINT32) (UINTN) mTable;
    Point->Size       = (UINT32) AllocateSize;

    *FEAddress = (VOID *) Point;

    DEBUG ( (EFI_D_INFO, "GetFEsegmentTableAddress:  Table          at 0x%x\n", *FEAddress));

    Status = gRT->SetVariable (
                   L"VarEdit",
                   &gVarableEditGuid,
                   EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                   sizeof (VARIABLE_EDIT_PTR),
                   (VOID *)Point
                   );

    if (EFI_ERROR (Status)) {
      return Status;
    }
    return EFI_SUCCESS;
  }

  //
  // LocateProtocol : LegacyRegion
  //
  Status = gBS->LocateProtocol (
                  &gEfiLegacyRegion2ProtocolGuid,
                  NULL,
                  (VOID **)&LegacyRegion
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //  Get FE segment Table Address
  //
  LegacyRegion->UnLock (LegacyRegion, 0xE0000, 0x20000, NULL);

  ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
  Regs.X.AX = Legacy16GetTableAddress;
  Regs.X.BX = F0000Region;
  Regs.X.CX = (UINT16) sizeof (VARIABLE_EDIT_PTR);
  Regs.X.DX = VARIABLE_EDIT_PTR_ALIGNMENT;

  BiosPrivate = LEGACY_BIOS_INSTANCE_FROM_THIS (LegacyBios);
  Status = LegacyBios->FarCall86 (
                         LegacyBios,
                         BiosPrivate->Legacy16CallSegment,
                         BiosPrivate->Legacy16CallOffset,
                         &Regs,
                         NULL,
                         0
                         );

  if (!EFI_ERROR (Regs.X.AX)) {
    Point = (VARIABLE_EDIT_PTR *) (UINTN) (Regs.X.DS * 16 + Regs.X.BX);
    Point->Signature  = VARIABLE_EDIT_PTR_SIGNATURE;
    Point->Address    = (UINT32) (UINTN) mTable;
    Point->Size       = (UINT32) AllocateSize;
  }

  LegacyRegion->Lock (LegacyRegion, 0xE0000, 0x20000, NULL);

  if (EFI_ERROR (Regs.X.AX)) {
    return Regs.X.AX;
  }

  //
  //  Return value
  //
  *FEAddress = (VOID *) Point;

  return EFI_SUCCESS;
}

/**
  Print the Variable Edit ptr and table

  @param[in]  the variable edit ptr address which store in E/F segment or variabel (VarEdit)

**/
VOID
EFIAPI
PrintDebug (
  IN  VOID        *FEAddress
  )
{
  VARIABLE_EDIT_PTR         *Point  = NULL;
  VARIABLE_EDIT_TABLE       *Table  = NULL;
  EFI_HII_PACKAGE_HEADER    *PkgHeader = NULL;
  UINTN                      Index  = 0;
  VFR_STR_OFFSET_PAIR       *PairPtr = NULL;

  if (NULL == FEAddress) {
    return;
  }

  Point = (VARIABLE_EDIT_PTR *) FEAddress;
  if (!IS_VARIABLE_EDIT_PTR (Point)) {
    return;
  }
  DEBUG ( (EFI_D_INFO, "  Point          at 0x%x\n", Point));
  DEBUG ( (EFI_D_INFO, "  Point->Address is 0x%x\n", Point->Address));
  DEBUG ( (EFI_D_INFO, "  Point->Size    is 0x%x\n", Point->Size));

  Table = (VARIABLE_EDIT_TABLE *) (UINTN) Point->Address;
  if (!IS_VARIABLE_EDIT_TABLE (Table)) {
    return;
  }
  DEBUG ( (EFI_D_INFO, "  Table          at 0x%x\n", Table));

  PairPtr = (VFR_STR_OFFSET_PAIR *) (Table + 1);
  for (Index = 0; Index < Table->PairCount; ++Index) {
    PkgHeader = (EFI_HII_PACKAGE_HEADER *) (PairPtr[Index].VfrBinOffset + (UINTN) Table);
    DEBUG ( (EFI_D_INFO, "    Pair[Vfr] at 0x%x -> Size: 0x%x, Type:0x%x\n", PkgHeader, PkgHeader->Length, PkgHeader->Type));

    PkgHeader = (EFI_HII_PACKAGE_HEADER *) (PairPtr[Index].StrPkgOffset + (UINTN) Table);
    DEBUG ( (EFI_D_INFO, "    Pair[Str] at 0x%x -> Size: 0x%x, Type:0x%x\n", PkgHeader, PkgHeader->Length, PkgHeader->Type));
  }
  return;
}

/**
  Update Setup variable by password status.

  @param[in,out]  Setup variable.

**/
VOID
PasswordStatusUpdate (
  IN OUT KERNEL_CONFIGURATION       *SetupVariable
  )
{
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *SysPasswordService = NULL;
  EFI_STATUS                              Status;

  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **) &SysPasswordService
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }
  if (SysPasswordService != NULL) {
    Status = SysPasswordService->GetStatus(SysPasswordService, SystemSupervisor);
    if (Status == EFI_SUCCESS) {
      SetupVariable->SupervisorFlag = 1;
    }

    Status = SysPasswordService->GetStatus(SysPasswordService, SystemUser);
    if (Status == EFI_SUCCESS) {
      SetupVariable->UserFlag = 1;
    }
  }
}

VOID
EFIAPI
InitializeSetupUtilityFunc (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  EFI_SETUP_UTILITY_APPLICATION_PROTOCOL  *SetupUtilityApp = NULL;
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *SysPasswordPoint = NULL;
  EFI_STATUS                               Status;
  static BOOLEAN                           IsDone = FALSE;

  if (IsDone == TRUE) {
    return;
  }
  IsDone = TRUE;

  DEBUG ( (EFI_D_INFO, "[%a:%d] start\n", __FUNCTION__, __LINE__));

  SetupUtilityApp = AllocatePool (sizeof(EFI_SETUP_UTILITY_APPLICATION_PROTOCOL));
  if (SetupUtilityApp == NULL) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:EFI_OUT_OF_RESOURCES\n", __FUNCTION__, __LINE__));
    return;
  }

  //
  // Install setup utility application protocol to trigger VFR driver to install HII package
  //
  mSetupUtilityHandle = NULL;
//[-start-170721-IB07400892-modify]//
#ifdef H2O_UVE_INITIALIZE_SETUP_UTILITY
    SetupUtilityApp->VfrDriverState = H2O_UVE_INITIALIZE_SETUP_UTILITY;
#else
    SetupUtilityApp->VfrDriverState = InitializeSetupUtility;
#endif
//[-end-170721-IB07400892-modify]//
  Status = gBS->InstallProtocolInterface (
                  &mSetupUtilityHandle,
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  SetupUtilityApp
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }

  //
  //  Disable System password check temporarily.
  //
  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&SysPasswordPoint
                  );

  if (EFI_ERROR (Status)) {
    SysPasswordPoint = NULL;
  } else {
    CopyMem (&OldSysPasswordInstance, SysPasswordPoint, sizeof (EFI_SYS_PASSWORD_SERVICE_PROTOCOL));
    SysPasswordPoint->GetStatus = GetPasswordStatus;
  }

  DEBUG ( (EFI_D_INFO, "[%a:%d] end\n", __FUNCTION__, __LINE__));

}

VOID
EFIAPI
ShutdownSetupUtilityFunc (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  EFI_SETUP_UTILITY_APPLICATION_PROTOCOL   *SetupUtilityApp = NULL;
  EFI_STATUS                               Status;
  static BOOLEAN                           IsDone = FALSE;

  if (IsDone == TRUE) {
    return;
  }
  IsDone = TRUE;

  DEBUG ( (EFI_D_INFO, "[%a:%d] start\n", __FUNCTION__, __LINE__));

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtilityApp
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }
  //
  // Re-install setup utility application protocol to trigger VFR driver to uninstall HII package
  //
//[-start-170721-IB07400892-modify]//
#ifdef H2O_UVE_SHUTDOWN_SETUP_UTILITY
  SetupUtilityApp->VfrDriverState = H2O_UVE_SHUTDOWN_SETUP_UTILITY;
#else
  SetupUtilityApp->VfrDriverState = ShutdownSetupUtility;
#endif
//[-end-170721-IB07400892-modify]//
  Status = gBS->ReinstallProtocolInterface (
                  mSetupUtilityHandle,
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  SetupUtilityApp,
                  SetupUtilityApp
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
  }

  DEBUG ( (EFI_D_INFO, "[%a:%d] end\n", __FUNCTION__, __LINE__));

}

VOID
EFIAPI
UninstallSetupUtilityFunc (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  EFI_SETUP_UTILITY_APPLICATION_PROTOCOL   *SetupUtilityApp = NULL;
  EFI_STATUS                               Status;
  static BOOLEAN                           IsDone = FALSE;

  if (IsDone == TRUE) {
    return;
  }
  IsDone = TRUE;

  DEBUG ( (EFI_D_INFO, "[%a:%d] start\n", __FUNCTION__, __LINE__));

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtilityApp
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }
  Status = gBS->UninstallProtocolInterface (
                  mSetupUtilityHandle,
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  SetupUtilityApp
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
  }

  DEBUG ( (EFI_D_INFO, "[%a:%d] end\n", __FUNCTION__, __LINE__));

}

/**
  Collect and fill SCU data to VARIABLE_EDIT_TABLE.
  VARIABLE_EDIT_TABLE is for H2OUVE utility used.

 @retval Status                 EFI_SUCCESS
                                Others      Initialization failed.
**/
VOID
EFIAPI
InitializeVariableEditTable (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{

  EFI_STATUS                                Status;
  UINTN                                     TotalSize = 0;
  UINTN                                     AllocateSize = 0;
  KERNEL_CONFIGURATION                     *SetupVariable = NULL;
  UINTN                                     SetupBufferSize = 0;
  UINT32                                    SetupAttributes = 0;
  PACKAGE_LIST                              PackageListsInfo = {0};
  EFI_SETUP_UTILITY_APPLICATION_PROTOCOL   *SetupUtilityApp = NULL;
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL        *SysPasswordPoint = NULL;


  DEBUG ( (EFI_D_INFO, "[%a] Start\n", __FUNCTION__));

  //
  // Off events to prevent re-entry into the function
  //
  Status = gBS->CloseEvent (Event);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // It was installed at InitializeSetupUtilityFunc() before ReadyToBoot
  //
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtilityApp
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }

  if (OldSysPasswordInstance.GetStatus != NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSysPasswordServiceProtocolGuid,
                    NULL,
                    (VOID **)&SysPasswordPoint
                    );

    if (Status == EFI_SUCCESS) {
      //
      //  Enable System password check.
      //
      SysPasswordPoint->GetStatus = OldSysPasswordInstance.GetStatus;
    }
  }

  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  &SetupAttributes,
                  &SetupBufferSize,
                  NULL
                  );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }

  SetupVariable = (KERNEL_CONFIGURATION *) AllocateZeroPool (SetupBufferSize);
  if (SetupVariable == NULL) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d]\n", __FUNCTION__, __LINE__));
    return;
  }

  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  &SetupAttributes,
                  &SetupBufferSize,
                  SetupVariable
                  );
  if (EFI_ERROR (Status)) {
    FreePool (SetupVariable);
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return;
  }

  //
  //  Get Package Info. Data
  //
  ZeroMem (&PackageListsInfo, sizeof (PACKAGE_LIST));
  Status = InitPackageListInfo (&PackageListsInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    goto InitializeVariableEditTableExit;
  }
  Status = GetRequiredSize (&PackageListsInfo, &TotalSize);
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    goto InitializeVariableEditTableExit;
  }

  //
  //  Allocate Reserved Memory
  //
  Status = AllocateReservedMemory (TotalSize, &AllocateSize);
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    goto InitializeVariableEditTableExit;
  }

  //
  //  Copy String, VfrBin
  //
  Status = FillAllData (&PackageListsInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    goto InitializeVariableEditTableExit;
  }

  RelPackagelistInfo (&PackageListsInfo);

  PasswordStatusUpdate (SetupVariable);

  Status = SetVariableToSensitiveVariable (
             SETUP_VARIABLE_NAME,
             &gSystemConfigurationGuid,
             SetupAttributes,
             SetupBufferSize,
             (VOID *) SetupVariable
             );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    goto InitializeVariableEditTableExit;
  }

  //
  //  Fill FE segment
  //
  Status = GetFEsegmentTableAddress (AllocateSize, &mFEaddress);
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    goto InitializeVariableEditTableExit;
  }

  //
  // Print Information
  //
  DEBUG ( (EFI_D_INFO, "Variable Edit PTR   at 0x%x\n", mFEaddress));
  DEBUG ( (EFI_D_INFO, "Variable Edit TABLE at 0x%x\n", mTable));
  PrintDebug (mFEaddress);
  DEBUG ( (EFI_D_INFO, "[%a] End\n", __FUNCTION__));

  return;

InitializeVariableEditTableExit:

  if (SetupVariable != NULL) {
    FreePool(SetupVariable);
  }

  RelPackagelistInfo (&PackageListsInfo);

  FreeTable();

  return;

}

/**
  Unload function of Content driver

  Disconnect the driver specified by ImageHandle from all the devices in the handle database.
  Uninstall all the protocols installed in the driver entry point.

  @param[in]  ImageHandle       The drivers' driver image.

  @retval EFI_SUCCESS           The image is unloaded.
  @retval Others                Failed to unload the image.

**/
EFI_STATUS
EFIAPI
VariableEditDriverUnload (
  IN  EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS           Status = EFI_SUCCESS;
  VARIABLE_EDIT_PTR   *Point  = NULL;

  FreeTable();
  Point = (VARIABLE_EDIT_PTR *) mFEaddress;
  Point->Signature  = 0;
  Point->Address    = 0;
  Point->Size       = 0;
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ImageHandle,
                  &mVariableEditIdentifyGuid,
                  NULL,
                  NULL
                  );

  return Status;
}

/**

  Decode the supervisor password or user password.

**/
VOID
DecodePassword (
  IN PASSWORD_INFO                             *PasswordInfo,
  IN PASSWORD_TYPE                             PasswordType,
  OUT CHAR16                                   **OutputPassword
)
{
  UINT8                      Index;
  CHAR16                     *Password;

  switch(PasswordType) {

  case SystemSupervisor:
    Password = AllocatePool (sizeof(CHAR16) * (PasswordInfo->SupervisorPwLength + 1));
    if (Password == NULL) {
      DEBUG ( (EFI_D_INFO, "Fail to allocate memory.\n"));
      return;
    }
    for (Index = 0; Index < PasswordInfo->SupervisorPwLength; Index++) {
      Password[Index] = PasswordInfo->SupervisorPw[Index] + (PasswordInfo->SupervisorPwLength - Index);
    }
    Password[Index] = 0;
    *OutputPassword = Password;
    break;

  case SystemUser:
    Password = AllocatePool (sizeof(CHAR16) * (PasswordInfo->UserPwLength + 1));
    if (Password == NULL) {
      DEBUG ( (EFI_D_INFO, "Fail to allocate memory.\n"));
      return;
    }
    for (Index = 0; Index < PasswordInfo->UserPwLength; Index++) {
      Password[Index] = PasswordInfo->UserPw[Index] + (PasswordInfo->UserPwLength - Index);
    }
    Password[Index] = 0;
    *OutputPassword = Password;
    break;

  default:
    return;
  }

}

/**

  Set defult password by specified default variable.

**/
VOID
UveSetPassword (
  VOID
  )
{
  EFI_STATUS                                Status;
  VARIABLE_EDIT_FUNCTION_PASSWORD_SETTING   *VarEditFunPw = NULL;
  UINTN                                     VarEditFunPwSize;
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL         *SysPasswordService;
  CHAR16                                    *SupervisorPw = NULL;
  CHAR16                                    *UserPw = NULL;
  UINT8                                     TempCheckSum;         

  DEBUG ( (EFI_D_INFO, "->SetDefaultPassword()\n"));

  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&SysPasswordService
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_INFO, "System password service protocal is not found.\n"));
    return;
  }

  Status = GetVariable2 (
             H2OUVE_VARSTORE_PW_NAME,
             &gH2oUveVarstoreGuid,
             &VarEditFunPw,
             &VarEditFunPwSize
             );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_INFO, "Default password info is not found.\n"));
    return;
  }

  if (VarEditFunPw->IsDone) {
    DEBUG ( (EFI_D_INFO, "The password info has been done.\n"));
    goto Done;
  }

  //
  // Check the CheckSum
  //
  TempCheckSum = VarEditFunPw->PasswordInfo.CheckSum;
  VarEditFunPw->PasswordInfo.CheckSum = 0;
  VarEditFunPw->PasswordInfo.CheckSum = CalculateCheckSum8 ((UINT8 *)&VarEditFunPw->PasswordInfo, sizeof(PASSWORD_INFO));
  if (VarEditFunPw->PasswordInfo.CheckSum != TempCheckSum) {
    //
    // Restore CheckSum
    //
    VarEditFunPw->PasswordInfo.CheckSum = TempCheckSum;
    DEBUG ( (EFI_D_INFO, "The CheckSum is not match.\n"));
    goto Done;
  }
  
  //
  //First, try to set default supervisor password.
  //The status of CheckPassword should be EFI_NOT_FOUND.
  //
  do {
    if ((VarEditFunPw->PasswordInfo.SupervisorPwLength > PcdGet32 (PcdDefaultSysPasswordMaxLength)) || 
        (VarEditFunPw->PasswordInfo.SupervisorPwLength < PcdGet32 (PcdDefaultSysPasswordMinLength))) {
      DEBUG ( (EFI_D_ERROR, "Default supervisor password length is out of range.\n"));
      //
      //Using the mechanism of break to skip the code below.
      //
      break;
    }
    
    Status = SysPasswordService->GetStatus (SysPasswordService, SystemSupervisor);
    if (Status != EFI_NOT_FOUND) {
      DEBUG ( (EFI_D_INFO, "The supervisor password already exists.\n"));
      //
      //Using the mechanism of break to skip the code below.
      //
      break;
    }

    DecodePassword(&VarEditFunPw->PasswordInfo, SystemSupervisor, &SupervisorPw);
    if (SupervisorPw == NULL) {
      DEBUG ( (EFI_D_INFO, "The supervisor password is NULL.\n"));
      //
      //Using the mechanism of break to skip the code below.
      //
      break;
    }
    if (PcdGetBool (PcdSecureSysPasswordSupported)) {
      SysPasswordService->UnlockPassword (SysPasswordService, L"", 0);
      Status = SysPasswordService->SetPassword (SysPasswordService, SupervisorPw, VarEditFunPw->PasswordInfo.SupervisorPwLength, SystemSupervisor);
      SysPasswordService->LockPassword (SysPasswordService);
    } else {
      Status = SysPasswordService->SetPassword (SysPasswordService, SupervisorPw, VarEditFunPw->PasswordInfo.SupervisorPwLength, SystemSupervisor);
    }
    DEBUG ( (EFI_D_INFO, "SetPassword[Supervisor] Status:%r\n", Status));
    FreePool (SupervisorPw);
  } while(0);

  //
  //Second, try to set default user password.
  //The status of CheckPassword should be EFI_NOT_FOUND.
  //
  if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
    do {
      if ((VarEditFunPw->PasswordInfo.UserPwLength > PcdGet32 (PcdDefaultSysPasswordMaxLength)) || 
          (VarEditFunPw->PasswordInfo.UserPwLength < PcdGet32 (PcdDefaultSysPasswordMinLength))) {
        DEBUG ( (EFI_D_ERROR, "Default supervisor password length is out of range.\n"));
        //
        //Using the mechanism of break to skip the code below.
        //
        break;
      }
      
      Status = SysPasswordService->GetStatus (SysPasswordService, SystemUser);
      if (Status != EFI_NOT_FOUND) {
        DEBUG ( (EFI_D_INFO, "The user password already exists.\n"));
        //
        //Using the mechanism of break to skip the code below.
        //
        break;
      }

      DecodePassword(&VarEditFunPw->PasswordInfo, SystemUser, &UserPw);
      if (UserPw == NULL) {
        DEBUG ( (EFI_D_INFO, "The user password is NULL.\n"));
        //
        //Using the mechanism of break to skip the code below.
        //
        break;
      }
 	    if (PcdGetBool (PcdSecureSysPasswordSupported)) {
 	      SysPasswordService->UnlockPassword (SysPasswordService, L"", 0);
 	      Status = SysPasswordService->SetPassword (SysPasswordService, UserPw, VarEditFunPw->PasswordInfo.UserPwLength, SystemUser);
 	      SysPasswordService->LockPassword (SysPasswordService);
 	    } else {
 	      Status = SysPasswordService->SetPassword (SysPasswordService, UserPw, VarEditFunPw->PasswordInfo.UserPwLength, SystemUser);
 	    }
 	    DEBUG ( (EFI_D_INFO, "SetPassword[User] Status:%r\n", Status));
      FreePool (UserPw);
    } while(0);
  }
  
Done:

  if (!VarEditFunPw->IsDone) {
    VarEditFunPw->IsDone = TRUE;
    Status = gRT->SetVariable (
                   H2OUVE_VARSTORE_PW_NAME,
                   &gH2oUveVarstoreGuid,
                   EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                   VarEditFunPwSize,
                   VarEditFunPw
                   );
  }

  if (VarEditFunPw) {
    FreePool (VarEditFunPw);
    VarEditFunPw = NULL;
  }

  DEBUG ( (EFI_D_INFO, "<-SetDefaultPassword()\n"));

  return;
}

VOID
EFIAPI
AutoSetPassword (
  IN  EFI_EVENT   Event,
  IN  VOID       *Context
  )
{
  UveSetPassword();
}
/**
  Check status of function disabled variable and initialize Variable edit table.

 @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval Status                 EFI_SUCCESS

**/
EFI_STATUS
EFIAPI
VariableEditDriverEntry (
  IN      EFI_HANDLE          ImageHandle,
  IN      EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS                   Status;
  VOID                        *Interface = NULL;
  VARIABLE_EDIT_FUNCTION_DISABLE_SETTING    VarEditFunDis = {0};
  UINTN                        VarEditFunDisSize = sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING);
  H2O_BDS_CP_HANDLE            CpHandle;
  EFI_EVENT                    Event;
  VOID                        *Registration;

  DEBUG ( (EFI_D_INFO, "[%a] Start\n", __FUNCTION__));
  Status = gRT->GetVariable (
                  H2OUVE_VARSTORE_NAME,
                  &gH2oUveVarstoreGuid,
                  NULL,
                  &VarEditFunDisSize,
                  &VarEditFunDis
                  );

  if ( VarEditFunDisSize != sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING)) {
    DEBUG ( (EFI_D_INFO, "Please check sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING) in Variable Store."));
  }
  if ( (!EFI_ERROR (Status)) &&
       (VarEditFunDis.VariableEditFunDis == TRUE) &&
       (VarEditFunDisSize == sizeof (VARIABLE_EDIT_FUNCTION_DISABLE_SETTING))
     ) {
    DEBUG ( (EFI_D_INFO, "[%a:%d] VariableEditor Function Disabled.\n", __FUNCTION__, __LINE__));
    return EFI_UNSUPPORTED;
  }

  //
  // Check if the custom-define protocol is installed or not
  //
  Status = gBS->LocateProtocol (
                  &mVariableEditIdentifyGuid,
                  NULL,
                  (VOID **)&Interface
                  );
  if (!EFI_ERROR (Status)) {
    return EFI_ALREADY_STARTED;
  }

  //
  // Create "ReadyToBoot" Event
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             InitializeVariableEditTable,
             NULL,
             &Event
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpReadyToBootBeforeProtocolGuid,
             InitializeSetupUtilityFunc,
             H2O_BDS_CP_MEDIUM,
             &CpHandle
             );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpReadyToBootAfterProtocolGuid,
             ShutdownSetupUtilityFunc,
             H2O_BDS_CP_MEDIUM,
             &CpHandle
             );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpReadyToBootAfterProtocolGuid,
             UninstallSetupUtilityFunc,
             H2O_BDS_CP_LOW,
             &CpHandle
             );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }

  //
  // Set password automatically.
  //
  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&Interface
                  );
  if (EFI_ERROR (Status)) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    AutoSetPassword,
                    NULL,
                    &Event
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gEfiSysPasswordServiceProtocolGuid,
                      Event,
                      &Registration
                      );
    }
  } else {
    UveSetPassword();
  }

  //
  // Install custom protocol
  //
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &mVariableEditIdentifyGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ( (EFI_D_ERROR, "[%a:%d] Status:%r\n", __FUNCTION__, __LINE__, Status));
    return Status;
  }

  DEBUG ( (EFI_D_INFO, "[%a] End\n", __FUNCTION__));

  return EFI_SUCCESS;
}

