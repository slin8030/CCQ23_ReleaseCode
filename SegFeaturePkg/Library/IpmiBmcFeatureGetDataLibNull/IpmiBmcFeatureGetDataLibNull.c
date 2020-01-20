/** @file
 IpmiBmcFeatureOemLib Null implement code.

 This c file contains driver entry function for DXE phase.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

/**
 IPMI will get ME Operational Version by this library function.

 BMC joint feature need Me operational version, project need override this library.

 @param[OUT]         MeVersion         The Me Operational version like "0A:011.0000.0015.1003".

 @retval EFI_SUCCESS                    Get Me Operational version.
 @retval EFI_UNSUPPORTED                Not support to get Me Operation version.
*/
EFI_STATUS
IpmiOemBmcGetMeOperationalVersion (
  OUT  CHAR8                             *MeVersion
  )
{
  return EFI_UNSUPPORTED;
}

/**
 IPMI will get ME Backup Version by this library function.

 BMC joint feature need Me Backup version, project need override this library.

 @param[OUT]         MeVersion         The Me Backup version like "0A:011.0000.0015.1003".

 @retval EFI_SUCCESS                    Get Me Backup version.
 @retval EFI_UNSUPPORTED                Not support to get Me Backup version.
*/
EFI_STATUS
IpmiOemBmcGetMeBackupVersion (
  OUT  CHAR8                             *MeVersion
  )
{
  return EFI_UNSUPPORTED;
}

/**
 IPMI will get ME Recovery Version by this library function.

 BMC joint feature need Me Recovery version, project need override this library.

 @param[OUT]         MeVersion         The Me Recovery version like "0A:011.0000.0015.1003".

 @retval EFI_SUCCESS                    Get Me Recovery version.
 @retval EFI_UNSUPPORTED                Not support to get Me Recovery version.
*/
EFI_STATUS
IpmiOemBmcGetMeRecoveryVersion (
  OUT  CHAR8                             *MeVersion
  )
{
  return EFI_UNSUPPORTED;
}

/**
 IPMI need get MAX Dimm slots by this library function.

 BMC joint feature need to know Dimm presence, first should get CPUs, Channels and Slots.

 @param[OUT]         CpuMaxNumber         Return MAX CPU numbers.
 @param[OUT]         ChannelMaxPerCpu     Return MAX Channel numbers per CPU.
 @param[OUT]         ChannelMaxPerCpu     Return MAX Slot numbers per Channel.

 @retval EFI_SUCCESS                      Get Dimm info.
 @retval EFI_UNSUPPORTED                  Not support to get Dimm info.
*/
EFI_STATUS
IpmiOemBmcGetDimmMax (
  OUT  UINT8                             *CpuMaxNumber,
  OUT  UINT8                             *ChannelMaxPerCpu,
  OUT  UINT8                             *SlotMaxPerChannel
  )
{
  return EFI_UNSUPPORTED;
}

/**
 IPMI need get Dimm presence by this library function.

 BMC joint feature need to know Dimm presence, IPMI will get information.

 @param[IN]         Cpu                 CPU of this Dimm.
 @param[IN]         Channel             Channel of this Dimm.
 @param[IN]         Slot                Slot of this Dimm.

 @retval TRUE                           Dimm presence.
 @retval FALSE                          Dimm not presence.
*/
BOOLEAN
IpmiOemBmcCheckSlotPresence (
  IN  UINT8                             Cpu,
  IN  UINT8                             Channel,
  IN  UINT8                             Slot
  )
{
  return FALSE;
}

