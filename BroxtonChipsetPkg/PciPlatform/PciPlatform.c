/*++

Copyright (c)  2003 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    PciPlatform.c

Abstract:
--*/


#include "PciPlatform.h"
#include "ScAccess.h"
#include "SaCommonDefinitions.h"
#include "PlatformBootMode.h"

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/CpuIo.h>
#include <Protocol/PciIo.h>
//#include <Guid/SetupVariable.h>
#include <Protocol/PciRootBridgeIo.h>
//#include "SetupMode.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Library/HobLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/PciLib.h>

//[-start-160818-IB07400772-add]//
#ifdef VBT_HOOK_SUPPORT

#ifdef _VBT_REGS_GOP_
#undef _VBT_REGS_GOP_
#endif
#ifndef _VBT_REGS_VBIOS_
#define _VBT_REGS_VBIOS_
#endif

#include <VBTAccess.h>
#include <ChipsetSetupConfig.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <SaAccess.h>
#endif
//[-end-160818-IB07400772-add]//

extern  PCI_OPTION_ROM_TABLE  mPciOptionRomTable[];
extern  UINTN                 mSizeOptionRomTable;

EFI_PCI_PLATFORM_PROTOCOL mPciPlatform = {
  PhaseNotify,
  PlatformPrepController,
  GetPlatformPolicy,
  GetPciRom
};

EFI_HANDLE mPciPlatformHandle = NULL;


//SYSTEM_CONFIGURATION          mSystemConfiguration;

BOOLEAN
BootLegacyNetwork (
  )
{
  EFI_CPU_IO2_PROTOCOL            *CpuIo;
  UINT8                           Index;
  UINT8                           Data;
  EFI_STATUS                      Status;

  Status = gBS->LocateProtocol (&gEfiCpuIo2ProtocolGuid, NULL, (VOID **) &CpuIo);
  ASSERT_EFI_ERROR (Status);

  // BUGBUG we need to organize CMOS access
  Index = CmosBootFlagAddress;
  CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, 0x72, 1, &Index);
  CpuIo->Io.Read (CpuIo, EfiCpuIoWidthUint8, 0x73, 1, &Data);

  return ((Data & B_CMOS_FORCE_NETWORK_BOOT) == B_CMOS_FORCE_NETWORK_BOOT);
}

EFI_STATUS
GetRawImage (
  IN EFI_GUID   *NameGuid,
  IN OUT VOID   **Buffer,
  IN OUT UINTN  *Size
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         HandleCount;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  UINT32                        AuthenticationStatus;

  Status = gBS->LocateHandleBuffer (ByProtocol,
                                    &gEfiFirmwareVolume2ProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer
                                    );
  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
  }

  //
  // Find desired image in all Fvs
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **) &Fv
                    );

    if ( EFI_ERROR ( Status ) ) {
      return EFI_LOAD_ERROR;
    }

    //
    // Try a raw file
    //
    *Buffer = NULL;
    *Size = 0;
    Status = Fv->ReadSection (Fv,
                              NameGuid,
                              EFI_SECTION_RAW,
                              0,
                              Buffer,
                              Size,
                              &AuthenticationStatus
                              );

    if ( !EFI_ERROR ( Status )) {
        break;
    }
  }

  if ( Index >= HandleCount ) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PhaseNotify (
  IN  EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE  Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  ) {
  UINT16    AcpiBaseAddr;

  // Broxton begin
  // ACPI base register in Broxon is located at PMC(bus 0, device 13, function 1), bar2(offset 0x20), we first program it at 0x400 
  // and expect to keep it. while PCI bus scan will allocate to a new value. Here we re-program it to 0x400 after resource allocation
  //
  if( (Phase == EfiPciHostBridgeEndResourceAllocation) && (ChipsetPhase == ChipsetExit) ){
    DEBUG((EFI_D_INFO, "Override Broxton bus 0, device 13, function 1, bar2(offset 0x20) to PcdScAcpiIoPortBaseAddress(0x400)\n"));
    AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
//[-start-160801-IB03090430-modify]//
    PciWrite32 (PCI_LIB_ADDRESS (0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, R_PMC_ACPI_BASE), AcpiBaseAddr);
//[-end-160801-IB03090430-modify]//
    return EFI_SUCCESS;
  }
  //
  // Broxton end

  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
PlatformPrepController (
  IN  EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_HANDLE                                     RootBridge,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS    PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE   Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
GetPlatformPolicy (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL        *This,
  OUT EFI_PCI_PLATFORM_POLICY               *PciPolicy
  )
{
  *PciPolicy = EFI_RESERVE_VGA_IO_ALIAS;
  return EFI_SUCCESS;
}
//[-start-160818-IB07400772-add]//
#ifdef VBT_HOOK_SUPPORT
EFI_STATUS
VBTPlatformHook (
  IN OUT  EFI_PHYSICAL_ADDRESS        *VBTAddress,
  IN OUT  UINT32                      *VBTSize
  )
{
  EFI_STATUS                          Status;
  UINTN                               VariableSize;
  CHIPSET_CONFIGURATION               *SystemConfiguration;
  UINTN                               eDpPanelNum = 0xFF;
  UINTN                               Index;

  VariableSize = 0;

  VariableSize = PcdGet32 (PcdSetupConfigSize);
  SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VariableSize);
  ASSERT (SystemConfiguration != NULL);

  Status = gRT->GetVariable (
                  L"Setup",
                  &gSystemConfigurationGuid,
                  NULL,
                  &VariableSize,
                  SystemConfiguration
                  );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Config LFP
  //
  DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpType = 0x%x\n", SystemConfiguration->VbtLfpType));
  if (SystemConfiguration->VbtLfpType == SCU_VBT_NO_DEVICE) {
    //
    // No Device, disable Type/Port/AUX setting
    //
    VBT16AndThenOr (*VBTAddress, R_VBT_LFP1_ACTIVE_CONFIG, (~B_VBT_LFP_ACTIVE_CONFIG ), (V_VBT_LFP_ACTIVE_CONFIG_NONE));
    VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_OUTPUT_PORT, (~B_VBT_LFP_OUTPUT_PORT ), (V_VBT_LFP_OUTPUT_PORT_EDP_NONE));
    VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_AUX, (~B_VBT_LFP_AUX ), (V_VBT_LFP_AUX_NONE));
    
  } else if ((SystemConfiguration->VbtLfpType == SCU_VBT_LFP_EDP) ||
             (SystemConfiguration->VbtLfpType == SCU_VBT_LFP_MIPI) ) {
  
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpType = 0x%x\n", SystemConfiguration->VbtLfpType));
    if (SystemConfiguration->VbtLfpType == SCU_VBT_LFP_EDP) {
      VBT16AndThenOr (*VBTAddress, R_VBT_LFP1_ACTIVE_CONFIG, (~B_VBT_LFP_ACTIVE_CONFIG ), (V_VBT_LFP_ACTIVE_CONFIG_eDP));
    } else {
      VBT16AndThenOr (*VBTAddress, R_VBT_LFP1_ACTIVE_CONFIG, (~B_VBT_LFP_ACTIVE_CONFIG ), (V_VBT_LFP_ACTIVE_CONFIG_MIPI));
    }

    //
    // LFP port
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpPort = 0x%x\n", SystemConfiguration->VbtLfpPort));
    switch (SystemConfiguration->VbtLfpPort) {
      case SCU_VBT_LFP_EDP_PORTA:
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_OUTPUT_PORT, (~B_VBT_LFP_OUTPUT_PORT ), (V_VBT_LFP_OUTPUT_PORT_EDP_PORTA));
        break;

      case SCU_VBT_LFP_MIPI_PORTA:
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_OUTPUT_PORT, (~B_VBT_LFP_OUTPUT_PORT ), (V_VBT_LFP_OUTPUT_PORT_MIPI_PORTA));
        break;
       
      case SCU_VBT_DEFAULT:
      default:
        break;
    }
    
    //
    // LFP AUX
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpAux = 0x%x\n", SystemConfiguration->VbtLfpAux));
    switch (SystemConfiguration->VbtLfpAux) {
      case SCU_VBT_DP_PORTA:
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_AUX, (~B_VBT_LFP_AUX ), (V_VBT_LFP_AUX_EDPA));
        break;
       
      case SCU_VBT_DEFAULT:
      default:
        break;
    }

    //
    // LFP Panel#
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpPanel = 0x%x\n", SystemConfiguration->VbtLfpPanel));
    if (SystemConfiguration->VbtLfpPanel != SCU_VBT_DEFAULT) {
      eDpPanelNum = SystemConfiguration->VbtLfpPanel - 1;
      VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_PANEL_TYPE, (~B_VBT_LFP_PANEL_TYPE ), eDpPanelNum);
    } else {
      eDpPanelNum = VBT8Read (*VBTAddress, R_VBT_LFP1_PANEL_TYPE);
    }
    
    //
    // LFP EDID
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpEdid = 0x%x\n", SystemConfiguration->VbtLfpEdid));
    if (SystemConfiguration->VbtLfpEdid != SCU_VBT_DEFAULT) {
      if (SystemConfiguration->VbtLfpEdid == SCU_VBT_ENABLE) {
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_EDID, (~B_VBT_LFP_EDID ), (V_VBT_LFP_EDID_ENABLED));
      } else {   
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP1_EDID, (~B_VBT_LFP_EDID ), (V_VBT_LFP_EDID_DISABLED));
      }
    }
    
//[-start-160818-IB07400772-add]//
    //
    // LFP HPD Inversion
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpHpdInversion = 0x%x\n", SystemConfiguration->VbtLfpHpdInversion));
    if (SystemConfiguration->VbtLfpHpdInversion != SCU_VBT_DEFAULT) {
      if (SystemConfiguration->VbtLfpHpdInversion == SCU_VBT_ENABLE) {
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP_HPD_INVERSION, (~B_VBT_LFP_HPD_INVERSION ), (V_VBT_LFP_HPD_INVERSION_ENABLED));
      } else {   
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP_HPD_INVERSION, (~B_VBT_LFP_HPD_INVERSION ), (V_VBT_LFP_HPD_INVERSION_DISABLED));
      }
    }
    //
    // LFP DDI Lane Reversal
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtLfpDdiLaneReversal = 0x%x\n", SystemConfiguration->VbtLfpDdiLaneReversal));
    if (SystemConfiguration->VbtLfpDdiLaneReversal != SCU_VBT_DEFAULT) {
      if (SystemConfiguration->VbtLfpDdiLaneReversal == SCU_VBT_ENABLE) {
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP_DDI_LANE_REVERSAL, (~B_VBT_LFP_DDI_LANE_REVERSAL ), (V_VBT_LFP_DDI_LANE_REVERSAL_ENABLED));
      } else {   
        VBT8AndThenOr (*VBTAddress, R_VBT_LFP_DDI_LANE_REVERSAL, (~B_VBT_LFP_DDI_LANE_REVERSAL ), (V_VBT_LFP_DDI_LANE_REVERSAL_DISABLED));
      }
    }
//[-end-160818-IB07400772-add]//

    //
    // Conifg Panel Resolution
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->IgdFlatPanel = 0x%x\n", SystemConfiguration->IgdFlatPanel));
    switch (SystemConfiguration->IgdFlatPanel) {
      case Resolution640x480:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 640);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 480);
        break;
        
      case Resolution800x600:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 800);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 600);
        break;
        
      case Resolution1024x768:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 1024);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 768);
        break;
        
      case Resolution1280x1024:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 1280);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 1024);
        break;
        
      case Resolution1366x768:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 1366);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 768);
        break;
        
      case Resolution1680x1050:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 1680);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 1050);
        break;
        
      case Resolution1920x1200:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 1920);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 1200);
        break;
        
      case Resolution1200x800:
        VBT16AndThenOr (*VBTAddress, GetWidthOffset (eDpPanelNum), (~B_VBT_PANEL_WIDTH ), 1200);
        VBT16AndThenOr (*VBTAddress, GetHeightOffset (eDpPanelNum), (~B_VBT_PANEL_HEIGHT ), 800);
        break;
        
      case ResolutionDefault:
      default:
        break;
    }
  } 

  //
  // Config EFP
  //
  for (Index = 0; Index < MAX_EFP_NUMBER; Index++) {

    //
    // EFP Device Type
    //
    DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpType[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpType[Index]));
    switch (SystemConfiguration->VbtEfpType[Index]) {
      
      case SCU_VBT_DP:
        VBT16AndThenOr (*VBTAddress, GetEfpDevTypeOffset(Index), (~B_VBT_EFP_DEVICE_TYPE ), (V_VBT_EFP_DEVICE_TYPE_DP)); 
        break;
        
      case SCU_VBT_HDMI_DVI:
        VBT16AndThenOr (*VBTAddress, GetEfpDevTypeOffset(Index), (~B_VBT_EFP_DEVICE_TYPE ), (V_VBT_EFP_DEVICE_TYPE_HDMI_DVI)); 
        break;
        
      case SCU_VBT_DVI_ONLY:
        VBT16AndThenOr (*VBTAddress, GetEfpDevTypeOffset(Index), (~B_VBT_EFP_DEVICE_TYPE ), (V_VBT_EFP_DEVICE_TYPE_DVI_ONLY)); 
        break;
        
      case SCU_VBT_DP_HDMI_DVI:
        VBT16AndThenOr (*VBTAddress, GetEfpDevTypeOffset(Index), (~B_VBT_EFP_DEVICE_TYPE ), (V_VBT_EFP_DEVICE_TYPE_DP_HDMI_DVI)); 
        break;
        
      case SCU_VBT_NO_DEVICE:
        //
        // No Device, disable Type/Port/DDC/AUX setting
        //
        VBT16AndThenOr (*VBTAddress, GetEfpDevTypeOffset(Index), (~B_VBT_EFP_DEVICE_TYPE ), (V_VBT_EFP_DEVICE_TYPE_NONE)); 
        VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_NONE)); 
        VBT8AndThenOr (*VBTAddress, GetEfpDdcConfigOffset(Index), (~B_VBT_EFP_DDC_CONFIG ), (V_VBT_EFP_DDC_CONFIG_NONE));
        VBT8AndThenOr (*VBTAddress, GetEfpAuxConfigOffset(Index), (~B_VBT_EFP_AUX ), (V_VBT_EFP_AUX_NONE));
        break;
        
      case SCU_VBT_DEFAULT:
      default:
        break;
    }

//[-start-160913-IB07400784-modify]//
    if ((SystemConfiguration->VbtEfpType[Index] != SCU_VBT_NO_DEVICE) &&
        (SystemConfiguration->VbtEfpType[Index] != SCU_VBT_DEFAULT)) {
//[-end-160913-IB07400784-modify]//
      //
      // EFP Device Port
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpPort[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpPort[Index]));
      switch (SystemConfiguration->VbtEfpPort[Index]) {
        
        case SCU_VBT_DP_PORTA:
          VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_DP_PORTA)); 
          break;
          
        case SCU_VBT_DP_PORTB:
          VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_DP_PORTB)); 
          break;
          
        case SCU_VBT_DP_PORTC:
          VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_DP_PORTC)); 
          break;
          
        case SCU_VBT_HDMI_PORTB:
          VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_HDMI_PORTB)); 
          break;
          
        case SCU_VBT_HDMI_PORTC:
          VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_HDMI_PORTC)); 
          break;
          
        case SCU_VBT_NO_DEVICE:
          VBT8AndThenOr (*VBTAddress, GetEfpOutputPortOffset(Index), (~B_VBT_EFP_OUTPUT_PORT ), (V_VBT_EFP_OUTPUT_PORT_NONE)); 
          break;
          
        case SCU_VBT_DEFAULT:
        default:
          break;
      }
      
      //
      // EFP DDC Port
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpDdc[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpDdc[Index]));
      switch (SystemConfiguration->VbtEfpDdc[Index]) {
        
        case SCU_VBT_HDMI_PORTB:
          VBT8AndThenOr (*VBTAddress, GetEfpDdcConfigOffset(Index), (~B_VBT_EFP_DDC_CONFIG ), (V_VBT_EFP_DDC_CONFIG_HDMI_B)); 
          break;
          
        case SCU_VBT_HDMI_PORTC:
          VBT8AndThenOr (*VBTAddress, GetEfpDdcConfigOffset(Index), (~B_VBT_EFP_DDC_CONFIG ), (V_VBT_EFP_DDC_CONFIG_HDMI_C)); 
          break;
          
        case SCU_VBT_NO_DEVICE:
          VBT8AndThenOr (*VBTAddress, GetEfpDdcConfigOffset(Index), (~B_VBT_EFP_DDC_CONFIG ), (V_VBT_EFP_DDC_CONFIG_NONE));
          break;
          
        case SCU_VBT_DEFAULT:
        default:
          break;
      }
      
      //
      // EFP AUX Port
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpAux[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpAux[Index]));
      switch (SystemConfiguration->VbtEfpAux[Index]) {
        
        case SCU_VBT_DP_PORTA:
          VBT8AndThenOr (*VBTAddress, GetEfpAuxConfigOffset(Index), (~B_VBT_EFP_AUX ), (V_VBT_EFP_AUX_DPA)); 
          break;
          
        case SCU_VBT_DP_PORTB:
          VBT8AndThenOr (*VBTAddress, GetEfpAuxConfigOffset(Index), (~B_VBT_EFP_AUX ), (V_VBT_EFP_AUX_DPB)); 
          break;
          
        case SCU_VBT_DP_PORTC:
          VBT8AndThenOr (*VBTAddress, GetEfpAuxConfigOffset(Index), (~B_VBT_EFP_AUX ), (V_VBT_EFP_AUX_DPC)); 
          break;
          
        case SCU_VBT_NO_DEVICE:
          VBT8AndThenOr (*VBTAddress, GetEfpAuxConfigOffset(Index), (~B_VBT_EFP_AUX ), (V_VBT_EFP_AUX_NONE)); 
          break;
          
        case SCU_VBT_DEFAULT:
        default:
          break;
      }
//[-start-160818-IB07400772-add]//
      //
      // EFP HDMI Level Shifter
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpHdmiLevelShifter[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpHdmiLevelShifter[Index]));
      if (SystemConfiguration->VbtEfpHdmiLevelShifter[Index] != SCU_VBT_DEFAULT) {
        VBT8AndThenOr (*VBTAddress, GetEfpHdmiLsConfigOffset(Index), (~B_VBT_EFP_HDMI_LS ), (SystemConfiguration->VbtEfpHdmiLevelShifter[Index] - 1)); 
      }
      
      //
      // EFP LSPCON
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpOnboardLspcon[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpOnboardLspcon[Index]));
      if (SystemConfiguration->VbtEfpOnboardLspcon[Index] != SCU_VBT_DEFAULT) {
        if (SystemConfiguration->VbtEfpOnboardLspcon[Index] == SCU_VBT_ENABLE) {
          VBT8AndThenOr (*VBTAddress, GetEfpLspconConfigOffset(Index), (~B_VBT_EFP_LSPCON), (V_VBT_EFP_LSPCON_ENABLED));
        } else {   
          VBT8AndThenOr (*VBTAddress, GetEfpLspconConfigOffset(Index), (~B_VBT_EFP_LSPCON), (V_VBT_EFP_LSPCON_DISABLED));
        }
      }
      //
      // EFP HPD Inversion
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpHpdInversion[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpHpdInversion[Index]));
      if (SystemConfiguration->VbtEfpHpdInversion[Index] != SCU_VBT_DEFAULT) {
        if (SystemConfiguration->VbtEfpHpdInversion[Index] == SCU_VBT_ENABLE) {
          VBT8AndThenOr (*VBTAddress, GetEfpHpdInversionConfigOffset(Index), (~B_VBT_EFP_HPD_INVERSION ), (V_VBT_EFP_HPD_INVERSION_ENABLED));
        } else {   
          VBT8AndThenOr (*VBTAddress, GetEfpHpdInversionConfigOffset(Index), (~B_VBT_EFP_HPD_INVERSION ), (V_VBT_EFP_HPD_INVERSION_DISABLED));
        }
      }
      //
      // EFP DDI Lane Reversal
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpDdiLaneReversal[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpDdiLaneReversal[Index]));
      if (SystemConfiguration->VbtEfpDdiLaneReversal[Index] != SCU_VBT_DEFAULT) {
        if (SystemConfiguration->VbtEfpDdiLaneReversal[Index] == SCU_VBT_ENABLE) {
          VBT8AndThenOr (*VBTAddress, GetEfpDdiLaneReversalConfigOffset(Index), (~B_VBT_EFP_DDI_LANE_REVERSAL ), (V_VBT_EFP_DDI_LANE_REVERSAL_ENABLED));
        } else {   
          VBT8AndThenOr (*VBTAddress, GetEfpDdiLaneReversalConfigOffset(Index), (~B_VBT_EFP_DDI_LANE_REVERSAL ), (V_VBT_EFP_DDI_LANE_REVERSAL_DISABLED));
        }
      }
      //
      // EFP USB Type C
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpUsbTypeC[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpUsbTypeC[Index]));
      if (SystemConfiguration->VbtEfpUsbTypeC[Index] != SCU_VBT_DEFAULT) {
        if (SystemConfiguration->VbtEfpUsbTypeC[Index] == SCU_VBT_ENABLE) {
          VBT8AndThenOr (*VBTAddress, GetEfpUsbTypeCConfigOffset(Index), (~B_VBT_EFP_USB_TYPEC ), (V_VBT_LFP_USB_TYPEC_ENABLED));
        } else {   
          VBT8AndThenOr (*VBTAddress, GetEfpUsbTypeCConfigOffset(Index), (~B_VBT_EFP_USB_TYPEC ), (V_VBT_LFP_USB_TYPEC_DISABLED));
        }
      }
      //
      // EFP Dockable Port
      //
      DEBUG ((EFI_D_ERROR, "SystemConfiguration->VbtEfpDockablePort[%x] = 0x%x\n", Index, SystemConfiguration->VbtEfpDockablePort[Index]));
      if (SystemConfiguration->VbtEfpDockablePort[Index] != SCU_VBT_DEFAULT) {
        if (SystemConfiguration->VbtEfpDockablePort[Index] == SCU_VBT_ENABLE) {
          VBT8AndThenOr (*VBTAddress, GetEfpDockConfigOffset(Index), (~B_VBT_EFP_DOCK ), (V_VBT_LFP_DOCK_ENABLED));
        } else {   
          VBT8AndThenOr (*VBTAddress, GetEfpDockConfigOffset(Index), (~B_VBT_EFP_DOCK ), (V_VBT_LFP_DOCK_DISABLED));
        }
      }
//[-end-160818-IB07400772-add]//
    }
  }

  FreePool (SystemConfiguration);

  return Status;
}

EFI_STATUS
VbiosVbtPlatformHook (
  IN  VOID                        **VBTAddress,
  IN  UINTN                       *VBTSize
  )
{
  EFI_PHYSICAL_ADDRESS                *VBTBaseAddressPtr;
  EFI_PHYSICAL_ADDRESS                VBTBaseAddress;
  CHAR8                               VBTSignature[4] = {'$','V','B','T'};
  UINTN                               Index;

  DEBUG ((EFI_D_ERROR, "VbiosVbtPlatformHook!!\n"));
  DEBUG ((EFI_D_ERROR, "*VbiosVBTAddress = %x!!\n", *VBTAddress));
  
  //
  //  Find VBT signature
  //
  VBTBaseAddressPtr = (EFI_PHYSICAL_ADDRESS*)*VBTAddress;
  for (Index = 0; Index < *VBTSize; Index++) {
    if(CompareMem(VBTBaseAddressPtr, VBTSignature, 4)==0) {
      VBTBaseAddress = (EFI_PHYSICAL_ADDRESS)((UINTN)VBTBaseAddressPtr);
      DEBUG ((EFI_D_ERROR, "VBTBaseAddress = 0x%x\n", VBTBaseAddress));
      DEBUG ((EFI_D_ERROR, "&VBTBaseAddress = 0x%x\n", &VBTBaseAddress));
      VBTPlatformHook (&VBTBaseAddress, (UINT32*)VBTSize);
      break;
    }
    (UINT8*)VBTBaseAddressPtr++;
  }

  return EFI_SUCCESS;
}  
#endif
//[-end-160818-IB07400772-add]//

EFI_STATUS
EFIAPI
GetPciRom (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL    *This,
  IN EFI_HANDLE                           PciHandle,
  OUT  VOID                               **RomImage,
  OUT  UINTN                              *RomSize
  )
/*++

Routine Description:
  GetPciRom from platform specific location for specific PCI device

Arguments:
  This -- Protocol instance
  PciHandle -- Identify the specific PCI devic
  RomImage -- Returns the ROM Image memory location
  RomSize -- Returns Rom Image size

Returns:

  EFI_SUCCESS
  EFI_NOT_FOUND
  EFI_OUT_OF_RESOURCES

--*/
{
//  EFI_STATUS                    Status;
//  EFI_PCI_IO_PROTOCOL           *PciIo;
//  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
//  UINTN                         Segment;
//  UINTN                         Bus;
//  UINTN                         Device;
//  UINTN                         Function;
//  UINT16                        VendorId;
//  UINT16                        DeviceId;
//  UINT16                        DeviceClass;
//  UINTN                         TableIndex;
//  BOOLEAN                       MfgMode;
////  VOID                          *HobList;
//  EFI_PLATFORM_SETUP_ID         *BootModeBuffer;
//
//  EFI_PEI_HOB_POINTERS        GuidHob;
//
////  PCI_OPTION_ROM_TABLE          *CurrentPciOptionRomTablePtr;
//
////  //
////  // Check for Mfg Mode
////  //
////  Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);
//  MfgMode = FALSE;
////  if (!EFI_ERROR (Status)) {
////    Status = GetNextGuidHob (&HobList, &gEfiPlatformBootModeGuid, &BootModeBuffer, NULL);
////    if (!EFI_ERROR(Status)) {
////      if ( !EfiCompareMem (
////              &BootModeBuffer->SetupName,
////              gEfiManufacturingSetupName,
////              EfiStrSize (gEfiManufacturingSetupName)
////              )
////         ) {
////        MfgMode = TRUE;
////      }
////    }
////  }
//
//// Check if system is in manufacturing mode.
//
//  GuidHob.Raw = GetHobList ();
//  if (GuidHob.Raw == NULL) {
//    return EFI_NOT_FOUND;
//  }
//
//  if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformBootModeGuid, GuidHob.Raw)) != NULL) {
//      BootModeBuffer = GET_GUID_HOB_DATA (GuidHob.Guid);
//
//      if (!CompareMem (&BootModeBuffer->SetupName, MANUFACTURE_SETUP_NAME,
//          StrSize (MANUFACTURE_SETUP_NAME)))
//      {
//        // System is in manufacturing mode.
//        MfgMode = TRUE;
//      }
//   }
//
//  Status = gBS->HandleProtocol (PciHandle,
//                                &gEfiPciIoProtocolGuid,
//                                (VOID **)&PciIo);
//  if (EFI_ERROR (Status)) {
//    return EFI_NOT_FOUND;
//  }
//
//  Status = gBS->LocateProtocol (&gEfiPciRootBridgeIoProtocolGuid, NULL, (VOID **) &PciRootBridgeIo);
//
//  if (EFI_ERROR (Status)) {
//    return EFI_NOT_FOUND;
//  }
//
//  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 0x0A, 1, &DeviceClass);
//
//  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
//
//  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 0, 1, &VendorId);
//
//  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 2, 1, &DeviceId);
///*
//  if ((VendorId == V_SA_IGD_VID) && ((DeviceId == IGD_DID_VLV_A0) ||
//      (DeviceId == IGD_DID_II) || (DeviceId == IGD_DID_0BE4))) {
//     if (mSystemConfiguration.GOPEnable == 1) {
//       return EFI_NOT_FOUND;
//     }
//  }
//*/  
//    //
//    // Do not run RAID or AHCI Option ROM if IDE
//    //
//    if ( (DeviceClass == ((PCI_CLASS_MASS_STORAGE << 8 ) | PCI_CLASS_MASS_STORAGE_IDE))
//      ) {
//      return EFI_NOT_FOUND;
//    }
//
//    //
//    // Run PXE ROM only if Boot network is enabled and not in MFG mode
//    //
//    if (DeviceClass == ((PCI_CLASS_NETWORK << 8 ) | PCI_CLASS_NETWORK_ETHERNET)) {
//      if (((mSystemConfiguration.BootNetwork == 0) && (MfgMode == FALSE )) || (mSystemConfiguration.FastBoot == 1)) {
//        return EFI_NOT_FOUND;
//      }
//    }
//
//    //
//    // Loop through table of Onboard option rom descriptions
//    //
//    for (TableIndex = 0; mPciOptionRomTable[TableIndex].VendorId != 0xffff; TableIndex++) {
//
//      //
//      // See if the PCI device specified by PciHandle matches at device in mPciOptionRomTable
//      //
//      if (VendorId != mPciOptionRomTable[TableIndex].VendorId ||
//          DeviceId != mPciOptionRomTable[TableIndex].DeviceId ||
//          ((DeviceClass == ((PCI_CLASS_NETWORK << 8 ) | PCI_CLASS_NETWORK_ETHERNET)) &&
//           (mPciOptionRomTable[TableIndex].Flag != mSystemConfiguration.BootNetwork))
//        ) {
//        continue;
//      }
//
//      Status = GetRawImage( &mPciOptionRomTable[TableIndex].FileName,
//                            RomImage,
//                            RomSize
//                        );
//
//      if ((VendorId == V_SA_IGD_VID) && (DeviceId == IGD_DID_BXT_A0)){
//        *(UINT16 *)(((UINTN) *RomImage) + OPROM_DID_OFFSET) = IGD_DID_BXT_A0;
//      }
//
//      if ((VendorId == V_SA_IGD_VID) && (DeviceId == IGD_DID_II)){
//        *(UINT16 *)(((UINTN) *RomImage) + OPROM_DID_OFFSET) = IGD_DID_II;
//      }
//
//      if ((VendorId == V_SA_IGD_VID) && (DeviceId == IGD_DID_0BE4)){
//        *(UINT16 *)(((UINTN) *RomImage) + OPROM_DID_OFFSET) = IGD_DID_0BE4;
//      }
//
//      if (EFI_ERROR (Status)) {
//        continue;
//      }
//      return EFI_SUCCESS;
//    }
//
//  return EFI_NOT_FOUND;
//[-start-160818-IB07400772-add]//
#ifdef VBT_HOOK_SUPPORT
  {
    EFI_STATUS                    Status;
    EFI_PCI_IO_PROTOCOL           *PciIo;
    UINT16                        VendorId;
    UINT16                        DeviceId;
    
    Status = gBS->HandleProtocol (PciHandle, &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
    if (EFI_ERROR (Status)) {
      return EFI_NOT_FOUND;
    }
    
    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 0, 1, &VendorId);
    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 2, 1, &DeviceId);

    if (((VendorId == V_SA_IGD_VID) && (DeviceId == V_SA_IGD_DID)) || 
        ((VendorId == V_SA_IGD_VID) && (DeviceId == V_SA_IGD_DID_BXTP))|| 
        ((VendorId == V_SA_IGD_VID) && (DeviceId == V_SA_IGD_DID_BXTP_1))) {
#ifdef APOLLOLAKE_CRB
      VbiosVbtPlatformHook (RomImage, RomSize);
#else
#ifdef USE_CRB_HW_CONFIG
      VbiosVbtPlatformHook (RomImage, RomSize);
#endif 
#endif
    }
  }
#endif
//[-end-160818-IB07400772-add]//
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PciPlatformDriverEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:
  EFI_STATUS

--*/
{
  EFI_STATUS  Status;
//  UINTN       VarSize;

//  VarSize = sizeof(SYSTEM_CONFIGURATION);
//  Status = gRT->GetVariable(L"Setup",
//                            &gEfiNormalSetupGuid,
//                            NULL,
//                            &VarSize,
//                            &mSystemConfiguration);
//  if (EFI_ERROR (Status)) {
//    CopyMem ((VOID *)&mSystemConfiguration, (VOID *)(UINTN)(PcdGet32(PcdFlashNvStorageVariableBase) + 0xeb0), VarSize);
//    Status = gRT->SetVariable (
//                    L"Setup",
//                    &gEfiNormalSetupGuid,
//                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
//                    VarSize,
//                    &mSystemConfiguration
//                    );  
//  }
//  ASSERT_EFI_ERROR(Status);
  //
  // Install on a new handle
  //
  Status = gBS->InstallProtocolInterface (
                  &mPciPlatformHandle,
                  &gEfiPciPlatformProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPciPlatform
                  );

  return Status;
}


