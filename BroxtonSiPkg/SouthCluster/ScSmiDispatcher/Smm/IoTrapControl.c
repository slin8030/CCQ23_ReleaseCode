/** @file
  Source file for the Io Trap Control in SMM

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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
#include <Library/UefiDriverEntryPoint.h>

#include <Protocol/ScSmmIoTrapControl.h>
#include "ScSmm.h"

GLOBAL_REMOVE_IF_UNREFERENCED EFI_HANDLE                                  IoTrapControlHandle;
GLOBAL_REMOVE_IF_UNREFERENCED SC_SMM_IO_TRAP_CONTROL_PROTOCOL             ScSmmIoTrapControlProtocol;

/**
  Pause IoTrap callback function.

  This function disables the SMI enable of IoTrap according to the DispatchHandle,
  which is returned by IoTrap callback registration.

  @param[in] This                 Pointer to the
        SC_SMM_IO_TRAP_CONTROL_PROTOCOL instance.
  @param[in] DispatchHandle       Handle of the child service to change state.

  @retval EFI_SUCCESS             This operation is complete.
  @retval EFI_INVALID_PARAMETER   The DispatchHandle is invalid.
  @retval EFI_ACCESS_DENIED       The SMI status is alrady PAUSED.
**/
EFI_STATUS
EFIAPI
IoTrapControlPause (
  IN SC_SMM_IO_TRAP_CONTROL_PROTOCOL    *This,
  IN EFI_HANDLE                         DispatchHandle
  )
{
  UINT16     AcpiBaseAddr;

  DEBUG((DEBUG_ERROR,"IoTrapControlPause, Entry\n"));
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  IoWrite16(AcpiBaseAddr+R_SMI_EN,
             (UINT16)(IoRead16 (AcpiBaseAddr + R_SMI_EN) & (~B_SMI_EN_MCSMI))
           );

  return EFI_SUCCESS;

}

/**
  Resume IoTrap callback function.

  This function enables the SMI enable of IoTrap according to the DispatchHandle,
  which is returned by IoTrap callback registration.

  @param[in] This                 Pointer to the
        SC_SMM_IO_TRAP_CONTROL_PROTOCOL instance.
  @param[in] DispatchHandle       Handle of the child service to change state.

  @retval EFI_SUCCESS             This operation is complete.
  @retval EFI_INVALID_PARAMETER   The DispatchHandle is invalid.
  @retval EFI_ACCESS_DENIED       The SMI status is alrady RESUMED.
**/
EFI_STATUS
EFIAPI
IoTrapControlResume (
  IN SC_SMM_IO_TRAP_CONTROL_PROTOCOL   *This,
  IN EFI_HANDLE                         DispatchHandle
  )
{

  UINT16     AcpiBaseAddr;

  DEBUG((DEBUG_INFO,"IoTrapControlResume, Entry\n"));
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  IoWrite16(AcpiBaseAddr+R_SMI_EN,
            (UINT16)(IoRead16 (AcpiBaseAddr + R_SMI_EN) | (B_SMI_EN_MCSMI))
           );

  return EFI_SUCCESS;
}

/**
  The IoTrap module abstracts PCH I/O trapping capabilities for other drivers.
  This driver manages the limited I/O trap resources.

  @param[in] ImageHandle                Image handle for this driver image

  @retval EFI_SUCCESS                   Driver initialization completed successfully
**/
EFI_STATUS
EFIAPI
InstallIoTrapControl (
  VOID
  )
{
  EFI_STATUS                            Status;

  DEBUG((DEBUG_INFO, "InstallIoTrapControl, Entry\n"));
  //
  // Initialize the IO TRAP control protocol.
  //
  ScSmmIoTrapControlProtocol.Pause       = IoTrapControlPause;
  ScSmmIoTrapControlProtocol.Resume      = IoTrapControlResume;

  //
  // Install IoTrap Control protocol interface
  //

  Status = gSmst->SmmInstallProtocolInterface (
                    &IoTrapControlHandle,
                    &gScSmmIoTrapControlGuid,
                    EFI_NATIVE_INTERFACE,
                    &ScSmmIoTrapControlProtocol
                    );

  ASSERT_EFI_ERROR (Status);

  return Status;
}
