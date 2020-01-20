/** @file
  SMBIOS Processor table (Type 4).

@copyright
 Copyright (c) 2013 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification Reference:
  System Management BIOS (SMBIOS) Reference Specification v2.8.0
  dated 2013-Mar-28 (DSP0134)
  http://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.8.0.pdf
**/

#include "SmbiosCpu.h"
#include <Library/PreSiliconLib.h>

///
/// Processor Information (Type 4)
///
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TABLE_TYPE4 SmbiosTableType4Data = {
  { EFI_SMBIOS_TYPE_PROCESSOR_INFORMATION, sizeof (SMBIOS_TABLE_TYPE4), 0 },
  TO_BE_FILLED,             ///< Socket
  CentralProcessor,         ///< ProcessorType
  TO_BE_FILLED,             ///< ProcessorFamily
  TO_BE_FILLED,             ///< ProcessorManufacture
  { TO_BE_FILLED },         ///< ProcessorId
  TO_BE_FILLED,             ///< ProcessorVersion
  { TO_BE_FILLED },         ///< Voltage
  100,                      ///< ExternalClock
  TO_BE_FILLED,             ///< MaxSpeed
  TO_BE_FILLED,             ///< CurrentSpeed
  TO_BE_FILLED,             ///< Status
  TO_BE_FILLED,             ///< ProcessorUpgrade
  TO_BE_FILLED,             ///< L1CacheHandle
  TO_BE_FILLED,             ///< L2CacheHandle
  TO_BE_FILLED,             ///< L3CacheHandle
  TO_BE_FILLED,             ///< SerialNumber
  TO_BE_FILLED,             ///< AssetTag
  TO_BE_FILLED,             ///< PartNumber
  TO_BE_FILLED,             ///< CoreCount
  TO_BE_FILLED,             ///< EnabledCoreCount
  TO_BE_FILLED,             ///< ThreadCount
  TO_BE_FILLED,             ///< ProcessorCharacteristics
  TO_BE_FILLED,             ///< ProcessorFamily2
};
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TYPE4_STRING_ARRAY SmbiosTableType4Strings = {
  TO_BE_FILLED_STRING,      ///< Socket
  INTEL_CORPORATION_STRING, ///< ProcessorManufacture
  TO_BE_FILLED_STRING,      ///< ProcessorVersion
  TO_BE_FILLED_STRING,      ///< SerialNumber
  TO_BE_FILLED_STRING,      ///< AssetTag
  TO_BE_FILLED_STRING,      ///< PartNumber
};

typedef struct _PROCESSOR_FAMILY_FIELD {
  CHAR8  ProcessorFamily[48];
  UINT16 ProcessorEnum;
} PROCESSOR_FAMILY_FIELD;

GLOBAL_REMOVE_IF_UNREFERENCED PROCESSOR_FAMILY_FIELD ProcessorFamilyField[] = {
  { "Intel(R) Core(TM) i7",  ProcessorFamilyIntelCoreI7 },
  { "Intel(R) Core(TM) i5",  ProcessorFamilyIntelCoreI5 },
  { "Intel(R) Core(TM) i3",  ProcessorFamilyIntelCoreI3 },
  { "Intel(R) Core(TM) M",   0x2C },                       ///< This will be part of the future SMBIOS specification. @todo Use enum from MdePkg when "ProcessorFamilyIntelCoreM" is added.
  { "Intel(R) Pentium(R)",   ProcessorFamilyPentium },
  { "Intel(R) Celeron(R)",   ProcessorFamilyCeleron },
  { "Intel(R) Atom(TM)",     ProcessorFamilyIntelAtom },
  { "Intel(R) Xeon(R)",      ProcessorFamilyIntelXeon },
};


/**
  Assigns the next string number in sequence, or 0 if string is null or empty.

  @param[in]      String       - The string pointer.
  @param[in, out] StringNumber - Pointer to the prior string number in sequence.

  @retval StringNumber         - The next string number in sequence, or 0 if string is null or empty.
**/
UINT8
AssignStringNumber (
  IN     CHAR8 *String,
  IN OUT UINT8 *StringNumber
  )
{
  if ((String == NULL) || (*String == '\0')) {
    return NO_STRING_AVAILABLE;
  } else {
    *StringNumber = *StringNumber + 1;
    return *StringNumber;
  }
}

///
/// Processor-specific routines
///
/**
  Returns the processor family and processor version of the processor installed in the system.

  @param[out] Processor Version - Returns the procesor version string installed in the system

  @retval                       - The Processor Family enum value
**/
UINT16
GetProcessorFamilyAndVersion (
  OUT CHAR8 **ProcessorVersion
  )
{
  UINTN              Index;
  EFI_CPUID_REGISTER CpuExtendedSupport;
  EFI_CPUID_REGISTER CpuBrandString;
  UINT16             ProcessorFamily;
//[-start-151211-IB08450332-add]//
  STATIC CHAR8       GenericCPU[]="ApolloLake SOC";
//[-end-151211-IB08450332-add]//

  ProcessorFamily = ProcessorFamilyOther;

  ///
  /// Get Brand string
  ///
  AsmCpuid (
          CPUID_EXTENDED_FUNCTION,
          &CpuExtendedSupport.RegEax,
          &CpuExtendedSupport.RegEbx,
          &CpuExtendedSupport.RegEcx,
          &CpuExtendedSupport.RegEdx
          );

  ///
  /// Check if Brand ID String is supported as per SDM Vol. 2A Chapter 3 Instruction Set Reference - CPUID
  ///
  if (CpuExtendedSupport.RegEax >= 0x80000004) {
    AsmCpuid (
            CPUID_BRAND_STRING1,
            &CpuBrandString.RegEax,
            &CpuBrandString.RegEbx,
            &CpuBrandString.RegEcx,
            &CpuBrandString.RegEdx
            );
//[-start-151211-IB08450332-modify]//
//[-start-160115-IB08450337-modify]//
    *(((UINT32 *)*ProcessorVersion)+0)=CpuBrandString.RegEax;
    *(((UINT32 *)*ProcessorVersion)+1)=CpuBrandString.RegEbx;
    *(((UINT32 *)*ProcessorVersion)+2)=CpuBrandString.RegEcx;
    *(((UINT32 *)*ProcessorVersion)+3)=CpuBrandString.RegEdx;
//[-end-160115-IB08450337-modify]//
//[-end-151211-IB08450332-modify]//

    AsmCpuid (
            CPUID_BRAND_STRING2,
            &CpuBrandString.RegEax,
            &CpuBrandString.RegEbx,
            &CpuBrandString.RegEcx,
            &CpuBrandString.RegEdx
            );
//[-start-151211-IB08450332-modify]//
//[-start-160115-IB08450337-modify]//
    *(((UINT32 *)*ProcessorVersion)+4)=CpuBrandString.RegEax;
    *(((UINT32 *)*ProcessorVersion)+5)=CpuBrandString.RegEbx;
    *(((UINT32 *)*ProcessorVersion)+6)=CpuBrandString.RegEcx;
    *(((UINT32 *)*ProcessorVersion)+7)=CpuBrandString.RegEdx;
//[-end-160115-IB08450337-modify]//
//[-end-151211-IB08450332-modify]//

    AsmCpuid (
            CPUID_BRAND_STRING3,
            &CpuBrandString.RegEax,
            &CpuBrandString.RegEbx,
            &CpuBrandString.RegEcx,
            &CpuBrandString.RegEdx
            );
//[-start-151211-IB08450332-modify]//
//[-start-160115-IB08450337-modify]//
    *(((UINT32 *)*ProcessorVersion)+8)=CpuBrandString.RegEax;
    *(((UINT32 *)*ProcessorVersion)+9)=CpuBrandString.RegEbx;
    *(((UINT32 *)*ProcessorVersion)+10)=CpuBrandString.RegEcx;
    *(((UINT32 *)*ProcessorVersion)+11)=CpuBrandString.RegEdx;
//[-end-160115-IB08450337-modify]//
//[-end-151211-IB08450332-modify]//

    *(*ProcessorVersion + MAXIMUM_CPU_BRAND_STRING_LENGTH) = 0;

    ///
    /// Remove preceeding spaces
    ///
    while (**ProcessorVersion == 0x20) {
      (*ProcessorVersion)++;
    }

//[-start-151211-IB08450332-add]//
    //
    // If the CPU brand string is NULL string, assign a default name.
    //
    if (**ProcessorVersion == 0) {
      *ProcessorVersion = GenericCPU;
    }
//[-end-151211-IB08450332-add]//

  } else {
    *ProcessorVersion = NULL;
    return ProcessorFamily;
  }

  for (Index = 0; Index < sizeof (ProcessorFamilyField) / sizeof (PROCESSOR_FAMILY_FIELD); Index++) {
    if (AsciiStrnCmp (
                *ProcessorVersion,
                ProcessorFamilyField[Index].ProcessorFamily,
                (AsciiStrLen (ProcessorFamilyField[Index].ProcessorFamily))
                ) == 0) {
      ProcessorFamily = ProcessorFamilyField[Index].ProcessorEnum;
      break;
    }
  }
  return ProcessorFamily;
}

/**
  Returns the processor voltage of the processor installed in the system.

  @retval Processor Voltage
**/
UINT8
GetProcessorVoltage (
  VOID
  )
{
  UINT8  SmbiosVoltage;
  UINT64 MsrValue;

  SmbiosVoltage = 0;
  MsrValue = 0;

  ///
  /// Core voltage = (float) IA32_PERF_STS(47:32) * (float) 1/(2^13)
  ///
  MsrValue = AsmReadMsr64 (MSR_IA32_PERF_STS);
  MsrValue = RShiftU64(MsrValue, 32);
  MsrValue &= 0x0FFFF;

  ///
  /// Convert to Volts multiplied by 10 for SMBIOS data, and set bit 7. See spec for details.
  ///
  MsrValue = MultU64x32 (MsrValue, 10);
  MsrValue = RShiftU64 (MsrValue, 13);
  SmbiosVoltage = (UINT8) (MsrValue | BIT7);
  return SmbiosVoltage;
}

/**
  Get processor status for socket base only

  @retval PROCESSOR_STATUS_DATA for socket base only
**/
UINT8
GetProcessorStatus (
  VOID
  )
{
  PROCESSOR_STATUS_DATA ProcessorStatus;

  ProcessorStatus.Reserved1 = 0;
  ProcessorStatus.Reserved2 = 0;
  ProcessorStatus.SocketPopulated = TRUE;

  ///
  /// For BSP, processor is always enabled.
  ///
  ProcessorStatus.CpuStatus = CpuStatusEnabled;

  return *(UINT8 *) &ProcessorStatus;
}

/**
  Returns the processor characteristics based on the processor's capabilities.

  @retval Processor Characteristics
**/
UINT16
GetProcessorCharacteristics (
  VOID
  )
{
  UINT16             ProcessorCharacteristics;
  EFI_CPUID_REGISTER CpuidRegs;
  UINT8              ThreadsPerCore;

  ProcessorCharacteristics = 0;

  ///
  /// 64-Bit Capable
  ///
  AsmCpuid (CPUID_EXTENDED_FUNCTION, &CpuidRegs.RegEax, NULL, NULL, NULL);
  if (CpuidRegs.RegEax > CPUID_EXTENDED_FUNCTION) {
    AsmCpuid (CPUID_EXTENDED_CPU_SIG, NULL, NULL, NULL, &CpuidRegs.RegEdx);
    //
    // Check Intel 64-bit capable (EDX[29])
    //
    if ((CpuidRegs.RegEdx & BIT29) != 0) {
      ProcessorCharacteristics |= SMBIOS_TYPE4_64BIT_CAPABLE;
    }
  }

  ///
  /// Multi-Core and HT capabilities
  ///
  AsmCpuidEx (CPUID_CORE_TOPOLOGY, 0, NULL, &CpuidRegs.RegEbx, NULL, NULL);
  ThreadsPerCore = (UINT8)CpuidRegs.RegEbx;

  AsmCpuidEx (CPUID_CORE_TOPOLOGY, 1, NULL, &CpuidRegs.RegEbx, NULL, NULL); 
  if ((CpuidRegs.RegEbx / ThreadsPerCore) > 1) {
    ProcessorCharacteristics |= SMBIOS_TYPE4_MULTI_CORE;
  }

  if (ThreadsPerCore > 1) {
    ProcessorCharacteristics |= SMBIOS_TYPE4_HARDWARE_THREAD;
  }

  ///
  /// Execute Protection
  ///
  AsmCpuid (CPUID_EXTENDED_FUNCTION, &CpuidRegs.RegEax, NULL, NULL, NULL);
  if (CpuidRegs.RegEax > CPUID_EXTENDED_FUNCTION) {
    AsmCpuid (CPUID_EXTENDED_CPU_SIG, NULL, NULL, NULL, &CpuidRegs.RegEdx);
    if ((CpuidRegs.RegEdx & B_CPUID_VERSION_INFO_EDX_XD) != 0) {
      ProcessorCharacteristics |= SMBIOS_TYPE4_EXECUTE_PROTECTION;
    }
  }

  ///
  /// Enhanced Virtualization
  ///
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, &CpuidRegs.RegEcx, NULL);
  if ((CpuidRegs.RegEcx & B_CPUID_VERSION_INFO_ECX_VME) != 0) {
    ProcessorCharacteristics |= SMBIOS_TYPE4_ENHANCED_VIRTUALIZATION;
  }

  ///
  /// Power/Performance Control
  ///
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, &CpuidRegs.RegEcx, NULL);
  if ((CpuidRegs.RegEcx & B_CPUID_VERSION_INFO_ECX_EIST) != 0) {
    ProcessorCharacteristics |= SMBIOS_TYPE4_POWER_PERF_CONTROL;
  }

  return ProcessorCharacteristics;
}

/**
  Add an SMBIOS table entry using EFI_SMBIOS_PROTOCOL.
  Create the full table record using the formatted section plus each non-null string, plus the terminating (double) null.

  @param[in] Entry            - The data for the fixed portion of the SMBIOS entry. The format of the data is
                                determined by EFI_SMBIOS_TABLE_HEADER.Type. The size of the formatted area is defined
                                by EFI_SMBIOS_TABLE_HEADER.Length and either followed by a double-null (0x0000) or
                                a set of null terminated strings and a null.
  @param[in] TableStrings     - Set of string pointers to append onto the full record.
                                If TableStrings is null, no strings are appended. Null strings are skipped.
  @param[in] NumberOfStrings  - Number of TableStrings to append, null strings are skipped.
  @param[in] SmbiosHandle     - A unique handle will be assigned to the SMBIOS record.

  @retval EFI_SUCCESS           Table was added.
  @retval EFI_OUT_OF_RESOURCES  Table was not added due to lack of system resources.
**/
EFI_STATUS
AddSmbiosTableEntry (
  IN  EFI_SMBIOS_TABLE_HEADER *Entry,
  IN  CHAR8                   **TableStrings,
  IN  UINT8                   NumberOfStrings,
  OUT EFI_SMBIOS_HANDLE       *SmbiosHandle
  )
{
  EFI_STATUS              Status;
  EFI_SMBIOS_TABLE_HEADER *Record;
  CHAR8                   *StringPtr;
  UINTN                   Size;
  UINTN                   Index;

  ///
  /// Calculate the total size of the full record
  ///
  Size = Entry->Length;

  ///
  /// Add the size of each non-null string
  ///
  if (TableStrings != NULL) {
    for (Index = 0; Index < NumberOfStrings; Index++) {
      if ((TableStrings[Index] != NULL) && (*TableStrings[Index] != '\0')) {
        Size += AsciiStrSize (TableStrings[Index]);
      }
    }
  }

  ///
  /// Add the size of the terminating double null
  /// If there were any strings added, just add the second null
  ///
  if (Size == Entry->Length) {
    Size += 2;
  } else {
    Size += 1;
  }

  ///
  /// Initialize the full record
  ///
  Record = (EFI_SMBIOS_TABLE_HEADER *) AllocateZeroPool(Size);
  if (Record == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Record, Entry, Entry->Length);

  ///
  /// Copy the strings to the end of the record
  ///
  StringPtr = ((CHAR8 *)Record) + Entry->Length;
  if (TableStrings != NULL) {
    for (Index = 0; Index < NumberOfStrings; Index++) {
      if ((TableStrings[Index] != NULL) && (*TableStrings[Index] != '\0')) {
        AsciiStrCpy (StringPtr, TableStrings[Index]);
        StringPtr += AsciiStrSize (TableStrings[Index]);
      }
    }
  }

  *SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = mSmbios->Add (mSmbios, NULL, SmbiosHandle, Record);

  FreePool (Record);
  return Status;
}

/**
  This function adds a SMBIOS type 4 (processor) entry.

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_NOT_FOUND         - if the Hii PackageList could not be found.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
InstallSmbiosType4 (
  VOID
  )
{
  EFI_STATUS         Status;
  VOID               *Hob;
  SMBIOS_SOCKET_INFO *SmbiosSocketInfo;
  CHAR8              BrandIdString[MAXIMUM_CPU_BRAND_STRING_LENGTH + 1];
  UINT64             MsrValue;
  UINT64             ProcessorCoreCount;
  UINT64             ProcessorThreadCount;
  EFI_SMBIOS_HANDLE  SmbiosHandle;
  UINT8              StringNumber;
  CPU_INIT_DATA_HOB  *CpuInitDataHob;
  CPU_CONFIG         *CpuConfig;
//[-start-161111-IB07400811-add]//
  EFI_CPUID_REGISTER CpuidRegs;
  UINT8              MaxCoreNumber;
  UINT32             Ecx;
//[-end-161111-IB07400811-add]//

  DEBUG ((DEBUG_INFO, "InstallSmbiosType4() - Start\n"));

  StringNumber = 0;

  ///
  /// Get CPU Init Data Hob
  ///
  Hob = GetFirstGuidHob (&gCpuInitDataHobGuid);
  if (Hob == NULL) {
    DEBUG ((DEBUG_ERROR, "CPU Data HOB not available, skipping SMBIOS table Type 4."));
    return EFI_NOT_FOUND;
  }
  CpuInitDataHob = (CPU_INIT_DATA_HOB *) ((UINTN) Hob + sizeof (EFI_HOB_GUID_TYPE));
  CpuConfig = (CPU_CONFIG *) (UINTN) CpuInitDataHob->CpuConfig;
  SmbiosSocketInfo = (SMBIOS_SOCKET_INFO *) (UINTN) CpuConfig->SmbiosSocketInfo;

  SmbiosTableType4Strings.Socket = (CHAR8 *) PcdGetPtr (PcdSmbiosDefaultSocketDesignation);
  SmbiosTableType4Data.Socket = AssignStringNumber(SmbiosTableType4Strings.Socket, &StringNumber);

  SmbiosTableType4Strings.ProcessorVersion = (CHAR8 *) &BrandIdString;

  SmbiosTableType4Data.ProcessorFamily2 = GetProcessorFamilyAndVersion (&SmbiosTableType4Strings.ProcessorVersion);
  if (SmbiosTableType4Data.ProcessorFamily2 > ProcessorFamilyIndicatorFamily2) {
    SmbiosTableType4Data.ProcessorFamily = ProcessorFamilyIndicatorFamily2;
  } else {
    SmbiosTableType4Data.ProcessorFamily = (UINT8) SmbiosTableType4Data.ProcessorFamily2;
  }
  SmbiosTableType4Data.ProcessorManufacture = AssignStringNumber(SmbiosTableType4Strings.ProcessorManufacture, &StringNumber);
  if (SmbiosTableType4Strings.ProcessorVersion == NULL) {
    SmbiosTableType4Strings.ProcessorVersion = (CHAR8 *) &BRAND_STRING_UNSUPPORTED;
  }
  SmbiosTableType4Data.ProcessorVersion = AssignStringNumber(SmbiosTableType4Strings.ProcessorVersion, &StringNumber);

  AsmCpuid (
    CPUID_VERSION_INFO,
    (UINT32 *) &SmbiosTableType4Data.ProcessorId.Signature,
    NULL,
    NULL,
    (UINT32 *) &SmbiosTableType4Data.ProcessorId.FeatureFlags
    );

  *(UINT8 *) &SmbiosTableType4Data.Voltage = GetProcessorVoltage ();
  SmbiosTableType4Data.MaxSpeed =(UINT16) (100 * (((UINT32) AsmReadMsr64 (MSR_PLATFORM_INFO) >> N_PLATFORM_INFO_MAX_RATIO) & B_PLATFORM_INFO_RATIO_MASK));
  SmbiosTableType4Data.CurrentSpeed =
                         (UINT16)(100 *
                                   (
                                     ((UINT32) AsmReadMsr64 (MSR_IA32_PERF_STS) >> N_IA32_PERF_STSP_STATE_TARGET)
                                     & B_IA32_PERF_STSP_STATE_MASK
                                   )
                                 );
  SmbiosTableType4Data.Status = GetProcessorStatus ();
  SmbiosTableType4Data.ProcessorUpgrade = ProcessorUpgradeOther;
  SmbiosTableType4Data.L1CacheHandle = mSmbiosL1CacheHandle;
  SmbiosTableType4Data.L2CacheHandle = mSmbiosL2CacheHandle;
  SmbiosTableType4Data.L3CacheHandle = mSmbiosL3CacheHandle;
  SmbiosTableType4Strings.SerialNumber = (CHAR8 *) PcdGetPtr (PcdSmbiosDefaultSerialNumber);
  SmbiosTableType4Data.SerialNumber = AssignStringNumber(SmbiosTableType4Strings.SerialNumber, &StringNumber);
  SmbiosTableType4Strings.AssetTag = (CHAR8 *) PcdGetPtr (PcdSmbiosDefaultAssetTag);
  SmbiosTableType4Data.AssetTag = AssignStringNumber(SmbiosTableType4Strings.AssetTag, &StringNumber);
  SmbiosTableType4Strings.PartNumber = (CHAR8 *) PcdGetPtr (PcdSmbiosDefaultPartNumber);
  SmbiosTableType4Data.PartNumber = AssignStringNumber(SmbiosTableType4Strings.PartNumber, &StringNumber);
  MsrValue = AsmReadMsr64 (MSR_CORE_THREAD_COUNT);
  if (PLATFORM_ID != VALUE_REAL_PLATFORM) { // pre-silicon start
  	if (MsrValue == 0) {  //msr 35h is 0 force set to one core one thread;
  		MsrValue = 0x10001;
    }
    else {
      MsrValue |=  LShiftU64(MsrValue, N_CORE_COUNT_OFFSET);
    }
  }
  ProcessorThreadCount = MsrValue & 0xffff;
  ProcessorCoreCount = (MsrValue >> 16) & 0xffff;
//[-start-161111-IB07400811-modify]//
  //
  // Get Max Core Number
  //
  Ecx = 1;
  AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, Ecx, &CpuidRegs.RegEax, &CpuidRegs.RegEbx, &CpuidRegs.RegEcx, &CpuidRegs.RegEdx);
  MaxCoreNumber =  (UINT8)(CpuidRegs.RegEbx & 0xFF);
  PcdSet8 (PcdMaxCpuCoreNumber, MaxCoreNumber);
  SmbiosTableType4Data.CoreCount = (UINT8) MaxCoreNumber;
//[-end-161111-IB07400811-modify]//
  SmbiosTableType4Data.EnabledCoreCount = (UINT8) ProcessorCoreCount;
  SmbiosTableType4Data.ThreadCount = (UINT8) ProcessorThreadCount;

  SmbiosTableType4Data.ProcessorCharacteristics = GetProcessorCharacteristics ();
  Status = AddSmbiosTableEntry (
             (EFI_SMBIOS_TABLE_HEADER *) &SmbiosTableType4Data,
             (CHAR8 **) &SmbiosTableType4Strings,
             SMBIOS_TYPE4_NUMBER_OF_STRINGS,
             &SmbiosHandle
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error adding SMBIOS table Type 4. Status = %r\n", Status));
  }

  DEBUG ((DEBUG_INFO, "InstallSmbiosType4() - End\n"));

  return Status;
}

