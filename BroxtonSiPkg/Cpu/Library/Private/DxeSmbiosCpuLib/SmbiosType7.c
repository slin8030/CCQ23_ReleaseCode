/** @file
  Data for SMBIOS Cache tables (Type 7).

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

///
/// Cache Information (Type 7)
///
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TABLE_TYPE7 SmbiosTableType7Data = {
  { EFI_SMBIOS_TYPE_CACHE_INFORMATION, sizeof (SMBIOS_TABLE_TYPE7), 0 },
  STRING_1,     ///< SocketDesignation
  TO_BE_FILLED, ///< CacheConfiguration
  TO_BE_FILLED, ///< MaximumCacheSize
  TO_BE_FILLED, ///< InstalledSize
  {             ///< SupportedSRAMType
    0,            ///< Other         :1;
    0,            ///< Unknown       :1;
    0,            ///< NonBurst      :1;
    0,            ///< Burst         :1;
    0,            ///< PipelineBurst :1;
    1,            ///< Synchronous   :1;
    0,            ///< Asynchronous  :1;
    0,            ///< Reserved      :9;
  },
  {             ///< CurrentSRAMType
    0,            ///< Other         :1;
    0,            ///< Unknown       :1;
    0,            ///< NonBurst      :1;
    0,            ///< Burst         :1;
    0,            ///< PipelineBurst :1;
    1,            ///< Synchronous   :1;
    0,            ///< Asynchronous  :1;
    0,            ///< Reserved      :9;
  },
  0,            ///< CacheSpeed
  TO_BE_FILLED, ///< ErrorCorrectionType
  TO_BE_FILLED, ///< SystemCacheType
  TO_BE_FILLED, ///< Associativity
};
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TYPE7_STRING_ARRAY SmbiosTableType7Strings = {
  TO_BE_FILLED_STRING, ///< SocketDesignation
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMBIOS_HANDLE mSmbiosL1CacheHandle = 0xFFFF;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMBIOS_HANDLE mSmbiosL2CacheHandle = 0xFFFF;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMBIOS_HANDLE mSmbiosL3CacheHandle = 0xFFFF;

///
/// Convert Cache Type Field to SMBIOS format
///
#define SMBIOS_CACHE_TYPE_MAX 5
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 SmbiosCacheTypeFieldConverter[SMBIOS_CACHE_TYPE_MAX] = {
  CacheTypeUnknown,
  CacheTypeData,
  CacheTypeInstruction,
  CacheTypeUnified,
  CacheTypeOther
};

GLOBAL_REMOVE_IF_UNREFERENCED CHAR8 *CacheStrings[] = {
  "L1 Cache",
  "L2 Cache",
  "L3 Cache"
};
GLOBAL_REMOVE_IF_UNREFERENCED CHAR8 UnknownString[] = "Unknown";


/**
  This function adds a SMBIOS type 7 (cache) entry.

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_NOT_FOUND         - if the Hii PackageList could not be found.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
InstallSmbiosType7 (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINT8                                 CacheIndex;
  UINT8                                 CacheLevel;
  UINT8                                 LxCacheType;
  UINT32                                Ways;
  UINT32                                Partitions;
  UINT32                                LineSets;
  UINT32                                Sets;
  UINT32                                LxCacheSize;
  EFI_CPUID_REGISTER                    CpuidRegisters;
  SMBIOS_TYPE7_CACHE_CONFIGURATION_DATA CacheConfig;
  EFI_SMBIOS_HANDLE                     SmbiosHandle;
//[-start-170622-IB07400877-add]//
#ifdef SMBIOS_CACHE_SIZE_FOLLOW_SPEC
  UINT64                                MsrValue;
  UINT64                                EnabledCoreCount;
  EFI_CPUID_REGISTER                    CpuidRegs;
  UINT8                                 MaxCoreNumber;
  UINT32                                Ecx;
#endif
//[-end-170622-IB07400877-add]//

  DEBUG ((DEBUG_INFO, "InstallSmbiosType7() - Start\n"));

  for (CacheIndex = 0;; CacheIndex++) {
    AsmCpuidEx (
      CPUID_CACHE_PARAMS,
      CacheIndex,
      &CpuidRegisters.RegEax,
      &CpuidRegisters.RegEbx,
      &CpuidRegisters.RegEcx,
      &CpuidRegisters.RegEdx
      );
    if ((CpuidRegisters.RegEax & V_CPUID_CACHE_TYPE_MASK) == 0) {
      break;
    }

    Ways        = ((CpuidRegisters.RegEbx >> B_CPUID_CACHE_PARAMS_WAYS_SHIFT) & 0x3FF) + 1;
    Partitions  = ((CpuidRegisters.RegEbx >> B_CPUID_CACHE_PARAMS_PARTITIONS_SHIFT) & 0x3FF) + 1;
    LineSets    = (CpuidRegisters.RegEbx & 0xFFF) + 1;
    Sets        = CpuidRegisters.RegEcx + 1;

    CacheLevel  = (UINT8) (CpuidRegisters.RegEax & V_CPUID_CACHE_LEVEL_MASK) >> B_CPUID_CACHE_LEVEL_SHIFT;
    LxCacheSize = (Ways * Partitions * LineSets * Sets) / 1024;
    LxCacheType = (UINT8) (CpuidRegisters.RegEax & V_CPUID_CACHE_TYPE_MASK);

    ///
    /// Make CacheLevel zero-based for indexing and for SMBIOS Cache Configuration format
    ///
    if (CacheLevel == 0) {
      return EFI_INVALID_PARAMETER;
    }
    CacheLevel--;

    if (CacheLevel < 3) {
      SmbiosTableType7Strings.SocketDesignation = CacheStrings[CacheLevel];
    } else {
      SmbiosTableType7Strings.SocketDesignation = (CHAR8 *) &UnknownString;
    }

    CacheConfig.Level = CacheLevel;
    CacheConfig.Socketed = 0;           // Not Socketed
    CacheConfig.Reserved1 = 0;
    CacheConfig.Location = 0;           // Internal Cache
    CacheConfig.Enable = 1;             // Cache enabled
    CacheConfig.OperationalMode = 1;    // Write Back
    CacheConfig.Reserved2 = 0;
    SmbiosTableType7Data.CacheConfiguration = *(UINT16 *) &CacheConfig;

    if (LxCacheSize > SMBIOS_TYPE7_USE_64K_GRANULARITY) {
      LxCacheSize >>= 6;
      LxCacheSize |=  SMBIOS_TYPE7_USE_64K_GRANULARITY;
    }
    SmbiosTableType7Data.MaximumCacheSize = (UINT16) LxCacheSize;
    SmbiosTableType7Data.InstalledSize = (UINT16) LxCacheSize;
    
//[-start-170622-IB07400877-add]//
#ifdef SMBIOS_CACHE_SIZE_FOLLOW_SPEC
    //
    // Get Enabled Core Number
    //
    MsrValue = AsmReadMsr64 (MSR_CORE_THREAD_COUNT);
    if (MsrValue == 0) {  //msr 35h is 0 force set to one core one thread;
      MsrValue = 0x10001;
    }
    else {
      MsrValue |=  LShiftU64(MsrValue, N_CORE_COUNT_OFFSET);
    }
    EnabledCoreCount = (MsrValue >> 16) & 0xffff;
    
    //
    // Get Max Core Number
    //
    Ecx = 1;
    AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, Ecx, &CpuidRegs.RegEax, &CpuidRegs.RegEbx, &CpuidRegs.RegEcx, &CpuidRegs.RegEdx);
    MaxCoreNumber =  (UINT8)(CpuidRegs.RegEbx & 0xFF);

    DEBUG ((DEBUG_INFO, "MaxCoreNumber = %x\n", MaxCoreNumber));
    DEBUG ((DEBUG_INFO, "EnabledCoreCount = %x\n", EnabledCoreCount));
    switch (CacheLevel) {
      case 0: // L1 Cache
        SmbiosTableType7Data.MaximumCacheSize = (UINT16) (LxCacheSize * MaxCoreNumber);
        SmbiosTableType7Data.InstalledSize = (UINT16) (LxCacheSize * EnabledCoreCount);
        break;
      case 1: // L2 Cache
        SmbiosTableType7Data.MaximumCacheSize = (UINT16) (LxCacheSize * 2);
        if (EnabledCoreCount >  (MaxCoreNumber / 2)) {
          SmbiosTableType7Data.InstalledSize = (UINT16) (LxCacheSize * 2);
        } else {
          SmbiosTableType7Data.InstalledSize = (UINT16) (LxCacheSize * 1);
        }
        break;
      default:
        break;
    }
#endif
//[-end-170622-IB07400877-add]//

    switch (CacheLevel) {
      case 0:
        SmbiosTableType7Data.ErrorCorrectionType = CacheErrorParity;
        break;
      case 1:
        SmbiosTableType7Data.ErrorCorrectionType = CacheErrorSingleBit;
        break;
      case 2:
        SmbiosTableType7Data.ErrorCorrectionType = CacheErrorMultiBit;
        break;
      default:
        SmbiosTableType7Data.ErrorCorrectionType = CacheErrorUnknown;
        break;
    }

    ///
    /// If cache type is larger or equal than 5, this is undefined type so mark it as "Other" Cache type.
    ///
    if (LxCacheType >= SMBIOS_CACHE_TYPE_MAX) {
      LxCacheType = SMBIOS_CACHE_TYPE_MAX - 1;
    }
    SmbiosTableType7Data.SystemCacheType = SmbiosCacheTypeFieldConverter[LxCacheType];

    ///
    /// Convert Associativity Ways to SMBIOS format
    ///
    switch (Ways) {
      case 2:
        Ways = CacheAssociativity2Way;
        break;
      case 4:
        Ways = CacheAssociativity4Way;
        break;
      case 8:
        Ways = CacheAssociativity8Way;
        break;
      case 12:
        Ways = CacheAssociativity12Way;
        break;
      case 16:
        Ways = CacheAssociativity16Way;
        break;
      case 24:
        Ways = CacheAssociativity24Way;
        break;
      case 32:
        Ways = CacheAssociativity32Way;
        break;
      case 48:
        Ways = CacheAssociativity48Way;
        break;
      case 64:
        Ways = CacheAssociativity64Way;
        break;
      default:
        Ways = CacheAssociativityOther;
        break;
    }
    SmbiosTableType7Data.Associativity = (UINT8) Ways;

    Status = AddSmbiosTableEntry (
               (EFI_SMBIOS_TABLE_HEADER *) &SmbiosTableType7Data,
               (CHAR8 **) &SmbiosTableType7Strings,
               SMBIOS_TYPE7_NUMBER_OF_STRINGS,
               &SmbiosHandle
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Error adding SMBIOS table Type 7. Status = %r\n", Status));
      return Status;
    }
    switch (CacheLevel) {
      case 0:
        mSmbiosL1CacheHandle = SmbiosHandle;
        break;
      case 1:
        mSmbiosL2CacheHandle = SmbiosHandle;
        break;
      case 2:
        mSmbiosL3CacheHandle = SmbiosHandle;
        break;
      default:
        break;
    }
  }

  DEBUG ((DEBUG_INFO, "InstallSmbiosType7() - End\n"));

  return EFI_SUCCESS;
}

