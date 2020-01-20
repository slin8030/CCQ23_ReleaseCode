/** @file

@brief
  SmmIchnDispatch Extended Protocol

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#ifndef _EFI_SMM_ICHN_DISPATCH_EX_H_
#define _EFI_SMM_ICHN_DISPATCH_EX_H_

#include <Protocol/SmmIchnDispatch.h>

#define EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL_GUID \
  { \
    0x3920405b, 0xc897, 0x44da, 0x88, 0xf3, 0x4c, 0x49, 0x8a, 0x6f, 0xf7, 0x36 \
  }
extern EFI_GUID                                   gEfiSmmIchnDispatchExProtocolGuid;

///
/// Forward reference for ANSI C compatibility
///
typedef struct _EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL;

///
/// Related Definitions
///
///
/// Ichn Dispatch Extended Types
///
typedef enum {
  IchnExPciExpress = NUM_ICHN_TYPES + 1,
  IchnExMonitor,
  IchnExSpi,
  IchnExQRT,
  IchnExGpioUnlock,
  IchnExTmrOverflow,
  IchnExPcie0Hotplug,
  IchnExPcie1Hotplug,
  IchnExPcie2Hotplug,
  IchnExPcie3Hotplug,
  IchnExPcie5Hotplug,
  IchnExPcie6Hotplug,
  IchnExPcie0LinkActive,
  IchnExPcie1LinkActive,
  IchnExPcie2LinkActive,
  IchnExPcie3LinkActive,
  IchnExPcie5LinkActive,
  IchnExPcie6LinkActive,
  ///
  /// INSERT NEW ITEMS JUST BEFORE THIS LINE
  ///
  IchnExTypeMAX // the maximum number of items in this enumeration
} EFI_SMM_ICHN_EX_SMI_TYPE;

typedef struct {
  EFI_SMM_ICHN_EX_SMI_TYPE  Type;
} EFI_SMM_ICHN_DISPATCH_EX_CONTEXT;

///
/// Member functions
///
/**
  @brief
  Dispatch function for a ICH n Extended specific SMI handler.

  @param[in] DispatchHandle       Handle of this dispatch function.
  @param[in] DispatchContext      Pointer to the dispatch function's context.
                                  The DispatchContext fields are filled in
                                  by the dispatching driver prior to
                                  invoking this dispatch function.

  @retval None
**/
typedef
VOID
(EFIAPI *EFI_SMM_ICHN_DISPATCH_EX) (
  IN  EFI_HANDLE                                DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT          * DispatchContext
  );

/**
  @brief
  Register a child SMI source dispatch function with a parent SMM driver

  @param[in] This                 Protocol instance pointer.
  @param[in] DispatchFunction     Pointer to dispatch function to be invoked for
                                  this SMI source
  @param[in] DispatchContext      Pointer to the dispatch function's context.
                                  The caller fills this context in before calling
                                  the register function to indicate to the register
                                  function the ICHN SMI source for which the dispatch
                                  function should be invoked.
  @param[in] DispatchHandle       Handle of dispatch function, for when interfacing
                                  with the parent SMM driver.

  @retval EFI_SUCCESS             The dispatch function has been successfully
                                  registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR        The driver was unable to enable the SMI source.
  @retval EFI_OUT_OF_RESOURCES    Not enough memory (system or SMM) to manage this
                                  child.
  @retval EFI_INVALID_PARAMETER   DispatchContext is invalid. The ICHN input value
                                  is not within valid range.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMM_ICHN_EX_REGISTER) (
  IN  EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL   * This,
  IN  EFI_SMM_ICHN_DISPATCH_EX            DispatchFunction,
  IN  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT    * DispatchContext,
  OUT EFI_HANDLE                          * DispatchHandle
  );

/**

  @brief
  Unregister a child SMI source dispatch function with a parent SMM driver

  @param[in] This                 Protocol instance pointer.
  @param[in] DispatchHandle       Handle of dispatch function to deregister.

  @retval EFI_SUCCESS             The dispatch function has been successfully
                                  unregistered and the SMI source has been disabled
                                  if there are no other registered child dispatch
                                  functions for this SMI source.
  @retval EFI_INVALID_PARAMETER   Handle is invalid.
  @retval Others                  TBD

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMM_ICHN_EX_UNREGISTER) (
  IN  EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL         * This,
  IN  EFI_HANDLE                                DispatchHandle
  );


/**
  This protocol provides a parent dispatch service for ICHN Specific SMIs
  This protocol extends EDK EFI_SMM_ICHN_DISPATCH_PROTOCOL.
**/
struct _EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL {
  EFI_SMM_ICHN_EX_REGISTER    Register;
  EFI_SMM_ICHN_EX_UNREGISTER  UnRegister;
};

#endif
