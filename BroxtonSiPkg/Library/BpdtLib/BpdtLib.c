/** @file
  This file contains the implementation of BpdtLib library.

  The library exposes an abstract interface for accessing boot data
  stored in the BPDT format on the Logical Boot Partition of the boot device.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/


#include <Uefi.h>
#include <SeCAccess.h>    // only needed for InternalGetBpSpi(), need to try and remove
#include <PiPei.h>        // needed for EFI_PEI_SERVICES
#include <Ppi/BlockIoPei.h>
#include <Ppi/Spi.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/ScPlatformLib.h>  //this includes ScAccess.h which includes RegsSpi.h  <sigh>
#include <Library/BootMediaLib.h>
#include <Library/BpdtLib.h>
#include <Library/PeiSpiInitLib.h>
#include <Library/BaseCryptLib.h>

#define DIR_HDR_SIGNATURE     SIGNATURE_32('$', 'C', 'P', 'D')
#define DIR_ENTRY_NAME_SIZE   12   //This is spec defined to be bytes
#define SPLIT_ENTRY_PART_1    BIT0
#define SPLIT_ENTRY_PART_2    BIT1
#define SPLIT_ENTRY_FLAGS     (BIT1 | BIT0)
#define BPDT_HOB_DATA_SIZE    (sizeof(BPDT_PAYLOAD_DATA) * BpdtMaxType)
#define SpiMmioBoundary       0xFFFC0000

//[-start-160510-IB03090427-add]//
extern EFI_GUID gUfsBootLunIdHobGuid;
extern EFI_GUID gEfiBootMediaHobGuid;
//[-end-160510-IB03090427-add]//

#pragma pack(push,1)
//UFS structs copied from UfsHci.h, since its not in an include directory
typedef struct {
  UINT8  Length;
  UINT8  DescType;
  UINT8  Rsvd1;
  UINT8  BootEn;
  UINT8  DescAccessEn;
  UINT8  InitPowerMode;
  UINT8  HighPriorityLun;
  UINT8  SecureRemovalType;
  UINT8  InitActiveIccLevel;
  UINT16 PeriodicRtcUpdate;
  UINT8  Rsvd2[5];
} UFS_CONFIG_DESC_GEN_HEADER;

typedef struct {
  UINT8  LunEn;
  UINT8  BootLunId;
  UINT8  LunWriteProt;
  UINT8  MemType;
  UINT32 NumAllocUnits;
  UINT8  DataReliability;
  UINT8  LogicBlkSize;
  UINT8  ProvisionType;
  UINT16 CtxCap;
  UINT8  Rsvd1[3];
} UFS_UNIT_DESC_CONFIG_PARAMS;
#pragma pack(pop)

//
// UFS 2.0 Spec Section 14.1.6.3 - Configuration Descriptor
//
typedef struct {
  UFS_CONFIG_DESC_GEN_HEADER  Header;
  UFS_UNIT_DESC_CONFIG_PARAMS UnitDescConfParams[8];
} UFS_CONFIG_DESC;


/**
  Calculate the Address in SPI of the selected Boot Partition

  @param    BpSel                 Which BP to locate on the SPI
  @param    BpFlashLinearAddress  The linear flash address of selected BP
  @param    LbpPointer            A pointer to the selected Boot partition on this media

  @retval   EFI_SUCCESS           The operation completed successfully.
**/
EFI_STATUS
EFIAPI
InternalGetBpSpi (
  IN  BOOT_PARITION_SELECT  BpSel,
  OUT UINT32                *BpFlashLinearAddress,
  OUT VOID                  **LbpPointer
  )
{
  UINT32  BiosAddr;

  //
  // Compute BP FlashLinearAddress
  //
  switch (BpSel) {
    case BootPart1:
      *BpFlashLinearAddress = 0x1000;
      break;
    case BootPart2:
      if (HeciPciRead16(R_SEC_DevID_VID) != 0xFFFF) {
        //
        // BP2 linear address is the midpoint between BIOS base and expansion data base
        //
        *BpFlashLinearAddress = (GetSpiFlashRegionBase(BIOS_Region) + GetSpiFlashRegionBase(DeviceExpansion1)) / 2;
      } else {
        //
        // W/A for non-secure boot
        //
        *BpFlashLinearAddress = 0x00800000>>1;  //FixedPcdGet32 (PcdFlashAreaSize) >> 1;
      }
      break;
    default:
      return EFI_NOT_FOUND;
      break;
  }
  DEBUG ((DEBUG_INFO, "BpFlashLinearAddress = %X\n", *BpFlashLinearAddress));
  //
  // Calculate Boot partition physical address
  // FLA[26:0] <= (Flash_Region_Limit) - (FFFF_FFFCh - bios_address)
  //
  BiosAddr = GetSpiFlashRegionLimit(BIOS_Region) + 0xFFC - *BpFlashLinearAddress;
  *LbpPointer = (VOID *)((UINTN)0xFFFFFFFC - BiosAddr);
  DEBUG ((DEBUG_INFO, "Flash address of BP%d = %X\n", BpSel+1, *LbpPointer));

  return EFI_SUCCESS;
}


/**
  Copy the Selected BootPartion from the boot media into Memory

  @param    BpSel               Which BP to locate on the block device
  @param    BootMediaType       The type of media currently being boot from
  @param    LbpPointer          A pointer to the selected Boot partition on this media

  @retval   EFI_SUCCESS         The operation completed successfully.
  @retval   EFI_DEVICE_ERROR
**/
EFI_STATUS
EFIAPI
InternalGetBpEmmcUfs (
  IN  BOOT_PARITION_SELECT  BpSel,
  IN  UINT8                 BootMediaType,
  OUT VOID                  **LbpPointer
  )
{
  UINT8                          Index;
  UINT8                          BootPartition=0;
  UINT32                         FvAlignment=0;
  UINT64                         BpLength=0;
  EFI_STATUS                     Status;
  UFS_CONFIG_DESC                *Config = NULL;
  EFI_HOB_GUID_TYPE              *GuidHobPtr = NULL;
  EFI_PEI_BLOCK_IO_MEDIA         Media;
  CONST EFI_PEI_SERVICES         **PeiServices;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI  *BlockIoPpi;


  if (BpSel >= BootPartMax) {
    return EFI_NOT_FOUND;
  }

  ZeroMem (&Media, sizeof(Media));
  BpLength = 0x400000;  //if (BpSel==BootPart1) BpLength=PcdGet32(??);  else
  FvAlignment = 1 << ((EFI_FVB2_ALIGNMENT_128) >> 16);

#ifdef FSP_FLAG
  PeiServices = GetPeiServicesTablePointer();
  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiVirtualBlockIoPpiGuid,  // GUID
                             0,                              // INSTANCE
                             NULL,                           // EFI_PEI_PPI_DESCRIPTOR
                             (VOID **) &BlockIoPpi           // PPI
                             );
  ASSERT_EFI_ERROR (Status);
#endif

  //
  // Read BIOS files from eMMC or UFS
  // IAFW is located at Boot LU B for UFS or boot partition 2 for EMMC.
  //
  if (BootMediaType == BootMediaUfs) {
    DEBUG ((EFI_D_INFO, "GetFvNotifyCallback() - Boot from UFS device.\n"));

    // Get UFS Configuration Descriptor Hob
    GuidHobPtr = GetFirstGuidHob (&gUfsBootLunIdHobGuid);
    if (GuidHobPtr == NULL) {
      ASSERT (GuidHobPtr != NULL);
      return EFI_NOT_FOUND;
    }
    Config = (UFS_CONFIG_DESC*) GET_GUID_HOB_DATA (GuidHobPtr);

    //ToDo: this only works for BP2 on UFS. Need to change to use BpSel.
    for (Index = 0; Index < 8; Index++) {
      if (Config->UnitDescConfParams[Index].BootLunId == 0x2) {  //Boot LU B
        BootPartition = Index;
        DEBUG ((EFI_D_INFO, "Boot LUN ID: %x\n", BootPartition));
        break;
      }
    }
  } else {
    BootPartition = (UINT8)BpSel + 1;    //eMMC part select
  }

#ifndef FSP_FLAG
  PeiServices = GetPeiServicesTablePointer();
  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiVirtualBlockIoPpiGuid,  // GUID
                             BootPartition,                  // INSTANCE
                             NULL,                           // EFI_PEI_PPI_DESCRIPTOR
                             (VOID **) &BlockIoPpi           // PPI
                             );
  ASSERT_EFI_ERROR (Status);
#endif

  // Check the boot media is present
  Status = BlockIoPpi->GetBlockDeviceMediaInfo (
                         (EFI_PEI_SERVICES **)PeiServices,
                         BlockIoPpi,
#ifndef FSP_FLAG
                         0,
#else
                         BootPartition,
#endif
                         &Media
                         );
  if (EFI_ERROR (Status) || !Media.MediaPresent) {
    DEBUG ((EFI_D_ERROR, "Fail to get MediaInfo !\n"));
    return EFI_NOT_FOUND;
  }

  // Copy BP from boot media into Memory
  *LbpPointer = AllocateAlignedPages (EFI_SIZE_TO_PAGES ((UINT32) BpLength), FvAlignment);
  Status = BlockIoPpi->ReadBlocks (
                         (EFI_PEI_SERVICES **)PeiServices,
                         BlockIoPpi,
#ifndef FSP_FLAG
                         0,
#else
                         BootPartition,
#endif
                         0,
                         (UINTN)BpLength,
                         *LbpPointer
                         );
  DEBUG ((DEBUG_INFO, "Memory address of BP%d = %X\n", BpSel + 1, *LbpPointer));
  return Status;
}

/**
  SPI CMD read w/a
  Because the upper 256k of address space is mapped to CSE SRAM,
  it cannot be directly accessed via MMIO from host.
**/
EFI_STATUS
EFIAPI
InternalCheckAndJoinUpperSpi (
  IN      BOOT_PARITION_SELECT  BpSel,
  IN OUT  BPDT_PAYLOAD_DATA     *CombinedSpiObb
  )
{
  VOID                      *UpperSpiObb;
  VOID                      *LbpPointer;    //just pace holder for func call, not used.
  UINT32                    Bp2LinearAddr;
  UINT32                    SpiPart1Size;
  UINT32                    SpiPart2Size;
  EFI_STATUS                Status;
  SC_SPI_PROTOCOL           *SpiPpi;
  CONST EFI_PEI_SERVICES    **PeiServices;

  //
  // Check if Obb part is in upper spi address space.
  // If the entry is from emmc this will be false, it can only be true for SPI.
  //
  if ((UINT32)CombinedSpiObb->DataPtr + CombinedSpiObb->Size >= SpiMmioBoundary) {

    //Get Linear Addr of BpSel (on BXT this will be BP2, since it is)
    InternalGetBpSpi (BpSel, &Bp2LinearAddr, &LbpPointer);

    SpiPart1Size = SpiMmioBoundary - (UINT32)CombinedSpiObb->DataPtr;
    SpiPart2Size = CombinedSpiObb->Size - SpiPart1Size;

    UpperSpiObb = AllocatePages (EFI_SIZE_TO_PAGES (CombinedSpiObb->Size));

    CopyMem (UpperSpiObb, CombinedSpiObb->DataPtr, SpiPart1Size);  //copy MemMapped part of Spi

    //
    // Install ScSpi Ppi
    //
    InstallScSpi ();
    PeiServices = GetPeiServicesTablePointer ();
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gScSpiPpiGuid,
                               0,
                               NULL,
                               (VOID **) &SpiPpi
                               );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Cannot Locate gScSpiPpiGuid: %r. Halting system.\n", Status));
      CpuDeadLoop ();
    }

    //
    // Use SPI cmd to read entry into Memory
    //
    Status = SpiPpi->FlashRead (
                       SpiPpi,
                       FlashRegionBios,
                       Bp2LinearAddr + SpiPart1Size,
                       SpiPart2Size,
                       (UINT8 *)((UINTN)UpperSpiObb + SpiPart1Size)
                       );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Read SPI fail: %r. Halting system.\n", Status));
      CpuDeadLoop ();
    }

    CombinedSpiObb->DataPtr = UpperSpiObb;
  }
  return EFI_SUCCESS;
}


/**
  This function will handle locating and combining both parts of Split Obb,
  regardless of which BootPartition that part1 or part2 is located on.
  After it correctly identifies and locates

  @param  ObbPayloadPtr    Pointer to other part of Obb (hopefully on Bp1)
                           - shouldnt matter if this ptr is actual memory or mmio,
                             since it needs to be combined anyway.
**/
EFI_STATUS
EFIAPI
InternalLocateSplitObb (
  IN   BOOT_PARITION_SELECT  BpSel,
  OUT  BPDT_PAYLOAD_DATA     *ObbPayloadPtr
  )
{
  UINT8               index;
  VOID                *LbpPointer;
  VOID                *LbpOffset;
  BPDT_HEADER         *BpdtHeaderPtr;
  BPDT_HEADER         *sBpdtHeader;
  BPDT_ENTRY          *BpdtEntryPtr;
  EFI_STATUS          Status;

  Status = GetBootPartitionPointer (BpSel, &LbpPointer);
  ASSERT_EFI_ERROR (Status);

  BpdtHeaderPtr = (BPDT_HEADER *)LbpPointer;
  Status = EFI_NOT_FOUND;
  do {
    //reset secondary header each time to ensure proper exit
    sBpdtHeader = NULL;
    BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeaderPtr + sizeof(BPDT_HEADER));

//[-start-160701-IB06740505-modify]//
    if (BpdtHeaderPtr->Signature != BPDT_SIGN_GREEN && BpdtHeaderPtr->Signature != BPDT_SIGN_YELLOW) {
//[-end-160701-IB06740505-modify]//
      DEBUG ((EFI_D_INFO, "Signature of BPDT Header is invalid - Stop Parsing BP%d.\n", BpSel));
      break;
    }
    for (index=0; index < BpdtHeaderPtr->DscCount; index++, BpdtEntryPtr++) {
      if (BpdtEntryPtr->LbpOffset == 0) {
        DEBUG ((EFI_D_INFO, "Skipping Stub-entry for Bpdt type: 0x%x \n", BpdtEntryPtr->Type));
        continue;
      }
      LbpOffset = (VOID *)((UINTN)LbpPointer + BpdtEntryPtr->LbpOffset);

      if (BpdtEntryPtr->Type == BpdtSbpdt) {
        sBpdtHeader = (BPDT_HEADER *)LbpOffset;
        continue;
      }
      if (BpdtEntryPtr->Type == BpdtObb) {
        ObbPayloadPtr->DataPtr = LbpOffset;
        ObbPayloadPtr->Size    = BpdtEntryPtr->Size;
        Status = EFI_SUCCESS;
        break;
      }
    }
    if (sBpdtHeader != NULL) {
      BpdtHeaderPtr = sBpdtHeader;
    }
  } while (sBpdtHeader != NULL);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to locate part of Split Obb: %r. Halting system.\n", Status));
    CpuDeadLoop ();
  }
  return Status;
}


/**
  Locate and return a pointer to the selected Logical Boot Partition.

  For block devices, the address will be the location in memory that the BP was copied to.
  For memory mapped device (eg SPI), the address will be the location on the device.

  @param    LbpSel         Which BP to locate on the boot device
  @param    Address        The address of the selected BP on the boot device

  @retval   EFI_SUCCESS    The operation completed successfully.
  @retval   other          The selected BP could not be found.
**/
EFI_STATUS
EFIAPI
GetBootPartitionPointer (
  IN  BOOT_PARITION_SELECT  LbpSel,
  OUT VOID                  **LbpPointer
  )
{
  BOOT_MEDIA_TYPE           BootMedia;
  UINT32                    BpLinearAddr;
  EFI_STATUS                Status;

  Status = GetBootMediaType (&BootMedia);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (BootMedia) {
    case BootMediaEmmc:
    case BootMediaUfs:
      DEBUG ((EFI_D_INFO, "CSE Boot Device is EMMC/UFS.\n"));
      Status = InternalGetBpEmmcUfs (LbpSel, BootMedia, LbpPointer);
      break;
    case BootMediaSpi:
      DEBUG ((EFI_D_INFO, "CSE Boot Device is SPI.\n"));
      Status = InternalGetBpSpi (LbpSel, &BpLinearAddr, LbpPointer);
      break;
    default:
      DEBUG ((EFI_D_ERROR, "\nGetBootPartitionPointer: Unable to determine boot device!\n"));
//[-start-160531-IB07220089-modify]//
//       Status = EFI_NOT_FOUND;
      Status = InternalGetBpSpi (LbpSel, &BpLinearAddr, LbpPointer);
//[-end-160531-IB07220089-modify]//
      break;
  }
  return Status;
}

/**
  Parse the Boot Partition Descriptor Table of the provided Logical Boot Partition.
  Note: "payload" is the Sub-Partition pointed to by the BDPT Entry of the given type.

  @param   LbpPointer      Pointer to the start of the BootPartition to be parsed
  @param   EntryType       The entry type of the subparition(payload) to look for
  @param   BpdtPayloadPtr  Address of the Struct to put the results into (MUST be allocated by caller)

  @retval  EFI_SUCCESS     The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetBpdtPayloadDataRaw (
  IN  VOID                *LbpPointer,
  IN  BPDT_ENTRY_TYPES    EntryType,
  OUT BPDT_PAYLOAD_DATA   *BpdtPayloadPtr
  )
{
  UINT8                   Index;
  VOID                    *LbpOffset;
  BPDT_HEADER             *BpdtHeaderPtr;
  BPDT_HEADER             *sBpdtHeader;
  BPDT_ENTRY              *BpdtEntryPtr;


  BpdtHeaderPtr = (BPDT_HEADER *)LbpPointer;
  do {
    //reset secondary header each time to ensure proper exit
    sBpdtHeader = NULL;
    BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeaderPtr + sizeof(BPDT_HEADER));

    DEBUG ((DEBUG_INFO, "BpdtHeaderPtr->Signature = %x\n", BpdtHeaderPtr->Signature));
    if ((BpdtHeaderPtr->Signature != BPDT_SIGN_GREEN) && (BpdtHeaderPtr->Signature != BPDT_SIGN_YELLOW)) {
      DEBUG ((EFI_D_ERROR, "Signature of BPDT Header is invalid - Stop Parsing RawBP.\n"));
      return EFI_NOT_FOUND;
    }
    for (Index=0; Index < BpdtHeaderPtr->DscCount; Index++, BpdtEntryPtr++) {
      if (BpdtEntryPtr->LbpOffset == 0) {
        DEBUG ((EFI_D_INFO, "Skipping Stub-entry for Bpdt type: %d\n", BpdtEntryPtr->Type));
        continue;
      }
      LbpOffset = (VOID *)((UINTN)LbpPointer + BpdtEntryPtr->LbpOffset);

      if (BpdtEntryPtr->Type == BpdtSbpdt) {
        sBpdtHeader = (BPDT_HEADER *)LbpOffset;
      }
      if (BpdtEntryPtr->Type == EntryType) {
        DEBUG ((EFI_D_INFO, "Found BPDT Entry. Type: %d, Addr: 0x%x\n", BpdtEntryPtr->Type, LbpOffset));
        BpdtPayloadPtr->DataPtr = LbpOffset;
        BpdtPayloadPtr->Size = BpdtEntryPtr->Size;
        return EFI_SUCCESS;
      }
    }
    if (sBpdtHeader != NULL) {
      BpdtHeaderPtr = sBpdtHeader;
    }
  } while (sBpdtHeader != NULL);

  BpdtPayloadPtr->DataPtr = NULL;
  BpdtPayloadPtr->Size = 0;
  return EFI_NOT_FOUND;
}

//[-start-160701-IB06740505-add]//
EFI_STATUS
EFIAPI
GetSpiBootPartitionLinearAddress (
  IN  BOOT_PARITION_SELECT  LbpSel,
  OUT UINT32                *BpFlashLinearAddress
  )
{
  EFI_STATUS            Status;
  VOID                  *LbpPointer;

  Status = InternalGetBpSpi (LbpSel, BpFlashLinearAddress, &LbpPointer);

  return Status;
}
//[-end-160701-IB06740505-add]//
/**
  Parse the Boot Partition Descriptor Table on the selected Logical Boot Partition.
  Takes the BP as input since some BPDT entry types can exist in both locations.
  Note: "payload" is the Sub-Partition pointed to by the BDPT Entry of the given type.

  @param   EntryType       The entry type of the subparition(payload) to look for
  @param   BpdtPayloadPtr  Pointer to a Struct (in a HOB) containing the Size and Absolute address
                           in memory(or spi) of the BPDT payload(subpart) of EntryType.
                           If the caller shadows the Payload for performance, it should update this value

  @retval  EFI_SUCCESS     The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetBpdtPayloadData (
  IN  BOOT_PARITION_SELECT  BpSel,
  IN  BPDT_ENTRY_TYPES      EntryType,
  OUT BPDT_PAYLOAD_DATA     **BpdtPayloadPtr
  )
{
  UINT8                 Index;
  VOID                  *LbpOffset;
  VOID                  *LbpBuffer = NULL;
  VOID                  *CombinedObb;
  BPDT_PAYLOAD_DATA     *BpdtPayloadList = NULL;
  BPDT_PAYLOAD_DATA     *AdditionalPayloadList = NULL;
  BPDT_PAYLOAD_DATA     AdditionalObbInfo;
  BPDT_PAYLOAD_DATA     *ObbInfoPart1Ptr;
  BPDT_PAYLOAD_DATA     *ObbInfoPart2Ptr;
  BPDT_HEADER           *BpdtHeaderPtr;
  BPDT_HEADER           *sBpdtHeader;
  BPDT_ENTRY            *BpdtEntryPtr;
  BPDT_ENTRY            *ObbEntryPtr = NULL;
  EFI_HOB_GUID_TYPE     *GuidHobPtr;
  EFI_STATUS            Status;
  EFI_GUID              CurrentBpdtDataGuid;
  EFI_GUID              AdditionalDataGuid;
  BOOT_PARITION_SELECT  AdditionalObbBp;

  //
  // Set the data guid's to the correct HOB's based on the selected BP
  // If Current contains Split OBB, then Additional needs to be the BP with the other SplitObb.
  //
  switch (BpSel) {
    case BootPart1:
      CurrentBpdtDataGuid = gEfiBpdtLibBp1DataGuid;
      AdditionalDataGuid  = gEfiBpdtLibBp2DataGuid;
      AdditionalObbBp     = BootPart2;
      break;
    case BootPart2:
      CurrentBpdtDataGuid = gEfiBpdtLibBp2DataGuid;
      AdditionalDataGuid  = gEfiBpdtLibBp1DataGuid;
      AdditionalObbBp     = BootPart1;
      break;
    default:
      DEBUG ((EFI_D_ERROR, "Invalid BootPartion parameter: %d\n", BpSel));
      return EFI_NOT_FOUND;
      break;
  }

  *BpdtPayloadPtr = NULL;
  GuidHobPtr = GetFirstGuidHob (&CurrentBpdtDataGuid);
  if (GuidHobPtr != NULL) {
    DEBUG ((EFI_D_INFO, "BPDT %d already parsed. Getting Payload info from HOB\n", BpSel+1));
    BpdtPayloadList = GET_GUID_HOB_DATA (GuidHobPtr);
    *BpdtPayloadPtr = &BpdtPayloadList[EntryType];
    if (*BpdtPayloadPtr == NULL) {
      DEBUG ((EFI_D_WARN, "Requested Payload info in NULL: %d\n", EntryType));
      return EFI_NOT_FOUND;
    } else {
      return EFI_SUCCESS;
    }
  }

  //Must use ZeroPool here to ensure correct error handling of missing entries.
  BpdtPayloadList = AllocateZeroPool (BPDT_HOB_DATA_SIZE);
  if (BpdtPayloadList == NULL) {
    ASSERT (BpdtPayloadList != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = GetBootPartitionPointer (BpSel, &LbpBuffer);
  ASSERT_EFI_ERROR (Status);

  //
  // Parse the BPDT and store the Entry offsets into the array
  //
  BpdtHeaderPtr = (BPDT_HEADER *)LbpBuffer;
  do {
    //reset secondary header each time to ensure proper exit
    sBpdtHeader = NULL;
    BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeaderPtr + sizeof(BPDT_HEADER));

//[-start-160701-IB06740505-modify]//
    if (BpdtHeaderPtr->Signature != BPDT_SIGN_GREEN && BpdtHeaderPtr->Signature != BPDT_SIGN_YELLOW) {
//[-end-160701-IB06740505-modify]//
      DEBUG ((EFI_D_ERROR, "Signature of BPDT Header is invalid - Stop Parsing BP%d.\n", BpSel+1));
      return EFI_NOT_FOUND;
    }
    for (Index=0; Index < BpdtHeaderPtr->DscCount; Index++, BpdtEntryPtr++) {
      if (BpdtEntryPtr->LbpOffset == 0) {
        DEBUG ((EFI_D_INFO, "Skipping Stub-entry for Bpdt type: %d\n", BpdtEntryPtr->Type));
        continue;
      }
      LbpOffset = (VOID *)((UINTN)LbpBuffer + BpdtEntryPtr->LbpOffset);

      if (BpdtEntryPtr->Type == BpdtSbpdt) {
        sBpdtHeader = (BPDT_HEADER *)LbpOffset;
      }
      if (BpdtEntryPtr->Type < BpdtMaxType) {
        DEBUG ((EFI_D_INFO, "Saving Info of BPDT Entry. Type: %d, Addr: 0x%x\n", BpdtEntryPtr->Type, LbpOffset));
        BpdtPayloadList[BpdtEntryPtr->Type].DataPtr = LbpOffset;
        BpdtPayloadList[BpdtEntryPtr->Type].Size = BpdtEntryPtr->Size;
      }
      if (BpdtEntryPtr->Type == BpdtObb) {
        ObbEntryPtr = BpdtEntryPtr;
      }
    }
    if (sBpdtHeader != NULL) {
      BpdtHeaderPtr = sBpdtHeader;
    }
  } while (sBpdtHeader != NULL);


  //
  // If the additional data HOB contains the address of Obb, then the special cases for
  // Obb were already handled by a previous call to this function (no need to recombine =).
  //
  GuidHobPtr = GetFirstGuidHob (&AdditionalDataGuid);
  if (GuidHobPtr != NULL) {
    AdditionalPayloadList = GET_GUID_HOB_DATA (GuidHobPtr);
  }
  if (AdditionalPayloadList != NULL && AdditionalPayloadList[BpdtObb].DataPtr != NULL) {
    // using copymem will auto scale if the struct defintion changes
    CopyMem (&BpdtPayloadList[BpdtObb], &AdditionalPayloadList[BpdtObb], sizeof (BPDT_PAYLOAD_DATA));
  } else {

    // Check for Payload overlap with non-MMIO SPI region
    InternalCheckAndJoinUpperSpi (BpSel, &BpdtPayloadList[BpdtObb]);

    //
    // look for and handle the Split OBB case
    //
    if (ObbEntryPtr != NULL && (ObbEntryPtr->Flags & SPLIT_ENTRY_FLAGS)) {

      DEBUG ((EFI_D_INFO, "Attempting to resolve Split Obb..\n"));
      InternalLocateSplitObb (AdditionalObbBp, &AdditionalObbInfo);

      // Since each Obb part could be on either BP, then we also need to check
      // it for overlap into the non-MMIO accessible SPI region.
      InternalCheckAndJoinUpperSpi (AdditionalObbBp, &AdditionalObbInfo);

      if (ObbEntryPtr->Flags & SPLIT_ENTRY_PART_1) {
        ObbInfoPart1Ptr = &BpdtPayloadList[BpdtObb];
        ObbInfoPart2Ptr = &AdditionalObbInfo;
      } else {
        ObbInfoPart1Ptr = &AdditionalObbInfo;
        ObbInfoPart2Ptr = &BpdtPayloadList[BpdtObb];
      }
      //
      // Combine the final parts together and set the output address to the result
      //
      CombinedObb = AllocatePages (EFI_SIZE_TO_PAGES ((UINTN)ObbInfoPart1Ptr->Size + ObbInfoPart2Ptr->Size));

      CopyMem (CombinedObb, ObbInfoPart1Ptr->DataPtr, ObbInfoPart1Ptr->Size);
      CopyMem ((VOID *)((UINTN)CombinedObb+ObbInfoPart1Ptr->Size), ObbInfoPart2Ptr->DataPtr, ObbInfoPart2Ptr->Size);
      BpdtPayloadList[BpdtObb].DataPtr = CombinedObb;
      BpdtPayloadList[BpdtObb].Size = ObbInfoPart1Ptr->Size + ObbInfoPart2Ptr->Size;
    }
  }

  //
  // Build HOB for BPDT data.
  // Need to use a HOB (or local PPI) instead of a module global since the library will be called in PreMem,
  // and PreMem globals will not work if executing in place (ie SPI boot with no code in CAR).
  //
  GuidHobPtr = BuildGuidDataHob (
    &gEfiBpdtLibBp2DataGuid,  //EFI_GUID *Guid
    BpdtPayloadList,          //VOID *Data
    BPDT_HOB_DATA_SIZE        //UINTN DataLength
  );
  *BpdtPayloadPtr = &BpdtPayloadList[EntryType];
  FreePool(BpdtPayloadList);

  if (GuidHobPtr == NULL || *BpdtPayloadPtr == NULL) {
   DEBUG ((EFI_D_ERROR, "Unable to create BPDT HOB.\n"));
   return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}


/**
  Search the Subpartition pointed to by BpdtPayloadPtr for a Directory entry with "EntryName",
  then fill in the Struct pointed to by SubPartPayloadPtr with the Addres and Size of the Dir Entry.
  Note: "payload" is the Data pointed to by the Directory Entry with the given name.

  @param   BpdtPayloadPtr     Location and Size of the Payload to search
  @param   EntryName          the String name of the Directory entry to look for
  @param   SubPartPayloadPtr  Address of the Struct to put the results into (MUST be allocated by caller)

  @retval  EFI_SUCCESS        The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetSubPartitionPayloadData (
  IN  BPDT_PAYLOAD_DATA     *BpdtPayloadPtr,
  IN  CONST CHAR8           *EntryName,
  OUT BPDT_PAYLOAD_DATA     *SubPartPayloadPtr
  )
{
  UINT8                   Index;
  CHAR8                   SearchName[DIR_ENTRY_NAME_SIZE];
  UINTN                   SearchNameSize;
  SUBPART_DIR_HEADER      *DirectoryHeader;
  SUBPART_DIR_ENTRY       *DirectoryEntry;

  //pad Name with zeros (per spec) to ensure correct match
  ZeroMem(SearchName, DIR_ENTRY_NAME_SIZE);
  //Safely handle the input string to ensure we dont copy garbage when EntryName is <12
  SearchNameSize = AsciiStrnLenS (EntryName, DIR_ENTRY_NAME_SIZE);
  CopyMem(SearchName, EntryName, SearchNameSize);

  DEBUG ((EFI_D_INFO, "Looking for Directory Entry with Name: %a\n", SearchName));
  DirectoryHeader = (SUBPART_DIR_HEADER *)BpdtPayloadPtr->DataPtr;
  DirectoryEntry = (SUBPART_DIR_ENTRY *)((UINTN)DirectoryHeader + sizeof(SUBPART_DIR_HEADER));

  if (DirectoryHeader->HeaderMarker != DIR_HDR_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "Directory Header is invalid - return Not_Found.\n"));
    return EFI_NOT_FOUND;
  }
  for (Index = 0; Index < DirectoryHeader->NumOfEntries; Index++, DirectoryEntry++) {
    DEBUG ((EFI_D_INFO, " %a Entry is located at %x\n",
      DirectoryEntry->EntryName, (UINTN)DirectoryHeader+DirectoryEntry->EntryOffset));
    if (!CompareMem(DirectoryEntry->EntryName, SearchName, DIR_ENTRY_NAME_SIZE)) {
      SubPartPayloadPtr->DataPtr = (VOID *)((UINTN)DirectoryHeader + DirectoryEntry->EntryOffset);
      SubPartPayloadPtr->Size = DirectoryEntry->EntrySize;
      return EFI_SUCCESS;
    }
  }
  DEBUG ((EFI_D_WARN, "Entry not found.\n"));
  return EFI_NOT_FOUND;
}


/**
  This is a wrapper function for using VerifyHashBpm that will only
  check the hash for the payloads used during normal boot.

  If you want to check other payloads (ie during recovery flow), then the
  calling code should locate and pass in the payload directly to VerifyHashBpm().

  @param  PayloadSel   Which hash in BPM to compare with.

  @retval EFI_SUCCESS  The function completed successfully and the Hash matched.
  @retval other        An error occured when locating and computing the Hash.
**/
EFI_STATUS
EFIAPI
LocateAndVerifyHashBpm (
  IN  BPM_HASH_SELECT    PayloadSel
  )
{

  UINT8                  Index;
  EFI_STATUS             Status;
  VOID                   *HashDataPtr;
  UINT32                 HashDataSize;
  UINT32                 FvTailAddr;
  BPM_DATA_FILE          *BpmPtr;
  BPDT_PAYLOAD_DATA      *IbbPayloadPtr;
  BPDT_PAYLOAD_DATA      *ObbPayloadPtr;
  BPDT_PAYLOAD_DATA      SubPartPayload;
  SUBPART_DIR_HEADER     *DirectoryHeader;
  SUBPART_DIR_ENTRY      *DirectoryEntry;


  //locate Boot Policy Metadata
  DEBUG ((EFI_D_INFO, "Searching IBB for BPM..\n" ));
  GetBpdtPayloadData (BootPart1, BpdtIbb, &IbbPayloadPtr);
  if (IbbPayloadPtr == NULL) {
    return EFI_NOT_FOUND;
  }
  Status = GetSubPartitionPayloadData (IbbPayloadPtr, "BPM.met", &SubPartPayload);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  BpmPtr = (BPM_DATA_FILE *)SubPartPayload.DataPtr;

  //Note: the names "IBBL" and "IBB" need to match the values in the MEU input xml
  switch (PayloadSel) {
    case HashIbbl:
      Status = GetSubPartitionPayloadData (IbbPayloadPtr, "IBBL", &SubPartPayload);
      HashDataPtr  = SubPartPayload.DataPtr;
      HashDataSize = SubPartPayload.Size;
      break;

    case HashIbbm:
      Status = GetSubPartitionPayloadData (IbbPayloadPtr, "IBB", &SubPartPayload);
      HashDataPtr  = SubPartPayload.DataPtr;
      HashDataSize = SubPartPayload.Size;
      break;

    case HashObb:
      Status = GetBpdtPayloadData (BootPart2, BpdtObb, &ObbPayloadPtr);
      if (ObbPayloadPtr == NULL) {
        return EFI_NOT_FOUND;
      }
      DirectoryHeader = (SUBPART_DIR_HEADER *)ObbPayloadPtr->DataPtr;
      DirectoryEntry = (SUBPART_DIR_ENTRY *)((UINTN)DirectoryHeader + sizeof(SUBPART_DIR_HEADER));
      HashDataPtr = (VOID *)((UINTN)DirectoryHeader + DirectoryEntry->EntryOffset);

      //Loop through and find the last dir entry; needed to calculate hashdatasize
      //start at 1, since DirectoryEntry is already pointing to first entry.
      for (Index = 1; Index < DirectoryHeader->NumOfEntries; Index++) {
        DirectoryEntry++;
      }
      //the hashed region is from the start of first dir payload to tail of last dir payload.
      FvTailAddr = (UINT32)DirectoryHeader + DirectoryEntry->EntryOffset + DirectoryEntry->EntrySize;
      HashDataSize = FvTailAddr - (UINT32)HashDataPtr;
      break;

    default:
      //Print Error
      return EFI_INVALID_PARAMETER;
      break;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = VerifyHashBpm(PayloadSel, BpmPtr, HashDataPtr, HashDataSize);
  return Status;
}



/**
  Computes the Hash of a given data block and compares to the one in the Boot Policy Metadata.

  @param  PayloadSel   Which hash in BPM to compare with.
  @param  PayloadPtr   Pointer to the begining of the data to be hashed.
  @param  DataSize     Size of the data to be hashed.
  @param  BpmPtr       Pointer to the BPM structure.

  @retval EFI_SUCCESS             If the function completed successfully.
  @retval EFI_ABORTED             If the attempt to compute the hash fails.
  @retval EFI_INVALID_PARAMETER   If the hash type or size in BPM is unsupported.
  @retval EFI_SECURITY_VIOLATION  If the has does not match the one in BPM.
**/
EFI_STATUS
EFIAPI
VerifyHashBpm (
  IN BPM_HASH_SELECT   PayloadSel,
  IN BPM_DATA_FILE     *BpmPtr,
  IN VOID              *PayloadPtr,
  IN UINT32            DataSize
  )
{
  UINT8                Index;
  UINT32               BpmHashAlgo;
  UINT32               BpmHashSize;
  UINT8                *BpmHash;
  UINTN                ContextSize;
  VOID                 *HashContext;
  BOOLEAN              HashResult;
  UINT8                Digest[SHA256_DIGEST_SIZE];


  ZeroMem (Digest, SHA256_DIGEST_SIZE);
  ContextSize = Sha256GetContextSize ();
  HashContext = AllocatePool (ContextSize);

  DEBUG((EFI_D_INFO, "VerifyFV: PayloadPtr = %x, DataSize = %x\n", PayloadPtr, DataSize));

  switch (PayloadSel) {
    case HashIbbl:
      BpmHashAlgo = BpmPtr->IbblHashAlgo;
      BpmHashSize = BpmPtr->IbblHashSize;
      BpmHash     = BpmPtr->IbblHash;
      break;

    case HashIbbm:
      BpmHashAlgo = BpmPtr->IbbmHashAlgo;
      BpmHashSize = BpmPtr->IbbmHashSize;
      BpmHash     = BpmPtr->IbbmHash;
      break;

    case HashObb:
    default:
      BpmHashAlgo = BpmPtr->ObbHashAlgo;
      BpmHashSize = BpmPtr->ObbHashSize;
      BpmHash     = BpmPtr->ObbHash;
      break;
  }

  //Currently only support SHA256
  if (BpmHashAlgo != 2 || BpmHashSize != SHA256_DIGEST_SIZE) {
    DEBUG((EFI_D_ERROR, "Hash length NOT correct for SHA256.\n"));
    return EFI_INVALID_PARAMETER;
  }

  //If hash enty in BPM is empty (all 0s), skip the OBB verification
  if (!CompareMem(BpmHash, Digest, SHA256_DIGEST_SIZE)) {
    return EFI_SUCCESS;
  }

  DEBUG((EFI_D_INFO, "BPM Info:\n" ));
  DEBUG((EFI_D_INFO, " Payload = %d, HashAlgo = %d, HashSize = %d\n", PayloadSel, BpmHashAlgo, BpmHashSize));
  DEBUG((EFI_D_INFO, " Hash ="));
  for (Index = 0; Index < BpmHashSize; Index++) {
    DEBUG((EFI_D_INFO, " %02x", BpmHash[Index]));
  }
  DEBUG((EFI_D_INFO, "\n"));

  HashResult = Sha256Init (HashContext);
  if (!HashResult) {
    DEBUG((EFI_D_ERROR, "Sha256Init Failed!\n"));
    return EFI_ABORTED;
  }
  HashResult = Sha256Update (HashContext, PayloadPtr, DataSize);
  if (!HashResult) {
    DEBUG((EFI_D_ERROR, "Sha256Update Failed!\n"));
    return EFI_ABORTED;
  }
  HashResult = Sha256Final (HashContext, Digest);
  if (!HashResult) {
    DEBUG((EFI_D_ERROR, "Sha256Final Failed!\n"));
    return EFI_ABORTED;
  }

  DEBUG((EFI_D_INFO, " Hash ="));
  for (Index = 0; Index < BpmHashSize; Index ++) {
    DEBUG((EFI_D_INFO, " %02x", Digest[Index]));
  }
  DEBUG((EFI_D_INFO, "\n"));

  if (CompareMem(BpmHash, Digest, BpmHashSize)) {
    DEBUG((EFI_D_ERROR, "\nHash Mis-Match. Return Security Violation\n"));
    return EFI_SECURITY_VIOLATION;
  }

  return EFI_SUCCESS;
}