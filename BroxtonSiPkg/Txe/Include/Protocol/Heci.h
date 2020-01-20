/** @file
  EFI HECI Protocol

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#ifndef _EFI_HECI_H
#define _EFI_HECI_H

#include <SeCState.h>
#include <SeCChipset.h>


typedef struct _EFI_HECI_PROTOCOL EFI_HECI_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_SENDWACK) (
  IN      HECI_DEVICE      HeciDev,
  IN OUT  UINT32           *Message,
  IN OUT  UINT32           Length,
  IN OUT  UINT32           *RecLength,
  IN      UINT8            HostAddress,
  IN      UINT8            SECAddress
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_READ_MESSAGE) (
  IN      HECI_DEVICE      HeciDev,
  IN      UINT32           Blocking,
  IN      UINT32           *MessageBody,
  IN OUT  UINT32           *Length
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_SEND_MESSAGE) (
  IN      HECI_DEVICE      HeciDev,
  IN      UINT32           *Message,
  IN      UINT32           Length,
  IN      UINT8            HostAddress,
  IN      UINT8            SECAddress
  );
typedef
EFI_STATUS
(EFIAPI *EFI_HECI_RESET) (
  IN      HECI_DEVICE      HeciDev
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_INIT) (
   IN  HECI_DEVICE      HeciDev
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_REINIT) (
  IN      HECI_DEVICE      HeciDev
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_RESET_WAIT) (
  IN    HECI_DEVICE      HeciDev,
  IN    UINT32           Delay
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_GET_SEC_STATUS) (
  IN UINT32                       *Status
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_GET_SEC_MODE) (
  IN HECI_DEVICE               HeciDev,
  IN UINT32                    *Mode
  );

 ///
 /// HECI protocol provided for DXE phase The interface functions are for sending/receiving
 /// HECI messages between host and CSE.
 ///
typedef struct _EFI_HECI_PROTOCOL {
  EFI_HECI_SENDWACK       SendwACK;
  EFI_HECI_READ_MESSAGE   ReadMsg;
  EFI_HECI_SEND_MESSAGE   SendMsg;
  EFI_HECI_RESET          ResetHeci;
  EFI_HECI_INIT           InitHeci;
  EFI_HECI_RESET_WAIT     SeCResetWait;
  EFI_HECI_REINIT         ReInitHeci;
  EFI_HECI_GET_SEC_STATUS  GetSeCStatus;
  EFI_HECI_GET_SEC_MODE    GetSeCMode;
} EFI_HECI_PROTOCOL;

extern EFI_GUID gEfiHeciProtocolGuid;
extern EFI_GUID gEfiHeciSmmProtocolGuid;
extern EFI_GUID gEfiHeciSmmRuntimeProtocolGuid;
extern EFI_GUID gEfiCseEndofPostGuid;
extern EFI_GUID gEfiCseEndofServicesGuid;

#endif // _EFI_HECI_H
