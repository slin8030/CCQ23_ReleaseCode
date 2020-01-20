/** @file
  Platform Emmc Dxe driver to configure Emmc Hs400 Tuning
  and set variable for Emmc Hs400 Tuning Data.

@copyright
 Copyright (c) 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/
#include "PlatformEmmcDxe.h"
#include <Library/DxeInsydeChipsetLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//

/**
  After EFI_EMMC_CARD_INFO_PROTOCOL and PCH_EMMC_TUNING_PROTOCOL installed,
  configure platform Emmc for Hs400 Tuning and set variable for HS400 Tuning Data.

  @param[in] Event                A pointer to the Event that triggered the callback.
  @param[in] Context              A pointer to private data registered with the callback function. 

**/
VOID
EFIAPI
ConfigurePlatformEmmc (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  EFI_EMMC_CARD_INFO_PROTOCOL       *EfiEmmcApplicationRegister;
  PCH_EMMC_TUNING_PROTOCOL          *PchEmmcTuningProtocol;
  EMMC_INFO                         EmmcInfo;
  EMMC_TUNING_DATA                  EmmcTuningData;
  PLATFORM_EMMC_TUNING_DATA         PlatformEmmcTuningData;
  UINT8                             PchEmmcTuningProtocolRevision;
  UINTN                             VariableSize;
  EFI_STATUS                        Status;

  PchEmmcTuningProtocolRevision = PCH_EMMC_TUNING_PROTOCOL_REVISION;

  DEBUG ((DEBUG_INFO, "ConfigurePlatformEmmc Start()\n"));

  Status = gBS->LocateProtocol (
              &gEfiEmmcCardInfoProtocolGuid,
              NULL,
              &EfiEmmcApplicationRegister
              );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gPchEmmcTuningProtocolGuid,
                  NULL,
                  &PchEmmcTuningProtocol
                  );
  ASSERT_EFI_ERROR (Status);
  //
  // Return if eMMC device does not support HS400
  //
  if (((EfiEmmcApplicationRegister->CardData->ExtCSDRegister.CARD_TYPE & B_EMMC_HS400_12V) == FALSE) && 
     ((EfiEmmcApplicationRegister->CardData->ExtCSDRegister.CARD_TYPE & B_EMMC_HS400_18V) == FALSE)) {
    return;
  }

  //
  // Get EmmcInfo from EmmcCardInfoProtocol
  // EmmcCardInfoProtocol is produced in the driver
  //
  EmmcInfo.PartitionHandle = EfiEmmcApplicationRegister->CardData->Partitions->Handle;
  EmmcInfo.Lba = 0;
  EmmcInfo.RelativeDevAddress = (EfiEmmcApplicationRegister->CardData->Address << 16);

  if (EfiEmmcApplicationRegister->CardData->CurrentBusWidth == 4) {
    EmmcInfo.HS200BusWidth = V_EMMC_HS200_BUS_WIDTH_4;
  } else{
    EmmcInfo.HS200BusWidth = V_EMMC_HS200_BUS_WIDTH_8;
  }

  //
  // For normal boot flow
  // 1. If ScsEmmcEnabled and ScsEmmcHs400Enabled policy set,
  //    a) Set ScsEmmcHs400TuningRequired policy to state tuning required in PEI, 
  //       - if RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion, a battery failure or CMOS clear.(Boot with default settings)
  //       - if non-volatile variable 'Hs400TuningData' does not exist
  //    b) RC installed Pch Emmc Tuning Protocol regardless of ScsEmmcHs400TuningRequired policy setting.in DXE
  //    c) If boot with default settings after CMOS cleared, platform delete variable 'Hs400TuningData' in DXE
  // 2. Once RC successfully installed Pch Emmc Tuning Protocol, it will be used to perform EmmcTune for Hs400.
  // 3. Then, platform must set the variable with returned EmmcTuningData no matter tuning pass of fail
  // 4. Platform shall set variable 'Hs400TuningData' for one time only or after CMOS clear
  // 
  // For fast boot flow
  // 1. If ScsEmmcEnabled and ScsEmmcHs400Enabled policy set,
  //    a) Set ScsEmmcHs400TuningRequired policy to state tuning not required, if non-volatile variable 'Hs400TuningData' exist
  //    b) RC installed Pch Emmc Tuning Protocol regardless of ScsEmmcHs400TuningRequired policy setting in DXE
  // 2. Once RC successfully installed Pch Emmc Tuning Protocol, it will be used to perform EmmcTune
  // 3. Since ScsEmmcHs400TuningRequired state tuning not required, RC will not perform Emmc Hs400 Tuning but just set the device to operate in HS400 mode if data is valid
  // 4. Platform shall not set variable 'Hs400TuningData'
  // 
  // For both normal boot and fast boot flow
  // If platform found no Pch Emmc Tuning protocol installed, this DXE module will not dispatch
  //

  Status = PchEmmcTuningProtocol->EmmcTune (PchEmmcTuningProtocol, PchEmmcTuningProtocolRevision, &EmmcInfo, &EmmcTuningData);

  //
  // Set variable if variable 'Hs400TuningData' is not set
  //
  PlatformEmmcTuningData.Hs400DataValid     = EmmcTuningData.Hs400DataValid;
  PlatformEmmcTuningData.Hs400RxStrobe1Dll  = EmmcTuningData.Hs400RxStrobe1Dll;
  PlatformEmmcTuningData.Hs400TxDataDll     = EmmcTuningData.Hs400TxDataDll;

  DEBUG ((DEBUG_INFO, "Hs400DataValid = 0x%x \n", PlatformEmmcTuningData.Hs400DataValid));
  DEBUG ((DEBUG_INFO, "Hs400RxStrobe1Dll = 0x%x \n", PlatformEmmcTuningData.Hs400RxStrobe1Dll));
  DEBUG ((DEBUG_INFO, "Hs400TxDataDll = 0x%x \n", PlatformEmmcTuningData.Hs400TxDataDll));

  VariableSize = sizeof (PLATFORM_EMMC_TUNING_DATA);
  Status = gRT->GetVariable (
                  HS400_TUNING_DATA_VAR,
                  &gPlatformEmmcHs400TuningInfoGuid,
                  NULL,
                  &VariableSize,
                  &PlatformEmmcTuningData
                  );
  if (EFI_ERROR (Status)) {
    Status = gRT->SetVariable (
                    HS400_TUNING_DATA_VAR,
                    &gPlatformEmmcHs400TuningInfoGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    sizeof (PLATFORM_EMMC_TUNING_DATA),
                    &PlatformEmmcTuningData
                    );
    ASSERT_EFI_ERROR (Status);
  }

  DEBUG ((DEBUG_INFO, "ConfigurePlatformEmmc End()\n"));

}
/**
  Entry point for the driver.

  This routine gets info from the EFI_EMMC_CARD_INFO_PROTOCOL and perform 
  Platform Emmc Hs400 mode tuning.
   
  @param[in] ImageHandle              Image Handle.
  @param[in] SystemTable              EFI System Table.

**/
EFI_STATUS
EFIAPI
PlatformEmmcInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                        Status;
  CHIPSET_CONFIGURATION             SetupData;
  UINTN                             VariableSize;
  PLATFORM_EMMC_TUNING_DATA         PlatformEmmcTuningData;
  EFI_EVENT                         EmmcApplicationRegisterEvent;
  VOID                              *EmmcApplicationRegisterEventRegistration;
  DEBUG ((DEBUG_ERROR, "PlatformEmmcInit Start()\n"));
  //
  // Delete Variable 'Hs400TuningData' if boot with default settings after CMOS clear
  //
  if (GetBootModeHob() == BOOT_WITH_DEFAULT_SETTINGS) {
    VariableSize = sizeof (PLATFORM_EMMC_TUNING_DATA);
   Status = gRT->GetVariable (
                    HS400_TUNING_DATA_VAR,
                    &gPlatformEmmcHs400TuningInfoGuid,
                    NULL,
                   &VariableSize,
                    &PlatformEmmcTuningData
                    );

    if (Status == EFI_SUCCESS) {
      Status = gRT->SetVariable (
                      HS400_TUNING_DATA_VAR,
                      &gPlatformEmmcHs400TuningInfoGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                      0,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
    }
  }

  VariableSize = sizeof (CHIPSET_CONFIGURATION);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SetupData, &VariableSize);
  Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    NULL,
                    &VariableSize,
                    &SetupData
                    );
//[-end-160803-IB07220122-modify]//
  ASSERT_EFI_ERROR (Status);

  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SetupData, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      return Status;  
    }
  }

  if ((SetupData.ScceMMCEnabled == FALSE) || (SetupData.ScceMMCHs400Enabled == FALSE)) {
    return EFI_ABORTED;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ConfigurePlatformEmmc,
                  NULL,
                  &EmmcApplicationRegisterEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->RegisterProtocolNotify (
                  &gEfiEmmcCardInfoProtocolGuid,
                  EmmcApplicationRegisterEvent,
                  &EmmcApplicationRegisterEventRegistration
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_ERROR, "PlatformEmmcInit end()\n"));
  return EFI_SUCCESS;
}

