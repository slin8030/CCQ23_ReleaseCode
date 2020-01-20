/** @file
 This is an implementation of the Whea Support protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2007 - 2016 Intel Corporation.

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

#include "WheaSupport.h"
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-160111-IB03090424-modify]//
#include <ChipsetSetupConfig.h>
//[-end-160111-IB03090424-modify]//
//[-start-160817-IB03090432-modify]//
#include <ScRegs/RegsPmc.h>
//[-end-160817-IB03090432-modify]//
#include <Library/IoLib.h>
//[-start-160803-IB07220122-add]//
#include <Library/UefiRuntimeServicesTableLib.h>
//[-end-160803-IB07220122-add]//
#include <Library/DxeInsydeChipsetLib.h>

#define SSRAM_PMC_START   0x680
#define SSRAM_PMC_END     0x8ff
#define SSRAM_PMC_SIZE    (SSRAM_PMC_END - SSRAM_PMC_START + 1)
#define SSRAM_CPU_START   0x900
#define SSRAM_CPU_END     0x13FF
#define SSRAM_CPU_SIZE    (SSRAM_CPU_END - SSRAM_CPU_START + 1)
#define SSRAM_PUNIT_START 0x4
#define SSRAM_PUNIT_END   0x67F
#define SSRAM_PUNIT_SIZE    (SSRAM_PUNIT_END - SSRAM_PUNIT_START + 1)
#define FIRMWARE_ERROR_RECORD_OFFSET 0x10
//[-start-160513-IB03090427-add]//
#define PMCPUNITSIZE      (0xFFF - SSRAM_PUNIT_START + 1)
//[-end-160513-IB03090427-add]//

#define GLOBAL_RESET 0X2
#define PMC_RESET_RECORD_CRASHLOG_REASON            0x708
#define PMC_RESET_RECORD_LAST_PLATFORM_EVENT_CAUSE  0x724
#define PUNIT_ERROR_RECORD_CRASHLOG_REASON          0x28C
#define CPU_GLB_CORE0_VALID                         0x934
#define CPU_GLB_CORE1_VALID                         0x938
#define CPU_GLB_CORE2_VALID                         0x93C
#define CPU_GLB_CORE3_VALID                         0x940
//
// WHEA Support driver instance data structure
//
STATIC
WHEA_SUPPORT_INST mWheaInst =
{
  EFI_WHEA_SUPPORT_INST_SIGNATURE,
  NULL,
  0,
  0,
  NULL,
  0,
  NULL,
  0,
  NULL,
  FALSE,
  0,
};

STATIC
EFI_ACPI_TABLE_PROTOCOL       *mAcpiTbl = NULL;
STATIC
BOOLEAN                         mReadyToBoot = FALSE;

/**
  Get PMC Crash log from PMC SSRAM

  @param[in] Destination                      Destination address to store the PMC crash log.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
GetPMCCrashLog(
    IN UINT8    *Destination
  )
{
  UINT32 PMC_SSRAM_BASE;
  PMC_SSRAM_BASE = PcdGet32(PcdPmcSsramBaseAddress0);
  DEBUG((EFI_D_INFO, "BERT PMC Log Destination = %lx \n ", (UINTN)Destination));
  DEBUG((EFI_D_INFO, "BERT PMC_SSRAM_BASE = %lx \n ", (UINTN)PMC_SSRAM_BASE));
  CopyMem((void*)Destination, (void*)(UINTN)(PMC_SSRAM_BASE + SSRAM_PMC_START), (SSRAM_PMC_END-SSRAM_PMC_START + 1));
  return EFI_SUCCESS;
}

/**
  Get Punit Crash log from PMC SSRAM

  @param[in] Destination                      Destination address to store the PMC crash log.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
GetPunitCrashLog(
    IN UINT8    *Destination
  )
{
  UINT32 PMC_SSRAM_BASE;
  PMC_SSRAM_BASE = PcdGet32(PcdPmcSsramBaseAddress0);
  DEBUG((EFI_D_INFO, "BERT Punit Log Destination = %lx \n ", (UINTN)Destination));
  DEBUG((EFI_D_INFO, "BERT PMC_SSRAM_BASE = %lx \n ", (UINTN)PMC_SSRAM_BASE));
  CopyMem((void*)Destination, (void*)(UINTN)(PMC_SSRAM_BASE + SSRAM_PUNIT_START), (SSRAM_PUNIT_END-SSRAM_PUNIT_START + 1));
  return EFI_SUCCESS;
}

/**
  Get CPU Crash log from PMC SSRAM

  @param[in] Destination                      Destination address to store the CPU crash log.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
GetCpuCrashLog(
    IN UINT8    *Destination
  )
{
  UINT32 PMC_SSRAM_BASE;
  PMC_SSRAM_BASE = PcdGet32(PcdPmcSsramBaseAddress0);
  DEBUG((EFI_D_INFO, "BERT CPU Log Destination = %lx \n ", (UINTN)Destination));
  DEBUG((EFI_D_INFO, "BERT PMC_SSRAM_BASE = %lx \n ", (UINTN)PMC_SSRAM_BASE));
  CopyMem((void*)Destination, (void*)(UINTN)(PMC_SSRAM_BASE + SSRAM_CPU_START), (SSRAM_CPU_END-SSRAM_CPU_START + 1));
  return EFI_SUCCESS;
}

/**
  Clear PMC and PUNIT SSRAM region
**/
VOID
ClearPmcPunitSSRAMRegion (
    VOID
  )
{
  UINT32 PMC_SSRAM_BASE;
  PMC_SSRAM_BASE = PcdGet32(PcdPmcSsramBaseAddress0);
  DEBUG((EFI_D_INFO, "BERT PMC_SSRAM_BASE = %lx \n ", (UINTN)PMC_SSRAM_BASE));
  DEBUG((EFI_D_INFO, "Clear PMC PUNIT Region \n"));
  gBS->SetMem ((void*)(UINTN)(PMC_SSRAM_BASE + SSRAM_PUNIT_START), PMCPUNITSIZE, 0);

}

/**
    Add a new Firmware Crashlog Entry to existing Error Status Block.

    @param[in] ErrStsBlk                          BERT GENERIC_ERROR_STATUS instance.
    @param[in out] FirmwareGenericErrorAddr       Pointer to this FirmwareGenericError entry address, updated on return.
    @param[in out] FirmwareCrashlogPayloadAddr    Pointer to Firmware Crashlog Entry payload address, updated on return.
    @param[in] EntrySize                          Firmware Crashlog Entry payload size.
    @param[in] EntrySource                        Firmware Crashlog Entry source.
    @param[in] EntryVersion                       Firmware Crashlog Entry version.
**/
void
WheaAddFirmwareCrashlogEntry(
        IN GENERIC_ERROR_STATUS    *ErrStsBlk,
        IN OUT FIRMWARE_GENERIC_ERROR **FirmwareGenericErrorAddr,
        IN OUT UINT8 **FirmwareCrashlogPayloadAddr,
        IN UINT32 EntrySize,
        IN UINT32 EntrySource,
        IN UINT32 EntryVersion
    )
{
    FIRMWARE_GENERIC_ERROR *mFirmwareGenericError = NULL;

    mFirmwareGenericError = (FIRMWARE_GENERIC_ERROR*)((UINT8*)(void*)ErrStsBlk + ErrStsBlk->ErrDataSize + sizeof(GENERIC_ERROR_STATUS));
    ZeroMem(mFirmwareGenericError, sizeof(FIRMWARE_GENERIC_ERROR));

    CopyMem(&mFirmwareGenericError->GenericErrorDataEntry.SectionType, &gEfiWheaFirmwareErrorSectionGuid, sizeof(EFI_GUID));
    mFirmwareGenericError->GenericErrorDataEntry.Severity = GENERIC_ERROR_FATAL;
    mFirmwareGenericError->GenericErrorDataEntry.Revision = GENERIC_ERROR_SECTION_REVISION;
    mFirmwareGenericError->GenericErrorDataEntry.SecValidMask.FruIdValid = 0;
    mFirmwareGenericError->GenericErrorDataEntry.SecValidMask.FruStringValid = 0;
    mFirmwareGenericError->GenericErrorDataEntry.SecValidMask.Resv1 = 0;
    mFirmwareGenericError->GenericErrorDataEntry.DataSize = EntrySize + sizeof(FIRMWARE_ERROR_RECORD);

    mFirmwareGenericError->FirmwareErrorRecord.FirmwareErrorRecordType = 1; //FW Crashlog Error Record
    mFirmwareGenericError->FirmwareErrorRecord.RecordIdentifier.FirmwareCrashlogSource = EntrySource;
    mFirmwareGenericError->FirmwareErrorRecord.RecordIdentifier.FirmwareCrashlogVersion = EntryVersion;

    ErrStsBlk->ErrDataSize += mFirmwareGenericError->GenericErrorDataEntry.DataSize + sizeof(GENERIC_ERROR_DATA_ENTRY);
    ErrStsBlk->BlockStatus.NumErrorDataEntry++;

    *FirmwareGenericErrorAddr = mFirmwareGenericError;
    *FirmwareCrashlogPayloadAddr = (UINT8*)(void*)mFirmwareGenericError + sizeof(FIRMWARE_GENERIC_ERROR);
}

/**
  Get PMC crash log and Punit crash log, and initialize the APEI BERT GENERIC_ERROR_STATUS structure.

  @param[in] ErrStsBlk                      BERT GENERIC_ERROR_STATUS instance.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
WheaGenElogFirmware(
    IN GENERIC_ERROR_STATUS    *ErrStsBlk
  )
{
  UINT32 PMC_SSRAM_BASE = 0;
  UINT32 PMCFwVersion = 0;
  UINT32 PunitFwVersion = 0;
  UINT32 CPUFwVersion = 0;
  UINT8 *PMCDest;
  UINT8 *PunitDest;
  UINT8 *CPUDest;
  FIRMWARE_GENERIC_ERROR       *PMCFirmwareGenericError = NULL;
  FIRMWARE_GENERIC_ERROR       *PunitFirmwareGenericError = NULL;
  FIRMWARE_GENERIC_ERROR       *CpuFirmwareGenericError = NULL;

  if (ErrStsBlk == NULL) {
    return EFI_NOT_FOUND;
  }

  PMC_SSRAM_BASE = PcdGet32(PcdPmcSsramBaseAddress0);
  PMCFwVersion = *(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + SSRAM_PMC_START);
  PunitFwVersion = *(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + SSRAM_PUNIT_START);
  CPUFwVersion = *(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + SSRAM_CPU_START);

    ErrStsBlk->RawDataOffset = 0;
    ErrStsBlk->RawDataSize = 0;
    ErrStsBlk->Severity = GENERIC_ERROR_FATAL;
    ErrStsBlk->BlockStatus.NumErrorDataEntry = 0;
    ErrStsBlk->ErrDataSize = 0;

    WheaAddFirmwareCrashlogEntry(ErrStsBlk, &PMCFirmwareGenericError, &PMCDest, SSRAM_PMC_SIZE, 0, PMCFwVersion);
    GetPMCCrashLog(PMCDest);

    WheaAddFirmwareCrashlogEntry(ErrStsBlk, &PunitFirmwareGenericError, &PunitDest, SSRAM_PUNIT_SIZE, 1, PunitFwVersion);
    GetPunitCrashLog(PunitDest);

    WheaAddFirmwareCrashlogEntry(ErrStsBlk, &CpuFirmwareGenericError, &CPUDest, SSRAM_CPU_SIZE, 2, CPUFwVersion);
    GetCpuCrashLog(CPUDest);

    ErrStsBlk->BlockStatus.MultipleUeValid = 1;
    ErrStsBlk->BlockStatus.UeValid = 1;

    mWheaInst.BootErrorRegionLen = ErrStsBlk->ErrDataSize + sizeof(GENERIC_ERROR_STATUS);
    mWheaInst.Bert->BootErrorRegionLen = (UINT32)mWheaInst.BootErrorRegionLen;

    DEBUG((EFI_D_INFO, " mWheaInst.ErrorStatusBlock Size = 0x%x\n",  mWheaInst.BootErrorRegionLen));
    DEBUG((EFI_D_INFO, " PunitFirmwareGenericError->GenericErrorDataEntry.DataSize = 0x%x\n", PunitFirmwareGenericError->GenericErrorDataEntry.DataSize));
    DEBUG((EFI_D_INFO, " PMCFirmwareGenericError->GenericErrorDataEntry.DataSize = 0x%x\n", PMCFirmwareGenericError->GenericErrorDataEntry.DataSize));
    DEBUG((EFI_D_INFO, " CPUFirmwareGenericError->GenericErrorDataEntry.DataSize = 0x%x\n", CpuFirmwareGenericError->GenericErrorDataEntry.DataSize));
    DEBUG((EFI_D_INFO, " ErrStsBlk->ErrDataSize = 0x%x\n", ErrStsBlk->ErrDataSize));
    DEBUG((EFI_D_INFO, " PunitCrashLogSize = 0x%x\n", SSRAM_PUNIT_SIZE));
    DEBUG((EFI_D_INFO, " PMCCrashLogSize = 0x%x\n", SSRAM_PMC_SIZE));
    DEBUG((EFI_D_INFO, " CPUCrashLogSize = 0x%x\n", SSRAM_CPU_SIZE));

    //
    // Cear the SSRAM region after copying the error log
    //
    ClearPmcPunitSSRAMRegion ();
    return EFI_SUCCESS;
}

/**
  Log firmware boot error log  in APEI BERT.

  @param[in] BootErrorTable                      APEI BERT address.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
WheaBootElogFirmware(
  IN EFI_ACPI_WHEA_BOOT_ERROR_TABLE   *BootErrorTable
  )
{
  GENERIC_ERROR_STATUS        *ErrStsBlk;
  if(BootErrorTable == NULL) return EFI_NOT_FOUND;

  ErrStsBlk = (GENERIC_ERROR_STATUS *)(UINTN)BootErrorTable->BootErrorRegion;
  if (ErrStsBlk->ErrDataSize >= BootErrorTable->BootErrorRegionLen)
    return EFI_OUT_OF_RESOURCES;

  return WheaGenElogFirmware (ErrStsBlk);
}

/**
  Install APEI BERT.

  @param[in] WheaInst                      APEI BERT Instance.

  @retval EFI_SUCCESS             The function completes successfully
**/
STATIC
EFI_STATUS
InstallWheaTables (
  WHEA_SUPPORT_INST                 *WheaInst
  )
{
  EFI_STATUS            Status;
  UINTN                 TableKey;
  if (WheaInst == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (WheaInst->TablesInstalled) {
    return EFI_ALREADY_STARTED;
  }
  if (mAcpiTbl) {
    DEBUG((EFI_D_INFO, "APEI Support. Install BERT Table \n"));
    TableKey = 0;
    Status = mAcpiTbl->InstallAcpiTable (mAcpiTbl, WheaInst->Bert, WheaInst->Bert->Header.Length, &TableKey);
    ASSERT_EFI_ERROR (Status);
    WheaInst->TablesInstalled = TRUE;
    gBS->FreePool(WheaInst->Bert);
  }
  return EFI_SUCCESS;
}

/**
  Check  APEI BERT table installation.

  @param[in] Event                      EFI Event.
  @param[in] Context                   The context that needs to be checked.

**/
VOID
EFIAPI
CheckInstallTables (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                    Status;
  mReadyToBoot = TRUE;
  Status = InstallWheaTables((WHEA_SUPPORT_INST *)Context);
  if (EFI_ERROR(Status) && (Status != EFI_ALREADY_STARTED)) {
    ASSERT_EFI_ERROR (Status);
  }
}

/**
  Check  ACPI Support.

  @param[in] Event                      EFI Event.
  @param[in] Context                   The context that needs to be checked.

**/
VOID
EFIAPI
CheckAcpiSupport (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                    Status;
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, &mAcpiTbl);
  ASSERT_EFI_ERROR (Status);
  if (mReadyToBoot) {
    Status = InstallWheaTables((WHEA_SUPPORT_INST *)Context);
    if (EFI_ERROR(Status) && (Status != EFI_ALREADY_STARTED)) {
      ASSERT_EFI_ERROR (Status);
    }
  }
}

/**
  Check  Create Bert Table or not.

  @param[in] VOID.

**/

BOOLEAN
CreateBertTable (
VOID
)
{
  UINT16                          AcpiBase;
  UINT32                          PmcTcoSts;
  UINT32                          PMC_SSRAM_BASE               = 0;
  UINTN                           PMCFwVersion                 = 0;
  UINTN                           PunitFwVersion               = 0;
  UINTN                           CpuFwVersion                 = 0;
  UINT32                          PmcResetRecordCrashLogReason = 0;
  BOOLEAN                         PmcError                     = FALSE;
  BOOLEAN                         PunitError                   = FALSE;
  BOOLEAN                         CpuError                     = FALSE;

  AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  PmcTcoSts = IoRead32(AcpiBase + R_TCO_STS);
  PMC_SSRAM_BASE = PcdGet32(PcdPmcSsramBaseAddress0);
  PMCFwVersion = *(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + SSRAM_PMC_START);
  PunitFwVersion = *(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + SSRAM_PUNIT_START);
  CpuFwVersion = *(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + SSRAM_CPU_START);
  PmcResetRecordCrashLogReason = (*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + PMC_RESET_RECORD_CRASHLOG_REASON));

  if ((PMCFwVersion != 0)&&
    (((PmcResetRecordCrashLogReason & 0x00000002) == 0x2)||
    (((*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + PMC_RESET_RECORD_LAST_PLATFORM_EVENT_CAUSE)) & 0x4000) == 0x4000))){
    PmcError = TRUE;
    }

  if ((PunitFwVersion != 0) &&
    ((*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + PUNIT_ERROR_RECORD_CRASHLOG_REASON)) != 0)){
    PunitError = TRUE;
    }

  if ((CpuFwVersion != 0) &&
    (((*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + CPU_GLB_CORE0_VALID)) != 0)||
    ((*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + CPU_GLB_CORE1_VALID)) != 0)||
    ((*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + CPU_GLB_CORE2_VALID)) != 0)||
    ((*(volatile UINT32* )(UINTN)(PMC_SSRAM_BASE + CPU_GLB_CORE3_VALID)) != 0))){
    CpuError = TRUE;
    }

    if (PmcError ||
        PunitError ||
        CpuError ||
        (PmcTcoSts & B_TCO_STS_SECOND_TO)){
    return TRUE;
    }
  else{
    return FALSE;
    }

}
/**
  Entry point of the APEI BERT support driver.

  ImageHandle   EFI_HANDLE: A handle for the image that is initializing this driver
  SystemTable   EFI_SYSTEM_TABLE: A pointer to the EFI system table

  @retval  EFI_SUCCESS:                       Driver initialized successfully
               EFI_LOAD_ERROR:                 Failed to Initialize or has been loaded
               EFI_OUT_OF_RESOURCES:     Could not allocate needed resources
**/
EFI_STATUS
InstallWheaSupport (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                      Status;
  STATIC EFI_EVENT                AcpiEvent;
  UINTN                           VariableSize;
//[-start-160111-IB03090424-modify]//
  CHIPSET_CONFIGURATION           SetupVariables;
//[-end-160111-IB03090424-modify]//
  BOOLEAN CreateBertTableOrNot    = FALSE;

//[-start-160111-IB03090424-modify]//
  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-end-160111-IB03090424-modify]//
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SetupVariables, &VariableSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &VariableSize,
                  &SetupVariables
                  );
//[-end-160803-IB07220122-modify]//
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SetupVariables, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;  
    }
  } 

  if (!SetupVariables.APEIBERT) {
    return EFI_UNSUPPORTED;
  }
  DEBUG((EFI_D_INFO, "BERT init\n"));
  //
  //If the PMC FW log version is zero,  PUNIT FW log version is zero and TCO SECOND_TO_STS is not set,
  //there are no PMC FW error log , PUNIT FW error log and other FW error logs.
  //
  CreateBertTableOrNot = CreateBertTable();
  if(!CreateBertTableOrNot){
    return EFI_UNSUPPORTED;
  }

  mWheaInst.Bert = (EFI_ACPI_WHEA_BOOT_ERROR_TABLE *)AllocateZeroPool (MAX_BERT_SIZE);
  ASSERT (mWheaInst.Bert != NULL);
  mWheaInst.Bert->Header.Signature          = EFI_ACPI_WHEA_BERT_TABLE_SIGNATURE;
  mWheaInst.Bert->Header.Length             = sizeof(EFI_ACPI_WHEA_BOOT_ERROR_TABLE);
  mWheaInst.Bert->Header.Revision           = EFI_WHEA_TABLE_REVISION;
  mWheaInst.Bert->Header.Checksum           = 0;
  UpdateAcpiTableIds(&mWheaInst.Bert->Header);
  //
  // Allocate memory space Error status blocks
  // Allocate memort space for Errorlog Address range
  //
  Status = gBS->AllocatePool (EfiReservedMemoryType, MAX_BOOT_ERROR_LOG_SIZE, &mWheaInst.ErrorStatusBlock);
  ASSERT_EFI_ERROR (Status);
  DEBUG((EFI_D_INFO, "mWheaInst.ErrorStatusBlock = %x\n", (UINTN) mWheaInst.ErrorStatusBlock));

  gBS->SetMem (mWheaInst.ErrorStatusBlock, MAX_BOOT_ERROR_LOG_SIZE,0);
  mWheaInst.BootErrorRegionLen          = MAX_BOOT_ERROR_LOG_SIZE;
  mWheaInst.BootErrorRegion             = (VOID *)((UINT8 *)mWheaInst.ErrorStatusBlock);
  mWheaInst.Bert->BootErrorRegionLen    = (UINT32)mWheaInst.BootErrorRegionLen;
  mWheaInst.Bert->BootErrorRegion       = (UINT64) (UINTN)mWheaInst.BootErrorRegion;
  Status = WheaBootElogFirmware(mWheaInst.Bert);
  if(EFI_ERROR(Status)){
    DEBUG((EFI_D_INFO, "No PMC and Punit error log \n"));
    }
  //
  //Register the ready to boot event.
  //
  mAcpiTbl = NULL;
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, &mAcpiTbl);

  Status = EfiCreateEventReadyToBootEx (
            TPL_NOTIFY,
            CheckInstallTables,
            &mWheaInst,
            &AcpiEvent
            );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
