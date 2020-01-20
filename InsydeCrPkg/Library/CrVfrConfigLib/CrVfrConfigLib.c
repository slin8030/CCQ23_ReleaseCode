/** @file
  The CrVfrConfigLib is discard now.

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

#include <Library/CrVfrConfigLib.h>

EFI_STATUS
CrSerialDevOptionInit (
  IN EFI_HII_HANDLE                         HiiHandle
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
CRScuAdvanceCallback (
  IN  EFI_QUESTION_ID             QuestionId
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
CRScuHotKeyCallback (
  IN  EFI_QUESTION_ID             QuestionId,
  IN  EFI_INPUT_KEY               Key
  )
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
CRConfigExtractCallBack (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
CRConfigRouteCallBack (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN CONST EFI_STRING                       Configuration,
  OUT      EFI_STRING                       *Progress
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
CRBdsScuInit (
  EFI_CR_POLICY_PROTOCOL                    *CRPolicy
  )
{
  return EFI_UNSUPPORTED;
}


