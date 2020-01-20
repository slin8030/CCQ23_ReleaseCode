/** @file
 PEI Chipset Services Library.

  Provide Wake Reason for ISCT.

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "ScAccess.h"
#include "ScAccess.h"
#include <PchRegs/PchRegsPcie.h>
#include "IsctWakeReason.h"
#include "PlatformBaseAddresses.h"
#include <ppi/EndOfPeiPhase.h>
#include <pi/PiPeiCis.h>
#include <PeiKscLib.h>
#include <ChipsetSetupConfig.h>
#include <Library/PciCf8Lib.h>
#include <Guid/BxtVariable.h>
#include "ScAccess.h"
#include <FrameworkPei.h>

#include <IndustryStandard/SmBus.h>
#include <IndustryStandard/Pci22.h>
#include <Ppi/AtaController.h>
#include <Guid/Capsule.h>
#include <Ppi/Cache.h>
#include <Ppi/MasterBootMode.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/RecoveryDevice.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/FvLoadFile.h>
#include <Ppi/DeviceRecoveryModule.h>
#include <Ppi/Capsule.h>
#include <Ppi/Reset.h>
#include <Ppi/Stall.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Ppi/MemoryDiscovered.h>
#include <Ppi/RecoveryModule.h>
#include <Ppi/Smbus2.h>
#include <Ppi/FirmwareVolumeInfo.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PciCf8Lib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/PcdLib.h>
#include <Library/SmbusLib.h>
#include <Library/TimerLib.h>
#include <Library/PrintLib.h>
#include <Library/ResetSystemLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PerformanceLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MtrrLib.h>

#define KSC_GETWAKE_STATUS    0x76
#define KSC_CLEARWAKE_STATUS  0x77

EFI_STATUS
EFIAPI
IsctGetWakeReason (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDesc,
  IN VOID                       *Ppi
  );

STATIC EFI_PEI_NOTIFY_DESCRIPTOR mIsctGetWakeReasonNotifyDesc = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiEndOfPeiSignalPpiGuid,
  IsctGetWakeReason
};

UINT8
IsNetworkDevicePME (
  )
/*++

Routine Description:

  Checks for a PCIe Network Device attached to the root ports to see if it caused the PME

Arguments:

  None
  
Returns:

  UINT8 NetworkWakePME - 1 An attached PCIe Network device caused a PME.
                         0 No PME caused by network device

--*/
{
  UINT8                   NetworkWakePME;
  UINT8                   RpFunction;
  UINTN                   RpBase;
  UINTN                   EpBase;
  UINT8                   CapPtr;
  UINT8                   NxtPtr;
  UINT8                   CapID;
  UINT8                   PMCreg;
  UINT8                   PMCSR;

  NetworkWakePME = 0;

  //
  // Scan PCH PCI-EX slots (Root Port) : Device 28 Function 0~3
  //
  for (RpFunction = 0; RpFunction < PCH_PCIE_MAX_ROOT_PORTS; RpFunction ++) {
    RpBase = MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, RpFunction, 0);
    DEBUG ((EFI_D_INFO, "IsctPei: PCI-EX Root Port: 0x%x ...\n", RpFunction));

    if ((MmioRead32 (RpBase + R_PCH_PCIE_SLCTL_SLSTS) & B_PCH_PCIE_SLCTL_SLSTS_PDS) != 0 && MmioRead16 (RpBase + R_PCH_PCIE_ID) == V_PCH_PCIE_VENDOR_ID) {
      //
      // Set WLAN PortBus = 1 to Read Endpoint.
      //
      MmioAndThenOr32(RpBase + R_PCH_PCIE_BNUM_SLT, 0xFF0000FF, 0x00010100);
      EpBase = MmPciAddress (0, 1, 0, 0, 0);

      //
      // A config write is required in order for the device to re-capture the Bus number,
      // according to PCI Express Base Specification, 2.2.6.2
      // Write to a read-only register VendorID to not cause any side effects.
      //
      MmioWrite16 (EpBase + R_PCH_PCIE_ID, 0);

      //
      // Check to see if the Device is a Network Device
      //
      if ((MmioRead16 (EpBase + PCI_CLASSCODE_OFFSET + 0x01) & 0xFF00) == (PCI_CLASS_NETWORK << 8)) { //PCI_CLASS_NETWORK_OTHER
        DEBUG ((EFI_D_INFO, "IsctPei: Found a network device on Root Port - 0x%x and device ID is - 0x%x\n", RpFunction, MmioRead16 (EpBase + R_PCH_PCIE_ID + 0x02)));

        //
        // Find out PMCSR register
        //
        CapPtr = MmioRead8 (EpBase + R_PCH_PCIE_CAPP);
        CapID = MmioRead8 (EpBase + CapPtr);
        NxtPtr = (UINT8) (MmioRead16 (EpBase + CapPtr) >> 8);
        PMCreg = CapPtr;

        while (CapID != 0x01) {
          CapID = MmioRead8 (EpBase + NxtPtr);
          if (CapID == 0x01) {
            PMCreg = NxtPtr;
            break;
          }
          NxtPtr = (UINT8) (MmioRead16 (EpBase + NxtPtr) >> 8);

          if (NxtPtr == 0){
            PMCreg = 0;
            break;
          }
        }

        if (PMCreg != 0) {
          PMCSR = PMCreg + 0x04;

          //
          // Check whether PME enabled. Set NetworkWakePME to 1 if device caused PME.
          //
          if (MmioRead16 (EpBase + PMCSR) & BIT15) {
            NetworkWakePME = 1;
            DEBUG ((EFI_D_INFO, "IsctPei: The network device 0x%x caused the PME\n", MmioRead16 (EpBase + R_PCH_PCIE_ID + 0x02)));
            //
            // Restore bus numbers on the WLAN bridge.
            //
            MmioAnd32(RpBase + R_PCH_PCIE_BNUM_SLT, 0xFF0000FF);
            break;
          }
        }
        //
        // Restore bus numbers on the WLAN bridge.
        //
        MmioAnd32(RpBase + R_PCH_PCIE_BNUM_SLT, 0xFF0000FF);
      }
    }
  }

  return NetworkWakePME;
}

EFI_STATUS
EFIAPI
IsctGetWakeReason (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDesc,
  IN VOID                       *Ppi
  )
/*++

Routine Description:

  Get system Wake Reason and save into CMOS 72/73 for ACPI ASL to use. 

Arguments:

  PeiServices       General purpose services available to every PEIM.
  
Returns:

--*/
{
//  EFI_STATUS                  Status;
  UINT16					  AcpiBase = 0;
  UINT16                      PM1STS;
//  UINT16                      USB29VID;
//  UINT16                      USB29STS;
//  UINT16                      xHCIVID;
//  UINT16                      xHCISTS;
  UINT8                       WakeReason;

  WakeReason = 0;
  //
  // Initialize base address for Power Management 
  //
  AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  DEBUG ((EFI_D_INFO, "IsctGetWakeReason: AcpiBase = %x\n", AcpiBase));


  PM1STS  = IoRead16(AcpiBase + R_ACPI_PM1_STS);
  PM1STS &= (B_ACPI_PM1_STS_PWRBTN | B_ACPI_PM1_STS_RTC | BIT14);

  //
  // Check PM1_STS
  //
  DEBUG ((EFI_D_INFO, "IsctPei: PM1_STS Value= %x \n", PM1STS));  
  DEBUG ((EFI_D_INFO, "  Bit set in PM1_STS: \n"));  
  switch (PM1STS){
    case B_ACPI_PM1_STS_PWRBTN:
      WakeReason |= 0x01; //User event
  DEBUG ((EFI_D_INFO, "    PowerButton\n"));  
      break;
    case B_ACPI_PM1_STS_RTC:
      WakeReason |= 0x04; //RTC Timer
  DEBUG ((EFI_D_INFO, "    RTC Timer\n")); 
      break;
    case BIT14:
      WakeReason |= 0x08; //Due to PME
  DEBUG ((EFI_D_INFO, "    PME\n")); 
      break;
    default:
      WakeReason = 0;
  DEBUG ((EFI_D_INFO, "    Unknown\n")); 
      break;
  }

  //
  // EHCI PME : Offset 0x54(15)
  //
/*
  USB29VID = MmioRead16 (
             MmPciAddress (
               0,
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_PCH_USB,
               PCI_FUNCTION_NUMBER_PCH_EHCI,
               R_PCH_USB_VENDOR_ID
               ));

  USB29STS = MmioRead16 (
             MmPciAddress (
               0,
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_PCH_USB,
               PCI_FUNCTION_NUMBER_PCH_EHCI,
               R_PCH_EHCI_PWR_CNTL_STS
               )) & (B_PCH_EHCI_PWR_CNTL_STS_PME_STS | B_PCH_EHCI_PWR_CNTL_STS_PME_EN);

  if (USB29VID != 0xFFFF && USB29VID != 0){
    if (USB29STS == 0x8100){
      DEBUG ((EFI_D_INFO, "IsctPei: EHCI Wake\n"));  
      WakeReason |= 0x01; //User event
    }
  }

  //
  // xHCI PME : Offset 0x74(15)
  //
  xHCIVID = MmioRead16 (
           MmPciAddress (
             0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_XHCI,
             PCI_FUNCTION_NUMBER_XHCI,
             R_PCH_USB_VENDOR_ID
             ));

  xHCISTS = MmioRead16 (
           MmPciAddress (
             0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_XHCI,
             PCI_FUNCTION_NUMBER_XHCI,
             R_XHCI_PWR_CNTL_STS
             )) & (B_XHCI_PWR_CNTL_STS_PME_STS | B_XHCI_PWR_CNTL_STS_PME_EN);

     
  if (xHCIVID != 0xFFFF && xHCIVID != 0){
     if (xHCISTS == 0x8100){
        DEBUG ((EFI_D_INFO, "IsctPei: xHCI Wake\n"));  
        WakeReason |= 0x01;		//User Event 0x08; //PME
     }
  }

  //
  // Check for Network Device PME from PCIe if PME wake reason
  //
    DEBUG ((EFI_D_INFO, "IsctPei: PME wake reason- check if from network device\n"));
    if(IsNetworkDevicePME())
    {
      WakeReason |= BIT3;	//BIT4;
      DEBUG ((EFI_D_INFO, "IsctPei: IsNetworkDevicePME() returned Yes\n"));
    }

  //
  // Set Isct Wake Reason
  //     
  DEBUG ((EFI_D_INFO, "IsctPei: Wake Reason reported to Agent= %x \n", WakeReason));                             

//
// Work around - Store Wake reason at EC offset 0x178 for ASL access
// Todo: Use Reserved ACPI NVS so that Win 8 will consider when Fast start up is enabled for Shutdown
//
    Status = SendKscCommand (0x81);    // Send Write command
    if(!EFI_ERROR(Status)) {
      Status = SendKscData (0xB2);	// EC Address location
      if(!EFI_ERROR(Status)) {
         Status = SendKscData (WakeReason);	// Wake reason
      }
    }
*/
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
IsctPeiEntryPoint (
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
/*++

Routine Description:

  Set up 

Arguments:

  PeiServices       General purpose services available to every PEIM.
  
Returns:

--*/
{
  EFI_STATUS                      Status;

  UINT8                           IsctEnabled;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *PeiReadOnlyVarPpi = NULL;
  UINTN                           VarSize;
  CHIPSET_CONFIGURATION            *SystemConfiguration;


  DEBUG ((EFI_D_INFO, "IsctPei Entry Point\n"));
  IsctEnabled = 0;

  //
  // Locate PEI Read Only Variable PPI.
  //
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
                             0,
                             NULL,
                             (VOID **)&PeiReadOnlyVarPpi
                             );
  if (Status == EFI_SUCCESS) {
    VarSize = PcdGet32 (PcdSetupConfigSize);
    SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VarSize);
    ASSERT (SystemConfiguration != NULL);
    
    Status = PeiReadOnlyVarPpi->GetVariable ( 
                                  PeiReadOnlyVarPpi, 
                                  SETUP_VARIABLE_NAME, 
                                  &gSystemConfigurationGuid,
                                  NULL,
                                  &VarSize,
                                  SystemConfiguration
                                  );
    if (Status == EFI_SUCCESS) {
      if (SystemConfiguration->IsctConfiguration != 0) {
        IsctEnabled = 0x01;
      }
    }
    FreePool (SystemConfiguration);
  }

  if (IsctEnabled == 0) {
    DEBUG ((EFI_D_INFO, "Isct Disabled\n"));
    return EFI_SUCCESS;
  }
  else {
    Status = PeiServicesNotifyPpi (&mIsctGetWakeReasonNotifyDesc);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_INFO, "IsctPei: Notify  EFI_PEI_END_OF_PEI_PHASE_PPI_GUID Status = %x \n", Status));
  }
  
  return EFI_SUCCESS;
}
