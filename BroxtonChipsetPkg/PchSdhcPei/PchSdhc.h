/** @file

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _PEI_PCH_SDHC_H
#define _PEI_PCH_SDHC_H

#include <PiPei.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <IndustryStandard/Pci.h>
//
// Driver Produced PPI Prototypes
//
#include <Ppi/SdController.h>

//
// Driver Consumed PPI Prototypes
//
#include <Ppi/PciCfg2.h>
#include <Ppi/EndOfPeiPhase.h>

#define SDHC_BUS_NUMBER                 0x08  //0x02
#define SDHC_FUNCTION_NUMBER            0x01

#define COMMAND_REGISTER                0x04
#define CLASSC_REGISTER                 0x08
#define SDHC_BASE_ADDRESS_REGISTER      0x10

#define SDC_CLASSC                      0x08050100

//#define SDC_MEM_BASE_ADDRESS            0xC4000000
#define SOC_EMMC_MMIO_ADDRESS           0xC0000000
#define SOC_SD_MMIO_ADDRESS             0xC2000000
#define PCI_SD_MMIO_ADDRESS             0xC4000000

#define PCH_NUMBER_OF_PCIE_BRIDGES      4
#define PCH_PCIE_BRIDGE_ADDRESS         0x1C0000

#define ROOT_BRIDGE_BUS_REGISTER        0x18
#define ROOT_BRIDGE_ADDRESS_REGISTER    0x20

#define ID_SOC_EMMC                     0
#define ID_SOC_SD                       1
#define ID_PCI_SD                       2

#define PEI_PCH_SDHC_SIGNATURE          SIGNATURE_32 ('S', 'D', 'H', 'C')

#define EFI_PCI_ADDRESS(bus, dev, func, reg) \
    ((UINT64) ((((UINTN) bus) << 24) + (((UINTN) dev) << 16) + (((UINTN) func) << 8) + ((UINTN) reg)))
  
#define PCI_SUBCLASS_SD_HOST_CONTROLLER   0x05
#define PCI_IF_STANDARD_HOST_NO_DMA       0x00
#define PCI_IF_STANDARD_HOST_SUPPORT_DMA  0x01

typedef struct {
  UINTN                      Signature;
  PEI_SD_CONTROLLER_PPI      SdControllerPpi;
  EFI_PEI_PPI_DESCRIPTOR     PpiList;
  EFI_PEI_NOTIFY_DESCRIPTOR  NotifyList;
  EFI_PEI_PCI_CFG2_PPI       *PciCfgPpi;
  UINTN                      TotalSdControllers;
  UINTN                      MemBase;
  UINTN                      RootBridge;
  UINTN                      PciCfgAddress;
} PCH_SDHC_DEVICE;

#define PCH_SDHC_DEVICE_FROM_THIS(a) \
  CR(a, PCH_SDHC_DEVICE, SdControllerPpi, PEI_PCH_SDHC_SIGNATURE)

#define PCH_SDHC_DEVICE_FROM_NOTIFY_DESC(a) CR(a, PCH_SDHC_DEVICE, NotifyList, PEI_PCH_SDHC_SIGNATURE)

#endif
