/** @file
  This protocol is used to report and control what BIOS is mapped to the
  BIOS address space anchored at 4GB boundary.

  This protocol is EFI compatible.

  E.G. For current generation ICH, the 4GB-16MB to 4GB range can be mapped
  to PCI, SPI, or FWH.

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


#ifndef _EFI_ACTIVE_BIOS_PROTOCOL_H_
#define _EFI_ACTIVE_BIOS_PROTOCOL_H_

//
// Define the  protocol GUID
//
#define EFI_ACTIVE_BIOS_PROTOCOL_GUID  \
  { 0xebbe2d1b, 0x1647, 0x4bda, {0xab, 0x9a, 0x78, 0x63, 0xe3, 0x96, 0xd4, 0x1a} }

typedef struct _EFI_ACTIVE_BIOS_PROTOCOL EFI_ACTIVE_BIOS_PROTOCOL;

//
// Protocol definitions
//
typedef enum {
  ActiveBiosStateSpi,
  ActiveBiosStatePci,
  ActiveBiosStateLpc,
  ActiveBiosStateMax
} EFI_ACTIVE_BIOS_STATE;

/**
  Change the current active BIOS settings to the requested state.
  The caller is responsible for requesting a supported state from
  the EFI_ACTIVE_BIOS_STATE selections.

  This will fail if someone has locked the interface and the correct key is
  not provided.

  @param[in]  This                    Pointer to the EFI_ACTIVE_BIOS_PROTOCOL instance.
  @param[in]  DesiredState            The requested state to configure the system for.
  @param[in]  Key                     If the interface is locked, Key must be the Key
                                      returned from the LockState function call.

  @retval  EFI_SUCCESS             Command succeed.
  @retval  EFI_ACCESS_DENIED       The interface is currently locked.
  @retval  EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_ACTIVE_BIOS_SET_ACTIVE_BIOS_STATE) (
  IN EFI_ACTIVE_BIOS_PROTOCOL     *This,
  IN EFI_ACTIVE_BIOS_STATE        DesiredState,
  IN UINTN                        Key
  );

/**
  Lock the current active BIOS state from further changes.  This allows a
  caller to implement a critical section.  This is optionally supported
  functionality.  Size conscious implementations may choose to require
  callers cooperate without support from this protocol.

  @param[in] This                    Pointer to the EFI_ACTIVE_BIOS_PROTOCOL instance.
  @param[in] Lock                    TRUE to lock the current state, FALSE to unlock.
  @param[in] Key                     If Lock is TRUE, then a key will be returned.  If
                                     Lock is FALSE, the key returned from the prior call
                                     to lock the protocol must be provided to unlock the
                                     protocol.  The value of Key is undefined except that it
                                     will never be 0.

  @retval    EFI_SUCCESS             Command succeed.
  @retval    EFI_UNSUPPORTED         The function is not supported.
  @retval    EFI_ACCESS_DENIED       The interface is currently locked.
  @retval    EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_ACTIVE_BIOS_LOCK_ACTIVE_BIOS_STATE) (
  IN     EFI_ACTIVE_BIOS_PROTOCOL   *This,
  IN     BOOLEAN                    Lock,
  IN OUT UINTN                      *Key
  );


///
/// Protocol definition
///
/// Note that some functions are optional.  This means that they may be NULL.
/// Caller is required to verify that an optional function is defined by checking
/// that the value is not NULL.
///
struct _EFI_ACTIVE_BIOS_PROTOCOL {
  EFI_ACTIVE_BIOS_STATE                       State;
  EFI_ACTIVE_BIOS_SET_ACTIVE_BIOS_STATE       SetState;
  EFI_ACTIVE_BIOS_LOCK_ACTIVE_BIOS_STATE      LockState;
};

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gEfiActiveBiosProtocolGuid;

#endif
