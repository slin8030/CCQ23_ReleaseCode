/** @file
  Definitions for HECI PEIM

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

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
#ifndef _HECI_PEIM_H
#define _HECI_PEIM_H

#include "HeciHpet.h"
#include "HeciCore.h"
#include <SeCAccess.h>
#include <HeciRegs.h>
#include <IndustryStandard/Pci22.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/SeCLib.h>
#include <Library/TimerLib.h>
#include <Guid/EventGroup.h>
//#include <Protocol/SeCplatformPolicy.h>

//
// Driver Produced Protocol Prototypes
//
#include <Ppi/HeciPpi.h>
#include <Protocol/SecRcInfo.h>

#define HECI_PRIVATE_DATA_SIGNATURE         SIGNATURE_32 ('H', 'e', 'c', 'i')
#define HECI_ROUND_UP_BUFFER_LENGTH(Length) ((UINT32) ((((Length) + 3) / 4) * 4))

//
// HECI private data structure
//
typedef struct {
  UINTN                   Signature;
  EFI_HANDLE              Handle;
  UINT32                  HeciMBAR;
  UINT32                  HeciMBAR0;
  UINT16                  DeviceInfo;
  UINT32                  RevisionInfo;
  EFI_HECI_PROTOCOL       HeciCtlr;
  volatile UINT32         *HpetTimer;
  EFI_SEC_RC_INFO_PROTOCOL SeCRcInfo;
} HECI_INSTANCE;
#endif
