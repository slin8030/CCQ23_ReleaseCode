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

 LegacySpeaker.c

Abstract:

  This file implements PEIM for Legacy Speaker. This file is valid for platforms both
  on IA32 and Itanium Product Family

--*/

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Ppi/Stall.h>

#include "LegacySpeaker.h"

EFI_STATUS
OutputBeep (
  IN  CONST EFI_PEI_SERVICES  **PeiServices,
  IN  UINTN             NumberOfBeep,
  IN  UINTN             BeepDuration,
  IN  UINTN             TimerInterval
  );

  
EFI_STATUS
TurnOnSpeaker (
  IN  CONST EFI_PEI_SERVICES    **PeiServices
  )
/*++

Routine Description:

  This function will enable the speaker to generate beep

Arguments:

  PeiServices     PeiServices to locate PPI


Returns:
  EFI_STATUS


--*/
{
  UINT8                   Data;
  Data = IoRead8 (EFI_SPEAKER_CONTROL_PORT);
  Data |= 0x03;
  IoWrite8(EFI_SPEAKER_CONTROL_PORT, Data);
  return EFI_SUCCESS;
}


EFI_STATUS
TurnOffSpeaker (
  IN  CONST EFI_PEI_SERVICES    **PeiServices
  )
/*++

Routine Description:

  This function will stop beep from speaker.

Arguments:

  PeiServices     PeiServices to locate PPI

Returns:

  Status


--*/
{
  UINT8                   Data;
  
  Data = IoRead8 (EFI_SPEAKER_CONTROL_PORT);
  Data &= 0xFC;
  IoWrite8(EFI_SPEAKER_CONTROL_PORT, Data);
  return EFI_SUCCESS;
}


EFI_STATUS
OutputBeep (
  IN  CONST EFI_PEI_SERVICES  **PeiServices,
  IN  UINTN             NumberOfBeep,
  IN  UINTN             BeepDuration,
  IN  UINTN             TimeInterval
  )
/*++

Routine Description:

  Generate beep sound based upon number of beeps and duration of the beep

Arguments:
    PeiServices       PeiServices to locate various PPIs
    NumberOfBeeps     Number of beeps which user want to produce
    BeepDuration      Duration for speaker gate need to be enabled
    TimeInterval      Interval between each beep
  
Returns:
  Does not return if the reset takes place.
  EFI_INVALID_PARAMETER   If ResetType is invalid.

--*/
{
  UINTN           Num;
  EFI_PEI_STALL_PPI*  StallPpi;

  (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);

  for (Num=0; Num < NumberOfBeep; Num++) {
    TurnOnSpeaker (PeiServices);
    StallPpi->Stall(PeiServices, StallPpi, BeepDuration);
    TurnOffSpeaker(PeiServices);
    StallPpi->Stall(PeiServices, StallPpi, TimeInterval);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ProgramToneFrequency (
  IN  CONST EFI_PEI_SERVICES            **PeiServices,
  IN  UINT16                            Frequency
  )
/*++

Routine Description:

  This function will program the speaker tone frequency. The value should be with 64k
  boundary since it takes only 16 bit value which gets programmed in two step IO opearattion

Arguments:
  Frequency     - A value which should be 16 bit only.

Returns:

  EFI_SUCESS

--*/
{
  UINT8                   Data;

  Data = 0xB6;
  IoWrite8(EFI_TIMER_CONTROL_PORT, Data);

  Data = (UINT8)(Frequency & 0x00FF);
  IoWrite8(EFI_TIMER_2_PORT, Data);
  Data = (UINT8)((Frequency & 0xFF00) >> 8);
  IoWrite8(EFI_TIMER_2_PORT, Data);
  return EFI_SUCCESS;
}


EFI_STATUS
GenerateBeepTone (
  IN  CONST EFI_PEI_SERVICES            **PeiServices,
  IN  UINTN                             NumberOfBeeps,
  IN  UINTN                             BeepDuration,
  IN  UINTN                             TimeInterval
  )
/*++

Routine Description:

  This function will generate the beep for specified duration.
 Arguments:
    PeiServices       PeiServices to locate various PPIs
    NumberOfBeeps     Number of beeps which user want to produce
    BeepDuration      Duration for speaker gate need to be enabled
    TimeInterval      Interval between each beep
  
Returns:
    EFI_STATUS

--*/
{

  if ((NumberOfBeeps == 1) && (BeepDuration == 0) && (TimeInterval == 0)) {
    TurnOnSpeaker (PeiServices);
    return EFI_SUCCESS;
  }

  if ((NumberOfBeeps == 0) && (BeepDuration == 0) && (TimeInterval == 0)) {
    TurnOffSpeaker (PeiServices);
    return EFI_SUCCESS;
  }

  if (BeepDuration == 0) {
    BeepDuration = EFI_DEFAULT_SHORT_BEEP_DURATION;
  }

  if (TimeInterval == 0) {
    TimeInterval = EFI_DEFAULT_BEEP_TIME_INTERVAL;
  }
  
  OutputBeep (PeiServices, NumberOfBeeps, BeepDuration, TimeInterval);
  return EFI_SUCCESS;
}

