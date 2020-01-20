/** @file
  APIs of PCH PCIE SMI Dispatch Protocol.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _SC_PCIE_SMI_DISPATCH_PROTOCOL_H_
#define _SC_PCIE_SMI_DISPATCH_PROTOCOL_H_

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                         gScPcieSmiDispatchProtocolGuid;

//
// Forward reference for ANSI C compatibility
//
typedef struct _SC_PCIE_SMI_DISPATCH_PROTOCOL    SC_PCIE_SMI_DISPATCH_PROTOCOL;

//
// Member functions
//

typedef struct {
  UINT8                                 RpIndex; ///< Root port index (0-based), 0: RP1, 1: RP2, n: RP(N+1)
  UINT8                                 BusNum;  ///< Root port pci bus number
  UINT8                                 DevNum;  ///< Root port pci device number
  UINT8                                 FuncNum; ///< Root port pci function number
} SC_PCIE_SMI_RP_CONTEXT;

/**
  Callback function for an PCH PCIE RP SMI handler dispatch.

  @param[in] DispatchHandle             The unique handle assigned to this handler by register function.
  @param[in] RpContext                  Pointer of PCH PCIE Root Port context.

**/
typedef
VOID
(EFIAPI *SC_PCIE_SMI_RP_DISPATCH_CALLBACK) (
  IN EFI_HANDLE                         DispatchHandle,
  IN SC_PCIE_SMI_RP_CONTEXT             *RpContext
  );

/**
  Register a child SMI source dispatch function for PCH PCIERP SMI events.

  @param[in] This                       Protocol instance pointer.
  @param[in] DispatchFunction           Pointer to dispatch function to be invoked for
                                        this SMI source
  @param[in] RpIndex                    Refer to PCH PCIE Root Port index.
                                        0: RP1, 1: RP2, n: RP(N+1)
  @param[out] DispatchHandle            Handle of dispatch function, for when interfacing
                                        with the parent SMM driver.

  @retval EFI_SUCCESS                   The dispatch function has been successfully
                                        registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR              The driver was unable to enable the SMI source.
  @retval EFI_OUT_OF_RESOURCES          Not enough memory (system or SMM) to manage this child.
  @retval EFI_ACCESS_DENIED             Return access denied if the EndOfDxe event has been triggered
**/
typedef
EFI_STATUS
(EFIAPI *SC_PCIE_SMI_RP_DISPATCH_REGISTER) (
  IN  SC_PCIE_SMI_DISPATCH_PROTOCOL     *This,
  IN  SC_PCIE_SMI_RP_DISPATCH_CALLBACK  DispatchFunction,
  IN  UINTN                             RpIndex,
  OUT EFI_HANDLE                        *DispatchHandle
  );

/**
  Unregister a child SMI source dispatch function with a parent PCIE SMM driver

  @param[in] This                       Protocol instance pointer.
  @param[in] DispatchHandle             Handle of dispatch function to deregister.

  @retval EFI_SUCCESS                   The dispatch function has been successfully
                                        unregistered and the SMI source has been disabled
                                        if there are no other registered child dispatch
                                        functions for this SMI source.
  @retval EFI_INVALID_PARAMETER         Handle is invalid.
  @retval EFI_ACCESS_DENIED             Return access denied if the EndOfDxe event has been triggered
**/
typedef
EFI_STATUS
(EFIAPI *SC_PCIE_SMI_DISPATCH_UNREGISTER) (
  IN  SC_PCIE_SMI_DISPATCH_PROTOCOL     *This,
  IN  EFI_HANDLE                        DispatchHandle
  );

/**
  Interface structure for PCH PCIE SMIs Dispatch Protocol
  The PCH PCIE SMI DISPATCH PROTOCOL provides the ability to dispatch function for PCH PCIE related SMIs.
  It contains SMI types of HotPlug, LinkActive, and Link EQ.
**/
struct _SC_PCIE_SMI_DISPATCH_PROTOCOL {
  //
  // Smi unregister function for PCH PCIE SMI DISPATCH PROTOCOL.
  //
  SC_PCIE_SMI_DISPATCH_UNREGISTER      UnRegister;
  //
  // PcieRpXHotPlug
  // The event is triggered when PCIE root port Hot-Plug Presence Detect.
  //
  SC_PCIE_SMI_RP_DISPATCH_REGISTER     HotPlugRegister;
  //
  // PcieRpXLinkActive
  // The event is triggered when Hot-Plug Link Active State Changed.
  //
  SC_PCIE_SMI_RP_DISPATCH_REGISTER     LinkActiveRegister;
  //
  // PcieRpXLinkEq
  // The event is triggered when Device Requests Software Link Equalization.
  //
  SC_PCIE_SMI_RP_DISPATCH_REGISTER     LinkEqRegister;
};

#endif
