/** @file

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/**@file
  This is the driver that locates the MemoryConfigurationData HOB, if it
  exists, and saves the data to nvRAM.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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


#include <PlatformBootMode.h>
//#include <Protocol/SetupMode.h>
#include <Protocol/MemInfo.h>
#include <Guid/PlatformInfo.h>
#include <Guid/HobList.h>
#include <Guid/MemoryConfigData.h>
#include <Guid/GlobalVariable.h>
#include <Guid/BxtVariable.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

#define MRC_DATA_REQUIRED_FROM_OUTSIDE
#include "MmrcData.h"

#define EFI_UNLOAD_IMAGE   EFIERR(29)

//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
extern EFI_GUID gFspNonVolatileStorageHobGuid;
extern EFI_GUID gFspVariableNvDataHobGuid;
#endif
//[-end-161123-IB07250310-add]//
extern EFI_GUID gEfiMemoryConfigVariableGuid;
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  mMemoryConfigVariable[] = L"MemoryConfig";
GLOBAL_REMOVE_IF_UNREFERENCED CHAR16  mMemoryBootVariable[]   = L"MemoryBootData";

VOID
PrintBinaryBuffer (
  IN        UINT8*      Buffer,
  IN        UINTN       BufferSize
)
{
  UINTN    CurrentByte = 0;

  if (BufferSize == 0) {
    DEBUG ((EFI_D_INFO, "Skipping print of 0 size buffer\n"));
    return;
  }
  DEBUG ((EFI_D_INFO, "Base  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n"));
  DEBUG ((EFI_D_INFO, "%4d %2x ", CurrentByte/16, Buffer[0]));
  for (CurrentByte = 1; CurrentByte < BufferSize; CurrentByte++) {
    if ((CurrentByte % 16) == 0) {
      DEBUG ((EFI_D_INFO, "\n%4d %2x ", CurrentByte/16, Buffer[CurrentByte]));
    } else {
      DEBUG ((EFI_D_INFO, "%2x ", Buffer[CurrentByte]));
    }
  }
  DEBUG ((EFI_D_INFO, "\n"));
  return;
}

/**
  This function takes in an gEfiMemoryConfigVariableGuid Variable name and a pointer
  to the data that will be saved. The data is first compared against the currently
  stored variable for differences. If no difference is found, the save will be skipped.

  @param[in] VariableName  - The variable name to save using the Variable Service.
  @param[in] Buffer        - Pointer to the data to save
  @param[in] BufferSize    - The size of the data to save

  @retval EFI_SUCCESS      - if the data is successfully saved or there was no data
  @retval EFI_UNLOAD_IMAGE - It is not success
**/
EFI_STATUS
SaveMrcData(
  IN        CHAR16      VariableName[],
  IN        UINT8*      Buffer,
  IN        UINTN       BufferSize
)
{
  EFI_STATUS               Status;
  VOID                     *CurrentVariableData = NULL;
  UINTN                    CurrentVariableSize  = 0;
  BOOLEAN                  SaveConfig           = FALSE;

  CurrentVariableSize = 1;

  CurrentVariableData = AllocatePool(CurrentVariableSize);

  DEBUG((EFI_D_INFO, "SaveMrcData %s\n", VariableName));

  if (CurrentVariableData == NULL) {
    ASSERT (CurrentVariableData != NULL);
    return EFI_UNLOAD_IMAGE;
  }

  Status = gRT->GetVariable (
                  VariableName,
                  &gEfiMemoryConfigVariableGuid,
                  NULL,
                  &CurrentVariableSize,
                  CurrentVariableData
                  );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePool(CurrentVariableData);
    CurrentVariableData = AllocatePool(CurrentVariableSize);

    if (CurrentVariableData == NULL) {
        DEBUG((EFI_D_ERROR, "AllocatePool failed\n"));
        return EFI_UNLOAD_IMAGE;
    }

    Status = gRT->GetVariable (
                    VariableName,
                    &gEfiMemoryConfigVariableGuid,
                    NULL,
                    &CurrentVariableSize,
                    CurrentVariableData
                    );
  }

  //
  // The Memory Configuration will not exist during first boot or if memory/MRC
  // firmware changes have been made.
  //
  if (EFI_ERROR (Status) || CompareMem (Buffer, CurrentVariableData, BufferSize)) {
    SaveConfig = TRUE;
    DEBUG((EFI_D_INFO, "GetVariable Status: %r\n", Status));
    DEBUG((EFI_D_INFO, "Saving %s\n", VariableName));
  } else {
    DEBUG((EFI_D_INFO, "Skip %s\n", VariableName));
  }


  if (SaveConfig) {
        Status = gRT->SetVariable (
                    VariableName,
                    &gEfiMemoryConfigVariableGuid,
                    (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS),
                    BufferSize,
                    (UINT8 *) Buffer
                    );

    ASSERT_EFI_ERROR (Status);

    //PrintBinaryBuffer ((UINT8*) Buffer, BufferSize);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Failed to write %s\n", VariableName));
      ASSERT_EFI_ERROR (Status);
    }
  }
  return EFI_SUCCESS;
}

/**
  This is the standard EFI driver point that detects whether there is a
  MemoryConfigurationData HOB and, if so, saves its data to nvRAM.

  @param[in] ImageHandle  - Handle for the image of this driver
  @param[in] SystemTable  - Pointer to the EFI System Table

  @retval EFI_SUCCESS      - if the data is successfully saved or there was no data
  @retval EFI_NOT_FOUND    - if the HOB list could not be located.
  @retval EFI_UNLOAD_IMAGE - It is not success
**/
//[-start-161129-IB03090436-modify]//
EFI_STATUS
EFIAPI
SaveMemoryConfigEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE               Handle                       = NULL;
  EFI_STATUS               Status;
  EFI_HOB_GUID_TYPE        *GuidHob                     = NULL;
  EFI_PLATFORM_INFO_HOB    *PlatformInfoHob             = NULL;
//   EFI_PLATFORM_SETUP_ID    *BootModeBuffer              = NULL;
  MEM_INFO_PROTOCOL        *MemInfoHobProtocol          = NULL;
#ifndef FSP_WRAPPER_FLAG
  MRC_NV_DATA_FRAME        *MemoryConfigHobData         = NULL;
  UINTN                    MemoryConfigHobDataSize      = 0;
#else
  EFI_HOB_GUID_TYPE        *VariableGuidHob             = NULL;
  MRC_PARAMS_SAVE_RESTORE  *NvHobData                   = NULL;
  BOOT_VARIABLE_NV_DATA    *VariableNvHobData           = NULL;
#endif



  UINT8                    Channel    = 0;
  UINT8                    Slot       = 0;
  
//[-start-160617-IB07400744-add]//
  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    return EFI_UNSUPPORTED;
  }
//[-end-160617-IB07400744-add]//

  //
  // Search for the Memory Configuration GUID HOB.  If it is not present, then
  // there's nothing we can do. It may not exist on the update path.
  //
  GuidHob = GetFirstGuidHob (&gEfiPlatformInfoGuid);

  if (GuidHob == NULL) {
    ASSERT (GuidHob != NULL);
    return EFI_NOT_FOUND;
  }

  PlatformInfoHob = GET_GUID_HOB_DATA (GuidHob);

//   GuidHob = GetFirstGuidHob (&gEfiPlatformBootModeGuid);
  
//   if (GuidHob == NULL) {
//     ASSERT (GuidHob != NULL);
//     return EFI_NOT_FOUND;
//   }
  
//   BootModeBuffer = GET_GUID_HOB_DATA (GuidHob);
  
//   if (!CompareMem (&BootModeBuffer->SetupName, MANUFACTURE_SETUP_NAME, StrSize (MANUFACTURE_SETUP_NAME))) {
//     //
//     // Don't save Memory Configuration in Manufacturing Mode.
//     // Clear memory configuration.
//     //
//     DEBUG ((EFI_D_INFO, "Invalidating the MRC SaveParam Data for MfgMode...\n"));
//     
//     //
//     // This driver does not produce any protocol services, so always unload it.
//     //
//     return EFI_UNLOAD_IMAGE;
//   }
  
  //
  // Search for the Memory Configuration GUID HOB.  If it is not present, then
  // there's nothing we can do. It may not exist on the update path.
  //

#ifndef FSP_WRAPPER_FLAG
  if ((GuidHob = GetFirstGuidHob (&gEfiMemoryConfigDataGuid)) != NULL) {
    MemoryConfigHobData = GET_GUID_HOB_DATA (GuidHob);
    MemoryConfigHobDataSize = GET_GUID_HOB_DATA_SIZE(GuidHob);
  }

  if (MemoryConfigHobData == NULL) {
    return EFI_NOT_FOUND;
  }
#else

  if ((GuidHob = GetFirstGuidHob (&gFspNonVolatileStorageHobGuid)) != NULL) {
    NvHobData = GET_GUID_HOB_DATA(GuidHob);
  }

  if (NvHobData == NULL) {
    return EFI_NOT_FOUND;
  }

  if ((VariableGuidHob = GetFirstGuidHob (&gFspVariableNvDataHobGuid)) != NULL) {
    VariableNvHobData = GET_GUID_HOB_DATA(VariableGuidHob);
  }

  if (VariableNvHobData == NULL) {
    return EFI_NOT_FOUND;
  }
#endif


  // The size of this structure must match
#ifndef FSP_WRAPPER_FLAG
  ASSERT(MemoryConfigHobData->MrcParamsSaveRestore.DataSize == sizeof(MRC_NV_DATA_FRAME));
#else
  ASSERT(NvHobData->DataSize == sizeof(MRC_NV_DATA_FRAME));
#endif

  //
  // Populate and install the MemInfoHobProtocol
  //
  MemInfoHobProtocol = (MEM_INFO_PROTOCOL*) AllocateZeroPool (sizeof (MEM_INFO_PROTOCOL));

  if (MemInfoHobProtocol != NULL) {
    MemInfoHobProtocol->MemInfoData.memSize = 0;

#ifndef FSP_WRAPPER_FLAG
    for (Channel = 0; Channel < CH_NUM; Channel++) {
      for (Slot = 0; Slot < DIMM_NUM; Slot++) {
        MemInfoHobProtocol->MemInfoData.memSize += MemoryConfigHobData->MrcParamsSaveRestore.Channel[Channel].SlotMem[Slot];;
        MemInfoHobProtocol->MemInfoData.dimmSize[Slot + (Channel * DIMM_NUM)] = MemoryConfigHobData->MrcParamsSaveRestore.Channel[Channel].SlotMem[Slot];
        MemInfoHobProtocol->MemInfoData.DimmPresent[Slot + (Channel * DIMM_NUM)] = MemoryConfigHobData->MrcParamsSaveRestore.Channel[Channel].DimmPresent[Slot];
        if (MemInfoHobProtocol->MemInfoData.DimmPresent[Slot + (Channel * DIMM_NUM)]) {
          MemInfoHobProtocol->MemInfoData.DimmsSpdData[Slot + (Channel * DIMM_NUM)] = MemoryConfigHobData->MrcParamsSaveRestore.Channel[Channel].SpdData[Slot].Buffer;
        } else {
          MemInfoHobProtocol->MemInfoData.DimmsSpdData[Slot + (Channel * DIMM_NUM)] = NULL;
        }
      }
    }

    MemInfoHobProtocol->MemInfoData.ddrFreq   = MemoryConfigHobData->MrcParamsSaveRestore.CurrentFrequency;
    MemInfoHobProtocol->MemInfoData.memSize   = MemoryConfigHobData->MrcParamsSaveRestore.SystemMemorySize;
    MemInfoHobProtocol->MemInfoData.ddrType   = MemoryConfigHobData->MrcParamsSaveRestore.Channel[0].DramType;
    MemInfoHobProtocol->MemInfoData.BusWidth  = MemoryConfigHobData->MrcParamsSaveRestore.BusWidth;
#else
    for (Channel = 0; Channel < CH_NUM; Channel++) {
      for (Slot = 0; Slot < DIMM_NUM; Slot++) {
        MemInfoHobProtocol->MemInfoData.memSize += NvHobData->Channel[Channel].SlotMem[Slot];;
        MemInfoHobProtocol->MemInfoData.dimmSize[Slot + (Channel * DIMM_NUM)] = NvHobData->Channel[Channel].SlotMem[Slot];
        MemInfoHobProtocol->MemInfoData.DimmPresent[Slot + (Channel * DIMM_NUM)] = NvHobData->Channel[Channel].DimmPresent[Slot];
        if (MemInfoHobProtocol->MemInfoData.DimmPresent[Slot + (Channel * DIMM_NUM)]) {
          MemInfoHobProtocol->MemInfoData.DimmsSpdData[Slot + (Channel * DIMM_NUM)] = NvHobData->Channel[Channel].SpdData[Slot].Buffer;
        } else {
          MemInfoHobProtocol->MemInfoData.DimmsSpdData[Slot + (Channel * DIMM_NUM)] = NULL;
        }
      }
    }

    MemInfoHobProtocol->MemInfoData.ddrFreq   = NvHobData->CurrentFrequency;
    MemInfoHobProtocol->MemInfoData.memSize   = NvHobData->SystemMemorySize;
    MemInfoHobProtocol->MemInfoData.ddrType   = NvHobData->Channel[0].DramType;
    MemInfoHobProtocol->MemInfoData.BusWidth  = NvHobData->BusWidth;
#endif
    DEBUG ((EFI_D_INFO, "SaveMemoryConfigEntryPoint - Freq:0x%x\n", MemInfoHobProtocol->MemInfoData.ddrFreq));
    DEBUG ((EFI_D_INFO, "SaveMemoryConfigEntryPoint - Memsize:0x%x\n", MemInfoHobProtocol->MemInfoData.memSize));

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gMemInfoProtocolGuid,
                    MemInfoHobProtocol,
                    NULL
                    );
  }

#ifndef FSP_WRAPPER_FLAG
  Status = SaveMrcData(mMemoryConfigVariable, (UINT8 *)&(MemoryConfigHobData->MrcParamsSaveRestore), sizeof(MRC_PARAMS_SAVE_RESTORE));
  if (EFI_ERROR(Status)){
    return Status;
  }

  Status = SaveMrcData(mMemoryBootVariable, (UINT8 *)&(MemoryConfigHobData->BootVariableNvData), sizeof(BOOT_VARIABLE_NV_DATA));
  if (EFI_ERROR(Status)){
    return Status;
  }
#else
  Status = SaveMrcData(mMemoryConfigVariable, (UINT8 *) NvHobData, sizeof(MRC_PARAMS_SAVE_RESTORE));
  if (EFI_ERROR(Status)){
    return Status;
  }

  Status = SaveMrcData(mMemoryBootVariable, (UINT8 *) VariableNvHobData, sizeof(BOOT_VARIABLE_NV_DATA));
  if (EFI_ERROR(Status)){
    return Status;
  }
#endif
  //
  // This driver does not produce any protocol services, so always unload it.
  //
  return EFI_UNLOAD_IMAGE;
}
//[-end-161129-IB03090436-modify]//
