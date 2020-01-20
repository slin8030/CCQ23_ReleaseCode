/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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

//[-start-160901-IB03090433-modify]//
#include <Uefi.h>
#include <Ppi/Spi.h>
#include <Guid/PlatformInfo.h>
#include <Guid/SystemNvDataGuid.h>
#include <Library/PeiServicesLib.h>
//[-start-160711-IB03090428-add]//
#include <Library/PeiPlatformConfigUpdateLib.h>
//[-end-160711-IB03090428-add]//
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/BpdtLib.h>
#include <Library/PeiSpiInitLib.h>
#include <Library/BootMediaLib.h>
#include <Library/FlashRegionLib.h>
//[-start-160701-IB06740505-add]//
#include <Library/PeiOemSvcKernelLib.h>
#include <ScRegs/RegsPcu.h>
//[-end-160701-IB06740505-add]//
#include <UfsMphyDataDef.h>
#include "FvCallback.h"
//[-end-160901-IB03090433-modify]//
//[-start-160624-IB03090428-add]//
#include <ChipsetSetupConfig.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Guid/AuthenticatedVariableFormat.h>
//[-end-160624-IB03090428-add]//
//[-start-161007-IB07400794-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-161007-IB07400794-add]//
//[-start-160619-IB07400744-add]//
#include <ChipsetPostCode.h>
//[-end-160619-IB07400744-add]//

#define MAX_DIGEST_SIZE    64
#define TEMP_TOLUD         0x80000000  //should be replaced with actual TOLUD read
//[-start-160519-IB03090427-add]//
#define BPDT_SIGNATURE     0x000055AA
//[-end-160519-IB03090427-add]//

//[-start-160802-IB03090430-add]//
//ZID is stored at offset 0xf00 in SPI or BP1.
#define ZID_LOCATION       0xf00
#define ZID_TOKEN  "ZID="
//size of ZID= ASCII token
#define ZID_TOKEN_SIZE     0x4
#define MAX_ZID_SIZE       16
#define NO_ZID  0

//[-start-160901-IB03090433-remove]//
//typedef enum {
//  BootMediaEmmc,
//  BootMediaUfs,
//  BootMediaSpi,
//  BootMediaMax
//} BOOT_MEDIA_TYPE;
//[-end-160901-IB03090433-remove]//
//[-end-160802-IB03090430-add]//


//[-start-160510-IB03090427-remove]//
//extern EFI_GUID gObbyFirmwareFileSystemFvGuid;
//[-end-160510-IB03090427-remove]//
extern EFI_GUID gFspSFirmwareFileSystemFvGuid;
//[-start-160525-IB03090427-modify]//
extern EFI_GUID gH2OFlashMapRegionPeiFvGuid;
//[-end-160525-IB03090427-modify]//
//[-start-160511-IB03090427-add]//
extern EFI_GUID gUfsPhyOverrideHobGuid;
extern BOOLEAN  mInRecoveryPei;
//[-end-160511-IB03090427-add]//
//[-start-160802-IB03090430-add]//
extern EFI_GUID gEfiBootMediaHobGuid;
//[-end-160802-IB03090430-add]//
//[-start-160624-IB03090428-add]//
extern EFI_GUID gFdoModeEnabledHobGuid;
//[-end-160624-IB03090428-add]//
VOID* GetNvStorageBase (VOID* Address);  //SecUma Hack

//[-start-160308-IB10860193-add]//
//typedef struct {
//  UINT32  Signature;
//  UINT32  Size;
//  UINT8   Format;
//  UINT8   State;
//  UINT16  Reserved;
//  UINT32  Reserved1;
//} VARIABLE_STORE_HEADER;
//
//typedef struct {
//  UINT16    StartId;
//  UINT8     State;
//  UINT8     Reserved;
//  UINT32    Attributes;
//  UINT32    NameSize;
//  UINT32    DataSize;
//  EFI_GUID  VendorGuid;
//} VARIABLE_HEADER;

#define VARIABLE_DATA             0x55AA
//[-end-160308-IB10860193-add]//

//[-start-160517-IB03090427-add]//
BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  );
//[-end-160517-IB03090427-add]//

BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  );
//[-end-160517-IB03090427-add]//

//[-start-160711-IB03090428-add]//
/**
  Produces the default variable HOB to be consumed by the variable driver.

  @param  NvStorageFvHeader   Pointer to the NV Storage firmware volume.

  @retval EFI_SUCCESS         If the function completed successfully.

**/
EFI_STATUS
EFIAPI
CreateDefaultVariableHob (
  IN CONST VARIABLE_STORE_HEADER  *VariableStoreHeader,
  IN       UINT32                 DefaultVariableDataSize
  )
{
  UINT32                  VariableDataOffset    = 0;
  UINT32                  VariableHobDataOffset = 0;

  UINT8                   *VariablePtr             = NULL;
  UINT8                   *VariableHobPtr          = NULL;
  VARIABLE_STORE_HEADER   *VariableStoreHeaderHob  = NULL;

  if (VariableStoreHeader == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "Total size requested for HOB = %d.\n", (sizeof (VARIABLE_STORE_HEADER) + DefaultVariableDataSize + HEADER_ALIGNMENT - 1)));

  VariableStoreHeaderHob = (VARIABLE_STORE_HEADER *) BuildGuidHob (&VariableStoreHeader->Signature,
                                                                   sizeof (VARIABLE_STORE_HEADER) + DefaultVariableDataSize + HEADER_ALIGNMENT - 1);

  if (VariableStoreHeaderHob == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((EFI_D_INFO, "Default HOB allocated at 0x%x\n", VariableStoreHeaderHob));

  //
  // Copy the variable store header to the beginning of the HOB
  //
  CopyMem (VariableStoreHeaderHob, VariableStoreHeader, sizeof (VARIABLE_STORE_HEADER));

  VariablePtr           = (UINT8 *) HEADER_ALIGN ((UINTN) (VariableStoreHeader + 1));
  VariableDataOffset    = (UINT32) ((UINTN) VariablePtr - (UINTN) VariableStoreHeader);
  VariableHobPtr        = (UINT8 *) HEADER_ALIGN ((UINTN) (VariableStoreHeaderHob + 1));
  VariableHobDataOffset = (UINT32) ((UINTN) VariableHobPtr - (UINTN) VariableStoreHeaderHob);

  //
  // Copy the Setup default variable
  //
  CopyMem (VariableHobPtr, VariablePtr, DefaultVariableDataSize);

  //
  // Update the variable store size in the HOB
  //
  VariableStoreHeaderHob->Size = sizeof (VARIABLE_STORE_HEADER) + DefaultVariableDataSize;

  return EFI_SUCCESS;
}

/**
  Caches the Setup defaults so Setup data is returned more quickly.

  @param  NvStorageFvHeader   Pointer to the NV Storage firmware volume.

  @retval EFI_SUCCESS         If the function completed successfully.
**/
EFI_STATUS
EFIAPI
CreateVariableHobs (
  IN CONST EFI_FIRMWARE_VOLUME_HEADER *NvStorageFvHeader
  )
{
  EFI_STATUS                        Status;

  EFI_PHYSICAL_ADDRESS              VariableStoreBase;
  EFI_GUID                          VariableGuid;
  UINT8                             VariableState;

  BOOLEAN                           AuthenticatedVariableStore = FALSE;
  BOOLEAN                           FoundVariableDefaults      = FALSE;

  UINT32                            VariableHeaderSize = 0;
  UINT32                            VariableNameSize   = 0;
  UINT32                            VariableDataSize   = 0;
  UINT32                            SetupVariableSize  = 0;

  CHAR16                            *VariableName                     = NULL;
  UINT8                             *VariableSearchCandidateDataPtr   = NULL;

  VARIABLE_STORE_HEADER             *VariableStoreHeader              = NULL;
  VARIABLE_HEADER                   *StartVariableStoreVariableHeader = NULL;
  VARIABLE_HEADER                   *VariableSearchHeader             = NULL;
  VARIABLE_HEADER                   *LastVariableStoreVariableHeader  = NULL;

//[-start-160624-IB03090428-modify]//
  CHIPSET_CONFIGURATION             *SystemConfiguration              = NULL;
//[-end-160624-IB03090428-modify]//

  DEBUG ((EFI_D_INFO, "Loading variable defaults from NvStorage...\n"));
  DEBUG ((EFI_D_INFO, "  NvStorageHeader at 0x%x\n", NvStorageFvHeader));

  if (NvStorageFvHeader == NULL \
    || NvStorageFvHeader->Signature != EFI_FVH_SIGNATURE \
    || !CompareGuid (&gEfiSystemNvDataFvGuid, &NvStorageFvHeader->FileSystemGuid)) {
    DEBUG ((EFI_D_ERROR, "  NvStorage FV passed to gather setup defaults is invalid!\n"));
    return EFI_INVALID_PARAMETER;
  }

  VariableStoreBase   = (EFI_PHYSICAL_ADDRESS) ((UINTN) NvStorageFvHeader + NvStorageFvHeader->HeaderLength);
  VariableStoreHeader = (VARIABLE_STORE_HEADER *) (UINTN) VariableStoreBase;

  DEBUG ((EFI_D_INFO, "  VariableStoreHeader at 0x%x. VariableStoreSize = %d\n", VariableStoreHeader, (UINTN) VariableStoreHeader->Size));

  AuthenticatedVariableStore = (BOOLEAN) CompareGuid (&VariableStoreHeader->Signature, &gEfiAuthenticatedVariableGuid);

  StartVariableStoreVariableHeader = (VARIABLE_HEADER *) HEADER_ALIGN (VariableStoreHeader + 1);
  LastVariableStoreVariableHeader = (VARIABLE_HEADER *) HEADER_ALIGN ((CHAR8 *) VariableStoreHeader + VariableStoreHeader->Size);

  DEBUG ((EFI_D_INFO, "  StartVariableStoreVariableHeader at 0x%x. LastVariableSearchHeader at 0x%x\n", StartVariableStoreVariableHeader, LastVariableStoreVariableHeader));

  ASSERT (StartVariableStoreVariableHeader < LastVariableStoreVariableHeader);

  VariableHeaderSize = sizeof (VARIABLE_HEADER);

  //
  // Attempt to find the SetupDefault variable
  //
  VariableSearchHeader = StartVariableStoreVariableHeader;

  while ((VariableSearchHeader != NULL)
    && (VariableSearchHeader <= LastVariableStoreVariableHeader)
    && (VariableSearchHeader->StartId == VARIABLE_DATA)) {
    VariableName = (CHAR16 *) ((CHAR8 *) ((CHAR8 *) VariableSearchHeader + VariableHeaderSize));

    if (!AuthenticatedVariableStore) {
      VariableNameSize = VariableSearchHeader->NameSize;
      VariableDataSize = VariableSearchHeader->DataSize;
      VariableGuid     = VariableSearchHeader->VendorGuid;
      VariableState    = VariableSearchHeader->State;
    } else {
      VariableNameSize = ((VARIABLE_HEADER *) VariableSearchHeader)->NameSize;
      VariableDataSize = ((VARIABLE_HEADER *) VariableSearchHeader)->DataSize;
      VariableGuid     = ((VARIABLE_HEADER *) VariableSearchHeader)->VendorGuid;
      VariableState    = ((VARIABLE_HEADER *) VariableSearchHeader)->State;
    }

    DEBUG ((EFI_D_INFO, "  VariableName at 0x%x.\n", VariableName));

    VariableSearchCandidateDataPtr = (UINT8 *) (((CHAR8 *) VariableSearchHeader + VariableHeaderSize) \
                                       + VariableNameSize \
                                       + GET_PAD_SIZE (VariableNameSize));

    DEBUG ((EFI_D_INFO, "  VariableSearchCandidatePtr at 0x%x.\n", (UINTN) VariableSearchCandidateDataPtr));
    DEBUG ((EFI_D_INFO, "    Variable name is %s.\n", VariableName));
    DEBUG ((EFI_D_INFO, "    Variable data size is %d bytes.\n", VariableDataSize));

//[-start-160624-IB03090428-modify]//
    if (CompareGuid (&VariableGuid, &gSystemConfigurationGuid) \
//[-end-160624-IB03090428-modify]//
      && !StrCmp (L"Setup", VariableName) \
      && (VariableState == VAR_ADDED)) {
      DEBUG ((EFI_D_INFO, "  Found the Setup defaults via BPDT to cache.\n"));

      SystemConfiguration   = (CHIPSET_CONFIGURATION *) VariableSearchCandidateDataPtr;
      SetupVariableSize     = VariableDataSize;
      FoundVariableDefaults = TRUE;
    }

    //
    // Get next variable header
    //
    VariableSearchHeader = (VARIABLE_HEADER *) HEADER_ALIGN ((CHAR8 *) VariableSearchCandidateDataPtr \
                                  + VariableDataSize \
                                  + GET_PAD_SIZE (VariableDataSize));
  }

  if (!FoundVariableDefaults) {
    DEBUG ((EFI_D_ERROR, "Could not find the setup defaults in the NvStorage FV!\n"));
    return EFI_NOT_FOUND;
  }

  //
  // Update values in Setup before it is written to the default HOB
  //
  Status = UpdateSetupDataValues (SystemConfiguration);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "  Couldn't override memory and platform information system configuration values.\n"));
  }

  DEBUG ((EFI_D_INFO, "  Total size of data to copy for default HOB = %d bytes.\n", (UINT32) ((VariableSearchCandidateDataPtr + VariableDataSize) - (UINTN) StartVariableStoreVariableHeader)));

  Status = CreateDefaultVariableHob (VariableStoreHeader, (UINT32) ((VariableSearchCandidateDataPtr + VariableDataSize) - (UINTN) StartVariableStoreVariableHeader));
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "  Error occurred creating the default variable HOB. Variable data is invalid.\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  DEBUG ((EFI_D_INFO, "Successfully read Setup defaults.\n"));

  return Status;
}
//[-end-160711-IB03090428-add]//
//[-start-160701-IB06740505-add]//
EFI_STATUS
PchResetPei (
  VOID
  )
/*++

Routine Description:

  Provide hard reset PPI service.
  To generate full hard reset, write 0x0E to ICH RESET_GENERATOR_PORT (0xCF9).

Arguments:

  PeiServices       General purpose services available to every PEIM.

Returns:

  Not return        System reset occured.
  EFI_DEVICE_ERROR  Device error, could not reset the system.

--*/
{
  IoWrite8 (
    R_RST_CNT,
    V_RST_CNT_HARDSTARTSTATE
    );

  IoWrite8 (
    R_RST_CNT,
    V_RST_CNT_HARDRESET
    );

  //
  // System reset occurred, should never reach at this line.
  //
  ASSERT_EFI_ERROR (EFI_DEVICE_ERROR);
  CpuDeadLoop();

  return EFI_DEVICE_ERROR;
}
//[-end-160701-IB06740505-add]//

/**
  <description>

  @param  DataOffset    Pointer in memory of the BPDT entry for UFS Phy Config in BP2

  @retval EFI_SUCCESS    If the function executes correctly
**/
EFI_STATUS
EFIAPI
ParseUfsPhyConfigPayload (
  IN VOID  *DataOffset
  )
{
  UFS_MPHY_OVERRIDE_CONFIG    UfsOverrideCfgData;
  UFS_MPHY_OVERRIDE_CONFIG    *TmpUfsOverrideCfg;

  DEBUG ((EFI_D_INFO, "Parsing UFS PHY Payload from BPDT..\n"));
  TmpUfsOverrideCfg = (UFS_MPHY_OVERRIDE_CONFIG *)DataOffset;
  CopyMem (
    &UfsOverrideCfgData,
    TmpUfsOverrideCfg,
    sizeof(UFS_MPHY_OVERRIDE_CONFIG) - (sizeof(UFS_MPHY_REG_CONFIG) *(UFS_MPHY_MAX_OVERRIDE_REGS - TmpUfsOverrideCfg->RegCount))
  );
  DEBUG ((EFI_D_INFO, "UfsMphyOverrideCfg.Magic:0x%x,0x%x,0x%x,0x%x \n", UfsOverrideCfgData.Magic[0], UfsOverrideCfgData.Magic[1], UfsOverrideCfgData.Magic[2], UfsOverrideCfgData.Magic[3]));
  DEBUG ((EFI_D_INFO, "PlatformInfo->UfsMphyOverrideCfg.Checksum:0x%x \n", UfsOverrideCfgData.Checksum));
  DEBUG ((EFI_D_INFO, "PlatformInfo->UfsMphyOverrideCfg.RegCount:0x%x \n", UfsOverrideCfgData.RegCount));

  //
  // Build HOB for BootMediaData
  //
  BuildGuidDataHob (
    &gUfsPhyOverrideHobGuid,
    &UfsOverrideCfgData,
    sizeof (UFS_MPHY_OVERRIDE_CONFIG)
  );

  return EFI_SUCCESS;
}



/**
  <description>

  @param  BpdtPayloadPtr    Pointer in memory of the BPDT entry for OBB

  @retval EFI_SUCCESS       If the function executes correctly
**/
EFI_STATUS
EFIAPI
ParseObbPayload (
  IN   BPDT_PAYLOAD_DATA    *BpdtPayloadPtr,
  IN   EFI_BOOT_MODE        BootMode
  )
{
  UINT8                       Index;
  UINT32                      FvAddr;
  UINT32                           PayloadTail;
  UINTN                            VariableSize;
  EFI_STATUS                       Status;
  SUBPART_DIR_HEADER               *DirectoryHeader;
  SUBPART_DIR_ENTRY                *DirectoryEntry;
  EFI_FIRMWARE_VOLUME_HEADER       *FvHeader;
  EFI_FIRMWARE_VOLUME_HEADER       *FvHeaderS3;
//[-start-160624-IB03090428-remove]//
//  EFI_PLATFORM_INFO_HOB            *PlatformInfo;
//[-end-160624-IB03090428-remove]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariableServices;
  CHIPSET_CONFIGURATION            SystemConfiguration;
//[-start-160624-IB03090428-remove]//
//  EFI_HOB_GUID_TYPE                *GuidHobPtr;
//[-end-160624-IB03090428-remove]//
  EFI_HOB_GUID_TYPE                *FdoEnabledGuidHob = NULL;
  EFI_GUID                         *FvName;
//[-start-170516-IB08450375-modify]//
  VOID                             *MemoryPayload;
//[-end-170516-IB08450375-modify]//
//[-start-170106-IB11270173-add]//
  VOID                        *Memory;
  UINT64                      FvLength;
//[-end-170106-IB11270173-add]//
  DEBUG ((EFI_D_INFO, "Parsing and checking OBB Payload from BPDT..\n"));
//[-start-170106-IB11270173-remove]//
//[-start-161026-IB11270166-add]//
//  #if !BXTI_PF_ENABLE
//  //
//  // if Payload is not already in Memory, then use SSE4 to copy it in
//  // this is ALOT faster than having the core copy from MMIO during intallFv.
//  //
//  if ( BootMode != BOOT_ON_S3_RESUME && (UINT32)BpdtPayloadPtr->DataPtr > TEMP_TOLUD ){
//    DEBUG ((EFI_D_INFO, "Copying OBB contents from MMIO to Memory\n"));
//    MemoryPayload = AllocatePages (EFI_SIZE_TO_PAGES (BpdtPayloadPtr->Size));
//    CopyMemSse4 (MemoryPayload, BpdtPayloadPtr->DataPtr, BpdtPayloadPtr->Size);
//    BpdtPayloadPtr->DataPtr = MemoryPayload;
//  }
//#endif
//[-end-161026-IB11270166-add]//
//[-end-170106-IB11270173-remove]//
  DirectoryHeader = (SUBPART_DIR_HEADER *)BpdtPayloadPtr->DataPtr;
  PayloadTail = (UINT32)DirectoryHeader + BpdtPayloadPtr->Size;   //Note: BpdtPayloads are padded for 4K alignment.
  DirectoryEntry = (SUBPART_DIR_ENTRY *)((UINTN)DirectoryHeader + sizeof(SUBPART_DIR_HEADER));
  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)((UINTN)DirectoryHeader + DirectoryEntry->EntryOffset);
  DEBUG ((EFI_D_INFO, "DirectoryHeader = %x, NumOf Dir Entries = %x\n", DirectoryHeader, DirectoryHeader->NumOfEntries));

//[-start-160517-IB03090427-remove]//
//  //this loop is just for info.
//  for (Index = 0; Index < DirectoryHeader->NumOfEntries; Index++, DirectoryEntry++) {
//    FvAddr = (UINT32)DirectoryHeader + DirectoryEntry->EntryOffset;
//    DEBUG ((EFI_D_INFO, "%a Entry is located at %x\n", DirectoryEntry->EntryName, FvAddr));
//  }
//[-end-160517-IB03090427-remove]//

//[-start-161123-IB07250310-remove]//
//  #ifdef FSP_WRAPPER_FLAG
//  if (BootMode != BOOT_ON_S3_RESUME) {
//    PcdSet32 (PcdFspsBaseAddress, (UINT32)FvHeader);
//  }
//  #endif
//[-end-161123-IB07250310-remove]//

//[-start-160517-IB03090427-modify]//
//  while ((UINT32)FvHeader + FvHeader->FvLength < PayloadTail) {
  for (Index = 0; Index < DirectoryHeader->NumOfEntries; Index++, DirectoryEntry++) {

    FvAddr = (UINT32)DirectoryHeader + DirectoryEntry->EntryOffset;
    DEBUG ((EFI_D_INFO, "%a Entry is located at %x\n", DirectoryEntry->EntryName, FvAddr));
    FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)FvAddr;

    if (FvHeader->Signature != EFI_FVH_SIGNATURE) {
      DEBUG ((EFI_D_INFO, "This Entry does not have an FV Header.\n"));
      continue;
    }
//[-end-160517-IB03090427-modify]//
    if (FvHeader->ExtHeaderOffset == 0) {
      FvName = (EFI_GUID *)((UINTN)FvHeader + 0x48);  // 0x48 == FvHeader->BlockMap
    } else {
      FvName = (EFI_GUID *)((UINTN)FvHeader + FvHeader->ExtHeaderOffset);
    }
    DEBUG ((EFI_D_INFO, "Found Fv with GUID: %g\n", FvName));

    if (BootMode == BOOT_ON_S3_RESUME) {
      // FspW requires both IBBR and FSP-S on S3 resume
      // but only copy FSP-S, do not install it.
      DEBUG ((DEBUG_INFO, "S3 Resume: Only looking for IBBR and FSP-S.\n"));
//[-start-161123-IB07250310-modify]//
//      if (CompareGuid(FvName, &gFspSFirmwareFileSystemFvGuid) ||
//[-start-160525-IB03090427-modify]//
          //CompareGuid(FvName, &gIbbrFirmwareFileSystemFvGuid)) {
      if (CompareGuid(FvName, &gH2OFlashMapRegionPeiFvGuid)) {
//[-end-160525-IB03090427-modify]//
//[-end-161123-IB07250310-modify]//
        FvHeaderS3 = FvHeader;
//[-start-170516-IB08450375-modify]//
        //#if !BXTI_PF_ENABLE
        if ( (UINT32)FvHeader > TEMP_TOLUD ) {
          MemoryPayload = AllocatePages( EFI_SIZE_TO_PAGES((UINT32)FvHeader->FvLength) );
          CopyMemSse4(MemoryPayload, FvHeader, (UINT32)FvHeader->FvLength);
          FvHeaderS3 = MemoryPayload;
        }
        //#endif
//[-end-170516-IB08450375-modify]//
//[-start-160525-IB03090427-modify]//
        //if (CompareGuid(FvName, &gIbbrFirmwareFileSystemFvGuid)) {
        if (CompareGuid(FvName, &gH2OFlashMapRegionPeiFvGuid)) {
//[-end-160525-IB03090427-modify]//
          PeiServicesInstallFvInfoPpi (NULL, FvHeaderS3, (UINT32)FvHeaderS3->FvLength, NULL, NULL);
          #ifndef FSP_WRAPPER_FLAG
          break;      //added for small performance increase - nonFspW only needs IBBR
          #endif
        } else {
          #ifdef FSP_WRAPPER_FLAG
          PcdSet32 (PcdFspsBaseAddress, (UINT32)FvHeaderS3);
          #endif
        }
      }
    }
    else if (CompareGuid (&FvHeader->FileSystemGuid, &gEfiSystemNvDataFvGuid)) {
//[-start-160824-IB07220130-add]//
        if (BootMode == BOOT_ON_FLASH_UPDATE) {
          continue;
        }
//[-end-160824-IB07220130-add]//
      PcdSet32(PcdFlashNvStorageVariableBase, (UINT32)FvHeader);
      PcdSet32(PcdFlashNvStorageFtwWorkingBase, PcdGet32(PcdFlashNvStorageVariableBase) + PcdGet32(PcdFlashNvStorageVariableSize));
      PcdSet32(PcdFlashNvStorageFtwSpareBase, PcdGet32(PcdFlashNvStorageFtwWorkingBase) + PcdGet32(PcdFlashNvStorageFtwWorkingSize));
      PcdSet32(PcdFlashNvStorageFactoryCopyBase, PcdGet32(PcdFlashNvStorageFtwSpareBase) + PcdGet32(PcdFlashNvStorageFtwSpareSize));
      DEBUG ((EFI_D_INFO, "NVStorage FV at 0x%x.\n", (UINT32) FvHeader));
      Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Could not locate EFI_PEI_READ_ONLY_VARIABLE2_PPI.\n"));
        ASSERT_EFI_ERROR (Status);
        return EFI_NOT_FOUND;
      }
      FdoEnabledGuidHob = GetFirstGuidHob (&gFdoModeEnabledHobGuid);

      VariableSize = sizeof (CHIPSET_CONFIGURATION);
      Status = VariableServices->GetVariable (
                                               VariableServices,
                                               SETUP_VARIABLE_NAME,
                                               &gSystemConfigurationGuid,
                                               NULL,
                                               &VariableSize,
                                               &SystemConfiguration
                                               );
        //
        // Load the variable defaults if:
        //   1. Variables loaded via RBP (e.g. Setup) are not present.
        //   2. Flash Descriptor Override (FDO) mode is enabled.
        //
        if (Status == EFI_NOT_FOUND || FdoEnabledGuidHob != NULL) {
        DEBUG ((EFI_D_INFO, "Initializing variable defaults from BPDT...\n"));

        // Create the variable default HOB
        Status = CreateVariableHobs (FvHeader);
      } else if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Unable to read Setup variable to determine if defauls should be loaded.\n"));
        ASSERT_EFI_ERROR (Status);
      }
    }
//[-start-160624-IB03090428-remove]//
//    else if (CompareGuid(FvName, &gObbyFirmwareFileSystemFvGuid)) {
//      DEBUG ((EFI_D_INFO, "Found OBBY: Saving info and skipping Install FV.\n"));
//      GuidHobPtr = GetFirstGuidHob (&gEfiPlatformInfoGuid);
//      ASSERT (GuidHobPtr != NULL);
//      PlatformInfo = GET_GUID_HOB_DATA(GuidHobPtr);
//      PlatformInfo->FvMain3Base = (UINTN)FvHeader;
//      PlatformInfo->FvMain3Length = (UINT32)(FvHeader->FvLength);
//    }
//[-end-160624-IB03090428-remove]//
    //
    // ONLY if none of other conditions have been met, then install the FV.
    // This step needs to be last, and should not install Fsp-S, Only copy it to memory (above).
    //
    else if (!CompareGuid(FvName, &gFspSFirmwareFileSystemFvGuid)) {
//[-start-160923-IB06740511-add]//
            if (BootMode == BOOT_IN_RECOVERY_MODE) {
              if (FvHeader->ExtHeaderOffset != 0) {
                if (CompareGuid(FvName, &gH2OFlashMapRegionDxeFvGuid)) {
                  continue;
                }
              }
            }
//[-end-160923-IB06740511-add]//
//[-start-170106-IB11270173-add]//
      //
      // Copy SPI data to memory to enhance the performance.    
      // In APL CRB, it only handles the DXE Fv.
      //
      FvLength = (UINTN)(UINT64)FvHeader->FvLength;
      Memory = AllocatePages ( EFI_SIZE_TO_PAGES((UINTN)FvLength));
      CopyMem(Memory , (VOID *) FvHeader , (UINTN)FvLength);
      FvHeader = Memory;
//[-end-170106-IB11270173-add]//
      PeiServicesInstallFvInfoPpi (
        NULL,
        FvHeader,
        (UINT32)(FvHeader->FvLength),
        NULL,
        NULL
        );
    }//if/else S3

    FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)((UINTN)FvHeader + (UINTN)FvHeader->FvLength);
  } //while (< PayloadTail)

//[-start-161123-IB07250310-remove]//
//  #ifdef FSP_WRAPPER_FLAG
//  DEBUG ((DEBUG_INFO, "PcdFspsBaseAddress:  0x%x\n", PcdGet32(PcdFspsBaseAddress) ));
//  #endif
//[-end-161123-IB07250310-remove]//

  return EFI_SUCCESS;
}

//[-start-160701-IB06740505-add]//
/**
  <description>

  @param  BpdtPayloadPtr    Pointer in memory of the BPDT entry for IBBR

  @retval EFI_SUCCESS       If the function executes correctly
**/
EFI_STATUS
EFIAPI
ParseIbbrPayload (
  IN   BPDT_PAYLOAD_DATA    *BpdtPayloadPtr,
  IN   EFI_BOOT_MODE        BootMode
  )
{
  UINT8                       Index;
  UINT32                      FvAddr;
//[-start-160619-IB07400744-modify]//
//#if !BXTI_PF_ENABLE
  VOID                        *MemoryPayload;
//#endif
//[-end-160619-IB07400744-modify]//
  SUBPART_DIR_HEADER          *DirectoryHeader;
  SUBPART_DIR_ENTRY           *DirectoryEntry;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;
  UINT32                      *FvName;
  VOID                        *Memory;
  UINT64                      FvLength;

  DEBUG ((EFI_D_INFO, "Parsing IBBR Payload from BPDT..\n"));

//[-start-161022-IB07400803-modify]//
//#if !BXTI_PF_ENABLE
  //
  // if Payload is not already in Memory, then use SSE4 to copy it in
  // this is ALOT faster than having the core copy from MMIO during intallFv.
  //
  if (!IsIOTGBoardIds()) {
    if ( BootMode != BOOT_ON_S3_RESUME && (UINT32)BpdtPayloadPtr->DataPtr > TEMP_TOLUD ){
      DEBUG ((EFI_D_INFO, "Copying IBB contents from MMIO to Memory\n"));
      MemoryPayload = AllocatePages (EFI_SIZE_TO_PAGES (BpdtPayloadPtr->Size));
      CopyMemSse4 (MemoryPayload, BpdtPayloadPtr->DataPtr, BpdtPayloadPtr->Size);
      BpdtPayloadPtr->DataPtr = MemoryPayload;
    }
  }
//#endif
//[-end-161022-IB07400803-modify]//

  DirectoryHeader = (SUBPART_DIR_HEADER *)BpdtPayloadPtr->DataPtr;
  DirectoryEntry = (SUBPART_DIR_ENTRY *)((UINTN)BpdtPayloadPtr->DataPtr + sizeof(SUBPART_DIR_HEADER));

  if (DirectoryHeader->HeaderMarker != SIGNATURE_32('$', 'C', 'P', 'D')) {
    return EFI_UNSUPPORTED;
  }

  FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)((UINTN)DirectoryHeader + DirectoryEntry->EntryOffset);
  DEBUG ((EFI_D_INFO, "DirectoryHeader = %x, NumOf Dir Entries = %x\n", DirectoryHeader, DirectoryHeader->NumOfEntries));

  for (Index = 0; Index < DirectoryHeader->NumOfEntries; Index++, DirectoryEntry++) {
    FvAddr = (UINT32)DirectoryHeader + DirectoryEntry->EntryOffset;
    DEBUG ((EFI_D_INFO, "%a Entry is located at %x\n", DirectoryEntry->EntryName, FvAddr));
    FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *)FvAddr;

    if (FvHeader->Signature != EFI_FVH_SIGNATURE) {
      DEBUG ((EFI_D_INFO, "This Entry does not have an FV Header.\n"));
      continue;
    }

    FvName = (UINT32 *)DirectoryEntry->EntryName;

    if ((*FvName == SIGNATURE_32('I', 'B', 'B', 'R'))) {
      FvLength = (UINTN)(UINT64)FvHeader->FvLength;
      // copy SPI data to memory
      Memory = AllocatePages ( EFI_SIZE_TO_PAGES((UINTN)FvLength));
      CopyMem(Memory , (VOID *) FvHeader , (UINTN)FvLength);
      FvHeader = Memory;
      PeiServicesInstallFvInfoPpi (
        NULL,
        (VOID *) (UINTN) FvHeader,
        (UINT32) FvLength,
        NULL,
        NULL
        );
    }
//[-start-161123-IB07250310-add]//
    #ifdef FSP_WRAPPER_FLAG
    if ((*FvName == SIGNATURE_32('F', 'S', 'P', 'S'))) {
      if (BootMode == BOOT_ON_S3_RESUME) {
        #if !BXTI_PF_ENABLE
        if ((UINT32)FvHeader > TEMP_TOLUD) {
          DEBUG ((DEBUG_INFO, "FvHeader > TEMP_TOLUD\n"));
          MemoryPayload = AllocatePages (EFI_SIZE_TO_PAGES ((UINT32)FvHeader->FvLength));
          CopyMemSse4 (MemoryPayload, FvHeader, (UINT32)FvHeader->FvLength);
          FvHeader = MemoryPayload;
        }
        #endif
      }
      PcdSet32 (PcdFspsBaseAddress, (UINT32)FvHeader);
    }
    #endif
//[-end-161123-IB07250310-add]//
  }

//[-start-161123-IB07250310-add]//
  #ifdef FSP_WRAPPER_FLAG
  DEBUG ((DEBUG_INFO, "PcdFspsBaseAddress:  0x%x\n", PcdGet32(PcdFspsBaseAddress)));
  #endif
//[-end-161123-IB07250310-add]//

  return EFI_SUCCESS;
}
//[-end-160701-IB06740505-add]//

/**
  <description>

  @param  Buffer        Fv Buffer to verify
  @param  Size          Size of the Buffer
  @param  SignAddress

  @retval TRUE    Hash verification was successful
  @retval FALSE   Hash verification failed
**/
BOOLEAN
VerifyFvHash (
  IN UINTN          Buffer,
  IN UINTN          Size,
  IN UINTN          SignAddress
  )
{
  UINT8             HashLength;
  UINT8             Digest[MAX_DIGEST_SIZE];
  UINTN             CtxSize;
  VOID              *HashCtx;
  BOOLEAN           HashResult;
  UINT8             Index;

  ZeroMem (Digest, MAX_DIGEST_SIZE);
  CtxSize = Sha256GetContextSize ();
  HashCtx = AllocatePool (CtxSize);

  DEBUG((DEBUG_INFO, "Init... \n"));
  HashResult = Sha256Init (HashCtx);
  if (!HashResult) {
    DEBUG((DEBUG_ERROR, "[Fail]\n"));
    return FALSE;
  }

  DEBUG((DEBUG_INFO, "Update... \n"));
  HashResult = Sha256Update (HashCtx, (VOID*)Buffer, Size);
  if (!HashResult) {
    DEBUG((DEBUG_ERROR, "[Fail]\n"));
    return FALSE;
  }

  DEBUG((DEBUG_INFO, "Finalize... \n"));
  HashResult = Sha256Final (HashCtx, Digest);
  if (!HashResult) {
    DEBUG((DEBUG_ERROR, "[Fail]\n"));
    return FALSE;
  }

  HashLength = * (UINT8*)(UINTN)SignAddress;
  for (Index = 0; Index < HashLength; Index ++) {
    if (Digest[Index] != *(UINT8*)(UINTN)(SignAddress + 1 + Index)) {
      DEBUG((DEBUG_ERROR, "Signed UnMatch\n"));
      break;
    }
  }
  if (Index == HashLength) {
    //
    // Signed Match
    //
    return TRUE;
  }

  //
  // Signed UnMatch
  //
  return FALSE;
}

//[-start-160701-IB06740505-remove]//
////[-start-160519-IB03090427-add]//
//EFI_STATUS
//EFIAPI
//SetFdmPcd (
//  IN const EFI_PEI_SERVICES           **PeiServices,
//  IN VOID                             *LbpPointer
//  )
//{
//  BPDT_HEADER                    *BpdtHeader;
//  BPDT_HEADER                    *sBpdtHeader;
//  BPDT_ENTRY                     *BpdtEntryPtr;
//  UINT8                          Index;
//  UINT8                          Index1;
//  UINT32                         Offset;
//  SUBPART_DIR_HEADER             *DirectoryHeader;
//  SUBPART_DIR_ENTRY              *DirectoryEntry;
//  EFI_FIRMWARE_VOLUME_HEADER     *FvHeader;
//
//  BpdtHeader = (BPDT_HEADER *) LbpPointer;
//
//    while (TRUE) {
//
//      sBpdtHeader = BpdtHeader;
//      BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeader + sizeof(BPDT_HEADER));
//
//      if (BpdtHeader->Signature == BPDT_SIGNATURE) {
//        for (Index = 0; Index < (UINT8)BpdtHeader->DscCount; Index++,BpdtEntryPtr++) {
//
//          if (BpdtSbpdt == BpdtEntryPtr->Type) {
//             sBpdtHeader = (BPDT_HEADER *)((UINTN)LbpPointer+ (UINTN)BpdtEntryPtr->LbpOffset);
//          }
//
//          if (BpdtObb == BpdtEntryPtr->Type) {
//            Offset = BpdtEntryPtr->LbpOffset;
//            DirectoryHeader = (SUBPART_DIR_HEADER*) ((UINTN) LbpPointer + Offset);
//            DirectoryEntry = (SUBPART_DIR_ENTRY *)((UINTN)DirectoryHeader + sizeof(SUBPART_DIR_HEADER));
//
//            for (Index1 = 0; Index1 < DirectoryHeader->NumOfEntries; Index1++, DirectoryEntry++) {
//              FvHeader = (EFI_FIRMWARE_VOLUME_HEADER*) (UINT32)DirectoryHeader + DirectoryEntry->EntryOffset;;
//
//              if (FvHeader->Signature != EFI_FVH_SIGNATURE) {
//                // FDM
//                Offset = DirectoryEntry->EntryOffset;
//                if (*(UINT32*)((UINT32)DirectoryHeader + Offset) == SIGNATURE_32 ('H', 'F', 'D', 'M')) {
//                  PcdSet64 (PcdH2OFlashDeviceMapStart, (UINT32) DirectoryHeader + Offset);
//                  DEBUG ((EFI_D_INFO, "HFDM is found at 0x%X\n", (UINT32) DirectoryHeader + Offset));
//                  break;
//                }
//              }
//            }
//          }
//        }
//
//        if (sBpdtHeader != BpdtHeader) {
//          BpdtHeader = sBpdtHeader;
//        } else {
//          break;
//        }
//
//      }
//    }
//
//  return EFI_SUCCESS;
//}
////[-end-160519-IB03090427-add]//
//[-end-160701-IB06740505-remove]//

//[-start-160701-IB06740505-add]//
EFI_STATUS
EFIAPI
SetFdmPcd2 (
  IN const EFI_PEI_SERVICES           **PeiServices,
  IN VOID                             *LbpPointer
  )
{
  UINT8    *HfdmPointer;
  UINT32   Offset;
  UINT32   FlashLinearAddress;
  UINT32   Bp2Size;
  BOOLEAN  FoundFlag;

  HfdmPointer = LbpPointer;
  FoundFlag = FALSE;

  //
  // BP2 linear address is the midpoint between BIOS base and expansion data base
  // (FlashLinearAddress - 4K) == BP1 size == BP2 size
  //
  GetSpiBootPartitionLinearAddress (BootPart2, &FlashLinearAddress);
  Bp2Size = FlashLinearAddress - 0x1000;

  //
  // Search NvCommon area anyway even if BPDT2 is corrupt.
  // NvCommon area must be in LBP2 and aligned on 4K boundary.
  //
  for (Offset = 0; Offset < Bp2Size; Offset += 0x1000) {
    if (*(UINT32*)((UINT32)HfdmPointer + Offset) == SIGNATURE_32 ('H', 'F', 'D', 'M')) {
      FoundFlag = TRUE;
      PcdSet64 (PcdH2OFlashDeviceMapStart, (UINT32) HfdmPointer + Offset);
      DEBUG ((EFI_D_INFO, "HFDM is found at 0x%X\n", (UINT32) HfdmPointer + Offset));
      break;
    }
  }

  if (!FoundFlag) {
    //
    // Need to found NvCommon region.
    // If not found, system deadloop here to prevent unexpected behavior.
    //
    CpuDeadLoop();
  }

  return EFI_SUCCESS;
}
//[-end-160701-IB06740505-add]//


//[-start-160901-IB03090433-modify]//
/**
 Read SPI or emmc and Set ZID PCD and boot

@retval EFI_SUCCESS    If the function executes correctly
**/

EFI_STATUS
SetZephyrId (
  VOID
  )
{
  SC_SPI_PROTOCOL           *SpiPpi;
  VOID                      *LbpPointer;
  UINT32                    Zid;
  UINT8                     StrZid[MAX_ZID_SIZE + 1] = { 0 };
  EFI_STATUS                Status = EFI_SUCCESS;

  if (BootMediaIsSpi ()) {
    InstallScSpi ();
    Status = PeiServicesLocatePpi (
      &gScSpiPpiGuid,
      0,
      NULL,
      (VOID **) &SpiPpi
      );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Cannot Locate gScSpiPpiGuid: %r \n", Status));
    }

    Status = SpiPpi->FlashRead (
      SpiPpi,
      FlashRegionAll,
      ZID_LOCATION,
      MAX_ZID_SIZE + 1,
      StrZid
      );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Read SPI ZID failed: %r\n", Status));
    }
  }  else {
      //
      // UFS/eMMC flow
      //
      Status = GetBootPartitionPointer (0, &LbpPointer);
      ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      AsciiStrCpyS (StrZid, MAX_ZID_SIZE, (CONST UINT8 *)LbpPointer + ZID_LOCATION);
  }

  //
  //if ZID is found then set PCD
  //
  if (AsciiStrnCmp (StrZid, ZID_TOKEN, ZID_TOKEN_SIZE) == 0) {
    Zid = (UINT32) AsciiStrHexToUint64 (StrZid + ZID_TOKEN_SIZE);
    PcdSet32 (PcdIfwiZid, Zid);
    DEBUG ((EFI_D_INFO, "IFWI ZID=0x%x\n", Zid));
  } else {
    DEBUG ((EFI_D_INFO, "Unable to read ZID from boot device  \n"));
    PcdSet32 (PcdIfwiZid, NO_ZID);
    Status = EFI_NOT_FOUND;
  }

  return Status;
}
//[-end-160901-IB03090433-modify]//

//[-start-160701-IB06740505-modify]//
/**
  Locate and install Firmware Volume Hob's once there is main memory

  @param  PeiServices       General purpose services available to every PEIM.
  @param  NotifyDescriptor  Notify that this module published.
  @param  Ppi               PPI that was installed.

  @retval  EFI_SUCCESS     The function completed successfully.
**/
EFI_STATUS
EFIAPI
GetFvNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
  BPDT_PAYLOAD_DATA     *BpdtPayloadPtr;
  EFI_HOB_GUID_TYPE     *GuidHobPtr;
  BPDT_HEADER           *Bp1HdrPtr;
  BPDT_HEADER           *Bp2HdrPtr;
  EFI_BOOT_MODE         BootMode;

  //
  // If the Hob exists, then GetBpdtPayloadAddress() has already been called
  // one or more times already, So we do not need to re-enter this flow.
  //
  GuidHobPtr = GetFirstGuidHob (&gEfiBpdtLibBp2DataGuid);
  if (GuidHobPtr != NULL) {
    DEBUG ((EFI_D_INFO, "GetFvNotifyCallback already called. Skipping.\n"));
    return Status;
  }

  PeiServicesGetBootMode (&BootMode);
  DEBUG ((EFI_D_INFO, "GetFvNotifyCallback: Processing BPDT Payloads.\n"));

  if (BootMode != BOOT_ON_S3_RESUME && BootMode != BOOT_IN_RECOVERY_MODE) {
    GetBootPartitionPointer (BootPart1, (VOID **)&Bp1HdrPtr);
    GetBootPartitionPointer (BootPart2, (VOID **)&Bp2HdrPtr);

    if ((Bp1HdrPtr->Signature != BPDT_SIGNATURE) || (Bp2HdrPtr->Signature != BPDT_SIGNATURE)) {
      //
      // if the signature do not exist, please check GPIO_111 pin should pull down for SPI boot
      //
      OemSvcSetRecoveryRequest ();
      PchResetPei ();
    }
  }

//[-start-161123-IB07250310-modify]//
//[-start-160519-IB03090427-add]//
#ifndef FSP_WRAPPER_FLAG
  if (!IsRunCrisisRecoveryMode()) {
#endif
//[-end-160519-IB03090427-add]//
    //
    // Get Entry info and call corresponding handler to parse the payload.
    //
    Status = GetBpdtPayloadData (BootPart2, BpdtUfsPhyConfig, &BpdtPayloadPtr);
    if (!EFI_ERROR(Status) && BpdtPayloadPtr->DataPtr != NULL && BpdtPayloadPtr->Size > 0) {
      ParseUfsPhyConfigPayload (BpdtPayloadPtr->DataPtr);
    }

    Status = GetBpdtPayloadData (BootPart2, BpdtObb, &BpdtPayloadPtr);
    if (!EFI_ERROR(Status) && BpdtPayloadPtr->DataPtr != NULL && BpdtPayloadPtr->Size > 0) {
      ParseObbPayload (BpdtPayloadPtr, BootMode);
    }

    Status = GetBpdtPayloadData (BootPart1, BpdtIbb, &BpdtPayloadPtr);
    if (!EFI_ERROR(Status) && BpdtPayloadPtr->DataPtr != NULL && BpdtPayloadPtr->Size > 0) {
      ParseIbbrPayload (BpdtPayloadPtr, BootMode);
    }

    //
    // Search IBBR from LBP2 to support FOTA
    //
    Status = GetBpdtPayloadData (BootPart2, BpdtIbb, &BpdtPayloadPtr);
    if (!EFI_ERROR(Status) && BpdtPayloadPtr->DataPtr != NULL && BpdtPayloadPtr->Size > 0) {
      ParseIbbrPayload (BpdtPayloadPtr, BootMode);
    }
//[-start-160519-IB03090427-add]//
#ifndef FSP_WRAPPER_FLAG
  }
#endif
//[-end-160519-IB03090427-add]//
//[-end-161123-IB07250310-modify]//

  //
  // Search TXE main is exist or not
  //
  Status = GetBpdtPayloadData (BootPart1, BpdtCseMain, &BpdtPayloadPtr);
  if (EFI_ERROR(Status) || BpdtPayloadPtr->DataPtr == NULL || BpdtPayloadPtr->Size == 0) {
    if (BootMode != BOOT_ON_S3_RESUME && BootMode != BOOT_IN_RECOVERY_MODE) {
      OemSvcSetRecoveryRequest ();
      PchResetPei ();
    }
  }

//[-start-160901-IB03090433-modify]//
  //Set ZID from ZID_LOCATION
  SetZephyrId ();
//[-end-160901-IB03090433-modify]//

//[-start-160519-IB03090427-add]//
//  SetFdmPcd (PeiServices, (VOID *)Bp2HdrPtr);
//[-end-160519-IB03090427-add]//
  GetBootPartitionPointer (BootPart2, (VOID **)&Bp2HdrPtr);
//[-start-190220-IB07401083-add]//
  PcdSet32 (PcdH2OFlashDeviceMapBp2Start, (UINT32) Bp2HdrPtr);
  DEBUG ((EFI_D_INFO, "PcdH2OFlashDeviceMapBp2Start at 0x%X\n", (UINT32) PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
//[-end-190220-IB07401083-add]//
  SetFdmPcd2 (PeiServices, (VOID *)Bp2HdrPtr);

  return Status;
}
//[-end-160701-IB06740505-modify]//

