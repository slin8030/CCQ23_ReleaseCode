/** @file
  This file contains the implementation of PreSiliconLib library.
  Its main purpose is to create module level variables for PreSi checks

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

#include <Uefi.h>
//#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PreSiliconLib.h>



/*

Valid Bit field values for (PcdPmcSsramBaseAddress0 + PcdPlatformIdRegisterOffset)
This is a fixed MMIO address for PreSi and at 0/13/3 for PostSi
See PlatformPkg.dec and PcdFixedAtBuild.dsc for actual BASE values

    31:27  Reserved
    26:21  Platform (0x1=SLE, 0x2=PureVP, 0x3=HVP, 0x4=Pure Simics)
    20:16  Program  (0x1=Morganfield, etc..)
    15:08  Rev ID
    07:03  Minor
    02:00  Major
*/
#define PLATFORMID_SHIFT  21
#define PROGRAMID_SHIFT   16
#define REVISIONID_SHIFT  8




UINT32
EFIAPI
GetIafwPlatformId (
  VOID
  )
{
  //
  // Fall back to Pcd value in these special cases:
  // 1) the contructor runs before the Pcd is set (only in PlatformPeim: constructor - setPcd - use)
  // 2) if called from a PreMem module when booting from SPI - the Constructor will fail to write
  //
    return PcdGet32(PcdIafwPlatformInfo) >> PLATFORMID_SHIFT & 0x1F;

}


UINT32
EFIAPI
GetIafwProgramId (
  VOID
  )
{

    return (PcdGet32(PcdIafwPlatformInfo) >> PROGRAMID_SHIFT) & 0x1F;

}


UINT32
EFIAPI
GetIafwRevisionId (
  VOID
  )
{

    return (PcdGet32(PcdIafwPlatformInfo) >> REVISIONID_SHIFT) & 0xFF;

}
