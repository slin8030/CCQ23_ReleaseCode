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
#include <Library/IpmiBmcFeatureOemLib.h>

BOOLEAN
IpmiOemBmcGetSupportNet (
  VOID
  )
{
  return FALSE;
}

EFI_STATUS
IpmiOemBmcSyncDnsServer (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
IpmiOemBmcSyncHostName (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
IpmiOemBmcSyncDomainName (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
IpmiOemBmcSyncLimitionType (
  IN  UINT8                             *IpmiDmConfig,
  IN  BOOLEAN                           SetToBmc
  )
{
  return EFI_UNSUPPORTED;
}

