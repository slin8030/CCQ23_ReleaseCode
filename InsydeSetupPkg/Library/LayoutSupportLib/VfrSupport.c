/** @file

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "LayoutSupportLib.h"

EFI_STATUS
GetImportInfoList (
  IN  EFI_GUID                                *FormsetGuid,
  OUT IMPORT_INFO                             **ImportInfoList,
  OUT UINT16                                  *ImportInfoCount
  )
{
  EFI_STATUS                                   Status;
  LIST_ENTRY                                   *VfrLink;
  LIST_ENTRY                                   *FormSetLink;
  LIST_ENTRY                                   *FormLink;
  LIST_ENTRY                                   *ImportLink;


  H2O_VFR_INFO                                 *VfrInfo;
  H2O_FORMSET_INFO                             *FormSetInfo;
  H2O_FORM_INFO                                *FormInfo;
  H2O_IMPORT_INFO                              *ImportInfo;


  IMPORT_INFO                                  *ImportList;
  UINT16                                       ImportCount;

  LIST_ENTRY                                   *LayoutListHead;
  LIST_ENTRY                                   *VfrListHead;

  Status = GetLayoutTreeAndVfrTree (&LayoutListHead, &VfrListHead);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status) || VfrListHead == NULL) {
    return EFI_NOT_FOUND;
  }

  ASSERT (FormsetGuid != NULL);
  ASSERT (ImportInfoCount != NULL);
  if (FormsetGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ImportCount = 0;
  VfrLink = GetFirstNode (VfrListHead);
  while (!IsNull (VfrListHead, VfrLink)) {
    VfrInfo = H2O_VFR_INFO_NODE_FROM_LINK (VfrLink);
    FormSetLink = GetFirstNode ( &VfrInfo->FormsetListHead);
    while (!IsNull (&VfrInfo->FormsetListHead, FormSetLink)) {
      FormSetInfo = H2O_FORMSET_INFO_NODE_FROM_LINK (FormSetLink);
      if (CompareGuid (&FormSetInfo->FormsetGuid, FormsetGuid)) {
        FormLink = GetFirstNode (&FormSetInfo->FormListHead);
        while (!IsNull (&FormSetInfo->FormListHead, FormLink)) {
          FormInfo = H2O_FORM_INFO_NODE_FROM_LINK (FormLink);
          ImportLink = GetFirstNode (&FormInfo->ImportListHead);
          while (!IsNull (&FormInfo->ImportListHead, ImportLink)) {
            ImportCount++;
            ImportLink = GetNextNode (&FormInfo->ImportListHead, ImportLink);
          }
          FormLink = GetNextNode (&FormSetInfo->FormListHead, FormLink);
        }
      }
      FormSetLink = GetNextNode (&VfrInfo->FormsetListHead, FormSetLink);
    }
    VfrLink = GetNextNode (VfrListHead, VfrLink);
  }

  if (ImportCount == 0) {
    return EFI_NOT_FOUND;
  }

  ImportList = AllocateZeroPool (ImportCount * sizeof (IMPORT_INFO));
  if (ImportList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *ImportInfoList  = ImportList;
  *ImportInfoCount = ImportCount;

  VfrLink = GetFirstNode (VfrListHead);
  while (!IsNull (VfrListHead, VfrLink)) {
    VfrInfo = H2O_VFR_INFO_NODE_FROM_LINK (VfrLink);
    VfrLink = GetNextNode (VfrListHead, VfrLink);

    FormSetLink = GetFirstNode (&VfrInfo->FormsetListHead);
    while (!IsNull (&VfrInfo->FormsetListHead, FormSetLink)) {
      FormSetInfo = H2O_FORMSET_INFO_NODE_FROM_LINK (FormSetLink);
      FormSetLink = GetNextNode (&VfrInfo->FormsetListHead, FormSetLink);

      if (CompareGuid (&FormSetInfo->FormsetGuid, FormsetGuid)) {
        FormLink = GetFirstNode (&FormSetInfo->FormListHead);

        while (!IsNull (&FormSetInfo->FormListHead, FormLink)) {
          FormInfo = H2O_FORM_INFO_NODE_FROM_LINK (FormLink);
          FormLink = GetNextNode (&FormSetInfo->FormListHead, FormLink);

          ImportLink = GetFirstNode (&FormInfo->ImportListHead);
          while (!IsNull (&FormInfo->ImportListHead, ImportLink)) {
            ImportInfo = H2O_IMPORT_INFO_NODE_FROM_LINK (ImportLink);

            ImportList->Flags     = ImportInfo->Flags;
            CopyGuid (&ImportList->SrcFormSetGuid, &ImportInfo->TargetFormsetGuid);
            ImportList->SrcFormId = ImportInfo->TargetFormId;
            ImportList->SrcId     = ImportInfo->TargetId;
            ImportList->DstFormId = (UINT16)FormInfo->FormId;
            ImportList->DstId     = ImportInfo->LocalId;
            ImportList++;

            ImportLink = GetNextNode (&FormInfo->ImportListHead, ImportLink);
          }
        }
      }
    }

  }

  return EFI_SUCCESS;
}

UINT32
GetStyleTypeByOpCode (
  IN    UINT8                                  OpCode
  )
{
  switch (OpCode) {

  case EFI_IFR_SUBTITLE_OP:
    return H2O_IFR_STYLE_TYPE_SUBTITLE;

  case EFI_IFR_CHECKBOX_OP:
    return H2O_IFR_STYLE_TYPE_CHECKBOX;

  case EFI_IFR_TEXT_OP:
    return H2O_IFR_STYLE_TYPE_TEXT;

  case EFI_IFR_ACTION_OP:
    return H2O_IFR_STYLE_TYPE_ACTION;

  case EFI_IFR_REF_OP:
    return H2O_IFR_STYLE_TYPE_GOTO;

  case EFI_IFR_PASSWORD_OP:
    return H2O_IFR_STYLE_TYPE_PASSWORD;

  case EFI_IFR_NUMERIC_OP:
    return H2O_IFR_STYLE_TYPE_NUMERIC;

  case EFI_IFR_ONE_OF_OP:
    return H2O_IFR_STYLE_TYPE_ONEOF;

  case EFI_IFR_TIME_OP:
    return H2O_IFR_STYLE_TYPE_TIME;

  case EFI_IFR_DATE_OP:
    return H2O_IFR_STYLE_TYPE_DATE;

  case EFI_IFR_ORDERED_LIST_OP:
    return H2O_IFR_STYLE_TYPE_ORDEREDLIST;

  case EFI_IFR_RESET_BUTTON_OP:
    return H2O_IFR_STYLE_TYPE_RESETBUTTON;

  case EFI_IFR_STRING_OP:
    return H2O_IFR_STYLE_TYPE_STRING;

  default:
    break;
  }

  ASSERT (FALSE);
  return H2O_IFR_STYLE_TYPE_SHEET;
}

