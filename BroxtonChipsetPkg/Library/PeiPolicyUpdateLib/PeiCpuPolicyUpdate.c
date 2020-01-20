/** @file
  This file is SampleCode of the library for Intel CPU PEI Policy Update initialization.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2009 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

//[-start-160509-IB03090427-modify]//
//[-start-151228-IB03090424-modify]//
#include <PeiCpuPolicyUpdate.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/CpuPolicyLib.h>
#include <Library/SteppingLib.h>
#include <Library/HobLib.h>
#include <Guid/PlatformInfo.h>
#include <Platform.h>

/**
  This function performs CPU PEI Policy initialization.

  @param[in] SiCpuPolicyPpi        The Cpu Policy PPI instance
  @param[in] SystemConfiguration   The pointer to get System Setup

  @retval EFI_SUCCESS              The PPI is installed and initialized.
  @retval EFI ERRORS               The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES     Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
UpdatePeiCpuPolicy (
  IN OUT  SI_CPU_POLICY_PPI      *SiCpuPolicyPpi,
  IN CHIPSET_CONFIGURATION        *SystemConfiguration
  )
{
  EFI_STATUS                  Status;
  CPU_CONFIG                  *CpuConfig;
  POWER_MGMT_CONFIG           *PowerMgmtConfig;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_PLATFORM_INFO_HOB       *PlatformInfo;
//[-start-160826-IB03090433-add]//
  UINTN                       McD0Base;
  UINT8                       RevId;
//[-end-160826-IB03090433-add]//

//[-start-160826-IB03090433-add]//
  McD0Base = MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN);
  RevId    = MmioRead8 (McD0Base + PCI_REVISION_ID_OFFSET);
//[-end-160826-IB03090433-add]//

  Status = GetConfigBlock((CONFIG_BLOCK_TABLE_HEADER*)SiCpuPolicyPpi,&gCpuConfigGuid,(VOID *)&CpuConfig);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, " Get config block for CpuConfig \n"));

  Status = GetConfigBlock((CONFIG_BLOCK_TABLE_HEADER*)SiCpuPolicyPpi,&gPowerMgmtConfigGuid,(VOID *)&PowerMgmtConfig);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, " Get config block for PowerMgmtConfig \n"));

  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  CpuConfig->ActiveProcessorCores        = SystemConfiguration->ActiveProcessorCores;
  CpuConfig->DisableCore1                = SystemConfiguration->Core1;
  CpuConfig->DisableCore2                = SystemConfiguration->Core2;
  CpuConfig->DisableCore3                = SystemConfiguration->Core3;
  CpuConfig->VmxEnable                   = SystemConfiguration->ProcessorVmxEnable;
  CpuConfig->ProcTraceMemSize            = SystemConfiguration->ProcTraceMemSize;
  CpuConfig->ProcTraceEnable             = SystemConfiguration->ProcTraceEnable;
//[-start-160317-IB03090425-add]//
  CpuConfig->EnableDts                   = SystemConfiguration->EnableDigitalThermalSensor;
//[-end-160317-IB03090425-add]//
//[-start-170105-IB07400830-add]//
  CpuConfig->AesEnable                   = SystemConfiguration->EnableAESNI;
//[-end-170105-IB07400830-add]//
  PowerMgmtConfig->Eist                  = SystemConfiguration->EnableGv;
  PowerMgmtConfig->BootPState            = SystemConfiguration->BootPState;
  PowerMgmtConfig->Cx                    = SystemConfiguration->EnableCx;
  PowerMgmtConfig->C1e                   = SystemConfiguration->EnableCxe;
  PowerMgmtConfig->BiProcHot             = SystemConfiguration->EnableProcHot;
  PowerMgmtConfig->PkgCStateLimit        = (MAX_PKG_C_STATE)SystemConfiguration->MaxPkgCState;
//[-start-160317-IB03090425-add]//
  PowerMgmtConfig->AutoThermalReporting  = SystemConfiguration->AutoThermalReporting;
//[-end-160317-IB03090425-add]//
//[-start-160929-IB07400790-add]//
  PowerMgmtConfig->TStates               = SystemConfiguration->CpuTstate;
//[-end-160929-IB07400790-add]//

//[-start-160719-IB03090429-modify]//
  if (SystemConfiguration->MonitorMwaitEnable == 2)
  {
    CpuConfig->MonitorMwaitEnable          = SOC_DEVICE_ENABLE;
  } else {
    CpuConfig->MonitorMwaitEnable          = SystemConfiguration->MonitorMwaitEnable;
  }

  //
  // Disable Monitor Mwait and Enable IO MWait redirection for only Linux based OS's.
  //
//[-start-160826-IB03090433-modify]//
  if ((BxtStepping() >= BxtPB0 ) && (RevId != V_SA_MC_RID_9)) {
//[-end-160826-IB03090433-modify]//
    if (((SystemConfiguration->MonitorMwaitEnable == SOC_DEVICE_DISABLE) && (SystemConfiguration->OsSelection == 0)) || (SystemConfiguration->OsSelection == 3)) {
        CpuConfig->MonitorMwaitEnable                    = SOC_DEVICE_DISABLE;
      PowerMgmtConfig->PmgCstCfgCtrIoMwaitRedirection  = SOC_DEVICE_ENABLE;
    }
  }
//[-end-160719-IB03090429-modify]//
  
  if ((PlatformInfo->BoardId == BOARD_ID_APL_RVP_1A)||(PlatformInfo->BoardId == BOARD_ID_APL_RVP_1C_LITE)){
    PowerMgmtConfig->VrConfig = SOC_DEVICE_ENABLE;
  }

  if (SystemConfiguration->EnableCx) {
    //
    // Clear C1 & C3 Auto demotion policy
    //
    PowerMgmtConfig->C1AutoDemotion = SOC_DEVICE_DISABLE;
    PowerMgmtConfig->C3AutoDemotion = SOC_DEVICE_DISABLE;

    switch (SystemConfiguration->CStateAutoDemotion) {
      case 0:
        //
        // Disable C1 and C3 Auto-demotion
        //
        break;

      case 1:
        //
        // Enable C3/C6/C7 Auto-demotion to C1
        //
        PowerMgmtConfig->C1AutoDemotion = SOC_DEVICE_ENABLE;
        break;

      case 2:
        //
        // Enable C6/C7 Auto-demotion to C3
        //
        PowerMgmtConfig->C3AutoDemotion = SOC_DEVICE_ENABLE;
        break;

      default:
      case 3:
        //
        // Enable C6/C7 Auto-demotion to C1 and C3
        //
        PowerMgmtConfig->C1AutoDemotion = SOC_DEVICE_ENABLE;
        PowerMgmtConfig->C3AutoDemotion = SOC_DEVICE_ENABLE;
        break;
    }
    //
    // Configure Un-demotion.
    //
    PowerMgmtConfig->C1UnDemotion = SOC_DEVICE_DISABLE;
    PowerMgmtConfig->C3UnDemotion = SOC_DEVICE_DISABLE;

    switch (SystemConfiguration->CStateUnDemotion) {
      case 0:
        //
        // Disable C1 and C3 Un-demotion
        //
        break;

      case 1:
        //
        // Enable C1 Un-demotion
        //
        PowerMgmtConfig->C1UnDemotion = SOC_DEVICE_ENABLE;
        break;

      case 2:
        //
        // Enable C3 Un-demotion
        //
        PowerMgmtConfig->C3UnDemotion = SOC_DEVICE_ENABLE;
        break;

      case 3:
        //
        // Enable C1 and C3 Un-demotion
        //
        PowerMgmtConfig->C1UnDemotion = SOC_DEVICE_ENABLE;
        PowerMgmtConfig->C3UnDemotion = SOC_DEVICE_ENABLE;
        break;

      default:
        break;
    }
    switch (SystemConfiguration->MaxCoreCState) {
      case 0:
        PowerMgmtConfig->UnlimitedCstate = SOC_DEVICE_ENABLE;
        break;

      case 1:
        PowerMgmtConfig->EnableC1 = SOC_DEVICE_ENABLE;
        break;

      case 2:
        PowerMgmtConfig->EnableC3 = SOC_DEVICE_ENABLE;
        break;

      case 3:
        PowerMgmtConfig->EnableC6 = SOC_DEVICE_ENABLE;
        break;

      case 4:
        PowerMgmtConfig->EnableC7 = SOC_DEVICE_ENABLE;
        break;

      case 5:
        PowerMgmtConfig->EnableC8 = SOC_DEVICE_ENABLE;
        break;

      case 6:
        PowerMgmtConfig->EnableC9 = SOC_DEVICE_ENABLE;
        break;

      case 7:
        PowerMgmtConfig->EnableC10 = SOC_DEVICE_ENABLE;
        break;

      case 8:
        PowerMgmtConfig->EnableCCx = SOC_DEVICE_ENABLE;
        break;

      default:
        break;
    }

    //
    // Pkg C-state Demotion/Un Demotion
    //
    PowerMgmtConfig->PkgCStateDemotion   = SystemConfiguration->PkgCStateDemotion;
    PowerMgmtConfig->PkgCStateUnDemotion = SystemConfiguration->PkgCStateUnDemotion;
  }

//[-start-160817-IB03090432-modify]//
  PowerMgmtConfig->TurboMode  = SystemConfiguration->TurboModeEnable;
//[-end-160817-IB03090432-modify]//
//[-start-160603-IB07400740-modify]//
//#if BXTI_PF_ENABLE
  PowerMgmtConfig->PowerLimit1Enable       = SystemConfiguration->PowerLimit1Enable;
  PowerMgmtConfig->PowerLimit1ClampEnable  = SystemConfiguration->PowerLimit1Clamp;
  PowerMgmtConfig->CustomPowerLimit1Time   = SystemConfiguration->PowerLimit1Time;
  PowerMgmtConfig->CustomPowerLimit1       = SystemConfiguration->PowerLimit1;
//#endif
//[-end-160603-IB07400740-modify]//

  return EFI_SUCCESS;
}
//[-end-151228-IB03090424-modify]//
//[-end-160509-IB03090427-modify]//
