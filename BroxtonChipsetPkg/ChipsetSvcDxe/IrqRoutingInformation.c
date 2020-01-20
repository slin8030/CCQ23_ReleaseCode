/** @file
 DXE Chipset Services Library.
	
 This file contains only one function that is IrqRoutingInformation().
 The function DxeCsSvcIrqRoutingInformation() use chipset services to return 
 four table for InitGetRoutingTable ().
	
***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <IrqRoutingInformation.h>
#include <ChipsetIrqRoutingEntry.h>

/**
 Provide IRQ Routing table according to platform specific information.

 @param[OUT]      VirtualBusTablePtr           A pointer to the VirtualBusTable that list all bridges in this platform 
                                               and bridges' virtual secondary bus.
 @param[OUT]      VirtualBusTableEntryNumber   A pointer to the VirtualBusTable entry number.
 @param[OUT]      IrqPoolTablePtr,             A pointer to the IRQ Pool Table.
 @param[OUT]      IrqPoolTableNumber,          A pointer to the IRQ Pool Table entry number.
 @param[OUT]      PirqPriorityTable,           A pointer to the PIRQ priority used Table.
 @param[OUT]      PirqPriorityTableEntryNumber A pointer to the PIRQ priority used Table entry number.
 @param[OUT]      IrqTablePtr                  A pointer to the IRQ routing Table.

 @retval          EFI_SUCCESS                  Get Platform specific IRQ Routing Info successfully..
 @retval          EFI_UNSUPPORTED              If the function is not implemented.
*/
EFI_STATUS
IrqRoutingInformation (
  OUT LEGACY_MODIFY_PIR_TABLE             **VirtualBusTablePtr,
  OUT UINT8                               *VirtualBusTableEntryNumber,
  OUT EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY **IrqPoolTablePtr,
  OUT UINT8                               *IrqPoolTableNumber,
  OUT UINT8                               **PirqPriorityTablePtr,
  OUT UINT8                               *PirqPriorityTableEntryNumber,
  OUT IRQ_ROUTING_TABLE                   **IrqRoutingTableInfoPtr
)
{
  UINTN  RoutingTableAndIPRegisterCount;
  UINTN  TargetEntryIndex;
  EFI_LEGACY_IRQ_ROUTING_ENTRY_AND_IP_REGISTER  *RoutingTableAndIPRegisterPtrTemp;
  EFI_LEGACY_IRQ_ROUTING_ENTRY  *IrqRoutingEntryPtrTemp;
  EFI_LEGACY_PIRQ_TABLE_HEADER  *IrqRoutingTableHeaderPtrTemp;
  EFI_LEGACY_PIRQ_TABLE_HEADER  IrqRoutingHeader = {
   0x52495024, 00, 01, 0000, 00, 00, 0000, 0x8086, 0x122e,
   00000000, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
  };

  RoutingTableAndIPRegisterPtrTemp = NULL;
  IrqRoutingEntryPtrTemp           = NULL;
  RoutingTableAndIPRegisterCount   = 0;
  TargetEntryIndex                 = 0;
  *VirtualBusTablePtr           = PcdGetExPtr (&gChipsetPkgTokenSpaceGuid, PcdVirtualBusTable);
  *VirtualBusTableEntryNumber   = (UINT8)(LibPcdGetExSize (&gChipsetPkgTokenSpaceGuid, PcdToken(PcdVirtualBusTable)) / sizeof (LEGACY_MODIFY_PIR_TABLE));
  *IrqPoolTablePtr              = PcdGetExPtr (&gChipsetPkgTokenSpaceGuid, PcdIrqPoolTable);
  *IrqPoolTableNumber           = (UINT8)(LibPcdGetExSize (&gChipsetPkgTokenSpaceGuid, PcdToken(PcdIrqPoolTable)) / sizeof (EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY));
  *PirqPriorityTablePtr         = PcdGetExPtr (&gChipsetPkgTokenSpaceGuid, PcdPirqPriorityTable);
  *PirqPriorityTableEntryNumber = (UINT8)(LibPcdGetExSize (&gChipsetPkgTokenSpaceGuid, PcdToken(PcdPirqPriorityTable)) / sizeof (UINT8));

  (*IrqRoutingTableInfoPtr) = AllocateZeroPool (sizeof (IRQ_ROUTING_TABLE));
  if ((*IrqRoutingTableInfoPtr) == NULL) {
    return EFI_UNSUPPORTED;
  }
  //
  // Allocate a Buffer to PcdControllerDeviceIrqRoutingEntry .
  // This PCD "PcdControllerDeviceIrqRoutingEntry" is merge the IRQ routing entry and the Interrupt pin register
  //
  RoutingTableAndIPRegisterCount = (LibPcdGetExSize (&gChipsetPkgTokenSpaceGuid, PcdToken (PcdControllerDeviceIrqRoutingEntry)) / sizeof (EFI_LEGACY_IRQ_ROUTING_ENTRY_AND_IP_REGISTER));
  RoutingTableAndIPRegisterPtrTemp = AllocateZeroPool (sizeof (EFI_LEGACY_IRQ_ROUTING_ENTRY_AND_IP_REGISTER) * RoutingTableAndIPRegisterCount);
  if (RoutingTableAndIPRegisterPtrTemp == NULL) {
    FreePool(*IrqRoutingTableInfoPtr);
    return EFI_UNSUPPORTED;
  }
  CopyMem (RoutingTableAndIPRegisterPtrTemp,
    PcdGetExPtr (&gChipsetPkgTokenSpaceGuid, PcdControllerDeviceIrqRoutingEntry),
    sizeof (EFI_LEGACY_IRQ_ROUTING_ENTRY_AND_IP_REGISTER) * RoutingTableAndIPRegisterCount);
  IrqRoutingEntryPtrTemp = AllocateZeroPool (sizeof (EFI_LEGACY_IRQ_ROUTING_ENTRY) * RoutingTableAndIPRegisterCount);
  if (IrqRoutingEntryPtrTemp == NULL) {
    FreePool(*IrqRoutingTableInfoPtr);
    return EFI_UNSUPPORTED;
  }

  for (TargetEntryIndex = 0; TargetEntryIndex < RoutingTableAndIPRegisterCount; TargetEntryIndex++) {    
    IrqRoutingEntryPtrTemp[TargetEntryIndex].Bus = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.Bus;
    IrqRoutingEntryPtrTemp[TargetEntryIndex].Device = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.Device;
    IrqRoutingEntryPtrTemp[TargetEntryIndex].PirqEntry[0] = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.PirqEntry[0];
    IrqRoutingEntryPtrTemp[TargetEntryIndex].PirqEntry[1] = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.PirqEntry[1];
    IrqRoutingEntryPtrTemp[TargetEntryIndex].PirqEntry[2] = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.PirqEntry[2];
    IrqRoutingEntryPtrTemp[TargetEntryIndex].PirqEntry[3] = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.PirqEntry[3];
    IrqRoutingEntryPtrTemp[TargetEntryIndex].Slot = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.Slot;
    IrqRoutingEntryPtrTemp[TargetEntryIndex].Reserved = RoutingTableAndIPRegisterPtrTemp[TargetEntryIndex].LeagcyIrqRoutingEntry.Reserved;
  }

  //
  // Allocate a Buffer to assemble IrqRouting Header and  PcdControllerDeviceIrqRoutingEntry.
  //
  IrqRoutingTableHeaderPtrTemp = AllocateZeroPool (sizeof (EFI_LEGACY_PIRQ_TABLE_HEADER) + sizeof (EFI_LEGACY_IRQ_ROUTING_ENTRY) * RoutingTableAndIPRegisterCount);
  if (IrqRoutingTableHeaderPtrTemp == NULL) {
    FreePool(*IrqRoutingTableInfoPtr);
    return EFI_UNSUPPORTED;
  }

  CopyMem ( IrqRoutingTableHeaderPtrTemp, 
            &IrqRoutingHeader, 
            sizeof(EFI_LEGACY_PIRQ_TABLE_HEADER));
  (*IrqRoutingTableInfoPtr)->IrqRoutingTableHeaderPtr = IrqRoutingTableHeaderPtrTemp;

  CopyMem ((IrqRoutingTableHeaderPtrTemp + 1),
            IrqRoutingEntryPtrTemp,  
            sizeof (EFI_LEGACY_IRQ_ROUTING_ENTRY) * RoutingTableAndIPRegisterCount);
  (*IrqRoutingTableInfoPtr)->IrqRoutingTablePtr   = (EFI_LEGACY_IRQ_ROUTING_ENTRY  *)(IrqRoutingTableHeaderPtrTemp + 1);
    (*IrqRoutingTableInfoPtr)->MaxRoutingTableCount = RoutingTableAndIPRegisterCount;
  (*IrqRoutingTableInfoPtr)->PirqLinkValuePtr     = (UINT8 *)PcdGetExPtr (&gChipsetPkgTokenSpaceGuid, PcdPirqLinkValueArray);
  FreePool(IrqRoutingEntryPtrTemp );
  FreePool(RoutingTableAndIPRegisterPtrTemp);
  return EFI_SUCCESS;  
}
