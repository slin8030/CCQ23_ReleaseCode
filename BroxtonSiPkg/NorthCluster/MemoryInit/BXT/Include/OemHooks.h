/** @file
  This file the include all the external OEM Hooks.

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

#ifndef _OEMHOOKS_H_
#define _OEMHOOKS_H_

#include "Mrc.h"
#include "Regs.h"
#include "MmrcProjectDefinitionsGenerated.h"

#define MAX_OEM_SPD_ADDR  125

typedef UINT8 OEM_MEMORY_DIMM_CONFIGURATION;

extern UINT8  OemSpdDataMemorySolderDown[MAX_OEM_SPD_ADDR + 1];
extern OEM_MEMORY_DIMM_CONFIGURATION  OemMemoryDimmConfiguration[MAX_CHANNELS][MAX_DIMMS];

typedef enum {
  DimmInstalled = 0,    ///< Check Spd Data thru the Smbus
  SolderDownMemory      ///< Check fixed Spd table
} OEM_MEMORY_DIMM_TYPE;

#define MTRR_PHYS_BASE_0  0200h
#define MTRR_PHYS_MASK_0  0201h
#define MTRR_PHYS_BASE_1  0202h
#define MTRR_PHYS_MASK_1  0203h
#define MTRR_PHYS_BASE_2  0204h
#define MTRR_PHYS_MASK_2  0205h
#define MTRR_PHYS_BASE_3  0206h
#define MTRR_PHYS_MASK_3  0207h
#define MTRR_PHYS_BASE_4  0208h
#define MTRR_PHYS_MASK_4  0209h
#define MTRR_PHYS_BASE_5  020Ah
#define MTRR_PHYS_MASK_5  020Bh
#define MTRR_PHYS_BASE_6  020Ch
#define MTRR_PHYS_MASK_6  020Dh
#define MTRR_PHYS_BASE_7  020Eh
#define MTRR_PHYS_MASK_7  020Fh
#define MTRR_PHYS_BASE_8  0210h
#define MTRR_PHYS_MASK_8  0211h
#define MTRR_MEMORY_TYPE_WP 05h
#define MTRR_MEMORY_TYPE_WB 06h
#define MTRR_MEMORY_TYPE_WC 01h
#define TEMP_CACHE_SIZE_ASM 01000h
#define TEMP_CACHE_SIZE_CPP 0x1000

#define SUS_IO_START_ADDRESS      0x80
#define SCORE_IO_START_ADDRESS    0x00



/**
  This function detects the platform under test.

  @param[in,out]  MrcData  Pointer to MRC Output Data that contains MRC Parameters

  @retval  MMRC_SUCCESS    The platform was detected.
**/
MMRC_STATUS
OemGetPlatformType (
  IN  OUT   MMRC_DATA   *MrcData
)
;


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
;


#endif
