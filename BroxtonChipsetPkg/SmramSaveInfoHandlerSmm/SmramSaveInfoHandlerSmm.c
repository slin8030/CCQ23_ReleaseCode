/** @file
  A helper driver to save information to SMRAM after SMRR is enabled.

@copyright
 Copyright (c) 2011 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:

**/

//[-start-151229-IB03090424-modify]//
#include "SmramSaveInfoHandlerSmm.h"

#define SMM_FROM_CPU_DRIVER_SAVE_INFO 0x81

#define EFI_SMRAM_CPU_NVS_HEADER_GUID \
  { \
    0x429501d9, 0xe447, 0x40f4, 0x86, 0x7b, 0x75, 0xc9, 0x3a, 0x1d, 0xb5, 0x4e \
  }

GLOBAL_REMOVE_IF_UNREFERENCED UINT8   mSmiDataRegister;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN mLocked = FALSE;
EFI_GUID mSmramCpuNvsHeaderGuid = EFI_SMRAM_CPU_NVS_HEADER_GUID;

CPU_INFO_PROTOCOL *mCpuInfoProtocol;
/**
  Dispatch function for a Software SMI handler.

  @param  DispatchHandle        The handle of this dispatch function.
  @param  DispatchContext       The pointer to the dispatch function's context.
                                The SwSmiInputValue field is filled in
                                by the software dispatch driver prior to
                                invoking this dispatch function.
                                The dispatch function will only be called
                                for input values for which it is registered.
**/
EFI_STATUS
SmramSaveInfoHandler (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_REGISTER_CONTEXT   *DispatchContext
  )
{
  EFI_STATUS                Status;
  UINT64                    VarData[3];
  EFI_PHYSICAL_ADDRESS      SmramDataAddress;

  ASSERT (DispatchContext != NULL);
  ASSERT (DispatchContext->SwSmiInputValue == mCpuInfoProtocol->CpuInfo->SmmbaseSwSmiNumber);
  DEBUG ((EFI_D_INFO, "SmramSaveInfoHandler() Enter\n"));
  DEBUG ((EFI_D_INFO, "mLocked = %x\n",mLocked));
  DEBUG ((EFI_D_INFO, "mSmiDataRegister = 0x%02x\n",mSmiDataRegister));
  DEBUG ((EFI_D_INFO, "IoRead8 (mSmiDataRegister) = 0x%02x\n",IoRead8 (mSmiDataRegister)));
  if (!mLocked && IoRead8 (mSmiDataRegister) == SMM_FROM_CPU_DRIVER_SAVE_INFO) {
//     VarSize = sizeof (VarData);
//     Status = gRT->GetVariable (
//                     L"SmramCpuNvs",
//                     &mSmramCpuNvsHeaderGuid,
//                     NULL,
//                     &VarSize,
//                     VarData
//                     );
//     DEBUG ((EFI_D_INFO, "Set variable Status %r\n", Status));
//     if (!EFI_ERROR (Status) && VarSize == sizeof (VarData)) {
//       CopyMem (
//         (VOID *)(UINTN)(VarData[0]),
//         (VOID *)(UINTN)(VarData[1]),
//         (UINTN)(VarData[2])
//         );
//     }

    VarData[2] = mCpuInfoProtocol->SmramCpuInfo->LockBoxSize;
    VarData[1] = mCpuInfoProtocol->SmramCpuInfo->SmramCpuData;

      Status = gSmst->SmmAllocatePages (
                 AllocateAnyPages,
                 EfiRuntimeServicesData,
                 EFI_SIZE_TO_PAGES ((UINTN)VarData[2]),
                 &SmramDataAddress
                 );
      ASSERT_EFI_ERROR (Status);
      DEBUG ((DEBUG_INFO, "CPU SMRAM NVS Data - %x\n", SmramDataAddress));
      DEBUG ((DEBUG_INFO, "CPU SMRAM NVS Data size - %x\n", VarData[2]));
      VarData[0] = (UINT64)SmramDataAddress;
      CopyMem (
              (VOID *) (UINTN) (VarData[0]),
              (VOID *) (UINTN) (VarData[1]),
              (UINTN) (VarData[2])
              );
  }
  return EFI_SUCCESS;
}

/**
  Smm Ready To Lock event notification handler.

  It sets a flag indicating that SMRAM has been locked.
  
  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
 **/
EFI_STATUS
EFIAPI
SmmReadyToLockEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  mLocked = TRUE;
  return EFI_SUCCESS;
}

/**
  Entry point function of this driver.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
SmramSaveInfoHandlerSmmMain (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS                    Status;
  VOID                          *Registration;
  EFI_SMM_SW_DISPATCH2_PROTOCOL *SmmSwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT   SmmSwDispatchContext;
  EFI_HANDLE                    DispatchHandle;

  DEBUG ((EFI_D_INFO, "SmramSaveInfoHandlerSmmMain() Enter\n"));
  ///
  /// Get SMI data register
  ///
  mSmiDataRegister = R_APM_STS;

  ///
  /// Register software SMI handler
  ///
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    NULL,
                    (VOID **)&SmmSwDispatch
                    );
  ASSERT_EFI_ERROR (Status);

  ///
  /// Locate CPU Info Protocol
  ///
  Status = gBS->LocateProtocol (
                  &gCpuInfoProtocolGuid,
                  NULL,
                  (VOID **) &mCpuInfoProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  SmmSwDispatchContext.SwSmiInputValue = mCpuInfoProtocol->CpuInfo->SmmbaseSwSmiNumber;

  Status = SmmSwDispatch->Register (
                            SmmSwDispatch,
                            (EFI_SMM_HANDLER_ENTRY_POINT2)&SmramSaveInfoHandler,
                            &SmmSwDispatchContext,
                            &DispatchHandle
                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Register SMM Ready To Lock Protocol notification
  //
  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmReadyToLockProtocolGuid,
                    SmmReadyToLockEventNotify,
                    &Registration
                    );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "SmramSaveInfoHandlerSmmMain() Exit\n"));
  return Status;
}
//[-end-151229-IB03090424-modify]//

