/** @file
  Definition for Oem Services Default Lib.

;******************************************************************************
;* Copyright (c) 2012- 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SMM_COMPAL_SVC_LIB_H_
#define _SMM_COMPAL_SVC_LIB_H_
#include "SetupConfig.h"
#include "CompalGlobalNvsArea.h"



#pragma pack (1)

typedef struct {
    UINT8   cbFunNo;
    UINT16  cbSubFunNo;
} OEMFunctionBuffer;

#pragma pack ()


EFI_STATUS
CompalSvc_CMFC_OemFunc (
  IN COMPAL_GLOBAL_NVS_AREA                  *CompalGlobalNvsArea,
  IN SYSTEM_CONFIGURATION                    *SetupVariable,
  IN OEMFunctionBuffer                       *FB
  );

#endif
