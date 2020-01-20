/*++

Copyright (c)  1999 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  BootMode.c

Abstract:

  EFI PEIM to provide the platform support functionality on the Thurley.


--*/

#include "PlatformInit.h"
//[-start-160824-IB07220130-add]//
#include <Library/CmosLib.h>
//[-end-160824-IB07220130-add]//

#define NORMALMODE        0
#define RECOVERYMODE      1
#define SAFEMODE          2
#define MANUFACTURINGMODE 3

EFI_PEI_PPI_DESCRIPTOR  mPpiListRecoveryBootMode = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiBootInRecoveryModePpiGuid,
  NULL
};


UINTN
GetConfigJumper(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
 )
/*++

Routine Description:

  Return the setting of the Bios configuration jumper

Arguments:

  VOID

Returns:

  RECOVERYMODE      - jumper set to recovery mode
  SAFEMODE          - jumper set to config mode
  NORMALMODE        - jumper in normal mode

--*/
{
  //
  // Do the Forced recovery detection based on logic chart above
  //
  
  /*  IsRecoveryJumper is hardcoded to False, so skip this.
  
  if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
    if (IsRecoveryJumper(PeiServices, PlatformInfoHob)) {
      return RECOVERYMODE;
    } else {
      return NORMALMODE;
    }
  } else {//pre-silicon
    return NORMALMODE;
  } */
  return NORMALMODE;
}


BOOLEAN
CheckIfRecoveryMode(
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
 )
{
//[-start-151216-IB07220025-add]//
  EFI_STATUS    Status;
  BOOLEAN       IsRecovery = FALSE;

  Status = OemSvcDetectRecoveryRequest (
             &IsRecovery
             );
  if (IsRecovery) {
    return TRUE;
  }
//[-end-151216-IB07220025-add]//
  if (GetConfigJumper(PeiServices, PlatformInfoHob) == RECOVERYMODE) {
    return TRUE;
  }
  return FALSE;
}


BOOLEAN
CheckIfSafeMode(
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
 )
{
  if (GetConfigJumper(PeiServices, PlatformInfoHob) == SAFEMODE) {
    return TRUE;
  }
  return FALSE;
}


BOOLEAN
CheckIfManufacturingMode (
  IN CONST EFI_PEI_SERVICES  **PeiServices
 )
{
  EFI_STATUS                  Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *Variable;
  UINT32                      Attributes;
  UINTN                       DataSize;
  CHAR16                      VarName[] = MFGMODE_VARIABLE_NAME;
  UINT8                       MfgMode;

  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
                             0,
                             NULL,
                             (VOID **) &Variable
                             );
  ASSERT_EFI_ERROR (Status);

  //
  // Check if SW MMJ mode
  //
  Attributes = (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS);
  DataSize = sizeof (MFG_MODE_VAR);

  Status = Variable->GetVariable (
                        Variable,
                        VarName,
                        &gMfgModeVariableGuid,
                        &Attributes,
                        &DataSize,
                        &MfgMode
                        );
  if (!(EFI_ERROR (Status))) {
    return TRUE;
  }
  return FALSE;
}


EFI_STATUS
EFIAPI
GetWakeupEventAndSaveToHob (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
/*++

Routine Description:

  Parse the status registers for figuring out the wake-up event and save it into
  an GUID HOB which will be referenced later. However, modification is required
  to meet the chipset register definition and the practical hardware design. Thus,
  this is just an example.

Arguments:

  PeiServices   - pointer to the PEI Service Table
  EFI_SUCCESS   - Always return Success
Returns:

  None


--*/
{
  UINT16  AcpiBaseAddr;
  UINT16  Pm1Sts;
  UINTN   Gpe0Sts;
  UINTN   WakeEventData;

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  
  //
  // Read the ACPI registers
  //
  Pm1Sts  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_STS);
  Gpe0Sts = IoRead32 (AcpiBaseAddr + R_ACPI_GPE0a_STS);

  //
  // Figure out the wake-up event
  //
  if ((Pm1Sts & B_ACPI_PM1_STS_PWRBTN) != 0) {
    WakeEventData = SMBIOS_WAKEUP_TYPE_POWER_SWITCH;
  } else if (((Pm1Sts & B_ACPI_PM1_STS_WAK) != 0)) {
    WakeEventData = SMBIOS_WAKEUP_TYPE_PCI_PME;
  } else if (Gpe0Sts != 0) {
    WakeEventData = SMBIOS_WAKEUP_TYPE_OTHERS;
  } else {
    WakeEventData = SMBIOS_WAKEUP_TYPE_UNKNOWN;
  }

  DEBUG ((EFI_D_INFO, "ACPI Wake Status Register: %04x\n", Pm1Sts));

  return EFI_SUCCESS;
}

//[-start-160824-IB07220130-add]//
/**
  Check secure flash image under S3 boot mode

  @param[in]  None

  @retval   Returns TRUE if the previous firmware update failed, otherwise returns FALSE

**/
BOOLEAN
DetectPendingUpdateImage (
  VOID
  )
{
  if (ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosSecureFlashS3Flag)) {
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosSecureFlashS3Flag, 2);
    return TRUE;
  }

  return FALSE;
}
//[-end-160824-IB07220130-add]//

//[-start-161117-IB07400815-add]//
EFI_STATUS
ResetCmosData (
  VOID
  )
{
  UINT8   CmosData;

  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
  CmosData &= (~B_CMOS_ACPI_CALLBACK_NOTIFY);
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
  
  return EFI_SUCCESS;
}
//[-end-161117-IB07400815-add]//

EFI_STATUS
UpdateBootMode (
  IN CONST EFI_PEI_SERVICES                       **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB                    *PlatformInfoHob
  )
{
  EFI_STATUS                        Status;
  EFI_BOOT_MODE                     BootMode;
  UINT16                            SleepType;
  CHAR16                            *strBootMode;
//[-start-160824-IB07220130-remove]//
//   PEI_CAPSULE_PPI                   *Capsule;
//[-end-160824-IB07220130-remove]//
//[-start-161117-IB07400815-add]//
  //
  // Reset Cmos Data
  //
  ResetCmosData ();
//[-end-161117-IB07400815-add]//
  DEBUG ((EFI_D_INFO, "PEIM UpdateBootMode Enter: \n"));
  Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);
  ASSERT_EFI_ERROR (Status);
  if (BootMode  == BOOT_IN_RECOVERY_MODE){
    DEBUG ((EFI_D_INFO, "PEIM UpdateBootMode Enter: Set Recovery Mode \n"));
    return Status;
  }
  DEBUG ((EFI_D_INFO, "GetWakeupEventAndSaveToHob \n"));
  GetWakeupEventAndSaveToHob (PeiServices);

  //
  // Let's assume things are OK if not told otherwise
  //
  BootMode = BOOT_WITH_FULL_CONFIGURATION;

  //
  // Check if we need to boot in forced recovery mode
  //
  if (CheckIfRecoveryMode(PeiServices, PlatformInfoHob)) {
    DEBUG ((EFI_D_ERROR, "Set Boot mode in recovery mode \n"));
    BootMode  = BOOT_IN_RECOVERY_MODE;
  }

  if (BootMode  == BOOT_IN_RECOVERY_MODE) {
//[-start-151224-IB07220029-modify]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
    //
    //  Not install gEfiPeiBootInRecoveryModePpiGuid to prevent loading recovery related drivers in EmuPEI phase.
    //  There is no need to load again in EmuPEI phase.
    //     
    Status = (*PeiServices)->LocatePpi ( 
                               PeiServices,
                               &gEmuPeiPpiGuid,
                               0,
                               NULL,
                               NULL
                               );
    if (EFI_ERROR(Status)) {
#endif
      Status = (*PeiServices)->InstallPpi (PeiServices, &mPpiListRecoveryBootMode);
      ASSERT_EFI_ERROR (Status);
#ifndef FSP_WRAPPER_FLAG
    }
#endif
//[-end-161123-IB07250310-modify]//
//[-end-151224-IB07220029-modify]//
  } else {
    if (GetSleepTypeAfterWakeup (PeiServices, &SleepType)) {
      switch (SleepType) {
        case V_ACPI_PM1_CNT_S3:
//[-start-160824-IB07220130-modify]//
          //
          // Determine if we're in capsule update mode
          //
          if (DetectPendingUpdateImage ()) {
            BootMode = BOOT_ON_FLASH_UPDATE;
          } else {
            BootMode = BOOT_ON_S3_RESUME;
          }           
          break;
//[-end-160824-IB07220130-modify]//

        case V_ACPI_PM1_CNT_S4:
          BootMode = BOOT_ON_S4_RESUME;
          break;

        case V_ACPI_PM1_CNT_S5:
          BootMode = BOOT_ON_S5_RESUME;
          break;
      } // switch (SleepType)
    }
    // Check for Safe Mode
  }

  switch (BootMode) {
    case BOOT_WITH_FULL_CONFIGURATION:
      strBootMode = L"BOOT_WITH_FULL_CONFIGURATION";
      break;
    case BOOT_WITH_MINIMAL_CONFIGURATION:
      strBootMode = L"BOOT_WITH_MINIMAL_CONFIGURATION";
      break;
    case BOOT_ASSUMING_NO_CONFIGURATION_CHANGES:
      strBootMode = L"BOOT_ASSUMING_NO_CONFIGURATION_CHANGES";
      break;
    case BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS:
      strBootMode = L"BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS";
      break;
    case BOOT_WITH_DEFAULT_SETTINGS:
      strBootMode = L"BOOT_WITH_DEFAULT_SETTINGS";
      break;
    case BOOT_ON_S4_RESUME:
      strBootMode = L"BOOT_ON_S4_RESUME";
      break;
    case BOOT_ON_S5_RESUME:
      strBootMode = L"BOOT_ON_S5_RESUME";
      break;
    case BOOT_ON_S2_RESUME:
      strBootMode = L"BOOT_ON_S2_RESUME";
      break;
    case BOOT_ON_S3_RESUME:
      strBootMode = L"BOOT_ON_S3_RESUME";
      break;
    case BOOT_ON_FLASH_UPDATE:
      strBootMode = L"BOOT_ON_FLASH_UPDATE";
      break;
    case BOOT_IN_RECOVERY_MODE:
      strBootMode = L"BOOT_IN_RECOVERY_MODE";
      break;
    default:
      strBootMode = L"Unknown boot mode";
  } // switch (BootMode)

  DEBUG ((EFI_D_INFO, "Setting BootMode to %s\n", strBootMode));
  Status = (*PeiServices)->SetBootMode(PeiServices, BootMode);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

BOOLEAN
GetSleepTypeAfterWakeup (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  OUT UINT16                    *SleepType
  )
/*++

Routine Description:

  Get sleep type after wakeup

Arguments:

  PeiServices       Pointer to the PEI Service Table.
  SleepType         Sleep type to be returned.

Returns:

  TRUE              A wake event occured without power failure.
  FALSE             Power failure occured or not a wakeup.

--*/
{
  UINT16  Pm1Sts;
  UINT16  Pm1Cnt;
  UINTN   GenPmCon1;
  UINT16  AcpiBaseAddr;
  
  //
  // VLV BIOS Specification 0.6.2 - Section 18.4, "Power Failure Consideration"
  //
  // When the SUS_PWR_FLR bit is set, it indicates the SUS well power is lost.
  // This bit is in the SUS Well and defaults to 1'b1 based on RSMRST# assertion (not cleared by any type of reset).
  // System BIOS should follow cold boot path if SUS_PWR_FLR (PBASE + 0x20[14]),
  // GEN_RST_STS (PBASE + 0x20[9]) or PWRBTNOR_STS (ABASE + 0x00[11]) is set to 1'b1
  // regardless of the value in the SLP_TYP (ABASE + 0x04[12:10]) field.
  //
  GenPmCon1 = MmioRead32 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  
  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  
  //
  // Read the ACPI registers
  //
  Pm1Sts  = IoRead16  (AcpiBaseAddr + R_ACPI_PM1_STS);
  Pm1Cnt  = IoRead16  (AcpiBaseAddr + R_ACPI_PM1_CNT);
  
  DEBUG ((EFI_D_INFO, "ACPI Pm1Cnt Register: %04x\n", Pm1Cnt));
  DEBUG ((EFI_D_INFO, "ACPI Pm1Sts Register: %04x\n", Pm1Sts));
  DEBUG ((EFI_D_INFO, "PMC GenPmCon1 Register: %08x\n", GenPmCon1));
  if(GenPmCon1 != 0xFFFF){  
    if ((GenPmCon1 & (B_PMC_GEN_PMCON_COLD_BOOT_STS | B_PMC_GEN_PMCON_WARM_RST_STS)) ||\
       (Pm1Sts & B_ACPI_PM1_STS_PRBTNOR) || \
       (((Pm1Sts & B_ACPI_PM1_STS_WAK) == 0) && ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3))) {
      // If power failure indicator, then don't attempt s3 resume.
      // Clear PM1_CNT of S3 and set it to S5 as we just had a power failure, and memory has
      // lost already.  This is to make sure no one will use PM1_CNT to check for S3 after
      // power failure.
      if ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) {
        Pm1Cnt = ((Pm1Cnt & ~B_ACPI_PM1_CNT_SLP_TYP) | V_ACPI_PM1_CNT_S5);
        IoWrite16  (AcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);
      }
      // Clear Wake Status (WAK_STS)
      //
      IoWrite16 ((AcpiBaseAddr + R_ACPI_PM1_STS), B_ACPI_PM1_STS_WAK);
    } 
  }
  //
  // Get sleep type if a wake event occurred and there is no power failure
  //
  if ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) {
    *SleepType = Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP;
    return TRUE;
  } else if((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S4){
    *SleepType = Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP;
    return TRUE;
  }
  return FALSE;
}


//VOID
//SetPlatformBootMode (
//  IN CONST EFI_PEI_SERVICES             **PeiServices,
//  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
//  )
//{
//  EFI_PLATFORM_SETUP_ID       PlatformSetupId;
//
//  ZeroMem(&PlatformSetupId, sizeof (EFI_PLATFORM_SETUP_ID));
//
//  CopyMem (&PlatformSetupId.SetupGuid,
//                           &gEfiNormalSetupGuid,
//                           sizeof (EFI_GUID));
//
//  if (CheckIfRecoveryMode(PeiServices, PlatformInfoHob)) {
//    //
//    // Recovery mode
//    //
//    CopyMem (&PlatformSetupId.SetupName,
//             SAFE_SETUP_NAME,
//             StrSize (SAFE_SETUP_NAME));
//    PlatformSetupId.PlatformBootMode = PLATFORM_RECOVERY_MODE;
//  } else if (CheckIfSafeMode(PeiServices, PlatformInfoHob)) {
//    //
//    // Safe mode also called config mode or maintenace mode.
//    //
//    CopyMem (&PlatformSetupId.SetupName,
//             SAFE_SETUP_NAME,
//             StrSize (SAFE_SETUP_NAME));
//    PlatformSetupId.PlatformBootMode = PLATFORM_SAFE_MODE;
//
//  } else if(0) { // else if (CheckIfManufacturingMode(PeiServices)) {
//    //
//    // Manufacturing mode
//    //
//    CopyMem (&PlatformSetupId.SetupName,
//             MANUFACTURE_SETUP_NAME,
//             StrSize (MANUFACTURE_SETUP_NAME));
//    PlatformSetupId.PlatformBootMode = PLATFORM_MANUFACTURING_MODE;
//
//  } else {
//    //
//    // Default to normal mode.
//    //
//    CopyMem (&PlatformSetupId.SetupName,
//             &NORMAL_SETUP_NAME,
//             StrSize (NORMAL_SETUP_NAME));
//    PlatformSetupId.PlatformBootMode = PLATFORM_NORMAL_MODE;
//  }
//
//  BuildGuidDataHob (&gEfiPlatformBootModeGuid,
//                    &PlatformSetupId,
//                    sizeof (EFI_PLATFORM_SETUP_ID));
//  return;
//}
