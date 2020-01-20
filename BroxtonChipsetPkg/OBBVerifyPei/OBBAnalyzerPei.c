/** @file
  Analyze the layout of IFWI for FOTA recovery.

@copyright
 Copyright (c) 2016 Intel Corporation. All rights reserved
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
**/

#include "OBBVerifyPei.h"


BOOLEAN
EFIAPI
VerifyFvHash (
  IN UINT8          BpmHash,
  IN UINTN          Buffer,
  IN UINTN          Size,
  IN UINTN          SignAddress
  )
{
  UINT8             HashLength;
  UINT8             Digest[SHA256_DIGEST_SIZE];
  UINTN             CtxSize;
  VOID              *HashCtx;
  BOOLEAN           HashResult;
  UINT8             Index;
  BPMDATA           *BpmData;

  ZeroMem (Digest, SHA256_DIGEST_SIZE);
  CtxSize = Sha256GetContextSize ();
  HashCtx = AllocatePool (CtxSize);

  DEBUG((EFI_D_INFO, "Init... \n"));
  HashResult = Sha256Init (HashCtx);
  if (!HashResult) {
    DEBUG((EFI_D_ERROR, "[Fail]\n"));
    return FALSE;
  }

  DEBUG((EFI_D_INFO, "Update... \n"));
  HashResult = Sha256Update (HashCtx, (VOID*)Buffer, Size);
  if (!HashResult) {
    DEBUG((EFI_D_ERROR, "[Fail]\n"));
    return FALSE;
  }

  DEBUG((EFI_D_INFO, "Finalize... \n"));
  HashResult = Sha256Final (HashCtx, Digest);
  if (!HashResult) {
    DEBUG((EFI_D_ERROR, "[Fail]\n"));
    return FALSE;
  }

  ///
  /// For BXT platform, refer to BPMDATA structure that define in "FotaCommon.h"  (IBBL | IBBM | OBB)
  /// e.g. OBB Offset: Hash start form offset + 0x0C + 0x8*3 + 0x20*2
  ///
  BpmData = (BPMDATA*)SignAddress;
  if ((BpmData->IbblHashSize != SHA256_DIGEST_SIZE) || (BpmData->IbbmHashSize != SHA256_DIGEST_SIZE)
    || (BpmData->ObbHashSize != SHA256_DIGEST_SIZE)) {
    DEBUG((EFI_D_ERROR, "Hash length NOT 32.\n"));
    return FALSE;
  }
  HashLength = SHA256_DIGEST_SIZE;
  DEBUG((EFI_D_INFO, "HashLength = %X\n", HashLength));

  for (Index = 0; Index < HashLength; Index ++) {
    DEBUG((EFI_D_INFO, "Index %02X: Digest = %02X, Sign = %02X\n", Index, Digest[Index], *(UINT8*)(UINTN)(SignAddress + 0x0C + 0x08*(BpmHash+1) + 0x20*BpmHash + Index)));
    if (Digest[Index] != *(UINT8*)(UINTN)(SignAddress + 0x0C + 0x08*(BpmHash+1) + 0x20*BpmHash + Index)) {
      DEBUG((EFI_D_ERROR, "Signed UnMatch\n"));
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

  // Sub Partition Offset is relative to BPDT offset
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

EFI_STATUS
EFIAPI
GetParDirOffset (
  IN  UINT8                    *DataInBuffer,
  IN  CHAR8                    *StrName,
  OUT UINT32                   *DataOffset,
  OUT UINT32                   *DataLength
  )
{
  EFI_STATUS                            Status;
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

  // Begin with "$CPD" signature  
  // Sub Partiton Offset is relative to "$CPD" offset
  DirectoryHeader = (DIRECTORY_HEADER *)((UINTN)DataInBuffer);
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
      *DataOffset = ParEntryOffset;
      *DataLength = ParEntryLength;
      return EFI_SUCCESS;
    }
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
FvBpmHashVerify (
  IN  UINT8                    *FileBuffer
  )
{
  EFI_STATUS                            Status;
  UINT32                                H1BpBpmHashOffset;
  UINT32                                H1BpBpmHashLength;
  UINT32                                H2BpBpmHashOffset;
  UINT32                                H2BpBpmHashLength;
  UINT8                                 HashLength;
  UINT8                                 IndexHash;
  UINT8                                 Index;

  UINT32                                Size;
  UINT32                                BpIbblHashOffset;
  UINT32                                BpIbblHashLength;
  UINTN                                 HashIbblBuffer;

  UINT32                                BpIbbmHashOffset;
  UINT32                                BpIbbmHashLength;
  UINTN                                 HashIbbmBuffer;

  UINT32                                BpSbpdtOffset;
  UINT32                                BpSbpdtSize;
  UINT32                                BpObbOffset;
  UINT32                                BpObbSize;
  UINT32                                IbbrHashOffset;
  UINT32                                IbbrHashLength;
  UINT32                                ObbHashOffset;
  UINT32                                ObbHashLength;
  UINT32                                ObbxHashOffset;
  UINT32                                ObbxHashLength; 
  UINTN                                 HashObbBuffer;
  BPMDATA                               *BpmData;

  DEBUG((EFI_D_INFO, "FvBpmHashVerify().\n"));

  Status = GetParDirOffsetFromBpdt(FileBuffer, bpIBB, "BPM.met", &H1BpBpmHashOffset, &H1BpBpmHashLength);
  DEBUG((EFI_D_INFO, "H1 BPM Offset: 0x%X.\n", H1BpBpmHashOffset));
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H1 Manifest BPM Hash Offset, Status = %r\n", Status));
    goto ErrorExit;
  }

  Status = GetParDirOffsetFromBpdt((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE), bpIBB, "BPM.met", &H2BpBpmHashOffset, &H2BpBpmHashLength);
  DEBUG((EFI_D_INFO, "H2 BPM Offset: 0x%X.\n", H2BpBpmHashOffset));
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 Manifest BPM Hash Offset, Status = %r\n", Status));
    goto ErrorExit;
  }

  //
  // Step 1: Verifying H1 and H2 Mainfest Hash
  //
  DEBUG((EFI_D_INFO, "\nVerifying H1 and H2 Mainfest Hash\n"));
  BpmData = (BPMDATA*)(FileBuffer + H1BpBpmHashOffset);
  if ((BpmData->IbblHashSize != SHA256_DIGEST_SIZE) || (BpmData->IbbmHashSize != SHA256_DIGEST_SIZE)
    || (BpmData->ObbHashSize != SHA256_DIGEST_SIZE)) {
    DEBUG((EFI_D_ERROR, "H1 Hash length NOT 32.\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }

  BpmData = (BPMDATA*)(FileBuffer + IFWI_SINGLE_SIZE + H2BpBpmHashOffset);
  if ((BpmData->IbblHashSize != SHA256_DIGEST_SIZE) || (BpmData->IbbmHashSize != SHA256_DIGEST_SIZE)
    || (BpmData->ObbHashSize != SHA256_DIGEST_SIZE)) {
    DEBUG((EFI_D_ERROR, "H2 Hash length NOT 32.\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }
  HashLength = SHA256_DIGEST_SIZE;
  DEBUG((EFI_D_INFO, "HashLength = %X\n", HashLength));

  for (IndexHash = 0; IndexHash < 3; IndexHash++) {
    for (Index = 0; Index < HashLength; Index ++) {
      DEBUG((EFI_D_INFO, "  Index %02X: H1 Sign = %02X, H2 Sign = %02X\n", Index, *(UINT8*)(FileBuffer + H1BpBpmHashOffset + 0x0C + 0x08*(IndexHash+1) + 0x20*IndexHash + Index),
        *(UINT8*)(FileBuffer + IFWI_SINGLE_SIZE + H2BpBpmHashOffset + 0x0C + 0x08*(IndexHash+1) + 0x20*IndexHash + Index)));
      if (*(UINT8*)(FileBuffer + H1BpBpmHashOffset + 0x0C + 0x08*(IndexHash+1) + 0x20*IndexHash + Index) !=
        *(UINT8*)(FileBuffer + IFWI_SINGLE_SIZE + H2BpBpmHashOffset + 0x0C + 0x08*(IndexHash+1) + 0x20*IndexHash + Index)) {
        DEBUG((EFI_D_ERROR, "Signed UnMatch\n"));
        break;
      }
    }

    if (Index != HashLength) {
      Status = EFI_CRC_ERROR;
      goto ErrorExit;
    }
  }

  //
  // Step 2: Verifying IBBL Hash
  //
  // H1 IBBL
  DEBUG((EFI_D_INFO, "\nVerifying IBBL Hash\n"));
  Status = GetParDirOffsetFromBpdt(FileBuffer, bpIBB, "IBBL", &BpIbblHashOffset, &BpIbblHashLength);
  DEBUG((EFI_D_INFO, "H1 IBBL Offset: 0x%X, Length: 0x%X.\n", BpIbblHashOffset, BpIbblHashLength));
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H1 IBBL Offset, Status = %r\n", Status));
    goto ErrorExit;
  }

  HashIbblBuffer = (UINTN)(FileBuffer + BpIbblHashOffset);
  Size = BpIbblHashLength;
  if (!VerifyFvHash(HashIbbl, HashIbblBuffer, Size, (UINTN)(FileBuffer + H1BpBpmHashOffset))) {
    DEBUG((EFI_D_INFO, "H1 IBBL Signed Match.\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }

  // H2 IBBL
  Status = GetParDirOffsetFromBpdt((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE), bpIBB, "IBBL", &BpIbblHashOffset, &BpIbblHashLength);
  DEBUG((EFI_D_INFO, "H2 IBBL Offset: 0x%X, Length: 0x%X.\n", BpIbblHashOffset, BpIbblHashLength));
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 IBBL Offset, Status = %r\n", Status));
    goto ErrorExit;
  }

  HashIbblBuffer = (UINTN)(FileBuffer + IFWI_SINGLE_SIZE + BpIbblHashOffset);
  Size = BpIbblHashLength;
  if (!VerifyFvHash(HashIbbl, HashIbblBuffer, Size, (UINTN)(FileBuffer + IFWI_SINGLE_SIZE + H2BpBpmHashOffset))) {
    DEBUG((EFI_D_ERROR, "H2 IBBL Signed not Match!\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }

  //
  // Step 3: Verifying IBBM Hash
  //
  // H1 IBBM
  DEBUG((EFI_D_INFO, "\nVerifying IBBM Hash\n"));
  Status = GetParDirOffsetFromBpdt(FileBuffer, bpIBB, "IBB", &BpIbbmHashOffset, &BpIbbmHashLength);
  DEBUG((EFI_D_INFO, "H1 IBBM Offset: 0x%X, Length: 0x%X.\n", BpIbbmHashOffset, BpIbbmHashLength));
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H1 IBBM Offset, Status = %r\n", Status));
    goto ErrorExit;
  }

  HashIbbmBuffer = (UINTN)(FileBuffer + BpIbbmHashOffset);
  Size = BpIbbmHashLength;
  if (!VerifyFvHash(HashIbbm, HashIbbmBuffer, Size, (UINTN)(FileBuffer + H1BpBpmHashOffset))) {
    DEBUG((EFI_D_ERROR, "H1 IBBM Signed not Match!\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }

  // H2 IBBM
  Status = GetParDirOffsetFromBpdt((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE), bpIBB, "IBB", &BpIbbmHashOffset, &BpIbbmHashLength);
  DEBUG((EFI_D_INFO, "H2 IBBM Offset: 0x%X, Length: 0x%X.\n", BpIbbmHashOffset, BpIbbmHashLength));
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 IBBM Offset, Status = %r\n", Status));
    goto ErrorExit;
  }

  HashIbbmBuffer = (UINTN)(FileBuffer + IFWI_SINGLE_SIZE + BpIbbmHashOffset);
  Size = BpIbbmHashLength;
  if (!VerifyFvHash(HashIbbm, HashIbbmBuffer, Size, (UINTN)(FileBuffer + IFWI_SINGLE_SIZE + H2BpBpmHashOffset))) {
    DEBUG((EFI_D_ERROR, "H2 IBBM Signed not Match!\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }

  //
  // Step 4: Verifying OBB Hash
  //
  // Get OBB from H2 SBPDT
  DEBUG((EFI_D_INFO, "\nVerifying OBB Hash\n"));
  Status = ProcessBpdtHdr((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE), bpSBpdt, &BpSbpdtOffset, &BpSbpdtSize);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read bpSBpdt region failed, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "BpSbpdtOffset = 0x%X, BpSbpdtSize = 0x%X.\n", BpSbpdtOffset, BpSbpdtSize));

  Status = ProcessBpdtHdr((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE + BpSbpdtOffset), bpObb, &BpObbOffset, &BpObbSize);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Read BpObb region failed, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "BpObbOffset = 0x%X, BpObbSize = 0x%X.\n", BpObbOffset, BpObbSize));

  //
  // Get IBBR FV info from "OBB" partition
  //
  Status = GetParDirOffset((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE + BpObbOffset), "IBBR", &IbbrHashOffset, &IbbrHashLength);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 IBBR Offset, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "IbbrHashOffset = 0x%X, Length: 0x%X.\n", IbbrHashOffset, IbbrHashLength));

  //
  // Get OBB FV info from "OBB" partition
  //
  Status = GetParDirOffset((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE + BpObbOffset), "OBB", &ObbHashOffset, &ObbHashLength);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 OBB Offset, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "ObbHashOffset = 0x%X, Length: 0x%X.\n", ObbHashOffset, ObbHashLength));

  //
  // Get OBBX FV info from "OBB" partition
  //
  Status = GetParDirOffset((UINT8*)(FileBuffer + IFWI_SINGLE_SIZE + BpObbOffset), "OBBX", &ObbxHashOffset, &ObbxHashLength);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can not find H2 OBBX Offset, Status = %r\n", Status));
    goto ErrorExit;
  }
  DEBUG((EFI_D_INFO, "ObbxHashOffset = 0x%X, Length: 0x%X.\n", ObbxHashOffset, ObbxHashLength));

  HashObbBuffer = (UINTN)(FileBuffer + IFWI_SINGLE_SIZE + BpObbOffset + IbbrHashOffset);
  Size = IbbrHashLength + ObbHashLength + ObbxHashLength;
  if (!VerifyFvHash(HashObb, HashObbBuffer, Size, (UINTN)(FileBuffer + IFWI_SINGLE_SIZE + H2BpBpmHashOffset))) {
    DEBUG((EFI_D_ERROR, "H2 OBB Signed not Match!\n"));
    Status = EFI_CRC_ERROR;
    goto ErrorExit;
  }

  Status = EFI_SUCCESS;  

ErrorExit:
  return Status;
}

/*++

Routine Description:

  Verify the microcode is supported or not

Arguments:

  FileBuffer  - The start address of the update image

Returns:

  TRUE          - Microcode is correct
  FALSE         - Microcode is not correct
  
--*/
BOOLEAN
IsCorrectMicrocode (
  IN  UINT8                    *FileBuffer,
  IN  UINT8                    McuPatchIndex
  )
{
  EFI_STATUS                               Status;
  CHAR8                                    *ParDirName;
  UINT32                                   BpPatchOffset;
  UINT32                                   BpPatchLength;
  EFI_CPU_MICROCODE_HEADER                 *MicrocodeEntryPoint;
  BOOLEAN                                  CorrectMicrocode;
  UINT32                                   ExtendedTableLength;
  EFI_CPU_MICROCODE_EXTENDED_TABLE         *ExtendedTable;
  EFI_CPU_MICROCODE_EXTENDED_TABLE_HEADER  *ExtendedTableHeader;
  UINT32                                   ExtendedTableCount;
  UINT32                                   Cpuid_RegEax;  
  UINT8                                    MsrPlatform;
  UINT8                                    Index;

  if (McuPatchIndex == 1) {
    ParDirName = "upatch1";
  } else if (McuPatchIndex == 2) {
    ParDirName = "upatch2";
  } else {
    DEBUG((EFI_D_ERROR, "Error parameter.\n"));
    CorrectMicrocode = FALSE;
    return CorrectMicrocode;
  }

  Status = GetParDirOffsetFromBpdt(FileBuffer, bpuCode, ParDirName, &BpPatchOffset, &BpPatchLength);
  DEBUG((EFI_D_INFO, "MCU patch %x Offset: 0x%X, length: 0x%X.\n", McuPatchIndex, BpPatchOffset, BpPatchLength));
  if ((EFI_ERROR(Status)) || (BpPatchLength == 0)) {
    DEBUG((EFI_D_ERROR, "Can not find MCU patch %x, Status = %r\n", McuPatchIndex, Status));
    CorrectMicrocode = FALSE;
    return CorrectMicrocode;
  }

  MicrocodeEntryPoint = (EFI_CPU_MICROCODE_HEADER*)(UINT8*)(FileBuffer + BpPatchOffset);

  AsmCpuid (EFI_CPUID_VERSION_INFO, &Cpuid_RegEax, NULL, NULL, NULL);

  //
  // The index of platform information resides in bits 50:52 of MSR IA32_PLATFORM_ID
  //
  MsrPlatform = (UINT8) (RShiftU64 ((AsmReadMsr64 (EFI_MSR_IA32_PLATFORM_ID) & B_EFI_MSR_IA32_PLATFORM_ID_PLATFORM_ID_BITS_MASK),  \
                                       N_EFI_MSR_IA32_PLATFORM_ID_PLATFORM_ID_BITS));

  CorrectMicrocode    = FALSE;
  //
  // Check if the microcode is for the Cpu and the version is newer
  // and the update can be processed on the platform
  //
  if (MicrocodeEntryPoint->HeaderVersion == 0x00000001) {
    if ((MicrocodeEntryPoint->ProcessorId == Cpuid_RegEax) &&
        (MicrocodeEntryPoint->ProcessorFlags & (1 << MsrPlatform)) ) {
      CorrectMicrocode = TRUE;
    } else {
      //
      // Check the  Extended Signature if the entended signature exist
      // Only the data size != 0 the extended signature may exist
      //
      ExtendedTableLength = MicrocodeEntryPoint->TotalSize - (MicrocodeEntryPoint->DataSize + sizeof (EFI_CPU_MICROCODE_HEADER));
      if (ExtendedTableLength != 0) {
        //
        // Extended Table exist, check if the CPU in support list
        //
        ExtendedTableHeader = (EFI_CPU_MICROCODE_EXTENDED_TABLE_HEADER *)((UINT8 *)(MicrocodeEntryPoint) + MicrocodeEntryPoint->DataSize + sizeof (EFI_CPU_MICROCODE_HEADER));
        //
        // Calulate Extended Checksum
        //
        if ((ExtendedTableLength % 4) == 0) {
          //
          // Checksum correct
          //
          ExtendedTableCount = ExtendedTableHeader->ExtendedSignatureCount;
          ExtendedTable      = (EFI_CPU_MICROCODE_EXTENDED_TABLE *)(ExtendedTableHeader + 1);
          for (Index = 0; Index < ExtendedTableCount; Index ++) {
            //
            // Verify Header
            //
            if ((ExtendedTable->ProcessorSignature == Cpuid_RegEax) &&
                (ExtendedTable->ProcessorFlag & (1 << MsrPlatform)) ) {
              //
              // Find one
              //
              CorrectMicrocode = TRUE;
              break;
            }
            ExtendedTable ++;
          }
        }
      }
    }
  }

  return CorrectMicrocode;
}
