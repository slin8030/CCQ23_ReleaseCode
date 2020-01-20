/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/
//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c) 2010 - 2016, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformCmosLib.h

Abstract:

--*/

#ifndef _PLATFORM_CMOS_LIB_H_
#define _PLATFORM_CMOS_LIB_H_

#define CMOS_ATTRIBUTE_EXCLUDE_FROM_CHECKSUM 0x1

typedef struct {
  UINT8 CmosAddress;
  UINT8 DefaultValue;
  UINT8 Attribute;
} CMOS_ENTRY;

//[-start-160526-IB10860200-remove]//
///**
//  Funtion to return platform CMOS entry.
//
//  @param [out]  CmosEntry  Platform CMOS entry.
//
//  @param [out]  CmosEnryCount Number of platform CMOS entry.
//
//  @return Status.
//**/
//RETURN_STATUS
//EFIAPI
//GetPlatformCmosEntry (
//  OUT CMOS_ENTRY  **CmosEntry,
//  OUT UINTN       *CmosEnryCount
//  );
//[-end-160526-IB10860200-remove]//

/**
  Funtion to check if Battery lost or CMOS cleared.

  @reval TRUE  Battery is always present.
  @reval FALSE CMOS is cleared.
**/
BOOLEAN
EFIAPI
CheckCmosBatteryStatus (
  VOID
  );

//[-start-160628-IB07400749-add]//
VOID
EFIAPI
ClearCmosBatteryStatus (
  VOID
  );
//[-end-160628-IB07400749-add]//
//[-start-180621-IB07400979-add]//
VOID
ChipsetLoadCmosDefault (
  VOID
  );
//[-end-180621-IB07400979-add]//
#endif // _PLATFORM_CMOS_LIB_H_
