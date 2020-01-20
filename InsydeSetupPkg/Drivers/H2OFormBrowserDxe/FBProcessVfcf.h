/** @file
  Header file for process VFCF of form browser

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

#ifndef _FB_PROCESS_VFCF_H_
#define _FB_PROCESS_VFCF_H_

#include "InternalH2OFormBrowser.h"
#include <Library/HiiExLib.h>

#define IS_IMPORT_STATEMENT(Statement)    ((BOOLEAN) (Statement != NULL && Statement->ImportInfo != NULL))

#define FORMSET_HII_RESOURCE_STRING_ID_INCREMENT  50
#define FORMSET_HII_RESOURCE_IMAGE_ID_INCREMENT   30

typedef struct _H2O_FORM_BROWSER_FORMSET_HII_RESOURCE    H2O_FORM_BROWSER_FORMSET_HII_RESOURCE;

#define H2O_FORM_BROWSER_FORMSET_HII_RESOURCE_SIGNATURE  SIGNATURE_32 ('H', 'F', 'H', 'R')

struct _H2O_FORM_BROWSER_FORMSET_HII_RESOURCE {
  UINTN                           Signature;
  LIST_ENTRY                      Link;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_GUID                        FormSetGuid;

  //
  // Record allocated HII resource which can be reused when form browser refresh formset.
  //
  EFI_STRING_ID                   *NewStringIdList;
  UINT16                          NewStringIdCount;
  UINT16                          NewStringIdIndex;

  EFI_IMAGE_ID                    *NewImageIdList;
  UINT16                          NewImageIdCount;
  UINT16                          NewImageIdIndex;
};

#define H2O_FORM_BROWSER_FORMSET_HII_RESOURCE_FROM_LINK(a)  CR (a, H2O_FORM_BROWSER_FORMSET_HII_RESOURCE, Link, H2O_FORM_BROWSER_FORMSET_HII_RESOURCE_SIGNATURE)

FORM_BROWSER_STATEMENT *
CreateDisabledStatementInLastForm (
  IN FORM_BROWSER_FORMSET                     *SrcFormSet,
  IN FORM_BROWSER_STATEMENT                   *SrcStatement,
  IN FORM_BROWSER_FORMSET                     *DstFormSet
  );

EFI_STATUS
CopyExpression (
  IN  FORM_BROWSER_FORMSET                    *SrcFormSet,
  IN     FORM_EXPRESSION                      *SrcExpression,
  IN     FORM_BROWSER_FORMSET                 *DstFormSet,
  IN     FORM_BROWSER_FORM                    *DstForm,
  IN OUT FORM_EXPRESSION                      **DstExpression
  );

BOOLEAN
HaveImportQuestion (
  IN FORM_BROWSER_FORM                        *Form
  );

BOOLEAN
HaveImportScuQuestion (
  IN FORM_BROWSER_FORMSET                     *FormSet,
  IN FORM_BROWSER_FORM                        *Form
  );

H2O_FORM_BROWSER_FORMSET_HII_RESOURCE *
GetFormSetHiiResource (
  IN FORM_BROWSER_FORMSET                     *FormSet
  );

FORM_BROWSER_STATEMENT *
GetStatementByQuestionId (
  IN FORM_BROWSER_FORMSET                     *FormSet,
  IN FORM_BROWSER_FORM                        *Form OPTIONAL,
  IN EFI_QUESTION_ID                          QuestionId
  );

FORM_BROWSER_STATEMENT *
GetNewStatement (
  IN FORM_BROWSER_FORMSET                     *FormSet
  );

EXPRESSION_OPCODE *
GetNewExpressionOpCode (
  IN FORM_BROWSER_FORMSET                     *FormSet
  );

EFI_STATUS
FBProcessVfcfImport (
  IN OUT FORM_BROWSER_FORMSET                 *FormSet
  );

EFI_STATUS
FBProcessVfcfProperty (
  IN FORM_BROWSER_FORMSET                     *FormSet
  );

VOID
DestroyImportFormSet (
  VOID
  );

#endif
