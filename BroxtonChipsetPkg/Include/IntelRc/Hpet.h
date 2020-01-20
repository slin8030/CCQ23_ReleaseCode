/**@file
  This file describes the contents of the ACPI HPET Table.

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification
**/

#ifndef _HPET_H
#define _HPET_H

//
// Statements that include other files
//

#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>
#include <IndustryStandard/HighPrecisionEventTimerTable.h>

//
// HPET Definitions
//

#define EFI_ACPI_HPET_TABLE_REVISION            0x1
//#define MAIN_COUNTER_MIN_PERIODIC_CLOCK_TICKS	  14318        //approx 1ms
#define MAIN_COUNTER_MIN_PERIODIC_CLOCK_TICKS	  0x80        //approx 1ms

#define HPET_BASE_ADDRESS                       0xFED00000
#define EFI_ACPI_EVENT_TIMER_BLOCK_ID           0x8086A001

#endif
