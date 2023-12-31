/**@file
 Library functions for hob processing.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/

#include <PiPei.h>

#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/FspWrapperPlatformLib.h>
#include <Library/FspCommonLib.h>

#include <Guid/MemoryTypeInformation.h>
#include <Ppi/Capsule.h>
#include <Guid/SmramMemoryReserve.h>
#include <Guid/GuidHobFspEas.h>
#include <Guid/AcpiS3Context.h>
#include <Guid/VariableFormat.h>
#include <Guid/PlatformCpuInfo.h>
#include <Private/CpuInitDataHob.h>
#include <SiPolicyHob.h>
#include <Private/Guid/ScPolicyHobGuid.h>
#include <FspmUpd.h>
//[-start-161123-IB07250310-add]//
#include <SmbiosProcessorInfoHob.h>
#include <SmbiosCacheInfoHob.h>

extern EFI_GUID gSiMemoryInfoDataGuid;
extern EFI_GUID gEfiGraphicsInfoHobGuid;
extern EFI_GUID gFspVariableNvDataHobGuid;
//[-end-161123-IB07250310-add]//
extern EFI_GUID gScDeviceTableHobGuid;
extern EFI_GUID gEfiBootMediaHobGuid;
extern EFI_GUID gEfiIfwiDnxRequestHobGuid;
extern EFI_GUID gPeiAcpiCpuDataGuid;

EFI_GUID gReservedMemoryResourceHobTsegGuid = {0xd038747c, 0xd00c, 0x4980, {0xb3, 0x19, 0x49, 0x01, 0x99, 0xa4, 0x7d, 0x55}};
EFI_GUID gDummyGuid              = { 0x00000000, 0xb74c, 0x4f11, {0x8c, 0x03, 0x41, 0x86, 0x65, 0x53, 0x28, 0x31}};

//
// Additional pages are used by DXE memory manager.
// It should be consistent between RetrieveRequiredMemorySize() and GetPeiMemSize()
//
#define PEI_ADDITIONAL_MEMORY_SIZE    (16 * EFI_PAGE_SIZE)

/**
  Get the mem size in memory type infromation table.

  @param[in] PeiServices  PEI Services table.

  @return the mem size in memory type infromation table.
**/
UINT64
GetMemorySizeInMemoryTypeInformation (
  IN EFI_PEI_SERVICES **PeiServices
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_MEMORY_TYPE_INFORMATION *MemoryData;
  UINT8                       Index;
  UINTN                       TempPageNum;

  MemoryData = NULL;
  Status     = (*PeiServices)->GetHobList (PeiServices, (VOID **) &Hob.Raw);
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION &&
      CompareGuid (&Hob.Guid->Name, &gEfiMemoryTypeInformationGuid)) {
      MemoryData = (EFI_MEMORY_TYPE_INFORMATION *) (Hob.Raw + sizeof (EFI_HOB_GENERIC_HEADER) + sizeof (EFI_GUID));
      break;
    }

    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  if (MemoryData == NULL) {
    return 0;
  }

  TempPageNum = 0;
  for (Index = 0; MemoryData[Index].Type != EfiMaxMemoryType; Index++) {
    //
    // Accumulate default memory size requirements
    //
    TempPageNum += MemoryData[Index].NumberOfPages;
  }

  return TempPageNum * EFI_PAGE_SIZE;
}

/**
  Get the mem size need to be reserved in PEI phase.

  @param[in] PeiServices  PEI Services table.

  @return the mem size need to be reserved in PEI phase.
**/
UINT64
RetrieveRequiredMemorySize (
  IN EFI_PEI_SERVICES **PeiServices
  )
{
  UINT64                      Size;

  Size = GetMemorySizeInMemoryTypeInformation (PeiServices);
  return Size + PEI_ADDITIONAL_MEMORY_SIZE;
}

/**
  Get the mem size need to be consumed and reserved in PEI phase.

  @param[in] PeiServices  PEI Services table.
  @param[in] BootMode     Current boot mode.

  @return the mem size need to be consumed and reserved in PEI phase.
**/
UINT64
GetPeiMemSize (
  IN EFI_PEI_SERVICES **PeiServices,
  IN UINT32           BootMode
  )
{
  UINT64                      Size;
  UINT64                      MinSize;

  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return PcdGet32 (PcdPeiRecoveryMinMemSize);
  }

  Size = GetMemorySizeInMemoryTypeInformation (PeiServices);

  if (BootMode == BOOT_ON_FLASH_UPDATE) {
    //
    // Maybe more size when in CapsuleUpdate phase ?
    //
    MinSize = PcdGet32 (PcdPeiMinMemSize);
  } else {
    MinSize = PcdGet32 (PcdPeiMinMemSize);
  }

  return MinSize + Size + PEI_ADDITIONAL_MEMORY_SIZE;
}

/**
  BIOS process FspBobList for Memory Resource Descriptor.

  @param[in] FspHobList  Pointer to the HOB data structure produced by FSP.

  @return If platform process the FSP hob list successfully.
**/
EFI_STATUS
EFIAPI
PostFspmHobProcess (
  IN VOID                 *FspHobList
  )
{
  EFI_PEI_HOB_POINTERS Hob;
  UINT64               LowMemorySize;
  UINT64               FspMemorySize;
  EFI_PHYSICAL_ADDRESS FspMemoryBase;
//  UINT64               PeiMemSize;
  EFI_PHYSICAL_ADDRESS PeiMemBase;
  EFI_PHYSICAL_ADDRESS Tolum;
  UINT64               S3PeiMemSize;
  EFI_PHYSICAL_ADDRESS S3PeiMemBase;
  BOOLEAN              FoundFspMemHob;
  EFI_STATUS           Status;
  EFI_BOOT_MODE        BootMode;
  //PEI_CAPSULE_PPI      *Capsule;
  //VOID                 *CapsuleBuffer;
  //UINTN                CapsuleBufferLength;
  //UINT64               RequiredMemSize;
  EFI_PEI_SERVICES     **PeiServices;
  FSPM_UPD             *FspmUpd;

  PeiServices = (EFI_PEI_SERVICES **)GetPeiServicesTablePointer ();

  PeiServicesGetBootMode (&BootMode);

  PeiMemBase = 0;
  Tolum = 0;
  LowMemorySize = 0;
  FspMemorySize = 0;
  FspMemoryBase = 0;
  FoundFspMemHob = FALSE;

  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();

  //
  // Parse the hob list from fsp
  // Report all the memory resource hob.
  //
  Hob.Raw = (UINT8 *)(UINTN)FspHobList;
  DEBUG((DEBUG_INFO, "FspHobList - 0x%x\n", FspHobList));

  while ((Hob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, Hob.Raw)) != NULL) {
    DEBUG((DEBUG_INFO, "\nResourceType: 0x%x\n", Hob.ResourceDescriptor->ResourceType));
    if ((Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) ||
        (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_MEMORY_RESERVED)) {
      DEBUG((DEBUG_INFO, "ResourceAttribute: 0x%x\n", Hob.ResourceDescriptor->ResourceAttribute));
      DEBUG((DEBUG_INFO, "PhysicalStart: 0x%lx\n", Hob.ResourceDescriptor->PhysicalStart));
      DEBUG((DEBUG_INFO, "ResourceLength: 0x%lx\n", Hob.ResourceDescriptor->ResourceLength));
      DEBUG((DEBUG_INFO, "Owner: %g\n\n", &Hob.ResourceDescriptor->Owner));
    }

    if ((Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) &&
        (Hob.ResourceDescriptor->PhysicalStart < BASE_4GB)) {
      if (Tolum <(Hob.ResourceDescriptor->PhysicalStart+Hob.ResourceDescriptor->ResourceLength)){
        Tolum = Hob.ResourceDescriptor->PhysicalStart+Hob.ResourceDescriptor->ResourceLength;
      }
    }
    if ((Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_MEMORY_RESERVED)  // Found the low memory length below 4G
        && (Hob.ResourceDescriptor->PhysicalStart >= BASE_1MB)
        && (Hob.ResourceDescriptor->PhysicalStart + Hob.ResourceDescriptor->ResourceLength <= BASE_4GB)
        && (CompareGuid (&Hob.ResourceDescriptor->Owner, &gFspReservedMemoryResourceHobGuid))) {
      FoundFspMemHob = TRUE;
      FspMemoryBase = Hob.ResourceDescriptor->PhysicalStart;
      FspMemorySize = Hob.ResourceDescriptor->ResourceLength;
      DEBUG((DEBUG_INFO, "Find fsp mem hob, base 0x%lx, len 0x%lx\n", FspMemoryBase, FspMemorySize));
    }

    //
    // Report the resource hob
    //
    BuildResourceDescriptorHob (
      Hob.ResourceDescriptor->ResourceType,
      Hob.ResourceDescriptor->ResourceAttribute,
      Hob.ResourceDescriptor->PhysicalStart,
      Hob.ResourceDescriptor->ResourceLength
      );

    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  if (!FoundFspMemHob) {
    DEBUG((DEBUG_INFO, "Didn't find the fsp used memory information.\n"));
    //ASSERT(FALSE);
  }

  DEBUG((DEBUG_INFO, "LowMemorySize: 0x%x.\n", LowMemorySize));
  DEBUG((DEBUG_INFO, "FspMemoryBase: 0x%x.\n", FspMemoryBase));
  DEBUG((DEBUG_INFO, "FspMemorySize: 0x%x.\n", FspMemorySize));

  if (BootMode == BOOT_ON_S3_RESUME) {

    S3PeiMemBase = 0;
    S3PeiMemSize = 0;
    Status = GetS3MemoryInfo (&S3PeiMemSize, &S3PeiMemBase);
    ASSERT_EFI_ERROR (Status);
    DEBUG((DEBUG_INFO, "S3 memory %lx - %lxh bytes\n", S3PeiMemBase, S3PeiMemSize));

    //
    // Make sure Stack and PeiMemory are not overlap
    //
    Status = PeiServicesInstallPeiMemory (
               S3PeiMemBase,
               S3PeiMemSize
               );
    ASSERT_EFI_ERROR (Status);
  } else {
//[-start-161123-IB07250310-modify]//
    //
    // It must install enough memory for PEI memory and DXE preserved memory.
    // Avoid boot time to use the memory above 4G.
    //
    PeiMemBase = Tolum - BASE_512MB;
    Status = PeiServicesInstallPeiMemory (
               PeiMemBase,
               BASE_512MB
               );
//[-end-161123-IB07250310-modify]//
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

/**
  Dump FSP SMBIOS memory info HOB

**/
VOID
DumpFspSmbiosMemoryInfoHob (
  IN VOID                 *FspHobList
  )
{
  EFI_PEI_HOB_POINTERS   Hob;
  FSP_SMBIOS_MEMORY_INFO *FspSmbiosMemoryInfo = NULL;
  UINT8                  ChannelIndex;
  UINT8                  DimmIndex;

  Hob.Raw = (VOID *)FspHobList;
  if (Hob.Raw != NULL) {
    if ((Hob.Raw = GetNextGuidHob (&gSiMemoryInfoDataGuid, Hob.Raw)) != NULL) {
      FspSmbiosMemoryInfo = GET_GUID_HOB_DATA (Hob.Guid);
    }
  }
  if (FspSmbiosMemoryInfo != NULL) {
    DEBUG((DEBUG_INFO, "\nFspSmbiosMemoryInfo\n"));
    DEBUG((DEBUG_INFO, "  |-> Revision : %d\n", FspSmbiosMemoryInfo->Revision));
    DEBUG((DEBUG_INFO, "  |-> MemoryType : %d\n", FspSmbiosMemoryInfo->MemoryType));
    DEBUG((DEBUG_INFO, "  |-> DataWidth : %d\n", FspSmbiosMemoryInfo->DataWidth));
    DEBUG((DEBUG_INFO, "  |-> MemoryFrequencyInMHz : %d\n", FspSmbiosMemoryInfo->MemoryFrequencyInMHz));
    DEBUG((DEBUG_INFO, "  |-> ErrorCorrectionType : %d\n", FspSmbiosMemoryInfo->ErrorCorrectionType));
    DEBUG((DEBUG_INFO, "  |-> ChannelCount : %d\n", FspSmbiosMemoryInfo->ChannelCount));
    DEBUG((DEBUG_INFO, "  |-> ChannelInfo\n"));
    for (ChannelIndex = 0; ChannelIndex < FspSmbiosMemoryInfo->ChannelCount; ChannelIndex++) {
      {
        CHANNEL_INFO  *ChannelInfo;

        ChannelInfo = &FspSmbiosMemoryInfo->ChannelInfo[ChannelIndex];
        DEBUG((DEBUG_INFO, "        |-> ChannelId : %d\n", ChannelInfo->ChannelId));
        DEBUG((DEBUG_INFO, "        |-> DimmCount : %d\n", ChannelInfo->DimmCount));
        DEBUG((DEBUG_INFO, "        |-> DimmInfo\n"));
        for (DimmIndex = 0; DimmIndex < MAX_DIMMS_NUM; DimmIndex++) {
          {
            DIMM_INFO  *DimmInfo;

            DimmInfo = &FspSmbiosMemoryInfo->ChannelInfo[ChannelIndex].DimmInfo[DimmIndex];
            if (ChannelIndex < (MAX_CHANNELS_NUM - 1)) {
              DEBUG((DEBUG_INFO, "        |     |-> DimmId : %d\n", DimmInfo->DimmId));
              DEBUG((DEBUG_INFO, "        |     |-> SizeInMb : %d\n", DimmInfo->SizeInMb));
            } else {
              DEBUG((DEBUG_INFO, "              |-> DimmId : %d\n", DimmInfo->DimmId));
              DEBUG((DEBUG_INFO, "              |-> SizeInMb : %d\n", DimmInfo->SizeInMb));
            }
          }
        }
      }
    }
    DEBUG((DEBUG_INFO, "\n"));
  }
}



/**
  Process FSP HOB list

  @param[in] FspHobList  Pointer to the HOB data structure produced by FSP.

**/
VOID
ProcessFspHobList (
  IN VOID                 *FspHobList
)
{
  EFI_PEI_HOB_POINTERS   FspHob;
  EFI_PEI_HOB_POINTERS   DxeHob;
  BOOLEAN                TransferFlag;

  /*
   * Get the HOB list for processing
   */
  FspHob.Raw = (VOID *)FspHobList;

  /*
   * Collect memory ranges
   */
  while (!END_OF_HOB_LIST (FspHob)) {
    TransferFlag = FALSE;
    if (FspHob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION) {
      if (CompareGuid(&FspHob.Guid->Name, &gScDeviceTableHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gScPolicyHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gSiPolicyHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gFspNonVolatileStorageHobGuid)) {
        DEBUG ((DEBUG_INFO, "Transfer Memory Config Data HOB!\n"));
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gFspVariableNvDataHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gEfiBootMediaHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gEfiIfwiDnxRequestHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gPeiAcpiCpuDataGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gEfiSmmPeiSmramMemoryReserveGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gCpuInitDataHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gSmbiosProcessorInfoHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gSmbiosCacheInfoHobGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gSiMemoryInfoDataGuid)) {
        TransferFlag = TRUE;
      } else if (CompareGuid(&FspHob.Guid->Name, &gEfiGraphicsInfoHobGuid)) {
        TransferFlag = TRUE;
      //  HobGuid = &gPlatformConfigDataGuid;
      //} else if (CompareGuid(&FspHob.Guid->Name, &gPlatformInfoHobGuid)) {
      //  TransferFlag = TRUE;
      //} else if (CompareGuid(&FspHob.Guid->Name, &gBiosIdGuid)) {
      //  TransferFlag = TRUE;
      //} else if (CompareGuid(&FspHob.Guid->Name, &gFspPcdDataBaseHobGuid)) {
      //  CopyMem (&FspHob.Guid->Name, &gPcdDataBaseHobGuid, sizeof (EFI_GUID));  // Replace original PCD Database HOB if gFspPcdDataBaseHobGuid exists.
      //  TransferFlag = TRUE;
      //} else if (CompareGuid(&FspHob.Guid->Name, &gEfiPlatformCpuInfoGuid)) {
      //  TransferFlag = TRUE;
      }

    }

    if (TransferFlag) {
      DEBUG ((DEBUG_INFO, "FSP  Extended    GUID HOB: {%g} HobLength = %x\n", &(FspHob.Guid->Name), FspHob.Header->HobLength));
      DxeHob.Raw = BuildGuidDataHob(&FspHob.Guid->Name, GET_GUID_HOB_DATA(FspHob.Raw), GET_GUID_HOB_DATA_SIZE(FspHob.Raw));
    }
    FspHob.Raw = GET_NEXT_HOB (FspHob);
  }
}

/**
  BIOS process FspBobList for other data (not Memory Resource Descriptor).

  @param[in] FspHobList  Pointer to the HOB data structure produced by FSP.

  @return If platform process the FSP hob list successfully.
**/
EFI_STATUS
EFIAPI
PostFspsHobProcess (
  IN VOID                 *FspHobList
  )
{
  ProcessFspHobList (FspHobList);

  DEBUG_CODE_BEGIN ();
    DumpFspSmbiosMemoryInfoHob (FspHobList);
  DEBUG_CODE_END ();

  return EFI_SUCCESS;
}
