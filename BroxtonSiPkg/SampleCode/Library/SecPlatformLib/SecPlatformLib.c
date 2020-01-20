/** @file
  Instance of SEC Platform Hook Lib.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2007 - 2016 Intel Corporation.

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

///
/// The package level header files this module uses
///
#include "SecPlatformLibrary.h"

GLOBAL_REMOVE_IF_UNREFERENCED PEI_SEC_PERFORMANCE_PPI  mSecPerformancePpi = {
  SecGetPerformance
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_PEI_PPI_DESCRIPTOR  mPeiSecPlatformPpi[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gPeiSecPerformancePpiGuid,
    &mSecPerformancePpi
  }
};

/**
  Perform those platform specific operations that are requried to be executed as early as possibile.

  @retval TRUE always return true.
**/
EFI_STATUS
EFIAPI
SecPlatformLibConstructor (
  )
{
  BOOLEAN                       DefaultsRestored;

  ///
  /// Init Apic Timer for Performance collection.
  /// Use EXCEPT_IA32_BOUND as interrupte type.
  ///
  PERF_CODE (
    InitializeApicTimer (0, (UINT32) -1, TRUE, 5);
  );

  DefaultsRestored = FALSE;
  ///
  /// Perform a checksum computation and verify if the checksum is correct. If the checksum is incorrect
  /// initialize all the CMOS location to their default values and recalculate the checksum.
  ///
#ifndef FSP_FLAG
  InitCmos (FALSE, &DefaultsRestored);
#endif //FSP_FLAG
  return EFI_SUCCESS;
}

/**
  This interface conveys performance information out of the Security (SEC) phase into PEI.

  This service is published by the SEC phase. The SEC phase handoff has an optional
  EFI_PEI_PPI_DESCRIPTOR list as its final argument when control is passed from SEC into the
  PEI Foundation. As such, if the platform supports collecting performance data in SEC,
  this information is encapsulated into the data structure abstracted by this service.
  This information is collected for the boot-strap processor (BSP) on IA-32.

  @param[in]  PeiServices  The pointer to the PEI Services Table.
  @param[in]  This         The pointer to this instance of the PEI_SEC_PERFORMANCE_PPI.
  @param[out] Performance  The pointer to performance data collected in SEC phase.

  @retval     EFI_SUCCESS  The data was successfully returned.
**/
EFI_STATUS
EFIAPI
SecGetPerformance (
  IN CONST EFI_PEI_SERVICES          **PeiServices,
  IN       PEI_SEC_PERFORMANCE_PPI   *This,
  OUT      FIRMWARE_SEC_PERFORMANCE  *Performance
  )
{
#ifndef FSP_FLAG
  UINT32  Size;
  UINT32  Count;
  UINT32  TopOfCar;
  UINT64  Ticker;

  ///
  /// |--------------| <- TopOfCar
  /// |Number of BSPs|
  /// |--------------|
  /// |     BIST     |
  /// |--------------|
  /// |     ....     |
  /// |--------------|
  /// |  TSC[63:32]  |
  /// |--------------|
  /// |  TSC[31:00]  |
  /// |--------------|
  ///
  TopOfCar = PcdGet32 (PcdTemporaryRamBase) + PcdGet32 (PcdTemporaryRamSize);
  Count = *(UINT32 *) (UINTN) (TopOfCar - sizeof (UINT32));
  Size  = Count * sizeof (UINT64);

  Ticker = *(UINT64 *) (UINTN) (TopOfCar - sizeof (UINT32) - Size - sizeof (UINT32) * 2);
  Performance->ResetEnd = GetTimeInNanoSecond (Ticker);
#endif //FSP_FLAG
  return EFI_SUCCESS;
}


/**
  A developer supplied function to perform platform specific operations.

  It's a developer supplied function to perform any operations appropriate to a
  given platform. It's invoked just before passing control to PEI core by SEC
  core. Platform developer may modify the SecCoreData and PPI list that is
  passed to PEI Core.

  @param[in, out] SecCoreData  The same parameter as passing to PEI core. It
                               could be overridden by this function.

  @retval         PpiList      The platform specific PPI list to be passed to PEI core or
                               NULL if there is no need of such platform specific PPI list.
**/
EFI_PEI_PPI_DESCRIPTOR *
EFIAPI
SecPlatformMain (
  IN OUT   EFI_SEC_PEI_HAND_OFF        *SecCoreData
  )
{
  EFI_PEI_PPI_DESCRIPTOR      *PpiList;

  PpiList = &mPeiSecPlatformPpi[0];

  return PpiList;
}


/**
  This interface conveys state information out of the Security (SEC) phase into PEI.

  @param[in]      PeiServices               Pointer to the PEI Services Table.
  @param[in, out] StructureSize             Pointer to the variable describing size of the input buffer.
  @param[out]     PlatformInformationRecord Pointer to the EFI_SEC_PLATFORM_INFORMATION_RECORD.

  @retval         EFI_SUCCESS               The data was successfully returned.
  @retval         EFI_BUFFER_TOO_SMALL      The buffer was too small.
**/
EFI_STATUS
EFIAPI
SecPlatformInformation (
  IN CONST EFI_PEI_SERVICES                      **PeiServices,
  IN OUT   UINT64                                *StructureSize,
  OUT      EFI_SEC_PLATFORM_INFORMATION_RECORD   *PlatformInformationRecord
  )
{
  UINT32  *Bist;
  UINT32  TopOfCar;

  ///
  /// The entries of BIST information, together with the number of them,
  /// reside in the bottom of stack, left untouched by normal stack operation.
  /// This routine copies the BIST information to the buffer pointed by
  /// PlatformInformationRecord for output.
  ///
  TopOfCar = PcdGet32 (PcdTemporaryRamBase) + PcdGet32 (PcdTemporaryRamSize);

  ///
  /// At this stage we only have information about the BSP.
  ///
  Bist = (UINT32 *) (UINTN) (TopOfCar - sizeof (UINT32) - sizeof (UINT32));

  CopyMem (PlatformInformationRecord, Bist, (UINTN)*StructureSize);

  return EFI_SUCCESS;
}

/**
  Platform USB init

  @param[in]  UsbDebugPortTempRamAddress
**/
VOID
EFIAPI
PlatformUsbInit (
  IN UINT32   UsbDebugPortTempRamAddress
  )
{
#ifndef FSP_FLAG

//  Put your platform USB init code here

  SetUsbDebugTempRamBase ((VOID *)(UINTN)UsbDebugPortTempRamAddress);

#endif //FSP_FLAG
}
