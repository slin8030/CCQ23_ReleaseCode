/** @file
  Chipset definition for SEC Devices.

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, SEC registers are denoted by "_SEC_" in register names
  - Registers / bits that are different between SEC generations are denoted by
    "_SEC_<generation_name>_" in register/bit names. e.g., "_ME_VLV_"
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a ME generation will be just named
    as "_SEC_" without <generation_name> inserted.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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

@par Specification
**/
#ifndef _SEC_CHIPSET_H_
#define _SEC_CHIPSET_H_

#include <Library/SeCChipsetLib.h>

#define SEC_BUS                     0
#define SEC_DEVICE_NUMBER           15

#define HECI_FUNCTION_NUMBER       0x00
#define HECI2_FUNCTION_NUMBER      0x01
#define HECI3_FUNCTION_NUMBER      0x02
#define SOL_FUNCTION_NUMBER        0x03


#define HECI_DEV_FCN               ((SEC_DEVICE_NUMBER) << 3 | (HECI_FUNCTION_NUMBER))
#define HECI2_DEV_FCN              ((SEC_DEVICE_NUMBER) << 3 | (HECI2_FUNCTION_NUMBER))

typedef enum {
  HECI1_DEVICE = 0,
  HECI2_DEVICE = 1,
  HECI3_DEVICE = 2
} HECI_DEVICE;

#define V_SEC_HECI_DEVICE_ID        0x1C3A
#define V_SEC_HECI2_DEVICE_ID       0x1C3B
#define V_SEC_IDER_DEVICE_ID        0x1C3C
#define V_SEC_SOL_DEVICE_ID         0x1C3D

#define R_SEC_DevID_VID        0x0
#define R_SEC_MEM_REQ          0x44
#define S_SEC_DevID_MASK       0xFFFF0000
#define S_SEC_DevID_RANGE_LO   0x5A9A       // B.Michael need to update in BWG0.3
#define S_SEC_DevID_RANGE_HI   0x5A9E       // B.Michael need to update in BWG0.3
#define R_SATT1_BRG_BA_LSB     0x00C4  // 0xcc
#define R_SATT1_CTRL           0x00C0
#define R_SATT1_SAP_SIZE       0x00C8
#define B_ENTRY_VLD        BIT0
#define R_SEC_FW_STS0               0x40
#define FPT_BAD             BIT5
#define B_SEC_MEM_REQ_VALID    BIT31
#define B_SEC_MEM_REQ_INVALID      BIT30
#define R_DID_MSG          0x60
#define S_SEC_UMA_SIZE_MASK    0xFFFF
#define R_SICR_HOST_ALIVENESS_REQ  0x214C
#define B_ALIVENESS_REQ        BIT0
#define R_HICR_HOST_ALIVENESS_RESP 0x2044
#define B_ALIVENESS_ACK        BIT0
//
// BAR values
//
#define HECI_BAR0    0x10
#define HECI_BAR1    0x14
//
// BIOS Messages
//
#define V_SEC_DID_MSG_MASK        0x10000000
#define B_SEC_EN_MSG_MASK     BIT29
#define B_HMRFPO_DIS_MASK         (BIT28+BIT29)
//
// ACK from SeC
//
#define B_SEC_DID_ACK_MASK        BIT28

#define B_SEC_BIOS_ACTION_MASK      (BIT27 + BIT26 + BIT25)

#define B_SEC_MESSAGE_TYPE_MASK     (BIT31 + BIT30 + BIT29 + BIT28)



#define R_SEC_GS_SHDW               0x48
#define R_SEC_CPU_REPLACE_STS_MASK  BIT5


#define R_SEC_HERS                  0xBC
#define B_SEC_EXTEND_REG_VALID      BIT31
#define B_SEC_EXTEND_REG_ALGORITHM  (BIT0 | BIT1 | BIT2 | BIT3)
#define V_SEC_SHA_1                 0x00
#define V_SEC_SHA_256               0x02
#define R_SEC_HER1                  0xC0
#define R_SEC_HER2                  0xC4
#define R_SEC_HER3                  0xC8
#define R_SEC_HER4                  0xCC
#define R_SEC_HER5                  0xD0
#define R_SEC_HER6                  0xD4
#define R_SEC_HER7                  0xD8
#define R_SEC_HER8                  0xDC

#define R_HECI_DEVIDLEC             0x800
#define B_HECI_DEVIDLEC_CIP         0x1
#define B_HECI_DEVIDLEC_IR          0x2
#define B_HECI_DEVIDLEC_DEVIDLE     0x4
#define B_HECI_DEVIDLEC_RR          0x8
#define B_HECI_DEVIDLEC_IRC         0x10

//
// SEC-related Chipset Definition
//
#define HeciEnable()    SeCDeviceControl (HECI1, Enabled);
#define Heci2Enable()   SeCDeviceControl (HECI2, Enabled);
#define Heci3Enable()   SeCDeviceControl (HECI3, Enabled);
#define FTPMEnable()    SeCDeviceControl (FTPM, Enabled);

#define HeciDisable()   SeCDeviceControl (HECI1, Disabled);
#define Heci2Disable()  SeCDeviceControl (HECI2, Disabled);
#define Heci3Disable()  SeCDeviceControl (HECI3, Disabled);
#define FTPMDisable()   SeCDeviceControl (FTPM, Disabled);
#define DisableAllSECDevices() \
    HeciDisable(); \
    Heci2Disable(); \
    Heci3Disable(); \
    FTPMDisable();

#endif
