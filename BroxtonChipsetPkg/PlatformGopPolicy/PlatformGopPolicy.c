/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/** @file
**/

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/PlatformGopPolicy.h>
//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
//#include <SetupMode.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/HobLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-151220-IB1127138-add]//
#include <Library/DxeOemSvcChipsetLib.h>
#include <Library/DxeInsydeChipsetLib.h>
//[-end-151220-IB1127138-add]//
//[-start-160216-IB03090424-add]//
#if (TABLET_PF_ENABLE == 0)
#include <Library/EcMiscLib.h>
#include <Guid/PlatformInfo.h>
#endif
//[-end-160216-IB03090424-add]//
//[-start-160725-IB07400761-add]//
#ifdef VBT_HOOK_SUPPORT

#ifdef _VBT_REGS_VBIOS_
#undef _VBT_REGS_VBIOS_
#endif

#ifndef _VBT_REGS_GOP_
#define _VBT_REGS_GOP_
#endif

#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include "VBTAccess.h"
#endif
//[-end-160725-IB07400761-add]//

EFI_BOOT_SERVICES   *gBS;


PLATFORM_GOP_POLICY_PROTOCOL  mPlatformGOPPolicy;
#if (TABLET_PF_ENABLE == 0)
extern EFI_GUID gPeiDefaultVbtGuid;
//[-start-160725-IB07400761-remove]//
//extern EFI_GUID gVbtMipiAuoGuid;
//extern EFI_GUID gVbtMipiSharpGuid;
//[-end-160725-IB07400761-remove]//
#endif


//[-start-160725-IB07400761-add]//
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
#endif 
//[-end-160725-IB07400761-add]//

//
// Function implementations
//

/*++

Routine Description:

  The function will excute with as the platform policy, and gives 
  the Platform Lid Status. IBV/OEM can customize this code for their specific
  policy action.
  
Arguments:

  CurrentLidStatus - Gives the current LID Status
  
Returns:

  EFI_SUCCESS.
  
--*/
EFI_STATUS
EFIAPI
GetPlatformLidStatus (
  OUT LID_STATUS *CurrentLidStatus
  )
{
//[-start-160216-IB03090424-modify]//
#if (TABLET_PF_ENABLE == 1)
  if (CurrentLidStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *CurrentLidStatus = LidOpen;
  return EFI_SUCCESS;
#else
  EFI_STATUS              Status = EFI_SUCCESS;
//[-start-161206-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
//[-end-170111-IB07400832-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  UINT8                   PortDataOut;
  UINT8                   DataBuffer[1];
#endif
#endif
//[-end-161206-IB07400822-modify]//
  EFI_PLATFORM_INFO_HOB   *PlatformInfoHobPtr = NULL;
  VOID                    *HobList;

  if (CurrentLidStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (PLATFORM_ID != VALUE_REAL_PLATFORM) {
    //
    // Keep lid open for pre-silicon
    //
    *CurrentLidStatus = LidOpen;
    return EFI_SUCCESS;
  }

  HobList = GetHobList ();
  if (HobList == NULL) {
    Status = EFI_NOT_FOUND;
    ASSERT_EFI_ERROR(Status);
  }

  if ((HobList = GetNextGuidHob (&gEfiPlatformInfoGuid, HobList)) != NULL) {
    PlatformInfoHobPtr = GET_GUID_HOB_DATA (HobList);
  }

  if (PlatformInfoHobPtr == NULL) {
    Status = EFI_NOT_FOUND;
    ASSERT_EFI_ERROR(Status);
  }

  if ((PlatformInfoHobPtr->BoardId == BOARD_ID_APL_RVP_1A) || (PlatformInfoHobPtr->BoardId == BOARD_ID_APL_RVP_2A)) {
//[-start-161205-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
//[-end-170111-IB07400832-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // CRB EC code to get LID status
    //
    DataBuffer[0] = EC_D_LID_STATE;
    Status = ReadEcRam(DataBuffer);
    if (Status == EFI_SUCCESS) {
      PortDataOut = DataBuffer[0];
      if ((PortDataOut & EC_B_LID_STATUS_OPEN) == EC_B_LID_STATUS_OPEN) {
        *CurrentLidStatus = LidOpen;
      } else {
        *CurrentLidStatus = LidClosed;
      }
    }
#else
    //
    // OEM EC code to get LID status
    //
    *CurrentLidStatus = LidOpen; // default always open
#endif    
#else // EC/KSC support is disabled
    *CurrentLidStatus = LidOpen; // default always open
#endif    
//[-end-161205-IB07400822-modify]//
  } else {
    //
    // Keep lid open for IOTG platforms
    //
    *CurrentLidStatus = LidOpen;
  }
  return Status;
#endif
//[-end-160216-IB03090424-modify]//
}

/*++

Routine Description:

  The function will excute and gives the Video Bios Table Size and Address.
  
Arguments:

  VbtAddress - Gives the Physical Address of Video BIOS Table

  VbtSize - Gives the Size of Video BIOS Table

Returns:

  EFI_STATUS.
  
--*/

EFI_STATUS
EFIAPI
GetVbtData (
   OUT EFI_PHYSICAL_ADDRESS *VbtAddress,
   OUT UINT32 *VbtSize
)
{
#if (TABLET_PF_ENABLE == 1)
  EFI_PEI_HOB_POINTERS     GuidHob;
  VBT_INFO                 *VbtInfo=NULL;
#else
//[-start-160803-IB07400768-remove]//
//  CHIPSET_CONFIGURATION          SystemConfiguration;
//  UINTN                         VarSize;
//[-end-160803-IB07400768-remove]//
#endif
  EFI_STATUS                    Status;
  EFI_GUID  BmpImageGuid = { 0xE08CA6D5, 0x8D02, 0x43ae, 0xAB, 0xB1, 0x95, 0x2C, 0xC7, 0x87, 0xC9, 0x33 };
  UINTN                         FvProtocolCount;
  EFI_HANDLE                    *FvHandles;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
  UINTN                         Index;
  UINT32                        AuthenticationStatus;
  UINT8                         *Buffer;
  UINTN                         VbtBufferSize;

  if (VbtAddress == NULL || VbtSize == NULL){
    return EFI_INVALID_PARAMETER;
  }

#if (TABLET_PF_ENABLE == 1)
  Status = EFI_NOT_FOUND;  
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
   if ((GuidHob.Raw = GetNextGuidHob (&gVbtInfoGuid, GuidHob.Raw)) != NULL) {
     VbtInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
     *VbtAddress = VbtInfo->VbtAddress;
     *VbtSize = VbtInfo->VbtSize;
     Status = EFI_SUCCESS;
     return Status;
   }
  }
#else

//[-start-160803-IB07400768-remove]//
//  VarSize = sizeof(CHIPSET_CONFIGURATION);
////[-start-160803-IB07220122-modify]//
////   Status = GetSystemConfigData(&SystemConfiguration, &VarSize);
//  Status = gRT->GetVariable (
//                  SETUP_VARIABLE_NAME,
//                  &gSystemConfigurationGuid,
//                  NULL,
//                  &VarSize,
//                  &SystemConfiguration
//                  );
////[-end-160803-IB07220122-modify]//
//  ASSERT_EFI_ERROR(Status);
//  
//  if (EFI_ERROR (Status)) {
//    Status = GetChipsetSetupVariableDxe (&SystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
//    if (EFI_ERROR (Status)) {
//      return Status;  
//    }
//  }
//[-end-160803-IB07400768-remove]//
//  if (SystemConfiguration.VbtSelect == 0) {
    BmpImageGuid = gPeiDefaultVbtGuid;
//  } else if (SystemConfiguration.VbtSelect == 1) {
//    BmpImageGuid = gVbtMipiAuoGuid;
//  } else if (SystemConfiguration.VbtSelect == 2) {
//    BmpImageGuid = gVbtMipiSharpGuid;
//  }
#endif

  Fv = NULL;
  Buffer = 0;
  FvHandles = NULL;
  Status = gBS->LocateHandleBuffer(
	  ByProtocol,
	  &gEfiFirmwareVolume2ProtocolGuid,
	  NULL,
	  &FvProtocolCount,
	  &FvHandles
	  );
  if (!EFI_ERROR(Status)) {
	  for (Index = 0; Index < FvProtocolCount; Index++) {
		  Status = gBS->HandleProtocol(
			  FvHandles[Index],
			  &gEfiFirmwareVolume2ProtocolGuid,
			  (VOID **)&Fv
			  );
		  VbtBufferSize = 0;
		  Status = Fv->ReadSection(
			  Fv,
			  &BmpImageGuid,
			  EFI_SECTION_RAW,
			  0,
			  &Buffer,
			  &VbtBufferSize,
			  &AuthenticationStatus
			  );
		  if (!EFI_ERROR(Status)) {
			  *VbtAddress = (EFI_PHYSICAL_ADDRESS)Buffer;
              DEBUG ((EFI_D_INFO, "[GopPolicy] VbtAddress %x\n",VbtAddress));
			  *VbtSize = (UINT32)VbtBufferSize;
			  Status = EFI_SUCCESS;
			  break;
		  }
	  }
  }
  else {
	  Status = EFI_NOT_FOUND;
  }

//[-start-160725-IB07400761-add]//
  if (FvHandles != NULL) {
    gBS->FreePool (FvHandles);
    FvHandles = NULL;
  }
  
#ifdef VBT_HOOK_SUPPORT
#ifdef APOLLOLAKE_CRB
  VBTPlatformHook (VbtAddress, VbtSize);
#else
#ifdef USE_CRB_HW_CONFIG
  VBTPlatformHook (VbtAddress, VbtSize);
#endif 
#endif
#endif 
//[-end-160725-IB07400761-add]//

  return Status;

}


/*++
 
Routine Description:
 
  Entry point for the Platform GOP Policy Driver.
  
Arguments:
 
  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.
 
Returns:
 
  EFI_SUCCESS           Initialization complete.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
 
--*/
 
EFI_STATUS
EFIAPI
PlatformGOPPolicyEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
//  SYSTEM_CONFIGURATION  SystemConfiguration;
//  UINTN                 VarSize;

  gBS = SystemTable->BootServices;

  gBS->SetMem (&mPlatformGOPPolicy, sizeof (PLATFORM_GOP_POLICY_PROTOCOL), 0);

  mPlatformGOPPolicy.Revision                = PLATFORM_GOP_POLICY_PROTOCOL_REVISION_01;
  mPlatformGOPPolicy.GetPlatformLidStatus    = GetPlatformLidStatus;
  mPlatformGOPPolicy.GetVbtData              = GetVbtData;

//  //
//  // Install protocol to allow access to this Policy.
//  //
//  VarSize = sizeof(SYSTEM_CONFIGURATION);
//  Status = gRT->GetVariable(
//                L"Setup",
//                &gEfiNormalSetupGuid,
//                NULL,
//                &VarSize,
//                &SystemConfiguration
//                );
//  ASSERT_EFI_ERROR(Status);

#if (WA_FOR_WIN7_ENABLE == 1)
  DEBUG((DEBUG_ERROR,"Win7: reture success.\n"));
  return EFI_SUCCESS;
#else  
  DEBUG((DEBUG_ERROR,"Not Win7: install GOP Policy.\n"));
//  if (SystemConfiguration.GOPEnable == 1 || PLATFORM_ID != VALUE_REAL_PLATFORM) { //pre-silicon
//[-start-151220-IB1127138-add]//
    //
    // OemServices
    //
    Status = OemSvcUpdateDxePlatformGopPolicy (&mPlatformGOPPolicy);
//[-end-151220-IB1127138-add]//
    Status = gBS->InstallMultipleProtocolInterfaces (
                &ImageHandle,
                &gPlatformGOPPolicyGuid,
                &mPlatformGOPPolicy,
                NULL
                );
//  }
  return Status;
#endif  
}
