/** @file

   The definition of Event Log POST Display Variable.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _EVENTLOG_POST_DISPLAY_VARIABLE_H_
#define _EVENTLOG_POST_DISPLAY_VARIABLE_H_


#define EVENT_LOG_POST_DISPLAY_VARIABLE_NAME   L"ELPostDisp"

//
// Ensure proper structure formats
//
#pragma pack(1)

///
/// Time Abstraction:
///  Year:       2000 - 2099
///  Month:      1 - 12
///  Day:        1 - 31
///  Hour:       0 - 23
///  Minute:     0 - 59
///  Second:     0 - 59
///
typedef struct {
  UINT16  Year;
  UINT8   Month;
  UINT8   Day;
  UINT8   Hour;
  UINT8   Minute;
  UINT8   Second;
} EL_POST_DISPLAY_VARIABLE;
#pragma pack()

extern EFI_GUID gH2OEventLogPostDisplayDxeVariableGuid;

#endif
