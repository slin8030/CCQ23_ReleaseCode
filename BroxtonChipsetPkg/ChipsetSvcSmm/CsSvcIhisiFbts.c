/** @file
 Implement the Chipset Servcie IHISI FBTS subfunction for this driver.

***************************************************************************
* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <Uefi.h>
#include <ScAccess.h>
#include <CsSvcIhisiFbts.h>
#include <ScAccess.h>
#include <ChipsetCmos.h>
#include <ChipsetSetupConfig.h>
#include <Library/IoLib.h>
#include <Library/CmosLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SpiAccessLib.h>
#include <Library/SmmOemSvcChipsetLib.h>
#include <Library/BaseOemSvcKernelLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/SmmVariable.h>
//[-start-160701-IB06740505-add]//
#include <Library/BpdtLib.h>
//[-end-160701-IB06740505-add]//
#include <Library/DxeInsydeChipsetLib.h>
//[-start-160824-IB07220130-add]//
#include <SecureFlash.h>
#include <Library/VariableLib.h>
//[-end-160824-IB07220130-add]//
//[-start-161012-IB10860211-add]//
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/AtaPassThru.h>
//[-end-161012-IB10860211-add]//
//[-start-160804-IB07400769-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160804-IB07400769-add]//
//[-start-160827-IB07400773-add]//
#include <Library/HeciMsgLib.h>
//[-end-160827-IB07400773-add]//
//[-start-161026-IB07400805-add]//
#include <Protocol/GlobalNvsArea.h>
//[-end-161026-IB07400805-add]//

BOOLEAN                                     mFlashME;
UINT32                                      SmiEnSave;
extern UINT32                               mIhisiFlash;
extern EFI_GLOBAL_NVS_AREA                 *mGlobalNvsArea;
extern EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL   *mSmmFwBlockService;
//[-start-161026-IB07400805-add]//
extern EFI_GLOBAL_NVS_AREA                 *mGlobalNvsArea; 
//[-end-161026-IB07400805-add]//

//[-start-151124-IB10860189-modify]//
FBTS_PLATFORM_ROM_MAP mOemRomMap[] = {   
//  {FbtsRomMapDxe,          FixedPcdGet32 (PcdFlashFvMainBase),              FixedPcdGet32 (PcdFlashFvMainSize)                 },
//  {FbtsRomMapNVRam,        0,                                               FixedPcdGet32 (PcdFlashNvStorageVariableSize)      },
//  {FbtsRomMapCpuMicrocode, FixedPcdGet32 (PcdFlashNvStorageMicrocodeBase),  FixedPcdGet32 (PcdFlashNvStorageMicrocodeSize)},
//  {FbtsRomMapFtwState,     0, FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize)},
//  {FbtsRomMapFtwBackup,    0,   FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize)},
//  {FbtsRomMapPei,          FixedPcdGet32 (PcdFlashFvRecoveryBase),          FixedPcdGet32 (PcdFlashFvRecoverySize)},
//  {FbtsRomMapPei,          FixedPcdGet32 (PcdFlashFvRecovery2Base),         FixedPcdGet32 (PcdFlashFvRecovery2Size)},
  {FbtsRomMapEos,          0,          0      }
};
//[-end-151124-IB10860189-modify]//

FLASH_BASE_MAP_TABLE                    mFlashBaseMapTable[] = {{FLASH_SIZE_1024K, ROM_1M_BASE },
                                                                {FLASH_SIZE_2048K, ROM_2M_BASE },
                                                                {FLASH_SIZE_4096K, ROM_4M_BASE },
                                                                {FLASH_SIZE_8192K, ROM_8M_BASE },
                                                                {FLASH_SIZE_16384K, ROM_16M_BASE }
                                                               };

//
// Add OEM private rom map table,
//
//[-start-160801-IB11270158-modify]//
//[-start-160616-IB10860201-modify]//
//[-start-160415-IB07220065-modify]//
//[-start-160310-IB07220049-add]//
//[-start-151124-IB10860189-modify]//
//[-start-160816-IB07400771-modify]//
//
// Do not protect TXE device exapnsion region, determined by Custemer/OEM.
//
FBTS_PLATFORM_PRIVATE_ROM mOemPrivateRomMap[] = {
  {0xFFFFF000, 0x1000},  //Device Expansion
  {FbtsRomMapEos,  0}
};
//[-end-160816-IB07400771-modify]//
//[-end-151124-IB10860189-modify]//
//[-end-160310-IB07220049-add]//
//[-end-160415-IB07220065-modify]//
//[-end-160616-IB10860201-modify]//
//[-end-160801-IB11270158-modify]//



//[-start-161012-IB10860211-add]//
EFI_STATUS
SendFlushCacheCommand (
  IN  EFI_ATA_PASS_THRU_PROTOCOL       *AtaPassThruPtr,
  IN  UINT16                           PortNumber,
  IN  UINT16                           PortMulNumber
  )
{
  EFI_STATUS                       Status;
  EFI_ATA_PASS_THRU_COMMAND_PACKET AtaPassThruCmdPacket = {0};
  EFI_ATA_STATUS_BLOCK             Asb                  = {0};
  EFI_ATA_COMMAND_BLOCK            Acb                  = {0};


  AtaPassThruCmdPacket.Asb                = &Asb;
  AtaPassThruCmdPacket.Acb                = &Acb;
  AtaPassThruCmdPacket.Acb->AtaDeviceHead = 0xE0;

  //
  // Set Non data command FLUSH_CACHE_EXT_CMD
  //
  AtaPassThruCmdPacket.Acb->AtaCommand = 0xEA;
  AtaPassThruCmdPacket.Protocol        = EFI_ATA_PASS_THRU_PROTOCOL_ATA_NON_DATA;
  AtaPassThruCmdPacket.Length          = EFI_ATA_PASS_THRU_LENGTH_NO_DATA_TRANSFER;

  //
  // Command timeout setting
  //
//   AtaPassThruCmdPacket.Timeout         = H19_ATA_COMMAND_TIMEOUT;

  if (AtaPassThruPtr != NULL) {
    Status = AtaPassThruPtr->PassThru (
                               AtaPassThruPtr,
                               PortNumber,
                               PortMulNumber,
                               &AtaPassThruCmdPacket,
                               0
                               );
    if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FlushHddCache (
  VOID
  )
{
  EFI_STATUS                       Status;
  UINT16                           PortNumber;
  UINT16                           PortMulNumber;
  UINTN                            HandleIndex;
  UINTN                            HandleCount;
  EFI_HANDLE                       *HandleBuffer;
  EFI_ATA_PASS_THRU_PROTOCOL       *AtaPassThruPtr;

  PortNumber        = 0xFFFF;
  PortMulNumber     = 0xFFFF;
  HandleBuffer      = NULL;
  AtaPassThruPtr    = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiAtaPassThruProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status) || (HandleCount == 0)) {
    return EFI_NOT_FOUND;
  }

  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    Status = gBS->HandleProtocol(HandleBuffer[HandleIndex], &gEfiAtaPassThruProtocolGuid, &AtaPassThruPtr);
    if (EFI_ERROR (Status)) {
      continue;
    }

    while (TRUE) {
      Status = AtaPassThruPtr->GetNextPort (AtaPassThruPtr, &PortNumber);
      if (EFI_ERROR (Status)) {
        break;
      }

      while (TRUE) {
        Status = AtaPassThruPtr->GetNextDevice (AtaPassThruPtr, PortNumber, &PortMulNumber);
        if (EFI_ERROR (Status)) {
          break;
        }

        //
        // Send ATA HDD Fluch cache command
        //
        Status = SendFlushCacheCommand (AtaPassThruPtr, PortNumber, PortMulNumber);
        if (Status == EFI_UNSUPPORTED) {
          break;
        }
      }
    }
  }

  return Status;
}
//[-end-161012-IB10860211-add]//

//[-start-160809-IB07400769-add]//
//[-start-170214-IB07400840-remove]//
//#define TXE_DEVICE_EXPANSION_BASE   0x006FF000
//#define TXE_DEVICE_EXPANSION_SIZE   0x00100000
//[-end-170214-IB07400840-remove]//

BOOLEAN mTxeDataRegionUnavailable = FALSE;

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

//[-start-180810-IB07400997-modify]//
  OriginalTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180810-IB07400997-modify]//

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
//[-start-180810-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180810-IB07400997-modify]//
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
//[-end-160809-IB07400769-add]//

/**
  Setup wakeup timer to automatically wakeup system

**/
VOID
SetRtcWakeup ()
{

  UINT8                   Data8;
  UINT8                   Index;
  UINT16                  Data16;
  UINT16                  AcpiBaseAddr;

  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  Index = (R_RTC_REGISTERD | B_PCH_NMI_EN_NMI_EN);
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_INDEX, 1, &Index);
  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_TARGET, 1, &Data8);
  Data8 &= BIT7;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_TARGET, 1, &Data8);

  Index = (R_RTC_REGISTERB | B_PCH_NMI_EN_NMI_EN);
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_INDEX, 1, &Index);
  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_TARGET, 1, &Data8);
  Data8 |= B_RTC_REGISTERB_AIE;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_TARGET, 1, &Data8);

  Index = (R_RTC_REGISTERB | B_PCH_NMI_EN_NMI_EN);
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_INDEX, 1, &Index);
  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_TARGET, 1, &Data8);
  Data8 &= ~(B_RTC_REGISTERB_SET);
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, R_RTC_TARGET, 1, &Data8);

  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBaseAddr, 1, &Data16);
  Data16 &= (1 << N_ACPI_PM1_STS_RTC);
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBaseAddr, 1, &Data16);

  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT16, (AcpiBaseAddr + R_ACPI_PM1_STS), 1, &Data16);
  Data16 |= (1 << N_ACPI_PM1_EN_RTC);
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT16, (AcpiBaseAddr + R_ACPI_PM1_EN), 1, &Data16);
}

/**
  Let system auto wakeup from S5

**/
STATIC
VOID
S5WakeUpSetting ()
{
  UINT8                        FullSystemConfig[FixedPcdGet32(PcdSetupConfigSize)];
  UINTN                        VarSize;
  EFI_SMM_VARIABLE_PROTOCOL   *SmmVariable;
  EFI_STATUS                   Status;
  CHIPSET_CONFIGURATION        SetupVariable;
  
  if (GetBootModeHob () != BOOT_IN_RECOVERY_MODE) {
    Status = gSmst->SmmLocateProtocol (
                      &gEfiSmmVariableProtocolGuid,
                      NULL,
                      (VOID **)&SmmVariable
                      );
    if (!EFI_ERROR (Status)) {
      VarSize = PcdGet32(PcdSetupConfigSize);
    
      Status = SmmVariable->SmmGetVariable(
                                     SETUP_VARIABLE_NAME,
                                     &gSystemConfigurationGuid,
                                     NULL,
                                     &VarSize,
                                     FullSystemConfig
                                     );
      if (EFI_ERROR (Status)) {
        Status = GetChipsetSetupVariableDxe ((CHIPSET_CONFIGURATION *)FullSystemConfig, sizeof (CHIPSET_CONFIGURATION));  
      }
      
      CopyMem (&SetupVariable, FullSystemConfig, sizeof(CHIPSET_CONFIGURATION));
      
//[-start-170314-IB07400847-modify]//
      if (!EFI_ERROR (Status) && (SetupVariable.WakeOnS5 != SCU_S5_WAKE_ON_RTC_BY_OS_UTILITY)) {
//[-end-170314-IB07400847-modify]//
        return;
      }
      
      SetRtcWakeup ();
    }
  }

  return;
}

/**
  AH=10h(FbtsGetSupportVersion), Get FBTS supported version and FBTS permission.

  @param[in] VersionStr          Flash tool version
  @param[in, out] Permission     Permission
**/
VOID
EFIAPI
IhisiFbtsGetPermission (
  IN  FBTS_TOOLS_VERSION_BUFFER           *VersionPtr,
  IN  OUT UINT16                          *Permission
  )
{
  UINTN             ATpVersion;

  if (VersionPtr == NULL || Permission == NULL) {
    return;
  }
  //
  // Intialial permission to FBTS_PERMISSION_ALLOWED
  //
  *Permission = FBTS_PERMISSION_ALLOWED;
  //
  // Check version signature
  //
  if (VersionPtr->Signature != FBTS_VERSION_SIGNATURE) {
    *Permission = FBTS_PERMISSION_DENY;
  }

  if (PcdGetBool (PcdAntiTheftSupported)) {
    //
    // Since IHISI v1.8.5+ supported flashing AT-p capable BIOS,
    // if the BIOS is AT-p Capable, then we do not allow old version (v1.8.5-)
    // flash tool to flash BIOS. We have AtpVersion field from 1.8.5
    //
    ATpVersion = ((UINTN) VersionPtr->ATpVersion[0] << 16) +
                 ((UINTN) VersionPtr->ATpVersion[1] << 8) +
                  (UINTN) VersionPtr->ATpVersion[2];
    if (ATpVersion < 0x313835) {
      *Permission = FBTS_PERMISSION_DENY;
    }
  }
}

/**
  AH=12h(FbtsGetPlatformRomMap), Get Oem define flash map.

  @param[in, out] RomMapBuffer          Pointer to the returned platform's ROM map protection structure.
                                        After version 1.7.6, InsydeFlash will skip this structure if it found definition in BVDT
  @param[in, out] PrivateRomMapBuffer   Pointer to the returned platform's private map structure.
                                        Flash utility will not flash these areas
                                        (even userenter /all in flashit or all=1 in platform.ini).

  @retval EFI_SUCCESS       Get OEM flash map successful.
  @retval EFI_UNSUPPORTED   FBTS_OEM_ROM_MAP_COUNT is 0 or module rom map buffer is full.
**/
EFI_STATUS
EFIAPI
IhisiFbtsGetOemFlashMap (
  IN OUT   FBTS_PLATFORM_ROM_MAP       **RomMapBuffer,
  IN OUT   FBTS_PLATFORM_PRIVATE_ROM   **PrivateRomMapBuffer
  ) {
  UINTN                         Media_mOemRomMapSize;
  UINTN                         Media_mOemPrivateRomMapSize;
  FBTS_PLATFORM_ROM_MAP         *Media_mOemRomMap;
  FBTS_PLATFORM_PRIVATE_ROM     *Media_mOemPrivateRomMap;
  UINTN                         Index;


  Media_mOemRomMapSize = (sizeof (mOemRomMap) / sizeof (mOemRomMap[0]));
  Media_mOemPrivateRomMapSize = (sizeof (mOemPrivateRomMap) / sizeof (mOemPrivateRomMap[0]));
  
  if (Media_mOemRomMapSize > (UINTN)(sizeof (FBTS_PLATFORM_ROM_MAP_BUFFER) / sizeof (FBTS_PLATFORM_ROM_MAP))) {
    return EFI_UNSUPPORTED;
  }

  if (Media_mOemPrivateRomMapSize > (UINTN)(sizeof (FBTS_PLATFORM_PRIVATE_ROM_BUFFER) / sizeof (FBTS_PLATFORM_PRIVATE_ROM))) {
    return EFI_UNSUPPORTED;
  }

  Media_mOemRomMap = (*RomMapBuffer);
  for (Index = 0; Index < Media_mOemRomMapSize; Index++) {
    Media_mOemRomMap[Index].Type = mOemRomMap[Index].Type;
    Media_mOemRomMap[Index].Address = mOemRomMap[Index].Address;
    Media_mOemRomMap[Index].Length = mOemRomMap[Index].Length;
    if (Media_mOemRomMap[Index].Type == FbtsRomMapNVRam) {
      Media_mOemRomMap[Index].Address = PcdGet32 (PcdFlashNvStorageVariableBase);
    }
    if (Media_mOemRomMap[Index].Type == FbtsRomMapEos) {
      break;
    }
  }
  
  Media_mOemPrivateRomMap = (*PrivateRomMapBuffer);
  for (Index = 0; Index < Media_mOemPrivateRomMapSize; Index++) {
    Media_mOemPrivateRomMap[Index].LinearAddress = mOemPrivateRomMap[Index].LinearAddress;
    Media_mOemPrivateRomMap[Index].Size = mOemPrivateRomMap[Index].Size;
    if (Media_mOemPrivateRomMap[Index].LinearAddress == (UINT32)FbtsRomMapEos) {
      break;
    }
  }

  return EFI_SUCCESS;
}

//[-start-160701-IB06740505-add]//
#define BPDT_GREEN_SIGNATURE    0x000055AA
#define BPDT_YELLOW_SIGNATURE   0x00AA55AA
#define BPDT_RED_SIGNATURE      0xFFFFFFFF

EFI_STATUS
EFIAPI
FotaPrepare (
  VOID
  )
{
  EFI_STATUS                  Status;
  STATIC  BOOLEAN             BackupDoneFlag = FALSE;
  STATIC  BOOLEAN             BackupSkipFlag = FALSE;
  UINTN                       Size;
  UINTN                       Address;
  UINT8                       Index;

  UINT8                       *Bpdt1Buf = NULL;
  BPDT_HEADER                 *Bpdt1Header;
  BPDT_ENTRY                  *Bpdt1Entry;
  BOOLEAN                     FoundIbb;
  UINT32                      SBpdt1Offset;

  UINT8                       *Bpdt2Buf = NULL;
  BPDT_HEADER                 *Bpdt2Header;
  BPDT_ENTRY                  *Bpdt2Entry;
  UINT32                      SBpdt2Offset;
  UINT32                      ObbOffset;
  UINT32                      ObbSize;
  BPDT_HEADER                 *SBpdt2Header;
  BPDT_ENTRY                  *SBpdt2Entry;

  UINT8                       *DirectoryBuf = NULL;
  SUBPART_DIR_HEADER          *DirectoryHeader;
  SUBPART_DIR_ENTRY           *DirectoryEntry;
  UINT64                      *EntryName;


  UINT8                       *CC1Buf = NULL;
  UINTN                       CC1Size;  // LBP1 Critical component area size
  UINTN                       CC1BackupAddress;

  UINT8                           EraseCount;
  UINT8                           WriteCount;

  if (BackupDoneFlag || BackupSkipFlag) {
    return EFI_ALREADY_STARTED;
  }

  Size   = 0x1000;
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    Size,
                    (VOID **)&Bpdt1Buf
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Address = 0x1000;
  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 Address,
                                 0,
                                 &Size,
                                 Bpdt1Buf
                                 );
  if (EFI_ERROR (Status)) {
    goto errorexit;
  }

  Bpdt1Header = (BPDT_HEADER *)Bpdt1Buf;
  Bpdt1Entry = (BPDT_ENTRY *)((UINTN)Bpdt1Buf + sizeof(BPDT_HEADER));

  if (Bpdt1Header->Signature != BPDT_GREEN_SIGNATURE) {
    //
    // BPDT1 is invalid, do not backup CC1 to LBP2.
    // Set BackupSkipFlag = TRUE to skip the function.
    //
    BackupSkipFlag = TRUE;    
    Status = EFI_UNSUPPORTED;
    goto errorexit;
  }

  //
  // Search for IBB exist to determin this is CC1
  // and calculate CC1 size.
  //
  FoundIbb = FALSE;
  SBpdt1Offset = 0;
  
  for (Index = 0; Index < Bpdt1Header->DscCount; Index++, Bpdt1Entry++) {
    if (Bpdt1Entry->Type == BpdtIbb) {
      FoundIbb = TRUE;
    }
    if (Bpdt1Entry->Type == BpdtSbpdt) {
      SBpdt1Offset = Bpdt1Entry->LbpOffset;
    }
  }

  if (FoundIbb == FALSE || SBpdt1Offset == 0) {
    Status = EFI_UNSUPPORTED;
    goto errorexit;
  }

  //
  // Get NvCommon Address from BPDT2
  // Because need to copy CC1 after NvCommon area to remain NvCommon data.
  //
  Size   = 0x1000;
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    Size,
                    (VOID **)&Bpdt2Buf
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

//[-start-170214-IB07400840-modify]//
//  Address = 0x380000;
  Address = PcdGet32(PcdFlashBootPartition2Base);
//[-end-170214-IB07400840-modify]//
  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 Address,
                                 0,
                                 &Size,
                                 Bpdt2Buf
                                 );
  if (EFI_ERROR (Status)) {
    goto errorexit;
  }

  Bpdt2Header = (BPDT_HEADER *)Bpdt2Buf;
  Bpdt2Entry = (BPDT_ENTRY *)((UINTN)Bpdt2Buf + sizeof(BPDT_HEADER));

  if (Bpdt2Header->Signature != BPDT_GREEN_SIGNATURE) {
    BackupSkipFlag = TRUE;    
    Status = EFI_UNSUPPORTED;
    goto errorexit;
  }

  //
  // Find SBPDT2 offset
  //
  SBpdt2Offset = 0;
  for (Index = 0; Index < Bpdt2Header->DscCount; Index++, Bpdt2Entry++) {
    if (Bpdt2Entry->Type == BpdtSbpdt) {
      SBpdt2Offset = Bpdt2Entry->LbpOffset;
    }
  }

  if (SBpdt2Offset == 0) {
    Status = EFI_UNSUPPORTED;
    goto errorexit;
  }

  //
  // Find OBB offset
  //
  SBpdt2Header = (BPDT_HEADER *)((UINTN)Bpdt2Buf + SBpdt2Offset);
  SBpdt2Entry = (BPDT_ENTRY *)((UINTN)SBpdt2Header + sizeof(BPDT_HEADER));

  ObbOffset = 0;
  ObbSize = 0;
  for (Index = 0; Index < SBpdt2Header->DscCount; Index++, SBpdt2Entry++) {
    if (SBpdt2Entry->Type == BpdtObb) {
      ObbOffset = SBpdt2Entry->LbpOffset;
      ObbSize = SBpdt2Entry->Size;
    }
  }

  if (ObbOffset == 0) {
    Status = EFI_UNSUPPORTED;
    goto errorexit;
  }

  //
  // Find OBB offset and size to determine CC1 backup address.
  // Backup CC1 into OBB region
  //
  Size   = 0x1000;
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    Size,
                    (VOID **)&DirectoryBuf
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

//[-start-170214-IB07400840-modify]//
//  Address = 0x380000 + ObbOffset;
  Address = PcdGet32(PcdFlashBootPartition2Base) + ObbOffset;
//[-end-170214-IB07400840-modify]//
  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 Address,
                                 0,
                                 &Size,
                                 DirectoryBuf
                                 );
  if (EFI_ERROR (Status)) {
    goto errorexit;
  }

  DirectoryHeader = (SUBPART_DIR_HEADER *)DirectoryBuf;
  DirectoryEntry = (SUBPART_DIR_ENTRY *)((UINTN)DirectoryBuf + sizeof(SUBPART_DIR_HEADER));

  CC1BackupAddress = 0;
  for (Index = 0; Index < DirectoryHeader->NumOfEntries; Index++, DirectoryEntry++) {
    EntryName = (UINT64 *)DirectoryEntry->EntryName;
    if (*EntryName == SIGNATURE_64('O', 'B', 'B', 0, 0, 0, 0, 0)) {
      CC1BackupAddress = Address + DirectoryEntry->EntryOffset;
    }
  }

  if (CC1BackupAddress == 0) {
    Status = EFI_UNSUPPORTED;
    goto errorexit;
  }

  //
  // Copy CC1 to memory
  //
  CC1Size = SBpdt1Offset - 0x1000;
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    CC1Size,
                    (VOID **)&CC1Buf
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Address = 0x1000;
  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 Address,
                                 0,
                                 &CC1Size,
                                 CC1Buf
                                 );
  if (EFI_ERROR (Status)) {
    goto errorexit;
  }

  Bpdt1Header = (BPDT_HEADER *)CC1Buf;
  Bpdt1Entry = (BPDT_ENTRY *)((UINTN)CC1Buf + sizeof(BPDT_HEADER));

  for (Index = 0; Index < Bpdt1Header->DscCount; Index++, Bpdt1Entry++) {
    //
    // Skip to patch BpdtIfpOverride offset because TXE has hardcode to find this area.
    //
    if (Bpdt1Entry->Type == BpdtIfpOverride) {
      continue;
    }
    if (Bpdt1Entry->Type == BpdtSbpdt) {
      Bpdt1Entry->Type = BpdtCseIdlm;
      Bpdt1Entry->LbpOffset = 0;
      Bpdt1Entry->Size = 0;
      continue;
    }

    if (Bpdt1Entry->Size != 0) {
//[-start-170214-IB07400840-modify]//
//      Bpdt1Entry->LbpOffset += ((UINT32)CC1BackupAddress - 0x380000);
      Bpdt1Entry->LbpOffset += ((UINT32)CC1BackupAddress - PcdGet32(PcdFlashBootPartition2Base));
//[-end-170214-IB07400840-modify]//
    }
  }

  //
  // Add previous OBB entry to new BPDT1 table.
  //
  Bpdt1Header->DscCount += 1;
  Bpdt1Entry->Type = BpdtObb;
  Bpdt1Entry->Flags = 0;
  Bpdt1Entry->LbpOffset = ObbOffset;
  Bpdt1Entry->Size = ObbSize;

  //
  // Override BPDT2 to BPDT1 (CC1Buf 0~0x1000) and set signature to RED (invalid).
  //
  Size = 0x1000;
  Bpdt1Header->Signature = BPDT_RED_SIGNATURE;

  Status = mSmmFwBlockService->EraseBlocks (
                                 mSmmFwBlockService,
//[-start-170214-IB07400840-modify]//
//                                 0x380000,
                                 PcdGet32(PcdFlashBootPartition2Base),
//[-end-170214-IB07400840-modify]//
                                 (UINTN *) &Size
                                 );
  if (!EFI_ERROR (Status)) {
    Status = mSmmFwBlockService->Write (
                                   mSmmFwBlockService,
//[-start-170214-IB07400840-modify]//
//                                   0x380000,
                                   PcdGet32(PcdFlashBootPartition2Base),
//[-end-170214-IB07400840-modify]//
                                   (UINTN *) &Size,
                                   CC1Buf
                                   );

  } else {
    goto errorexit;
  }
  
  //
  // Write CC1 after NvCommon area
  //
  EraseCount      = 0;
  WriteCount      = 0;
  do {
    Status = mSmmFwBlockService->EraseBlocks (
                                   mSmmFwBlockService,
                                   CC1BackupAddress,
                                   (UINTN *) &CC1Size
                                   );
    if (!EFI_ERROR (Status)) {
      EraseCount = 0;
      Status = mSmmFwBlockService->Write (
                                     mSmmFwBlockService,
                                     CC1BackupAddress,
                                     (UINTN *) &CC1Size,
                                     CC1Buf
                                     );
      if (!EFI_ERROR (Status)) {
        break;
      } else {
        WriteCount++;
      }
    } else {
      EraseCount++;
    }
  } while ((EraseCount < 100) && (WriteCount < 100));

  if (EraseCount > 100 || WriteCount > 100) {
    goto errorexit;
  }

  //
  // CC1 data write to OBB complete, set signature to YELLOW (Recovery state).
  //
  Size = 0x1000;
  Bpdt1Header->Signature = BPDT_YELLOW_SIGNATURE;
  Status = mSmmFwBlockService->Write (
                                 mSmmFwBlockService,
//[-start-170214-IB07400840-modify]//
//                                 0x380000,
                                 PcdGet32(PcdFlashBootPartition2Base),
//[-end-170214-IB07400840-modify]//
                                 (UINTN *) &Size,
                                 CC1Buf
                                 );
  if (EFI_ERROR (Status)) {
    goto errorexit;
  }
  BackupDoneFlag = TRUE;

errorexit:
  gSmst->SmmFreePool (Bpdt1Buf);

  if (Bpdt2Buf != NULL) {
    gSmst->SmmFreePool (Bpdt2Buf);
  }
  if (DirectoryBuf != NULL) {
    gSmst->SmmFreePool (DirectoryBuf);
  }
  if (CC1Buf != NULL) {
    gSmst->SmmFreePool (CC1Buf);
  }
  
  return Status;  
}

//[-start-161122-IB07400817-add]//
BOOLEAN              InvalidDone = FALSE;
BOOLEAN              ValidDone = FALSE;
//[-end-161122-IB07400817-add]//
//[-start-180809-IB07400997-add]//
BOOLEAN              mFlashBeWritten = FALSE;
//[-end-180809-IB07400997-add]//

EFI_STATUS
EFIAPI
UpdateBpdt1Signature (
  BOOLEAN  InvalidFlag
  )
{
  EFI_STATUS                  Status = EFI_SUCCESS;
  UINTN                       Size;
  UINTN                       Address;
  UINT32                      Signature;
//[-start-161122-IB07400817-remove]//
//  STATIC BOOLEAN              InvalidDone = FALSE;
//  STATIC BOOLEAN              ValidDone = FALSE;
//[-end-161122-IB07400817-remove]//
  UINT8                       *Buffer;

  Signature = 0;
  if (InvalidFlag && !InvalidDone) {
    //
    // Set BPDT1 to invalid.
    //
    Size = 0x1000;
    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      Size,
                      (VOID **)&Buffer
                      );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  
    Address = 0x1000;
    Status = mSmmFwBlockService->Read (
                                   mSmmFwBlockService,
                                   Address,
                                   0,
                                   &Size,
                                   Buffer
                                   );
    if (EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }
  
    Status = mSmmFwBlockService->EraseBlocks (
                                   mSmmFwBlockService,
                                   Address,
                                   (UINTN *) &Size
                                   );
    if (!EFI_ERROR (Status)) {
      *((UINT32 *)Buffer) = BPDT_RED_SIGNATURE;
      Status = mSmmFwBlockService->Write (
                                     mSmmFwBlockService,
                                     Address,
                                     (UINTN *) &Size,
                                     Buffer
                                     );
  
      if (!EFI_ERROR (Status)) {
        Signature = 0xFFFFFFFF;
      }
    }
  } else if (!InvalidFlag && !ValidDone) {
    //
    // Set BPDT1 to valid.
    //
    Signature = BPDT_GREEN_SIGNATURE;
    Size = 4;

    Status = mSmmFwBlockService->Write (
                                   mSmmFwBlockService,
                                   0x1000,
                                   (UINTN *)&Size,
                                   (UINT8 *)&Signature
                                   );
  }

  if (Signature == 0) {
    return EFI_ALREADY_STARTED;
  }

  if (InvalidFlag) {
    InvalidDone = TRUE;
  } else {
    ValidDone = TRUE;
  }

  return Status;
}
//[-end-160701-IB06740505-add]//

/**
  AH=15h(FBTS write) :  Hook function before Write process

  @param[in, out] WriteDataBufferPtr    Pointer to data buffer for write.
  @param[in, out] WriteSizePtr          Write size.
  @param[in, out] RomBaseAddressPtr     Target linear address to write.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoBeforeWriteProcess (
  IN OUT    UINT8                      *WriteDataBufferPtr,
  IN OUT    UINTN                      *WriteSizePtr,
  IN OUT    UINTN                      *RomBaseAddressPtr
  )
{
  EFI_STATUS                  Status;
  UINT8                       Index;
  UINT16                      PmBase;
  UINT32                      Data32;
  UINTN                       ROMBase;
  UINTN                       ROMOffset;
  UINTN                       WriteSize;
  UINTN                       RomBaseAddress;
  UINT8                       SpiFlashNumber;
  UINT8                       WholeROMSizeIndex;
  FLASH_DEVICE               *Buffer;
  FLASH_REGION               *FlashRegionPtr;
  UINT8                      *OutputDataBuffer;
//[-start-160809-IB07400769-add]//
  UINT8                       CmosData;
  UINT16                      AcpiBaseAddr;
  UINT16                      Pm1Cnt;
  UINT32                      GpeData32;
//[-end-160809-IB07400769-add]//

  WriteSize         = *WriteSizePtr;
  RomBaseAddress    = *RomBaseAddressPtr;

  OutputDataBuffer  = NULL;
  ROMBase           = 0;
  ROMOffset         = 0;
  SpiFlashNumber    = 0;
  WholeROMSizeIndex = 0;
  PmBase            = 0;
  SmiEnSave         = 0;
  Data32            = 0;

  if (*(mSmmFwBlockService->FlashMode) == SMM_FW_DEFAULT_MODE) {
    if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
      OemSvcEcIdle (TRUE);
    }
    
    *(mSmmFwBlockService->FlashMode) = SMM_FW_FLASH_MODE;
  }
  
  if ( !mFlashME ) {
    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      sizeof (FLASH_DEVICE),
                      (VOID **)&Buffer
                      );
    if (!EFI_ERROR(Status)) {
      Status = mSmmFwBlockService->DetectDevice (
                                     mSmmFwBlockService,
                                     (UINT8 *) Buffer
                                     );
    }
    
    if (!EFI_ERROR (Status )) {
      Status = mSmmFwBlockService->GetSpiFlashNumber (
                                     mSmmFwBlockService,
                                     (UINT8 *)&SpiFlashNumber
                                     );
    }
    
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Get ROM Information Failure!\n"));
    }

    if (!EFI_ERROR (Status)) {
      if (Buffer->DeviceInfo.Size == 0xFF) {
        ROMBase = (UINTN)(0x100000000 - (Buffer->DeviceInfo.BlockMap.Mutiple * Buffer->DeviceInfo.BlockMap.BlockSize) * 0x100);
        ROMOffset = RomBaseAddress - ROMBase;
      } else {
        WholeROMSizeIndex = Buffer->DeviceInfo.Size;
        if ( SpiFlashNumber == 2 ) {
          WholeROMSizeIndex = WholeROMSizeIndex + 1;
        }

        for (Index = 0; Index < (sizeof (mFlashBaseMapTable) / sizeof (FLASH_BASE_MAP_TABLE)); Index = Index + 1) {
          if (mFlashBaseMapTable[Index].SizeIndex == WholeROMSizeIndex) {
            ROMBase   = mFlashBaseMapTable[Index].Base;
            ROMOffset = RomBaseAddress - ROMBase;
            break;
          }
        }
      }

      Status = gSmst->SmmAllocatePool (
                        EfiRuntimeServicesData,
                        (sizeof (FLASH_REGION) * TOTAL_FLASH_REGION),
                        (VOID **)&OutputDataBuffer
                        );

      if (!EFI_ERROR (Status)) {
        Status = mSmmFwBlockService->GetFlashTable (
                                       mSmmFwBlockService,
                                       OutputDataBuffer
                                       );
        if (!EFI_ERROR (Status)) {
          FlashRegionPtr = (FLASH_REGION *) OutputDataBuffer;
//[-start-160810-IB11270161-modify]//
          for (Index = APL_DESC_REGION; Index < APL_MAX_FLASH_REGION; Index = Index + 1, FlashRegionPtr++) {
            if (Index == APL_ME_REGION) {
//[-end-160810-IB11270161-modify]//
              if (!(((UINT32) (ROMOffset + WriteSize - 1) < FlashRegionPtr->Offset) ||
                  ((FlashRegionPtr->Offset + FlashRegionPtr->Size - 1) < (UINT32) (ROMOffset)))) {
                mFlashME = TRUE;
                break;
              }
            }
          }
        }
      }
      if (EFI_ERROR( Status )) {
        DEBUG (( EFI_D_ERROR, "Get ROM Map Information Failure!\n"));
      }
    }
  }

  if (mFlashME == TRUE) {
    PmBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
    gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &SmiEnSave);
    Data32 = SmiEnSave & ~B_SMI_EN_GBL_SMI;
    gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &Data32);
  }

//[-start-151124-IB10860189-add]//
  
  if (*(mSmmFwBlockService->FlashMode) == SMM_FW_FLASH_MODE) {
//[-start-170215-IB07400840-modify]//
    if (PcdGet32(PcdFlashSpiRomSize) != 0) {
      *RomBaseAddressPtr = *RomBaseAddressPtr - (UINTN)((UINT64)0x100000000 - PcdGet32(PcdFlashSpiRomSize));
    } else {
      *RomBaseAddressPtr = *RomBaseAddressPtr - ROM_8M_BASE; //Chang MMIO address to SPI address
    }
//[-end-170215-IB07400840-modify]//
  }
//[-end-151124-IB10860189-add]//

//[-start-160809-IB07400769-add]//
  if (*(mSmmFwBlockService->FlashMode) == SMM_FW_FLASH_MODE) {
//[-start-170214-IB07400840-modify]//
    if ((*RomBaseAddressPtr >= PcdGet32(PcdFlashDeviceExpansionBase)) && ((UINT64)*RomBaseAddressPtr < ((UINT64)PcdGet32(PcdFlashDeviceExpansionBase) + (UINT64)PcdGet32(PcdFlashDeviceExpansionSize)))) {
//[-end-170214-IB07400840-modify]//
      if (!mTxeDataRegionUnavailable) {
        DEBUG ((EFI_D_ERROR, "Flash Device Expansion Region, TXE data region Unavailable\n"));
//[-start-160827-IB07400773-add]//
        if (PcdGetBool (PcdSecureFlashSupported)) { // Secure flash enabled (POST), send "IFWI prepare for update" to info TXEs
	        Status = HeciIfwiPrepareForUpdate();
          DEBUG ((EFI_D_ERROR, "HeciIfwiPrepareForUpdate status = %r\n", Status));
          MmioOr32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_ETR, (UINT32) (B_PMC_ETR_CF9GR));
        }
//[-end-160827-IB07400773-add]//
//[-start-161019-IB07400799-add]//
        if (!PcdGetBool (PcdSecureFlashSupported)) { // Secure flash disabled
          //
          // Workaround: Always Gen SW SCI to disable fTPM during update BIOS
          //
          AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
          Pm1Cnt  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_CNT);
          
          if (Pm1Cnt & B_ACPI_PM1_CNT_SCI_EN) { // SCI_EN
            DEBUG ((EFI_D_ERROR, "Clear SW GPE Status...\n"));
            IoWrite32(AcpiBaseAddr + R_ACPI_GPE0a_STS, B_ACPI_GPE0a_STS_SWGPE);
            
            DEBUG ((EFI_D_ERROR, "Enable SW GPE...\n"));
            GpeData32 = IoRead32(AcpiBaseAddr + R_ACPI_GPE0a_EN);
            GpeData32 |= B_ACPI_GPE0a_EN_SWGPE_EN;
            IoWrite32(AcpiBaseAddr + R_ACPI_GPE0a_EN, GpeData32);
            
//[-start-161026-IB07400805-modify]//
            mGlobalNvsArea->SwGpeFun = SW_GPE_FLASH_WORKAROUND;
//[-end-161026-IB07400805-modify]//

            DEBUG ((EFI_D_ERROR, "Gen SW GPE...\n"));
            GpeData32 = IoRead32(AcpiBaseAddr + R_ACPI_GPE_CNTL);
            GpeData32 |= B_ACPI_GPE_CNTL_SWGPE_CTRL;
            IoWrite32(AcpiBaseAddr + R_ACPI_GPE_CNTL, GpeData32);
            Stall (100);
            GpeData32 &= ~B_ACPI_GPE_CNTL_SWGPE_CTRL;
            IoWrite32(AcpiBaseAddr + R_ACPI_GPE_CNTL, GpeData32);
          }
        }
//[-end-161019-IB07400799-add]//
        //
        // Workaround: Set flag to skip all HECI/fTPM access when TXE data region Unavailable
        //
        CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2); 
        CmosData |= B_CMOS_TXE_DATA_UNAVAILABLE; // TXE Data Unavailable
        WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData);
        
        mTxeDataRegionUnavailable = TRUE;
      }
    }
  }
//[-end-160809-IB07400769-add]//

//[-start-160701-IB06740505-add]//
//[-start-170214-IB07400840-modify]//
//  if (*RomBaseAddressPtr >= 0x1000 && *RomBaseAddressPtr < 0x380000) {
  if (*RomBaseAddressPtr >= PcdGet32(PcdFlashIfwiRegionBase) && *RomBaseAddressPtr < PcdGet32(PcdFlashBootPartition2Base)) {
//[-end-170214-IB07400840-modify]//
//[-start-160814-IB07400770-add]//
//[-start-160827-IB07400773-modify]//
//[-start-161026-IB07400805-remove]//
//    if (!PcdGetBool (PcdSecureFlashSupported)) { // Secure flash disabled
//      //
//      // Workaround: Always Gen SW SCI to disable fTPM during update BIOS
//      //
//      AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
//      Pm1Cnt  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_CNT);
//      
//      if (Pm1Cnt & B_ACPI_PM1_CNT_SCI_EN) { // SCI_EN
//        DEBUG ((EFI_D_ERROR, "Clear SW GPE Status...\n"));
//        IoWrite32(AcpiBaseAddr + R_ACPI_GPE0a_STS, B_ACPI_GPE0a_STS_SWGPE);
//        
//        DEBUG ((EFI_D_ERROR, "Enable SW GPE...\n"));
//        GpeData32 = IoRead32(AcpiBaseAddr + R_ACPI_GPE0a_EN);
//        GpeData32 |= B_ACPI_GPE0a_EN_SWGPE_EN;
//        IoWrite32(AcpiBaseAddr + R_ACPI_GPE0a_EN, GpeData32);
//          
//        DEBUG ((EFI_D_ERROR, "Gen SW GPE...\n"));
//        GpeData32 = IoRead32(AcpiBaseAddr + R_ACPI_GPE_CNTL);
//        GpeData32 |= B_ACPI_GPE_CNTL_SWGPE_CTRL;
//        IoWrite32(AcpiBaseAddr + R_ACPI_GPE_CNTL, GpeData32);
//        Stall (100);
//        GpeData32 &= ~B_ACPI_GPE_CNTL_SWGPE_CTRL;
//        IoWrite32(AcpiBaseAddr + R_ACPI_GPE_CNTL, GpeData32);
//      }
//    }
//[-end-161026-IB07400805-remove]//
//[-end-160827-IB07400773-modify]//
//[-end-160814-IB07400770-add]//
    //
    // Copy H1 BPDT/CC to H2 before IFWI update
    //
    FotaPrepare ();
  }

//[-start-170214-IB07400840-modify]//
//  if (*RomBaseAddressPtr >= 0x2000 && *RomBaseAddressPtr < 0x380000) {
  if (*RomBaseAddressPtr >= (PcdGet32(PcdFlashIfwiRegionBase) + 0x1000) && *RomBaseAddressPtr < PcdGet32(PcdFlashBootPartition2Base)) {
//[-end-170214-IB07400840-modify]//
    UpdateBpdt1Signature (TRUE);
  }

//[-start-170214-IB07400840-modify]//
//  if (*RomBaseAddressPtr >= 0x380000) {
  if (*RomBaseAddressPtr >= PcdGet32(PcdFlashBootPartition2Base)) {
//[-end-170214-IB07400840-modify]//
    UpdateBpdt1Signature (FALSE);
  }
//[-end-160701-IB06740505-add]//

  return EFI_SUCCESS;
}

/**
  AH=15h(FBTS write) : Hook function after Write process.

  @param[in]      WriteStatus

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoAfterWriteProcess (
  IN EFI_STATUS        WriteStatus
  )
{
  UINT16                      PmBase;

  PmBase            = 0;

  if (mFlashME == TRUE) {
    PmBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
    gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &SmiEnSave);
  }

  if ((EFI_ERROR (WriteStatus)) && (mFlashME)) {
    mFlashME = FALSE;
    DEBUG ((EFI_D_ERROR, "Write ME Failure!\n"));
  }
  
//[-start-180809-IB07400997-add]//
  mFlashBeWritten = TRUE;
//[-end-180809-IB07400997-add]//

  return EFI_SUCCESS;
}

/**
  AH=14h(FbtsRead) : Hook function before read process

  @param[in, out] ReadAddress           Target linear address to read.
  @param[in, out] ReadSize              Read size.
  @param[in, out] DataBuffer            Pointer to returned data buffer.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoBeforeReadProcess (
  IN OUT UINTN                            *ReadAddress,
  IN OUT UINTN                            *ReadSize,
  IN OUT UINT8                            *DataBuffer
  )
{
  if (*(mSmmFwBlockService->FlashMode) == SMM_FW_DEFAULT_MODE) {
    if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
      OemSvcEcIdle (TRUE);
    }
    
    *(mSmmFwBlockService->FlashMode) = SMM_FW_FLASH_MODE;
  }

//[-start-151124-IB10860189-add]//
//[-start-170215-IB07400840-modify]//
  if (*(mSmmFwBlockService->FlashMode) == SMM_FW_FLASH_MODE) {
    if (PcdGet32(PcdFlashSpiRomSize) != 0) {
      *ReadAddress = *ReadAddress - (UINTN)((UINT64)0x100000000 - PcdGet32(PcdFlashSpiRomSize));
    } else {
      *ReadAddress = *ReadAddress - ROM_8M_BASE; //Chang MMIO address to SPI address
    }
  }
//[-end-170215-IB07400840-modify]//
//[-end-151124-IB10860189-add]//

  return EFI_SUCCESS;
}

/**
  AH=14h(FbtsRead) :  Hook function after read process

  @retval EFI_SUCCESS        Function returns successfully
**/
EFI_STATUS
EFIAPI
IhisiFbtsDoAfterReadProcess (
  IN EFI_STATUS        ReadStatus
  )
{
  return EFI_SUCCESS;
}

//[-start-160824-IB07220130-add]//
/**
  Function to entry S3.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsS3 (
  VOID
  )
{
  UINT16                      PmBase;
  UINT32                      Data32;
  UINT32                      GlobalSmiEn;
  UINT32                      SleepStatus;
  UINTN                       Size;
  IMAGE_INFO                  ImageInfo;
  EFI_STATUS                  Status;
  
  Size = sizeof (IMAGE_INFO);
  Status = CommonGetVariable (
             SECURE_FLASH_INFORMATION_NAME,
             &gSecureFlashInfoGuid,
             &Size,
             &ImageInfo);
  if ((Status == EFI_SUCCESS) && (ImageInfo.FlashMode)) {
    PmBase = (UINT16) PcdGet16(PcdScAcpiIoPortBaseAddress);

    gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_ACPI_PM1_CNT), 1, &SleepStatus);
    SleepStatus = (SleepStatus & ~(B_ACPI_PM1_CNT_SLP_TYP + B_ACPI_PM1_CNT_SLP_EN)) | V_ACPI_PM1_CNT_S3;

    //
    // Disable global SMI to avoid PCH generate any SMI# 
    //
    gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &GlobalSmiEn);
    Data32 = GlobalSmiEn & ~B_SMI_EN_GBL_SMI;
    gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &Data32);

    //
    // Enable S3 SLP_TYP and SLP_EN
    //
    gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_ACPI_PM1_CNT), 1, &SleepStatus);
    SleepStatus |= B_ACPI_PM1_CNT_SLP_EN;
    gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_ACPI_PM1_CNT), 1, &SleepStatus);

    //
    // Enable  global SMI 
    //
    gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &GlobalSmiEn);
    Data32 = GlobalSmiEn | B_SMI_EN_GBL_SMI;
    gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (PmBase + R_SMI_EN), 1, &Data32);
    
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}
//[-end-160824-IB07220130-add]//

/**
  AH=16h(Fbtscomplete), This function uses to execute Ap terminate.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsApTerminated (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  AH=16h(Fbtscomplete), This function uses to execute normal flash.
                        (Update whole image or BIOS region by normal or secure flash.)

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
IhisiFbtsNormalFlash (
  VOID
  )
{
//[-start-161021-IB07400800-modify]//
  if (mTxeDataRegionUnavailable) {
    //
    // TXE data region unavailable, do not Purify Variable
    //
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
//[-end-161021-IB07400800-modify]//
}

/**
  AH=16h(Fbtscomplete), This function uses to execute Partial flash.
                        (Update specific address or update single firmware volume.)

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
EFIAPI
IhisiFbtsPartialFlash (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  AH=16h(Fbtscomplete), This function is a hook funciton berfore ApRequest execute.

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
**/
EFI_STATUS
EFIAPI
IhisiFbtsOemComplete (
  IN UINT8           ApRequest
  )
{
//[-start-160824-IB07220130-add]//
  EFI_STATUS    Status;

  Status = EFI_UNSUPPORTED;
//[-end-160824-IB07220130-add]//

  if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
    OemSvcEcIdle (FALSE);
  }
//[-start-151224-IB07220029-add]//
//[-start-160905-IB07400778-modify]//
  if ((ReadExtCmos8(R_RTC_EXT_INDEX, R_RTC_EXT_TARGET, CmosRecoveryOnFlagAddress)) == V_CMOS_FAST_RECOVERY_ENABLED) {
    WriteExtCmos8(R_RTC_EXT_INDEX, R_RTC_EXT_TARGET, CmosRecoveryOnFlagAddress, V_CMOS_FAST_RECOVERY_DISABLED);
  }
//[-end-160905-IB07400778-modify]//
//[-end-151224-IB07220029-add]//

//[-start-160824-IB07220130-add]//
  if (ApRequest == FlashCompleteS3) {
    Status = IhisiFbtsS3 ();
  }
//[-end-160824-IB07220130-add]//

//[-start-161122-IB07400817-add]//
  //
  // Reset Flag if reflash after H2OFFT -n
  //
  InvalidDone = FALSE;
  ValidDone = FALSE;
//[-end-161122-IB07400817-add]//

//[-start-160824-IB07220130-modify]//
  return Status;
//[-end-160824-IB07220130-modify]//
}

/**
  Function to do system shutdown.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsShutDown (
  VOID
  )
{
  UINT32                        Buffer;
  UINT16                        AcpiBaseAddr;

  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  S5WakeUpSetting ();

  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, (AcpiBaseAddr + R_SMI_EN), 1, &Buffer);
  Buffer = Buffer & ~B_SMI_EN_ON_SLP_EN;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (AcpiBaseAddr + R_SMI_EN), 1, &Buffer);

  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, (AcpiBaseAddr + R_ACPI_PM1_CNT), 1, &Buffer);
  Buffer = Buffer & ~(B_ACPI_PM1_CNT_SLP_EN | B_ACPI_PM1_CNT_SLP_TYP);
  Buffer |= V_ACPI_PM1_CNT_S5;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (AcpiBaseAddr + R_ACPI_PM1_CNT), 1, &Buffer);
  Buffer |= B_ACPI_PM1_CNT_SLP_EN;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, (AcpiBaseAddr + R_ACPI_PM1_CNT), 1, &Buffer);

  return EFI_SUCCESS;
}

/**
  Function to reboot system.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsReboot (
  VOID
  )
{
  UINT8                         Buffer;
//[-start-161012-IB10860211-add]//
  UINTN                         Size;
  EFI_STATUS                    Status;
  IMAGE_INFO                    ImageInfo;
  
  Size = sizeof (IMAGE_INFO);
  Status = CommonGetVariable (
               L"SecureFlashInfo",
               &gSecureFlashInfoGuid,
               &Size,
               &ImageInfo
               );
			   
  if (gBS != NULL && !EFI_ERROR (Status) && ImageInfo.FlashMode == TRUE) {
    FlushHddCache ();
  }
//[-end-161012-IB10860211-add]//

//[-start-160827-IB07400773-modify]//
  if (PcdGetBool (PcdSecureFlashSupported)) { // Secure flash enabled
//[-start-180809-IB07400997-modify]//
    if (mFlashBeWritten) {
      //
      // SPI ROM already be updated, run global reboot
      //
      MmioOr32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_ETR, (UINT32) (B_PMC_ETR_CF9GR));
      Buffer = 0xE;
    } else {
      //
      // Run secure flash under Shell/DOS, just do cold reboot to keep memory data
      //
      Buffer = 0x6; //Cold reboot
    }
//[-end-180809-IB07400997-modify]//
  } else { // Secure flash disable, require global reboot
    MmioOr32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_ETR, (UINT32) (B_PMC_ETR_CF9GR));
    Buffer = 0xE;
  }
//[-end-160827-IB07400773-modify]//

  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, 0xcf9, 1, &Buffer);
  mFlashME = FALSE;

  return EFI_SUCCESS;
}

/**
  Function to flash complete do nothing.

  @retval EFI_SUCCESS      Successfully returns.
**/
EFI_STATUS
EFIAPI
IhisiFbtsApRequestDoNothing (
  VOID
  )
{

//[-start-160815-IB11270162-remove]//
//[-start-160624-IB10860203-add]//
//  if (!PcdGetBool (PcdSecureFlashSupported)) {
//    IoWrite8 (0xCF9, 0x6);
// }
//[-end-160624-IB10860203-add]//
//[-end-160815-IB11270162-remove]//
//[-start-161006-IB07400793-remove]//
////[-start-160812-IB07220127-add]//
//  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
//    //
//    // If this is fast crisis, 
//    // just reset at here to avoid restoring authenticated variables
//    // in RecoveryFlash.c, because the process of which will corrupt 
//    // Device Expansion region.
//    //
////[-start-160923-IB06740511-modify]//
//    IoWrite8 (0xCF9, 0xE);
////[-end-160923-IB06740511-modify]//
//  }
////[-end-160812-IB07220127-add]//
//[-end-161006-IB07400793-remove]//
//[-start-160826-IB07400773-modify]//
  if (!PcdGetBool (PcdSecureFlashSupported)) { // Secure flash disable, require global reboot
    MmioOr32 (PcdGet32(PcdPmcIpc1BaseAddress0) + R_PMC_ETR, (UINT32) (B_PMC_ETR_CF9GR));
  }
//[-end-160826-IB07400773-modify]//
  return EFI_SUCCESS;
}

