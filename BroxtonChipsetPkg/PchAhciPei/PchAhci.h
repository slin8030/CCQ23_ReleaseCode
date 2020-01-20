/** @file
  Include File For AHCI Pci Host Controller Pei Module.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _PCH_AHCI_PEI_H_
#define _PCH_AHCI_PEI_H_

#include <PiPei.h>

#include <Ppi/MasterBootMode.h>
#include <Ppi/AhciController.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/Stall.h>

#include <IndustryStandard/Pci.h>

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>

#define AHCI_HC_PEI_SIGNATURE    SIGNATURE_32 ('A', 'H', 'C', 'P')
#define MAX_AHCI_HCS             4
#define AHCI_PCI_HOST_CONTROLLER_MMIO_BASE    0x90604000

typedef struct {
  UINTN                         Signature;
  PEI_AHCI_CONTROLLER_PPI       AhciControllerPpi;
  EFI_PEI_PPI_DESCRIPTOR        PpiDescriptor;
  EFI_PEI_NOTIFY_DESCRIPTOR     NotifyDescriptor;
  UINTN                         TotalAhciHcs;
  UINTN                         AhciHcPciAddr[MAX_AHCI_HCS];
} AHCI_HC_PEI_PRIVATE_DATA;

#define AHCI_HC_PEI_PRIVATE_DATA_FROM_THIS(a)         CR (a, AHCI_HC_PEI_PRIVATE_DATA, AhciControllerPpi, AHCI_HC_PEI_SIGNATURE)
#define AHCI_HC_PEI_PRIVATE_DATA_FROM_NOTIFY_DESC(a)  CR (a, AHCI_HC_PEI_PRIVATE_DATA, NotifyDescriptor,  AHCI_HC_PEI_SIGNATURE)

/**
  Get the MMIO base address of AHCI host controller.

  @param[in]  PeiServices        The pointer to the PEI Services Table.
  @param[in]  This               The pointer to this instance of the PEI_AHCI_CONTROLLER_PPI.
  @param[in]  AhciControllerId   The ID of the AHCI host controller.
  @param[out] AhciMemBaseAddr    Pointer to the AHCI host controller MMIO base address.

  @retval EFI_SUCCESS            The operation succeeds.
  @retval EFI_INVALID_PARAMETER  The parameters are invalid.

**/
EFI_STATUS
EFIAPI
GetAhciMemBase (
  IN  EFI_PEI_SERVICES         **PeiServices,
  IN  PEI_AHCI_CONTROLLER_PPI  *This,
  IN  UINT8                    AhciControllerId,
  OUT EFI_PHYSICAL_ADDRESS     *AhciMemBaseAddr
  );

#endif
