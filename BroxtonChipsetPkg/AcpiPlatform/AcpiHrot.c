/*++

Copyright (c) 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiHrot.c

Abstract:

  Hardware Root of Trust ACPI Table


--*/

#include <Protocol/SeCOperation.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/AcpiTable.h>
#include <Guid/PlatformInfo.h>
#include <Library/ScPlatformLib.h>
#include <IndustryStandard/Acpi.h>
#include <AcpiHrot.h>
//[-start-160509-IB03090427-add]//
#include <Library/HeciMsgLib.h>
#include <SeCAccess.h>
//[-end-160509-IB03090427-add]//
//[-start-180430-IB07400962-add]//
#include <Guid/GlobalVariable.h>
#include <Library/UefiRuntimeServicesTableLib.h>
//[-end-180430-IB07400962-add]//

//[-start-170516-IB08450375-modify]//
#define EFI_ACPI_HROT_SIGNATURE           SIGNATURE_32('P', 'S', 'D', 'S')  // 'PSDS'
//[-end-170516-IB08450375-modify]//
#define EFI_ACPI_HROT_TABLE_REVISION      0x1
#define EFI_ACPI_OEM_ID                   "INTEL "  // OEMID 6 bytes long
#define EFI_ACPI_OEM_TABLE_ID             SIGNATURE_64('E','D','K','2',' ',' ',' ',' ') // OEM table id 8 bytes long
#define EFI_ACPI_OEM_REVISION             0x00000005
#define EFI_ACPI_CREATOR_ID               SIGNATURE_32('I','N','T','L')
#define EFI_ACPI_CREATOR_REVISION         0x0100000D
#define EFI_ACPI_HROT_FW_VENDOR_SIZE      16
#define EFI_ACPI_HROT_FW_VENDOR           "INTEL           "   // 16 bytes long
#define EFI_ACPI_HROT_FW_DATA_LEN_SIZE    16
//[-start-170516-IB08450375-add]//
#define EFI_ACPI_HROT_FW_DATA_LEN         32
//[-end-170516-IB08450375-add]//
#define MAX_DIGEST_SIZE                   64

#pragma pack(1)

//
// HROT Structure
//
//[-start-170516-IB08450375-modify]//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER       Header;
  UINT32                            Capabilities;
  SEC_VERSION_INFO                  FwVer;
  UINT8                             FwVendor[EFI_ACPI_HROT_FW_VENDOR_SIZE];
  UINT8                             SecBootEnabled;
  UINT8                             MeasuredBootEnabled;
  UINT8                             HwrotType;
  UINT8                             FwHashIndex;
  UINT8                             FwHashDataLen;
  UINT8                             FwHashData;
} EFI_ACPI_HROT_TABLE;
//[-end-170516-IB08450375-modify]//

#pragma pack()

SEC_INFOMATION                       SeCInfo;


//[-start-160509-IB03090427-add]//
UINT32
GetSeCOpMode()
{
  HECI_FWS_REGISTER                   SeCFirmwareStatus;

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  DEBUG ((EFI_D_INFO, "R_SEC_FW_STS0 is %08x %x\n", SeCFirmwareStatus.ul, SeCFirmwareStatus.r.SeCOperationMode));
  return SeCFirmwareStatus.r.SeCOperationMode;
}

EFI_STATUS
GetSeCFwVersion (
  SEC_VERSION_INFO *SeCVersion
  )
{
  EFI_STATUS            Status;
  GEN_GET_FW_VER_ACK    MsgGenGetFwVersionAckData;

  Status = HeciGetFwVersionMsg(&MsgGenGetFwVersionAckData);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  SeCVersion->CodeMajor = MsgGenGetFwVersionAckData.Data.CodeMajor;
  SeCVersion->CodeMinor = MsgGenGetFwVersionAckData.Data.CodeMinor;
  SeCVersion->CodeHotFix = MsgGenGetFwVersionAckData.Data.CodeHotFix;
  SeCVersion->CodeBuildNo = MsgGenGetFwVersionAckData.Data.CodeBuildNo;
  return EFI_SUCCESS;
}

EFI_STATUS
GetSeCCapability (
  UINT32      *SeCCapability
  )
{
  EFI_STATUS               Status;
  GEN_GET_FW_CAPSKU        MsgGenGetFwCapsSku;
  GEN_GET_FW_CAPS_SKU_ACK  MsgGenGetFwCapsSkuAck;

  Status = HeciGetFwCapsSkuMsg (&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  *SeCCapability = MsgGenGetFwCapsSkuAck.Data.FWCapSku.Data;
  return EFI_SUCCESS;
}
//[-end-160509-IB03090427-add]//

/**
  Process Boot Partition Data Table Header

  @param[in]  DataInBuffer            - Memory address pointer of FW binary file
  @param[in]  BpdtDescType            - Type of desired BPDT
  @param[out] DataOffset              - Return offset address of desired BPDT
  @param[out] DataSize                - Return size of desired BPDT

  @retval EFI_SUCCESS                 - Find desired BPDT offset
  @retval EFI_INVALID_PARAMETER       - DataInBuffer is NULL
  @retval EFI_UNSUPPORTED             - BPDT header pre verification failed
**/
EFI_STATUS
EFIAPI
ProcessBpdtHdr (
  IN  UINT8                    *DataInBuffer,
  IN  UINT32                   BpdtDescType,
  OUT UINT32                   *DataOffset,
  OUT UINT32                   *DataSize
  )
{
  EFI_STATUS                            Status;
  BPDT_HEADER                           *BpdtHeader;
  UINT8                                 Index;

  if (NULL == DataInBuffer) {
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }

  BpdtHeader = (BPDT_HEADER *)DataInBuffer;
  DEBUG ((EFI_D_INFO, "Signature = 0x%X, DscCount = 0x%X.\n", BpdtHeader->Signature, BpdtHeader->DscCount));
  if ((BpdtHeader->Signature != BPDT_SIGNATURE) || (BpdtHeader->DscCount > MAX_NUM_SUB_PAR)) {
    DEBUG ((EFI_D_ERROR, "BPDT header pre verification failed.\n"));
    return EFI_UNSUPPORTED;
  }

  for (Index = 0; Index < (UINT8)BpdtHeader->DscCount; Index++) {
    DEBUG ((EFI_D_INFO, "Region Type = 0x%X, Region Offset = 0x%08X, Region Length = 0x%08X.\n", BpdtHeader->BpdtDscriptor[Index].RegionType,
      BpdtHeader->BpdtDscriptor[Index].RegOffset, BpdtHeader->BpdtDscriptor[Index].RegSize));
    if (BpdtDescType == BpdtHeader->BpdtDscriptor[Index].RegionType) {
      *DataOffset = BpdtHeader->BpdtDscriptor[Index].RegOffset;
      *DataSize = BpdtHeader->BpdtDscriptor[Index].RegSize;
      return EFI_SUCCESS;
    }
  }

  return EFI_UNSUPPORTED;
}

/**
  Get Partition Directory offset From BPDT header

  @param[in]  DataInBuffer            - Memory address pointer of FW binary file
  @param[in]  BpdtDescType            - Type of desired BPDT
  @param[in]  StrName                 - String of Sub-Partition Directory
  @param[out] DataOffset              - Return offset address of desired BPDT
  @param[out] DataLength              - Return length of desired BPDT

  @retval EFI_SUCCESS                 - Find Partition Directory offset
  @retval EFI_INVALID_PARAMETER       - DataInBuffer is NULL
  @retval EFI_UNSUPPORTED             - Sub-Partition directory header pre verification failed
**/
EFI_STATUS
EFIAPI
GetParDirOffsetFromBpdt (
  IN  UINT8                    *DataInBuffer,
  IN  UINT8                    BpdtDescType,
  IN  CHAR8                    *StrName,
  OUT UINT32                   *DataOffset,
  OUT UINT32                   *DataLength
  )
{
  EFI_STATUS                            Status;
  UINT32                                BpTypeOffset;
  UINT32                                BpTypeSize;
  DIRECTORY_HEADER                      *DirectoryHeader;
  UINT8                                 Index;
  UINT32                                ParEntryOffset;
  UINT32                                ParEntryLength;
  UINTN                                 EntryNameSize;
  UINT8                                 StrParEntryName[12];

  if (NULL == DataInBuffer) {
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }
  *DataOffset = 0;
  EntryNameSize = 12;

  Status = ProcessBpdtHdr(DataInBuffer, BpdtDescType, &BpTypeOffset, &BpTypeSize);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Get BPDT Description type error, Status = %r.\n", Status));
    return Status;
  }

  ///
  /// Sub Partition Offset is relative to BPDT offset
  ///
  DirectoryHeader = (DIRECTORY_HEADER *)((UINTN)DataInBuffer + BpTypeOffset);
  DEBUG ((EFI_D_INFO, "DirectoryHeader = 0x%X , NumOfEntries = 0x%X.\n", (UINTN)DirectoryHeader, DirectoryHeader->NumOfEntries));
  if ((DirectoryHeader->HeaderMarker != DIR_HDR_SIGNATURE) || (DirectoryHeader->NumOfEntries > MAX_NUM_SUB_PAR)) {
    DEBUG ((EFI_D_ERROR, "Sub-Partition directory header pre verification failed.\n"));
    return EFI_UNSUPPORTED;
  }

  ZeroMem(StrParEntryName, EntryNameSize);
  CopyMem(StrParEntryName, StrName, AsciiStrLen(StrName));
  for (Index = 0; Index < DirectoryHeader->NumOfEntries; Index++) {
    ParEntryOffset = DirectoryHeader->PatDirEntry[Index].EntryOffset;
    ParEntryLength = DirectoryHeader->PatDirEntry[Index].EntrySize;
    DEBUG ((EFI_D_INFO, "EntryName: %a\n", DirectoryHeader->PatDirEntry[Index].EntryName));
    if (!CompareMem(DirectoryHeader->PatDirEntry[Index].EntryName, StrParEntryName, EntryNameSize)) {
      DEBUG ((EFI_D_INFO, "Got Sub-Partition Directory: %a\n", DirectoryHeader->PatDirEntry[Index].EntryName));
      *DataOffset = BpTypeOffset + ParEntryOffset;
      *DataLength = ParEntryLength;
      return EFI_SUCCESS;
    }
  }

  return EFI_UNSUPPORTED;
}

/*++

Routine Description:
  Calculate the Address of Boot Partition 1

Arguments:
  Address             The address

Returns:

  EFI_SUCCESS         The operation completed successfully.
  EFI_DEVICE_ERROR


--*/

EFI_STATUS
EFIAPI
FindBootPartition1 (
  OUT UINT32                    *Address
  )
{
  UINT32  SecondBPFlashLinearAddress;
  UINT32  BiosAddr;

  //
  //Compute Second BP FlashLinearAddress
  //

  SecondBPFlashLinearAddress = 0x1000;

  DEBUG ((DEBUG_INFO, "SecondBPFlashLinearAddress = %X\n", SecondBPFlashLinearAddress));
  //
  // Calculate Boot partition #2 physical address
  // FLA[26:0] <= (Flash_Regionn_Limit) - (FFFF_FFFCh - bios_address)
  //
  BiosAddr = GetSpiFlashRegionLimit (BIOS_Region) + 0xFFC - SecondBPFlashLinearAddress;
  *Address = 0xFFFFFFFC - BiosAddr;

  DEBUG ((DEBUG_INFO, "system BP1 Address = %X\n", *Address));

  return EFI_SUCCESS;
}

//[-start-160506-IB07400723-remove]//
//#if BXTI_PF_ENABLE
//[-end-160506-IB07400723-remove]//
/*++
Routine Description:
  Get hash value for all Firmware

Arguments:
  FwHashData              Pointer of FW hash value stored
  FwHashIndex             Return number set of hash value
  HashDataSize            Return total size of hash value

Returns:
  EFI_SUCCESS             Success get all FW hash value
  EFI_ERROR               Unable to get hash value

--*/
//[-start-170516-IB08450375-modify]//
EFI_STATUS
EFIAPI
GetFwHashDataSize (
  OUT UINT8     *FwHashData,
  OUT UINT8     *FwHashIndex,
  OUT UINT32    *HashDataSize
)
//[-end-170516-IB08450375-modify]//
{
  EFI_STATUS                     Status;
  UINT32                         H1BpBpmHashOffset;
  UINT32                         H1BpBpmHashLength;
  VOID                           *FvBuffer;
  UINT8                          *FileBuffer = NULL;
  BPMDATA                        *BpmData;
  EFI_PLATFORM_INFO_HOB          *mPlatformInfo;
  EFI_PEI_HOB_POINTERS           Hob;

  *FwHashIndex = 0;
  *HashDataSize = 0;

  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  mPlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  Status = FindBootPartition1 ((UINT32 *) &FvBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DEBUG ((DEBUG_INFO, "SPI Boot Partition 1 at = %X\n", FvBuffer));
  FileBuffer = (UINT8*) FvBuffer;
  Status = GetParDirOffsetFromBpdt(FileBuffer, bpIBB, "BPM.met", &H1BpBpmHashOffset, &H1BpBpmHashLength);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DEBUG((EFI_D_INFO, "H1 BPM Offset: 0x%X.\n", H1BpBpmHashOffset));
  BpmData = (BPMDATA*)(FileBuffer + H1BpBpmHashOffset);

//[-start-170516-IB08450375-modify]//
  if ((mPlatformInfo->BootGuard & BIT1) > 1 == 0x1) {
    //TXE Measurement Hash
    CopyMem(FwHashData, mPlatformInfo->TxeMeasurementHash, (sizeof(mPlatformInfo->TxeMeasurementHash)));
    (*FwHashIndex)++;
    (*HashDataSize) += (sizeof(mPlatformInfo->TxeMeasurementHash));

    FwHashData += (sizeof(mPlatformInfo->TxeMeasurementHash));
  }

  if ((mPlatformInfo->BootGuard & BIT0) == 0x1) {
    //IBBL Hash
    CopyMem(FwHashData, BpmData->IbblHash, BpmData->IbblHashSize);
    (*FwHashIndex)++;
    (*HashDataSize) += BpmData->IbblHashSize;

    //IBBM Hash
    FwHashData += BpmData->IbblHashSize;

    CopyMem(FwHashData, BpmData->IbbmHash, BpmData->IbbmHashSize);
    (*FwHashIndex)++;
    (*HashDataSize) += BpmData->IbbmHashSize;

    //OBB Hash
    FwHashData += BpmData->IbbmHashSize;

    CopyMem(FwHashData, BpmData->ObbHash, BpmData->ObbHashSize);
    (*FwHashIndex)++;
    (*HashDataSize) += BpmData->ObbHashSize;
  }
//[-end-170516-IB08450375-modify]//

  return EFI_SUCCESS;
}

/*++

Routine Description:

Install Hardware Root of Trust ACPI Table

Returns:

EFI_SUCCESS           -  Installed HROT ACPI table successfully.

--*/
EFI_STATUS
EFIAPI
InstallAcpiHrot (
  )
{
//[-start-170516-IB08450375-modify]//
  EFI_ACPI_HROT_TABLE            *mHrot = NULL;
  UINTN                          AcpiTableKey;
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINT8                          *tempFwHashData = NULL;
  UINT8                          tempFwHashIndex;
  UINT32                         FwHashDataSize;
  EFI_PLATFORM_INFO_HOB          *mPlatformInfo;
  EFI_PEI_HOB_POINTERS           Hob;
//  UINTN                          VarSize;
//  SYSTEM_CONFIGURATION           SystemConfiguration;
//[-end-170516-IB08450375-modify]//
//[-start-180430-IB07400962-add]//
  UINTN                          VarSize;
  UINT8                          SecureBootEnabled;
//[-end-180430-IB07400962-add]//

  DEBUG((EFI_D_INFO, "InstallAcpiHrot() start\n"));

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, &AcpiTable);
  if (EFI_ERROR(Status)) {
    return Status;
  }

//[-start-170516-IB08450375-modify]//
  tempFwHashData = AllocateZeroPool(EFI_ACPI_HROT_FW_DATA_LEN_SIZE * MAX_DIGEST_SIZE);
  Status = GetFwHashDataSize(tempFwHashData, &tempFwHashIndex, &FwHashDataSize);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  mHrot = AllocateZeroPool(sizeof(EFI_ACPI_HROT_TABLE) - sizeof(mHrot->FwHashData) + FwHashDataSize);

  if (mHrot == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  mPlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

//
// RC query SetupConfig for secure boot. But it should be always enabled.
//
//  //
//  // Get Setup Variable
//  //
//  VarSize = sizeof (SYSTEM_CONFIGURATION);
//
//  Status = gRT->GetVariable (
//                  L"Setup",
//                  &gEfiSetupVariableGuid,
//                  NULL,
//                  &VarSize,
//                  &SystemConfiguration
//                  );
//
//  ASSERT_EFI_ERROR (Status);
//[-end-170516-IB08450375-modify]//

//[-start-180430-IB07400962-add]//
  SecureBootEnabled = 0;
  VarSize = sizeof (UINT8);
  Status = gRT->GetVariable (L"SecureBoot", &gEfiGlobalVariableGuid, NULL, &VarSize, &SecureBootEnabled);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_INFO, "GetVariable SecureBoot %r\n", Status));
  }
//[-end-180430-IB07400962-add]//

  mHrot->Header.Signature               = EFI_ACPI_HROT_SIGNATURE;
  mHrot->Header.Length                  = sizeof(EFI_ACPI_HROT_TABLE) - sizeof(mHrot->FwHashData) + FwHashDataSize;
  mHrot->Header.Revision                = EFI_ACPI_HROT_TABLE_REVISION;
  mHrot->Header.Checksum                = 0;
  CopyMem(&mHrot->Header.OemId, EFI_ACPI_OEM_ID, 6);
  mHrot->Header.OemTableId              = EFI_ACPI_OEM_TABLE_ID;
  mHrot->Header.OemRevision             = EFI_ACPI_OEM_REVISION;
  mHrot->Header.CreatorId               = EFI_ACPI_CREATOR_ID;
  mHrot->Header.CreatorRevision         = EFI_ACPI_CREATOR_REVISION;

  //Update SEC Info
//[-start-160509-IB03090427-modify]//
  Status = GetSeCCapability(&SeCInfo.SeCCapability);
  if (!EFI_ERROR(Status)) {
    mHrot-> Capabilities                  = SeCInfo.SeCCapability;
  }
  Status = GetSeCFwVersion(&SeCInfo.SeCVer);
  if (!EFI_ERROR(Status)) {
    mHrot->FwVer.CodeMajor                = SeCInfo.SeCVer.CodeMajor;
    mHrot->FwVer.CodeMinor                = SeCInfo.SeCVer.CodeMinor;
    mHrot->FwVer.CodeHotFix               = SeCInfo.SeCVer.CodeHotFix;
    mHrot->FwVer.CodeBuildNo              = SeCInfo.SeCVer.CodeBuildNo;
  }
//[-end-160509-IB03090427-modify]//
  CopyMem(&mHrot->FwVendor, EFI_ACPI_HROT_FW_VENDOR, EFI_ACPI_HROT_FW_VENDOR_SIZE);
//[-start-170516-IB08450375-modify]//
  //mHrot->SecBootEnabled                 = (UINT8) SeCInfo.SeCEnable;
  //00 - Secure boot is Disabled; 01 - UEFI Secure boot is enabled; 10 - Boot Guard 2.0 is Enabled; 11 - Boot Guard 2.0 + UEFI Secure boot is enabled
//[-start-180430-IB07400962-modify]//
  mHrot->SecBootEnabled                  = ((mPlatformInfo->BootGuard & BIT0) << 1) | (SecureBootEnabled);//SystemConfiguration.SecureBoot;
//[-end-180430-IB07400962-modify]//
  mHrot->MeasuredBootEnabled             = (mPlatformInfo->BootGuard & BIT1) >> 1;

  //0 - No HWRoT; 1 - ROM based RoT; 2 - TXE; 3 - CSE; 4 - ACM; 5 - TXT
  mHrot->HwrotType                      = 0x3;
  mHrot->FwHashIndex                    = tempFwHashIndex;
  mHrot->FwHashDataLen                  = EFI_ACPI_HROT_FW_DATA_LEN;
  CopyMem(&(mHrot->FwHashData),tempFwHashData, FwHashDataSize);
//[-end-170516-IB08450375-modify]//

  AcpiTableKey = 0;
  Status = AcpiTable->InstallAcpiTable(AcpiTable, mHrot, mHrot->Header.Length, &AcpiTableKey);

  FreePool (mHrot);
  FreePool (tempFwHashData);

  DEBUG((EFI_D_INFO, "InstallAcpiHrot() end\n"));
  return Status;
}
//[-start-160506-IB07400723-remove]//
//#endif
//[-end-160506-IB07400723-remove]//

/*++

Routine Description:

Entry point to install Hardware Root of Trust ACPI Table

Returns:

EFI_SUCCESS           -  Installed HROT ACPI table successfully.

--*/
EFI_STATUS
AcpiHrotDxeEntryPoint (
  )
{
//[-start-160509-IB03090427-modify]//
//[-start-160506-IB07400723-remove]//
//#if BXTI_PF_ENABLE
//[-end-160506-IB07400723-remove]//
  DEBUG((DEBUG_ERROR, "AcpiHrotDxe() Entry start\n"));

//[-start-170516-IB08450375-remove]//
//  SeCInfo.SeCOpMode = GetSeCOpMode();
//  SeCInfo.SeCEnable = (SeCInfo.SeCOpMode == 0) ? 1 : 0;
//
//  if(!SeCInfo.SeCEnable) {
//    DEBUG((DEBUG_ERROR, "SEC disabled\n"));
//    return EFI_UNSUPPORTED;
//  }
//[-end-170516-IB08450375-remove]//
//[-end-160509-IB03090427-modify]//

  InstallAcpiHrot();
  
  DEBUG((DEBUG_ERROR, "AcpiHrotDxe Entry end\n"));
//[-start-160506-IB07400723-remove]//
//#endif
//[-end-160506-IB07400723-remove]//
  return EFI_SUCCESS;
    
}