//;******************************************************************************
//;* Copyright (c) 1983-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; 
//; Module Name:
//; 
//;   xtuCpuItem.h
//; 
//; Abstract:
//; 
//;   
//; 

#ifndef _XTU_CPU_ITEM_H_
#define _XTU_CPU_ITEM_H_

#include "Efi.h"
#include "SetupUtility.h"
#include "CpuRegs.h"
#include "EfiHobLib.h"
#include "EfiCommonLib.h"

#include EFI_GUID_DEFINITION (XTUInfoHob )
#include EFI_GUID_DEFINITION (Hob)

#define   B_MSR_TURBO_POWER_CURRENT_LIMIT_TDC_LIMIT (0x7FFF<<16)
#define   B_MSR_TURBO_POWER_CURRENT_LIMIT_TDP_LIMIT (0x7FFF<<0)

typedef struct {
  STRING_REF                                PromptToken;
  STRING_REF                                HelpToken;
  UINT16                                    Max;
  UINT8                                     Min;
  UINT8                                     Step;
  EFI_FORM_LABEL                            label;
} ITEM_OPTION;

EFI_STATUS
XTUCPUOption (
  IN EFI_HII_HANDLE                     HiiHandle
  );

#endif
