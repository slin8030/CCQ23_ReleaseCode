/** @file
  This file the include all the external MRC Function.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

#ifndef _MRCFUNC_H_
#define _MRCFUNC_H_

#include "Mrc.h"

/**
  Bit scan forward 8 bits.

  @param[in]  Input

  @retval  UINT8
**/
UINT8
BitScanForward8 (
  IN        UINT8   Input
  );

/**
  Bit scan reverse 16 bits.

  @param[in]  Input

  @retval  INT8
**/
INT8
BitScanReverse16 (
  IN        UINT16   Input
  );

/**
  Fill input structure.

  @param[in,out]  MrcData

  @retval  MMRC_STATUS
**/
MMRC_STATUS
FillInputStructure (
  IN  OUT   MMRC_DATA   *MrcData
  );

/**
  FillOutputStructure

  @param[in,out]  MrcData

  @retval  MMRC_STATUS
**/
MMRC_STATUS
FillOutputStructure (
  IN  OUT   MMRC_DATA   *MrcData
  );

/**
  Returns the IAFW version.

  @param[in,out]      MrcData        MRC Data Structure
  @param[out]         IafwVersion    The IAFW version

**/
VOID
GetIafwVersion (
  IN  OUT   MMRC_DATA   *MrcData,
  OUT       MrcVersion  *IafwVersion
  );

/**
  Returns a UINT32 value representing the MRC and MMRC versions

  @param[in,out]      MrcData        MRC Data Structure

  @retval             MrcVersion     A UINT32 representation of the MRC Version
                                     [31:16] = Mrc Version
                                     [15:0]  = MMRC Version
**/
UINT32
GetMrcVersions (
  IN  OUT   MMRC_DATA   *MrcData
);

/**
  Calculates a CRC-32 of the specified data buffer.

  @param[in] Data     - Pointer to the data buffer.
  @param[in] DataSize - Size of the data buffer, in bytes.

  @retval The CRC-32 value.
**/
extern
UINT32
MrcCalculateCrc32 (
  IN     const UINT8  *const Data,
  IN     const UINT32 DataSize
  );
#endif

