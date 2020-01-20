/** @file
  This module is for Switchable Graphics Feature PEI pre-initialize.

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

#include <SwitchableGraphicsPei.h>

//
// Module globals
//
STATIC H2O_SWITCHABLE_GRAPHICS_PPI            mH2OSwitchableGraphicsPpi = {
  SwitchableGraphicsGpioRead,
  SwitchableGraphicsGpioWrite,
  SwitchableGraphicsStall
};

STATIC EFI_PEI_PPI_DESCRIPTOR                 mPpiListSwitchableGraphics = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gH2OSwitchableGraphicsPpiGuid,
  &mH2OSwitchableGraphicsPpi
};

/**

  Switchable Graphics feature PEIM entry point.
  This module will modify SG related SA MCH policy, set SG mode and power enable the discrete GPU.

  @param[in] FileHandle     Handle of the file being invoked.
  @param[in] PeiServices    It's a general purpose services available to every PEIM.

  @retval EFI_SUCCESS       Switchable Graphics feature PEI initialized successfully.
  @retval EFI_UNSUPPORTED   Switchable Graphics feature doesn't be supported.

**/
EFI_STATUS
SwitchableGraphicsPeiInitEntry (
  IN EFI_PEI_FILE_HANDLE                      FileHandle,
  IN CONST EFI_PEI_SERVICES                   **PeiServices
  )
{
  EFI_STATUS                                  Status;
  SI_SA_POLICY_PPI                              *ChvPolicyPpi;
  SG_INFORMATION_DATA_HOB                     *SgInfoDataHob;
  SYSTEM_CONFIGURATION                        SystemConfiguration;

  //
  // Install Switchable Graphics PPI for module dependency,
  // Switchable Graphics PEI module must be dispatched before SA initialize PEI module,
  // so must add dependency in SA initialize PEI module.
  //
  PeiServicesInstallPpi (&mPpiListSwitchableGraphics);

  if (FeaturePcdGet (PcdSwitchableGraphicsSupported)) {
    //
    // Get Setup Variable and SA platform policy PPI.
    //
    Status = SwitchableGraphicsPeiInitialize (&ChvPolicyPpi, &SystemConfiguration);
    if (EFI_ERROR (Status)) {
     	return Status;
    }

    //
    // Base on SG related System Configuration variables to change SA MCH policy.
    //
    SetSgRelatedSaPlatformPolicy (ChvPolicyPpi, SystemConfiguration);

    //
    // Create Switchable Graphics information data HOB 
    //
    CreateSgInfoDataHob (&SgInfoDataHob, SystemConfiguration);

    //
    // Base on SG mode to power enable the discrete GPU or not.
    //
    MxmDgpuPowerEnableSequence (mH2OSwitchableGraphicsPpi, SgInfoDataHob);

    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

/**

  Switchable Graphics feature PEIM initialize function.
  Prepare variables and PPI services this module needed.

  @param[in, out] SaPlatformPolicyPpi   A double pointer of SA platform policy PPI to access the GTC
                                        (Graphics Translation Configuration) related information.
  @param[in, out] SystemConfiguration   It's a pointer to setup variables (system configuration).

  @retval EFI_SUCCESS    Get system configuration and related PPI successfully.
  @retval !EFI_SUCCESS   Get variable or PPI service failed.

**/
STATIC
EFI_STATUS
SwitchableGraphicsPeiInitialize (
  IN OUT SI_SA_POLICY_PPI                       **ChvPolicyPpi,
  IN OUT SYSTEM_CONFIGURATION                 *SystemConfiguration
  )
{
  EFI_PEI_READ_ONLY_VARIABLE2_PPI             *VariableServices;
  EFI_STATUS                                  Status;
  UINTN                                       VariableSize;

  //
  // Locate PEI Read Only Variable PPI
  //
  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&VariableServices
             );
  if (EFI_ERROR (Status)) {
   	return Status;
  }

  //
  // Get Setup Variable
  //
  VariableSize = PcdGet32 (PcdSetupConfigSize);
  Status = VariableServices->GetVariable (
                                VariableServices,
                                L"Setup",
                                &gSystemConfigurationGuid,
                                NULL,
                                &VariableSize,
                                SystemConfiguration
                                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Locate SA platform Policy PPI
  //
  Status = PeiServicesLocatePpi (
             &gChvPolicyPpiGuid,
             0,
             NULL,
             (VOID **)ChvPolicyPpi
             );
  if (EFI_ERROR (Status)) {
   	return Status;
  }

  return EFI_SUCCESS;
}

/**

  Graphics Translation Configuration of Primary Display setting didn't have SG mode,
  it just had IGD, PEG or PCI mode in PEI graphics Initialize code, so feature code needed base on
  System Configuration settings to change the Graphics Translation Configuration settings,
  SG mode output display is go through IGD, so the primary display should set to IGD.

  @param[in, out] SaPlatformPolicyPpi   A pointer of SA platform policy PPI to access the GTC
                                        (Graphics Translation Configuration) related information.
  @param[in]      SystemConfiguration   It's the setup variables (system configuration).

  @retval None.

**/
STATIC
VOID
SetSgRelatedSaPlatformPolicy (
  IN OUT SI_SA_POLICY_PPI                        *ChvPolicyPpi,
  IN SYSTEM_CONFIGURATION                     SystemConfiguration
  )
{
  if (SystemConfiguration.EnSGFunction == EnableSgFunction) {
    //
    // It can not add SG mode in reference code,
    // so need use setup configuration setting to make up the same action whit SG.
    //
    if (SystemConfiguration.PrimaryVideoAdaptor == DisplayModeSg) {
      if (SystemConfiguration.Igd == IgdDisable) {
        //
        // Set to PEG mode when SG mode to disable IGD
        //
        ChvPolicyPpi->GtConfig.PrimaryDisplay = DisplayModeDgpu;
      } else {
        //
        // Set to IGD mode when SG mode to enable IGD
        //
        ChvPolicyPpi->GtConfig.PrimaryDisplay = DisplayModeIgpu;
      }
    }
    //
    // If project's discrete GPU after power enable sequence still can't be powered on successfully,
    // "always enable PEG of PCIE configuration" policy need be set to always enable in SCU.
    //
  }
}

/**

  Base on PCD variables to set SG related information data, then through HOB pass to DXE driver.

  @param[in, out] SgInfoDataHob   A double pointer of SG information data HOB.
  @param[in] SystemConfiguration   It's the setup variables (system configuration).

  @retval EFI_SUCCESS    Created SG information data HOB and initialized successfully.
  @retval !EFI_SUCCESS   Created SG information data HOB failed.

**/
STATIC
EFI_STATUS
CreateSgInfoDataHob (
  IN OUT SG_INFORMATION_DATA_HOB              **SgInfoDataHob,
  IN SYSTEM_CONFIGURATION                     SystemConfiguration
  )
{
  EFI_STATUS                                  Status;
//[-start-160421-IB08450342-remove]//
  //EFI_PEI_READ_ONLY_VARIABLE2_PPI             *VariableServices;
  //UINTN                                       VariableSize;
  //UINT8                                       BootType;
//[-end-160421-IB08450342-remove]//

  Status = PeiServicesCreateHob (
             EFI_HOB_TYPE_GUID_EXTENSION,
             sizeof (SG_INFORMATION_DATA_HOB),
             (VOID **)SgInfoDataHob
             );
  if (EFI_ERROR (Status)) {
   	return Status;
  }

  (**SgInfoDataHob).EfiHobGuidType.Name = gH2OSgInformationDataHobGuid;
  (**SgInfoDataHob).SgGpioSupport   = PcdGetBool (PcdSgGpioSupport);

  (**SgInfoDataHob).ActiveDgpuPwrEnableDelay = PcdGet16 (PcdSgActiveDgpuPwrEnableDelay);
  (**SgInfoDataHob).ActiveDgpuHoldRstDelay   = PcdGet16 (PcdSgActiveDgpuHoldRstDelay);
  (**SgInfoDataHob).InactiveDgpuHoldRstDelay = PcdGet16 (PcdSgInactiveDgpuHoldRstDelay);
  (**SgInfoDataHob).DgpuBridgeBus           = PcdGet8 (PcdSgPcieBridgeBus);
  (**SgInfoDataHob).DgpuBridgeDevice        = PcdGet8 (PcdSgPcieBridgeDevice);
  (**SgInfoDataHob).DgpuBridgeFunction      = PcdGet8 (PcdSgPcieBridgeFunction);
  (**SgInfoDataHob).AmdSecondaryGrcphicsCommandRegister    = PcdGet8 (PcdAmdSecondaryGrcphicsCommandRegister);
  (**SgInfoDataHob).NvidiaSecondaryGrcphicsCommandRegister = PcdGet8 (PcdNvidiaSecondaryGrcphicsCommandRegister);
  (**SgInfoDataHob).OptimusDgpuHotPlugSupport      = PcdGetBool (PcdSgNvidiaOptimusDgpuHotPlugSupport);
  (**SgInfoDataHob).OptimusDgpuPowerControlSupport = PcdGetBool (PcdSgNvidiaOptimusDgpuPowerControlSupport);
  (**SgInfoDataHob).GpsFeatureSupport              = PcdGetBool (PcdSgNvidiaGpsFeatureSupport);
  (**SgInfoDataHob).OptimusGc6FeatureSupport       = PcdGetBool (PcdSgNvidiaOptimusGc6FeatureSupport);

  //
  // GPIO Pin Setting
  //
  (**SgInfoDataHob).SgDgpuHoldRstGpioCommunityOffset         = PcdGet32 (PcdSgDgpuHoldRstGpioCommunityOffset);
  (**SgInfoDataHob).SgDgpuHoldRstGpioPinOffset               = PcdGet32 (PcdSgDgpuHoldRstGpioPinOffset);
  (**SgInfoDataHob).SgDgpuHoldRstGpioPinActiveInformation    = PcdGetBool (PcdSgDgpuHoldRstGpioPinActiveInformation);
  (**SgInfoDataHob).SgDgpuPwrEnableGpioCommunityOffset       = PcdGet32 (PcdSgDgpuPwrEnableGpioCommunityOffset);
  (**SgInfoDataHob).SgDgpuPwrEnableGpioPinOffset             = PcdGet32 (PcdSgDgpuPwrEnableGpioPinOffset);
  (**SgInfoDataHob).SgDgpuPwrEnableGpioPinActiveInformation  = PcdGetBool (PcdSgDgpuPwrEnableGpioPinActiveInformation);
  (**SgInfoDataHob).SgDgpuEnable                             = PcdGetBool (PcdSgDgpuEnable);
  (**SgInfoDataHob).SgDgpuDisable                            = PcdGetBool (PcdSgDgpuDisable);
  (**SgInfoDataHob).SgDgpuPEnable                            = PcdGetBool (PcdSgDgpuPEnable);
  (**SgInfoDataHob).SgDgpuPDisable                           = PcdGetBool (PcdSgDgpuPDisable);
  
  CopyMem (&((**SgInfoDataHob).SgModeMxmBinaryGuid), PcdGetPtr (PcdSgModeMxmBinaryGuid), sizeof (EFI_GUID));
  CopyMem (&((**SgInfoDataHob).AmdPowerXpressSsdtGuid), PcdGetPtr (PcdAmdPowerXpressSsdtGuid), sizeof (EFI_GUID));
  CopyMem (&((**SgInfoDataHob).NvidiaOptimusSsdtGuid), PcdGetPtr (PcdNvidiaOptimusSsdtGuid), sizeof (EFI_GUID));
  
//[-start-160421-IB08450342-remove]//
  ////
  //// Locate PEI Read Only Variable PPI
  ////
  //Status = PeiServicesLocatePpi (
  //           &gEfiPeiReadOnlyVariable2PpiGuid,
  //           0,
  //           NULL,
  //           (VOID **)&VariableServices
  //           );
  //if (EFI_ERROR (Status)) {
  //  return Status;
  //}
  //
  ////
  //// Get BootType Variable
  ////
  //BootType = DUAL_BOOT_TYPE;
  //VariableSize = PcdGet32 (PcdSetupConfigSize);
  //Status = VariableServices->GetVariable (
  //                              VariableServices,
  //                              L"BootType",
  //                              &gSystemConfigurationGuid,
  //                              NULL,
  //                              &VariableSize,
  //                              &BootType
  //                              );
//[-end-160421-IB08450342-remove]//

  //
  // System Configuration variables to set SG mode default value, 
  // 0 = SgModeDisabled, 1 = SgModeMuxed, 2 = SgModeMuxless, 3 = SgModeDgpu.
  //
  (**SgInfoDataHob).SgMode = SetSgModeValue (SystemConfiguration);
//[-start-160421-IB08450342-modify]//
  (**SgInfoDataHob).BootType = SystemConfiguration.BootType;
//[-end-160421-IB08450342-modify]//

  return EFI_SUCCESS;
}

/**

  Base on system configuration to set Switchable Graphics own SG mode variable.

  @param[in] SystemConfiguration   It's the setup variables (system configuration).

  @retval SgModeDisabled   SG mode should be set to disabled on internal graphics only platform,
                           and should not power enable the discrete GPU.
  @retval SgModeMuxless    SG mode should be set to Muxless on dual graphics platform,
                           dual graphics platform ever used multiplexer to choose VGA output
                           that called Muxed, Muxless indicate the platform doesn't have multiplexer
                           and every output just go through internal or discrete GPU.
  @retval SgModeDgpu       SG mode should be set to discrete GPU on discrete graphics only platform,
                           if the discrete GPU used MXM interface, discrete GPU should be powered on.

**/
STATIC
SG_MODE_SETTING
SetSgModeValue (
  IN SYSTEM_CONFIGURATION                     SystemConfiguration
  )
{
  SG_MODE_SETTING                             SgMode;

  //
  // Set SG mode default
  //
  SgMode = SgModeDisabled;

  //
  // Base on System Configuration variable data to change SG mode
  //
  if (SystemConfiguration.PrimaryVideoAdaptor == DisplayModeSg) {
    SgMode = SgModeMuxless;
  }

  if ((SystemConfiguration.PrimaryVideoAdaptor == DisplayModeDgpu) ||
      (SystemConfiguration.PrimaryVideoAdaptor == DisplayModeAuto) ||
      (SystemConfiguration.PrimaryVideoAdaptor == DisplayModePci)  ||
      (SystemConfiguration.Igd == IgdDisable)) {
    SgMode = SgModeDgpu;
  }

  if ((SystemConfiguration.EnSGFunction == DisableSgFunction) ||
      (SystemConfiguration.PrimaryVideoAdaptor == DisplayModeIgpu)) {
    SgMode = SgModeDisabled;
  }

  return SgMode;
}

/**

  Power enable MXM (Mobile PCI Express Module) interface discrete GPU in this function,
  and reserved a hook point for OEM to customization.

  @param[in, out] SgInfoDataHob   A pointer of SG information data HOB.
  @param[in]      SgPpi           A pointer of PEI Switchable Graphics PPI Function
                                  for SG GPIO read/write and stall.

  @retval None.

**/
STATIC
VOID
MxmDgpuPowerEnableSequence (
  IN H2O_SWITCHABLE_GRAPHICS_PPI              SgPpi,
  IN OUT SG_INFORMATION_DATA_HOB              *SgInfoDataHob
  )
{
  EFI_STATUS                                  Status;

  //
  // OEM service hook for OEM customization,
  // base on project own discrete GPU power enable sequence to enable the GPU,
  // and OEM can base on SG mode variable to do different sequence,
  // or base on SKU ID to change SG mode or any SG information data HOB too.
  //
  Status = OemSvcMxmDgpuPowerSequence ();
  if (!EFI_ERROR (Status)) {
    //
    // If return EFI_SUCCESS, it means MXM GPU power sequence already done and success,
    // doesn't need do anythings else in this function, so just return.
    //
    return;
  }
  //
  // If return EFI_UNSUPPORTED, it means the OEM service doesn't do anything then returned go through here.
  // If return EFI_MEDIA_CHANGED, it means the OEM service change the SG information data HOB,
  // but still need MXM GPU power enable sequence in this function.
  //
#ifndef BRASWELL_CRB
  if (SgInfoDataHob->SgGpioSupport == SgGpioNotSupported) {
    return;
  }
  //
  // Initialize the Active pin
  //
  if (SgInfoDataHob->SgDgpuHoldRstGpioPinActiveInformation == TRUE) {
    SgInfoDataHob->SgDgpuEnable = TRUE;
    SgInfoDataHob->SgDgpuDisable = FALSE;
  } else {
    SgInfoDataHob->SgDgpuEnable = FALSE;
    SgInfoDataHob->SgDgpuDisable = TRUE;
  }
  if (SgInfoDataHob->SgDgpuPwrEnableGpioPinActiveInformation == TRUE) {
    SgInfoDataHob->SgDgpuPEnable = TRUE;
    SgInfoDataHob->SgDgpuPDisable = FALSE;
  } else {
    SgInfoDataHob->SgDgpuPEnable = FALSE;
    SgInfoDataHob->SgDgpuPDisable = TRUE;
  }
  //
  // If this platform used GPIO to control MXM interface discrete GPU power,
  // through default sequence to power enable the GPU for CRB.
  //
  if (SgInfoDataHob->SgMode == SgModeDisabled) {
    //
    // If can not detect GPU on MXM slot or SG mode disable (internal only SKU),
    // make sure the GPU power down, and select internal GPU to control output.
    //
//    if (SgPpi.GpioRead (SgInfoDataHob->SgDgpuPwrEnable) == Active) {
//      //
//      // If GPIO default set discrete GPU power enable,
//      // through the power down sequence to disable the discrete GPU.
//      // Power down sequence: (1) Active DgpuHoldRst 
//      //                      (2) Inactive DgpuPwrEnable
//      //
//      SgPpi.GpioWrite (
//          SgInfoDataHob->SgDgpuHoldRstGpioCommunityOffset, 
//          SgInfoDataHob->SgDgpuHoldRstGpioPinOffset,
//          SgInfoDataHob->SgDgpuHoldRstGpioPinActiveInformation,
//          SgInfoDataHob->SgDgpuEnable
//          );
//       SgPpi.GpioWrite (
//       SgInfoDataHob->SgDgpuPwrEnableGpioCommunityOffset,
//       SgInfoDataHob->SgDgpuPwrEnableGpioPinOffset,
//       SgInfoDataHob->SgDgpuPwrEnableGpioPinActiveInformation,
//       SgInfoDataHob->SgDgpuPDisable
//       );
//    } else {
       SgPpi.GpioWrite (
           SgInfoDataHob->SgDgpuHoldRstGpioCommunityOffset, 
           SgInfoDataHob->SgDgpuHoldRstGpioPinOffset,
           SgInfoDataHob->SgDgpuHoldRstGpioPinActiveInformation,
           SgInfoDataHob->SgDgpuDisable
           );    
//      }
  } else {
    //
    // Power enable the discrete GPU when dual VGA mode (SG) or discrete GPU only mode.
    // Power enable sequence: (1) Active DgpuHoldRst => Delay 100ms
    //                        (2) Active DgpuPwrEnable => Delay 300ms
    //                        (3) Inactive DgpuHoldRst => Delay 100ms
    // The delay time after GPIO active or inactive that can tune by project for optimization.
    //

    SgPpi.GpioWrite (
        SgInfoDataHob->SgDgpuHoldRstGpioCommunityOffset, 
        SgInfoDataHob->SgDgpuHoldRstGpioPinOffset,
        SgInfoDataHob->SgDgpuHoldRstGpioPinActiveInformation,
        SgInfoDataHob->SgDgpuEnable
        );

    SgPpi.Stall (SgInfoDataHob->ActiveDgpuHoldRstDelay * 1000);
    
    SgPpi.GpioWrite (
        SgInfoDataHob->SgDgpuPwrEnableGpioCommunityOffset,
        SgInfoDataHob->SgDgpuPwrEnableGpioPinOffset,
        SgInfoDataHob->SgDgpuPwrEnableGpioPinActiveInformation,
        SgInfoDataHob->SgDgpuPEnable
        );

    SgPpi.Stall (SgInfoDataHob->ActiveDgpuPwrEnableDelay * 1000);

//   SgPpi.GpioWrite (
//       SgInfoDataHob->SgDgpuHoldRstGpioCommunityOffset, 
//       SgInfoDataHob->SgDgpuHoldRstGpioPinOffset,
//       SgInfoDataHob->SgDgpuHoldRstGpioPinActiveInformation,
//       SgInfoDataHob->SgDgpuDisable
//       );

//    SgPpi.Stall (SgInfoDataHob->InactiveDgpuHoldRstDelay * 1000);

  }
#endif
}

/**

  SG own related GPIO pin read function.

  @param[in] Value   Bit 0 to 6 decide which pin will be read,
                     bit 7 decide this pin high active or low active,
                     bit 8 to 15 decide the GPIO Group.

  @retval TRUE    This GPIO pin is active this moment.
  @retval FALSE   This GPIO pin is inactive this moment.

**/
BOOLEAN
EFIAPI
SwitchableGraphicsGpioRead (
  IN UINT16                                   GpioData
  )
{
  return 0;
}

/**

  SG own related GPIO pin write function.

  @param[in] Value   Bit 0 to 6 decide which pin will be read,
                     bit 7 decide this pin high active or low active,
                     bit 8 to 15 decide the GPIO Group.
  @param[in] Level   TRUE  Base on GPIO value information to let this GPIO pin "active".
                     FALSE Base on GPIO value information to let this GPIO pin "inactive".

  @retval None.

**/
VOID
EFIAPI
SwitchableGraphicsGpioWrite (
  IN       UINT32                 CommunityOffset,
  IN       UINT32                 PinOffset,
  IN       BOOLEAN                Active,
  IN       BOOLEAN                Level
  )
{
  UINT32  Data32;

  if (Active == 0) {
    Level = (~Level) & 0x1;
  }

  Data32 = MmioRead32 ((UINTN) (IO_BASE_ADDRESS + CommunityOffset + PinOffset));

  if (Level == 1) {
  Data32 |= (BIT1);
  } else {
      Data32 &= ~(BIT1);
  }

  MmioWrite32 ((UINTN) (IO_BASE_ADDRESS + CommunityOffset + PinOffset), Data32);
}

/**

  Waits for at least the given number of microseconds.

  @param[in]  Microseconds   - Desired length of time to wait
  
  @retval   EFI_SUCCESS    - If the desired amount of time passed.
  @retval   !EFI_SUCCESS   - If error occurs while locating CpuIoPpi.

**/
EFI_STATUS
EFIAPI
SwitchableGraphicsStall (
  IN UINTN                                    Microseconds
  )
{
  UINT32                                      CurrentTick;
  UINT32                                      OriginalTick;
  UINT32                                      RemainingTick;
  UINTN                                       Counts;
  UINTN                                       Ticks;

  if (Microseconds == 0) {
    return EFI_SUCCESS;
  }

  OriginalTick  = IoRead32 (ACPI_TIMER_ADDR) & 0x00FFFFFF;

  CurrentTick   = OriginalTick;

  //
  // The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
  //
  // The loops needed by timer overflow
  //
  Counts = Ticks / ACPI_TIMER_MAX_VALUE;
  //
  // remaining clocks within one loop
  //
  RemainingTick = Ticks % ACPI_TIMER_MAX_VALUE;
  //
  // not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  // one I/O operation, and maybe generate SMI
  //
  while (Counts != 0) {
    CurrentTick = IoRead32 (ACPI_TIMER_ADDR) & 0x00FFFFFF;

    if (CurrentTick <= OriginalTick) {
      Counts--;
    }

    OriginalTick = CurrentTick;
  }

  while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
    OriginalTick  = CurrentTick;
    CurrentTick   = (IoRead32 (ACPI_TIMER_ADDR) & 0x00FFFFFF);
  }

  return EFI_SUCCESS;
}
