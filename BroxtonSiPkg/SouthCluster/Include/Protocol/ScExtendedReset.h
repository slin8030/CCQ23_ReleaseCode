/** @file
  SC Extended Reset Protocol

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2008 - 2016 Intel Corporation.

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
#ifndef _EFI_EXTENDED_RESET_H_
#define _EFI_EXTENDED_RESET_H_




#define EFI_EXTENDED_RESET_PROTOCOL_GUID \
  { \
    0xf0bbfca0, 0x684e, 0x48b3, 0xba, 0xe2, 0x6c, 0x84, 0xb8, 0x9e, 0x53, 0x39 \
  }
extern EFI_GUID                                 gEfiExtendedResetProtocolGuid;

//
// Forward reference for ANSI C compatibility
//
typedef struct _EFI_EXTENDED_RESET_PROTOCOL EFI_SC_EXTENDED_RESET_PROTOCOL;

//
// Related Definitions
//
//
// SC Extended Reset Types
//
typedef struct {
  UINT8 PowerCycle : 1;   ///< 0: Disabled*; 1: Enabled
  UINT8 GlobalReset : 1;  ///< 0: Disabled*; 1: Enabled
  UINT8 SusPwrDnAck : 1;  ///< 0: Do Nothing;
                          ///< 1: GPIO[30](SUS_PWR_DN_ACK) level is set low prior to Global Reset(for systems with an embedded controller)
  UINT8 RsvdBits : 5;     ///< Reserved fields for future expansion w/o protocol change
} SC_EXTENDED_RESET_TYPES;

//
// Member functions
//
/**
  Execute SC Extended Reset from the host controller.

  @param[in]  This                    Pointer to the EFI_SC_EXTENDED_RESET_PROTOCOL instance.
  @param[in]  ScExtendedResetTypes    SC Extended Reset Types which includes PowerCycle, Globalreset.

  @retval  EFI_INVALID_PARAMETER      If ResetType is invalid.

  Does not return if the reset takes place.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_SC_EXTENDED_RESET) (
  IN     EFI_SC_EXTENDED_RESET_PROTOCOL   * This,
  IN     SC_EXTENDED_RESET_TYPES          ScExtendedResetTypes
  );



/**
  This protocol is used to execute South Cluster Extended Reset from the host controller.
**/
struct _EFI_EXTENDED_RESET_PROTOCOL {
  EFI_SC_EXTENDED_RESET  Reset;
};

#endif
