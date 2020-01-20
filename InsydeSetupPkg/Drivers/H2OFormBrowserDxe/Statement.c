/** @file
 Define function of statement.
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

UINT16       mStatementIndex;
BOOLEAN      mInScopeSubtitle;

BOOLEAN
IsSelectable (
  IN FORM_BROWSER_STATEMENT     *Statement
  )
{
  EXPRESS_RESULT                ExpressResult;

  ASSERT (Statement);
  if (Statement == NULL) {
    return FALSE;
  }

  ExpressResult = EvaluateExpressionList(Statement->Expression, FALSE, NULL, NULL);
  if (ExpressResult == ExpressGrayOut || Statement->Operand == EFI_IFR_SUBTITLE_OP) {
    return FALSE;
  }
  if (Statement->Locked) {
    return FALSE;
  }
  if ((Statement->QuestionFlags & EFI_IFR_FLAG_READ_ONLY) != 0) {
    return FALSE;
  }

  if (Statement->QuestionId != 0 ||
      Statement->Operand == EFI_IFR_RESET_BUTTON_OP) {
    return TRUE;
  }

  return FALSE;
}

/**
 Initialize statement data.

 @param[in]      FormSet          Specified FormSet
 @param[in]      Form             Specified Form
 @param[in]      ExpressResult    Result of the expression list
 @param[in]      Page             Specified page
 @param[in, out] Statement        Formbrowser page identifer

**/
VOID
InitH2OStatement (
  IN     FORM_BROWSER_FORMSET      *FormSet,
  IN     FORM_BROWSER_FORM         *Form,
  IN     EXPRESS_RESULT               ExpressResult,
  IN     H2O_FORM_BROWSER_P        *Page,
  IN OUT FORM_BROWSER_STATEMENT    *Statement
  )
{
  H2O_FORM_BROWSER_S            *H2OStatement;
  EFI_IMAGE_INPUT               *ImageIn;
  EFI_STATUS                    Status;
  BOOLEAN                       HavePassword;

  ZeroMem (&Statement->Statement, sizeof (H2O_FORM_BROWSER_S));
  H2OStatement                  = &Statement->Statement;
  H2OStatement->Signature       = H2O_FORM_BROWSER_STATEMENT_SIGNATURE;
  H2OStatement->Size            = sizeof (H2O_FORM_BROWSER_Q);
  H2OStatement->PageId          = Page->PageId;
  H2OStatement->StatementId     = Statement->StatementId;
  H2OStatement->Operand         = Statement->Operand;
  H2OStatement->IfrOpCode       = Statement->OpCode;

  if (ExpressResult == ExpressGrayOut || Statement->Operand == EFI_IFR_SUBTITLE_OP) {
    H2OStatement->GrayedOut = TRUE;
  }
  if (Statement->Locked) {
    H2OStatement->Locked = TRUE;
  }
  if ((Statement->QuestionFlags & EFI_IFR_FLAG_READ_ONLY) != 0) {
    H2OStatement->ReadOnly = TRUE;
  }

  H2OStatement->Selectable      = IsSelectable (Statement);

  if (Statement->Prompt == 0) {
    H2OStatement->Prompt        = AllocateZeroPool (sizeof (CHAR16));
  } else {
    H2OStatement->Prompt        = GetString (Statement->Prompt, Form->FormSet->HiiHandle);
  }
  if (Statement->Help == 0) {
    H2OStatement->Help          = AllocateZeroPool (sizeof (CHAR16));
  } else {
    H2OStatement->Help          = GetString (Statement->Help, Form->FormSet->HiiHandle);
  }
  if (Statement->TextTwo == 0) {
    H2OStatement->TextTwo       = NULL;
  } else {
    H2OStatement->TextTwo       = GetString (Statement->TextTwo, Form->FormSet->HiiHandle);
  }

  H2OStatement->QuestionId      = Statement->QuestionId;
  H2OStatement->VarStoreId      = Statement->VarStoreId;
  H2OStatement->VariableName    = Statement->VariableName;
  H2OStatement->VariableOffset  = Statement->VarStoreInfo.VarOffset;
  H2OStatement->QuestionFlags   = Statement->QuestionFlags;

  if (H2OStatement->Operand == EFI_IFR_ORDERED_LIST_OP) {
    if (GetArrayData (Statement->BufferValue, Statement->ValueType, 0) == 0) {
      GetQuestionDefault (FormSet, Form, Statement, 0);
    }
  }

  CopyHiiValue (&H2OStatement->HiiValue, &Statement->HiiValue);
  if (H2OStatement->Operand == EFI_IFR_PASSWORD_OP) {
    //
    // In order to make display engine to hide password, change HII value to speace/empty string in H2O statement.
    //
    HavePassword = (*((CHAR16 *) Statement->HiiValue.Buffer) != CHAR_NULL) ? TRUE : FALSE;

    H2OStatement->HiiValue.Type         = EFI_IFR_TYPE_STRING;
    H2OStatement->HiiValue.Buffer       = AllocateZeroPool (H2OStatement->HiiValue.BufferLen);
    if (H2OStatement->HiiValue.Buffer != NULL) {
      if (HavePassword) {
        CopyMem (H2OStatement->HiiValue.Buffer, gSpaceString, StrSize (gSpaceString));
      } else {
        CopyMem (H2OStatement->HiiValue.Buffer, gEmptyString, StrSize (gEmptyString));
      }
    }
  }

  H2OStatement->Flags           = Statement->Flags;
  H2OStatement->ContainerCount  = Statement->MaxContainers;
  H2OStatement->Minimum         = Statement->Minimum;
  H2OStatement->Maximum         = Statement->Maximum;
  H2OStatement->Step            = Statement->Step;
  H2OStatement->DefaultId       = Statement->DefaultId;
  CopyGuid (&H2OStatement->RefreshGuid, &Statement->RefreshGuid);
  H2OStatement->RefreshInterval = Statement->RefreshInterval;

  H2OStatement->Image = NULL;
  if (Statement->ImageId != 0) {
    ImageIn = AllocateZeroPool (sizeof (EFI_IMAGE_INPUT));
    ASSERT (ImageIn != NULL);
    if (ImageIn == NULL) {
      return ;
    }
    Status = gHiiImage->GetImage (
                          gHiiImage,
                          FormSet->HiiHandle,
                          (EFI_IMAGE_ID) Statement->ImageId,
                          ImageIn
                          );
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {
      H2OStatement->Image = ImageIn;
    } else {
      FreePool (ImageIn);
    }
  }

  H2OStatement->Animation       = NULL;

  H2OStatement->NumberOfOptions = Statement->NumberOfOptions;
  H2OStatement->Options         = AllocateZeroPool (sizeof (H2O_FORM_BROWSER_O) * Statement->NumberOfOptions);
//H2OStatement->Inconsistent;
//H2OStatement->ErrorText;

  CopyGuid (&H2OStatement->FormsetGuid, &Form->FormSet->Guid);
  H2OStatement->FormId          = Form->FormId;

}

VOID
DestroyH2OStatement (
  IN OUT FORM_BROWSER_STATEMENT *Statement
  )
{
  H2O_FORM_BROWSER_S            *H2OStatement;
  H2O_FORM_BROWSER_O            *H2OOptions;
  UINTN                         Index;


  H2OStatement                  = &Statement->Statement;

  FBFreePool ((VOID **) &H2OStatement->Prompt);
  FBFreePool ((VOID **) &H2OStatement->Help);
  FBFreePool ((VOID **) &H2OStatement->TextTwo);
  if (H2OStatement->Image != NULL) {
    FBFreePool ((VOID **) &(H2OStatement->Image->Bitmap));
  }
  FBFreePool ((VOID **) &H2OStatement->Image);
  if (H2OStatement->Operand == EFI_IFR_PASSWORD_OP) {
    FBFreePool ((VOID **) &H2OStatement->HiiValue.Buffer);
  }

  H2OOptions = H2OStatement->Options;
  if (H2OOptions != NULL) {
    for (Index = 0; Index < H2OStatement->NumberOfOptions; Index++) {
      FBFreePool ((VOID **) &(H2OOptions[Index].Text));
      if (H2OOptions[Index].Image != NULL) {
        FBFreePool ((VOID **) &(H2OOptions[Index].Image->Bitmap));
      }
      FBFreePool ((VOID **) &(H2OOptions[Index].Image));
    }
  }
  FBFreePool ((VOID **) &H2OStatement->Options);
}



