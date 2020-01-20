/** @file
  SEC Chipset Lib implementation.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/PerformanceLib.h>
#include <Library/TimerLib.h>
#include <SeCState.h>
#include <CoreBiosMsg.h>
#include <HeciRegs.h>
#include <SeCAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Library/MmPciLib.h>
#include <Library/PcdLib.h>

/**
  Put SEC device into D0I3.

  @param[in] Function          Select of SEC device

  @retval VOID                 None
**/
VOID
SetD0I3Bit (
  IN  UINT32   Function
  )
{
  UINTN                           DevicePciCfgBase;
  UINT32                          DeviceBar[2];
  UINTN                           *Bar;

  ///
  /// Get Device MMIO address
  ///
  DevicePciCfgBase = MmPciBase(SEC_BUS, SEC_DEVICE_NUMBER, Function);
  if (MmioRead16 (DevicePciCfgBase + PCI_DEVICE_ID_OFFSET) == 0xFFFF) {
    DEBUG ((DEBUG_ERROR, "Device disabled, can't set D0I3 bit!\n"));
    return;
  }

  DeviceBar[0] = MmioRead32 (DevicePciCfgBase + PCI_BASE_ADDRESSREG_OFFSET) & 0xFFFFFFF0;
  DeviceBar[1] = 0x0;
  if ((MmioRead32 (DevicePciCfgBase + PCI_BASE_ADDRESSREG_OFFSET) & 0x6) == 0x4) {
    DeviceBar[1] = MmioRead32 (DevicePciCfgBase + (PCI_BASE_ADDRESSREG_OFFSET + 4));
  }

  ///
  /// Put CSME Device in D0I3
  ///
  MmioOr8 (DevicePciCfgBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);
  if (!(DeviceBar[0] == 0x0 && DeviceBar[1] == 0x0) && !(DeviceBar[0] == 0xFFFFFFF0 && DeviceBar[1] == 0xFFFFFFFF)) {
    Bar = (UINTN*)DeviceBar;
    MmioOr32 (*Bar + D0I3C, BIT2);
  }
  MmioAnd8 (DevicePciCfgBase + PCI_COMMAND_OFFSET,(UINT8)~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER));
}

/**
  Enable/Disable SEC devices

  @param[in] WhichDevice       Select of SEC device
  @param[in] DeviceFuncCtrl    Function control

  @retval VOID                 None
**/
VOID
SeCDeviceControl (
  IN  SEC_DEVICE                   WhichDevice,
  IN  SEC_DEVICE_FUNC_CTRL         DeviceFuncCtrl
  )
{

  UINT32                            PmcBase;
  UINT32                            FuncDisableReg;

  PmcBase         = PMC_BASE_ADDRESS;
  FuncDisableReg  = MmioRead32 (PmcBase + R_PMC_FUNC_DIS);

if(Disabled == DeviceFuncCtrl)
  {
  switch (WhichDevice){
    case HECI1:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),FuncDisableReg| BIT27);
    DEBUG ((DEBUG_INFO, "HECI1 Device disabled\n"));
    break;
    case HECI2:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),FuncDisableReg| BIT26);
    DEBUG ((DEBUG_INFO, "HECI2 Device disabled\n"));
    break;
    case HECI3:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),FuncDisableReg| BIT25);
    DEBUG ((DEBUG_INFO, "HECI3 Device disabled\n"));
    break;
    case FTPM:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),(FuncDisableReg| BIT27| BIT26|BIT25));
    DEBUG ((DEBUG_INFO, "FTPM Device disabled\n"));
    break;
    default:
    break;
    }
  }else{
  switch (WhichDevice){
    case HECI1:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),(FuncDisableReg & (~BIT27)));
    DEBUG ((DEBUG_INFO, "HECI1 Device Enabled\n"));
    break;
    case HECI2:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),(FuncDisableReg& (~BIT26)));
    DEBUG ((DEBUG_INFO, "HECI2 Device Enabled\n"));
    break;
    case HECI3:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),(FuncDisableReg& (~ BIT25)));
    DEBUG ((DEBUG_INFO, "HECI3 Device Enabled\n"));
    break;
    case FTPM:
    MmioWrite32((PmcBase+R_PMC_FUNC_DIS),(FuncDisableReg &(~(BIT27| BIT26|BIT25))));
    DEBUG ((DEBUG_INFO, "FTPM Device Enabled\n"));
    break;
    default:
    break;
    }
  }
}
