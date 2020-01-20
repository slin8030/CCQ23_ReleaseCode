/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/


#include <Base.h>
#include <Library/IoLib.h>
#include <Library/PlatformCmosLib.h>
#include "CmosMap.h"
#include <ScAccess.h>
#include "PlatformBaseAddresses.h"
#include <Library/SteppingLib.h>
//[-start-180621-IB07400979-add]//
#include <ChipsetCmos.h>
#include <Library/CmosLib.h>
//[-end-180621-IB07400979-add]//


#define DEFAULT_VALUE          0
#define  DEFAULT_ATTRIBUTES     0
#define  EXCLUDE_FROM_CHECKSUM   CMOS_ATTRIBUTE_EXCLUDE_FROM_CHECKSUM

#define CMOS_DEBUG_PRINT_LEVEL_DEFAULT_VALUE      0x46   // EFI_D_WARN|EFI_D_INFO|EFI_D_LOAD
#define CMOS_DEBUG_PRINT_LEVEL_3_DEFAULT_VALUE    0x80   // EFI_D_ERROR

// Add the CMOS entry below
//[-start-160526-IB10860200-remove]//
//CMOS_ENTRY mCmosTable[] = {
//{ CPU_HT_POLICY, CPU_HT_POLICY_ENABLED, EXCLUDE_FROM_CHECKSUM },
//{ TPM_POLICY, TPM_POLICY_ENABLED, DEFAULT_ATTRIBUTES },
//{ CMOS_LCDPANELTYPE_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_LCDPANELSCALING_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_IGDBOOTTYPE_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_BACKLIGHT_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_LFP_PANEL_COLOR_DEPTH_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_EDP_ACTIVE_LFP_CONFIG_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_PRIMARY_DISPLAY_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_IGD_DISPLAY_PIPE_B_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_SDVOPANELTYPE_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_PLATFORM_RESET_OS, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_CPU_BSP_SELECT, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_CPU_RATIO_OFFSET, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_ICH_PORT80_OFFSET, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ CMOS_MAXRATIO_CONFIG_REG, DEFAULT_VALUE, DEFAULT_ATTRIBUTES },
//{ RTC_ADDRESS_CENTURY, RTC_ADDRESS_CENTURY_DEFAULT, CMOS_ATTRIBUTE_EXCLUDE_FROM_CHECKSUM },
//{ CMOS_POST_CODE_BREAK_REG, DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_POST_CODE_BREAK_1_REG, DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_POST_CODE_BREAK_2_REG, DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_POST_CODE_BREAK_3_REG, DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_DEBUG_PRINT_LEVEL_REG, CMOS_DEBUG_PRINT_LEVEL_DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_DEBUG_PRINT_LEVEL_1_REG, DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_DEBUG_PRINT_LEVEL_2_REG, DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//{ CMOS_DEBUG_PRINT_LEVEL_3_REG, CMOS_DEBUG_PRINT_LEVEL_3_DEFAULT_VALUE, EXCLUDE_FROM_CHECKSUM },
//};
//
///**
//  Funtion to return platform CMOS entry.
//
//  @param [out]  CmosEntry  Platform CMOS entry.
//
//  @param [out]  CmosEntryCount Number of platform CMOS entry.
//
//  @return Status.
//**/
//RETURN_STATUS
//EFIAPI
//GetPlatformCmosEntry (
//  OUT CMOS_ENTRY  **CmosEntry,
//  OUT UINTN       *CmosEntryCount
//  )
//{
//  *CmosEntry = mCmosTable;
//  *CmosEntryCount = sizeof(mCmosTable)/sizeof(mCmosTable[0]);
//  return RETURN_SUCCESS;
//}
//[-end-160526-IB10860200-remove]//

/**
  Function to check if Battery lost or CMOS cleared.

  @reval TRUE  Battery is always present.
  @reval FALSE CMOS is cleared.
**/
BOOLEAN
EFIAPI
CheckCmosBatteryStatus (
  VOID
  )
{
  UINT8 PmcPmcon1;
  //
  // Check if the CMOS battery is present
  // Checks RTC_PWR_STS bit in the GEN_PMCON_1 register
  //
//[-start-160817-IB03090432-remove]//
//  if((BxtStepping() <= BxtA1)) {
//    return TRUE;
//    }
//[-end-160817-IB03090432-remove]//
//[-start-160628-IB07400749-modify]//
  PmcPmcon1 = MmioRead8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  if (PmcPmcon1 != 0xFF){  
    if ((PmcPmcon1 & B_PMC_GEN_PMCON_RTC_PWR_STS) == 0) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else {
  	return TRUE;
  }
//[-end-160628-IB07400749-modify]//
}

//[-start-160628-IB07400749-add]//
VOID
EFIAPI
ClearCmosBatteryStatus (
  VOID
  )
{
  UINT8   Buffer8;
  
  Buffer8 = MmioRead8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  //CMOS Battery then clear status
  if (Buffer8 & B_PMC_GEN_PMCON_RTC_PWR_STS) {
    Buffer8 &= ~B_PMC_GEN_PMCON_RTC_PWR_STS;
    MmioWrite8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, Buffer8);
  }
  
  return;
}
//[-end-160628-IB07400749-add]//
//[-start-180621-IB07400979-add]//
VOID
ChipsetLoadCmosDefault (
  VOID
  )
{
  UINT8   CmosData;
  
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
  CmosData &= ~(B_CMOS_S5_WAKE_ON_USB);
  CmosData &= ~(B_CMOS_ACPI_CALLBACK_NOTIFY);
  CmosData &= ~(B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT);
  CmosData &= ~(B_CMOS_EFI_LEGACY_BOOT_ORDER);
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
  
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2); 
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND 
  CmosData &= ~(B_CMOS_VAR_DATA_CLEAR_WORKAROUND);
#endif
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData); 
}
//[-end-180621-IB07400979-add]//
