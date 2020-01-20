/** @file
  Source code file for Platform Init Pre-Memory PEI module

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2018 Intel Corporation.

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

//[-start-160517-IB03090427-modify]//
#include <ScAccess.h>
#include <SaAccess.h>
#include <SeCAccess.h>
#include <PlatformBaseAddresses.h>
#include <FrameworkPei.h>
#include <Ppi/MfgMemoryTest.h>
#include <Ppi/TemporaryRamSupport.h>
#include <Ppi/BlockIo.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/SecUma.h>
#include <Ppi/FvLoadFile.h>
#include <Ppi/Stall.h>
#include <Ppi/MemoryDiscovered.h>
#include <Ppi/FirmwareVolumeInfo.h>
#include <Ppi/SiPolicyPpi.h>
#include <Ppi/BiosReservedMemory.h>
#include <Ppi/DramPolicyPpi.h>
#include <Ppi/SaPolicy.h>
#include <Ppi/ScPolicyPreMem.h>
//[-start-151216-IB07220025-add]//
#include <Ppi/BootInRecoveryMode.h>
//[-end-151216-IB07220025-add]//
//[-start-151224-IB07220029-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
#include <Ppi/EmuPei.h>
#include <Guid/FirmwareFileSystem.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Guid/FirmwareFileSystem3.h>
#endif
//[-end-161123-IB07250310-modify]//
//[-end-151224-IB07220029-add]//
#include <Guid/Capsule.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Guid/SystemNvDataGuid.h>
#include <Guid/PlatformInfo.h>
//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Guid/FirmwarePerformance.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PerformanceLib.h>
#include <Library/MtrrLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/I2cLib.h>
#include <Library/PmicLib.h>
#include <Library/PeiSiPolicyLib.h>
#include <Library/CpuPolicyLib.h>
#include <Library/FlashRegionLib.h>
#include <Library/ScPlatformLib.h>
#include <Library/PeiPolicyInitLib.h>
#include <Library/EcLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/PeiScPolicyLib.h>
#include <Library/PeiSiPolicyUpdateLib.h>
#include <Library/PeiSaPolicyLib.h>

#include "Smip.h"
#include "Stall.h"
#include "PlatformId.h"
#include "FvCallback.h"
#include "MemoryCallback.h"
#include "BoardGpiosPreMem.h"
#include "PlatformInitPreMem.h"
#include <FastRecoveryData.h>

#include <Library/SteppingLib.h>
#include <Library/HeciMsgLib.h>
#include <Ppi/SecPlatformInformation.h>
#include <Library/PlatformSecLib.h>
#include <Library/TimerLib.h>
#include <Ppi/Spi.h>
//[-start-160420-IB03090426-add]//
#include <Private/Library/PeiScInitLib.h>
//[-end-160420-IB03090426-add]//

#if (TABLET_PF_ENABLE == 1)
#include <PmicReg_WhiskeyCove.h>
#endif

#if (ENBDT_PF_ENABLE == 1)
//SSC
  #include <Library/PmcIpcLib.h>
  #include <SscRegs.h>
  #include <Library/SideBandLib.h>
#endif

//[-start-151221-IB11270139-add]//
#include <Library/BaseOemSvcKernelLib.h>
#include <Library/PeiOemSvcKernelLib.h>
//[-end-151221-IB11270139-add]//
//[-start-151216-IB11270137-add]//
#include <Library/PeiOemSvcChipsetLib.h>
//[-end-151216-IB11270137-add]//
#include <Guid/BiosFirstBootHobGuid.h>
//[-start-160217-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160217-IB07400702-add]//
//[-start-160304-IB07400706-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160304-IB07400706-add]//
//[-start-160604-IB07400740-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160604-IB07400740-add]//

//[-start-170516-IB08450375-add]//
//[-start-170518-IB07400867-modify]//
//#if (BXTI_PF_ENABLE == 1)
#define EFI_ACPI_HROT_FW_DATA_LEN         32
//#endif
//[-end-170518-IB07400867-modify]//
//[-end-170516-IB08450375-add]//


extern EFI_GUID gEfiBootMediaHobGuid;
//[-start-161128-IB11270169-add]//
extern EFI_GUID gFdoModeEnabledHobGuid;
//[-end-161128-IB11270169-add]//
//
// The global indicator, the FvFileLoader callback will modify it to TRUE after loading PEIM into memory
//
BOOLEAN ImageInMemory = FALSE;
CHAR8   mGdtTable[0x40];

#ifndef MEMORY_TEST_DISABLE
//Memory Test Manufacturing mode
#define DATA_PATTERN_ARRAY_SIZE (sizeof(DataPatternForMemoryTest) / sizeof(UINT32))
UINT32 DataPatternForMemoryTest[] = {
  0x55555555, 0xAAAAAAAA, 0x55555510, 0x555555EF, 0x55555510, 0x555555EF, 0x55555510, 0x555555EF,
  0x55555555, 0xAAAAAAAA, 0x55551055, 0x5555EF55, 0x55551055, 0x5555EF55, 0x55551055, 0x5555EF55,
  0x55555555, 0xAAAAAAAA, 0x55105555, 0x55EF5555, 0x55105555, 0x55EF5555, 0x55105555, 0x55EF5555,
  0x55555555, 0xAAAAAAAA, 0x10555555, 0xEF555555, 0x10555555, 0xEF555555, 0x10555555, 0xEF555555
};
#endif

extern EFI_PEI_PPI_DESCRIPTOR mCseUfsSelectPpiList[];
extern EFI_PEI_PPI_DESCRIPTOR mCseEmmcSelectPpiList[];
extern EFI_PEI_PPI_DESCRIPTOR mCseSpiSelectPpiList[];
//[-start-151216-IB07220025-add]//
extern EFI_PEI_PPI_DESCRIPTOR mPpiListRecoveryBootMode;
//[-end-151216-IB07220025-add]//

#ifndef FSP_WRAPPER_FLAG
EFI_SEC_PLATFORM_INFORMATION_PPI  mSecPlatformInformationPpi = { SecPlatformInformation };
#endif

#define PEI_STALL_RESOLUTION   1
static EFI_PEI_STALL_PPI  mStallPpi = {
  PEI_STALL_RESOLUTION,
  Stall
};

#if defined(PRAM_SUPPORT) || defined(SGX_SUPPORT)
static PEI_BIOS_RESERVED_MEMORY_POLICY_PPI mPeiBiosReservedMemoryPolicyPpi = {
  GetBiosReservedMemoryPolicy
};

static EFI_PEI_PPI_DESCRIPTOR mBiosReservedMemoryPolicyPpi =
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gBiosReservedMemoryPolicyPpiGuid,
    &mPeiBiosReservedMemoryPolicyPpi
  };
#endif

static EFI_PEI_PPI_DESCRIPTOR mInstallStallPpi =
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiPeiStallPpiGuid,
    &mStallPpi
  };

#ifndef MEMORY_TEST_DISABLE
static PEI_MFG_MEMORY_TEST_PPI mPeiMfgMemoryTestPpi = {
  MfgMemoryTest
};

//static EFI_PEI_PPI_DESCRIPTOR mMfgMemTestPpi =
//  {
//    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
//    &gPeiMfgMemoryTestPpiGuid,
//    &mPeiMfgMemoryTestPpi
//  };
#endif

static EFI_PEI_PPI_DESCRIPTOR mPeiTemporaryRamSupportPpiPpi[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiTemporaryRamSupportPpiGuid,
    NULL
  }
};

#ifndef FSP_WRAPPER_FLAG
static EFI_PEI_PPI_DESCRIPTOR mPeiPlatformInformationPpiPpi[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiSecPlatformInformationPpiGuid,
    &mSecPlatformInformationPpi
  }
};

EFI_PEI_NOTIFY_DESCRIPTOR mNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiMemoryDiscoveredPpiGuid,
    MemoryDiscoveredPpiNotifyCallback
  }
};
#endif

//
// Notify Callbacks for SPI and non-SPI boot devices.
// These are installed by MemoryDiscovered Callback, since they require main memory.
//
//[-start-161018-IB06740518-modify]//
EFI_PEI_NOTIFY_DESCRIPTOR mFvNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH,
    &gEfiPeiVirtualBlockIoPpiGuid,  //non-SPI boot - installed after MemInit
    GetFvNotifyCallback
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gCseSpiSelectPpiGuid,   //SPI boot - installed by PeiSecUma
    GetFvNotifyCallback
  }
};
//[-end-161018-IB06740518-modify]//

//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
extern EFI_GUID gFspTempRamExitGuid;
EFI_STATUS
EFIAPI
FspTempRamExitCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "FspTempRamExitCallback\n"));
  Status = PeiServicesNotifyPpi (&mFvNotifyList[0]);
  return Status;
}

EFI_PEI_NOTIFY_DESCRIPTOR mFspTempRamExitList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gFspTempRamExitGuid,
    FspTempRamExitCallback
  }
};
#endif
//[-end-161123-IB07250310-add]//

EFI_STATUS
EFIAPI
GetBiosReservedMemoryPolicy (
  IN CONST EFI_PEI_SERVICES               **PeiServices,
  IN PEI_BIOS_RESERVED_MEMORY_POLICY_PPI  *This,
  IN OUT BIOS_RESERVED_MEMORY_CONFIG      *BiosReservedMemoryPolicy
  );

//[-start-151224-IB07220029-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
BOOLEAN  mInRecoveryPei = FALSE;

BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  )
{
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    return mInRecoveryPei;
  } else {
    return FALSE;
  }
}

VOID
FastCrisisRecoveryCheck (
  VOID    
  )
{
  VOID           *EmuPeiPpi;
  EFI_STATUS     Status;
  
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    Status = PeiServicesLocatePpi (
                    &gEmuPeiPpiGuid,
                    0,
                    NULL,
                    (VOID **) &EmuPeiPpi
                    );
    mInRecoveryPei = EFI_ERROR(Status) ? FALSE : TRUE;
  }
}
#endif
//[-end-161123-IB07250310-modify]//
//[-end-151224-IB07220029-add]//
VOID
CopyMemSse4 (
  IN VOID* Dst,
  IN VOID* Src,
  IN UINTN SizeInBytes
  )
{
  _asm {
    // Initialize pointers to start of the USWC memory
    mov esi, Src
    mov edx, Src

    // Initialize pointer to end of the USWC memory
    add edx, SizeInBytes

    // Initialize pointer to start of the cacheable WB buffer
    mov edi, Dst

    // save xmm0 ~ xmm3 to stack
    sub     esp, 040h
    movdqu  [esp], xmm0
    movdqu  [esp + 16], xmm1
    movdqu  [esp + 32], xmm2
    movdqu  [esp + 48], xmm3

    // Start of Bulk Load loop
    inner_start:
    // Load data from USWC Memory using Streaming Load
    MOVNTDQA xmm0, xmmword ptr [esi]
    MOVNTDQA xmm1, xmmword ptr [esi + 16]
    MOVNTDQA xmm2, xmmword ptr [esi + 32]
    MOVNTDQA xmm3, xmmword ptr [esi + 48]

    // Copy data to buffer
    MOVDQA xmmword ptr [edi], xmm0
    MOVDQA xmmword ptr [edi + 16], xmm1
    MOVDQA xmmword ptr [edi + 32], xmm2
    MOVDQA xmmword ptr [edi + 48], xmm3

    // Increment pointers by cache line size and test for end of loop
    add esi, 040h
    add edi, 040h
    cmp esi, edx
    jne inner_start

    // restore xmm0 ~ xmm3
    mfence
    movdqu  xmm0, [esp]
    movdqu  xmm1, [esp + 16]
    movdqu  xmm2, [esp + 32]
    movdqu  xmm3, [esp + 48]
    add     esp, 040h // stack cleanup
  }
  // End of Bulk Load loop
}


/*++

Routine Description:

This function get SA setup config in PEI.

Arguments:

PeiServices     Pointer to PEI Services.
This            Pei memory test PPI pointer.
SaPreMemConfig  Pointer to SA Pre Mem Config Block

Returns:

EFI_SUCCESS         The operation completed successfully.
EFI_DEVICE_ERROR    Memory test failed. It's not safe to use this range of memory.

--*/
EFI_STATUS
EFIAPI
UpdateSaPreMemPolicy (
  IN OUT SA_PRE_MEM_CONFIG   *SaPreMemConfig
  )
{
  EFI_STATUS                         Status;
  UINTN                              VariableSize;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI    *VariableServices;
//[-end-160803-IB07220122-add]//
  CHIPSET_CONFIGURATION              SystemConfiguration;

//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//
  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigDataPreMem (&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//
  if (Status == EFI_SUCCESS) {
    SaPreMemConfig->IgdDvmt50PreAlloc = SystemConfiguration.IgdDvmt50PreAlloc;
    SaPreMemConfig->ApertureSize      = SystemConfiguration.IgdApertureSize;
    SaPreMemConfig->GttSize           = SystemConfiguration.GTTSize;
    SaPreMemConfig->InternalGraphics  = SystemConfiguration.Igd;
    SaPreMemConfig->PrimaryDisplay    = SystemConfiguration.PrimaryVideoAdaptor;
    if (SystemConfiguration.PrimaryVideoAdaptor == 4) {
      ///
      /// When Primary Display is selected as HG, Display is driven on-board and PrimaryDisplay should be set as 0. (IGD)
      ///
      SaPreMemConfig->PrimaryDisplay = 0;
    }
  }
  return Status;
}

/*++

Routine Description:

This function get NPK setup config in PEI.

Arguments:

PeiServices     Pointer to PEI Services.
This            Pei memory test PPI pointer.
NpkPreMemConfig  Pointer to Npk Pre Mem Config Block

Returns:

EFI_SUCCESS         The operation completed successfully.
EFI_DEVICE_ERROR    Memory test failed. It's not safe to use this range of memory.

--*/
EFI_STATUS
EFIAPI
UpdateNpkPreMemPolicy (
  IN OUT NPK_PRE_MEM_CONFIG   *NpkPreMemConfig
  )
{
  EFI_STATUS                         Status;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI    *VariableServices;
//[-end-160803-IB07220122-add]//
  UINTN                              VariableSize;
  CHIPSET_CONFIGURATION              SystemConfiguration;

  DEBUG ((DEBUG_INFO, "Begin UpdateNpkPreMemPolicy \n"));
//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//
  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigDataPreMem (&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//
  if (Status == EFI_SUCCESS) {
    DEBUG ((DEBUG_INFO, "Started UpdateNpkPreMemPolicy , status = %x \n",Status));

    if (SystemConfiguration.NpkEn == 3) { // Auto
      //
      // For Auto , enable NPK for Android and disable for Windows
      //
      if (SystemConfiguration.OsSelection == 1) { // Android
        NpkPreMemConfig->NpkEn          = 1;
      } else {
        NpkPreMemConfig->NpkEn          = 0;
      }
    } else { // use setup value
      NpkPreMemConfig->NpkEn            = SystemConfiguration.NpkEn;              ///< 0-disabled, 1-enabled, 2-debugger, 3-auto
    }

    NpkPreMemConfig->FwTraceEn          = SystemConfiguration.FwTraceEn;          ///< 0-disabled, 1-enabled
    NpkPreMemConfig->FwTraceDestination = SystemConfiguration.FwTraceDestination; ///< 1-NPK_TRACE_TO_MEMORY, 2-NPK_TRACE_TO_DCI, 3-NPK_TRACE_TO_BSSB, 4-NPK_TRACE_TO_PTI.
    NpkPreMemConfig->RecoverDump        = SystemConfiguration.RecoverDump;        ///< 0-disabled, 1-enabled,
    NpkPreMemConfig->Msc0Size           = SystemConfiguration.Msc0Size;           ///< 0-0MB, 1-1MB, 2-8MB, 3-64MB, 4-128MB, 5-256MB, 6-512MB, 7-1GB.
    NpkPreMemConfig->Msc0Wrap           = SystemConfiguration.Msc0Wrap;           ///< 0-n0-warp, 1-warp.
    NpkPreMemConfig->Msc1Size           = SystemConfiguration.Msc1Size;           ///< 0-0MB, 1-1Mb, 2-8MB, 3-64MB, 4-128MB, 5-256MB, 6-512MB, 7-1GB.
    NpkPreMemConfig->Msc1Wrap           = SystemConfiguration.Msc1Wrap;           ///< 0-n0-warp, 1-warp.
    NpkPreMemConfig->PtiMode            = SystemConfiguration.PtiMode;            ///< 0-0ff, 1-x4, 2-x8, 3-x12, 4-x16
    NpkPreMemConfig->PtiTraining        = SystemConfiguration.PtiTraining;        ///< 0-off, 1-6=1-6.
    NpkPreMemConfig->PtiSpeed           = SystemConfiguration.PtiSpeed;           ///< 0-full, 1-half, 2-quarter,
    NpkPreMemConfig->PunitMlvl          = SystemConfiguration.PunitMlvl;          ///< 1 = message level 0
    NpkPreMemConfig->PmcMlvl            = SystemConfiguration.PmcMlvl;            ///< 1 = message level 0
    NpkPreMemConfig->SwTraceEn          = SystemConfiguration.SwTraceEn;          ///< 0-disabled, 1-enabled
  }
  return Status;
}

#if defined(PRAM_SUPPORT) || defined(SGX_SUPPORT)

/*++
Routine Description:

  This function is to get Bios Reserved Memory in PEI.

Arguments:

  PeiServices               - Pointer to PEI Services.
  This                      - Pei memory test PPI pointer.
  BiosReservedMemoryPolicy  - Pointer to BiosReservedMemorypolicy.

Returns:

  EFI_SUCCESS - The operation completed successfully.

--*/
EFI_STATUS
EFIAPI
GetBiosReservedMemoryPolicy (
  IN CONST EFI_PEI_SERVICES               **PeiServices,
  IN PEI_BIOS_RESERVED_MEMORY_POLICY_PPI  *This,
  IN OUT BIOS_RESERVED_MEMORY_CONFIG      *BiosReservedMemoryPolicy
  )
{
  EFI_STATUS                         Status;
  UINTN                              VariableSize;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI    *VariableServices;
//[-end-160803-IB07220122-add]//
  CHIPSET_CONFIGURATION              SystemConfiguration;

//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//
  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigDataPreMem (&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//


  if (Status == EFI_SUCCESS) {
#ifdef PRAM_SUPPORT
    BiosReservedMemoryPolicy->Pram = SystemConfiguration.Pram;
#endif


  } else {
#ifdef PRAM_SUPPORT
    BiosReservedMemoryPolicy->Pram = 0x30;
#endif

  }

  Status = EFI_SUCCESS;


#ifdef PRAM_SUPPORT
  DEBUG ((DEBUG_INFO, "SystemConfiguration.Pram = %x \n", SystemConfiguration.Pram));
#endif


  return Status;
}

#endif

#ifndef MEMORY_TEST_DISABLE
/*++
Routine Description:

  This function checks the memory range in PEI.

Arguments:

  PeiServices     Pointer to PEI Services.
  This            Pei memory test PPI pointer.
  BeginAddress    Beginning of the memory address to be checked.
  MemoryLength    Bytes of memory range to be checked.
  Operation       Type of memory check operation to be performed.
  ErrorAddress    Return the address of the error memory address.

Returns:

  EFI_SUCCESS         The operation completed successfully.
  EFI_DEVICE_ERROR    Memory test failed. It's not safe to use this range of memory.
--*/
EFI_STATUS
EFIAPI
MfgMemoryTest (
  IN  CONST EFI_PEI_SERVICES             **PeiServices,
  IN  PEI_MFG_MEMORY_TEST_PPI            *This,
  IN  UINT32                             BeginAddress,
  IN  UINT32                             MemoryLength
  )
{
  UINT32 i;
  UINT32 memAddr;
  UINT32 readData;
  UINT32 xorData;
  UINT32 TestFlag = 0;

  memAddr = BeginAddress;

  // Output Message for MFG
  DEBUG ((DEBUG_INFO, "MFGMODE SET\n"));

  // Writing the pattern in defined location.
  while (memAddr < (BeginAddress+MemoryLength)) {
    for (i = 0; i < DATA_PATTERN_ARRAY_SIZE; i++) {
      if (memAddr > (BeginAddress+MemoryLength -4)) {
        memAddr = memAddr + 4;
        break;
      }

      *((volatile UINT32*) memAddr) = DataPatternForMemoryTest[i];
      memAddr = memAddr + 4;
    }
  }

  // Verify the pattern
  memAddr = BeginAddress;

  while (memAddr < (BeginAddress+MemoryLength)) {
    for (i = 0; i < DATA_PATTERN_ARRAY_SIZE; i++) {
      if (memAddr > (BeginAddress+MemoryLength -4)) {
        memAddr = memAddr + 4;
        break;
      }

      readData = *((volatile UINT32*) memAddr);
      xorData = readData ^ DataPatternForMemoryTest[i];

      /* If xorData is non-zero, this particular memAddr has a failure. */
      if (xorData != 0x00000000) {
        DEBUG ((DEBUG_ERROR, "Expected value....: %x\n", DataPatternForMemoryTest[i]));
        DEBUG ((DEBUG_ERROR, "ReadData value....: %x\n", readData));
        DEBUG ((DEBUG_ERROR, "Pattern failure at....: %x\n", memAddr));
        TestFlag = 1;
      }

      memAddr = memAddr + 4;
    }
  }

  if (TestFlag) {
    return EFI_DEVICE_ERROR;
  }

  //Output Message for MFG
  DEBUG ((DEBUG_INFO, "MFGMODE MEMORY TEST PASSED\n"));

  return EFI_SUCCESS;
}
#endif

BOOLEAN
IsRtcUipAlwaysSet (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  )
{
  EFI_PEI_STALL_PPI *StallPpi;
  UINTN             Count;

  PeiServicesLocatePpi (&gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);

  for (Count = 0; Count < 500; Count++) { // Maximum waiting approximates to 1.5 seconds (= 3 msec * 500)
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERA);
    if ((IoRead8 (R_RTC_TARGET2) & B_RTC_REGISTERA_UIP) == 0) {
      return FALSE;
    }

    StallPpi->Stall (PeiServices, StallPpi, 3000);
  }

  return TRUE;
}


VOID
RtcPowerFailureHandler (
  VOID
  )
{
  UINT16          Data16;
  UINT8           Data8;

  ///
  /// CHV BIOS Specification 0.5.0 - Section 29.4.3, "Power Failure Consideration"
  ///
  /// When the RTC_PWR_STS bit is set, it indicates that the RTCRST# signal went low.
  /// Software should clear this bit. For example, changing the RTC battery sets this bit.
  /// System BIOS should reset CMOS to default values if the RTC_PWR_STS bit is set.
  /// The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set
  /// before memory initialization. This will ensure that the RTC state machine has been
  /// initialized.
  /// 1. If the RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion or a
  ///    battery failure, steps 2 through 5 should be executed.
  /// 2.  Set RTC Register 0x0A[6:4] to 110b or 111b.
  /// 3.  Set RTC Register 0x0B[7].
  /// 4.  Set RTC Register 0x0A[6:4] to 010b.
  /// 5.  Clear RTC Register 0x0B[7].
  ///

  Data16 = MmioRead16 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);

  if ((Data16 & B_PMC_GEN_PMCON_RTC_PWR_STS) != 0) {
    ///
    /// 2. Set RTC Register 0Ah[6:4] to 110b or 111b
    ///
    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERA);
    Data8 = IoRead8 (R_RTC_TARGET2) & (UINT8) ~(B_RTC_REGISTERA_DV);
    Data8 |= (UINT8) (V_RTC_REGISTERA_DV_DIV_RST1);
    IoWrite8 (R_RTC_TARGET2, Data8);

    ///
    /// 3. Set RTC Register 0Bh[7].
    ///
    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERB);
    IoOr8 (R_RTC_TARGET2, (UINT8) B_RTC_REGISTERB_SET);

    ///
    /// 4. Set RTC Register 0Ah[6:4] to 010b
    ///
    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERA);
    Data8 = IoRead8 (R_RTC_TARGET2) & (UINT8) ~(B_RTC_REGISTERA_DV);
    Data8 |= (UINT8) (V_RTC_REGISTERA_DV_NORM_OP);
    IoWrite8 (R_RTC_TARGET2, Data8);

    ///
    /// 5. Clear RTC Register 0Bh[7].
    ///
    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERB);
    IoAnd8 (R_RTC_TARGET2, (UINT8) ~B_RTC_REGISTERB_SET);
  }

  return;
}


VOID
ScBaseInit (
  VOID
  )
{
  //DEBUG ((DEBUG_INFO, "ScBaseInit() - Start\n"));

  // Set BARs for PMC SSRAM (0/13/3)
  // Allocation for these regions is done in PlatformInitFinalConfig() via call to BuildResourceDescriptorHob()
  MmioWrite32(
    MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC_SSRAM, 0x10),  //Write BAR0-lower
    PcdGet32(PcdPmcSsramBaseAddress0)
    );
  MmioWrite32(
    MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC_SSRAM, 0x18),  //Write BAR1-lower
    PcdGet32(PcdPmcSsramBaseAddress1)
    );
  MmioWrite16(
    MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC_SSRAM, 0x4),   //Set BME and MSE
    0x6
    );

  //
  // Set SPI Base Address
  //
  MmioWrite32 (
    MmPciAddress (0,DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI, R_SPI_BASE),
    (UINT32)((SPI_BASE_ADDRESS & B_SPI_BASE_BAR))
    );

//[-start-160817-IB03090432-add]//
  //
  // Enable SPI Memory decode
  //
  MmioWrite16(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI, R_SPI_COMMAND),
    EFI_PCI_COMMAND_MEMORY_SPACE
    );
//[-end-160817-IB03090432-add]//

  //
  // Set P2SB Base Address
  //
//[-start-160815-IB03090432-modify]//
  MmioWrite32(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_P2SB, PCI_FUNCTION_NUMBER_P2SB, R_PCH_P2SB_SBREG_BAR),
    (UINT32)((PcdGet32(PcdP2SBBaseAddress)))
    );
//[-end-160815-IB03090432-modify]//

  //
  // Enable P2SB Memory decode
  //
//[-start-160817-IB03090432-modify]//
  MmioWrite16(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_P2SB, PCI_FUNCTION_NUMBER_P2SB, PCI_COMMAND_OFFSET),
    EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE
    );
//[-end-160817-IB03090432-modify]//

//[-start-161023-IB07400803-remove]//
    //
    // Move to SEC phase (platformseclib.c)
    //
//#if (TABLET_PF_ENABLE == 0)
//  PchLpcIoDecodeRangesSet (
//    (V_PCH_LPC_IOD_LPT_378  << N_PCH_LPC_IOD_LPT)  |
//    (V_PCH_LPC_IOD_COMB_3E8 << N_PCH_LPC_IOD_COMB) |
//    (V_PCH_LPC_IOD_COMA_3F8 << N_PCH_LPC_IOD_COMA)
//    );
//
//  PchLpcIoEnableDecodingSet (
//    B_PCH_LPC_IOE_ME2  |
//    B_PCH_LPC_IOE_SE   |
//    B_PCH_LPC_IOE_ME1  |
//    B_PCH_LPC_IOE_KE   |
//    B_PCH_LPC_IOE_HGE  |
//    B_PCH_LPC_IOE_LGE  |
//    B_PCH_LPC_IOE_FDE  |
//    B_PCH_LPC_IOE_PPE  |
//    B_PCH_LPC_IOE_CBE  |
//    B_PCH_LPC_IOE_CAE
//    );
//#endif
//[-end-161023-IB07400803-remove]//

  //DEBUG ((DEBUG_INFO, "ScBaseInit() - End\n"));
}


/**
  This function performs Silicon Policy initialization.

  @param[in]  FirmwareConfiguration  It uses to skip specific policy init that depends
                                     on the 'FirmwareConfiguration' variable.

  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI ERRORS             The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
PeiSiPolicyInit (
  VOID
  )
{
  EFI_STATUS             Status;
  SI_POLICY_PPI          *SiPolicyPpi;

  //
  // Call SiCreatePolicyDefaults to initialize Silicon Policy structure
  // and get all Intel default policy settings.
  //
  Status = SiCreatePolicyDefaults (&SiPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  //
  // Update and override all platform related and customized settings below.
  //
  UpdatePeiSiPolicy (SiPolicyPpi);
//[-start-151216-IB11270137-add]//
  //
  // OemServices
  //
  Status = OemSvcUpdatePeiSiPlatformPolicy (SiPolicyPpi);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcUpdatePeiSiPlatformPolicy, Status : %r\n", Status));  
  ASSERT ( ( Status == EFI_SUCCESS ) || ( Status == EFI_UNSUPPORTED ) || ( Status == EFI_MEDIA_CHANGED ) );
  if (Status == EFI_SUCCESS) {
    return Status;
  }
//[-end-151216-IB11270137-add]//
  //
  // Install SiPolicyPpi.
  // While installed, RC assumes the Policy is ready and finalized. So please
  // update and override any setting before calling this function.
  //
  Status = SiInstallPolicyPpi (SiPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

//[-start-161109-IB07400810-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// PCIe ClockReq# Pins Config
//
#define W_GPIO_209            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x00d0     //PCIE_CLKREQ0_B
#define W_GPIO_210            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x00d8     //PCIE_CLKREQ1_B
#define W_GPIO_211            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x00e0     //PCIE_CLKREQ2_B
#define W_GPIO_212            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x00e8     //PCIE_CLKREQ3_B

//
// RVP PCIe Reset Pins Config
//
#define N_GPIO_13             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0068     //GPIO_13
#define N_GPIO_15             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0078     //GPIO_15
#define N_GPIO_37             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0128     //PWM3
#define W_GPIO_152            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x00b0     //ISH_GPIO_6
#endif
//[-end-161109-IB07400810-add]//

/**
  This function performs SC PreMem Policy initialization.

  @param[in]  StartTimerTicker       The Start Timer Ticker for PFET# enabled

  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI ERRORS             The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
PeiScPreMemPolicyInit (
  IN UINT64 *StartTimerTicker
  )
{
  EFI_STATUS             Status;
  SC_PREMEM_POLICY_PPI   *ScPreMemPolicy;
  SC_PCIE_PREMEM_CONFIG  *PciePreMemConfig;
//[-start-161109-IB07400810-add]//
  UINT8                  BoardId;
//[-end-161109-IB07400810-add]//

  //
  // Call ScCreatePreMemConfigBlocks to initialize SC Policy structure
  // and get all Intel default policy settings.
  //
  Status = ScCreatePreMemConfigBlocks (&ScPreMemPolicy);
  ASSERT_EFI_ERROR (Status);

  //
  // Update and override all platform related and customized settings below.
  //
  Status = GetConfigBlock ((VOID *) ScPreMemPolicy, &gPcieRpPreMemConfigGuid, (VOID *) &PciePreMemConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Update PCIe PERST# and CLK# policies
  //
  PciePreMemConfig->StartTimerTickerOfPfetAssert = (UINTN) *StartTimerTicker;
  
//[-start-161109-IB07400810-modify]//
  BoardId = MultiPlatformGetBoardIds();
  
  switch (BoardId) {
  case BOARD_ID_APL_RVP_1A:
  case BOARD_ID_APL_RVP_2A:
    //
    // ApolloLake RVP PCI-E HW port map:
    //
    // PCIE Lane 4 (0/0x14/0, Port 0) -> x4 Slot (Slot 2), ClkReq = 2, PCIE_WAKE2_N, Reset Pin: GPIO_13 , Power Pin: GPIO_17
    // PCIE Lane 5 (0/0x14/1, Port 1) -> M.2 WLAN,         ClkReq = 3, PCIE_WAKE3_N, Reset Pin: GPIO_15
    // PCIE Lane 0 (0/0x13/0, Port 2) -> x4 Slot (Slot 1), ClkReq = 0, PCIE_WAKE0_N, Reset Pin: GPIO_152, Power Pin: GPIO_19
    // PCIE Lane 1 (0/0x13/1, Port 3) -> x4 Slot (Slot 1)
    // PCIE Lane 2 (0/0x13/2, Port 4) -> i211,             ClkReq = 1, PCIE_WAKE1_N, Reset Pin: GPIO_37
    // PCIE Lane 3 (0/0x13/3, Port 5) -> x
    //
    PciePreMemConfig->RootPort[0].Perst = N_GPIO_13;  // Slot2
    PciePreMemConfig->RootPort[1].Perst = N_GPIO_15;  // NGFF
    PciePreMemConfig->RootPort[2].Perst = W_GPIO_152; // Slot1
    PciePreMemConfig->RootPort[3].Perst = 0;
    PciePreMemConfig->RootPort[4].Perst = N_GPIO_37;  // LOM
    PciePreMemConfig->RootPort[5].Perst = 0;

    PciePreMemConfig->RootPort[0].Clock = W_GPIO_211;  // ClockReq2, Slot2
    PciePreMemConfig->RootPort[1].Clock = W_GPIO_212;  // ClockReq3, NGFF
    PciePreMemConfig->RootPort[2].Clock = W_GPIO_209;  // ClockReq0, Slot1
    PciePreMemConfig->RootPort[3].Clock = 0;
    PciePreMemConfig->RootPort[4].Clock = W_GPIO_210;  // ClockReq1, LOM, Pull Low, always enabled
    PciePreMemConfig->RootPort[5].Clock = 0;
    break;
  case BOARD_ID_OXH_CRB:
  case BOARD_ID_LFH_CRB:
  case BOARD_ID_JNH_CRB:
    //
    // ApolloLake-I OxbowHill PCI-E HW port map:
    //
    // PCIE Lane 4 (0/0x14/0, Port 0) => M.2 3G,              ClkReq = 2, PCIE_WAKE2_N, Reset Pin: N/A
    // PCIE Lane 5 (0/0x14/1, Port 1) => M.2 Wifi/Bluetooth,  ClkReq = 3, PCIE_WAKE3_N, Reset Pin: N/A 
    // PCIE Lane 0 (0/0x13/0, Port 2) => x4 slot (x2),        ClkReq = 1, PCIE_WAKE1_N, Reset Pin: N/A
    // PCIE Lane 1 (0/0x13/1, Port 3) => x4 slot (x2)
    // PCIE Lane 2 (0/0x13/2, Port 4) => LAN,                 ClkReq = 0, PCIE_WAKE0_N, Reset Pin: N/A
    // PCIE Lane 3 (0/0x13/3, Port 5) => x
    //
    PciePreMemConfig->RootPort[0].Perst = 0;
    PciePreMemConfig->RootPort[1].Perst = 0;
    PciePreMemConfig->RootPort[2].Perst = 0;
    PciePreMemConfig->RootPort[3].Perst = 0;
    PciePreMemConfig->RootPort[4].Perst = 0;
    PciePreMemConfig->RootPort[5].Perst = 0;

    PciePreMemConfig->RootPort[0].Clock = W_GPIO_211;  // ClockReq2, M.2 3G
    PciePreMemConfig->RootPort[1].Clock = W_GPIO_212;  // ClockReq3, M.2 Wifi/Bluetooth
    PciePreMemConfig->RootPort[2].Clock = W_GPIO_210;  // ClockReq1, x4 slot (x2)
    PciePreMemConfig->RootPort[3].Clock = 0;
    PciePreMemConfig->RootPort[4].Clock = W_GPIO_209;  // ClockReq0, LOM, Pull Low, always enabled
    PciePreMemConfig->RootPort[5].Clock = 0;
    break;
  default:
    PciePreMemConfig->RootPort[0].Perst = 0;
    PciePreMemConfig->RootPort[1].Perst = 0;
    PciePreMemConfig->RootPort[2].Perst = 0;
    PciePreMemConfig->RootPort[3].Perst = 0;
    PciePreMemConfig->RootPort[4].Perst = 0;
    PciePreMemConfig->RootPort[5].Perst = 0;

    PciePreMemConfig->RootPort[0].Clock = 0;
    PciePreMemConfig->RootPort[1].Clock = 0;
    PciePreMemConfig->RootPort[2].Clock = 0;
    PciePreMemConfig->RootPort[3].Clock = 0;
    PciePreMemConfig->RootPort[4].Clock = 0;
    PciePreMemConfig->RootPort[5].Clock = 0;
    break;
  }
//[-end-161109-IB07400810-modify]//

#if (TABLET_PF_ENABLE == 1)
  if (GetBxtSeries() == Bxt1) {
    PciePreMemConfig->RootPort[0].Perst = S_GPIO_191; // WiFi
    PciePreMemConfig->RootPort[1].Perst = S_GPIO_192; // Wigi
    PciePreMemConfig->RootPort[2].Perst = 0;
    PciePreMemConfig->RootPort[3].Perst = 0;
    PciePreMemConfig->RootPort[4].Perst = 0;
    PciePreMemConfig->RootPort[5].Perst = 0;
  }
#endif
  //
  // Install ScPreMemPolicyPpi.
  // While installed, RC assumes the Policy is ready and finalized. So please
  // update and override any setting before calling this function.
  //
  Status = ScInstallPreMemPolicyPpi (ScPreMemPolicy);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  This function performs SA PreMem Policy initialization.

  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI ERRORS             The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
PeiSaPreMemPolicyInit (
  VOID
  )
{
  EFI_STATUS             Status;
  SI_SA_POLICY_PPI       *SaPolicyPpi;
  SA_PRE_MEM_CONFIG      *SaPreMemConfig = NULL;
  NPK_PRE_MEM_CONFIG     *NpkPreMemConfig = NULL;
  //
  // Call SaCreatePreMemConfigBlocks to initialize SA Policy structure
  // and get all Intel default policy settings.
  //
  Status = SaCreatePreMemConfigBlocks (&SaPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  //
  // Update and override all platform related and customized settings below.
  //
  Status = GetConfigBlock ((VOID *) SaPolicyPpi, &gSaPreMemConfigGuid, (VOID *) &SaPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SaPolicyPpi, &gNpkPreMemConfigGuid, (VOID *) &NpkPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Update Npk Pre-mem policies with setup values
  //
  UpdateNpkPreMemPolicy(NpkPreMemConfig);


  //
  // Update SA Pre-mem policies with setup values
  //
  UpdateSaPreMemPolicy (SaPreMemConfig);

  //
  // Install SaPreMemPolicyPpi.
  // While installed, RC assumes the Policy is ready and finalized. So please
  // update and override any setting before calling this function.
  //
  Status = SaInstallPreMemPolicyPpi (SaPolicyPpi);
  ASSERT_EFI_ERROR (Status);


  return Status;
}
//[-start-151224-IB07220029-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
EFI_STATUS
EFIAPI
GetStage2FromRecoveryImage (
  IN const EFI_PEI_SERVICES           **PeiServices
  )
{
  EFI_STATUS                            Status;
  FAST_RECOVERY_DXE_TO_PEI_DATA         *DxeToPeiData;
  UINTN                                 RecoveryCapsuleSize;
  VOID                                  *Buffer;
  EFI_FIRMWARE_VOLUME_HEADER            *FvHeader;
  UINTN                                 FvHeaderAddress;
  BOOLEAN                               FindFv;
  EFI_FIRMWARE_VOLUME_EXT_HEADER        *FwVolExHeaderInfo;

  Status = (*PeiServices)->LocatePpi (
                                  PeiServices, 
                                  &gEmuPeiPpiGuid, 
                                  0,
                                  NULL,
                                  (VOID **)&DxeToPeiData
                                  );
  if (Status == EFI_NOT_FOUND) {
    return Status;
  }

  RecoveryCapsuleSize = (UINTN)DxeToPeiData->RecoveryImageSize;
  Buffer = (VOID *)(UINTN)DxeToPeiData->RecoveryImageAddress;
  FvHeaderAddress = (UINTN)Buffer;
  do {
    FindFv = FALSE;
    FvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)FvHeaderAddress;
    if (FvHeader->Signature == EFI_FVH_SIGNATURE) {
      if (CompareGuid (&gEfiFirmwareFileSystemGuid, &FvHeader->FileSystemGuid)) {
        if (FvHeader->BlockMap[0].NumBlocks == 1) {
          if (FvHeader->ExtHeaderOffset != 0) {
            FwVolExHeaderInfo = (EFI_FIRMWARE_VOLUME_EXT_HEADER*)(((UINT8 *)FvHeader) + FvHeader->ExtHeaderOffset);
            if (!CompareGuid(&FwVolExHeaderInfo->FvName, &gH2OFlashMapRegionPeiFvGuid)) {
              FvHeaderAddress += (UINTN)FvHeader->FvLength;
              continue;
            }
          }
          FindFv = TRUE;
          BuildFvHob (FvHeaderAddress, FvHeader->FvLength);
          FvHeaderAddress += (UINTN)FvHeader->FvLength;
        }
      }

      //
      // File System Guid 2
      //
      if ((CompareGuid (&gEfiFirmwareFileSystem2Guid, &FvHeader->FileSystemGuid)) || 
          (CompareGuid (&gEfiFirmwareFileSystem3Guid, &FvHeader->FileSystemGuid))) {  
          if (FvHeader->ExtHeaderOffset != 0) {
            FwVolExHeaderInfo = (EFI_FIRMWARE_VOLUME_EXT_HEADER*)(((UINT8 *)FvHeader) + FvHeader->ExtHeaderOffset);
            if (!CompareGuid(&FwVolExHeaderInfo->FvName, &gH2OFlashMapRegionPeiFvGuid)) {
              FvHeaderAddress += (UINTN)FvHeader->FvLength;
              continue;
            }
          }
          FindFv = TRUE;
          BuildFvHob ( FvHeaderAddress, FvHeader->FvLength);
          FvHeaderAddress += (UINTN)FvHeader->FvLength;
      }
    }
    if (!FindFv) {
      FvHeaderAddress += 1;
    }
  } while (((FvHeaderAddress - (UINTN)Buffer)+ sizeof (EFI_FIRMWARE_VOLUME_HEADER)) < RecoveryCapsuleSize);
  return EFI_SUCCESS;
}
#endif
//[-end-161123-IB07250310-modify]//
//[-end-151224-IB07220029-add]//

#if (ENBDT_PF_ENABLE == 1)
// DDR SSC
EFI_STATUS
EFIAPI
PeiDDRSSCInit (
  VOID
  )
{
  EFI_STATUS                      Status;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
//[-end-160803-IB07220122-add]//
  SSC_IPC_BUFFER                  WBuf;
  UINT32                          BufferSize = 0;
  CHIPSET_CONFIGURATION            SystemConfiguration;
  UINTN                           VariableSize;
  //static table for the SSC settings (corresponding with the SSC settings 0~-0.5%, 0.1% stepping)
  //Modulation Freq = 32KHz
  SSC_SETTING                     SSC_Select_Table[] = {{No_SSC, 0x12B, 0},
                                                        {M01_SSC, 0x12B, 0x1062},
                                                        {M02_SSC, 0x12B, 0x2BB0},
                                                        {M03_SSC, 0x12B, 0x46FF},
                                                        {M04_SSC, 0x12B, 0x624D},
                                                        {M05_SSC, 0x12B, 0x7D9C}};

  //static table for the clock bending settings (corresponding with the clock bending settings 1.3%, 0.6%, 0, -0.9%)
  CLOCK_BENDING_SETTING           CLK_Bending_Table[] = {{Clk_Bending_13, 0xA00000, 0x7E},
                                                         {Clk_Bending_06, 0xC00000, 0x7D},
                                                         {No_Clk_Bending, 0x0, 0x7D},
                                                         {Clk_Bending_M09, 0xDB6C20, 0x7B}};

  // default value of the 4 SSC setting registers
  WBuf.LJ1PLL_CTRL_1.Data = LJ1PLL_CR_RW_CONTROL_1_DEFAULT;
  WBuf.LJ1PLL_CTRL_2.Data = LJ1PLL_CR_RW_CONTROL_2_DEFAULT;
  WBuf.LJ1PLL_CTRL_3 = LJ1PLL_CR_RW_CONTROL_3_DEFAULT;
  WBuf.LJ1PLL_CTRL_5.Data = LJ1PLL_CR_RW_CONTROL_5_DEFAULT;
  BufferSize = sizeof (UINT32) * 4;

//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigDataPreMem(&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Setup Variable is not ready for SSC setting! Used default value!!\n\r"));
    // Set default value of SSC
    WBuf.LJ1PLL_CTRL_2.Fields.ssc_cyc_to_peak_m1 = SSC_Select_Table[SSC_DEFAULT_SETTING].Ssc_Cyc_To_Peak;
    WBuf.LJ1PLL_CTRL_2.Fields.ssc_frac_step = SSC_Select_Table[SSC_DEFAULT_SETTING].Ffs_Frac_Step;
    // Set default value of Clock bending
    WBuf.LJ1PLL_CTRL_5.Fields.pll_ratio_frac = CLK_Bending_Table[CLK_BENDING_DEFAULT_SETTING].Pll_Ratio_Frac;
    WBuf.LJ1PLL_CTRL_5.Fields.pll_ratio_int = CLK_Bending_Table[CLK_BENDING_DEFAULT_SETTING].Pll_Ratio_Int;

    // send the IPC command for SSC
    Status = IpcSendCommandEx(IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_SSC_APPLY_NOW, &WBuf, BufferSize);

    // Delay for 1ms to avoid the SSC doesn't set correctly sometimes
    MicroSecondDelay (1000);

    // set the ssc_en to Disable!
    WBuf.LJ1PLL_CTRL_1.Fields.ssc_en = SSC_DISABLE;
    WBuf.LJ1PLL_CTRL_1.Fields.ssc_en_ovr = SSC_DISABLE;
    Status = IpcSendCommandEx(IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_SSC_APPLY_NOW, &WBuf, BufferSize);
    return Status;
  }


  if (SystemConfiguration.DDRSSCEnable) {
    // get the correct register values of the SSC setting
    WBuf.LJ1PLL_CTRL_2.Fields.ssc_cyc_to_peak_m1 = SSC_Select_Table[SystemConfiguration.DDRSSCSelection].Ssc_Cyc_To_Peak;
    WBuf.LJ1PLL_CTRL_2.Fields.ssc_frac_step = SSC_Select_Table[SystemConfiguration.DDRSSCSelection].Ffs_Frac_Step;
    // get the correct register values of the clock bending setting
    WBuf.LJ1PLL_CTRL_5.Fields.pll_ratio_frac = CLK_Bending_Table[SystemConfiguration.DDRCLKBending].Pll_Ratio_Frac;
    WBuf.LJ1PLL_CTRL_5.Fields.pll_ratio_int = CLK_Bending_Table[SystemConfiguration.DDRCLKBending].Pll_Ratio_Int;

    // send the IPC command for SSC settings
    Status = IpcSendCommandEx(IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_SSC_APPLY_NOW, &WBuf, BufferSize);

    // Delay for 1ms to avoid the SSC doesn't set correctly sometimes
    MicroSecondDelay (1000);

    // set the ssc_en and ssc_en_ovr to Enable!
    WBuf.LJ1PLL_CTRL_1.Fields.ssc_en = SSC_ENABLE;
    WBuf.LJ1PLL_CTRL_1.Fields.ssc_en_ovr = SSC_ENABLE;

    // send the IPC command for SSC EN
    Status = IpcSendCommandEx(IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_SSC_APPLY_NOW, &WBuf, BufferSize);
  } else {
    // get the correct register values of the clock bending setting
    WBuf.LJ1PLL_CTRL_5.Fields.pll_ratio_frac = CLK_Bending_Table[SystemConfiguration.DDRCLKBending].Pll_Ratio_Frac;
    WBuf.LJ1PLL_CTRL_5.Fields.pll_ratio_int = CLK_Bending_Table[SystemConfiguration.DDRCLKBending].Pll_Ratio_Int;

    Status = IpcSendCommandEx (IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_SSC_APPLY_NOW, &WBuf, BufferSize);
    return Status;
  }

  return Status;
}

// USB3, PCie, SATA, eDP, DP, eMMC, SD and SDIO SSC
EFI_STATUS
EFIAPI
PeiHighSpeedSerialInterfaceSSCInit (
  VOID
  )
{
  EFI_STATUS                        Status;
//[-start-161216-IB03090437-modify]//
  SSC_LCPLL_IPC_BUFFER              WBuf;
  UINT8                             BufferSize = 0;
//[-end-161216-IB03090437-modify]//
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariableServices;
//[-end-160803-IB07220122-add]//
  CHIPSET_CONFIGURATION           SystemConfiguration;
  UINTN                             VariableSize;
  //static table for the SSC settings (corresponding with the SSC settings 0~-0.5%, 0.1% stepping)
  //Modulation Freq = 32KHz
  SSC_SETTING                     HSSIO_SSC_Select_Table[] = {{ No_SSC, 0x12B, 0 },
                                                              { M01_SSC, 0x12B, 0x1062 },
                                                              { M02_SSC, 0x12B, 0x2BB0 },
                                                              { M03_SSC, 0x12B, 0x46FF },
                                                              { M04_SSC, 0x12B, 0x624D },
                                                              { M05_SSC, 0x12B, 0x7D9C }};
  //Read LCPLL registers for default register settings.
//[-start-161216-IB03090437-modify]//
  WBuf.LCPLL_CTRL_1.Data = LCPLL_CR_RW_CONTROL_1_DEFAULT;
  WBuf.LCPLL_CTRL_2.Data = LCPLL_CR_RW_CONTROL_2_DEFAULT;
  BufferSize = sizeof (UINT32) * 2;
//[-end-161216-IB03090437-modify]//
  DEBUG ((DEBUG_INFO, "HSSIO : Default value of LCPLL_CTRL_1 register: 0x%x\n LCPLL_CTRL_2 register: 0x%x\n", WBuf.LCPLL_CTRL_1.Data,WBuf.LCPLL_CTRL_2.Data));

//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigDataPreMem (&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//
  if (EFI_ERROR (Status)) {
//[-start-161216-IB03090437-modify]//
    DEBUG((DEBUG_ERROR, "\nHSSIO : Setup Variable is not ready for SSC setting! Leave the default system HSSIO SSC settings!!\n\r"));
//[-end-161216-IB03090437-modify]//
    return EFI_SUCCESS;
  }

//[-start-161216-IB03090437-modify]//
  if (SystemConfiguration.HSSIOSSCEnable) {
    //High Speed Serial IO SSC enable
    WBuf.LCPLL_CTRL_2.Fields.ssc_cyc_to_peak_m1 = HSSIO_SSC_Select_Table[SystemConfiguration.HSSIOSSCSelection].Ssc_Cyc_To_Peak;
    WBuf.LCPLL_CTRL_2.Fields.ssc_frac_step = HSSIO_SSC_Select_Table[SystemConfiguration.HSSIOSSCSelection].Ffs_Frac_Step;
    DEBUG ((DEBUG_INFO, "HSSIO enable : write LCPLL_CTRL_2 register: 0x%x\n", WBuf.LCPLL_CTRL_2.Data));

    WBuf.LCPLL_CTRL_1.Fields.ssc_en_ovr = SSC_ENABLE;
    WBuf.LCPLL_CTRL_1.Fields.ssc_en = SSC_ENABLE;
    DEBUG ((DEBUG_INFO, "HSSIO enable : write LCPLL_CTRL_1 register: 0x%x\n", WBuf.LCPLL_CTRL_1.Data));

    Status = IpcSendCommandEx(IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_LCPLL_APPLY_NOW, &WBuf, BufferSize);
  } else {
    //High Speed Serial IO SSC disable
    WBuf.LCPLL_CTRL_1.Fields.ssc_en = SSC_DISABLE;
    WBuf.LCPLL_CTRL_1.Fields.ssc_en_ovr = SSC_ENABLE;
    DEBUG ((DEBUG_INFO, "HSSIO disable : write LCPLL_CTRL_1 register: 0x%x\n", WBuf.LCPLL_CTRL_1.Data));
    Status = IpcSendCommandEx(IPC_CMD_ID_EMI_RFI_SUPPORT, IPC_SUBCMD_ID_LCPLL_APPLY_NOW, &WBuf, BufferSize);
  }
//[-end-161216-IB03090437-modify]//
  return EFI_SUCCESS;
}
#endif

/**
  This is the entry point of PEIM

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS if it completed successfully.
**/
EFI_STATUS
EFIAPI
PlatformInitPreMemEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_PLATFORM_INFO_HOB            PlatformInfo;
  EFI_STATUS                       Status = EFI_SUCCESS;
  EFI_PEI_PPI_DESCRIPTOR           *PeiPpiDescriptor;
  FIRMWARE_SEC_PERFORMANCE         Performance;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariableServices;
//[-end-160803-IB07220122-add]//
  VOID                             *Memory;
  IA32_DESCRIPTOR                  GdtDscriptor;
  UINT32                           Temp32;
  UINT32                           IfwiVerAddr;
  DRAM_POLICY_PPI                  *DramPolicy;
  EFI_PEI_PPI_DESCRIPTOR           *NewPeiPpiDescriptor;
  EFI_BOOT_MODE                    BootMode;
//[-start-160616-IB07220100-modify]//
  CarMapStruc               *CarMap = NULL;
//[-end-160616-IB07220100-modify]//
  CHIPSET_CONFIGURATION     SystemConfiguration;
//[-start-161128-IB11270169-add]//
  HECI_FWS_REGISTER                CseFirmwareStatus;
  UINTN                            HeciStatusRegisterAddress;
  EFI_HOB_GUID_TYPE                *FdoEnabledGuidHob = NULL;
//[-end-161128-IB11270169-add]//
  UINTN                            VariableSize;
  EFI_PEI_HOB_POINTERS             Hob;
  EFI_PLATFORM_INFO_HOB            *PlatformInfoPtr;
  UINT64                           StartTimerTicker = 0;
  UINT64                           Tick;
  UINTN                            AcpiVarHobSize;
  #if (ENBDT_PF_ENABLE == 1)
  MBP_CURRENT_BOOT_MEDIA           BootMediaData;
  #endif
#if (TABLET_PF_ENABLE == 1)
  UINT8                     Data8;
#endif
  BOOLEAN              *BiosFirstBootFlagBuffer = NULL;
  BIOS_FIRST_BOOT_HOB  *BiosReservedMemoryHob = NULL;
//[-start-170518-IB07400867-add]//
  UINTN                            Index;
//[-end-170518-IB07400867-add]//

  Status = PeiServicesRegisterForShadow (FileHandle);

  if (Status == EFI_ALREADY_STARTED) {
    ImageInMemory = TRUE;
  } else if (Status == EFI_NOT_FOUND) {
    ASSERT_EFI_ERROR (Status);
  }

//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_START);
//[-end-160217-IB07400702-add]//
  if (!ImageInMemory) {
    //
    // Since PEI has no PCI enumerator, set the BAR & I/O space enable ourselves
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_SC_BASE_INIT);
//[-end-160217-IB07400702-add]//
    ScBaseInit ();

//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_GPIO_INIT);
//[-end-160217-IB07400702-add]//
    MultiPlatformGpioProgramPreMem (&StartTimerTicker);
  }
//[-start-160510-IB03090427-remove]//
//  Status =  InstallMonoStatusCode (FileHandle, PeiServices);
//[-end-160510-IB03090427-remove]//
  AsmReadGdtr (&GdtDscriptor);
  DEBUG ((DEBUG_INFO, "GdtDscriptor Base Address:0x%X\n", (UINT32) GdtDscriptor.Base));
  
//[-start-190521-IB16530023-remove]//
////[-start-190226-IB07401085-modify]//
//#ifndef IOTG_SIC_RC_CODE_SUPPORTED
//[-end-190521-IB16530023-remove]//
  //
  // Get MBP data from CSE and creates Dedicated HOB.
  //
  if (ImageInMemory) {
    Status = PeiServicesGetBootMode (&BootMode);
    if (Status == EFI_SUCCESS && BootMode != BOOT_ON_S3_RESUME && BootMode != BOOT_IN_RECOVERY_MODE) {
      Status = HeciMBP ();
      ASSERT_EFI_ERROR (Status);
    }
  }
//[-start-190521-IB16530023-remove]//
//#endif
////[-end-190226-IB07401085-modify]//
//[-end-190521-IB16530023-remove]//

  PERF_START_EX (NULL, NULL, NULL, 0, 0x9100);
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_START);
//[-end-160217-IB07400702-add]//
  SeCUmaEntry(FileHandle, PeiServices);
  PERF_END_EX (NULL, NULL, NULL, 0, 0x9101);

  Status = PeiScPreMemPolicyInit (&StartTimerTicker);
  ASSERT_EFI_ERROR (Status);
  
//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_SI_POLICY_INIT);
//[-end-160217-IB07400702-add]//
  Status = PeiSiPolicyInit ();
  ASSERT_EFI_ERROR (Status);
//[-start-160420-IB03090426-add]//
  //
  // Init SPI PPI for GetStage2FromSPICallback function
  //
//  ScInitPrePolicy ();
//[-end-160420-IB03090426-add]//

  if (!ImageInMemory) {
    if (GdtDscriptor.Base >= 0xFE000000) {
      IfwiVerAddr = GdtDscriptor.Base;
      IfwiVerAddr &= 0xfffff000;  // 4K alignment to get IBBL base address.
      IfwiVerAddr +=0x1000;  // the address of IBBL end
      for (Temp32 = 0; Temp32 < 0x8000; Temp32 += 0x10){
        CarMap = (CarMapStruc *)(IfwiVerAddr-Temp32);
        if (CarMap->Sign == SIGNATURE_32 ('$','S','I','G')) {
          DEBUG ((DEBUG_INFO, "CarMap Address:0x%X\n", (UINT32) CarMap));
//[-start-160516-IB07400729-add]//
          DEBUG ((DEBUG_INFO, "CarMap CarBase         :0x%08x\n", (UINT32) CarMap->CarBase));
          DEBUG ((DEBUG_INFO, "CarMap CarSize         :0x%08x\n", (UINT32) CarMap->CarSize));
          DEBUG ((DEBUG_INFO, "CarMap IBBSource       :0x%08x\n", (UINT32) CarMap->IBBSource));
          DEBUG ((DEBUG_INFO, "CarMap IBBBase         :0x%08x\n", (UINT32) CarMap->IBBBase));
          DEBUG ((DEBUG_INFO, "CarMap IBBSize         :0x%08x\n", (UINT32) CarMap->IBBSize));
          DEBUG ((DEBUG_INFO, "CarMap IBBLSource      :0x%08x\n", (UINT32) CarMap->IBBLSource));
          DEBUG ((DEBUG_INFO, "CarMap IBBLBase        :0x%08x\n", (UINT32) CarMap->IBBLBase));
          DEBUG ((DEBUG_INFO, "CarMap IBBLSize        :0x%08x\n", (UINT32) CarMap->IBBLSize));
          DEBUG ((DEBUG_INFO, "CarMap FITBase         :0x%08x\n", (UINT32) CarMap->FITBase));
//[-start-160830-IB07400776-modify]//
          DEBUG ((DEBUG_INFO, "CarMap TempRamBase     :0x%08x\n", (UINT32) CarMap->TempRamBase));
          DEBUG ((DEBUG_INFO, "CarMap TempRamBase     :0x%08x\n", (UINT32) CarMap->TempRamBase));
//[-end-160830-IB07400776-modify]//
          DEBUG ((DEBUG_INFO, "CarMap HostToCse       :0x%08x\n", (UINT32) CarMap->HostToCse));
          DEBUG ((DEBUG_INFO, "CarMap CseToHost       :0x%08x\n", (UINT32) CarMap->CseToHost));
          DEBUG ((DEBUG_INFO, "CarMap ChunkIndex      :0x%08x\n", (UINT32) CarMap->ChunkIndex));
          DEBUG ((DEBUG_INFO, "CarMap NumberOfChunks  :0x%08x\n", (UINT32) CarMap->NumberOfChunks));
          DEBUG ((DEBUG_INFO, "CarMap IbbSizeLeft     :0x%08x\n", (UINT32) CarMap->IbbSizeLeft));
          DEBUG ((DEBUG_INFO, "CarMap Chunksize       :0x%08x\n", (UINT32) CarMap->Chunksize));
          DEBUG ((DEBUG_INFO, "CarMap IbblPerfRecord0 :0x%l016x\n", (UINT64) CarMap->IbblPerfRecord0));
          DEBUG ((DEBUG_INFO, "CarMap IbblPerfRecord1 :0x%l016x\n", (UINT64) CarMap->IbblPerfRecord1));
          DEBUG ((DEBUG_INFO, "CarMap IbblPerfRecord2 :0x%l016x\n", (UINT64) CarMap->IbblPerfRecord2));
          DEBUG ((DEBUG_INFO, "CarMap IbblPerfRecord3 :0x%l016x\n", (UINT64) CarMap->IbblPerfRecord3));
          DEBUG ((DEBUG_INFO, "CarMap IbblPerfRecord4 :0x%l016x\n", (UINT64) CarMap->IbblPerfRecord4));
          DEBUG ((DEBUG_INFO, "CarMap IbblPerfRecord5 :0x%l016x\n", (UINT64) CarMap->IbblPerfRecord5));
//[-end-160516-IB07400729-add]//
//[-start-170518-IB07400867-add]//
          DEBUG ((DEBUG_INFO, "CarMap BootPolicyExist :0x%08x\n", (UINT32) CarMap->BootPolicyExist));
          DEBUG ((DEBUG_INFO, "CarMap TxeHashExist    :0x%08x\n", (UINT32) CarMap->TxeHashExist));
          DEBUG ((DEBUG_INFO, "CarMap BootGuard       :0x%08x\n", (UINT32) CarMap->BootGuard));
          DEBUG ((DEBUG_INFO, "CarMap TxeHash         :\n"));
          for (Index = 0; Index < 32; Index++) {
            DEBUG ((DEBUG_INFO, "0x%02x ", (UINT8) CarMap->TxeHash[Index]));
            if (((Index + 1) % 16) == 0) {
              DEBUG ((DEBUG_INFO, "\n"));
            }
          }
//[-end-170518-IB07400867-add]//
          break;
        }
      }
    }

    //
    // After ScBaseInit(), Check the PlatformID, if invalid then set to Safe_Warning_Value.
    // Safe_Warning_Value for PcdIafwPlatformInfo = Real_Silicon + Max_RevId
    //
    if (PcdGet8 (PcdPlatformIdRegisterOffset) != 0) {
      Temp32 = MmioRead32 (0xFF03A02C );  //Simics PlatId w/a for BXT
    } else {
      Temp32 = MmioRead32 (PcdGet32(PcdPmcSsramBaseAddress0) + PcdGet8 (PcdPlatformIdRegisterOffset));
    }
    if (Temp32 == 0 || Temp32 == 0xFFFFFFFF) {
      PcdSet32 (PcdIafwPlatformInfo, 0x0000FF00);
      DEBUG ((DEBUG_INFO, "Warning: PcdIafwPlatformInfo set to Safe_Warning_Value\n"));
    } else {
      PcdSet32 (PcdIafwPlatformInfo, Temp32);
    }
    DEBUG ((DEBUG_INFO, "PcdIafwPlatformInfo:0x%X  PlatID:0x%X\n", PcdGet32 (PcdIafwPlatformInfo), PLATFORM_ID));

    //
    // Initialize PlatformInfo HOB
    //
    ZeroMem (&PlatformInfo, sizeof (PlatformInfo));

    PlatformInfo.SsidSvid = (UINT32)CarMap;

//[-start-170518-IB07400867-add]//
    CopyMem(PlatformInfo.TxeMeasurementHash, CarMap->TxeHash , EFI_ACPI_HROT_FW_DATA_LEN);
    PlatformInfo.BootGuard = CarMap->BootGuard;
//[-end-170518-IB07400867-add]//

//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_READ_BXT_IDS);
//[-end-160217-IB07400702-add]//
//[-start-160406-IB07400715-modify]//
#ifdef APOLLOLAKE_CRB
    //
    // CRB Board Ids
    //
//#if BXTI_PF_ENABLE == 1
////[-start-170516-IB08450375-add]//
//    CopyMem(PlatformInfo.TxeMeasurementHash, CarMap->TxeHash , EFI_ACPI_HROT_FW_DATA_LEN);
//    PlatformInfo.BootGuard = CarMap->BootGuard;
////[-end-170516-IB08450375-add]//
    Status = ReadBxtIPlatformIds (PeiServices, &PlatformInfo);
//#else
//    Status = ReadBxtPlatformIds(PeiServices, &PlatformInfo);
//#endif

#else
    //
    // OEM Board ID (OEM services or Build Platform Ids)
    //
//[-start-160427-IB07400720-modify]//
    Status = OemSvcGetBoardFabIds(&PlatformInfo.BoardId, &PlatformInfo.BoardRev);
    if (Status == EFI_MEDIA_CHANGED) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcGetBoardFabIds, Status : %r\n", Status)); 
    } else {
      if (PcdGet8 (PcdBuildPlatformType) == BOARD_ID_APL_RVP_1A) {
        PlatformInfo.BoardId         = BOARD_ID_APL_RVP_1A;
        PlatformInfo.BoardRev        = 0; //FABA
      } 
      if (PcdGet8 (PcdBuildPlatformType) == BOARD_ID_APL_RVP_2A) {
        PlatformInfo.BoardId         = BOARD_ID_APL_RVP_2A;
        PlatformInfo.BoardRev        = 0; //FABA
      } 
      if (PcdGet8 (PcdBuildPlatformType) == BOARD_ID_OXH_CRB) {
        PlatformInfo.BoardId         = BOARD_ID_OXH_CRB;
        PlatformInfo.BoardRev        = 0; //FABA
      } 
      if (PcdGet8 (PcdBuildPlatformType) == BOARD_ID_LFH_CRB) {
        PlatformInfo.BoardId         = BOARD_ID_LFH_CRB;
        PlatformInfo.BoardRev        = 0; //FABA
      } 
      if (PcdGet8 (PcdBuildPlatformType) == BOARD_ID_JNH_CRB) {
        PlatformInfo.BoardId         = BOARD_ID_JNH_CRB;
        PlatformInfo.BoardRev        = 0; //FABA
      } 
    }
//[-end-160427-IB07400720-modify]//
    
    DEBUG((DEBUG_INFO, "BoardId:  [0x%08x]\n", PlatformInfo.BoardId));
    DEBUG((DEBUG_INFO, "FabId:    [0x%08x]\n", PlatformInfo.BoardRev));

#endif
//[-end-160406-IB07400715-modify]//

    #ifndef VP_BIOS_ENABLE
    ASSERT_EFI_ERROR (Status);
    #endif
    //
    // Build HOB for PlatformInfo
    //
    BuildGuidDataHob (
      &gEfiPlatformInfoGuid,
      &PlatformInfo,
      sizeof (EFI_PLATFORM_INFO_HOB)
      );
    //
    // Attempt to locate SMIP and publish its data to PPI's and PCDs.
    // Currently no reason to check Status, but could add in future.
    //
    // This currently installs gDramPolicyPpiGuid, but may move in future
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_SMIP_INIT);
//[-end-160217-IB07400702-add]//
    Status = SmipInit ((VOID *)CarMap->FITBase,PlatformInfo.BoardId);
    
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_ISH_GPIO);
//[-end-160217-IB07400702-add]//
//[-start-160720-IB03090429-modify]//
    MultiPlatformGpioUpdatePreMem ();
//[-end-160720-IB03090429-modify]//
    //Print out Patch version string (BXT)
    AsmWriteMsr64 (0x8B, 0);
    AsmCpuid (0x1, NULL, NULL, NULL, NULL);
    Temp32 = (UINT32)(AsmReadMsr64 (0x8B) >> 32);
    DEBUG ((DEBUG_INFO, "PatchInfo:  0x%08x ", Temp32 ));
    DEBUG ((DEBUG_INFO, "%08x \n", (UINT32)(AsmReadMsr64 (0x8B))));

//[-start-160531-IB07220089-add]//
    //
    // Enable the upper 128-byte bank of RTC RAM.
    // PCR [RTC] + 0x3400 [2] = 1
    //
    SideBandAndThenOr32 (
      PID_RTC, 
      R_PCH_PCR_RTC_CONF, 
      ~0u, 
      B_PCH_PCR_RTC_CONF_UCMOS_EN
      );
  
    // Ensure UART2 is Unhidden
    SideBandAndThenOr32 (
      SB_PORTID_PSF3,
      R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2,
      ~(UINT32)(B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS),
      0
      );
//[-end-160531-IB07220089-add]//
  
    //
    // Set the new boot mode for MRC
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_UPDATE_BOOT_MODE);
//[-end-160217-IB07400702-add]//
    Status = UpdateBootMode (PeiServices, &PlatformInfo);
    ASSERT_EFI_ERROR (Status);
    
//    //
//    // Initialize MfgMemoryTest PPIs
//    //
//    Status = (*PeiServices)->InstallPpi (PeiServices, &mMfgMemTestPpi);
//    ASSERT_EFI_ERROR (Status);

    //
    // Setting 8254
    // Program timer 1 as refresh timer
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_SETTING_8254);
//[-end-160217-IB07400702-add]//
    IoWrite8 (0x43, 0x54);
    IoWrite8 (0x41, 0x12);

    //
    // RTC power failure handling
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_RTC_HANDLER);
//[-end-160217-IB07400702-add]//
    RtcPowerFailureHandler ();
    
//[-start-151221-IB11270139-add]//
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_OEM_EC_INIT);
//[-end-160217-IB07400702-add]//
    OemSvcEcInit();
    //
    // OemServices
    //
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_OEM_INIT_STAGE1);
//[-end-160217-IB07400702-add]//
    OemSvcInitPlatformStage1 ();
//[-end-151221-IB11270139-add]//
    Status = PeiSaPreMemPolicyInit();
    ASSERT_EFI_ERROR (Status);

    #if (ENBDT_PF_ENABLE == 1)
    if (GetBxtSeries() == BxtP) {
      // DDR SSC
      PeiDDRSSCInit ();
      // USB3, PCie, SATA, eDP, DP, eMMC, SD and SDIO SSC
      PeiHighSpeedSerialInterfaceSSCInit ();
    }
    #endif

    #if defined(PRAM_SUPPORT) || defined(SGX_SUPPORT)
    //
    // Install Ppi for BIOS reserved memory
    //
    Status = PeiServicesInstallPpi (&mBiosReservedMemoryPolicyPpi);
    #endif

//[-start-160803-IB07220122-add]//
    Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
//[-end-160803-IB07220122-add]//

    VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
    ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//     Status = GetSystemConfigDataPreMem(&SystemConfiguration, &VariableSize);
    Status = VariableServices->GetVariable (
                                 VariableServices,
                                 SETUP_VARIABLE_NAME,
                                 &gSystemConfigurationGuid,
                                 NULL,
                                 &VariableSize,
                                 &SystemConfiguration
                                 );
//[-end-160803-IB07220122-modify]//
    AcpiVarHobSize = sizeof (UINT64);
    BuildGuidDataHob (
      &gEfiAcpiVariableCompatiblityGuid,
      &SystemConfiguration.AcpiVariableSetCompatibility,
      sizeof (AcpiVarHobSize)
      );
    DEBUG ((DEBUG_INFO, "AcpiVariableAddr : 0x%08x\n", SystemConfiguration.AcpiVariableSetCompatibility));


#if (TABLET_PF_ENABLE == 1)
    // Read the MODEMCTRL level
    Data8 = PmicRead8 (DEVICE1_BASE_ADDRESS, WKC_MODEMCTRL_REG);
    if (SystemConfiguration.ModemSel == 3) { //PCIe 7360
      // Set SDWN_N and MODEMOFF to high to enable PCIe Modem device
      Data8 |= (WKC_MASK_SDWN_N|WKC_MASK_MODEMOFF);
      PmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_MODEMCTRL_REG, Data8);
      DEBUG ((DEBUG_INFO, "Set SDWN_N and MODEMOFF to high to enable PCIe Modem device \n"));
    } else if (SystemConfiguration.ModemSel != 0){ // SSIC modem
      // Clear SDWNDRV for SSIC modem
      Data8 &= ~ (WKC_MASK_SDWNDRV);
      PmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_MODEMCTRL_REG, Data8);
      DEBUG ((DEBUG_INFO, "Clear SDWNDRV for SSIC modem \n"));
    }
    DEBUG ((DEBUG_INFO, "Modem MODEMCTRL = %x\n", Data8));
#endif

    PERF_START_EX (NULL, "RstVctr", "IBBL", 1, 0x1000);
    Tick = CarMap->IbblPerfRecord0;
    PERF_END_EX (NULL, "RstVctr", "IBBL", Tick, 0x1001);

    PERF_START_EX (NULL, "InitNEM", "IBBL", Tick, 0x1010);
    Tick = CarMap->IbblPerfRecord1;
    PERF_END_EX (NULL, "InitNEM", "IBBL", Tick, 0x1011);

    PERF_START_EX (NULL, "IBBLSdw", "IBBL", Tick, 0x1020);
    Tick = CarMap->IbblPerfRecord2;
    PERF_END_EX (NULL, "IBBLSdw", "IBBL", Tick, 0x1021);

    PERF_START_EX (NULL, "IBBMLod", "IBBL", Tick, 0x1030);
    Tick = CarMap->IbblPerfRecord3;
    PERF_END_EX (NULL, "IBBMLod", "IBBL", Tick, 0x1031);

    PERF_START_EX (NULL, "IBBMVer", "IBBL", Tick, 0x1040);
    Tick = CarMap->IbblPerfRecord4;
    PERF_END_EX (NULL, "IBBMVer", "IBBL", Tick, 0x1041);

    //
    // Normal boot - build Hob for SEC performance data.
    //
    Performance.ResetEnd = GetTimeInNanoSecond(CarMap->IbblPerfRecord0);
    if (!EFI_ERROR (Status)) {
      BuildGuidDataHob (
        &gEfiFirmwarePerformanceGuid,
        &Performance,
        sizeof (FIRMWARE_SEC_PERFORMANCE)
      );
      DEBUG ((EFI_D_INFO, "FPDT: SEC Performance Hob ResetEnd = %ld\n", Performance.ResetEnd));
    }

  } else {  //PostMem
//[-start-151224-IB07220029-add]//
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_RECOVERY_CHECK);
//[-end-160217-IB07400702-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
    FastCrisisRecoveryCheck ();
#endif
//[-end-161123-IB07250310-modify]//
//[-end-151224-IB07220029-add]//
    Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
    ASSERT (Hob.Raw != NULL);
    PlatformInfoPtr = GET_GUID_HOB_DATA(Hob.Raw);
    CarMap =(CarMapStruc *) (UINT32)PlatformInfoPtr->SsidSvid;
    //
    // Locate and Reinstall necessary PPI's before MemoryCallback is run
    //
    Status = PeiServicesLocatePpi (
               &gDramPolicyPpiGuid,
               0,
               &PeiPpiDescriptor,
               NULL // PPI
               );

    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_ERROR, "Couldn't locate DRAM Policy PPI, LocatePpi returned %r.\n", Status));
    } else {
      DramPolicy          = (DRAM_POLICY_PPI *)        AllocateZeroPool (sizeof (DRAM_POLICY_PPI));
      NewPeiPpiDescriptor = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
      if ((DramPolicy == NULL) || (NewPeiPpiDescriptor == NULL)) {
        DEBUG ((DEBUG_ERROR, "Couldn't allocate memory for DRAM Policy PPI.\n"));
      } else {
        (*PeiServices)->CopyMem (
                          (VOID *) DramPolicy,
                          (VOID *) PeiPpiDescriptor->Ppi,
                          sizeof (DRAM_POLICY_PPI)
                          );
        NewPeiPpiDescriptor->Ppi = DramPolicy;
        NewPeiPpiDescriptor->Guid = &gDramPolicyPpiGuid;
        NewPeiPpiDescriptor->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
        Status = (**PeiServices).ReInstallPpi (
                                   PeiServices,
                                   PeiPpiDescriptor,
                                   NewPeiPpiDescriptor
                                   );
      }
    }

    if (GdtDscriptor.Base >= 0xFE000000) {
      (*PeiServices)->CopyMem (
                        (VOID *) mGdtTable,
                        (VOID *) GdtDscriptor.Base,
                        GdtDscriptor.Limit + 1
                        );
      GdtDscriptor.Base = (UINT32) mGdtTable;
      AsmWriteGdtr (&GdtDscriptor);
    }

//[-start-161128-IB11270169-add]//
    //
    // Set "Force Volatile Mode" in the variable driver
    // If Firmware Descriptor Override (FDO) boot is enabled OR
    // If CSE is in recovery mode
    //
    HeciStatusRegisterAddress = MmPciAddress (0, SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_SEC_FW_STS0);
    CseFirmwareStatus.ul = MmioRead32 (HeciStatusRegisterAddress);

    FdoEnabledGuidHob = GetFirstGuidHob (&gFdoModeEnabledHobGuid);
    if (FdoEnabledGuidHob != NULL || CseFirmwareStatus.r.SeCOperationState == SEC_IN_RECOVERY_MODE) {
      PcdSetBool (PcdForceVolatileVariable, TRUE);
    }
//[-end-161128-IB11270169-add]//
//    //
//    // locate the MfgMemoryTest PPI
//    //
//    Status = (*PeiServices)->LocatePpi (
//                               PeiServices,
//                               &gPeiMfgMemoryTestPpiGuid,  // GUID
//                               0,                          // INSTANCE
//                               &PeiPpiDescriptor,          // EFI_PEI_PPI_DESCRIPTOR
//                               NULL // PPI
//                               );
//    if (Status == EFI_SUCCESS) {
//      //
//      // Reinstall the MfgMemoryTest PPI
//      //
//      Status = (**PeiServices).ReInstallPpi (
//                                PeiServices,
//                                PeiPpiDescriptor,
//                                &mMfgMemTestPpi
//                                );
//    }

    //
    // locate the TemporaryRamSupport PPI
    //
    Status = PeiServicesLocatePpi (
               &gEfiTemporaryRamSupportPpiGuid,  // GUID
               0,                                // INSTANCE
               &PeiPpiDescriptor,                // EFI_PEI_PPI_DESCRIPTOR
               NULL // PPI
               );

    if (Status == EFI_SUCCESS) {
      //
      // Reinstall the TemporaryRamSupport PPI
      //
      Status = PeiServicesReInstallPpi (
                 PeiPpiDescriptor,
                 mPeiTemporaryRamSupportPpiPpi
                 );
    }

#ifndef FSP_WRAPPER_FLAG
    //
    // locate the PlatformInformation PPI
    //
    DEBUG ((DEBUG_INFO, "LocatePpi mPeiPlatformInformationPpiPpi\n"));
    Status = PeiServicesLocatePpi (
               &gEfiSecPlatformInformationPpiGuid,  // GUID
               0,                                // INSTANCE
               &PeiPpiDescriptor,                // EFI_PEI_PPI_DESCRIPTOR
               NULL // PPI
               );

    if (Status == EFI_SUCCESS) {
      DEBUG ((DEBUG_INFO, "ReInstallPpi mPeiPlatformInformationPpiPpi\n"));
      //
      // Reinstall the PlatformInformation PPI
      //
      Status = PeiServicesReInstallPpi (
                 PeiPpiDescriptor,
                 mPeiPlatformInformationPpiPpi
                 );
    }
#endif

#if defined(PRAM_SUPPORT) || defined(SGX_SUPPORT)
    //
    // locate the BiosReservedMemory PPI
    //
    Status = PeiServicesLocatePpi (
               &gBiosReservedMemoryPolicyPpiGuid,
               0,
               &PeiPpiDescriptor,
               NULL // PPI
               );

    if (Status == EFI_SUCCESS) {
      Status = PeiServicesReInstallPpi (
                 PeiPpiDescriptor,
                 &mBiosReservedMemoryPolicyPpi
                 );
    }
#endif

//[-start-151216-IB07220025-add]//
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gEfiPeiBootInRecoveryModePpiGuid,  // GUID
                               0,                          // INSTANCE
                               &PeiPpiDescriptor,   // EFI_PEI_PPI_DESCRIPTOR
                               NULL // PPI
                               );
    if (Status == EFI_SUCCESS) {
      //
      // Reinstall the PPI
      //
      DEBUG ((EFI_D_INFO, "\nReInstall BootInRecoveryMode PPI\n"));
      Status = (*PeiServices)->ReInstallPpi (
                                PeiServices,
                                PeiPpiDescriptor,
                                &mPpiListRecoveryBootMode
                                );
    }
//[-end-151216-IB07220025-add]//

    //
    // Initialize Stall PPIs
    //
    Status = PeiServicesInstallPpi (&mInstallStallPpi);
    ASSERT_EFI_ERROR (Status);

    Status = PeiServicesGetBootMode (&BootMode);
//[-start-170210-IB07400839-add]//
    if (BootMode == BOOT_IN_RECOVERY_MODE) {
      PcdSetBool (PcdForceVolatileVariable, TRUE);
    }
//[-end-170210-IB07400839-add]//
#if (ENBDT_PF_ENABLE == 1)
    if (BootMode == BOOT_ON_S3_RESUME) {
//[-start-160803-IB07220122-add]//
      Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
      if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
//[-end-160803-IB07220122-add]//
      VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
      ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//       Status = GetSystemConfigData(&SystemConfiguration, &VariableSize);
      Status = VariableServices->GetVariable (
                                   VariableServices,
                                   SETUP_VARIABLE_NAME,
                                   &gSystemConfigurationGuid,
                                   NULL,
                                   &VariableSize,
                                   &SystemConfiguration
                                   );
//[-end-160803-IB07220122-modify]//

      if (!EFI_ERROR (Status)) {
        SetMem (&BootMediaData, sizeof(MBP_CURRENT_BOOT_MEDIA), 0x0);
        switch (SystemConfiguration.CseBootDevice) {
          case 0:
            DEBUG ((DEBUG_INFO, "CSE Boot Device is EMMC.\n"));
            Status = (*PeiServices)->InstallPpi (PeiServices, mCseEmmcSelectPpiList);
            break;
          case 1:
            DEBUG ((DEBUG_INFO, "CSE Boot Device is UFS.\n"));
            Status = (*PeiServices)->InstallPpi (PeiServices, mCseUfsSelectPpiList);
            break;
          case 2:
            DEBUG ((DEBUG_INFO, "CSE Boot Device is SPI.\n"));
            Status = (*PeiServices)->InstallPpi (PeiServices, mCseSpiSelectPpiList);
            break;
          default:
            DEBUG ((EFI_D_ERROR, "\nCSE Boot device is unknown. Cannot continue!\n"));
            CpuDeadLoop();
            break;

        }
        BootMediaData.PhysicalData = SystemConfiguration.CseBootDevice;
        //
        // Build HOB for BootMediaData
        //
        BuildGuidDataHob (
          &gEfiBootMediaHobGuid,
          &BootMediaData,
          sizeof (MBP_CURRENT_BOOT_MEDIA)
          );

//[-start-160824-IB07220130-add]//
        PcdSet32(PcdFlashNvStorageVariableBase, SystemConfiguration.NvStorageVariableBase);
        PcdSetBool (PcdNvStorageHaveVariable, TRUE);
        DEBUG ((EFI_D_INFO, "NVStorageBase = 0x%x, PcdNvStorageHaveVariable = 0x%x\n", PcdGet32 (PcdFlashNvStorageVariableBase), PcdGetBool (PcdNvStorageHaveVariable)));
//[-end-160824-IB07220130-add]//
      }
    }

//[-start-160824-IB07220130-add]//
    if (BootMode == BOOT_ON_FLASH_UPDATE) {
      Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
      if (EFI_ERROR (Status)) {
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
      VariableSize = PcdGet32 (PcdSetupConfigSize);
      Status = VariableServices->GetVariable (
                                   VariableServices,
                                   SETUP_VARIABLE_NAME,
                                   &gSystemConfigurationGuid,
                                   NULL,
                                   &VariableSize,
                                   &SystemConfiguration
                                   );

      if (!EFI_ERROR (Status)) {
        PcdSet32 (PcdFlashNvStorageVariableBase, SystemConfiguration.NvStorageVariableBase);
        PcdSetBool (PcdNvStorageHaveVariable, TRUE);
        DEBUG ((EFI_D_INFO, "NVStorageBase = 0x%x, PcdNvStorageHaveVariable = 0x%x\n", PcdGet32 (PcdFlashNvStorageVariableBase), PcdGetBool (PcdNvStorageHaveVariable)));
      }
    }
//[-end-160824-IB07220130-add]//
#endif

//[-start-151224-IB07220029-modify]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
    if (!IsRunCrisisRecoveryMode()) {
#endif
//[-end-161123-IB07250310-modify]//
//[-end-151224-IB07220029-modify]//
      //
      // copy IBBM from Cache to DRAM. the hardcode address need to be changed
      // to use the parameter in IBBL.
      //
      Memory = AllocatePages ( EFI_SIZE_TO_PAGES(PcdGet32 (PcdFlashFvRecoverySize)));
      if (Memory != NULL) {
//[-start-161022-IB07400803-modify]//
//#if BXTI_PF_ENABLE
        if (IsIOTGBoardIds()) {
          CopyMem (Memory , (VOID *) CarMap->IBBBase, PcdGet32 (PcdFlashFvRecoverySize));
        } else {
//#else
          CopyMemSse4 (Memory, (VOID *)CarMap->IBBBase, PcdGet32 (PcdFlashFvRecoverySize));
        }
//#endif
//[-end-161022-IB07400803-modify]//
        DEBUG ((DEBUG_INFO, "IBBM address: %x\n", Memory));
        PeiServicesInstallFvInfoPpi (
          NULL,
          (VOID *)Memory,
          PcdGet32 (PcdFlashFvRecoverySize),
          NULL,
          NULL
          );
      } else  {
        ASSERT (FALSE);
      }
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
    }
#endif
//[-end-161123-IB07250310-modify]//

    DEBUG ((DEBUG_INFO, "PreMem Policy Init - Start\n"));
    //
    // Initialize Pre-Mem PEI Platform Policy
    //
    Status = PeiPolicyInitPreMem ();
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, "PreMem Policy Init - End\n\n"));


#ifndef FSP_WRAPPER_FLAG
//[-start-151224-IB07220029-modify]//
    if (!IsRunCrisisRecoveryMode()) {
      //
      // Register Notify callbacks after Shadow re-entry for gEfiPeiMemoryDiscoveredPpiGuid
      // All PPI Re-Installs need to be done before this.
      //
       Status = PeiServicesNotifyPpi (&mNotifyList[0]);
       ASSERT_EFI_ERROR (Status);
     }
//[-end-151224-IB07220029-modify]//

//[-start-161123-IB07250310-modify]//
  //
  // Register Notify Callbacks to handle additional FV discovery from NVM.
  // (Do this After notify MemoryCallback for performance reasons)
  //
  //      SPI - Notify on gCseSpiSelectPpiGuid.
  // eMMC/UFS - Notify on gEfiPeiVirtualBlockIoPpiGuid, which is installed after gCseEmmcSelectPpiGuid or gCseUfsSelectPpiGuid.
  //
//[-start-151224-IB07220029-modify]//
  Status = PeiServicesNotifyPpi (&mFvNotifyList[0]);
  ASSERT_EFI_ERROR (Status);
  if (IsRunCrisisRecoveryMode()) {
//[-start-160217-IB07400702-add]//
    CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_GET_STAGE2_IMAGE); 
//[-end-160217-IB07400702-add]//
    GetStage2FromRecoveryImage (PeiServices);
  }
//[-end-151224-IB07220029-modify]//
#else
    //
    // Register Notify Callback to process OBB loading.
    // In FSP+Wrapper, the MTRRs are set after TempRamExit, not gEfiPeiMemoryDiscoveredPpiGuid.
    //
    Status = PeiServicesNotifyPpi (&mFspTempRamExitList[0]);
    ASSERT_EFI_ERROR (Status);
#endif
//[-end-161123-IB07250310-modify]//

//[-start-151124-IB08450330-add]//
    //
    // Disable Enable InSMM.STS (EISS).  
    // (Should be initialized somewhere after DXE if the platform configured to 
    // lock BIOS)
    //  
    MmioAnd32 (
        MmPciAddress (
              0,
              DEFAULT_PCI_BUS_NUMBER_SC,
              PCI_DEVICE_NUMBER_SPI,
              PCI_FUNCTION_NUMBER_SPI,
              R_SPI_BCR
              ), 
        (UINT32)(~B_SPI_BCR_SMM_BWP)
        );
//[-end-151124-IB08450330-add]//

    BiosFirstBootFlagBuffer = AllocateZeroPool (sizeof (BOOLEAN));
    BiosReservedMemoryHob = BuildGuidHob (&gBiosFirstBootHobGuid, sizeof (BIOS_FIRST_BOOT_HOB));
    BiosReservedMemoryHob->BufferAddress = (UINT32)BiosFirstBootFlagBuffer;

  } //end PostMem

  DEBUG ((DEBUG_INFO, "PeiInitPlatform end\n"));

//[-start-160217-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_PLATFORM_INIT_PRE_MEM_END);
//[-end-160217-IB07400702-add]//

  return Status;
}

//[-start-151204-IB02950555-modify]//
//[-start-160406-IB07400715-modify]//
//#if BXTI_PF_ENABLE == 1
#ifdef APOLLOLAKE_CRB
//[-end-160406-IB07400715-modify]//
//
// Read Platform ID for IOTG Platforms
//
EFI_STATUS
ReadBxtIPlatformIds (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB  *PlatformInfoHob
  )
{
  EFI_STATUS                  Status;
  UINT8                       BoardId = 0;
  UINT8                       FabId = 0;
  BXT_CONF_PAD0               padConfg0;
  BXT_CONF_PAD1               padConfg1;
  IN UINT32                   CommAndOffset;

  DEBUG((DEBUG_INFO, "Port(0x62) = %02X\n", IoRead8 (0x62)));

  if (IoRead8(0x62) != 0xFF) {
//[-start-161206-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
//[-end-170111-IB07400832-modify]//
    PlatformInfoHob->ECPresent = 1;
//[-start-160406-IB07400715-modify]//
//    Status = GetBoardIdFabId(PeiServices, &BoardId, &FabId);
//
//    if (EFI_ERROR(Status)) {
//      DEBUG((EFI_D_INFO, "Could not get BoardId from EC, default to Mineral Hill\n"));
//      BoardId = BOARD_ID_MNH_RVP;
//    }
//
//    PlatformInfoHob->BoardId = BoardId;
//    PlatformInfoHob->BoardRev = FabId;
    //
    // EC exist, get platform Ids from EC
    //
    ReadBxtPlatformIds(PeiServices, PlatformInfoHob);
//[-end-160406-IB07400715-modify]//
#else
    //
    // EC/KSC support is disabled, 
    // We can not get the boardId, using builded BoardId.
    //
    PlatformInfoHob->ECPresent = 0;
    PlatformInfoHob->BoardId   = PcdGet8 (PcdBuildPlatformType);
    PlatformInfoHob->BoardRev  = 0; //FABA
#endif
//[-end-161206-IB07400822-modify]//

  } else {
    PlatformInfoHob->ECPresent = 0;
    Status = GetEmbeddedBoardIdFabId (PeiServices, &BoardId, &FabId);

    if (BoardId == 0x04) {
      // It might be APL or MH when the LPC WA is not implemented and EC is not working properly

      PlatformInfoHob->ECPresent = 1;
      // Using GPIO_113 (SVID strap) to differentiate APL/MH
      // APL: GPIO_113 -> GND
      // MH : GPIO_113 -> Float
      CommAndOffset = GetCommOffset (NORTHWEST, 0x0220);
      padConfg1.padCnf1 = GpioPadRead (CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
      padConfg1.r.Term = 0x0C;
      GpioPadWrite (CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);

      padConfg0.padCnf0 = GpioPadRead (CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
      padConfg0.r.PMode = 0;
      padConfg0.r.GPIORxTxDis = 0x1;
      GpioPadWrite (CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);

      padConfg0.padCnf0 = GpioPadRead (CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);

      if (padConfg0.r.GPIORxState) {
        DEBUG ((DEBUG_INFO, "I'm Mineral Hill\n\n"));
        PlatformInfoHob->BoardId = BOARD_ID_MNH_RVP;
      } else {
        DEBUG ((DEBUG_INFO, "I'm Apollo Lake \n\n"));
        PlatformInfoHob->BoardId = BOARD_ID_APL_RVP_1A;
      }
      PlatformInfoHob->BoardRev = 0;

      padConfg1.r.Term = 0x04;
      GpioPadWrite (CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);

    } else if (BoardId == BOARD_ID_OXH_CRB) {
      PlatformInfoHob->BoardId = BOARD_ID_OXH_CRB;
      PlatformInfoHob->BoardRev = FabId;
      DEBUG ((DEBUG_INFO, "I'm Oxbow Hill \n\n"));
    } else if (BoardId == BOARD_ID_LFH_CRB) {
      PlatformInfoHob->BoardId = BOARD_ID_LFH_CRB;
      PlatformInfoHob->BoardRev = FabId;
      DEBUG ((DEBUG_INFO, "I'm Leaf Hill \n\n"));
//[-start-160929-IB07400790-modify]//
    } else if (BoardId == BOARD_ID_JNH_CRB) {
//[-end-160929-IB07400790-modify]//
      PlatformInfoHob->BoardId = BOARD_ID_JNH_CRB;
      PlatformInfoHob->BoardRev = FabId;
      DEBUG ((DEBUG_INFO, "I'm Juniper Hill \n\n"));
    } else {

      Status = GetIVIBoardIdFabId (PeiServices, &BoardId, &FabId);

      if (BoardId == BOARD_ID_BFH_IVI) {
        PlatformInfoHob->BoardId = BOARD_ID_BFH_IVI;
        DEBUG ((DEBUG_INFO, "I'm Gordon Ridge BMP Interposer \n\n"));
      } else if (BoardId == BOARD_ID_MRB_IVI) {
        PlatformInfoHob->BoardId = BOARD_ID_MRB_IVI;
        DEBUG ((DEBUG_INFO, "I'm Gordon Ridge BMP MRB \n\n"));
      } else {
        //to be added
        DEBUG ((DEBUG_INFO, "BoardId form GPIO strap: %02X\n", BoardId));
      }
    }
  }

  DEBUG ((DEBUG_INFO, "BoardId:  [0x%08x]\n", PlatformInfoHob->BoardId));
  DEBUG ((DEBUG_INFO, "FabId:    [0x%08x]\n", PlatformInfoHob->BoardRev));

  return EFI_SUCCESS;
}

//[-start-160406-IB07400715-remove]//
//#else
//[-end-160406-IB07400715-remove]//

VOID
_10SecPause (
  VOID
  )
{
  UINT32 countdown = 10;
  DEBUG ((DEBUG_INFO, "DEBUG HALT, will continue in ...\n"));

  while (countdown > 0) {
    DEBUG ((DEBUG_INFO, "%d...", countdown));
    //One second delay
    MicroSecondDelay(1000000);
    countdown--;
  }

}

//
// Read Platform ID for All Non-IOTG Platforms
//
EFI_STATUS
ReadBxtPlatformIds (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB  *PlatformInfoHob
  )
{
  EFI_STATUS  Status;
  UINT8       BoardId         = 0;
  UINT8       FabId           = 0;
  UINT8       BomIdPss        = 0;
  UINT8       OsSelPss        = 0;
  UINT8       DockId          = 0;
#if (ENBDT_PF_ENABLE == 1)
  UINT8       EcMajorRevision = 0;
  UINT8       EcMinorRevision = 0;
#endif

  //
  // Pre-Determined Board IDs
  //
#if RVV_ENABLE == 1
  BoardId = BOARD_ID_BXT_RVV;
  DEBUG ((DEBUG_INFO, "I am RVV1 booting\n"));
  DEBUG ((DEBUG_INFO, "RVV does not support docking\n"));
#elif RVVP_ENABLE == 1
  BoardId = BOARD_ID_BXT_RVVP;
  DEBUG ((DEBUG_INFO, "I am RVVP booting\n"));
  DEBUG ((DEBUG_INFO, "RVVP does not support docking\n"));
  _10SecPause (); // TODO: Temporary add delay for MRC. Will Remove after validate on all platform.
#endif

  if (PLATFORM_ID != VALUE_REAL_PLATFORM) {
    BoardId = BOARD_ID_BXT_RVP;
  }

//[-start-161018-IB06740518-add]//
  //
  // Perform DetecEc before reading the Board ID to prevent sporadic S3 failures caused by EC BoardID command failure on APL RVP
  //
#if (ENBDT_PF_ENABLE == 1)
    PERF_START_EX (NULL, NULL, NULL, 0, 0xA100);
    if (DetectEc (&EcMajorRevision, &EcMinorRevision)) {
      PlatformInfoHob->EcMajorRevision = EcMajorRevision;
      PlatformInfoHob->EcMinorRevision = EcMinorRevision;
    }
    PERF_END_EX (NULL, NULL, NULL, 0, 0xA101);
#endif
//[-end-161018-IB06740518-add]//

  //
  // Read the Board ID if not pre-determined
  //
  if (BoardId == 0) {
//[-start-160128-IB11270146-modify]//
    Status = OemSvcGetBoardFabIds(&BoardId, &FabId);
    if (Status == EFI_MEDIA_CHANGED) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcGetBoardFabIds, Status : %r\n", Status)); 
      DEBUG((EFI_D_ERROR | EFI_D_INFO, "Platform BoardId:%x FabId%x\n", BoardId, FabId));
    } else {    
      // Get Board ID and FAB ID
      Status = GetBoardIdFabId(PeiServices, &BoardId, &FabId);
  
      if (EFI_ERROR(Status)) {
        BoardId = BOARD_ID_BXT_RVP;
        FabId = FAB_ID_D;
      }
    }
//[-end-160128-IB11270146-modify]//

    //
    // Read OS selection and BOM selection from PSS if not
    // an OEM SMIP build. Customer may not have PSS in an
    // OEM SMIP build.
    //
#if OEM_SMIP_BUILD == 0
    // If BomIdPss and OsSelPss are not found, try to set to Safe Value respectively.

    // Get BOM ID from PSS
    Status = GetBomIdPss (PeiServices, &BomIdPss);
    if (EFI_ERROR (Status)) {
      if (BoardId == BOARD_ID_BXT_FFD) {
        if (FabId == FAB_ID_A) {
           BomIdPss = PSS_BOM_1;  //EVT/PR0
        }else{
           BomIdPss = PSS_BOM_2;  //PR05 +
        }
      } else {
        BomIdPss = PSS_BOM_0;
      }
    }

    // Get OS Selection from PSS
    Status = GetOsSelPss (PeiServices, &OsSelPss);
    if (EFI_ERROR (Status)) {
      if (BoardId == BOARD_ID_BXT_FFD) {
        OsSelPss = PSS_AOS;
      } else {
        OsSelPss = PSS_WOS;
      }
    }

    // Get Dock ID from GPIO Expander
    if (BoardId == BOARD_ID_BXT_RVP) {
      Status = GetDockId (PeiServices, &DockId);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
#endif

//[-start-161018-IB06740518-remove]//
//#if (ENBDT_PF_ENABLE == 1)
//    if ((BoardId == BOARD_ID_APL_RVP_1A) || (BoardId == BOARD_ID_APL_RVP_2A) ||
//        (BoardId == BOARD_ID_APL_RVP_1C_LITE) ||(BoardId == BOARD_ID_MNH_RVP)) {
//      PERF_START_EX (NULL, NULL, NULL, 0, 0xA100);
//      if (DetectEc (&EcMajorRevision, &EcMinorRevision)) {
//        PlatformInfoHob->EcMajorRevision = EcMajorRevision;
//        PlatformInfoHob->EcMinorRevision = EcMinorRevision;
//      }
//      PERF_END_EX (NULL, NULL, NULL, 0, 0xA101);
//    }
//#endif
//[-end-161018-IB06740518-remove]//
  }

  PlatformInfoHob->BoardId  = BoardId;
  PlatformInfoHob->BoardRev = FabId;
  PlatformInfoHob->BomIdPss = BomIdPss;
  PlatformInfoHob->OsSelPss = OsSelPss;
  PlatformInfoHob->DockId   = DockId;

  DEBUG((DEBUG_INFO, "BoardId:  [0x%08x]\n", PlatformInfoHob->BoardId));
  DEBUG((DEBUG_INFO, "BoardRev: [0x%08x]\n", PlatformInfoHob->BoardRev));
  DEBUG((DEBUG_INFO, "BomIdPss: [0x%08x]\n", PlatformInfoHob->BomIdPss));
  DEBUG((DEBUG_INFO, "OsSelPss: [0x%08x]\n", PlatformInfoHob->OsSelPss));
  DEBUG((DEBUG_INFO, "DockId:   [0x%08x]\n", PlatformInfoHob->DockId));

  return EFI_SUCCESS;
}
#endif
//[-end-160517-IB03090427-modify]//
