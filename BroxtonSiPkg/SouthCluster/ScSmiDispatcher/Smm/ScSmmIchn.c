/** @file
  File to contain all the hardware specific stuff for the Smm Ichn dispatch protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

#include "ScSmmHelpers.h"
#include "PlatformBaseAddresses.h"

SC_SMM_SOURCE_DESC ICHN_SOURCE_DESCS[NUM_ICHN_TYPES] = {
  ///
  /// IchnMch
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnPme
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnRtcAlarm
  ///
  {
    SC_SMM_SCI_EN_DEPENDENT,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_ACPI_PM1_EN
        },
        S_ACPI_PM1_EN,
        N_ACPI_PM1_EN_RTC
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_ACPI_PM1_STS
        },
        S_ACPI_PM1_STS,
        N_ACPI_PM1_STS_RTC
      }
    }
  },
  ///
  /// IchnRingIndicate
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnAc97Wake
  /// Not supported,
  /// we just fill in invalid initializer and not use it.
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnSerialIrq
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_STS
        },
        S_SMI_STS,
        N_SMI_STS_SERIRQ
      }
    }
  },
  ///
  /// IchnY2KRollover
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnTcoTimeout
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_EN
        },
        S_SMI_EN,
        N_SMI_EN_TCO
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_STS
        },
        S_SMI_STS,
        N_SMI_STS_TCO
      }
    }
  },
  ///
  /// IchnOsTco
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnNmi
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_EN
        },
        S_SMI_EN,
        N_SMI_EN_TCO
      },
      {
        {
          PCR_ADDR_TYPE,
          SC_PCR_ADDRESS (PID_ITSS, R_PCR_ITSS_NMICSTS)
        },
        S_PCR_ITSS_NMICSTS,
        N_PCR_ITSS_NMI2SMIEN
      }
    },
    {
      {
        {
          PCR_ADDR_TYPE,
          SC_PCR_ADDRESS (PID_ITSS, R_PCR_ITSS_NMICSTS)
        },
        S_PCR_ITSS_NMICSTS,
        N_PCR_ITSS_NMI2SMISTS
      }
    }
  },
  ///
  /// IchnIntruderDetect
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnBiosWp
  ///
#ifdef PCIESC_SUPPORT
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_EN
        },
        S_SMI_EN,
        N_SMI_EN_SPI_SSMI
      },
      {
        {
          PCIE_ADDR_TYPE,
          {
            (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
            (PCI_DEVICE_NUMBER_SPI << 16) |
            (PCI_FUNCTION_NUMBER_SPI << 8) |
            R_SPI_BCR
          }
        },
        S_SPI_BCR,
        N_SPI_BCR_BLE
      }
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_STS
        },
        S_SMI_STS,
        N_SMI_STS_SPI_SSMI
      }
    }
  },
#else
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
#endif
  ///
  /// IchnMcSmi
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_EN
        },
        S_SMI_EN,
        N_SMI_EN_MCSMI
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_SMI_STS
        },
        S_SMI_STS,
        N_SMI_STS_MCSMI
      }
    }
  },
  ///
  /// IchnPmeB0
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnThrmSts
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnIntelUsb2
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi7
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi6
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi5
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnMonSmi4
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap13
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap12, KBC_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap11
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap10
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap9, PIRQDH_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap8, PIRQCG_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap7, PIRQBF_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap6, PIRQAE_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap5
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap3
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap2
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap1
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnDevTrap0, IDE_ACT_STS
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// SC I/O Trap register 3 monitor,
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// SC I/O Trap register 2 monitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// SC I/O Trap register 1 monitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// SC I/O Trap register 0 monitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
};

SC_SMM_SOURCE_DESC ICHN_EX_SOURCE_DESCS[IchnExTypeMAX - IchnExPciExpress] = {
  ///
  /// IchnExPciExpress
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExMonitor
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExSpi
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExQRT
  ///
  NULL_SOURCE_DESC_INITIALIZER,
  ///
  /// IchnExGpioUnlock
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      NULL_BIT_DESC_INITIALIZER,
      NULL_BIT_DESC_INITIALIZER
    },
    {
      NULL_BIT_DESC_INITIALIZER
    }
  },
  ///
  /// IchnExTmrOverflow
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_ACPI_PM1_EN
        },
        S_ACPI_PM1_EN,
        N_ACPI_PM1_EN_TMROF
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          ACPI_ADDR_TYPE,
          R_ACPI_PM1_STS
        },
        S_ACPI_PM1_STS,
        N_ACPI_PM1_STS_TMROF
      }
    }
  },
#ifdef PCIESC_SUPPORT
  ///
  /// IchnExPcie0Hotplug/IchnExMEX0Hotplug
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie1Hotplug/IchnExMEX1Hotplug
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie2Hotplug
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie3Hotplug
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie4Hotplug
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie5Hotplug
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPPDM
      }
    }
  },
  ///
  /// IchnExPcie0LinkActive/IchnExMEX0LinkActive
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie1LinkActive/IchnExMEX1LinkActive
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie2LinkActive
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie3LinkActive
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie4LinkActive
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPLAS
      }
    }
  },
  ///
  /// IchnExPcie5LinkActive
  ///
  {
    SC_SMM_NO_FLAGS,
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6 << 8) |
          R_PCIE_MPC
          )
        },
        S_PCIE_MPC,
        N_PCIE_MPC_HPME
      },
      NULL_BIT_DESC_INITIALIZER
    },
    {
      {
        {
          PCIE_ADDR_TYPE,
          (
          (DEFAULT_PCI_BUS_NUMBER_SC << 24) |
          (PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2 << 16) |
          (PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6 << 8) |
          R_PCIE_SMSCS
          )
        },
        S_PCIE_SMSCS,
        N_PCIE_SMSCS_HPLAS
      }
    }
  },
#endif
};

/**
  Clear the SMI status bit after the SMI handling is done

  @param[in] SrcDesc              Pointer to the SC SMI source description table

  @retval None
**/
VOID
EFIAPI
ScSmmIchnClearSource (
  IN SC_SMM_SOURCE_DESC *SrcDesc
  )
{
  ScSmmClearSource (SrcDesc);
}
