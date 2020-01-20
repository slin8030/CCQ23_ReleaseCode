/** @file
  This file include all the external DetectDimm.

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


#ifndef _DETECTDIMMS_H_
#define _DETECTDIMMS_H_

#include "Mrc.h"
#include "McFunc.h"
#include "MrcFunc.h"
#include "ConfigMemData.h"

//
// beep tones
//
#define NOTE(x) ((119318200+(x)/2)/(x))

#pragma pack(1)

typedef struct {
  UINT8  TimingDataIndex;
  UINT16 MaxMchVal;
  UINT8  Ddr3MinMCHVal;
  UINT8  Ddr3LowSpdByte;
  UINT8  Ddr3HighSpdByte;
  UINT8  Ddr3HighSpdByteMask;
  UINT8  Ddr3FtbSpdByte;
  char   Name[6];
} STRUCT_TIMING_DATA;

typedef enum {
  MrcMsgOk = 0,
  MrcMsgDdrTypeNotSupported,
  MrcMsgModuleTypeNotSupported,
  MrcMsgDeviceWidthNotSupported,
  MrcMsgDimm0IsNotPopulated,
  MrcMsgBanksNotSupported,
  MrcMsgDDenNotSupported,
  MrcMsgDiffNoRanksBetweenSoDimms,
  MrcMsgMoreDrNotSupported,
  MrcMsgNoDimmsPresent,
  MrcMsgClXTckNotLess20Ns,
  MrcMsgNullPtr,
  MrcMsgTimingOutBoundary,
  MrcMsgMixingX8X16,
  MrcMsgDimmFreqNotSupported,
  MrcMsgNonEccDimmDetected,
  MrcMsgCasLatencyNotSupported,
  MrcMsgUnknown,
  MrcMsgNonEccSoDimmNotSupported,
  MrcBothChannelMustBePopulated,
  MrcMsgMaxError,
} DETECT_DIMMS_ERROR;

typedef enum {
  MrcDimmVoltageOk = 0,
  MrcDimmIllegalVoltageUAndS,
  MrcDimmIllegalVoltageULAndS,
  MrcDimmIllegalSpdData,
  MrcDimmMaxError
} DETECT_VOLTAGE_DIMMS_ERROR;

//
// JEDEC_SPD_BYTE4_DRAM_DENSITY
// Do not modify this enum.
// These entries must match the DDR3L SPD BYTE4 Definition
//
typedef enum {
  JEDEC_SPD_BYTE4_DRAM_DENSITY_256Mb = 0,
  JEDEC_SPD_BYTE4_DRAM_DENSITY_512Mb,
  JEDEC_SPD_BYTE4_DRAM_DENSITY_1Gb,
  JEDEC_SPD_BYTE4_DRAM_DENSITY_2Gb,
  JEDEC_SPD_BYTE4_DRAM_DENSITY_4Gb,
  JEDEC_SPD_BYTE4_DRAM_DENSITY_8Gb,
  JEDEC_SPD_BYTE4_DRAM_DENSITY_16Gb
} JEDEC_SPD_BYTE4_DRAM_DENSITY;

//
// SPD constraints in DRAM clocks
//
#define MIN_SUPPORTED_TCL     7
#define MAX_SUPPORTED_TCL     20
#define ADJUST_TCL            3
#define ADJUST_TRP            3
#define ADJUST_TRCD           3
#define ADJUST_TRAS           9
#define ADJUST_TWR            0
#define ADJUST_TRFC           15
#define ADJUST_TWTR           0
#define ADJUST_TRRD           0
#define ADJUST_TRTP           0
#define MIN_SUPPORTED_TWR     10
#define MAX_SUPPORTED_TWR     50
#define MIN_SUPPORTED_TRCD    7
#define MAX_SUPPORTED_TRCD    20
#define MIN_SUPPORTED_TRRD    6
#define MAX_SUPPORTED_TRRD    10
#define MIN_SUPPORTED_TRP     7
#define MAX_SUPPORTED_TRP     20
#define MIN_SUPPORTED_TRAS    24
#define MAX_SUPPORTED_TRAS    44
#define MIN_SUPPORTED_TRFC    128
#define MAX_SUPPORTED_TRFC    467
#define MIN_SUPPORTED_TWTR    2
#define MAX_SUPPORTED_TWTR    8
#define MIN_SUPPORTED_TRTP    6
#define MAX_SUPPORTED_TRTP    10
#define MIN_SUPPORTED_TFAW    16
#define MAX_SUPPORTED_TFAW    33
#define MIN_SUPPORTED_TCCD    4
#define MAX_SUPPORTED_TCCD    4
#define MIN_SUPPORTED_TWTP    0
#define MAX_SUPPORTED_TWTP    0xFF
#define MIN_SUPPORTED_TWCL    7
#define MAX_SUPPORTED_TWCL    18
#define MIN_SUPPORTED_TCMD    1
#define MAX_SUPPORTED_TCMD    3

///
/// DIMM Configuration Flags, used to determine the proper voltage
/// and also to enable/disable setup options
///
typedef union {
#ifndef ASM_INC
  struct {
    UINT32  Force125VDimmAt135V         : 1;  ///< [0]: Enable setup option to force 1.25V DIMM run at 1.35V
    UINT32  Force135VDimmAt150V         : 1;  ///< [1]: Enable setup option to force 1.35V DIMM run at 1.5V
    UINT32  AddMemoryBehaviour          : 1;  ///< [2]: Enable setup option for Memory Behavior
    UINT32  Detected125VDimm            : 1;  ///< [3]: 1.25V DIMM Detected
    UINT32  Detected135VDimm            : 1;  ///< [4]: 1.35V DIMM Detected
    UINT32  Detected150VDimm            : 1;  ///< [5]: 1.50V DIMM Detected
    UINT32  Detected125Or135VDimm       : 1;  ///< [6]: 1.25V/1.35V DIMM Detected
    UINT32  Detected135Or150VDimm       : 1;  ///< [7]: 1.35V/1.50V DIMM Detected
    UINT32  Detected125VDimmAt1600      : 1;  ///< [8]: 1.25V@1600 DIMM Detected
    UINT32  RunDimmVoltageAt            : 2;  ///< [10:9]: 0 = 1.25V; 1 = 1.35V; 2 = 1.5V
    UINT32  Reserved                    : 21; ///< [31:11]: Reserved
  } FLAGS;
#endif
  UINT32  Data;
} DIMM_VOLTAGE_FLAGS;

/**
  This function provides access to the relevant SPD bytes array.
  Needed by Bdat.c.

  @param[in]   MrcData                Host structure for all MRC global data.
  @param[out]  Ddr3RelevantSpdBytes   Returned pointer to the SPD byte array.
  @param[out]  NumElements            Number of elements in the SPD byte array.

  @retval None.
**/
VOID
GetDdr3RelevantSpdBytes (
  IN    MMRC_DATA   *MrcData,
  OUT   UINT8       **Ddr3RelevantSpdBytes,
  OUT   UINT32      *NumElements
);

/**
  This function detects the memory configuration using the data gotten from the SPD (Serial Presence Detect)

  @param[in,out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters

  @retval     MMRC_SUCCESS  Memory detection was successful.
  @retval     Others        Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error.
**/
MMRC_STATUS
DetectDimms (
  IN  OUT   MMRC_DATA   *MrcData
);


/**
  FindTrasTrpTrcd

  @param[in,out]  MrcData

  @retval  MMRC_STATUS
**/
MMRC_STATUS
FindTrasTrpTrcd (
  IN  OUT   MMRC_DATA     *MrcData
);

/**

  CalcDimmConfig

  @param[in,out]  MrcData
  @param[in]      Channel

  @retval  MMRC_STATUS

**/
MMRC_STATUS
CalcDimmConfig (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8                   Channel
);

/**
  This function reads SPD data and determines which slots are populated.

  @param[in,out]  MrcData   Host structure for all MRC global data.
  @param[in]      Channel   Current Channel being examined.
  @param[in]      SpdTable  Return data of SPD bytes.
  @param[in]      TableLen  Length of SPD Table.

  @retval  MMRC_SUCCESS     SPD data gotten correctly
  @retval  Others           Error trying to get SPD data
**/
MMRC_STATUS
GetSpdData (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8                   Channel,
  IN        UINT8                   *SpdTable,
  IN        UINT8                   TableLen
);

/**
  This function uses SPD data to detect the presence of unbuffered DDR DIMMs

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Memory identify was successful.
  @retval  Others           Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
IdentifyDimms (
  IN  OUT   MMRC_DATA     *MrcData
);

/**
  This function uses SPD data to determine the timings for the memory channel

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Timing comply with JEDEC spec.
  @retval  Others           Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
FindTclTacTClk (
  IN  OUT   MMRC_DATA     *MrcData
);

/**

  This function enforces the common frequency across channels

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Timing comply with JEDEC spec.
  @retval  Others           Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
EnforceCommonFreq (
  IN  OUT   MMRC_DATA     *MrcData
);

/**
  This function converts the MrcData parameters from SPD values to values that the rest of the MMRC Expects

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Default return value
**/
MMRC_STATUS
ConvertDimmParams (
  IN  OUT   MMRC_DATA     *MrcData
);

/**
  This function determines if coldboot is required

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  TRUE             Coldboot required
  @retval  FALSE            Coldboot NOT required
**/
BOOLEAN
CheckColdBootRequired (
  IN  OUT   MMRC_DATA   *MrcData
);

#pragma pack()

#endif
