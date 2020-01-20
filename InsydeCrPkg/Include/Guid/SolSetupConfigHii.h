/** @file

   The definition of Cr Config HII.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SOL_SETUP_CONFIG_HII_H_
#define _SOL_SETUP_CONFIG_HII_H_

//#include <UefiUefiBaseType.h>

#define SOL_FORMSET_GUID   { \
    0x48F88A03, 0x0DFC, 0x4479, {0x88, 0x07, 0x61, 0x9E, 0xCF, 0x8F, 0x04, 0xDE} \
  }
  
#define SOL_CONFIG_UTIL_VARSTORE_GUID \
  { \
    0x116E4D2F, 0xD27C, 0x447A, {0xA5, 0x6E, 0x2A, 0xE7, 0x65, 0x1C, 0xDE, 0xC8} \
  } 

#define LINK_SOL_FORMSET_CLASS_GUID \
  { \
    0xAC04BEEF, 0x93B6, 0x4006, {0x82, 0xD1, 0x0D, 0x07, 0xA0, 0x39, 0xF0, 0x78} \
  } 

#define SOL_VARSTORE_NAME   L"SolConfig"

#define SOL_IP_ADDR_STR_INDEX        0
#define SOL_MASK_STR_INDEX           1
#define SOL_GATEWAY_STR_INDEX        2
#define SOL_ADMIN_IP_STR_INDEX       3
#define SOL_LOGIN_ACCOUNT_STR_INDEX  4
#define SOL_LOGIN_PASSWORD_STR_INDEX 5
#define SOL_ADMIN_MAC_STR_INDEX      6

#pragma pack(1)

typedef struct {
  UINT8                  SolEnable;
  UINT8                  UseDhcp;
  UINT16                 PortNumber;
  UINT8                  LocalIp[4];
  UINT8                  SubnetMask[4];
  UINT8                  Gateway[4];
  UINT8                  AdminIp[4];
  UINT8                  AdminMac[32];
  CHAR16                 UserName[13];
  CHAR16                 Password[13];
} SOL_CONFIGURATION;

#pragma pack()

extern EFI_GUID gSolFormsetGuid;
extern EFI_GUID gSolConfigUtilVarstoreGuid;

#endif
