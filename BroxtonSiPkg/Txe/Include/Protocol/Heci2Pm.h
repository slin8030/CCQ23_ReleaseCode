/** @file
  EFI HECI2 Power Management Protocol

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

@par Specification
**/
#ifndef _EFI_HECI2_PM_PROTOCOL_H
#define _EFI_HECI2_PM_PROTOCOL_H

extern EFI_GUID gEfiHeci2PmProtocolGuid;

typedef
BOOLEAN
(EFIAPI *EFI_HECI2_PM_IS_IDLE) (
  VOID
  );

typedef
VOID
(EFIAPI *EFI_HECI2_PM_SET_ACTIVE) (
  VOID
  );

typedef
VOID
(EFIAPI *EFI_HECI2_PM_SET_IDLE) (
  VOID
  );

typedef
UINTN
(EFIAPI *EFI_HECI2_PM_GET_HECIBAR) (
  VOID
  );

typedef
VOID
(EFIAPI *EFI_HECI2_PM_SET_HECIBAR) (
  UINTN
  );

typedef
BOOLEAN
(EFIAPI *EFI_HECI2_PM_AT_RUNTIME) (
  VOID
  );

typedef struct _EFI_HECI2_PM_PROTOCOL {
  EFI_HECI2_PM_IS_IDLE     IsIdle;
  EFI_HECI2_PM_SET_ACTIVE  SetActive;
  EFI_HECI2_PM_SET_IDLE    SetIdle;
  EFI_HECI2_PM_GET_HECIBAR GetHeciBar;
  EFI_HECI2_PM_SET_HECIBAR SetHeciBar;
  EFI_HECI2_PM_AT_RUNTIME  AtRuntime;
} EFI_HECI2_PM_PROTOCOL;

#endif
