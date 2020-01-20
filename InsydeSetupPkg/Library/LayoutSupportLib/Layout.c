/** @file

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

#include "LayoutSupportLib.h"

STATIC
EFI_STATUS
ParsePseudoClass (
  IN CONST CHAR8                                 *PseudoClassStr,
  IN OUT   UINT32                                *PseudoClass
  )
{
  if (PseudoClass == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (AsciiStrCmp (PseudoClassStr, ":selectable") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_SELECTABLE;
  } else if (AsciiStrCmp (PseudoClassStr, ":grayout") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_GRAYOUT;
  } else if (AsciiStrCmp (PseudoClassStr, ":highlight") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_HIGHLIGHT;
  } else if (AsciiStrCmp (PseudoClassStr, ":link") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_LINK;
  } else if (AsciiStrCmp (PseudoClassStr, ":visited") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_VISITED;
  } else if (AsciiStrCmp (PseudoClassStr, ":hover") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_HOVER;
  } else if (AsciiStrCmp (PseudoClassStr, ":focus") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_FOCUS;
  } else if (AsciiStrCmp (PseudoClassStr, ":active") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_ACTIVE;
  } else if (AsciiStrCmp (PseudoClassStr, ":enabled") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_ENABLED;
  } else if (AsciiStrCmp (PseudoClassStr, ":disabled") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_DISABLED;
  } else if (AsciiStrCmp (PseudoClassStr, ":not") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_NOT;
  } else if (AsciiStrCmp (PseudoClassStr, ":help") == 0) {
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_HELP;
  } else {
    //
    // default
    //
    *PseudoClass = H2O_STYLE_PSEUDO_CLASS_NORMAL;
  }

  return EFI_SUCCESS;
}

STATIC
VOID
CountLayoutOpCodes (
  IN  UINT8                                    *BlockDataBuffer,
  IN  UINT32                                   BlockDataBufferLength,
  OUT UINT32                                   *NumberOfPanel,
  OUT UINT32                                   *NumberOfStyle,
  OUT UINT32                                   *NumberOfProperty,
  OUT UINT32                                   *NumberOfHotKey
  )
{
  UINT32                                       BlockOffset;
  UINT8                                        *BlockData;
  UINT8                                        BlockType;
  UINT8                                        BlockSize;
  UINT32                                       PanelNum;
  UINT32                                       StyleNum;
  UINT32                                       PropertyNum;
  UINT32                                       HotKeyNum;

  PanelNum    = 0;
  StyleNum    = 0;
  PropertyNum = 0;
  HotKeyNum   = 0;

  BlockOffset = 0;
  while (BlockOffset < BlockDataBufferLength) {
    BlockData = BlockDataBuffer + BlockOffset;

    BlockType = ((H2O_HII_LAYOUT_BLOCK *) BlockData)->BlockType;
    BlockSize = ((H2O_HII_LAYOUT_BLOCK *) BlockData)->BlockSize;
    if (BlockSize == 0) {
      break;
    }

    BlockOffset += BlockSize;

    switch (BlockType) {

    case H2O_HII_LIBT_PANEL_BEGIN:
      PanelNum++;
      break;

    case H2O_HII_LIBT_STYLE_BEGIN:
      StyleNum++;
      break;

    case H2O_HII_LIBT_PROPERTY:
      StyleNum++;
      PropertyNum++;
      break;

    case H2O_HII_LIBT_EXT_HOTKEY:
      HotKeyNum++;
      break;
    }
  }

  *NumberOfPanel    = PanelNum;
  *NumberOfStyle    = StyleNum;
  *NumberOfProperty = PropertyNum;
  *NumberOfHotKey   = HotKeyNum;
}

STATIC
EFI_STATUS
ParseLayoutPkg (
  IN     UINT8                                 *PkgBuffer,
  OUT    LIST_ENTRY                            **LayoutListHead,
  OUT    LIST_ENTRY                            **VfrListHead
  )
{
  EFI_STATUS                                   Status;

  H2O_LAYOUT_PACKAGE_HDR                       *CurrentLayoutPkg;
  H2O_LAYOUT_INFO                              *CurrentLayout;
  H2O_PANEL_INFO                               *CurrentPanel;
  H2O_STYLE_INFO                               *CurrentStyle;
  H2O_PROPERTY_INFO                            *CurrentProperty;

  H2O_VFR_INFO                                 *CurrentVfr;
  H2O_FORMSET_INFO                             *CurrentFormset;
  H2O_FORM_INFO                                *CurrentForm;
  H2O_HOTKEY_INFO                              *CurrentHotkey;
  H2O_IMPORT_INFO                              *CurrentImport;
  H2O_STATEMENT_INFO                           *CurrentStatement;

  UINT8                                        *BlockDataBuffer;
  UINT32                                       BlockDataBufferLength;
  UINT8                                        *BlockData;
  UINT8                                        BlockType;
  UINT8                                        BlockSize;
  UINT32                                       BlockOffset;

  H2O_LAYOUT_ID                                OldLayoutId;
  H2O_PANEL_ID                                 OldPanelId;
  LIST_ENTRY                                   *Link;
  H2O_LAYOUT_INFO                              *TempLayout;
  H2O_PANEL_INFO                               *TempPanel;
  CHAR8                                        *PseudoClassStr;

  H2O_HII_LIBT_EXT_IMPORT_BLOCK                *ImportBlock;
  H2O_HII_LIBT_EXT_FORMSET_BLOCK               *FormSetBlock;
  H2O_HII_LIBT_LAYOUT_BEGIN_BLOCK              *LayoutBeginBlock;
  H2O_HII_LIBT_EXT_STATEMENT_BLOCK             *StatementBlock;

  UINT32                                       PanelIndex;
  UINT32                                       PanelCount;
  H2O_PANEL_INFO                               *PanelBuffer;
  UINT32                                       StyleIndex;
  UINT32                                       StyleCount;
  H2O_STYLE_INFO                               *StyleBuffer;
  UINT32                                       PropertyIndex;
  UINT32                                       PropertyCount;
  H2O_PROPERTY_INFO                            *PropertyBuffer;
  UINT32                                       HotKeyIndex;
  UINT32                                       HotKeyCount;
  H2O_HOTKEY_INFO                              *HotKeyBuffer;

  if (PkgBuffer == NULL || LayoutListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;

  CurrentLayoutPkg = NULL;
  CurrentLayout    = NULL;
  CurrentPanel     = NULL;
  CurrentStyle     = NULL;
  CurrentProperty  = NULL;
  CurrentVfr       = NULL;
  CurrentFormset   = NULL;
  CurrentForm      = NULL;
  CurrentHotkey    = NULL;
  CurrentStatement = NULL;

  TempLayout = NULL;
  TempPanel  = NULL;

  //
  // Layout Pkg
  //
  CurrentLayoutPkg = (H2O_LAYOUT_PACKAGE_HDR *) (PkgBuffer + sizeof(UINT32));
  if (CurrentLayoutPkg->Header.Type != H2O_HII_PACKAGE_LAYOUTS) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Block Data Buffer
  //
  BlockDataBuffer       = (UINT8 *)(CurrentLayoutPkg + 1);
  BlockDataBufferLength = CurrentLayoutPkg->Header.Length - CurrentLayoutPkg->HdrSize;
  CountLayoutOpCodes (BlockDataBuffer, BlockDataBufferLength, &PanelCount, &StyleCount, &PropertyCount, &HotKeyCount);
  PanelBuffer    = AllocateZeroPool (sizeof (H2O_PANEL_INFO)    * PanelCount);
  StyleBuffer    = AllocateZeroPool (sizeof (H2O_STYLE_INFO)    * StyleCount);
  PropertyBuffer = AllocateZeroPool (sizeof (H2O_PROPERTY_INFO) * PropertyCount);
  HotKeyBuffer   = AllocateZeroPool (sizeof (H2O_HOTKEY_INFO)   * HotKeyCount);
  if ((PanelBuffer    == NULL && PanelCount    != 0) ||
      (StyleBuffer    == NULL && StyleCount    != 0) ||
      (PropertyBuffer == NULL && PropertyCount != 0) ||
      (HotKeyBuffer   == NULL && HotKeyCount   != 0)) {
    return EFI_OUT_OF_RESOURCES;
  }
  PanelIndex    = 0;
  StyleIndex    = 0;
  PropertyIndex = 0;
  HotKeyIndex   = 0;

  //
  // Init Layout List
  //
  *LayoutListHead = AllocateZeroPool (sizeof (LIST_ENTRY));
  if (*LayoutListHead == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  InitializeListHead (*LayoutListHead);

  //
  // Init Vfr List
  //
  *VfrListHead = AllocateZeroPool (sizeof (LIST_ENTRY));
  if (*VfrListHead == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  InitializeListHead (*VfrListHead);

  //
  // Parse each layout block
  //
  BlockOffset = 0;
  while (BlockOffset < BlockDataBufferLength) {
    BlockData = BlockDataBuffer + BlockOffset;

    BlockType = ((H2O_HII_LAYOUT_BLOCK *) BlockData)->BlockType;
    BlockSize = ((H2O_HII_LAYOUT_BLOCK *) BlockData)->BlockSize;

    if (BlockSize == 0) {
      break;
    }

    BlockOffset += BlockSize;

    switch (BlockType) {

    case H2O_HII_LIBT_LAYOUT_BEGIN:
      CurrentLayout = AllocateZeroPool (sizeof (H2O_LAYOUT_INFO));
      if (CurrentLayout == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      LayoutBeginBlock = (H2O_HII_LIBT_LAYOUT_BEGIN_BLOCK *)BlockData;

      CurrentLayout->Signature = H2O_LAYOUT_INFO_NODE_SIGNATURE;
      CurrentLayout->LayoutId = LayoutBeginBlock->LayoutId;
      CopyGuid (&CurrentLayout->DisplayEngineGuid[0], &LayoutBeginBlock->DisplayEngineGuid[0]);
      CopyGuid (&CurrentLayout->DisplayEngineGuid[1], &LayoutBeginBlock->DisplayEngineGuid[1]);
      CopyGuid (&CurrentLayout->DisplayEngineGuid[2], &LayoutBeginBlock->DisplayEngineGuid[2]);
      InitializeListHead (&CurrentLayout->PanelListHead);
      InitializeListHead (&CurrentLayout->HotkeyListHead);
      InitializeListHead (&CurrentLayout->StyleListHead);
      InsertTailList (*LayoutListHead, &CurrentLayout->Link);
      break;

    case H2O_HII_LIBT_LAYOUT_END:
      CurrentLayout = NULL;
      CurrentPanel  = NULL;
      CurrentHotkey = NULL;
      CurrentStyle  = NULL;
      break;

    case H2O_HII_LIBT_PANEL_BEGIN:
      if (CurrentLayout == NULL || PanelBuffer == NULL) {
        ASSERT (FALSE);
        CurrentPanel = NULL;
        break;
      }
      CurrentPanel = &PanelBuffer[PanelIndex++];
      CurrentPanel->Signature = H2O_PANEL_INFO_NODE_SIGNATURE;
      CurrentPanel->PanelId = ((H2O_HII_LIBT_PANEL_BEGIN_BLOCK *)BlockData)->PanelId;
      CurrentPanel->PanelType = ((H2O_HII_LIBT_PANEL_BEGIN_BLOCK *)BlockData)->PanelType;
      CopyMem (&CurrentPanel->PanelGuid, &((H2O_HII_LIBT_PANEL_BEGIN_BLOCK *)BlockData)->PanelGuid, sizeof (EFI_GUID));
      CurrentPanel->ParentLayout = CurrentLayout;
      InitializeListHead (&CurrentPanel->StyleListHead);
      InsertTailList (&CurrentLayout->PanelListHead, &CurrentPanel->Link);
      break;

    case H2O_HII_LIBT_PANEL_END:
      CurrentPanel = NULL;
      CurrentStyle = NULL;
      break;

    case H2O_HII_LIBT_STYLE_BEGIN:
      if ((CurrentPanel == NULL &&
           CurrentLayout == NULL &&
           CurrentHotkey == NULL &&
           CurrentStatement == NULL &&
           CurrentForm == NULL &&
           CurrentFormset == NULL) ||
          StyleBuffer == NULL) {
        ASSERT (FALSE);
        CurrentStyle = NULL;
        break;
      }
      CurrentStyle = &StyleBuffer[StyleIndex++];
      CurrentStyle->Signature = H2O_STYLE_INFO_NODE_SIGNATURE;
      CurrentStyle->StyleType = ((H2O_HII_LIBT_STYLE_BEGIN_BLOCK *)BlockData)->Type;
      CurrentStyle->ClassName = (CHAR8 *)BlockData + ((H2O_HII_LIBT_STYLE_BEGIN_BLOCK *)BlockData)->ClassNameOffset;
      PseudoClassStr = (CHAR8 *)BlockData + ((H2O_HII_LIBT_STYLE_BEGIN_BLOCK *)BlockData)->PseudoClassOffset;
      ParsePseudoClass (PseudoClassStr, &CurrentStyle->PseudoClass);
      InitializeListHead (&CurrentStyle->PropertyListHead);
      if (CurrentHotkey != NULL) {
        InsertTailList (&CurrentHotkey->StyleListHead, &CurrentStyle->Link);
      } else if (CurrentStatement != NULL) {
        InsertTailList (&CurrentStatement->StyleListHead, &CurrentStyle->Link);
      } else if (CurrentForm != NULL) {
        InsertTailList (&CurrentForm->StyleListHead, &CurrentStyle->Link);
      } else if (CurrentFormset != NULL) {
        InsertTailList (&CurrentFormset->StyleListHead, &CurrentStyle->Link);
      } else if (CurrentPanel != NULL) {
        InsertTailList (&CurrentPanel->StyleListHead, &CurrentStyle->Link);
      } else if (CurrentLayout != NULL) {
        InsertTailList (&CurrentLayout->StyleListHead, &CurrentStyle->Link);
      }
      break;

    case H2O_HII_LIBT_STYLE_END:
      CurrentStyle = NULL;
      break;

    case H2O_HII_LIBT_PROPERTY:
      if ((CurrentStyle == NULL &&
           CurrentPanel == NULL &&
           CurrentLayout == NULL &&
           CurrentHotkey == NULL &&
           CurrentStatement == NULL &&
           CurrentForm == NULL &&
           CurrentFormset == NULL) ||
          PropertyBuffer == NULL) {
        ASSERT (FALSE);
        CurrentProperty = NULL;
        break;
      }
      if (CurrentStyle == NULL) {
        if (StyleBuffer == NULL) {
          break;
        }
        CurrentStyle = &StyleBuffer[StyleIndex++];
        CurrentStyle->Signature = H2O_STYLE_INFO_NODE_SIGNATURE;
        InitializeListHead (&CurrentStyle->PropertyListHead);
        if (CurrentHotkey != NULL) {
          CurrentStyle->StyleType = H2O_IFR_STYLE_TYPE_HOTKEY;
          InsertTailList (&CurrentHotkey->StyleListHead, &CurrentStyle->Link);
        } else if (CurrentStatement != NULL) {
          CurrentStyle->StyleType = H2O_IFR_STYLE_TYPE_STATEMENT;
          InsertTailList (&CurrentStatement->StyleListHead, &CurrentStyle->Link);
        } else if (CurrentForm != NULL) {
          CurrentStyle->StyleType = H2O_IFR_STYLE_TYPE_FORM;
          InsertTailList (&CurrentForm->StyleListHead, &CurrentStyle->Link);
        } else if (CurrentFormset != NULL) {
          CurrentStyle->StyleType = H2O_IFR_STYLE_TYPE_FORMSET;
          InsertTailList (&CurrentFormset->StyleListHead, &CurrentStyle->Link);
        } else if (CurrentPanel != NULL) {
          CurrentStyle->StyleType = H2O_IFR_STYLE_TYPE_PANEL;
          InsertTailList (&CurrentPanel->StyleListHead, &CurrentStyle->Link);
        } else if (CurrentLayout != NULL) {
          CurrentStyle->StyleType = H2O_IFR_STYLE_TYPE_LAYOUT;
          InsertTailList (&CurrentLayout->StyleListHead, &CurrentStyle->Link);
        }
      }
      CurrentProperty = &PropertyBuffer[PropertyIndex++];
      CurrentProperty->Signature = H2O_PROPERTY_INFO_NODE_SIGNATURE;
      CurrentProperty->ValueType = ((H2O_HII_LIBT_PROPERTY_BLOCK *)BlockData)->ValueType;
      CopyMem (&CurrentProperty->H2OValue, &((H2O_HII_LIBT_PROPERTY_BLOCK *)BlockData)->H2OValue, sizeof (H2O_VALUE));
      CurrentProperty->IdentifierStr = (CHAR8 *)BlockData + ((H2O_HII_LIBT_PROPERTY_BLOCK *)BlockData)->IdentifierOffset;
      CurrentProperty->ValueStr = (CHAR8 *)BlockData + ((H2O_HII_LIBT_PROPERTY_BLOCK *)BlockData)->ValueOffset;
      InsertTailList (&CurrentStyle->PropertyListHead, &CurrentProperty->Link);
      CurrentProperty = NULL;
      break;

    case H2O_HII_LIBT_LAYOUT_DUP:
      OldLayoutId = ((H2O_HII_LIBT_LAYOUT_DUP_BLOCK *)BlockData)->OldLayoutId;
      Link = *LayoutListHead;
      Link = Link->ForwardLink;
      do {
        TempLayout = H2O_LAYOUT_INFO_NODE_FROM_LINK (Link);
        if (OldLayoutId == TempLayout->LayoutId) {
          CurrentLayout = AllocateCopyPool (sizeof (H2O_LAYOUT_INFO), TempLayout);
          if (CurrentLayout == NULL) {
            return EFI_OUT_OF_RESOURCES;
          }
          //
          // New LayoutId
          //
          CurrentLayout->LayoutId = ((H2O_HII_LIBT_LAYOUT_DUP_BLOCK *)BlockData)->LayoutId;
          InitializeListHead (&CurrentLayout->PanelListHead);
          InitializeListHead (&CurrentLayout->HotkeyListHead);
          InsertTailList (*LayoutListHead, &CurrentLayout->Link);
          break;
        }
        Link = Link->ForwardLink;
      } while (IsNodeAtEnd (*LayoutListHead, Link));
      break;

    case H2O_HII_LIBT_PANEL_DUP:
      OldLayoutId = ((H2O_HII_LIBT_PANEL_DUP_BLOCK *)BlockData)->OldLayoutId;
      Link = *LayoutListHead;
      Link = Link->ForwardLink;
      do {
        //
        // Found OldLayoutId
        //
        TempLayout = H2O_LAYOUT_INFO_NODE_FROM_LINK (Link);
        if (OldLayoutId == TempLayout->LayoutId) {

          OldPanelId = ((H2O_HII_LIBT_PANEL_DUP_BLOCK *)BlockData)->OldPanelId;
          Link = &TempLayout->PanelListHead;
          Link = Link->ForwardLink;
          do {
            //
            // Found OldPanelId
            //
            TempPanel = H2O_PANEL_INFO_NODE_FROM_LINK (Link);
            if (OldPanelId == TempPanel->PanelId) {
              CurrentPanel = AllocateCopyPool (sizeof (H2O_PANEL_INFO), TempPanel);
              //
              // New PanelId
              //
              CurrentPanel->PanelId = ((H2O_HII_LIBT_PANEL_DUP_BLOCK *)BlockData)->PanelId;
              InsertTailList (&CurrentLayout->PanelListHead, &CurrentPanel->Link);
              break;
            }
            Link = Link->ForwardLink;
          } while (IsNodeAtEnd (&TempLayout->PanelListHead, Link));

          break;
        }
        Link = Link->ForwardLink;
      } while (IsNodeAtEnd (*LayoutListHead, Link));
      break;

    case H2O_HII_LIBT_EXT2:
      break;

    case H2O_HII_LIBT_EXT4:
      break;

    case H2O_HII_LIBT_EXT_VFR:
      CurrentVfr = AllocateZeroPool (sizeof (H2O_VFR_INFO));
      if (CurrentVfr == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      CurrentVfr->Signature = H2O_VFR_INFO_NODE_SIGNATURE;
      CurrentVfr->VfrId = ((H2O_HII_LIBT_EXT_VFR_BLOCK *)BlockData)->VfrId;
      InitializeListHead (&CurrentVfr->FormsetListHead);
      InsertTailList (*VfrListHead, &CurrentVfr->Link);
      break;

    case H2O_HII_LIBT_EXT_VFR_END:
      CurrentVfr     = NULL;
      CurrentFormset = NULL;
      CurrentForm    = NULL;
      CurrentHotkey  = NULL;
      CurrentStyle   = NULL;
      break;

    case H2O_HII_LIBT_EXT_FORMSET:
      if (CurrentVfr == NULL) {
        ASSERT (FALSE);
        break;
      }
      CurrentFormset = AllocateZeroPool (sizeof (H2O_FORMSET_INFO));
      if (CurrentFormset == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      FormSetBlock = (H2O_HII_LIBT_EXT_FORMSET_BLOCK *)BlockData;

      CurrentFormset->Signature = H2O_FORMSET_INFO_NODE_SIGNATURE;
      CopyMem (&CurrentFormset->FormsetGuid, &FormSetBlock->FormsetGuid, sizeof (EFI_GUID));
      InitializeListHead (&CurrentFormset->FormListHead);
      InitializeListHead (&CurrentFormset->HotkeyListHead);
      InitializeListHead (&CurrentFormset->StyleListHead);
      InsertTailList (&CurrentVfr->FormsetListHead, &CurrentFormset->Link);
      break;

    case H2O_HII_LIBT_EXT_FORMSET_END:
      CurrentFormset = NULL;
      CurrentForm    = NULL;
      CurrentHotkey  = NULL;
      CurrentStyle   = NULL;
      break;

    case H2O_HII_LIBT_EXT_FORM:
      if (CurrentFormset == NULL) {
        ASSERT (FALSE);
        break;
      }
      CurrentForm = AllocateZeroPool (sizeof (H2O_FORM_INFO));
      if (CurrentForm == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      CurrentForm->Signature = H2O_FORM_INFO_NODE_SIGNATURE;
      CurrentForm->FormId = ((H2O_HII_LIBT_EXT_FORM_BLOCK *)BlockData)->FormId;
      InitializeListHead (&CurrentForm->StyleListHead);
      InitializeListHead (&CurrentForm->HotkeyListHead);
      InitializeListHead (&CurrentForm->ImportListHead);
      InitializeListHead (&CurrentForm->StatementListHead);
      InsertTailList (&CurrentFormset->FormListHead, &CurrentForm->Link);
      break;

    case H2O_HII_LIBT_EXT_FORM_END:
      CurrentForm = NULL;
      CurrentHotkey  = NULL;
      CurrentStyle   = NULL;
      break;

    case H2O_HII_LIBT_EXT_HOTKEY:
      if ((CurrentStatement == NULL &&
           CurrentForm == NULL &&
           CurrentFormset == NULL &&
           CurrentLayout == NULL) ||
          HotKeyBuffer == NULL) {
        ASSERT (FALSE);
        break;
      }
      CurrentHotkey = &HotKeyBuffer[HotKeyIndex++];
      CurrentHotkey->Signature = H2O_HOTKEY_INFO_NODE_SIGNATURE;
      CurrentHotkey->HotkeyId = ((H2O_HII_LIBT_EXT_HOTKEY_BLOCK *)BlockData)->HotkeyId;
      InitializeListHead (&CurrentHotkey->StyleListHead);
      if (CurrentStatement != NULL) {
        InsertTailList (&CurrentStatement->HotkeyListHead, &CurrentHotkey->Link);
      } else if (CurrentForm != NULL) {
        InsertTailList (&CurrentForm->HotkeyListHead, &CurrentHotkey->Link);
      } else if (CurrentFormset != NULL) {
        InsertTailList (&CurrentFormset->HotkeyListHead, &CurrentHotkey->Link);
      } else if (CurrentLayout != NULL) {
        InsertTailList (&CurrentLayout->HotkeyListHead, &CurrentHotkey->Link);
      }
      break;

    case H2O_HII_LIBT_EXT_HOTKEY_END:
      CurrentHotkey = NULL;
      CurrentStyle   = NULL;
      break;

    case H2O_HII_LIBT_EXT_IMPORT:
      ASSERT (CurrentForm != NULL);
      if (CurrentForm == NULL) {
        break ;
      }

      CurrentImport = AllocateZeroPool (sizeof (H2O_IMPORT_INFO));
      if (CurrentImport == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      ImportBlock = (H2O_HII_LIBT_EXT_IMPORT_BLOCK *)BlockData;

      CurrentImport->Signature = H2O_IMPORT_INFO_NODE_SIGNATURE;
      CopyGuid (&CurrentImport->TargetFormsetGuid, &ImportBlock->TargetFormsetGuid);
      CurrentImport->Flags        = ImportBlock->Flags;
      CurrentImport->TargetFormId = ImportBlock->TargetFormId;
      CurrentImport->TargetId     = ImportBlock->TargetId;
      CurrentImport->LocalId      = ImportBlock->LocalId;

      InitializeListHead (&CurrentImport->Link);
      InsertTailList (&CurrentForm->ImportListHead, &CurrentImport->Link);
      break;

    case H2O_HII_LIBT_EXT_STATEMENT:
      if (CurrentForm == NULL) {
        ASSERT (FALSE);
        break;
      }
      CurrentStatement = AllocateZeroPool (sizeof (H2O_STATEMENT_INFO));
      if (CurrentStatement == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      StatementBlock = (H2O_HII_LIBT_EXT_STATEMENT_BLOCK *)BlockData;

      CurrentStatement->Signature = H2O_STATEMENT_INFO_NODE_SIGNATURE;
      CurrentStatement->IsQuestion = ((StatementBlock->Flags & H2O_HII_STATEMENT_QUESTION) == H2O_HII_STATEMENT_QUESTION);
      CurrentStatement->QuestionId = StatementBlock->TargetId;

      InitializeListHead (&CurrentStatement->Link);
      InitializeListHead (&CurrentStatement->StyleListHead);
      InitializeListHead (&CurrentStatement->HotkeyListHead);
      InsertTailList (&CurrentForm->StatementListHead, &CurrentStatement->Link);
      break;

    case H2O_HII_LIBT_EXT_STATEMENT_END:
      CurrentStatement = NULL;
      CurrentHotkey    = NULL;
      CurrentStyle     = NULL;
      break;

    default:
      ASSERT (FALSE);
      break;
    }
  }

  return Status;
}

STATIC
EFI_STATUS
GetLayoutPackage (
  IN     EFI_HII_HANDLE                  HiiHandle,
  OUT    UINT8                           **LayoutPackage
  )
{
  EFI_STATUS                             Status;

  EFI_HII_DATABASE_PROTOCOL              *HiiDatabase;
  EFI_HII_PACKAGE_LIST_HEADER            *HiiPackageList;
  EFI_HII_PACKAGE_HEADER                 *Package;
  UINT32                                 PackageLength;
  UINTN                                  BufferSize;
  UINTN                                  Offset;

  if (LayoutPackage == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *LayoutPackage = NULL;

  //
  // Locate HiiDatabase Protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get the HII package list by HiiHandle
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = (EFI_HII_PACKAGE_LIST_HEADER *) AllocatePool (BufferSize);
    if (HiiPackageList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = HiiDatabase->ExportPackageLists (HiiDatabase, HiiHandle, &BufferSize, HiiPackageList);
    if (EFI_ERROR (Status)) {
      SafeFreePool ((VOID **) &HiiPackageList);
      return Status;
    }
  } else {
    return Status;
  }

  //
  // Find layout package by package type
  //
  Status = EFI_NOT_FOUND;
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  PackageLength = HiiPackageList->PackageLength;
  while (Offset < PackageLength) {
    Package = (EFI_HII_PACKAGE_HEADER *) (((UINT8 *) HiiPackageList) + Offset);

    if (Package->Type == H2O_HII_PACKAGE_LAYOUTS) {
      *LayoutPackage = (UINT8 *) AllocatePool (PackageLength);
      if (*LayoutPackage != NULL) {
        CopyMem (*LayoutPackage, Package, PackageLength);
        Status = EFI_SUCCESS;
      } else {
        Status = EFI_BUFFER_TOO_SMALL;
      }
      break;
    }
    Offset += Package->Length;
  }

  if (Offset >= PackageLength) {
    Status = EFI_NOT_FOUND;
  }

  SafeFreePool ((VOID **) &HiiPackageList);

  return Status;
}

EFI_STATUS
GetLayoutTreeAndVfrTree (
  OUT    LIST_ENTRY                              **LayoutListHead,
  OUT    LIST_ENTRY                              **VfrListHead
  )
{
  EFI_STATUS                                     Status;
  LAYOUT_DATABASE_PROTOCOL                       *LayoutDatabase;
  UINT8                                          *PkgBuffer;
  LIST_ENTRY                                     *TempLayoutListHead;
  LIST_ENTRY                                     *TempVfrListHead;

  if (LayoutListHead == NULL || VfrListHead == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *LayoutListHead = NULL;
  *LayoutListHead = NULL;

  Status = gBS->LocateProtocol (&gLayoutDatabaseProtocolGuid, NULL, (VOID **) &LayoutDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TempLayoutListHead = LayoutDatabase->LayoutListHead;
  TempVfrListHead = LayoutDatabase->VfrListHead;
  if (TempLayoutListHead == NULL) {
    //
    // Get layout pkg buffer
    //
    PkgBuffer = NULL;
    Status = GetLayoutPackage (LayoutDatabase->LayoutPkgHiiHandle, &PkgBuffer);
    //
    // BUGBUG: HiiDatabase not support layout pkg yet
    //
    if (EFI_ERROR (Status)) {
      //return Status;
    }
    PkgBuffer = (UINT8 *)(UINTN *)LayoutDatabase->LayoutPkgAddr;
    if (PkgBuffer == NULL) {
      return EFI_NOT_FOUND;
    }

    //
    // Parse layout pkg into layout list
    //
    Status = ParseLayoutPkg(PkgBuffer, &TempLayoutListHead, &TempVfrListHead);
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    LayoutDatabase->LayoutListHead = TempLayoutListHead;
    LayoutDatabase->VfrListHead = TempVfrListHead;
  }

  *LayoutListHead = LayoutDatabase->LayoutListHead;
  *VfrListHead = LayoutDatabase->VfrListHead;

  return EFI_SUCCESS;
}

EFI_STATUS
GetLayoutById (
  IN     UINT32                                  LayoutId,
  IN     EFI_GUID                                *DisplayEngineGuid,
  OUT    H2O_LAYOUT_INFO                         **Layout
  )
{
  EFI_STATUS                                     Status;
  LIST_ENTRY                                     *LayoutList;
  LIST_ENTRY                                     *VfrListHead;
  LIST_ENTRY                                     *LayoutLink;
  H2O_LAYOUT_INFO                                *LayoutInfo;

  ASSERT (Layout != NULL);
  ASSERT (DisplayEngineGuid != NULL);

  Status = GetLayoutTreeAndVfrTree (&LayoutList, &VfrListHead);
  if (EFI_ERROR (Status) || LayoutList == NULL) {
    return Status;
  }

  //
  // Get Layout
  //
  LayoutLink = GetFirstNode (LayoutList);
  while (!IsNull (LayoutList, LayoutLink)) {
    LayoutInfo = H2O_LAYOUT_INFO_NODE_FROM_LINK (LayoutLink);
    if ((LayoutInfo->LayoutId == LayoutId) &&
        ((CompareGuid(&LayoutInfo->DisplayEngineGuid[0], DisplayEngineGuid)) ||
         (CompareGuid(&LayoutInfo->DisplayEngineGuid[1], DisplayEngineGuid)) ||
         (CompareGuid(&LayoutInfo->DisplayEngineGuid[2], DisplayEngineGuid)))) {
      *Layout = LayoutInfo;
      return EFI_SUCCESS;
    }
    LayoutLink = GetNextNode (LayoutList, LayoutLink);
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
GetFormsetLayoutByGuid (
  IN     EFI_GUID                                *FormsetGuid,
  OUT    H2O_FORMSET_INFO                        **Formset
  )
{
  EFI_STATUS                                     Status;
  LIST_ENTRY                                     *LayoutListHead;
  LIST_ENTRY                                     *VfrListHead;
  LIST_ENTRY                                     *FormsetListHead;
  LIST_ENTRY                                     *VfrLink;
  LIST_ENTRY                                     *FormsetLink;
  H2O_VFR_INFO                                   *TempVfr;
  H2O_FORMSET_INFO                               *TempFormset;

  Status = GetLayoutTreeAndVfrTree (&LayoutListHead, &VfrListHead);
  if (EFI_ERROR (Status) || VfrListHead == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Get vfr
  //
  VfrLink = VfrListHead;
  if (IsNull (VfrLink, VfrLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  *Formset = NULL;
  TempFormset = NULL;
  do {
    VfrLink = VfrLink->ForwardLink;
    TempVfr = H2O_VFR_INFO_NODE_FROM_LINK (VfrLink);

    //
    // Get formset
    //
    FormsetListHead = &TempVfr->FormsetListHead;
    FormsetLink = FormsetListHead;
    if (IsNull (FormsetLink, FormsetLink->ForwardLink)) {
      continue;
    }
    do {
      FormsetLink = FormsetLink->ForwardLink;;
      TempFormset = H2O_FORMSET_INFO_NODE_FROM_LINK (FormsetLink);

      if (CompareGuid (&TempFormset->FormsetGuid, FormsetGuid)) {
        *Formset = TempFormset;
        break;
      }
    } while (!IsNodeAtEnd (FormsetListHead, FormsetLink));
    if (*Formset != NULL) {
      break;
    }

  } while (!IsNodeAtEnd (VfrListHead, VfrLink));

  if (*Formset == NULL) {
    *Formset = TempFormset;
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetLayoutIdByGuid (
  IN     EFI_GUID                                *Guid,
  OUT    UINT32                                  *LayoutId
  )
{
  EFI_STATUS                                     Status;
  H2O_PROPERTY_INFO                              *Property;
  H2O_PROPERTY_VALUE                             PropValue;
  LIST_ENTRY                                     *LayoutList;
  LIST_ENTRY                                     *VfrListHead;

  Status = GetLayoutTreeAndVfrTree (&LayoutList, &VfrListHead);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = GetPropertyFromVfrInVfcf (
             Guid,
             0,
             0,
             H2O_IFR_STYLE_TYPE_FORMSET,
             H2O_STYLE_PSEUDO_CLASS_NORMAL,
             "layout",
             NULL,
             &Property,
             &PropValue
             );
  if (!EFI_ERROR (Status)) {
    *LayoutId = PropValue.H2OValue.Value.U16;
  }

  return Status;
}

H2O_FORM_INFO *
GetFormLayoutByFormId (
  IN H2O_FORMSET_INFO                            *Formset,
  IN UINT32                                      FormId
  )
{
  LIST_ENTRY                                     *FormLink;
  H2O_FORM_INFO                                  *FormInfo;

  if (Formset == NULL) {
    return NULL;
  }

  FormLink = &Formset->FormListHead;
  if (IsNull (FormLink, FormLink->ForwardLink)) {
    return NULL;
  }

  do {
    FormLink = FormLink->ForwardLink;
    FormInfo = H2O_FORM_INFO_NODE_FROM_LINK (FormLink);
    if (FormInfo->FormId == FormId) {
      return FormInfo;
    }
  } while (!IsNodeAtEnd (&Formset->FormListHead, FormLink));

  return NULL;
}

H2O_STATEMENT_INFO *
GetStatementLayoutByQuestionId (
  IN H2O_FORMSET_INFO                            *Formset,
  IN H2O_FORM_INFO                               *FormInfo,
  IN BOOLEAN                                     IsQuestion,
  IN UINT16                                      Id
  )
{
  LIST_ENTRY                                     *FormLink;
  LIST_ENTRY                                     *StatementLink;
  H2O_FORM_INFO                                  *Form;
  H2O_STATEMENT_INFO                             *Statement;

  if (Formset == NULL) {
    return NULL;
  }

  if (IsListEmpty (&Formset->FormListHead)) {
    return NULL;
  }

  FormLink = GetFirstNode (&Formset->FormListHead);
  while (!IsNull (&Formset->FormListHead, FormLink)) {
    Form     = H2O_FORM_INFO_NODE_FROM_LINK (FormLink);
    FormLink = GetNextNode (&Formset->FormListHead, FormLink);
    if (IsListEmpty (&Form->StatementListHead)) {
      continue;
    }

    StatementLink = GetFirstNode (&Form->StatementListHead);
    while (!IsNull (&Form->StatementListHead, StatementLink)) {
      Statement     = H2O_STATEMENT_INFO_NODE_FROM_LINK (StatementLink);
      StatementLink = GetNextNode (&Form->StatementListHead, StatementLink);
      if (Statement->IsQuestion == IsQuestion &&
          Statement->QuestionId == Id) {
        return Statement;
      }
    }
  }

  return NULL;
}

VOID
SafeFreePool(
  IN   VOID                                **Buffer
  )
{
  if (*Buffer != NULL) {
    FreePool (*Buffer);
  }
  *Buffer = NULL;
}

