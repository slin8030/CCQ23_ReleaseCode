/** @file
  Content file contains function definitions for Variable Edit Smm Driver.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#ifndef _VARIABLE_EDIT_SMM_H_
#define _VARIABLE_EDIT_SMM_H_

#include <PiSmm.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/SmmVariable.h>
#include <H2OIhisi.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmOemSvcH2oUveLib.h>
#include <Protocol/VariableEditBootTypeInfo.h>
#include <Guid/H2oUveConfigUtilHii.h>

#define array_size(arr)     (sizeof(arr) / sizeof((arr)[0]))

enum {
  UveVariableConfirm = 0x52,
  UveBootInfoService = 0x53,
};
#endif
