/** @file
 PEI Chipset Services Library.

 This file contains only one function that is PeiCsSvcSetPlatformHardwareSwitch().
 The function PeiCsSvcSetPlatformHardwareSwitch() use chipset services to feedback 
 its proprietary settings of the hardware switches.

***************************************************************************
* Copyright (c) 2014-2017, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <ChipsetSetupConfig.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Guid/PlatformHardwareSwitch.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

//
// helper function prototypes
//
EFI_STATUS
PlatformHardwareSwitchInitWithSetupVar (
  IN PLATFORM_HARDWARE_SWITCH   *PlatformHardwareSwitch
  );

/**
  Initialize platform hardware switch from SystemConfiguration variable.

  @param[in]  PlatformHardwareSwitch     Pointer to PLATFORM_HARDWARE_SWITCH.

  @retval EFI_SUCCESS                 Initialize platform hardware Switch value from SystemConfiguration variable success.
  @retval EFI_NOT_FOUND               The gPeiReadOnlyVariablePpiGuid not been install.
**/
EFI_STATUS
PlatformHardwareSwitchInitWithSetupVar (
  IN PLATFORM_HARDWARE_SWITCH   *PlatformHardwareSwitch
  )
{
  EFI_STATUS                       Status;
//  VOID                             *SetupVariable;
  CHIPSET_CONFIGURATION            SystemConfiguration;
  UINTN                            VariableSize;
//  UINTN                            DataSize;
  EFI_STATUS                       VariableStatus;
  EFI_GUID                         mSystemConfigurationGuid  = SYSTEM_CONFIGURATION_GUID;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *Variable;
//[-start-161228-IB04530802-add]//
  UINT8                            PortIndex;
  UINT64 ConfigMask;
//[-end-161228-IB04530802-add]//

  //
  // Since BDW platform has change PCH policy from DXE to PEI, we shouldn't use the statement;
  // Otherwise, system will fail to get PlatformHardwareSwitch HOB when S3 resume.
  //
  //
  //EFI_BOOT_MODE                    BootMode;
  //
  //Status = PeiServicesGetBootMode (&BootMode);
  //if (!EFI_ERROR (Status) && (BootMode == BOOT_ON_S3_RESUME)) {
  //  return EFI_SUCCESS;
  //}
  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&Variable
             );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  VariableSize = sizeof (CHIPSET_CONFIGURATION);
//  DataSize = PcdGet32 (PcdSetupConfigSize);
//  SetupVariable = AllocateZeroPool (DataSize);
  //
  // Get desired ratio from NVRAM variable
  //
  VariableStatus = Variable->GetVariable (
                               Variable,
                               L"Setup",
                               &mSystemConfigurationGuid,
                               NULL,
                               &VariableSize,//&DataSize,
                               &SystemConfiguration//SetupVariable
                               );
  
//  CopyMem (&SystemConfiguration, SetupVariable, sizeof(CHIPSET_CONFIGURATION));

  
  /*
  Sample code to set Platform Hardware Switch by platform dessign
  All ports is enabled by default. All PCIE ports is configured to Auto
  Below sample code disable Sata PORT4, USB PORT5 and PCIE PORT6.

  PlatformHardwareSwitch->SataPortSwitch.Config.Port04En = PORT_DISABLE;
  PlatformHardwareSwitch->UsbPortSwitch.Config.Port05En = PORT_DISABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port06En = PORT_DISABLE;
  */
  
  //
  // Sata Port
  //
//[-start-161228-IB04530802-modify]//
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port00En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port01En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port02En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port03En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port04En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port05En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port06En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port07En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port08En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port09En = PORT_ENABLE;

  for (PortIndex = 0; PortIndex < SC_MAX_SATA_PORTS; PortIndex++) {
      ConfigMask = LShiftU64 ((UINT64) PORT_ENABLE, PortIndex);
    if (SystemConfiguration.SataPort[PortIndex] && ((PlatformHardwareSwitch->SataPortSwitch.CONFIG >> PortIndex) & 0x1)) {
        PlatformHardwareSwitch->SataPortSwitch.CONFIG |= ConfigMask;
    } else {
        PlatformHardwareSwitch->SataPortSwitch.CONFIG &= ~ConfigMask;
    }
  }
//[-end-161228-IB04530802-modify]//

  //
  // USB Port
  //  
//[-start-161228-IB04530802-modify]//
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port00En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port01En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port02En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port03En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port04En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port05En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port06En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port07En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port08En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port09En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port10En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port11En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port12En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port13En = PORT_ENABLE;

#if (ENBDT_PF_ENABLE == 1)

  // USB 2.0 Ports
  for (PortIndex = 0; PortIndex < SC_MAX_USB2_PORTS; PortIndex++) {
    ConfigMask = LShiftU64 ((UINT64) PORT_ENABLE, PortIndex);
    if (SystemConfiguration.ScUsbPort[PortIndex] && ((PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG >> PortIndex) & 0x1)) {
      PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG |= ConfigMask;
    } else {
      PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG &= ~ConfigMask;
    }
  }

  // USB 3.0 Ports, we don't have USB 3.0 setup items, use platform hardware switch directly
  for (PortIndex = 0; PortIndex < SC_MAX_USB3_PORTS; PortIndex++) {
    ConfigMask = LShiftU64 ((UINT64) PORT_ENABLE, PortIndex);
    if (((PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG >> PortIndex) & 0x1)) {
      PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG |= ConfigMask;
    } else {
      PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG &= ~ConfigMask;
    }
  }
  
#else

  // USB 2.0 Ports
  for (PortIndex = 0; PortIndex < SC_MAX_USB2_PORTS; PortIndex++) {
    ConfigMask = LShiftU64 ((UINT64) PORT_ENABLE, PortIndex);
    if ((PortIndex < 3) && (SystemConfiguration.ScUsbPort[PortIndex] && ((PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG >> PortIndex) & 0x1))) {
      PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG |= ConfigMask;
    } else {
      PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG &= ~ConfigMask;
    }
  }

  // USB 3.0 Ports, we don't have USB 3.0 setup items, use platform hardware switch directly
  for (PortIndex = 0; PortIndex < SC_MAX_USB3_PORTS; PortIndex++) {
    ConfigMask = LShiftU64 ((UINT64) PORT_ENABLE, PortIndex);
    if ((PortIndex < 2) && ((PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG >> PortIndex) & 0x1)) {
      PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG |= ConfigMask;
    } else {
      PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG &= ~ConfigMask;
    }
  }
  
#endif
//[-end-161228-IB04530802-modify]//
  
  //
  // PCIE
  //    
//[-start-161228-IB04530802-modify]//
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port00En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port01En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port02En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port03En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port04En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port05En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port06En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port07En = PORT_ENABLE;   

  if (SystemConfiguration.PcieRootPortEn[0] && PlatformHardwareSwitch->PciePortSwitch.Config.Port00En) {
    if (PlatformHardwareSwitch->PciePortSwitch.Config.Port00En == PCIE_PORT_AUTO) {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port00En = PCIE_PORT_AUTO;
    } else {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port00En = PORT_ENABLE;
    }
  } else {
    PlatformHardwareSwitch->PciePortSwitch.Config.Port00En = PORT_DISABLE;
  }

  if (SystemConfiguration.PcieRootPortEn[1] && PlatformHardwareSwitch->PciePortSwitch.Config.Port01En) {
    if (PlatformHardwareSwitch->PciePortSwitch.Config.Port01En == PCIE_PORT_AUTO) {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port01En = PCIE_PORT_AUTO;
    } else {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port01En = PORT_ENABLE;
    }
  } else {
    PlatformHardwareSwitch->PciePortSwitch.Config.Port01En = PORT_DISABLE;
  }

  if (SystemConfiguration.PcieRootPortEn[2] && PlatformHardwareSwitch->PciePortSwitch.Config.Port02En) {
    if (PlatformHardwareSwitch->PciePortSwitch.Config.Port02En == PCIE_PORT_AUTO) {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port02En = PCIE_PORT_AUTO;
    } else {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port02En = PORT_ENABLE;
    }
  } else {
    PlatformHardwareSwitch->PciePortSwitch.Config.Port02En = PORT_DISABLE;
  }

  if (SystemConfiguration.PcieRootPortEn[3] && PlatformHardwareSwitch->PciePortSwitch.Config.Port03En) {
    if (PlatformHardwareSwitch->PciePortSwitch.Config.Port03En == PCIE_PORT_AUTO) {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port03En = PCIE_PORT_AUTO;
    } else {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port03En = PORT_ENABLE;
    }
  } else {
    PlatformHardwareSwitch->PciePortSwitch.Config.Port03En = PORT_DISABLE;
  }

  if (SystemConfiguration.PcieRootPortEn[4] && PlatformHardwareSwitch->PciePortSwitch.Config.Port04En) {
    if (PlatformHardwareSwitch->PciePortSwitch.Config.Port04En == PCIE_PORT_AUTO) {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port04En = PCIE_PORT_AUTO;
    } else {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port04En = PORT_ENABLE;
    }
  } else {
    PlatformHardwareSwitch->PciePortSwitch.Config.Port04En = PORT_DISABLE;
  }

  if (SystemConfiguration.PcieRootPortEn[5] && PlatformHardwareSwitch->PciePortSwitch.Config.Port05En) {
    if (PlatformHardwareSwitch->PciePortSwitch.Config.Port05En == PCIE_PORT_AUTO) {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port05En = PCIE_PORT_AUTO;
    } else {
      PlatformHardwareSwitch->PciePortSwitch.Config.Port05En = PORT_ENABLE;
    }
  } else {
    PlatformHardwareSwitch->PciePortSwitch.Config.Port05En = PORT_DISABLE;
  }
//[-end-161228-IB04530802-modify]//

  return EFI_SUCCESS;
}

/**
  To feedback its proprietary settings of the hardware switches.
  if you wants it to be not auto detected while it is set to 'PORT_ENABLE'. 
  That means, you has 3 choices to a port of PCI-e Ports - 'PORT_DISABLE', 'PCIE_PORT_AUTO', 
  and 'PORT_ENABLE'. For Usb and Sata ports, only two options are available -'PORT_DISABLE' and 'PORT_ENABLE'.

  @param[out]  PlatformHardwareSwitch    A Pointer to PLATFORM_HARDWARE_SWITCH.

  @retval      EFI_SUCCESS                Set hardware switch success.
  @retval      EFI_NOT_FOUND              Locate ReadOnlyVariablePpi failed.
**/
EFI_STATUS
SetPlatformHardwareSwitch (
  OUT PLATFORM_HARDWARE_SWITCH          **PlatformHardwareSwitchDptr
  )
{
  EFI_STATUS                            Status;
  PLATFORM_HARDWARE_SWITCH              *PlatformHardwareSwitch;

  PlatformHardwareSwitch = *PlatformHardwareSwitchDptr;

  //
  // Default Enable all ports
  //

  PlatformHardwareSwitch->SataPortSwitch.CONFIG = 0xFFFFFFFFFFFFFFFF;
//[-start-161228-IB04530802-modify]//
  PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG = 0xFFFFFFFFFFFFFFFF;
  PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG = 0xFFFFFFFFFFFFFFFF;
//[-end-161228-IB04530802-modify]//
//  //
//  // Default set PCIE port configure to Auto mode
//  //
//  PlatformHardwareSwitch->PciePortSwitch.CONFIG = 0xAAAAAAAAAAAAAAAA;
  //
  // because PchPlatformPolicy struct has changed (CougarPoint), a default "Auto(0x2)" will became "Disable(0x0)" when type convert
  // So default set PCIE port configure to Enable mode
  //
//[-start-170317-IB15550025-modify]//
//[-start-170714-IB07400890-modify]//
//  PlatformHardwareSwitch->PciePortSwitch.CONFIG = 0xAAAAAAAAAAAAAAAA;
  PlatformHardwareSwitch->PciePortSwitch.CONFIG = 0x5555555555555555;
//[-end-170714-IB07400890-modify]//
//[-end-170317-IB15550025-modify]//

  /*
  Sample code to set Platform Hardware Switch by platform dessign
  All ports is enabled by default. All PCIE ports is configured to Auto
  Below sample code disable Sata PORT4, USB PORT5 and PCIE PORT6.

  PlatformHardwareSwitch->SataPortSwitch.Config.Port04En = PORT_DISABLE;
  PlatformHardwareSwitch->UsbPortSwitch.Config.Port05En = PORT_DISABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port06En = PORT_DISABLE;
  */
  
  //
  // Sata Port
  //
//[-start-161228-IB04530802-modify]//
  PlatformHardwareSwitch->SataPortSwitch.Config.Port00En = PORT_ENABLE;
  PlatformHardwareSwitch->SataPortSwitch.Config.Port01En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port02En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port03En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port04En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port05En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port06En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port07En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port08En = PORT_ENABLE;
//  PlatformHardwareSwitch->SataPortSwitch.Config.Port09En = PORT_ENABLE;
  
  //
  // USB Port
  //  
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port00En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port01En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port02En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port03En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port04En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port05En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port06En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbHsPortSwitch.Config.Port07En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port08En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port09En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port10En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port11En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port12En = PORT_ENABLE;
//  PlatformHardwareSwitch->UsbPortSwitch.Config.Port13En = PORT_ENABLE;

  PlatformHardwareSwitch->UsbSsPortSwitch.Config.Port00En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbSsPortSwitch.Config.Port01En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbSsPortSwitch.Config.Port02En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbSsPortSwitch.Config.Port03En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbSsPortSwitch.Config.Port04En = PORT_ENABLE;
  PlatformHardwareSwitch->UsbSsPortSwitch.Config.Port05En = PORT_ENABLE;

//[-start-170317-IB15550025-modify]//
//[-start-170714-IB07400890-modify]//
  //
  // PCIE
  //    
  PlatformHardwareSwitch->PciePortSwitch.Config.Port00En = PORT_ENABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port01En = PORT_ENABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port02En = PORT_ENABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port03En = PORT_ENABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port04En = PORT_ENABLE;
  PlatformHardwareSwitch->PciePortSwitch.Config.Port05En = PORT_ENABLE;
//[-end-170714-IB07400890-modify]//
//[-end-170317-IB15550025-modify]//
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port06En = PORT_ENABLE;
//  PlatformHardwareSwitch->PciePortSwitch.Config.Port07En = PORT_ENABLE; 
//[-end-161228-IB04530802-modify]//

  //
  // Initialize platform hardware switch from SystemConfiguration variable.
  //
  Status = PlatformHardwareSwitchInitWithSetupVar (PlatformHardwareSwitch);

  return Status;
}

