/** @file
  Definition for Smm OEM Services H2oUve Lib.

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

#ifndef _SMM_OEM_SVC_H2OUVE_LIB_H_
#define _SMM_OEM_SVC_H2OUVE_LIB_H_

#include <Protocol/VariableEditBootTypeInfo.h>

#define DONT__CARE                            (0xFFFFFFFF)
#define ARRAY_SIZE(arr)                       (sizeof(arr) / sizeof((arr)[0]))
#define CHECK_CONDITION(reg, mask, val)       (((DONT__CARE) == (mask)) || (((reg) & (mask)) == (val)))
#define BS_RT_NV_VAR_ATTR                     (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)
#define VARIABLE_BUFFER_TO_DATA(p)            ((VOID *)(((UINT8 *)(&p->Name[0])) + (((p)->NameLength + 1) * sizeof(CHAR16))))
#define RESULT_VALUE_BUFFER_INVAID   2

typedef enum {
  _UVE__52H_ = 0x00005200, // AH=52h, H2O UEFI Variable Edit SMI service - Confirm the legality of the variables
  _UVE__53H_ = 0x00005300, // AH=53h, H2O UEFI Variable Edit SMI service - Boot information-related services
  _AH__MASK_ = 0x0000FF00,

  _ECX__01H_ = 0x00000001, // ECH=01h, Get Current Boot option order information
  _ECX__02H_ = 0x00000002, // ECH=02h, Set Current Boot option order sequence
  _ECX__03H_ = 0x00000003, // ECH=03h, Get Current Boot type order information
  _ECX__04H_ = 0x00000004, // ECH=04h, Set Current Boot type order sequence
  _ECX__05H_ = 0x00000005, // ECH=05h, Get Current Boot type name
  _ECX_MASK_ = 0x000000FF,
} H2OUVE_RAX_MASK;



#pragma pack(1)

//
// CallBackList struct
//
typedef struct _CALLBACK_DATA {
  UINT16                CallbackID;
  CHAR8                 StrData[1];   // Variable length
                                      // null-terminated ascii string 
} CALLBACK_DATA;

typedef struct _CALLBACK_LIST {
  UINT32                ListCount;
  CALLBACK_DATA         List[1];     // Variable size
} CALLBACK_LIST;

//
// ResultList struct
//
typedef struct _RESULT_DATA {
  UINT16                CallbackID;
  UINT16                Result;
} RESULT_DATA;

typedef struct _RESULT_ARRAY {
  UINT32                ArrayCount;
  RESULT_DATA           Array[1];     // Variable size
} RESULT_ARRAY;

typedef struct _VARIABLE_BUFFER_PTR {
  EFI_GUID              Guid;
  UINT32                NameLength ;
  UINT32                DataSize;
  CHAR16                Name[1];   // Variable length
} VARIABLE_BUFFER_PTR;

typedef
EFI_STATUS
(EFIAPI * H2OUVE_SERVICE_FUNCTION_POINT) (
  IN CONST UINT64               Rax,
  IN CONST UINT64               Rcx,
  IN CONST UINT64               Rsi,
  IN CONST UINT64               Rdi
  );

typedef struct _H2OUVE_SERVICE_FUNCTION_MAP_TABLE {
  UINT64                        MaskRax;
  UINT64                        ValueRax;
  UINT64                        MaskRcx;
  UINT64                        ValueRcx;
  UINT64                        MaskRsi;
  UINT64                        ValueRsi;
  UINT64                        MaskRdi;
  UINT64                        ValueRdi;
  H2OUVE_SERVICE_FUNCTION_POINT FuncPtr;
} H2OUVE_SERVICE_FUNCTION_MAP_TABLE;

typedef struct _H2OUVE_BOOT_TYPE_NAME {
  UINT32                        BufferSize;
  UINT16                        BootTypeNumber;
  CHAR16                        String[1];
} H2OUVE_BOOT_TYPE_NAME;

typedef struct _H2OUVE_BOOT_TYPE_INFORMATION {
  UINT32                        BufferSize;
  UINT16                        Count;
  UINT8                         Reserve0[8];
  CHAR16                        BootTypeString[32];
  UINT16                        BootTypeNumber[MAX_BOOT_ORDER_NUMBER];
  UINT16                        BootTypeDefaultNumber[MAX_BOOT_ORDER_NUMBER];
} H2OUVE_BOOT_TYPE_INFORMATION;

typedef struct _H2OUVE_BOOT_OPTION_MAPPING {
  UINT16                        BootOptionNumber;
  UINT16                        BootOptionType;
} H2OUVE_BOOT_OPTION_MAPPING;

typedef struct _H2OUVE_BOOT_OPTION_INFORMATION {
  UINT32                        BufferSize;
  BOOLEAN                       BootOrderByType; 
  BOOLEAN                       EfiFirst; 
  UINT16                        Count;
  UINT8                         Reserve0[6];
  H2OUVE_BOOT_OPTION_MAPPING    Map[1];
} H2OUVE_BOOT_OPTION_INFORMATION;

#pragma pack()

VOID
SetVarEditBootTypeInfo (
  VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL                 *ProtocolPtr
);

EFI_STATUS
H2oUveSmiService (
  VOID
);


EFI_STATUS
EFIAPI
SetupCallbackSyncCallbackIhisi (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);

EFI_STATUS
EFIAPI
H2oUveSmiServiceDispatcher (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);


EFI_STATUS
EFIAPI
GetBootTypeOrderInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);

EFI_STATUS
EFIAPI
SetBootTypeOrderInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);

EFI_STATUS
EFIAPI
GetCurrentBootTypeName (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);

EFI_STATUS
EFIAPI
GetCurrentBootTypeInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);


EFI_STATUS
EFIAPI
SetCurrentBootTypeInformation (
  IN CONST UINT64                       Rax,
  IN CONST UINT64                       Rcx,
  IN CONST UINT64                       Rsi,
  IN CONST UINT64                       Rdi
);

#endif
