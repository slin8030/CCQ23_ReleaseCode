/** @file

   The definition of BIOS Storage Variable.

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


#ifndef _BIOS_STORAGE_VARIABLE_H_
#define _BIOS_STORAGE_VARIABLE_H_

extern EFI_GUID gH2OBiosStorageVariableGuid;

#define MAX_BIOS_STORAGE_VARIABLE_SIZE         0x800   // 0x800 = 2k byte
#define RESERVED_VAR_NUMBER                    0xFFFF  // Can't be used. 

#define BIOS_STORAGE_LOG_NUMBER_VAR_NAME       L"BiosStorageLogNum"
#define BIOS_STORAGE_LOG_ORDER_VAR_NAME        L"BiosElOrder"
#define BIOS_STORAGE_LOG_VIEW_ORDER_VAR_NAME   L"BiosElViewOrder"

typedef enum {
  CLEAR_EVT_ALL = 0,
  CLEAR_EVT_BY_NUM,
  CLEAR_EVT_MAX
} CLEAR_EVT_TYPE;

#pragma pack(1)

#pragma pack()

#endif
