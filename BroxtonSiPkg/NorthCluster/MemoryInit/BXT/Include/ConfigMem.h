/** @file
  Data definition for memory Configuration.

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

#ifndef _CONFIGMEM_H_
#define _CONFIGMEM_H_

#include "Mrc.h"
#include "McFunc.h"
#include "MrcFunc.h"

#if SIM
extern FILE *gLogFile;
#endif

/**
  Indicates when memory training initialization is complete.

  @param[in,out]  MrcData   Host structure for all MRC global data.
  @param[in]      Channel   Current Channel being examined.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
SetDDRInitializationComplete (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Channel
);

/**
  Asserts a DDR reset

  @param[in,out]  MrcData   Host structure for all MRC global data.
  @param[in]      Channel   Current Channel being examined.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
PerformDDR3Reset (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Channel
);

/**
  Set Odt matrix data.

  @param[in,out]  MrcData
  @param[in]      Node
  @param[in]      Channel
  @param[in,out]  MrsValue[MAX_RANKS][MR_COUNT]

  @retval  None
**/
extern
VOID
SetOdtMatrix (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Node,
  IN        UINT8        Channel,
  IN  OUT   UINT32       MrsValue[MAX_RANKS][MR_COUNT]
);

/**
  Notify Punit that BIOS reset is complete.
  Polls the BIOS_RESET_CPL register waiting for the  PUNIT to be ready.

  @param[in,out]  MrcData

  @retval  None
**/
MMRC_STATUS
InitPunitSetting (
  IN OUT  MMRC_DATA *MrcData
);

/**
  If voltage changed, checked reset flag.

  @param[in,out]  MrcData

  @retval  None
**/
UINT32
CheckResetForVoltageChange (
  IN  OUT   MMRC_DATA   *MrcData
  )
;

/**
  Executes the SPD-Only reset when needed

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  True   If a reset is required due to a change in the frequency
  @retval  False  Otherwise
**/
BOOLEAN
ChangeFrequency (
  IN  OUT   MMRC_DATA   *MrcData
);


/**
  Initialize external silicon.

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
ExternalSiliconInit (
  IN  OUT   MMRC_DATA   *MrcData
)
;
/**
  Get input parameters.

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
GetInputParameters (
  IN  OUT   MMRC_DATA   *MrcData
)
;
/**
  Convert input parameters.

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
ConvertInputParameters (
  IN  OUT   MMRC_DATA   *MrcData
)
;
/**
  Record MRC status.

  @param[in,out]  MrcData   Host structure for all MRC global data.
  @param[in]      Finish    Indicates if this is the 1st or 2nd instance of this call

  @retval  None
**/
VOID
RecordMrcStatus (
  IN  OUT   MMRC_DATA   *MrcData,
  IN  BOOLEAN           Finish
  )
;


#if VP_BIOS_ENABLE
/**
  Set Mrc done flag for VP.

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
SetMrcDoneForVP (
  MMRC_DATA         *MrcData
);
#endif

/**
  Calculate memory map.

  @param[in,out]  MrcData           Host structure for all MRC global data.
  @param[in]      CapsuleStartIndex
  @param[in]      StringIndex
  @param[in]      Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS
ProgramMemoryMap (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
);

/**
  Print memory config data.

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  None
**/
void
PrintMemoryConfig(
  MMRC_DATA         *MrcData
);

#endif
