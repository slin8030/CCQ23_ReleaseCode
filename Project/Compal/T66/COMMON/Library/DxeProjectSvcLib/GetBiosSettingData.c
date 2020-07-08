/** @file
  Get Bios setting data by Signature(GUID :gEfiBIOSSettingGuid; Variable Name:L"BiosSetting").

;******************************************************************************
;* Copyright (c) 2016, Compal Electronics, Inc.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Library/DxeProjectSvcLib.h"
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include "ProjectDefinition.h"
#include <Library/UefiRuntimeServicesTableLib.h>

/**
  Get Bios setting data by Signature(GUID :gEfiBIOSSettingGuid; Variable Name:L"BiosSetting").

  @param[In Out] BiosSetting      Return BiosSetting data point
  @param[In]     Signature        Search by Signature

  @return EFI_INVALID_PARAMETER   BiosSetting is NULL
  @return EFI_OUT_OF_RESOURCES    Allocate memory fail.
  @return EFI_NOT_FOUND           Not found signature data.
  @return EFI_SUCCESS             Function execute successful.
**/
EFI_STATUS
GetBiosSettingData (
  IN OUT BIOS_SETTING_STRUCT      **BiosSetting,
  IN UINT32                         Signature
  )
{
  EFI_STATUS                Status;
  BIOS_SETTING_STRUCT       *HeaderStru;
  BIOS_SETTING_STRUCT       *BiosSettingTitle;
  UINTN                     BufferSize;
  UINTN                     EndAddress;
  /***********************************************
  BiosSetting Structure Architecture:

  Signature (BIOS_SETTING_SIGNATURE_$BST)
   DataSize (Total DataSize)
   Data---Signature                               <-----
           DataSize                                     :
           Data---                                      :
                |                                       :
                | DataSize                              :
                |                                       :
               --- Signature                            :
                    DataSize
                    Data---                        (Total DataSize)
                         |
                         | DataSize                     :
                         |                              :
                        --- Signature                   :
                             DataSize                   :
                             Data---                    :
                                  |                     :
                                  | DataSize            :
                                  |                     :
                                 ---                    :
                                  .                     :
                                  .                     :
                                 ....             <-----
  **************************************************/
  //
  // BiosSetting is NULL , return EFI_INVALID_PARAMETER
  //
  if (BiosSetting == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  BufferSize = 0;
  Status = gRT->GetVariable (
                  BIOS_SETTING_VARIABLE_NAME,
                  &gEfiBIOSSettingGuid,
                  NULL,
                  &BufferSize,
                  NULL
                  );
  //
  // Not found Variable , return EFI_NOT_FOUND
  //
  if (Status != EFI_BUFFER_TOO_SMALL) {
    Status = EFI_NOT_FOUND;
    goto Exit;
  }

  //
  // Allocate memory fail., return EFI_OUT_OF_RESOURCES
  //
  BiosSettingTitle = NULL;
  BiosSettingTitle = AllocatePool (BufferSize);
  if (BiosSettingTitle == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gRT->GetVariable (
                BIOS_SETTING_VARIABLE_NAME,
                &gEfiBIOSSettingGuid,
                NULL,
                &BufferSize,
                (VOID *)BiosSettingTitle
                );
  //
  // Not found Variable , return EFI_NOT_FOUND
  //
  if (EFI_ERROR(Status) || (BiosSettingTitle->Header.Signature != BIOS_SETTING_SIGNATURE)) {
    FreePool(BiosSettingTitle);
    Status = EFI_NOT_FOUND;
    goto Exit;
  }

  //
  // Search Signature is title BIOS_SETTING_SIGNATURE, return title structure directly.
  //
  if (Signature == BIOS_SETTING_SIGNATURE) {
    *BiosSetting = BiosSettingTitle;
    Status = EFI_SUCCESS;
    goto Exit;
  }

  //
  // Search sub BiosSetting by Signature
  //
  Status = EFI_NOT_FOUND;
  EndAddress = (UINTN)((UINT8*)&BiosSettingTitle->Data + BiosSettingTitle->Header.DataSize);
  HeaderStru =(BIOS_SETTING_STRUCT *)&BiosSettingTitle->Data;
  while ((UINTN)HeaderStru < EndAddress) {
    if (HeaderStru->Header.Signature == Signature) {
      *BiosSetting = AllocateCopyPool(sizeof(BIOS_SETTING_STRUCT) + HeaderStru->Header.DataSize, HeaderStru);
      Status = EFI_SUCCESS;
      break;
    }
    HeaderStru = (BIOS_SETTING_STRUCT *)(HeaderStru->Data + HeaderStru->Header.DataSize);
  }

  FreePool(BiosSettingTitle);

Exit:
  return Status;
}



