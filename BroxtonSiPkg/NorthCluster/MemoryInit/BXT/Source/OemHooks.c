/** @file
  This file include all the OEM hooks for MRC.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#include "OemHooks.h"
#include "IchRegs.h"

#include "ConfigMem.h"
#include "Mailbox.h"

/**
  This function detects the platform under test.

  @param[in,out]  MrcData  Pointer to MRC Output Data that contains MRC Parameters

  @retval  MMRC_SUCCESS    The platform was detected.
**/
MMRC_STATUS
OemGetPlatformType (
  IN  OUT   MMRC_DATA   *MrcData
  )
{

  UINT8   Channel;


  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform = P_SIL;

    if (MrcData->PlatformID != 0) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform = P_EMU;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration = C_PO;
      MrcData->BootMode = EM;
      MrcData->RestorePath = FALSE;
    } else {
      if (MrcData->BxtSeries == SERIES_BXT) {
        MrcData->DynamicVars[Channel][BXT_A0_WA] = 1;
      }
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform = P_SIL;
    }
#if defined BXT_PO && (BXT_PO == 1)
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration = C_PO;
#else
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration = C_POR;
#endif

  }

#if SIM
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform = P_SIM;
    MrcData->BootMode = RS;
    MrcData->DynamicVars[Channel][BXT_A0_WA] = 0;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration = C_PO;
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "Simulation Platform\n"));
#endif

#if JTAG
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {

    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform = P_SIL;
    if (MrcData->CpuStepping == STEPPING_A0) {
      MrcData->BootMode = S5;//A0;
      MrcData->DynamicVars[Channel][BXT_A0_WA] = 1;
    }
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration = C_PO;
    MrcData->PlatformID = 0;
    MrcData->RestorePath = FALSE;

  }
  // MmrcDebugPrint((MMRC_DBG_MIN, "Simulation Platform\n"));
#endif
  return MMRC_SUCCESS;
}

/**
  This function sets the OEM specific information

  @param[in,out]  MrcData  Pointer to MRC Output Data that contains MRC Parameters

  @retval  MMRC_SUCCESS    OEM information set correctly
  @retval  Others          Otherwise
**/
MMRC_STATUS
OemSetMrcData (
  IN  OUT   MMRC_DATA   *MrcData
)
{

  MRC_OEM_FRAME *OemMrcData;
#ifndef SIM
#if  1//DETECT_DIMMS
  //UINT8         SpdAddress;
  UINT8         Channel;
  UINT8         Dimm;
#endif
#endif
  OemMrcData  = &MrcData->OemMrcData;

#if SMM_SUPPORT
  OemMrcData->TsegSize = 0x4;
#endif

  OemMrcData->MmioSize = 0x400;

#if  1//DETECT_DIMMS
#if SIM
  OemMrcData->SpdAddressTable[0][0] = 0xA0;
  //OemMrcData->SpdAddressTable[0][1] = 1;
  OemMrcData->SpdAddressTable[1][0] = 0xA4;
  //OemMrcData->SpdAddressTable[1][1] = 3;
#else

  for (Channel = 0; Channel < 2; Channel++) {
    for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {
      OemMrcData->SpdAddressTable[Channel][Dimm] = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.SpdAddress[Channel];
    }
  }
#endif
#endif  //#if  DETECT_DIMMS
/*
  OemMrcData->PhyVrefTable.LinearToPhysicalTable  = (UINT8 *) LinearToPhysicalVrefCodesIsl90727;
  OemMrcData->PhyVrefTable.SizeOfLtoPTable        = sizeof (LinearToPhysicalVrefCodesIsl90727) / sizeof (UINT8);
  OemMrcData->PhyVrefTable.PhysicalToLinearTable  = (UINT8 *) PhysicalToLinearVrefCodesIsl90727;
  OemMrcData->PhyVrefTable.SizeOfPtoLTable        = sizeof (PhysicalToLinearVrefCodesIsl90727) / sizeof (UINT8);
  */
  return MMRC_SUCCESS;
}

/**
  This function sets the OEM Error handling

  @param[in]  IndexError  Error Code. Refer to MrcDeadLoop in MmrcLibraries.c

  @retval  None
**/
VOID
OemMemoryErrorCode (
  IN        UINT8                 IndexError
)
{
  return;
}
