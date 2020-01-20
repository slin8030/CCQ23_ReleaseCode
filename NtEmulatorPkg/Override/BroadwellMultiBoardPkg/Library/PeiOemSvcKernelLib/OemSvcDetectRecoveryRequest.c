/** @file
  This OemService provides OEM to decide the method of recovery request. 
  When DXE loader found that the DXE-core of firmware volume is corrupt, it will force system to restart. 
  This service will be called to set the recovery requests before system restart. 
  To design the recovery requests according to OEM specification.

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/IoLib.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/PchPlatformLib.h>

#include <Library/PcdLib.h>
#define R_GPIO_LVL              0x0C
#define R_SIO_GP1               0x0C

//
// helper function prototypes 
//
extern
UINT32
EFIAPI
IoRead32 (
  IN      UINTN                 Port
  );

/**
  This OemService provides OEM to detect the recovery mode. 
  OEM designs the rule to detect that boot mode is recovery mode or not, 
  The rule bases on the recovery request which defined in OemService "OemSetRecoveryRequest".

  @param[in, out]  *IsRecovery        If service detects that the current system is recovery mode.
                                      This parameter will return TRUE, else return FALSE.

  @retval          EFI_MEDIA_CHANGED  The Recovery is detected. (*IsRecovery is set)
**/
EFI_STATUS
OemSvcDetectRecoveryRequest (
  IN OUT BOOLEAN                        *IsRecovery
  )
{
  *IsRecovery = FALSE; 
  return EFI_MEDIA_CHANGED;
} 
