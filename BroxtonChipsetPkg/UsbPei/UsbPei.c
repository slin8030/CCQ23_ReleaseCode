/** @file

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
/*++
  This file contains an 'Intel Pre-EFI Module' and is licensed  
  for Intel CPUs and Chipsets under the terms of your license   
  agreement with Intel or your vendor.  This file may be        
  modified by the user, subject to additional terms of the      
  license agreement                                             
--*/
/** @file
  Implementation of Usb Controller PPI.

Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include "UsbPei.h"

//[-start-151216-IB07220025-add]//
BOOLEAN mPcdEhciRecoveryEnabled = TRUE;
//[-end-151216-IB07220025-add]//

EFI_STATUS
EFIAPI
GetEhciController (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN PEI_USB_CONTROLLER_PPI         *This,
  IN UINT8                          UsbControllerId,
  IN UINTN                          *ControllerType,
  IN UINTN                          *BaseAddress
  );
//
// must add this callback to reset EHCI controller or USB KB/Mouse will fail in BDS
//  
STATIC
EFI_STATUS
EFIAPI
EndOfPeiPpiNotifyCallback (
  IN EFI_PEI_SERVICES                    **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR            *NotifyDescriptor,
  IN VOID                                 *Ppi
  );

//
// Globals
//
PEI_USB_CONTROLLER_PPI mUsbControllerPpi[1] = {{GetEhciController}};

EFI_PEI_PPI_DESCRIPTOR mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiUsbControllerPpiGuid,
  NULL
};

//[-start-151216-IB07220025-modify]//
UINTN mIch7EhciPciReg[PCI_MAX_EHCI_USB_CONTROLLERS] = {
  PCI_LIB_ADDRESS (0, 21, 0, 0),
};
//[-end-151216-IB07220025-modify]//

static
EFI_PEI_NOTIFY_DESCRIPTOR mResetEHCI[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiEndOfPeiSignalPpiGuid,
    EndOfPeiPpiNotifyCallback
  }
};



/**
 Register notify ppi to reset the EHCI.
 CAUTION!!! This function may NOT be called due to the
 MAX_PPI_DESCRIPTORS(in PeiCore.h) exceed 64 and caused
 EHCI malfunction in DXE phase. For this situation, put
 the EHCI HC reset code into DXE chipset init driver

 @param[in]         PeiServices         Describes the list of possible PEI Services.
 @param[in]         NotifyDescriptor    A pointer to notification structure this PEIM registered on install.
 @param[in]         Ppi                 A pointer to the PPI interface in notify descriptor.
                    
 @retval            EFI_SUCCESS         Procedure complete.
 @retval            others              Error occurs
*/
STATIC
EFI_STATUS
EFIAPI
EndOfPeiPpiNotifyCallback (
  IN EFI_PEI_SERVICES                    **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR            *NotifyDescriptor,
  IN VOID                                 *Ppi
  )
{
  EFI_STATUS          Status;
  EFI_PEI_STALL_PPI  *StallPpi;
  UINTN               Index;
  UINT8              *BaseAddress;
  UINT32              Command;
  
  Status = (**PeiServices).LocatePpi (
                             (CONST EFI_PEI_SERVICES**)PeiServices,
                             &gEfiPeiStallPpiGuid,
                             0,
                             NULL,
                             (VOID **)&StallPpi
                             );
  
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Reset the HC
  //
//[-start-151216-IB07220025-modify]//
  for (Index = 0; Index < PCI_MAX_EHCI_USB_CONTROLLERS; Index ++) {
    BaseAddress = (UINT8*) (PciRead32 (mIch7EhciPciReg[Index] | 0x10) & 0xFFFFFFF0);
//[-end-151216-IB07220025-modify]//
    BaseAddress = BaseAddress + (*(UINT32*)(UINTN)BaseAddress & 0xff);
    //
    // Halt HC first
    //
    Command = *(UINT32*)(UINTN)BaseAddress;
    Command &= ~0x01;
    *(UINT32*)(UINTN)BaseAddress = Command;
    //
    // Stall 1 millisecond
    //
    StallPpi->Stall (
                (CONST EFI_PEI_SERVICES **)PeiServices,
                StallPpi,
                1000
                );
    //
    // HCReset
    //
    Command = *(UINT32*)(UINTN)BaseAddress;
    Command |= 0x02;
    *(UINT32*)(UINTN)BaseAddress = Command;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
EnableEhciController (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN ICH_EHCI_DEVICE          *PeiIchEhciDev,
  IN UINT8                    UsbControllerId
  );

/**
  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS            PPI successfully installed

**/
EFI_STATUS
EFIAPI
PeimInitializeIchUsb (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS              Status;
  UINTN                   i;
  EFI_PHYSICAL_ADDRESS    AllocateAddress;
  ICH_EHCI_DEVICE         *PeiIchEhciDev;

  Status = (**PeiServices).AllocatePages (
                             PeiServices,
                             EfiBootServicesCode,
                             1,
                             &AllocateAddress
                             );

  ASSERT_EFI_ERROR (Status);

//[-start-151216-IB07220025-modify]//
  if (mPcdEhciRecoveryEnabled) {
//[-end-151216-IB07220025-modify]//
    //
    // EHCI recovery is enabled
    //
    PeiIchEhciDev = (ICH_EHCI_DEVICE *)((UINTN)AllocateAddress);
    ZeroMem (PeiIchEhciDev, sizeof(PeiIchEhciDev));
    
    PeiIchEhciDev->Signature            = PEI_ICH_EHCI_SIGNATURE;
    CopyMem(&(PeiIchEhciDev->UsbControllerPpi), &mUsbControllerPpi[0], sizeof(PEI_USB_CONTROLLER_PPI));
    CopyMem(&(PeiIchEhciDev->PpiList), &mPpiList, sizeof(mPpiList));
    PeiIchEhciDev->PpiList.Ppi          = &PeiIchEhciDev->UsbControllerPpi;
    
    //
    // Assign resources and enable Ehci controllers
    //
    for (i = 0; i < PCI_MAX_EHCI_USB_CONTROLLERS; i++) {
//[-start-151216-IB07220025-modify]//
      PeiIchEhciDev->MmioBase[i] = 0xfc300000 + 0x2000 * i;
//[-end-151216-IB07220025-modify]//
      Status = EnableEhciController ((EFI_PEI_SERVICES **) PeiServices, PeiIchEhciDev, (UINT8)i);
      ASSERT_EFI_ERROR (Status);
    }   
    //
    // Install USB Controller PPI
    //
    Status = (**PeiServices).InstallPpi (
                               PeiServices, 
                               &PeiIchEhciDev->PpiList
                               );
    
    ASSERT_EFI_ERROR (Status);
    Status = (**PeiServices).NotifyPpi (PeiServices, &mResetEHCI[0]);

    ASSERT_EFI_ERROR (Status);
  } else {
    ASSERT_EFI_ERROR (FALSE);
  }

  return Status;
}

EFI_STATUS
EFIAPI
GetEhciController (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN PEI_USB_CONTROLLER_PPI         *This,
  IN UINT8                          UsbControllerId,
  IN UINTN                          *ControllerType,
  IN UINTN                          *BaseAddress
  )
{
  ICH_EHCI_DEVICE         *PeiIchEhciDev;

  PeiIchEhciDev = ICH_EHCI_DEVICE_FROM_THIS (This);
  
  if (UsbControllerId >= PCI_MAX_EHCI_USB_CONTROLLERS) {
    return EFI_INVALID_PARAMETER;
  }
  
//[-start-151216-IB07220025-modify]//
  *ControllerType = PEI_XHCI_CONTROLLER;
//[-end-151216-IB07220025-modify]//

  *BaseAddress = PeiIchEhciDev->MmioBase[UsbControllerId];

  return EFI_SUCCESS;
}

EFI_STATUS
EnableEhciController (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN ICH_EHCI_DEVICE          *PeiIchEhciDev,
  IN UINT8                    UsbControllerId
  )
{
  UINTN                     BaseAddress;
  UINT16                    CmdReg;

  if (UsbControllerId >= PCI_MAX_EHCI_USB_CONTROLLERS) {
    return EFI_INVALID_PARAMETER;
  }
  
  BaseAddress =  PeiIchEhciDev->MmioBase[UsbControllerId];
  
//[-start-151216-IB07220025-modify]//
  //
  // Assign base address register
  //
  PciWrite32 (mIch7EhciPciReg[UsbControllerId] | 0x10, (UINT32) BaseAddress);
 
  //
  // Enable ICH Ehci register
  //
  CmdReg = PciRead16 (mIch7EhciPciReg[UsbControllerId] | 0x04);

  CmdReg = (UINT16) (CmdReg | BIT1 | BIT2);

  PciWrite16 (mIch7EhciPciReg[UsbControllerId] | 0x04, CmdReg);
//[-end-151216-IB07220025-modify]//

  return EFI_SUCCESS;  
}
