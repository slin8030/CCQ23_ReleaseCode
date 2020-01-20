/** @file
  Header file definitions for SC reset

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#ifndef _SC_RESET_RT_H_
#define _SC_RESET_RT_H_

#include <PiDxe.h>

#include <Guid/EventGroup.h>
#include <Private/Guid/ScPolicyHobGuid.h>
#include <Protocol/Reset.h>
#include <Protocol/ScExtendedReset.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include "ScAccess.h"
#include <Library/BaseLib.h>
//
// Driver private data
//
#define RESET_SIGNATURE SIGNATURE_32 ('I', 'E', 'R', 'S')

typedef struct {
  UINT32                          Signature;
  EFI_HANDLE                      Handle;
  EFI_SC_EXTENDED_RESET_PROTOCOL ScExtendedResetProtocol;
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  CHAR16                          *CapsuleVariableName;
#endif
  UINTN                           PmcBase;
  UINTN                           AcpiBar;
  UINT8                           ResetSelect;
} SC_RESET_INSTANCE;

#define RESET_INSTANCE_FROM_THIS(a) \
  CR ( \
  a, \
  SC_RESET_INSTANCE, \
  ScExtendedResetProtocol, \
  RESET_SIGNATURE \
  )

/**
  Install and initialize reset protocols.

  @param[in] ImageHandle               Image handle of the loaded driver
  @param[in] SystemTable               Pointer to the System Table

  @retval EFI_SUCCESS                   Thread can be successfully created
  @retval EFI_OUT_OF_RESOURCES          Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR              Cannot create the timer service
**/
EFI_STATUS
EFIAPI
InitializeScReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

/**
  Generate system reset by Reset Control Register (IO Cf9h)

  @param[in] ResetType                 Warm or cold
  @param[in] ResetStatus               Possible cause of reset
  @param[in] DataSize                  Size of ResetData in bytes
  @param[in] ResetData                 Optional Unicode string
**/
VOID
EFIAPI
IntelScResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN VOID             *ResetData OPTIONAL
  );

#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
/**
  If need be, do any special reset required for capsules. For this
  implementation where we're called from the ResetSystem() api,
  just set our capsule variable and return to let the caller
  do a soft reset.

  @param[in] CapsuleDataPtr            Pointer to the capsule block descriptors
**/
VOID
CapsuleReset (
  IN UINTN   CapsuleDataPtr
  );
#endif

/**
  Execute SC Extended Reset from the host controller.

  @param[in] This                      Pointer to the EFI_SC_EXTENDED_RESET_PROTOCOL instance.
  @param[in] ScExtendedResetTypes      SC Extended Reset Types which includes PowerCycle, Globalreset.

  @retval EFI_SUCCESS                  Successfully completed.
  @retval EFI_INVALID_PARAMETER        If ResetType is invalid.
**/
EFI_STATUS
EFIAPI
ScExtendedReset (
  IN EFI_SC_EXTENDED_RESET_PROTOCOL   *This,
  IN SC_EXTENDED_RESET_TYPES          ScExtendedResetTypes
  );

/**
  Fixup internal data pointers so that the services can be called in virtual mode.

  @param[in]  Event                     The event registered.
  @param[in]  Context                   Event context. Not used in this event handler.
**/
VOID
EFIAPI
ScResetVirtualddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );
#endif
