/** @file
  This file inlcude external Memory DIMM initialization.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

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

#ifndef _MCFUNC_H_
#define _MCFUNC_H_

#include "Mrc.h"
#include "MmrcHooks.h"
#ifdef FSP_FLAG
#include <Library/FspCommonLib.h>
#include <FspEas.h>
#include <FspmUpd.h>
#endif
extern UINT16     mFrequencyMultiplier[C_MAXDDR];
//extern UINT8      mDdrFreq2WLTable[C_MAXDDR];
//extern UINT8      mFawTableDdr3[C_MAXDDR][2];
//extern UINT8      mFawTableDdr4[C_MAXDDR][2];

typedef union {
  struct {
    unsigned int  IAFW_version       : 16; ///< [15:0]: MAJOR/MINOR revision for IAFW
    unsigned int  HALT_before_MRC    : 1;  ///< [16]:   JMP$ before MRC
    unsigned int  HALT_after_MRC     : 1;  ///< [17]:   JMP$ after MRC
    unsigned int  PcBreaking         : 1;  ///< [18]:   JMP$ in JEDEC Init
    unsigned int  TakeHeadlessPath   : 1;  ///< [19]:   Change Bootmode to support Headless flow
    unsigned int  SkipMemTest        : 1;  ///< [20]:   Bypass the memory test at end of MemInit
    unsigned int  LogMaxEnable       : 1;  ///< [21]:   Generating MAX logs for debug
    unsigned int  RunRMT             : 1;  ///< [22]:   Run RMT when set -> MrcData.MrcRmtSupport
    unsigned int  ScrambingEn        : 1;  ///< [23]:   Not required to enable scrambling but will ensure it is
    unsigned int  DualRank           : 1;  ///< [24]:   Disable Dual Rank when '1'
    unsigned int  DynamicSrDisable   : 1;  ///< [25]:   DynamicSr Disable: Disables Dynamic SR when set to 1
    unsigned int  LctDisable         : 1;  ///< [25]:   Disable MRC Late Command Training when set to 1
    unsigned int  availSV            : 1;  ///< [27:27]: FutureExpansion
    unsigned int  RAMDebugEnable     : 4;  ///< [31:28]: Used for RAM debug enabling
  } SKPD_bits;
  UINT32  Data;
} SKPD_struct;


typedef union {
  struct {
    unsigned int  postcode           :  8;  ///<   [7:0]:  post-code copy for automation purposes
    unsigned int  resv               :  1;  ///<     [8]:  Reserved
    unsigned int  readPBD            :  1;  ///<     [9]:  enables Read per-bit-deskew
    unsigned int  writePBD           :  1;  ///<    [10]:  enables Write per-bit-deskew
    unsigned int  speed              :  2;  ///< [12:11]:  00=1600;01=2133;10=2667;11=3200
    unsigned int  NoFB               :  1;  ///<    [13]:  Forces S5 path when set - won't use FB data
    unsigned int  RMTmode            :  2;  ///< [15:14]:  RMT cmd mode:ev mode; used with RunRMT in SKPD
    unsigned int  BreakPostcode      :  8;  ///< [23:16]:  Post-code register for breaking
    unsigned int  Reserved           :  8;  ///< [32:24]:  FutureExpansion
  } DEBUP0_bits;
  UINT32  Data;
} DEBUP0_struct;


typedef union {
  struct {
    UINT32  Type               :  2;  //   [1:0]
    UINT32  Density            :  4;  //   [5:2]
    UINT32  IOWidth            :  2;  //   [7:6]
    UINT32  Reserved           :  24;  //  [31:8]
  } Bits;
  UINT32  Data;
} MRR8;

#define RAMDEBUG                           0xF0000000  //31:28

/**
  Mc stall.

  @param[in]  DelayHPET

  @retval  None
**/
VOID
McStall (
  IN      UINT32    DelayHPET
);

/*++

Routine Description:

  Save/Restores platform specific trained values.

Arguments:

  *MrcData           :  Main MRC data structure.

Returns:

  Nothing

--*/

VOID
ProjectSpecificSaveRestores (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  SAVE_RESTORE_TASK Task
);

/*++

Routine Description:

  Restores trained values for S0/S3/FB paths

Arguments:

  *MrcData           :
  CapsuleStartIndex  :
  StringIndex        :
  Channel            :

Returns:

  Success/Fail

--*/
MMRC_STATUS
RestoreAlgos (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
);

/*++

Routine Description:

  Saves trained values for S0/S3/FB paths

Arguments:

  *MrcData           :
  CapsuleStartIndex  :
  StringIndex        :
  Channel            :

Returns:

  Success/Fail

--*/
MMRC_STATUS
SaveAlgos(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
);
/*++

Routine Description:

  Configures a checkpoint to halt the CPU in a specific memory training step


Arguments:

  CheckPoint:       Specifies the code related to an MRC step

Returns:

  None

--*/
VOID
HandleCheckpoint (
  IN        MMRC_DATA *MrcData,
  IN        UINT8      CheckPoint
);

MMRC_STATUS
DISPLAYMMRCVERSION (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
);

MMRC_STATUS
DisplayInputParameters(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
);

/**
  Mc stall with nano second.

  @param[in]  DelayHPET

  @retval  None
**/
VOID
McStallNanoSecond (
  IN      UINT32    DelayHPET
);

/**
  Initilialize the MRC parameters with default values and/or from setup options

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
MrcInitializeParameters (
  IN  OUT   MMRC_DATA   *MrcData
);

/**
  This function gets the platform setup including all BAR address

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
GetPlatformSettings (
  IN  OUT   MMRC_DATA   *MrcData
);

/**
  This function determines which kind of boot path needs to be followed in MRC flow

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
DetermineBootMode (
  IN  OUT   MMRC_DATA   *MrcData
);

/**
  Enables the High Precision Event Timer

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
McEnableHpet (
  IN  OUT   MMRC_DATA   *MrcData
);

/**
  Disables the High Precision Event Timer

  @param[in,out]  MrcData   Host structure for all MRC global data.
  @param[in]      Channel   Current Channel being examined.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
McDisableHPET (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Channel
);

/**
  Get core stepping.

  @param[in]  None

  @retval  UINT8
**/
UINT8
GetCoreStepping ();

/**
  Get cpu type.

  @param[in]  None

  @retval  UINT16
**/
UINT16
GetCpuType ();

/**
  get_initial_seed

  @param[in]  None

  @retval  UINT32

**/
UINT32
get_initial_seed ();

/**
  This function configures the scrambler

  @param[in]  MrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
SetScrambler (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
);

/**
  Calculates the largest integer that is both
  a power of two and less than Input

  @param[in]  Operand   value to calculate power of two

  @retval  the largest integer that is both  a power of two and less than Input
**/
UINT32
GetPowerOfTwo (
  IN        UINT32        Operand
);

#endif

