/** @file
  Prototype of the PreSiliconLib library.

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

#ifndef _PRE_SI_LIB_H_
#define _PRE_SI_LIB_H_

//extern UINT32 mIafwPlatformInfo;


#define VALUE_SLE_UPF        0x11  // 0x11 HSLE and CSLE Unified Power Format
#define VALUE_PURE_SIMICS    0x4   // 0x4=Pure Simics
#define VALUE_PUREVP         0x2   // 0x2=PureVP
#define VALUE_INNOVATOR_HVP  0x3   // 0x3=HVP
#define VALUE_SLE            0x1   // 0x1=HSLE and CSLE
#define VALUE_REAL_PLATFORM  0x0   // 0x0=Not a PreSi Platform


#define PLATFORM_ID  GetIafwPlatformId()
#define PROGRAM_ID   GetIafwProgramId()
#define REVISION_ID  GetIafwRevisionId()

/**
  Get Iafw platform ID

  @retval UNIT32            Platform ID.
**/
UINT32
EFIAPI
GetIafwPlatformId (
  VOID
  );

/**
  Get Iafw Program ID

  @retval UNIT32            Program ID.
**/
UINT32
EFIAPI
GetIafwProgramId (
  VOID
  );

/**
  Get Iafw Revision ID

  @retval UNIT32            Revision ID .
**/
UINT32
EFIAPI
GetIafwRevisionId (
  VOID
  );


#endif // _PRE_SI_LIB_H_
