/** @file
  This file includes a memory call back function notified when MRC is done,
  following action is performed in this file,
    1. ICH initialization after MRC.
    2. SIO initialization.
    3. Install ResetSystem and FinvFv PPI.
    4. Set MTRR for PEI
    5. Create FV HOB and Flash HOB


 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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

//[-start-160509-IB03090427-modify]//
#include <SaRegs.h>
#include <SaCommonDefinitions.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Guid/NpkInfoHob.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MtrrLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>

#include "MemoryCallback.h"


//[-start-160817-IB03090432-remove]//
//extern EFI_GUID gEfiBootMediaHobGuid;
//[-end-160817-IB03090432-remove]//
extern EFI_GUID gUfsBootLunIdHobGuid;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gMicroCodepointerGuid;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gUfsPhyOverrideHobGuid;

#ifndef FSP_WRAPPER_FLAG

STATIC
EFI_STATUS
GetMemorySize (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  OUT UINT64                    *LowMemoryLength,
  OUT UINT64                    *HighMemoryLength
  )
{
  EFI_STATUS              Status;
  EFI_PEI_HOB_POINTERS    Hob;

  *HighMemoryLength = 0;
  *LowMemoryLength = 0x100000;
  //
  // Get the HOB list for processing
  //
  Status = (*PeiServices)->GetHobList (PeiServices, (VOID **) &Hob.Raw);
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
        // Need memory above 1MB to be collected here
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

/**
  This function will be called when MRC is done.

  @param  PeiServices General purpose services available to every PEIM.
  @param  NotifyDescriptor Information about the notify event..
  @param  Ppi The notify context.

  @retval EFI_SUCCESS If the function completed successfully.
**/
EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  UINT8                        Index;
  MTRR_SETTINGS                MtrrSetting;
  UINT64                       MemoryLength;
  UINT64                       MemOverflow;
  UINT64                       MemoryLengthUc;
  UINT64                       MaxMemoryLength;
  UINT64                       LowMemoryLength;
  UINT64                       HighMemoryLength;
  UINT64                       MsrData;
  UINT32                       TSegBase;
#if (NPK_ENABLE == 1)
  EFI_PEI_HOB_POINTERS         GuidHob;
  NPK_RESERVE_HOB              *NpkReserveHob = NULL;
#endif

  //
  // Variable initialization
  //
  LowMemoryLength = 0;
  HighMemoryLength = 0;
  MemoryLengthUc = 0;

  //
  // Determine memory usage
  //
  GetMemorySize (
    (CONST EFI_PEI_SERVICES **) PeiServices,
    &LowMemoryLength,
    &HighMemoryLength
    );

  //
  // The algorithm is not determining memory properly this
  // W/A forces low memory to TSEG base to match memory config.
  //
  TSegBase = (UINT32)MmPciAddress (
                       0,
                       SA_MC_BUS,
                       SA_MC_DEV,
                       SA_MC_FUN,
                       R_SA_TSEG
                       );

  LowMemoryLength = (MmioRead32 (TSegBase) & 0xFFFFFFF0);

#if (NPK_ENABLE == 1)
  //Hard code to reduce the NPK reserve mem 512MB.
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gNPKReserveMemGuid, GuidHob.Raw)) != NULL) {
      NpkReserveHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }
  if (NpkReserveHob == NULL) {
    DEBUG ((DEBUG_INFO, "NpkReserveHob not found....\n"));
  } else {
    LowMemoryLength = LowMemoryLength - NpkReserveHob->NpkPreMemConfig.Msc0Size - NpkReserveHob->NpkPreMemConfig.Msc1Size;
  }
#endif
  DEBUG ((DEBUG_INFO, "LowMemoryLength=%x \n", LowMemoryLength));

  MaxMemoryLength = LowMemoryLength;
  // Round up to nearest 256MB
  MemOverflow = (LowMemoryLength & 0x0fffffff);
  if (MemOverflow != 0) {
    MaxMemoryLength = LowMemoryLength + (0x10000000 - MemOverflow);
  }

  ZeroMem (&MtrrSetting, sizeof(MTRR_SETTINGS));
  for (Index = 0; Index < 2; Index++) {
    MtrrSetting.Fixed.Mtrr[Index] = 0x0606060606060606;
    // PcdStatusCodeUseRam Enabled set RAMDEBUG region  0x70000 of size 0x10000 to UC
    if ((Index == 0)&&(FeaturePcdGet (PcdStatusCodeUseRam))){
      MtrrSetting.Fixed.Mtrr[Index] = 0x0006060606060606;
    }
   }
  for (Index = 3; Index < 11; Index++){
    MtrrSetting.Fixed.Mtrr[Index] = 0x0505050505050505;
   }

  Index = 0;
  MemOverflow =0;
  while (MaxMemoryLength > MemOverflow){
    MtrrSetting.Variables.Mtrr[Index].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;
    MemoryLength = MaxMemoryLength - MemOverflow;
    MemoryLength = GetPowerOfTwo64 (MemoryLength);
    MtrrSetting.Variables.Mtrr[Index].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
    DEBUG ((DEBUG_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));

    MemOverflow += MemoryLength;
    Index++;
  }

  MemoryLength = LowMemoryLength;

  while (MaxMemoryLength != MemoryLength) {
    MemoryLengthUc = GetPowerOfTwo64 (MaxMemoryLength - MemoryLength);
    MtrrSetting.Variables.Mtrr[Index].Base = ((MaxMemoryLength - MemoryLengthUc) & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheUncacheable;
    MtrrSetting.Variables.Mtrr[Index].Mask= ((~(MemoryLengthUc   - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
    DEBUG ((DEBUG_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));
    MaxMemoryLength -= MemoryLengthUc;
    Index++;
  }

  MemOverflow =0x100000000;
  while (HighMemoryLength > 0) {
    MtrrSetting.Variables.Mtrr[Index].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;
    MemoryLength = HighMemoryLength;
    MemoryLength = GetPowerOfTwo64 (MemoryLength);

    if (MemoryLength > 0x100000000) {
      MemoryLength = 0x100000000;   //Cap at Max 4G
    }

    MtrrSetting.Variables.Mtrr[Index].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
    DEBUG ((DEBUG_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));

    MemOverflow += MemoryLength;
    HighMemoryLength -= MemoryLength;
    Index++;
  }
//[-start-170516-IB08450375-modify]//
//#if !BXTI_PF_ENABLE
  //
  // Set the flash area to uncachable speculative read to improve boot performance in PEI phase
  //
  MtrrSetting.Variables.Mtrr[Index].Base = (FixedPcdGet32 (PcdFlashAreaBaseAddress) | CacheWriteCombining);
  MtrrSetting.Variables.Mtrr[Index].Mask = (((~(UINT64)(FixedPcdGet32 (PcdFlashAreaSize) - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED);
//#endif
//[-end-170516-IB08450375-modify]//

  //
  // Disable No-Eviction Mode
  //
  MsrData = AsmReadMsr64 (0x2E0);
  MsrData &= ~BIT1;
  AsmWriteMsr64 (0x2E0, MsrData);

  MsrData = AsmReadMsr64 (0x2E0);
  MsrData &= ~BIT0;
  AsmWriteMsr64 (0x2E0, MsrData);

  AsmInvd ();
  MtrrSetAllMtrrs(&MtrrSetting);

  //
  // Enable Cache MTRR
  //
  AsmMsrBitFieldWrite64 (MTRR_LIB_IA32_MTRR_DEF_TYPE, 10, 11, 3);

  return EFI_SUCCESS;
}
#endif  //ifndef FSP_WRAPPER_FLAG

//[-end-160509-IB03090427-modify]//
