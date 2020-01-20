/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "UnLockedBarHandleSmm.h"

EFI_SMM_VARIABLE_PROTOCOL           *mSmmVariable;

EFI_STATUS
UnLockedBarHandleEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
	EFI_STATUS                            Status;           
  VOID                                  *PciEnumerationCompleteProtocol;
  EFI_SMM_BASE2_PROTOCOL                *SmmBase; 
  BOOLEAN                               InSmm;

  SmmBase = NULL;

  Status = gBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, (VOID **)&SmmBase);
  ASSERT_EFI_ERROR (Status);

  Status = SmmBase->InSmm (SmmBase, &InSmm);
  ASSERT_EFI_ERROR (Status);  
  
  if (!InSmm) {
    return EFI_SUCCESS;
  } else {
    //
    // In SMM
    //
    Status = gSmst->SmmLocateProtocol (&gEfiSmmVariableProtocolGuid, NULL, (VOID**)&mSmmVariable);    
    ASSERT_EFI_ERROR (Status);
    
    Status = gBS->LocateProtocol(&gEfiPciEnumerationCompleteProtocolGuid, NULL, &PciEnumerationCompleteProtocol);
    ASSERT_EFI_ERROR (Status);
      
    if (!EFI_ERROR(Status)) {
      SaveUnLockedBar();
    }
  }
  return EFI_SUCCESS;
}

BOOLEAN
IsSaveDynamicBar (
  EFI_PCI_IO_PROTOCOL    *PciIo,
  UINTN                  BarIndex
  )
{
  BOOLEAN              Save;
  PCI_TYPE00           Pci;
  UINTN                Segment;
  UINTN                Bus;
  UINTN                Device;
  UINTN                Function;
 
  Save = TRUE;  
  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (PCI_TYPE00) / sizeof (UINT32), &Pci);
  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);

  // Skip PCI Bridge and CardBus Bridge
  if(IS_PCI_BRIDGE (&Pci) || IS_CARDBUS_BRIDGE (&Pci)) {
    Save = FALSE;
  }

  // Skip empty Bar
  if (Pci.Device.Bar[BarIndex] == 0) {
    Save = FALSE;
  }
  
  // Skip IO Space
  if ((Pci.Device.Bar[BarIndex] != 0) && ((Pci.Device.Bar[BarIndex] & IO_SPACE_INDICATOR) == IO_SPACE_INDICATOR)) {
    Save = FALSE;
  }
  return Save;
}


EFI_STATUS
EFIAPI
SaveUnLockedBar (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  UINTN                                 BarIndex;
  UINTN                                 NoHandles;
  EFI_HANDLE                            Handle;
  EFI_HANDLE                            *HandleBuffer;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  PCI_TYPE00                            Pci;
  UINTN                                 Segment;
  UINTN                                 Bus;
  UINTN                                 Device;
  UINTN                                 Function;
  UINTN                                 DynamicBarCount;
  DEVICE_BASE_ADDRESS                   *DynamicBar;
  //
  // Note: Fixed BARs are platform dependent.
  //
//[-start-160818-IB03090432-modify]//
  DEVICE_BASE_ADDRESS                   FixedBar[] = {
//    {0, PCI_DEVICE_NUMBER_PCH_LPC,  PCI_FUNCTION_NUMBER_PCH_LPC,  R_LPC_PMC_BASE ,  0},         
//    {0, PCI_DEVICE_NUMBER_PCH_LPC,  PCI_FUNCTION_NUMBER_PCH_LPC,  R_LPC_IO_BASE  ,  0},
//    {0, PCI_DEVICE_NUMBER_PCH_LPC,  PCI_FUNCTION_NUMBER_PCH_LPC,  R_LPC_ILB_BASE ,  0},
    {0, PCI_DEVICE_NUMBER_SPI,      PCI_FUNCTION_NUMBER_SPI,      R_SPI_BASE         ,  0},
  };
//[-end-160818-IB03090432-modify]//

  Handle = NULL;
  DynamicBar = NULL;
  DynamicBarCount = 0;

  //
  // Save Fixed BARs
  //  
  for (Index = 0; Index < (sizeof (FixedBar) / sizeof (DEVICE_BASE_ADDRESS)); Index++) {
    FixedBar[Index].Bar = MmioRead32 (
                            MmPciAddress (
                            0,
                            FixedBar[Index].Bus,
                            FixedBar[Index].Dev,
                            FixedBar[Index].Func,
                            FixedBar[Index].Offset 
                            )
                          );
  }
  
  Status = mSmmVariable->SmmSetVariable (
                  FIXED_BAR_VARIABLE_NAME,
                  &gSaveUnLockedBarVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof(FixedBar),
                  FixedBar
                  ); 
  ASSERT_EFI_ERROR (Status);
  
  ///
  /// Get Dynamic bar via PciIo
  ///    
  Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiPciIoProtocolGuid,
                NULL,
                &NoHandles,
                &HandleBuffer
                );
  ASSERT_EFI_ERROR (Status);
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    NoHandles * PCI_MAX_BAR * sizeof (DEVICE_BASE_ADDRESS),
                    &DynamicBar
                    );  
    ASSERT_EFI_ERROR (Status);
    
    for (Index = 0; Index < NoHandles; Index++) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiPciIoProtocolGuid,
                      &PciIo
                      );
      if (EFI_ERROR (Status)) {
        continue;
      }
      
      PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (PCI_TYPE00) / sizeof (UINT32), &Pci);
      PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
      
      for (BarIndex = 0; BarIndex < PCI_MAX_BAR; BarIndex++) {
        if (IsSaveDynamicBar(PciIo, BarIndex)) {
          DynamicBar[DynamicBarCount].Bus = (UINT8)Bus;
          DynamicBar[DynamicBarCount].Dev = (UINT8)Device;
          DynamicBar[DynamicBarCount].Func = (UINT8)Function;
          DynamicBar[DynamicBarCount].Offset = (UINT8) (PCI_BASE_ADDRESSREG_OFFSET + BarIndex * 4);
          DynamicBar[DynamicBarCount].Bar = Pci.Device.Bar[BarIndex];
          DynamicBarCount++;
        }
      }
    }
  }
  
  //
  // Save Dynamic BARs
  //
  Status = mSmmVariable->SmmSetVariable (
                  DYNAMIC_BAR_VARIABLE_NAME,
                  &gSaveUnLockedBarVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  DynamicBarCount * sizeof(DEVICE_BASE_ADDRESS),
                  DynamicBar
                  );  
  ASSERT_EFI_ERROR (Status);

  Status = gSmst->SmmInstallProtocolInterface (
             &Handle,
             &gSaveUnlockedBarDoneProtocolGuid,
             EFI_NATIVE_INTERFACE,
             NULL
             );  
  ASSERT_EFI_ERROR (Status);
  
  return EFI_SUCCESS;
}

