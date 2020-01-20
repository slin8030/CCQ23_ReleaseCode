/** @file
  CPU Platform Lib implementation.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

@par Specification Reference:
**/
#include "CpuPlatformLibrary.h"
#include <Library/MmPciLib.h>
#include <SaRegs.h>

#define SKIP_MICROCODE_CHECKSUM_CHECK 1

/**
  Return CPU Family ID

  @retval CPU_FAMILY              CPU Family ID
**/
CPU_FAMILY
EFIAPI
GetCpuFamily (
  VOID
  )
{
  EFI_CPUID_REGISTER Cpuid;
  ///
  /// Read the CPUID information
  ///
  AsmCpuid (CPUID_VERSION_INFO, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  return ((CPU_FAMILY) (Cpuid.RegEax & CPUID_FULL_FAMILY_MODEL));
}

/**
  Return Cpu stepping type

  @retval UINT8                   Cpu stepping type
**/
CPU_STEPPING
EFIAPI
GetCpuStepping (
  VOID
  )
{
  EFI_CPUID_REGISTER Cpuid;
  ///
  /// Read the CPUID information
  ///
  AsmCpuid (CPUID_VERSION_INFO, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  return ((CPU_STEPPING) (Cpuid.RegEax & CPUID_FULL_STEPPING));
}


/**
  Returns the processor microcode revision of the processor installed in the system.

  @retval Processor Microcode Revision
**/
UINT32
GetCpuUcodeRevision (
  VOID
  )
{
  AsmWriteMsr64 (MSR_IA32_BIOS_SIGN_ID, 0);
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, NULL, NULL);
  return (UINT32) RShiftU64 (AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID), 32);
}

/**
  Verify the DWORD type checksum

  @param[in] ChecksumAddr  - The start address to be checkumed
  @param[in] ChecksumLen   - The length of data to be checksumed

  @retval EFI_SUCCESS           - Checksum correct
  @retval EFI_CRC_ERROR         - Checksum incorrect
**/
EFI_STATUS
Checksum32Verify (
  IN UINT32 *ChecksumAddr,
  IN UINT32 ChecksumLen
  )
{
#if SKIP_MICROCODE_CHECKSUM_CHECK
  return EFI_SUCCESS;
#else
  UINT32 Checksum;
  UINT32 Index;

  Checksum = 0;

  for (Index = 0; Index < ChecksumLen; Index++) {
    Checksum += ChecksumAddr[Index];
  }

  return (Checksum == 0) ? EFI_SUCCESS : EFI_CRC_ERROR;
#endif
}

/**
  This function checks the MCU revision to decide if BIOS needs to load
  microcode.

  @param[in] MicrocodePointer - Microcode in memory
  @param[in] Revision         - Current CPU microcode revision

  @retval EFI_SUCCESS - BIOS needs to load microcode
  @retval EFI_ABORTED - Don't need to update microcode
**/
EFI_STATUS
CheckMcuRevision (
  IN CPU_MICROCODE_HEADER *MicrocodePointer,
  IN UINT32               Revision
  )
{
  EFI_STATUS Status;
  Status = EFI_ABORTED;

  if ((MicrocodePointer->UpdateRevision & 0x80000000) ||
     (MicrocodePointer->UpdateRevision > Revision) ||
     (Revision == 0)){
     Status = EFI_SUCCESS;
  }

  return Status;
}

/**
  Check if this microcode is correct one for processor

  @param[in] Cpuid               - processor CPUID
  @param[in] MicrocodeEntryPoint - entry point of microcode
  @param[in] Revision            - revision of microcode

  @retval CorrectMicrocode if this microcode is correct
**/
BOOLEAN
CheckMicrocode (
  IN UINT32               Cpuid,
  IN CPU_MICROCODE_HEADER *MicrocodeEntryPoint,
  IN UINT32               *Revision
  )
{
  EFI_STATUS                          Status;
  UINT8                               ExtendedIndex;
  UINT8                               MsrPlatform;
  UINT32                              ExtendedTableLength;
  UINT32                              ExtendedTableCount;
  BOOLEAN                             CorrectMicrocode;
  CPU_MICROCODE_EXTENDED_TABLE        *ExtendedTable;
  CPU_MICROCODE_EXTENDED_TABLE_HEADER *ExtendedTableHeader;

  Status              = EFI_NOT_FOUND;
  ExtendedTableLength = 0;
  CorrectMicrocode    = FALSE;

  if (MicrocodeEntryPoint == NULL) {
    return FALSE;
  }

  ///
  /// The index of platform information resides in bits 50:52 of MSR IA32_PLATFORM_ID
  ///
  MsrPlatform = (UINT8) (RShiftU64 (AsmReadMsr64 (MSR_IA32_PLATFORM_ID), N_PLATFORM_ID_SHIFT) & B_PLATFORM_ID_MASK);

  ///
  /// Check if the microcode is for the Cpu and the version is newer
  /// and the update can be processed on the platform
  ///
  if ((MicrocodeEntryPoint->HeaderVersion == 0x00000001) &&
      !EFI_ERROR (CheckMcuRevision (MicrocodeEntryPoint, *Revision))
      ) {
    if ((MicrocodeEntryPoint->ProcessorId == Cpuid) && (MicrocodeEntryPoint->ProcessorFlags & (1 << MsrPlatform))) {
      if (MicrocodeEntryPoint->DataSize == 0) {
        Status = Checksum32Verify ((UINT32 *) MicrocodeEntryPoint, 2048 / sizeof (UINT32));
      } else {
        Status = Checksum32Verify (
                        (UINT32 *) MicrocodeEntryPoint,
                        (MicrocodeEntryPoint->DataSize + sizeof (CPU_MICROCODE_HEADER)) / sizeof (UINT32)
                        );
      }

      if (!EFI_ERROR (Status)) {
        CorrectMicrocode = TRUE;
      }
    } else if ((MicrocodeEntryPoint->DataSize != 0)) {
      ///
      /// Check the  Extended Signature if the entended signature exist
      /// Only the data size != 0 the extended signature may exist
      ///
      ExtendedTableLength = MicrocodeEntryPoint->TotalSize - (MicrocodeEntryPoint->DataSize + sizeof (CPU_MICROCODE_HEADER));
      if (ExtendedTableLength != 0) {
        ///
        /// Extended Table exist, check if the CPU in support list
        ///
        ExtendedTableHeader = (CPU_MICROCODE_EXTENDED_TABLE_HEADER *) ((UINT8 *) (MicrocodeEntryPoint) + MicrocodeEntryPoint->DataSize + 48);
        ///
        /// Calulate Extended Checksum
        ///
        if ((ExtendedTableLength % 4) == 0) {
          Status = Checksum32Verify ((UINT32 *) ExtendedTableHeader, ExtendedTableLength / sizeof (UINT32));
          if (!EFI_ERROR (Status)) {
            ///
            /// Checksum correct
            ///
            ExtendedTableCount  = ExtendedTableHeader->ExtendedSignatureCount;
            ExtendedTable       = (CPU_MICROCODE_EXTENDED_TABLE *) (ExtendedTableHeader + 1);
            for (ExtendedIndex = 0; ExtendedIndex < ExtendedTableCount; ExtendedIndex++) {
              ///
              /// Verify Header
              ///
              if ((ExtendedTable->ProcessorSignature == Cpuid) && (ExtendedTable->ProcessorFlag & (1 << MsrPlatform))) {
                Status = Checksum32Verify (
                                (UINT32 *) ExtendedTable,
                                sizeof (CPU_MICROCODE_EXTENDED_TABLE) / sizeof (UINT32)
                                );
                if (!EFI_ERROR (Status)) {
                  ///
                  /// Find one
                  ///
                  CorrectMicrocode = TRUE;
                  break;
                }
              }

              ExtendedTable++;
            }
          }
        }
      }
    }
  }

  return CorrectMicrocode;
}


/**
  This function is to program Trace Hub ACPI base address to processor's MSR NPK_STH_ACPIBAR_BASE.

  @param[in]  TraceHubAcpiBase - Base address of Trace Hub ACPI Base address
**/
VOID
EFIAPI
CpuWriteTraceHubAcpiBase (
  IN UINT64  TraceHubAcpiBase
  )
{
  ///
  /// Masked MSR NPK_STH_ACPIBAR_BASE[17:0]
  ///
  TraceHubAcpiBase &= ~V_MSR_NPK_STH_ACPIBAR_BASE_MASK;

  ///
  /// Set MSR NPK_STH_ACPIBAR_BASE[0] LOCK bit for the AET packets to be directed to NPK MMIO.
  ///
  AsmWriteMsr64 (MSR_NPK_STH_ACPIBAR_BASE, TraceHubAcpiBase | B_MSR_NPK_STH_ACPIBAR_BASE_LOCK);
}

