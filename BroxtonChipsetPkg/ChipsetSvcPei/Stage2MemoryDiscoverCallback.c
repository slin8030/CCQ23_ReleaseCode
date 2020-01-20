/** @file
 PEI Chipset Services Library.

 This file contains only one function that is PeiCsSvcStage2MemoryDiscoverCallback().
 The function PeiCsSvcStage2MemoryDiscoverCallback() use chipset services to install
 Firmware Volume Hob's once there is main memory.
;******************************************************************************
;* Copyright (c) 2013-2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
//
// Libraries
//
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/BaseMemoryLib.h>
//
// GUID
//
#include <Guid/PlatformInfo.h>
#include <Guid/MemoryTypeInformation.h>

//
// Ppi
//
#include <Ppi/Cache.h>
#include <Ppi/ReadOnlyVariable2.h>

//registers
#include <PlatformBaseAddresses.h>
#include <ScAccess.h>
#include <CpuRegs.h>

#include <ChipsetSetupConfig.h>
#include <Library/MemoryAllocationLib.h>
#include <SaCommonDefinitions.h>
//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
#include <Private\ScPmcFunctionDisableResetHob.h>
#include <Private\CpuInitDataHob.h>
#include <ConfigBlock.h>
#include <Ppi\PowerMgmtConfig.h>
#include <CpuPowerMgmt.h>
#endif
//[-end-161123-IB07250310-add]//

typedef UINT32  EFI_BOOT_MODE;
//
// Need min. of 48MB PEI phase
//
#define  PEI_MIN_MEMORY_SIZE               (6 * 0x800000)
#define  PEI_RECOVERY_MIN_MEMORY_SIZE      (6 * 0x800000)

//
// Definition copied from MrcRegs.h. We don't include that header file
// due to its complicated dependency.
// NOTE: Remove it when RC implement it.
//
#define EC_BASE                             0xE0000000
#define CHV_CMD_READ_REG                    0x10  /**< Read Message Bus Register Command */
#define CHV_CMD_WRITE_REG                   0x11  /**< Write Message Bus Register Command */
#define CHV_UNIT_DUNIT0                     0x01  /**< D Unit Channel 0*/
#define MC_DRFC_OFFSET                      0x8

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 REFWMLO                      :4;   /**< Refresh Opportunistic Watermark */
        UINT32 REFWMHI                      :4;   /**< Refresh High Watermark*/
        UINT32 REFWMPNC                     :4;   /**< Refresh Panic Watermark */
        UINT32 tREFI                        :3;   /**< bit [14:12] Refresh Period */
        UINT32 reserved1                    :1;
        UINT32 REFCNTMAX                    :2;   /**< Refresh Max tREFI Interval */
        UINT32 reserved2                    :2;
        UINT32 REFSKEWDIS                   :1;   /**< tREFI counters */
        UINT32 REFDBTCLR                    :1;
        UINT32 reserved3                    :2;
        UINT32 CuRefRate                    :3;   /**< bit [26:24] */
        UINT32 reserved4                    :5;
    } field;
} RegDRFC;
#pragma pack()

VOID
Isb32Write (
  UINT8                       portid,
  UINT8                       command,
  UINT16                      offset,
  UINT32                      data
  );

UINT32 
Isb32Read (
  UINT8                       portid,
  UINT8                       command,
  UINT16                      offset
  );

//
// This is the memory needed for PEI to start up DXE.
//
// Over-estimating this size will lead to higher fragmentation
// of main memory.  Under-estimation of this will cause catastrophic
// failure of PEI to load DXE.  Generally, the failure may only be
// realized during capsule updates.
//
#define PRERESERVED_PEI_MEMORY ( \
  EFI_SIZE_TO_PAGES (3 * 0x800000)   /* PEI Core memory based stack          */ \
  )

//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;
#endif
//[-end-161123-IB07250310-modify]//

//[-start-161006-IB05400716-remove]//
////[-start-160530-IB07220088-modify]//
//EFI_MEMORY_TYPE_INFORMATION mDefaultMemoryTypeInformation[] = {
//  { EfiACPIReclaimMemory,       0x40  },    // 256K(0x40 pages) for ASL
//  { EfiACPIMemoryNVS,           0x100 },    // 1M(0x100 pages) for S3, SMM, HII, etc
//  { EfiReservedMemoryType,      0x2500 },   // 25M(0x2600 pages) for BIOS Reserved
//  { EfiRuntimeServicesData,     0x500 },    // 5M(0x500 pages) for UEFI OS runtime data to make S4 emmory consistency
//  { EfiRuntimeServicesCode,     0x290  },   // 2624K(0x290 pages)  for UEFI OS runtime drivers to make S4 memory consistency
//  { EfiBootServicesCode,        0x1000},    // 16M(0x1000 pages) for boot service drivers to reduce memory fragmental
//  { EfiBootServicesData,        0xB800},    // 184M(0xB800 pages) for boot service data to reduce memory fragmental
//  { EfiLoaderCode,              0x200 },    // 2M(0x200 pages) for UEFI OS boot loader to keep on same address 
//  { EfiLoaderData,              0x100 },
//  { EfiMaxMemoryType,           0     }
//};
////[-end-160530-IB07220088-modify]//
//[-end-161006-IB05400716-remove]//

/**
 Turn system into S5 if power button is pressed

 @param[in]         NONE

 @retval            NONE           
*/
VOID
CheckPowerOffNow (
  VOID
  )
{
  UINT16  Pm1Sts;
  UINT16  AcpiBaseAddr;

  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Read and check the ACPI registers
  //
  Pm1Sts = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_STS);
  if ((Pm1Sts & B_ACPI_PM1_STS_PWRBTN) == B_ACPI_PM1_STS_PWRBTN) {
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_STS, B_ACPI_PM1_STS_PWRBTN);
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5);
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5 + B_ACPI_PM1_CNT_SLP_EN);
    //
    // Should not return
    //
    CpuDeadLoop();
  }
}


/**
 Get current memory size.

 @param[in]         PeiServices          General purpose services available to every PEIM.
 @param[in]         LowMemoryLength      Memory between 1MB~4GB
 @param[in]         HighMemoryLength     Memory above 4GB
 

 @retval EFI_SUCCESS           The interface could be successfully installed
 @retval EFI_NOT_FOUND         The variable could not be discovered
 @retval EFI_BUFFER_TOO_SMALL  The caller buffer is not large enough
*/
STATIC
EFI_STATUS
GetMemorySize (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  OUT UINT64              *LowMemoryLength,
  OUT UINT64              *HighMemoryLength
  )
{
  EFI_STATUS              Status;
  EFI_PEI_HOB_POINTERS    Hob;

  *HighMemoryLength = 0;
  *LowMemoryLength = 0x100000;
  //
  // Get the HOB list for processing
  //
  Status = (*PeiServices)->GetHobList (PeiServices, (VOID **)&Hob.Raw);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Collect memory ranges
  //
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        //
        // Need memory above 1MB ~ 4GB to be collected here
        //
        if (Hob.ResourceDescriptor->PhysicalStart >= 0x100000 &&
            Hob.ResourceDescriptor->PhysicalStart < (EFI_PHYSICAL_ADDRESS) 0x100000000) {
          *LowMemoryLength += (UINT64) (Hob.ResourceDescriptor->ResourceLength);
        } else if (Hob.ResourceDescriptor->PhysicalStart >= (EFI_PHYSICAL_ADDRESS) 0x100000000) {
          *HighMemoryLength += (UINT64) (Hob.ResourceDescriptor->ResourceLength);
        }
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
MeasuredBootInit (
  IN CHIPSET_CONFIGURATION         *SystemConfiguration
  )
{
  return EFI_SUCCESS;
}

/**
 Publish Memory Type Information.

 @param[in]         NONE              

 @retval EFI_SUCCESS           The interface could be successfully installed
 @retval EFI_NOT_FOUND         The variable could not be discovered
 @retval EFI_BUFFER_TOO_SMALL  The caller buffer is not large enough
*/
EFI_STATUS
EFIAPI
PublishMemoryTypeInfo (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *Variable;
  UINTN                           DataSize;
  EFI_MEMORY_TYPE_INFORMATION     MemoryData[EfiMaxMemoryType + 1];
  CHIPSET_CONFIGURATION          *SystemConfiguration;
//[-start-161006-IB05400716-add]//
  EFI_MEMORY_TYPE_INFORMATION     *MemoryTypeInformation;
  UINTN                           MemoryTableSize;
//[-end-161006-IB05400716-add]//

  DEBUG((EFI_D_INFO, "INFO: PublishMemoryTypeInfo start\n"));

//[-start-161006-IB05400716-add]//
  MemoryTypeInformation = (EFI_MEMORY_TYPE_INFORMATION *)PcdGetPtr (PcdPreserveMemoryTable);
  MemoryTableSize =  LibPcdGetSize (PcdToken (PcdPreserveMemoryTable));
//[-end-161006-IB05400716-add]//

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&Variable
             );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "WARNING: Locating Pei variable failed 0x%x \n", Status));
    DEBUG((EFI_D_ERROR, "Build Hob from default\n"));
    //
    // Build the default GUID'd HOB for DXE
    //
//[-start-161006-IB05400716-modify]//
    BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, MemoryTypeInformation, MemoryTableSize);
//[-end-161006-IB05400716-modify]//
    return Status;
  } 
  DataSize = PcdGet32 (PcdSetupConfigSize);
  SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocateZeroPool (DataSize);
  Status = Variable->GetVariable(Variable,
                       L"Setup",
                       &gSystemConfigurationGuid,
                       NULL,
                       &DataSize,
                       SystemConfiguration
                       );
  DataSize = sizeof (MemoryData);

  // This variable is saved in BDS stage. Now read it back
  Status = Variable->GetVariable (
                      Variable,
                      EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME,
                      &gEfiMemoryTypeInformationGuid,
                      NULL,
                      &DataSize,
                      &MemoryData
                      );
  if (EFI_ERROR (Status)) {
    //build default
//    if (SystemConfiguration->OsSelection == 1) {
//      DEBUG((EFI_D_ERROR, "Build Hob from default\n"));
//      BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, mDefaultMemoryTypeInformationAos, sizeof (mDefaultMemoryTypeInformationAos) );
//    } else {
      DEBUG((EFI_D_ERROR, "Build Hob from default\n"));
//[-start-161006-IB05400716-modify]//
      BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, MemoryTypeInformation, MemoryTableSize);
//[-end-161006-IB05400716-modify]//
//    }
  } else {
    // Build the GUID'd HOB for DXE from variable
    DEBUG((EFI_D_ERROR, "Build Hob from variable \n"));
    BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, MemoryData, DataSize);
  }

  Status = MeasuredBootInit (SystemConfiguration);  

  return Status;
}


/**
 Install Firmware Volume Hob's once there is main memory
 Build up resource HOB

 @param[in]         BootMode              Current Boot mode

 @retval            EFI_SUCCESS         Function returns successfully  
*/
EFI_STATUS
EFIAPI
Stage2MemoryDiscoverCallback (
  IN  EFI_BOOT_MODE       *BootMode
  )
{
  EFI_STATUS                  Status;
//  EFI_BOOT_MODE               BootMode;
  EFI_CPUID_REGISTER          FeatureInfo;
  UINT8                       CpuAddressWidth;
  UINT16                      Pm1Cnt;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_PLATFORM_INFO_HOB       *PlatformInfo;
//  UINT32                      SpiBase;
  UINT16                      AcpiBaseAddr;
//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
  SC_PMC_FUNCTION_DISABLE_RESET_HOB *FuncDisHob;
  CPU_INIT_DATA_HOB                 *CpuInitDataHob;
  POWER_MGMT_CONFIG                 *PowerMgmtConfig;

  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    //
    // Get SC PMC fuction disable reset HOB.
    //
    Hob.Guid = GetFirstGuidHob (&gScPmcFunctionDisableResetHobGuid);
    if (Hob.Guid != NULL) {
      FuncDisHob = GET_GUID_HOB_DATA (Hob.Guid);
      FuncDisHob->ResetType = SC_PMC_FUNCTION_DISABLE_NO_RESET;
    }
  }
  //
  // Get Cpu Init Data Hob to override the SW SMI S3 restore MSR number from 0x48 to 0x62
  //
  Hob.Guid = GetFirstGuidHob (&gCpuInitDataHobGuid);
  if (Hob.Guid != NULL) {
    CpuInitDataHob = GET_GUID_HOB_DATA (Hob.Guid);
    PowerMgmtConfig = (POWER_MGMT_CONFIG *) (UINTN)CpuInitDataHob->PowerMgmtConfig;
    PowerMgmtConfig->S3RestoreMsrSwSmiNumber = (UINT8)SW_SMI_S3_RESTORE_MSR;
  }
#endif
//[-end-161123-IB07250310-add]//

//[-start-160616-IB07220100-add]//
  Status = EFI_SUCCESS;
//[-end-160616-IB07220100-add]//

  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

//  Status = (*PeiServices)->GetBootMode ((CONST EFI_PEI_SERVICES **) PeiServices, &BootMode);

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  
  //
  // Check if user wants to turn off in PEI phase
  //
  if ((*BootMode != BOOT_ON_S3_RESUME) && (*BootMode != BOOT_ON_FLASH_UPDATE)) {
    CheckPowerOffNow();
  } else {
    Pm1Cnt  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_CNT);
    Pm1Cnt &= ~B_ACPI_PM1_CNT_SLP_TYP;
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);
  }

  //
  // Set PEI cache mode here
  //
  //  SetPeiCacheMode (PeiServices);

  //
  //  Pulish memory tyoe info
  //
  PublishMemoryTypeInfo ();

  //
  // Work done if on a S3 resume
  //
  if (*BootMode == BOOT_ON_S3_RESUME) {
    return EFI_SUCCESS;
  }

  //
  // Add HOP entries for reserved MMIO ranges so that DXE will know about them.
  // Note: this really only need to be done for addresses that are outside the upper 16MB.
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PMC_BASE_ADDRESS,
    0x1000
    );
  DEBUG ((DEBUG_INFO, "PmcBase            : 0x%x\n", PMC_BASE_ADDRESS));
  //Spi BAR needs to be set to SPI_BASE_ADDRESS before it can be read..
//  SpiBase = MmPci32( 0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI, R_SPI_BASE ) & B_SPI_BASE_BAR;
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    SPI_BASE_ADDRESS,
    0x1000
    );
  DEBUG ((DEBUG_INFO, "SpiBase            : 0x%x\n", SPI_BASE_ADDRESS));

//[-start-170418-IB15590028-add]//  
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    HECI2_BASE_ADDRESS,
    0x1000
    );

  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    HECI3_BASE_ADDRESS,
    0x1000
    );
//[-end-170418-IB15590028-add]//

  // Local APIC
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    LOCAL_APIC_BASE_ADDRESS,
    0x1000
  );
  DEBUG ((DEBUG_INFO, "LOCAL_APIC_BASE_ADDRESS : 0x%x\n", LOCAL_APIC_BASE_ADDRESS));

  // IO APIC
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    IO_APIC_BASE_ADDRESS,
    0x1000
  );
  DEBUG ((DEBUG_INFO, "IO_APIC_ADDRESS    : 0x%x\n", IO_APIC_BASE_ADDRESS));

  // Adding the PCIE Express area to the memory table as type 2 memory.
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PlatformInfo->PciData.PciExpressBase,
    PlatformInfo->PciData.PciExpressSize
    );
  DEBUG ((DEBUG_INFO, "PciExpressBase     : 0x%x\n", PlatformInfo->PciData.PciExpressBase));

//[-start-151216-IB07220026-remove]//
//   // Adding the Flashpart to the memory table as type 2 memory.
//   BuildResourceDescriptorHob (
//     EFI_RESOURCE_FIRMWARE_DEVICE,
//     (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
//     PcdGet32(PcdFlashAreaBaseAddress),
//     PcdGet32(PcdFlashAreaSize)
//     );
//   DEBUG ((DEBUG_INFO, "FLASH_BASE_ADDRESS : 0x%x\n", PcdGet32(PcdFlashAreaBaseAddress)));
//[-end-151216-IB07220026-remove]//

  // P2SB (BXT) 16MB
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdP2SBBaseAddress),
    0x1000000
  );
  
  // PMC IPC (BXT) 8KB and 4KB
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcIpc1BaseAddress0),
    0x2000
  );

    BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcIpc1BaseAddress1),
    0x1000
  );

  //PMC SSRAM (BXT) 8KB and 4KB
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcSsramBaseAddress0),
    0x2000
    );
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcSsramBaseAddress1),
    0x1000
    );

  //
  // Create a CPU hand-off information
  //
  CpuAddressWidth = 32;
  AsmCpuid (CPUID_EXTENDED_FUNCTION, &FeatureInfo.RegEax, &FeatureInfo.RegEbx, &FeatureInfo.RegEcx, &FeatureInfo.RegEdx);
  if (FeatureInfo.RegEax >= CPUID_VIR_PHY_ADDRESS_SIZE) {
    AsmCpuid (CPUID_VIR_PHY_ADDRESS_SIZE, &FeatureInfo.RegEax, &FeatureInfo.RegEbx, &FeatureInfo.RegEcx, &FeatureInfo.RegEdx);
    CpuAddressWidth = (UINT8) (FeatureInfo.RegEax & 0xFF);
  }

  BuildCpuHob(CpuAddressWidth, 16);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

