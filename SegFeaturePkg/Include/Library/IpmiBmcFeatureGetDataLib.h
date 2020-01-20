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
#ifndef _IPMI_BMC_FEATURE_GET_DATA_LIB_H_
#define _IPMI_BMC_FEATURE_GET_DATA_LIB_H_

EFI_STATUS
IpmiOemBmcGetMeOperationalVersion (
  OUT  CHAR8                             *MeVersion
  );

EFI_STATUS
IpmiOemBmcGetMeBackupVersion (
  OUT  CHAR8                             *MeVersion
  );

EFI_STATUS
IpmiOemBmcGetMeRecoveryVersion (
  OUT  CHAR8                             *MeVersion
  );

EFI_STATUS
IpmiOemBmcGetDimmMax (
  OUT  UINT8                             *CpuMaxNumber,
  OUT  UINT8                             *ChannelMaxPerCpu,
  OUT  UINT8                             *SlotMaxPerChannel
  );

EFI_STATUS
IpmiOemBmcGetDimmMax (
  OUT  UINT8                             *CpuMaxNumber,
  OUT  UINT8                             *ChannelMaxPerCpu,
  OUT  UINT8                             *SlotMaxPerChannel
  );

BOOLEAN
IpmiOemBmcCheckSlotPresence (
  IN  UINT8                             CpuMaxNumber,
  IN  UINT8                             ChannelMaxPerCpu,
  IN  UINT8                             SlotMaxPerChannel
  );

#endif

