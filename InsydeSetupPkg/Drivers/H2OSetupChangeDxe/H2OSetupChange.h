/** @file

  Header file of H2O Setup Change DXE implementation.

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

#ifndef _H2O_SETUP_CHANGE_H_
#define _H2O_SETUP_CHANGE_H_

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/H2ODialog.h>
#include <Protocol/H2OSubmitSvc.h>
#include <Protocol/H2OFormBrowser.h>
#include <Guid/SetupChangeVariable.h>
#include <Uefi/UefiBaseType.h>
#include <Pi/PiStatusCode.h>
#include <Protocol/ReportStatusCodeHandler.h>

//
// TimeStamp use
//
#define BASE_YEAR                           1970
#define BASE_MONTH                          1
#define BASE_DAY                            1

#define DAYS_PER_YEAR                       365
#define HOURS_PER_DAY                       24
#define MINUTES_PER_HOUR                    60
#define SECONDS_PER_MINUTE                  60
#define SECONDS_PER_DAY                     86400
#define SECONDS_PER_HOUR                    3600

//
//The policy when there is not enough space to record the all history of Setup Change.
//  0x0 : Do Nothing. Stop recording the history.
//  0x1 : Clear all of the old history.
//  0x2 : Overwrite the older history. Delete the oldest history to have enough space for new history.
//  0xFF: Pop up a dialog to let user to choose the policy.
#define SETUP_CHANGE_MAX_OPTION             3
#define SETUP_CHANGE_DISPLAY_DO_NOTHING     0
#define SETUP_CHANGE_DISPLAY_CLEAR          1
#define SETUP_CHANGE_DISPLAY_OVERWRITE      2
#define SETUP_CHANGE_DISPLAY_USE_DIALOG     0xFF
#define SETUP_CHANGE_MAX_OPTION_STRING_SIZE 100

#define FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE    SIGNATURE_32 ('F', 'B', 'S', 'C')

typedef struct _FORM_BROWSER_STATEMENT_CHANGED FORM_BROWSER_STATEMENT_CHANGED;

#pragma pack(1)

struct _FORM_BROWSER_STATEMENT_CHANGED {
  UINTN                           Signature;
  LIST_ENTRY                      Link;
  H2O_FORM_BROWSER_Q              Question;
  CHAR16                          *ChangedInfoStr;
} ;

typedef struct _DATE_CHANGED_INFO {
  EFI_HII_DATE                    OrgDate;
} DATE_CHANGED_INFO;

typedef struct {
  EFI_STATUS_CODE_DATA  DataHeader;
  SETUP_DATE_CHANGE     DateData;
} SETUP_DATE_CHANGE_STATUS_CODE_MESSAGE;

#pragma pack()

#define FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK(a)       CR (a, FORM_BROWSER_STATEMENT_CHANGED, Link, FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE)

#define DATE_CHANGE_INFO_VAR_NAME              L"DateChange"

EFI_STATUS
EFIAPI
ExecuteSubmitSvc (
  IN    H2O_SUBMIT_SVC_PROTOCOL             *This,
  OUT   UINT32                              *Request,
  OUT   BOOLEAN                             *ShowSubmitDialog
  );

EFI_STATUS
LogDateChange (
  IN    CHAR16                              *Prompt,
  IN    VOID                                *OrgDate,
  IN    VOID                                *ModDate
  );

EFI_STATUS
RestoreDateChange (
  VOID
  );

#endif  
