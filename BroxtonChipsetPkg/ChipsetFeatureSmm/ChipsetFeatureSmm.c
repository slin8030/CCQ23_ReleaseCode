/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*++

Module Name:

    ChipsetFeatureSmm.c

Abstract:

    This is a generic template for a child of the ChipsetFeatureSmm driver.

 
--*/

#include "ChipsetFeatureSmm.h"

//[-start-161027-IB07400806-modify]//
#ifdef APOLLOLAKE_CRB
#define CHIPSET_SWSMI_TEST_ONLY
#endif
//[-end-161027-IB07400806-modify]//

//
// Test Only
//
#define CHIPSET_SWSMI_TEST_PREPARE_FOR_UPDATE   0xD0
#define CHIPSET_SWSMI_TEST_SW_GPE               0xD1
//[-start-170315-IB07400848-add]//
//[-start-170329-IB07400853-modify]//
#define CHIPSET_SWSMI_SMM_VARIABLE_TEST         0xD2
//[-end-170329-IB07400853-modify]//
//[-end-170315-IB07400848-add]//

//[-start-161117-IB07400815-add]//
#define CHIPSET_RUNTIME_AND_ASL_POST_CODE       0x01
//[-end-161117-IB07400815-add]//

//[-start-160923-IB07400789-add]//
#define LPC_BUS                                 DEFAULT_PCI_BUS_NUMBER_SC
#define LPC_DEV                                 PCI_DEVICE_NUMBER_PCH_LPC
#define LPC_FUN                                 PCI_FUNCTION_NUMBER_PCH_LPC
#define R_LUKMC                                 0x94

#define EBDA(a)                                 (*(UINT8*)(UINTN)(((*(UINT16*)(UINTN)0x40e) << 4) + (a)))
#define EBDA_POINTER_DEVICE_FLAG                0x032
#define BDA_KBBHEAD                             (*(UINT16*)(UINTN)(0x41a))
#define BDA_KBBTAIL                             (*(UINT16*)(UINTN)(0x41c))
//[-end-160923-IB07400789-add]//

VOID
StallInternalFunction(
  IN UINTN              Microseconds
  )
{
  UINTN                 Ticks;
  UINTN                 Counts;
  UINT32                CurrentTick;
  UINT32                OriginalTick;
  UINT64                RemainingTick;
  UINT16                mAcpiBaseAddr;

  mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

//[-start-180813-IB07400997-modify]//
  OriginalTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//

  CurrentTick = OriginalTick;

  //
  //The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
  Counts = Ticks / V_ACPI_PM1_TMR_MAX_VAL;  //The loops needed by timer overflow
  RemainingTick = Ticks % V_ACPI_PM1_TMR_MAX_VAL;  //remaining clocks within one loop

  //
  //not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  //one I/O operation, and maybe generate SMI
  //

  while (Counts != 0) {
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
    if (CurrentTick < OriginalTick) {
//[-end-180813-IB07400997-modify]//
      Counts --;
    }
    OriginalTick = CurrentTick;
  }

  while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick) ) {
    OriginalTick = CurrentTick;
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//
  }
}

EFI_STATUS
Stall (
  IN UINTN              Microseconds
  )
/*++

Routine Description:

  Waits for at least the given number of microseconds.

Arguments:

  Microseconds          - Desired length of time to wait

Returns:

  EFI_SUCCESS           - If the desired amount of time passed.

--*/
{
  if (Microseconds == 0) {
    return EFI_SUCCESS;
  }

  StallInternalFunction(Microseconds);

  return EFI_SUCCESS;
}

EFI_STATUS
ChipsetFeatureSwSmiCallBack (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  )
{
  UINT8       Function;
  EFI_STATUS  Status;
//[-start-161117-IB07400815-add]//
  UINT32      RtPostCodeData;
//[-end-161117-IB07400815-add]//

  Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, 0xB3, 1, &Function);
  
//[-start-161117-IB07400815-modify]//
  if (Function != CHIPSET_RUNTIME_AND_ASL_POST_CODE) {
    DEBUG ((EFI_D_ERROR, "ChipsetFeatureSwSmiCallBack, Fun = %x\n", Function));
    CHIPSET_POST_CODE (SMI_CHIPSET_FEATURE);
    CHIPSET_POST_CODE (Function);
  }
//[-end-161117-IB07400815-modify]//
  
  switch (Function) {
#ifdef CHIPSET_SWSMI_TEST_ONLY

  case CHIPSET_SWSMI_TEST_PREPARE_FOR_UPDATE:
    DEBUG ((EFI_D_ERROR, "Before HeciIfwiPrepareForUpdate...\n"));
    DEBUG ((EFI_D_ERROR, "Offset 0x40:0x%08x\n", HeciPciRead32(R_SEC_FW_STS0)));
    DEBUG ((EFI_D_ERROR, "Offset 0x48:0x%08x\n", HeciPciRead32(0x48)));
    DEBUG ((EFI_D_ERROR, "Offset 0x60:0x%08x\n", HeciPciRead32(0x60)));
    DEBUG ((EFI_D_ERROR, "Offset 0x64:0x%08x\n", HeciPciRead32(0x64)));
    DEBUG ((EFI_D_ERROR, "Offset 0x68:0x%08x\n", HeciPciRead32(0x68)));
    DEBUG ((EFI_D_ERROR, "Offset 0x6C:0x%08x\n", HeciPciRead32(0x6C)));
    
    Status = HeciIfwiPrepareForUpdate();
    DEBUG ((EFI_D_ERROR, "HeciIfwiPrepareForUpdate %x\n", Status));
    DEBUG ((EFI_D_ERROR, "After HeciIfwiPrepareForUpdate...\n"));
    DEBUG ((EFI_D_ERROR, "Offset 0x40:0x%08x\n", HeciPciRead32(R_SEC_FW_STS0)));
    DEBUG ((EFI_D_ERROR, "Offset 0x48:0x%08x\n", HeciPciRead32(0x48)));
    DEBUG ((EFI_D_ERROR, "Offset 0x60:0x%08x\n", HeciPciRead32(0x60)));
    DEBUG ((EFI_D_ERROR, "Offset 0x64:0x%08x\n", HeciPciRead32(0x64)));
    DEBUG ((EFI_D_ERROR, "Offset 0x68:0x%08x\n", HeciPciRead32(0x68)));
    DEBUG ((EFI_D_ERROR, "Offset 0x6C:0x%08x\n", HeciPciRead32(0x6C)));
    break;
    
  case CHIPSET_SWSMI_TEST_SW_GPE:
    {     
      UINT16  AcpiBaseAddr;
      UINT32  GpeData32;
        
      AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
      DEBUG ((EFI_D_ERROR, "Clear SW GPE Status...\n"));
      IoWrite32(AcpiBaseAddr + R_ACPI_GPE0a_STS, B_ACPI_GPE0a_STS_SWGPE);
      
      DEBUG ((EFI_D_ERROR, "Enable SW GPE...\n"));
      GpeData32 = IoRead32(AcpiBaseAddr + R_ACPI_GPE0a_EN);
      GpeData32 |= B_ACPI_GPE0a_EN_SWGPE_EN;
      IoWrite32(AcpiBaseAddr + R_ACPI_GPE0a_EN, GpeData32);
        
      DEBUG ((EFI_D_ERROR, "Gen SW GPE...\n"));
      GpeData32 = IoRead32(AcpiBaseAddr + R_ACPI_GPE_CNTL);
      GpeData32 |= B_ACPI_GPE_CNTL_SWGPE_CTRL;
      IoWrite32(AcpiBaseAddr + R_ACPI_GPE_CNTL, GpeData32);
      Stall (100);
      GpeData32 &= ~B_ACPI_GPE_CNTL_SWGPE_CTRL;
      IoWrite32(AcpiBaseAddr + R_ACPI_GPE_CNTL, GpeData32);
    }
    break;
//[-start-170315-IB07400848-add]//
//[-start-170329-IB07400853-modify]//
  case CHIPSET_SWSMI_SMM_VARIABLE_TEST:  
    {
      EFI_SMM_VARIABLE_PROTOCOL   *SmmVariable;
      UINTN                       VariableNameSize;
      CHAR16                      *VariableName;
      EFI_GUID                    VendorGuid;
      UINTN                       VariableNameBufferSize;
      UINTN                       VariableSize;
      UINT8                       *VariableData;
      UINTN                       VariableIndex;
      UINT8                       FullSystemConfig[FixedPcdGet32(PcdSetupConfigSize)];
      
      DEBUG ((EFI_D_ERROR, "SMM Variable Test\n"));
      
      Status = gSmst->SmmLocateProtocol (
        &gEfiSmmVariableProtocolGuid,
        NULL,
        (VOID **)&SmmVariable
        );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Get SMM Variable %r!!\n", Status));
        break;
      }

      //
      // SMM Get Next Variable Test
      //
      DEBUG ((EFI_D_ERROR, "SMM Get Next Variable Test!!\n"));
      
      VariableNameSize = sizeof (CHAR16);
      VariableName = AllocateZeroPool (VariableNameSize);
      VariableNameBufferSize = VariableNameSize;
      
      while (TRUE) {
        Status = SmmVariable->SmmGetNextVariableName (
          &VariableNameSize,
          VariableName,
          &VendorGuid
          );

        if (Status == EFI_BUFFER_TOO_SMALL) {
          VariableName = ReallocatePool (VariableNameBufferSize, VariableNameSize, VariableName);
          VariableNameBufferSize = VariableNameSize;
          //
          // Try again using the new buffer.
          //
          Status = SmmVariable->SmmGetNextVariableName (
            &VariableNameSize,
            VariableName,
            &VendorGuid
            );
        }

        if (EFI_ERROR (Status)) {
          //
          // No more variable available, finish search.
          //
          if (Status != EFI_NOT_FOUND) {
            DEBUG ((EFI_D_ERROR, "GetNextVariableName %r\n", Status));
          }
          break;
        }

        //
        // Dump Variable Detail
        //
        DEBUG ((EFI_D_ERROR, "VariableName = %s, Guid = %g\n", VariableName, &VendorGuid));
      }

      FreePool (VariableName);
      
      //
      // SMM Get Variable Test (Setup for example)
      //
      DEBUG ((EFI_D_ERROR, "SMM Get Variable Test!!\n"));
      
      VariableSize = sizeof (CHIPSET_CONFIGURATION);
      Status = SmmVariable->SmmGetVariable(
        SETUP_VARIABLE_NAME,
        &gSystemConfigurationGuid,
        NULL,
        &VariableSize,
        FullSystemConfig
        );
      
      DEBUG ((EFI_D_ERROR, "SMM Setup Variable %r!!\n", Status));
      
      //
      // SMM Set Variable Test
      //
      DEBUG ((EFI_D_ERROR, "SMM Set Variable Test!!\n"));
      
      VariableSize = sizeof (CHIPSET_CONFIGURATION);
      Status = SmmVariable->SmmSetVariable(
        L"Setup2",
        &gSystemConfigurationGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        VariableSize,
        FullSystemConfig
        );
      DEBUG ((EFI_D_ERROR, "SMM Set Variable %r!!\n", Status));
      
      
      //
      // SMM Variable Reclaim Test
      //
      DEBUG ((EFI_D_ERROR, "SMM Variable Reclaim Test!!\n"));
      VariableSize = sizeof (CHIPSET_CONFIGURATION);
      VariableData = AllocateZeroPool (VariableSize);
      
      for (VariableIndex = 0; VariableIndex < 1000; VariableIndex++) {
        SetMem (VariableData, VariableSize, (UINT8)VariableIndex);
        Status = SmmVariable->SmmSetVariable (
          L"SmmVarRecliamTest",
          &gEfiSmmVariableProtocolGuid,
          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
          VariableSize,
          VariableData
          );
        DEBUG ((EFI_D_ERROR, "SmmVarRecliamTest:%05d, Status = %r\n", VariableIndex, Status));
        if (EFI_ERROR (Status)) {
          break;
        }
      }
      
      FreePool (VariableData);
      DEBUG ((EFI_D_ERROR, "SMM Variable Reclaim %r!!\n", Status));
    }
    break;
//[-end-170329-IB07400853-modify]//
//[-end-170315-IB07400848-add]//
#endif    
//[-start-161117-IB07400815-add]//
  case CHIPSET_RUNTIME_AND_ASL_POST_CODE:
    RtPostCodeData = ReadExtCmos32 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosRtPostCodeByte0); 
    CHIPSET_POST_CODE (RtPostCodeData);
    break;
//[-end-161117-IB07400815-add]//
  case 0x00:
  default:
    break;
  }
//[-start-170314-IB07400847-add]//
  //
  // Clear Function Number
  //
  Function = 0x00;
  Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, 0xB3, 1, &Function);
//[-end-170314-IB07400847-add]//
  return EFI_SUCCESS;
}

//[-start-160923-IB07400789-add]//
EFI_STATUS
EFIAPI
ChipsetfeatureEnableAcpiCallback (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  UINT8   CmosData;
  UINT16  AcpiBase;
  UINT32  RegData32;
  
  DEBUG ((EFI_D_INFO, "ChipsetfeatureEnableAcpiCallback\n"));
  
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature); 
  if (CmosData & B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT) { // Win7 Virtual KBC support
  
    DEBUG ((EFI_D_INFO, "Win7 Virtual KBC support!!\n"));
    AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

    //
    // Enable LEGACY USB3 SMI for Win7 Virtual KBC
    //
    RegData32 = IoRead32(AcpiBase + R_SMI_EN);
    RegData32 |= B_SMI_EN_LEGACY_USB3;
    IoWrite32(AcpiBase + R_SMI_EN, RegData32);

  } else {
    PciWrite32(PCI_LIB_ADDRESS (LPC_BUS, LPC_DEV, LPC_FUN, R_LUKMC), 0x0); // Disable Legacy Control
  }
  
//[-start-170626-IB07400880-add]//
  if (CmosData & B_CMOS_S5_WAKE_ON_USB) { // S5 Wake On USB

    DEBUG ((EFI_D_INFO, "S5 Wake On USB support!!\n"));
    AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

    //
    // Enable LEGACY USB3 SMI for S5 Wake On USB
    //
    RegData32 = IoRead32(AcpiBase + R_SMI_EN);
    RegData32 |= B_SMI_EN_LEGACY_USB3;
    IoWrite32(AcpiBase + R_SMI_EN, RegData32);
  }
//[-end-170626-IB07400880-add]//

  // Reset Keyboard buffer pointer
  BDA_KBBTAIL = BDA_KBBHEAD;
  
  // Enable Mouse for Win7 Legacy Free support
  EBDA(EBDA_POINTER_DEVICE_FLAG) &= 0;

  return EFI_SUCCESS;
}
//[-end-160923-IB07400789-add]//

/**
 Initializes the Chipset Feature SMM Driver

 @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
InitializeChipsetFeatureSmm (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_HANDLE                                Handle;
  EFI_SMM_SW_DISPATCH2_PROTOCOL             *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT               SwContext2;
//[-start-160923-IB07400789-add]//
  VOID                                      *AcpiEnableAcpiEvent;
//[-end-160923-IB07400789-add]//
  
  Handle = NULL;

  //
  //  Get the Sw dispatch protocol
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid,
                                     NULL,
                                     (VOID **) &SwDispatch
                                     );
  ASSERT_EFI_ERROR(Status);

  SwContext2.SwSmiInputValue = CHIPSET_FEATURE_SW_SMI;
  Status = SwDispatch->Register (
                          SwDispatch,
                          ChipsetFeatureSwSmiCallBack,
                          &SwContext2,
                          &Handle
                          );

  ASSERT_EFI_ERROR (Status);
  
//[-start-160923-IB07400789-add]//
  AcpiEnableAcpiEvent = NULL;
  Status = gSmst->SmmRegisterProtocolNotify (
              &gAcpiEnableCallbackStartProtocolGuid,
              ChipsetfeatureEnableAcpiCallback,
              &AcpiEnableAcpiEvent
              );  
  ASSERT_EFI_ERROR (Status);
//[-end-160923-IB07400789-add]//

  return EFI_SUCCESS;
}

