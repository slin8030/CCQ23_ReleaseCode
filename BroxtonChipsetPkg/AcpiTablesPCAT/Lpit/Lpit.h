/** @file

@copyright
 Copyright (c) 2013 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/


#ifndef _LPIT_H_
#define _LPIT_H_

//
// Statements that include other files
//

#include <IndustryStandard/Acpi20.h>

// signature "LPIT"
#define ACPI_LOW_POWER_IDLE_TABLE_SIGNATURE  0x5449504c
#define ACPI_OEM_LPIT_REVISION               0x00000000

#define ACPI_LOW_POWER_IDLE_MWAIT_TYPE          0x0
#define ACPI_LOW_POWER_IDLE_DEFAULT_FLAG        0x0
#define ACPI_LOW_POWER_IDLE_NO_RES_COUNTER_FLAG 0x2
#define ACPI_LOW_POWER_IDLE_RES_FREQ_TSC        0x0

//
// LPI state count
//
#define ACPI_LPI_STATE_COUNT            2

//
// LPI TRIGGER (HW C10) 
//
#define ACPI_LPI_TRIGGER {0x7F,0x1,0x2,0x0,0x60}

//
// LPI residency counter (HW C10)
//
#define  ACPI_LPI_RES_COUNTER   {0x7F,64,0x0,0x0,0x632}

//
// LPI DUMMY COUNTER
//
#define  ACPI_DUMMY_RES_COUNTER   {0x0,0,0x0,0x0,0x0}


//
// LPI break-even residency in us (HW C10)
//
#define  ACPI_LPI_MIN_RES   30000

//
// LPI latency in us (HW C10) 
//  
#define  ACPI_LPI_LATENCY   3000

//
// LPI ID (HW C10 audio) 
//  
#define  ACPI_LPI_AUDIO_ID   0

//
// LPI ID (HW C10 CS) 
//  
#define  ACPI_LPI_CS_ID   1

#define EFI_ACPI_LOW_POWER_IDLE_TABLE_REVISION 0x1

//
// Ensure proper structure formats
//
#pragma pack(1)

typedef union _EFI_ACPI_LPI_STATE_FLAGS {
  struct {
    UINT32 Disabled           :1;
    UINT32 CounterUnavailable :1;
    UINT32 Reserved           :30;
  };
  UINT32 AsUlong;
} EFI_ACPI_LPI_STATE_FLAGS, *PEFI_ACPI_LPI_STATE_FLAGS;

// Only Mwait LPI here:

typedef struct _EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR {
  UINT32 Type;        // offset: 0
  UINT32 Length;      // offset: 4
  UINT16 UniqueId;    // offset: 8
  UINT8 Reserved[2]; // offset: 10
  EFI_ACPI_LPI_STATE_FLAGS Flags; // offset: 12
  EFI_ACPI_2_0_GENERIC_ADDRESS_STRUCTURE EntryTrigger; // offset: 16
  UINT32 Residency;  // offset: 28
  UINT32 Latency; // offset: 32
  EFI_ACPI_2_0_GENERIC_ADDRESS_STRUCTURE ResidencyCounter; // offset: 36
  UINT64 ResidencyCounterFrequency; //offset: 48
} EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR;

#pragma pack()

//
//  LPI ACPI table header
//
#pragma pack(1)

typedef struct _ACPI_LOW_POWER_IDLE_TABLE {
  EFI_ACPI_DESCRIPTION_HEADER             Header;
  EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR     LpiStates[ACPI_LPI_STATE_COUNT];
} ACPI_LOW_POWER_IDLE_TABLE;

#pragma pack()


#endif //_LPIT_H_
