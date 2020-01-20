/** @file
  Header file for H2O Console Redirection Device Menu

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CR_DEVICE_MENU_H_
#define _CR_DEVICE_MENU_H_

#pragma pack(1)

typedef struct _CR_DEV_NODE {
  LIST_ENTRY                Link;
  UINT8                     Index;
  EFI_FORM_ID               FormId;
  EFI_QUESTION_ID           QuestionId;
  EFI_STRING_ID             PromptStrId;
  EFI_STRING_ID             StatusStrId;
  CR_DEVICE_SETTING         Dev;
} CR_DEV_NODE;

#pragma pack()

#define CR_DEV_NODE_FROM_LINK(a) BASE_CR (a, CR_DEV_NODE, Link)

EFI_STATUS
GetDevNodeByQuestionId (
  IN EFI_QUESTION_ID                          QuestionId,
  IN OUT CR_DEV_NODE                          **DevNode
  );

CHAR16*
GetStatusStr (
  IN CR_DEVICE_SETTING                        *Device
  );

EFI_STATUS
SetDevVariables (
  VOID
  );

EFI_STATUS
DestroyDevices (
  VOID
  );

EFI_STATUS
UpdateDevices (
  IN BOOLEAN                                  LoadDefault
  );

EFI_STATUS
CrUpdateMenu (
  IN EFI_HII_HANDLE                           HiiHandle
  );

#endif
