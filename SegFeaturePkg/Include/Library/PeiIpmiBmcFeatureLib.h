/** @file
 Pei Ipmi Bmc feature header file.

 This file contains functions prototype that can easily retrieve SEL Info via
 using H2O IPMI SEL Data protocol.

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

#ifndef _IPMI_OEM_BMC_FEATURE_PEI_LIB_H_
#define _IPMI_OEM_BMC_FEATURE_PEI_LIB_H_

typedef enum {
  IpmiRecoveryInitializing,
  IpmiRecoveryValidatingImage,
  IpmiRecoveryProgrammingImage,
  IpmiRecoverySuccessfullyUpdated,
  IpmiRecoveryFailedInitialize = 0x80,
  IpmiRecoveryFailedValidateImage,
  IpmiRecoveryFailedProgramImage
} IPMI_RECOVERY_BIOS_STATUS;

EFI_STATUS
IpmiBmcFeatureLibEnterRecovery (
  VOID
  );

EFI_STATUS
IpmiBmcFeatureLibSetRecoveryStatus (
  IN  IPMI_RECOVERY_BIOS_STATUS             RecoveryStatus,
  IN  UINT8                                 Percent
  );

EFI_STATUS
EFIAPI
IpmiBmcFeatureLibSendRecoveryEnd (
  void
  );

UINT8
IpmiBmcFeatureChangeBootMode (
    void
    );

#endif

