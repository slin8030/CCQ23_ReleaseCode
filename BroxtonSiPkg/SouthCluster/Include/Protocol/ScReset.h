/** @file
  SC Reset Protocol

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
#ifndef _SC_RESET_H_
#define _SC_RESET_H_


//
#define RESET_PROTOCOL_GUID \
  { \
    0xdb63592c, 0xb8cc, 0x44c8, 0x91, 0x8c, 0x51, 0xf5, 0x34, 0x59, 0x8a, 0x5a \
  }
#define RESET_CALLBACK_PROTOCOL_GUID \
  { \
    0x3a3300ab, 0xc929, 0x487d, 0xab, 0x34, 0x15, 0x9b, 0xc1, 0x35, 0x62, 0xc0 \
  }
extern EFI_GUID                             gScResetProtocolGuid;
extern EFI_GUID                             gScResetCallbackProtocolGuid;

///
/// Forward reference for ANSI C compatibility
///
typedef struct _SC_RESET_PROTOCOL          SC_RESET_PROTOCOL;

typedef struct _SC_RESET_CALLBACK_PROTOCOL SC_RESET_CALLBACK_PROTOCOL;

///
/// Related Definitions
///
///
/// SC Reset Types
///
typedef enum {
  ColdReset,
  WarmReset,
  ShutdownReset,
  PowerCycleReset,
  GlobalReset,
  GlobalResetWithEc
} SC_RESET_TYPE;

///
/// Member functions
///
/**
  @brief
  Execute SC Reset from the host controller.

  @param[in] This                 Pointer to the SC_RESET_PROTOCOL instance.
  @param[in] ScResetType          SC Reset Types which includes ColdReset, WarmReset, ShutdownReset,
                                  PowerCycleReset, GlobalReset, GlobalResetWithEc

  @retval EFI_SUCCESS             Successfully completed.
  @retval EFI_INVALID_PARAMETER   If ResetType is invalid.
**/
typedef
EFI_STATUS
(EFIAPI *SC_RESET) (
  IN     SC_RESET_PROTOCOL       * This,
  IN     SC_RESET_TYPE           ScResetType
  )
;

/**
  @brief
  Execute call back function for SC Reset.

  @param[in] ScResetType         SC Reset Types which includes PowerCycle, Globalreset.

  @retval EFI_SUCCESS             The callback function has been done successfully
  @retval EFI_NOT_FOUND           Failed to find SC Reset Callback protocol. Or, none of
                                  callback protocol is installed.
  @retval Others                  Do not do any reset from SC
**/
typedef
EFI_STATUS
(EFIAPI *SC_RESET_CALLBACK) (
  IN     SC_RESET_TYPE           ScResetType
  )
;

///
/// Interface structure for the SC Reset Protocol
///
struct _SC_RESET_PROTOCOL {
  SC_RESET Reset;
};

/**
  This protocol is used to execute South Cluster Reset from the host controller.
**/
struct _SC_RESET_CALLBACK_PROTOCOL {
  SC_RESET_CALLBACK  ResetCallback;
};

#endif
