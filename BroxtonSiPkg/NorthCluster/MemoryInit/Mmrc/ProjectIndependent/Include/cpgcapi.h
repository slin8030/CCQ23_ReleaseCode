/** @file

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
#ifndef _CPGCIMP_H_
#define _CPGCIMP_H_

#include "Mmrc.h"
#include "MmrcProjectData.h"
#include "MmrcHooks.h"
#include "MmrcLibraries.h"

//long

#define CPGC_CONFIG_EARLYTRAINING 0
#define CPGC_CONFIG_VA 1
#define CPGC_CONFIG_TURNAROUNDS 2
#define CPGC_CONFIG_LFSR_ADDRESS 3
#define CPGC_CONFIG_CMD_STRESS 4
#define CPGC_CONFIG_MEMINIT 5
#define CPGC_LDO 6
#define CPGC_TRAFFIC 7
#define CPGC_CONFIG_EARLY_MPR_TRAINING 8
#define CPGC_CONFIG_EARLY_CMD_TRAINING 9
#define CPGC_MEMORY_INIT 10

enum {
  DQ_Setup,CADB_Setup,CADB_LFSR,DQ_LFSR
};

enum {
  CPGC_CHUNK_EVEN,CPGC_CHUNK_ODD,CPGC_CHUNK_ALL
};

extern UINT32 GetAddress (MMRC_DATA *MrcData, UINT8 Channel, UINT8 Rank);

/**
  L_Cpgc setup

  @param[in,out]   MrcData
  @param[in]       Channel
  @param[in]       SetupType

  @retval      MMRC_STATUS
**/
MMRC_STATUS L_CpgcSetup (
    IN OUT  MMRC_DATA  *MrcData,
    IN      UINT8      Channel,
    IN      UINT8      SetupType
);

/**
  L_Cpgc reconfig

  @param[in,out]   MrcData
  @param[in]       Channel

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcReconfig (
  IN OUT  MMRC_DATA  *MrcData,
  IN      UINT8      Channel
);

/**
  L_Cpgc runTest

  @param[in,out]    MrcData
  @param[in]        Channel
  @param[out]       BytelaneStatus
  @param[out]       LANE_ERR_LO
  @param[out]       LANE_ERR_HI
  @param[out]       ErrorCount

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcRunTest (
IN OUT  MMRC_DATA  *MrcData,
IN      UINT8      Channel,
OUT     UINT16     *BytelaneStatus,
OUT     UINT32     *LANE_ERR_LO,
OUT     UINT32     *LANE_ERR_HI,
OUT     UINT32     *ECC_LANE_ERR,
OUT     UINT32     *ErrorCount
);

/**
  L_Cpgc run cadb test

  @param[in]       ModMrcData
  @param[in]       CapsuleStartIndex
  @param[in]       StringIndex
  @param[in]       Channel
  @param[in]       pTestPassed

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcRunCadbTest (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  BOOLEAN * const   pTestPassed
);

/**
  L_Cpgc rotate cadb

  @param[in]       ModMrcData
  @param[in]       CapsuleStartIndex
  @param[in]       StringIndex
  @param[in]       Channel
  @param[in]       rotationN
  @param[in]       pRotationDone

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcRotateCadb (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  UINT8             rotationN,          // pass in which rotation to use
  UINT8 * const     pRotationDone       // return value of BOOLEAN - is rotation finished?
);

/**
  L_Cpgc initialize.

  @param[in]       MrcData
  @param[in]       CapsuleStartIndex
  @param[in]       StringIndex
  @param[in]       Channel

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcInit(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
);

/**
  L_Cpgc enable

  @param[in]       MrcData
  @param[in]       CapsuleStartIndex
  @param[in]       StringIndex
  @param[in]       Channel

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcEnable(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
);

/**
  L_Cpgc disable

  @param[in]       MrcData
  @param[in]       CapsuleStartIndex
  @param[in]       StringIndex
  @param[in]       Channel

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcDisable(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
);

/**
  L_Cpgc stop test

  @param[in]       MrcData
  @param[in]       CapsuleStartIndex
  @param[in]       StringIndex
  @param[in]       Channel

  @retval      MMRC_STATUS
**/
MMRC_STATUS
L_CpgcStopTest(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
);


#endif
