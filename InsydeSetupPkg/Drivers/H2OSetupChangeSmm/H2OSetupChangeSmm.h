/** @file

  Header file of Setup Change SMM implementation.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _H2O_SETUP_CHANGE_SMM_H_
#define _H2O_SETUP_CHANGE_SMM_H_

//
// Statements that include other header files
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/VariableLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Protocol/H2OIhisi.h>
#include <Guid/EventGroup.h>
#include <Guid/SetupChangeVariable.h>
#include <H2OIhisi.h>

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


#endif
