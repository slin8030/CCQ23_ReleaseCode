/** @file
 H2O IPMI Bmc feture library implement code.

 This c file contains H2O IPMI Bmc feature library instance for PEI phase.

;******************************************************************************
;* Copyright (c) 2016 - 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
  
#include <Library/PeiServicesLib.h>
#include <Library/PeiIpmiBmcFeatureLib.h>

EFI_STATUS
EFIAPI
IpmiBmcFeatureLibEnterRecovery (
  void
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSetRecoveryStatus (
  IN  IPMI_RECOVERY_BIOS_STATUS             RecoveryStatus,
  IN  UINT8                                 Percent
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSendRecoveryEnd (
  void
  )
{
  return EFI_UNSUPPORTED;
}

UINT8
IpmiBmcFeatureChangeBootMode (
  void
  )
{
  return 0xFF;
}

