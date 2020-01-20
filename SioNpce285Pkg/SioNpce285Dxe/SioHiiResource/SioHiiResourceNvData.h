/** @file

Declaration file for NV Data

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#ifndef _SIO_NPCE285_HII_RESOURCE_NV_DATA_H_
#define _SIO_NPCE285_HII_RESOURCE_NV_DATA_H_

#include <SioCommon.h>

//
// Used by VFR for form or button identification
//
#define SIO_CONFIGURATION_VARSTORE_ID     0x4000
#define SIO_CONFIGURATION_FORM_ID         0x4001

//
// EFI Variable attributes
//
#define EFI_VARIABLE_NON_VOLATILE         0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS   0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS       0x00000004
#define EFI_VARIABLE_READ_ONLY            0x00000008

//
// Nv Data structure referenced by IFR
//
#pragma pack(1)
typedef struct {
  UINT8                 AutoUpdateNum; ///< Count the number of the auto update SIO_DEVICE_LIST_TABLE
  BOOLEAN                  NotFirstBoot;
} SIO_CONFIGURATION;
#pragma pack()

#endif
