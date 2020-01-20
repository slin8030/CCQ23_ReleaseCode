/** @file
  SC BIOS Write Protect Driver.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
#include "ScBiosWriteProtect.h"

///
/// Global variables
///
EFI_SMM_ICHN_DISPATCH_PROTOCOL  *mIchnDispatch;
EFI_SMM_SW_DISPATCH2_PROTOCOL   *mSwDispatch;
UINTN                           mPciSpiRegBase;

/**
  This hardware SMI handler will be run every time the BIOS Write Enable bit is set.

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None
**/
VOID
EFIAPI
ScBiosWpCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  )
{
  ///
  /// Disable BIOSWE bit to protect BIOS
  ///
  MmioAnd8 ((UINTN) (mPciSpiRegBase + R_SPI_BCR), (UINT8) ~B_SPI_BCR_BIOSWE);
  //
  // Clear Sync SMI Status
  //
  MmioOr16 ((UINTN) (mPciSpiRegBase + R_SPI_BCR), (UINT16) B_SPI_BCR_SYNC_SS);
}

/**
  Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  SMM_BWP and BLE bits will be set here

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None
**/
VOID
EFIAPI
ScBiosLockSwSmiCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_REGISTER_CONTEXT   *DispatchContext
  )
{
  EFI_STATUS                    Status;
  EFI_SMM_ICHN_DISPATCH_CONTEXT IchnContext;
  EFI_HANDLE                    IchnHandle;

  if (mIchnDispatch == NULL) {
    return;
  }

  IchnHandle = NULL;

  ///
  /// Set SMM_BWP bit before registering IchnBiosWp
  ///
  MmioOr8 ((UINTN) (mPciSpiRegBase + R_SPI_BCR), (UINT8) B_SPI_BCR_SMM_BWP);
  ///
  /// Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  ///
  IchnContext.Type = IchnBiosWp;
  Status = mIchnDispatch->Register (
                            mIchnDispatch,
                            ScBiosWpCallback,
                            &IchnContext,
                            &IchnHandle
                            );
  ASSERT_EFI_ERROR (Status);
}

/**
  Entry point for SC Bios Write Protect driver.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
**/
EFI_STATUS
EFIAPI
InstallScBiosWriteProtect (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
{
  EFI_STATUS                  Status;
  SC_POLICY_HOB               *ScPolicy;
  EFI_HANDLE                  SwHandle;
  EFI_SMM_SW_REGISTER_CONTEXT SwContext;

  EFI_PEI_HOB_POINTERS        HobPtr;
  SC_LOCK_DOWN_CONFIG         *LockDownConfig;
  ///
  /// Get Sc Platform Policy Hob
  ///
  HobPtr.Guid   = GetFirstGuidHob (&gScPolicyHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  ScPolicy = (SC_POLICY_HOB*) GET_GUID_HOB_DATA (HobPtr.Guid);
  Status = GetConfigBlock ((VOID *) ScPolicy, &gLockDownConfigGuid, (VOID *) &LockDownConfig);
  ASSERT_EFI_ERROR (Status);

  if ((LockDownConfig->BiosLock == TRUE)) {
    mPciSpiRegBase = MmPciBase (
                         DEFAULT_PCI_BUS_NUMBER_SC,
                         PCI_DEVICE_NUMBER_SPI,
                         PCI_FUNCTION_NUMBER_SPI
                         );
    ///
    /// Get the ICHn protocol
    ///
    mIchnDispatch = NULL;
    Status        = gSmst->SmmLocateProtocol (&gEfiSmmIchnDispatchProtocolGuid, NULL, (VOID **) &mIchnDispatch);
    ASSERT_EFI_ERROR (Status);
    ///
    /// Locate the SC SMM SW dispatch protocol
    ///
    SwHandle  = NULL;
    Status    = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID **) &mSwDispatch);
    ASSERT_EFI_ERROR (Status);
    ///
    /// Register BIOS Lock SW SMI handler
    ///
    SwContext.SwSmiInputValue = LockDownConfig->BiosLockSwSmiNumber;
    Status = mSwDispatch->Register (
                            mSwDispatch,
                            (EFI_SMM_HANDLER_ENTRY_POINT2)ScBiosLockSwSmiCallback,
                            &SwContext,
                            &SwHandle
                            );
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}
