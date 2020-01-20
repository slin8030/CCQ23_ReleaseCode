/** @file

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

/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

 LegacySpeaker.h

Abstract:

  Speaker enabling related data

--*/

#ifndef _PEI_LEGACY_SPEAKER_H
#define _PEI_LEGACY_SPEAKER_H

#include <Uefi.h>
#include <Ppi/Speaker.h>
//[-start-160929-IB05400712-modify]//
#include <Library/TimerLib.h>
//[-end-160929-IB05400712-modify]//
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PEIMEntryPoint.h>
#include <Library/PeiServicesLib.h>


//
// Speaker Related Port Information
//
#define EFI_TIMER_COUNTER_PORT      0x40
#define EFI_TIMER_CONTROL_PORT      0x43
#define EFI_TIMER_2_PORT            0x42
#define EFI_SPEAKER_CONTROL_PORT    0x61

#define EFI_SPEAKER_OFF_MASK        0xFC

#define EFI_DEFAULT_BEEP_FREQUENCY  0x500

//
// Default Intervals/Beep Duration
//
#define EFI_DEFAULT_LONG_BEEP_DURATION  0x70000
#define EFI_DEFAULT_SHORT_BEEP_DURATION 0x50000
#define EFI_DEFAULT_BEEP_TIME_INTERVAL  0x20000

typedef struct {
	UINTN BeepDuration;
  UINTN TimerInterval;
} EFI_BEEP_STRUCTURE;

typedef struct {
	EFI_BEEP_STRUCTURE  BeepData[9];
} EFI_BEEP_TABLE;

#define NUMBERS_OF_BEEP_DATA      0x03

#define TIMES_OF_MEMORY_ERROR_BEEP    0x03
#define MEMORY_ERROR_BEEP_INDEX       0x01

/**
 GC_TODO: Add function description

 @param [in]   ImageHandle      GC_TODO: add argument description
 @param [in]   SystemTable      GC_TODO: add argument description

 @return GC_TODO: add return values

**/
EFI_STATUS
EFIAPI
InitializeSpeakerInterface (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

/**
 GC_TODO: Add function description

 @param [in]   Frequency        GC_TODO: add argument description

 @return GC_TODO: add return values

**/
EFI_STATUS
ProgramToneFrequency (
  IN  UINT16                  Frequency
  );

/**
 GC_TODO: Add function description

 @param [in]   NumberOfBeeps    GC_TODO: add argument description
 @param [in]   BeepDuration     GC_TODO: add argument description
 @param [in]   TimeInterval     GC_TODO: add argument description

 @return GC_TODO: add return values

**/
EFI_STATUS
GenerateBeepTone (
  IN  UINTN                             NumberOfBeeps,
  IN  UINTN                             BeepDuration,
  IN  UINTN                             TimeInterval
  );

/**
 This function will generate the beep for specified duration.


 @retval EFI_STATUS

**/
EFI_STATUS
MemoryErrorBeep (
  );

/**
 GC_TODO: Add function description


 @return GC_TODO: add return values

**/
EFI_STATUS
TurnOnSpeaker (
  );

/**
 GC_TODO: Add function description


 @return GC_TODO: add return values

**/
EFI_STATUS
TurnOffSpeaker (
  );

EFI_STATUS
OutputBeep (
  IN  UINTN                   NumberOfBeep,
  IN  UINTN                   BeepDuration,
  IN  UINTN                   TimerInterval
  );

#endif
