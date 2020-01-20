/** @file
  Declaration file for NV Data

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Module Name:

  SioHiiResourceNvData.h

Abstract:

  Declaration file for NV Data.

**/

#ifndef _SIO_WPCD374L_HII_RESOURCE_NV_DATA_H_
#define _SIO_WPCD374L_HII_RESOURCE_NV_DATA_H_

#include <Guid/SioHiiResourceForm.h>
#include <Sio/SioCommon.h>

//
// Used by VFR for form or button identification
//
#define SIO_CONFIGURATION_VARSTORE_ID     0x4000
#define SIO_CONFIGURATION_FORM_ID         0x4001
#define SIO_WDT_QUESTION_ID               0x4351

#define SIO_COM1_LABEL                    0x4800
#define SIO_COM1_UPDATE_QUESTION_ID       0x4850
#define SIO_COM2_LABEL                    0x4801
#define SIO_COM2_UPDATE_QUESTION_ID       0x4851
#define SIO_COM3_LABEL                    0x4802
#define SIO_COM3_UPDATE_QUESTION_ID       0x4852
#define SIO_COM4_LABEL                    0x4803
#define SIO_COM4_UPDATE_QUESTION_ID       0x4853
#define SIO_COM5_LABEL                    0x4804
#define SIO_COM5_UPDATE_QUESTION_ID       0x4854
#define SIO_COM6_LABEL                    0x4805
#define SIO_COM6_UPDATE_QUESTION_ID       0x4855

//
// EFI Variable attributes
//
#define EFI_VARIABLE_NON_VOLATILE       0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS     0x00000004
#define EFI_VARIABLE_READ_ONLY          0x00000008

//
// VarOffset that will be used to create question
// all these values are computed from the structure
// defined below
//
#define VAR_OFFSET(Field)          ((UINT16) ((UINTN) &(((SIO_CONFIGURATION *) 0)->Field)))

//
// Nv Data structure referenced by IFR
//
#pragma pack(1)
typedef struct {
  SIO_DEVICE_LIST_TABLE DeviceA; ///< Com1
  SIO_DEVICE_LIST_TABLE DeviceB; ///< Com2
  SIO_DEVICE_LIST_TABLE DeviceC; ///< LPT
  UINT8                 AutoUpdateNum; ///< Count the number of the auto update SIO_DEVICE_LIST_TABLE
  UINT8                 Com1Type; 
  UINT8                 Com2Type;  
  UINT8                 Com3Type; 
  UINT8                 Com4Type;    
  UINT8                 Com5Type; 
  UINT8                 Com6Type; 
} SIO_CONFIGURATION;
#pragma pack()

#endif