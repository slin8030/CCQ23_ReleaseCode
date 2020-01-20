/** @file
 IPMI Bmc feature oem library header file.

 This file contains functions prototype that can easily access FUR data via
 using H2O IPMI FRU protocol.

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
#ifndef _IPMI_OEM_BMC_FEATURE_H_
#define _IPMI_OEM_BMC_FEATURE_H_

BOOLEAN
IpmiOemBmcGetSupportNet (
  VOID
  );

EFI_STATUS
IpmiOemBmcSyncDnsServer (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  );

EFI_STATUS
IpmiOemBmcSyncHostName (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  );

EFI_STATUS
IpmiOemBmcSyncDomainName (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  );

EFI_STATUS
IpmiOemBmcSyncLimitionType (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  );
#endif

