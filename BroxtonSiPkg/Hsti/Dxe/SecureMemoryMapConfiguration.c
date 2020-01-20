/** @file
  This file contains the tests for the SecureMemoryMapConfiguration bit

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#include "HstiSiliconDxe.h"

typedef struct {
  UINT64   Base;
  UINT64   End;
} MEMORY_RANGE;

typedef enum {
  LockableMemoryRangeLowDram,
  LockableMemoryRangeHighDram,
  LockableMemoryRangeMax,
} LOCKABLE_MEMORY_RAMGE;

MEMORY_RANGE  mLockableMemoryRange[LockableMemoryRangeMax] = {
  // 1. Low DRAM (0 - BMBOUND)
  { 0x0, 0 },
  // 2. High DRAM (4GB - BMBOUND_HI)
  { SIZE_4GB, 0 },
};

typedef enum {
  NonLockableMemoryRangeLocalApic,
  NonLockableMemoryRangeGTTMMADR,
  NonLockableMemoryRangeGMADR,
  //NonLockableMemoryRangeABASE,
  NonLockableMemoryRangePBASE,
  NonLockableMemoryRangeSBASE,
  NonLockableMemoryRangeMax,
} NONLOCKABLE_MEMORY_RAMGE;

MEMORY_RANGE  mNonLockableMemoryRange[NonLockableMemoryRangeMax] = {
  { 0, 0 },
  // 1. Local APIC for each CPU thread (IA32_APICBASE MSR 0x1B)
};

/**
  Check for overlaps in single range array

  @param[in] Range     - Pointer to Range array
  @param[in] Count     - Number of Enties

  @retval BOOLEAN - Overlap Exists
**/
BOOLEAN
CheckOverlap (
  IN MEMORY_RANGE *Range,
  IN UINTN        Count
  )
{
  UINTN  Index;
  UINTN  SubIndex;

  for (Index = 0; Index < Count - 1; Index++) {
    if ((Range[Index].Base == 0) && (Range[Index].End == 0)) {
      continue;
    }
    for (SubIndex = Index + 1; SubIndex < Count; SubIndex++) {
      if ((Range[SubIndex].Base == 0) && (Range[SubIndex].End == 0)) {
        continue;
      }
      if (((Range[Index].Base >= Range[SubIndex].Base) && (Range[Index].Base <= Range[SubIndex].End)) ||
          ((Range[SubIndex].Base >= Range[Index].Base) && (Range[SubIndex].Base <= Range[Index].End))) {
        DEBUG ((DEBUG_ERROR, "OVERLAP: \n"));
        DEBUG ((DEBUG_ERROR, "  0x%016lx - 0x%016lx\n", Range[Index].Base, Range[Index].End));
        DEBUG ((DEBUG_ERROR, "  0x%016lx - 0x%016lx\n", Range[SubIndex].Base, Range[SubIndex].End));
        return TRUE;
      }
    }
  }

  DEBUG ((DEBUG_INFO, "CheckOverlap: PASS\n"));
  return FALSE;
}

/**
  Check for overlaps between two arrays of memory ranges

  @param[in] Range1     - Pointer to Range1 array
  @param[in] Count1     - Number of Enties
  @param[in] Range2     - Pointer to Range2 array
  @param[in] Count2     - Number of Enties

  @retval BOOLEAN - Overlap Exists
**/
BOOLEAN
CheckOverlap2 (
  IN MEMORY_RANGE *Range1,
  IN UINTN        Count1,
  IN MEMORY_RANGE *Range2,
  IN UINTN        Count2
  )
{
  UINTN  Index1;
  UINTN  Index2;

  for (Index1 = 0; Index1 < Count1; Index1++) {
    if ((Range1[Index1].Base == 0) && (Range1[Index1].End == 0)) {
      continue;
    }
    for (Index2 = 0; Index2 < Count2; Index2++) {
      if ((Range2[Index2].Base == 0) && (Range2[Index2].End == 0)) {
        continue;
      }
      if (((Range1[Index1].Base >= Range2[Index2].Base) && (Range1[Index1].Base <= Range2[Index2].End)) ||
          ((Range2[Index2].Base >= Range1[Index1].Base) && (Range2[Index2].Base <= Range1[Index1].End))) {
        DEBUG ((DEBUG_ERROR, "OVERLAP2: \n"));
        DEBUG ((DEBUG_ERROR, "  0x%016lx - 0x%016lx\n", Range1[Index1].Base, Range1[Index1].End));
        DEBUG ((DEBUG_ERROR, "  0x%016lx - 0x%016lx\n", Range2[Index2].Base, Range2[Index2].End));
        return TRUE;
      }
    }
  }

  DEBUG ((DEBUG_INFO, "CheckOverlap2: PASS\n"));
  return FALSE;
}

/**
  Dumps Ranges to Serial

  @param[in] Range     - Pointer to Range array
  @param[in] Count     - Number of Enties

**/
VOID
DumpRange (
  IN MEMORY_RANGE *Range,
  IN UINTN        Count
  )
{
  UINTN  Index;

  for (Index = 0; Index < Count; Index ++) {
    DEBUG ((DEBUG_INFO, "  [%02d] 0x%016lx - 0x%016lx\n", Index, Range[Index].Base, Range[Index].End));
  }
}

/**
  Run tests for SecureMemoryMapConfiguration bit
**/
VOID
CheckSecureMemoryMapConfiguration (
  VOID
  )
{
  EFI_STATUS                      Status;
  BOOLEAN                         Result;
  UINT64                          Data64;
  UINT32                          ApertureSize;
  UINT32                          Msac;
  CHAR16                          *HstiErrorString;
  UINTN                           McD2BaseAddress;
  UINTN                           LpcBaseAddress;
  UINTN                           SpiBaseAddress;
  UINTN                           PmcBaseAddress;
  EFI_PHYSICAL_ADDRESS            BMBOUND;
  EFI_PHYSICAL_ADDRESS            BMBOUND_HI;

  if ((mFeatureImplemented[1] & HSTI_BYTE1_SECURE_MEMORY_MAP_CONFIGURATION) == 0) {
    return;
  }

  Result = TRUE;

  McD2BaseAddress = MmPciBase(SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0);
  LpcBaseAddress  = MmPciBase(DEFAULT_PCI_BUS_NUMBER_SC,PCI_DEVICE_NUMBER_PCH_LPC,PCI_FUNCTION_NUMBER_PCH_LPC);
  SpiBaseAddress  = MmPciBase(DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI);
  PmcBaseAddress = MmPciBase(DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC);

  DEBUG((DEBUG_INFO, "  Table 3-3. Memory Map Secure Configuration\n"));

  DEBUG((DEBUG_INFO, "    1. Lockable/fixed memory ranges overlap\n"));
  //
  // Get the BMBOUND and BMBOUND_HI from TOLUD and TOUUD CUNIT registers - need to find how to do a sideband access to read CUNIT registers
  //
  //

  BMBOUND_HI = (EFI_PHYSICAL_ADDRESS)LShiftU64 ((MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, TOUUD_HI_0_0_0_PCI_CUNIT_REG)) & 0xFFFFFFFF),32);
  BMBOUND_HI = (EFI_PHYSICAL_ADDRESS) (BMBOUND_HI + (MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, TOUUD_LO_0_0_0_PCI_CUNIT_REG)) & 0xFFF00000));

  BMBOUND = MmioRead32 (MmPciAddress (0, SA_MC_BUS, 0, 0, R_SA_TOLUD)) & B_SA_TOLUD_TOLUD_MASK;
  DEBUG((EFI_D_INFO, "BMBOUND = %x\n", BMBOUND));
  DEBUG((EFI_D_INFO, "BMBOUND_HI = %x\n", BMBOUND_HI));

  mLockableMemoryRange[LockableMemoryRangeLowDram].End = ((EFI_PHYSICAL_ADDRESS)BMBOUND) - 1;
  mLockableMemoryRange[LockableMemoryRangeHighDram].End = ((EFI_PHYSICAL_ADDRESS)BMBOUND_HI) - 1;

  DEBUG((DEBUG_INFO, "Lockable memory ranges:\n"));
  DumpRange(mLockableMemoryRange, LockableMemoryRangeMax);
  if (CheckOverlap(mLockableMemoryRange, LockableMemoryRangeMax)) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE1_SECURE_MEMORY_MAP_CONFIGURATION_ERROR_CODE_2, HSTI_MEMORY_MAP_SECURITY_CONFIGURATION, HSTI_BYTE1_SECURE_MEMORY_MAP_CONFIGURATION_ERROR_STRING_2);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    2. Non lockable memory ranges overlap\n"));
  //Local APIC
  Data64 = AsmReadMsr64(EFI_MSR_IA32_APIC_BASE);
  DEBUG((DEBUG_INFO, "    i. Local APIC\n"));
  if ((Data64 & B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE) != 0) {
    if ((Data64 & B_EFI_MSR_IA32_APIC_BASE_M_XAPIC) == 0) {
      mNonLockableMemoryRange[NonLockableMemoryRangeLocalApic].Base = Data64 & 0xffffff000ULL;
      mNonLockableMemoryRange[NonLockableMemoryRangeLocalApic].End = mNonLockableMemoryRange[NonLockableMemoryRangeLocalApic].Base + SIZE_4KB - 1;
    }
  }

  //Device 2 (IGD):
  DEBUG((DEBUG_INFO, "    ii. Device 2 (IGD)\n"));
  mNonLockableMemoryRange[NonLockableMemoryRangeGTTMMADR].Base = MmioRead32(McD2BaseAddress + R_SA_IGD_GTTMMADR) & 0xFFFFFFF0;
  mNonLockableMemoryRange[NonLockableMemoryRangeGTTMMADR].End = mNonLockableMemoryRange[NonLockableMemoryRangeGTTMMADR].Base + SIZE_4MB - 1;

  DEBUG((DEBUG_INFO, "   Read R_SA_IGD_MSAC_OFFSET \n"));
  Msac = MmioRead8(McD2BaseAddress + R_SA_IGD_MSAC_OFFSET);
  DEBUG((DEBUG_INFO, "   Msac = %x \n",Msac));
  ApertureSize = ((Msac & (BIT0 | BIT1 | BIT2 | BIT3 | BIT4)) + 1) * SIZE_128MB;
  DEBUG((DEBUG_INFO, "   ApertureSize = %x \n", ApertureSize));
  DEBUG((DEBUG_INFO, "   Read R_SA_IGD_GMADR \n"));
  mNonLockableMemoryRange[NonLockableMemoryRangeGMADR].Base = MmioRead32(McD2BaseAddress + R_SA_IGD_GMADR) & 0xF8000000;
  mNonLockableMemoryRange[NonLockableMemoryRangeGMADR].End = mNonLockableMemoryRange[NonLockableMemoryRangeGMADR].Base + ApertureSize - 1;

  //ABASE  : ABASE size is 128 bytes
  //DEBUG((DEBUG_INFO, "    iii. ABASE\n"));
  //mNonLockableMemoryRange[NonLockableMemoryRangeABASE].Base = (UINT32)PcdGet16(PcdScAcpiIoPortBaseAddress);//MmioRead32(PmcBaseAddress + R_PMC_ACPI_BASE);
  //mNonLockableMemoryRange[NonLockableMemoryRangeABASE].End  = mNonLockableMemoryRange[NonLockableMemoryRangeABASE].Base + 0x80 - 1;

  DEBUG((DEBUG_INFO, "    iv. PBASE\n"));
  mNonLockableMemoryRange[NonLockableMemoryRangePBASE].Base = (UINT32)PcdGet32(PcdPmcIpc1BaseAddress0);//MmioRead32(LpcBaseAddress + R_PMC_BASE );
  mNonLockableMemoryRange[NonLockableMemoryRangePBASE].End = mNonLockableMemoryRange[NonLockableMemoryRangePBASE].Base + 0x1000 - 1;

  mNonLockableMemoryRange[NonLockableMemoryRangeSBASE].Base = MmioRead32(SpiBaseAddress + R_SPI_BASE) & B_SPI_BASE_BAR;
  mNonLockableMemoryRange[NonLockableMemoryRangeSBASE].End = mNonLockableMemoryRange[NonLockableMemoryRangeSBASE].Base + 0x200 - 1;

  DEBUG((DEBUG_INFO, "Non lockable memory ranges:\n"));
  DumpRange(mNonLockableMemoryRange, NonLockableMemoryRangeMax);
  if (CheckOverlap(mNonLockableMemoryRange, NonLockableMemoryRangeMax) ||
    CheckOverlap2(mLockableMemoryRange, LockableMemoryRangeMax, mNonLockableMemoryRange, NonLockableMemoryRangeMax)) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE1_SECURE_MEMORY_MAP_CONFIGURATION_ERROR_CODE_3, HSTI_MEMORY_MAP_SECURITY_CONFIGURATION, HSTI_BYTE1_SECURE_MEMORY_MAP_CONFIGURATION_ERROR_STRING_3);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }
  //
  // ALL PASS
  //
  if (Result) {
    Status = HstiLibSetFeaturesVerified(
    PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
    NULL,
    1,
    HSTI_BYTE1_SECURE_MEMORY_MAP_CONFIGURATION
    );
    ASSERT_EFI_ERROR(Status);
  }

  return;

}
