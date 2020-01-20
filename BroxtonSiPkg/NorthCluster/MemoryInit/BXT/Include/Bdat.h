/** @file
  This file contains Initialization of the BDAT data structures for both
  v1.0 and v2.0 taken from v0.6 of "A Compatible Method to Access BIOS
  Data Structures".

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

#ifndef _BDAT_H_
#define _BDAT_H_

///
/// BDAT definitions
///
#define MAX_NODE              1                     ///< Max processors per system
#define MAX_DIMM              2                     ///< Max DIMMs per channel
#define MAX_RANK_DIMM         2                     ///< Max ranks per DIMM
//#define MAX_BITS             72
#define MAX_SPD_BYTE        256                     ///< Number of bytes in Serial EEPROM

VOID
FillRMTBdatStructure (
  IN  OUT   MMRC_DATA         *MrcData,
      OUT   VOID              *RmtDataBuffer,
  IN        UINT32            RmtDataBufferSize
);

UINT32
GetRMTStructSize (
  IN  OUT   MMRC_DATA         *MrcData
);

#endif // _BDAT_H_
