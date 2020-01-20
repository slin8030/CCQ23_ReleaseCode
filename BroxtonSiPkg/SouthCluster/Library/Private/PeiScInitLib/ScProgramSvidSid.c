/** @file
  program SC device SSID & SDID

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
//[-start-160115-IB11270145-add]//
#include <Library/PeiOemSvcChipsetLib.h>
//[-end-160115-IB11270145-add]//
#include "ScInitPei.h"
//[-start-160115-IB11270145-remove]//

# if 0
///
/// SVID / SID init table entry
///
typedef struct {
  UINT8 DeviceNumber;
  UINT8 FunctionNumber;
  UINT8 SvidRegOffset;
} SC_SVID_SID_INIT_ENTRY;

STATIC SC_SVID_SID_INIT_ENTRY SvidSidInitTable[] = {
  { PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    PCI_SVID_OFFSET
  },
  {
    PCI_DEVICE_NUMBER_GMM,  //GMM device
    PCI_FUNCTION_NUMBER_GMM,
    R_GMM_SSID
  },
  {
    PCI_DEVICE_NUMBER_ISH,  //ISH device
    PCI_FUNCTION_NUMBER_ISH,
    R_ISH_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI2,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI1,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI0,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART0,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART1,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART2,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART3,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C7,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C6,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C5,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C4,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C3,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C2,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C1,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C0,
    R_LPSS_IO_SSID
  },
  {
    PCI_DEVICE_NUMBER_HDA,
    PCI_FUNCTION_NUMBER_HDA,
    R_HDA_SVID
  },
  {
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1,
    R_PCIE_SVID
  },
  {
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2,
    R_PCIE_SVID
  },
  {
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3,
    R_PCIE_SVID
  },
  {
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4,
    R_PCIE_SVID
  },
  {
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5,
    R_PCIE_SVID
  },
  {
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6,
    R_PCIE_SVID
  },
  {
    PCI_DEVICE_NUMBER_XHCI,
    PCI_FUNCTION_NUMBER_XHCI,
    R_XHCI_SVID
  },
  {
    PCI_DEVICE_NUMBER_OTG,
    PCI_FUNCTION_NUMBER_OTG,
    R_OTG_SSID
  },
  {
    PCI_DEVICE_NUMBER_PMC_PWM,
    PCI_FUNCTION_NUMBER_PMC_PWM,
    PCI_SVID_OFFSET
  },
  {
    PCI_DEVICE_NUMBER_SATA,
    PCI_FUNCTION_NUMBER_SATA,
    R_SATA_SS
  },
  {
    PCI_DEVICE_NUMBER_SCC_SDCARD,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID
  },
  {
    PCI_DEVICE_NUMBER_SCC_UFS,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID
  },
  {
    PCI_DEVICE_NUMBER_SCC_SDIO,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID
  },
  {
    PCI_DEVICE_NUMBER_SCC_EMMC,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID
  }
};
#endif
//[-end-160115-IB11270145-remove]//

//[-start-160115-IB11270145-modify]//
/**
  Program SC devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID).

  @param[in] ScPolicy  The SC Platform Policy protocol instance

  @retval EFI_SUCCESS  The function completed successfully
**/
EFI_STATUS
ProgramSvidSid (
  IN SC_POLICY_PPI  *ScPolicy
  )
{
  UINT8             Index;
  UINT8             BusNumber;
  UINTN             PcieAddressBase;
  UINT8             DeviceNumber;
  UINT8             FunctionNumber;
  UINT8             SvidRegOffset;
  UINTN             SvidSidInitTableSize;
  EFI_STATUS        Status;
  SC_GENERAL_CONFIG *ScGeneralConfig;
  SSID_SVID_PEI_CONFIG *SsidSvidTbl;

  SsidSvidTbl = NULL;
  SvidSidInitTableSize = 0;

  DEBUG ((DEBUG_INFO, "ProgramSvidSid() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gScGeneralConfigGuid, (VOID *) &ScGeneralConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // OemServices
  //
  Status = OemSvcUpdateSsidSvidPei (&SsidSvidTbl, &SvidSidInitTableSize);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcUpdateSsidSvidPei, Status : %r\n", Status));
  if (Status == EFI_SUCCESS) {
    return Status;
  }
  if ((ScGeneralConfig->SubSystemVendorId != 0) ||
      (ScGeneralConfig->SubSystemId != 0))
  {
    for (Index = 0; Index < SvidSidInitTableSize; Index++) {
      BusNumber       = SsidSvidTbl[Index].Bus;
      DeviceNumber    = SsidSvidTbl[Index].Device;
      FunctionNumber  = SsidSvidTbl[Index].Function;
      SvidRegOffset   = SsidSvidTbl[Index].SvidRegisterOffset;
      PcieAddressBase = MmPciBase (
                          BusNumber,
                          DeviceNumber,
                          FunctionNumber
                          );
      //
      // Skip if the device is disabled
      //
      if (MmioRead16 (PcieAddressBase) != V_INTEL_VENDOR_ID) {
        continue;
      }
      //
      // Program Pch devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID)
      //
      MmioWrite32 (
        (UINTN) (PcieAddressBase + SvidRegOffset),
        (UINT32) (SsidSvidTbl[Index].SsidSvid)
        );
    }
  }

  DEBUG ((DEBUG_INFO, "ProgramSvidSid() End\n"));

  return EFI_SUCCESS;
}
//[-end-160115-IB11270145-modify]//