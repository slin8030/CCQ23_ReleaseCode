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
//; 
//; 
//;------------------------------------------------------------------------------
//; 
//; Abstract: Provide BIOS region lock protocol to protect BIOS region
//;
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
//[-start-160420-IB03090426-add]//
#include <Library/MmPciLib.h>
//[-end-160420-IB03090426-add]//
#include "BiosRegionLockInfo.h"

UINT8 *mSpiBase = NULL;

EFI_STATUS
BiosRegionLockInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
/*++

Routine Description:

  Entry point for Bios Region Lock driver. Install BIOS region lock protocol to protect BIOS region.
  
Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.

--*/
{
  EFI_HANDLE                            Handle;
  EFI_STATUS                            Status;
  BIOS_REGION_LOCK_INSTANCE            *BiosRegionLockInstance;
  BIOS_REGION_LOCK_PROTOCOL            *BiosRegionLock;
//[-start-160420-IB03090426-add]//
  UINTN                                 SpiPciBase;
//[-end-160420-IB03090426-add]//

//[-start-160420-IB03090426-add]//
  SpiPciBase = MmPciBase ( DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI);
  MmioOr32 (SpiPciBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
//[-end-160420-IB03090426-add]//
  
  mSpiBase = (UINT8*)(UINTN)(MmioRead32 (
                              MmPciAddress (0,
                                DEFAULT_PCI_BUS_NUMBER_SC,
                                PCI_DEVICE_NUMBER_SPI,
                                PCI_FUNCTION_NUMBER_SPI,
                                R_SPI_BASE
                              )
                            ) & B_SPI_BASE_BAR);
  
  if (MmSpi16 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) {
    DEBUG ((EFI_D_ERROR, "SPI has been locked - Access Denied!\n"));
    return EFI_DEVICE_ERROR;
  }

  BiosRegionLockInstance = AllocateZeroPool (sizeof(BIOS_REGION_LOCK_INSTANCE));
  if (BiosRegionLockInstance == NULL) {
    DEBUG ((EFI_D_ERROR, "Allocate Pool Failure!\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  BiosRegionLock = &BiosRegionLockInstance->BiosRegionLock;
  BiosRegionLock->SetRegionByType      = SetRegionByType;
  BiosRegionLock->SetRegionByAddress   = SetRegionByAddress;
  BiosRegionLock->ClearRegionByType    = ClearRegionByType;
  BiosRegionLock->ClearRegionByAddress = ClearRegionByAddress;
  BiosRegionLock->Lock                 = Lock;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiBiosRegionLockProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  BiosRegionLock
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Install EfiBiosRegionLockProtocol Failure!\n"));
    FreePool (BiosRegionLockInstance);
  }
#ifdef EFI_DEBUG
  else {
    DEBUG ((EFI_D_ERROR, "Install EfiBiosRegionLockProtocol Success!\n"));
  }
#endif

  return Status;
}
