/** @file
  Define the PCI devices which be skipped add to the PCI root bridge.

;******************************************************************************
;* Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeOemSvcKernelLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

/*++

Todo:
  Define the relateaed data.
  

--*/
PCI_SKIP_TABLE      mPciSkipTable[] = {
  {
    0x8086,
    0x5A9C  //D15:F1 HECI2
  },
  {
    0x8086,
    0x5A9E  //D15:F2 HECI3
  },
  {
    0xffff,
    0xffff
  }
};

/**
  This OemService provides OEM to define the PCI devices which be skipped add to the PCI root bridge. 
  OEM can implement this function by adding the Vendor ID and Device ID of skipping device to PciSkipTable.

  @param[out]  *PciSkipTableCount    Point to the count of PCI skipping devices.
  @param[out]  *PciSkipTable         Point to the PCI Skip Table.
  
  @retval      EFI_UNSUPPORTED       Returns unsupported by default.
  @retval      EFI_SUCCESS           Install PCI skip table success.
  @retval      EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed. 
  @retval      Others                Base on OEM design.
**/
EFI_STATUS 
OemSvcInstallPciSkipTable (
  OUT UINTN                          *PciSkipTableCount,
  OUT PCI_SKIP_TABLE                 **PciSkipTable
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  *PciSkipTableCount = (sizeof(mPciSkipTable) / sizeof(PCI_SKIP_TABLE));
  (*PciSkipTable)   = AllocateZeroPool (sizeof (mPciSkipTable));
  CopyMem ((*PciSkipTable), mPciSkipTable, sizeof (mPciSkipTable));

  return EFI_MEDIA_CHANGED;
}

