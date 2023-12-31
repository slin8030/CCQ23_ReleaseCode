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


//
// Speaker Related Port Information
//
#define EFI_TIMER_COUNTER_PORT            0x40
#define EFI_TIMER_CONTROL_PORT            0x43
#define EFI_TIMER_2_PORT                  0x42
#define EFI_SPEAKER_CONTROL_PORT          0x61
#define EFI_SPEAKER_OFF_MASK              0xFC


//
// Default Intervals/Beep Duration
//
#define EFI_DEFAULT_BEEP_FREQUENCY        0x500
#define EFI_DEFAULT_LONG_BEEP_DURATION    0x70000
#define EFI_DEFAULT_SHORT_BEEP_DURATION   0x50000
#define EFI_DEFAULT_BEEP_TIME_INTERVAL    0x20000


EFI_STATUS 
ProgramToneFrequency (
  IN  CONST EFI_PEI_SERVICES  **PeiServices,
  IN  UINT16 Frequency
  );


EFI_STATUS
GenerateBeepTone (
  IN  CONST EFI_PEI_SERVICES                  **PeiServices,
  IN  UINTN                             NumberOfBeeps,
  IN  UINTN                             BeepDuration,
  IN  UINTN                             TimeInterval
  );

  
EFI_STATUS
TurnOnSpeaker (
  IN  CONST EFI_PEI_SERVICES  **PeiServices
  );

  
EFI_STATUS
TurnOffSpeaker (
  IN  CONST EFI_PEI_SERVICES  **PeiServices
  );

#endif
