/** @file
  EFI HECI PPI

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
#ifndef _HECI_PPI_H
#define _HECI_PPI_H

#include <SeCState.h>
#include <Protocol/Heci.h>

typedef struct _EFI_HECI_PPI EFI_HECI_PPI;

///
/// The interface functions are for sending/receiving HECI messages between host and CSE in PEI phase.
///
typedef struct _EFI_HECI_PPI {
  EFI_HECI_SENDWACK       SendwACK;
  EFI_HECI_READ_MESSAGE   ReadMsg;
  EFI_HECI_SEND_MESSAGE   SendMsg;
  EFI_HECI_RESET          ResetHeci;
  EFI_HECI_INIT           InitHeci;
  EFI_HECI_RESET_WAIT     SeCResetWait;
  EFI_HECI_REINIT         ReInitHeci;
  EFI_HECI_GET_SEC_STATUS  GetSeCStatus;
  EFI_HECI_GET_SEC_MODE    GetSeCMode;
} EFI_HECI_PPI;

extern EFI_GUID gEfiHeciPpiGuid;

#endif // _HECI_PPI_H
