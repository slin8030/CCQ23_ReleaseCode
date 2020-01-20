/** @file
  Register names for SC private chipset register

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, SC registers are denoted by "_SC_" in register names
  - Registers / bits that are different between SC generations are denoted by
    "_SC_<generation_name>_" in register/bit names.
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SC generation will be just named
    as "_SC_" without <generation_name> inserted.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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
#ifndef _SC_REGS_GPIO_H_
#define _SC_REGS_GPIO_H_

//
// GPIO Common Private Configuration Registers
//
#define R_PCR_GPIO_PADCFGLOCK_0   0x60
#define R_PCR_GPIO_PADCFGLOCKTX_0 0x64
#define R_PCR_GPIO_PADCFGLOCK_1   0x68
#define R_PCR_GPIO_PADCFGLOCKTX_1 0x6C
#define R_PCR_GPIO_PADCFGLOCK_2   0x70
#define R_PCR_GPIO_PADCFGLOCKTX_2 0x74

#define R_PCR_GPIO_SMI_STS_0      0x140
#define R_PCR_GPIO_SMI_STS_1      0x144
#define R_PCR_GPIO_SMI_STS_2      0x148
#define R_PCR_GPIO_SMI_EN_0       0x150
#define R_PCR_GPIO_SMI_EN_1       0x154
#define R_PCR_GPIO_SMI_EN_2       0x158
#define S_GPIO_GP_SMI_EN          4
#define S_GPIO_GP_SMI_STS         4
#define V_GPIO_NUM_SUPPORTED_GPIS 248
#define B_GPIO_RXINV              BIT23
#define N_GPIO_RXINV              23

#define B_GPIO_IOSSTATE           (BIT17 | BIT16 | BIT15 | BIT14) ///< IO Standby State
#define N_GPIO_IOSSTATE           14
#define B_GPIO_IOSTERM            (BIT9 | BIT8)                   ///< IO Standby Termination
#define N_GPIO_IOSTERM            8
#define N_GPIO_GP_SMI_EN          10
#define N_GPIO_GP_SMI_STS         10
#define B_GPIO_GP_SMI_STS         BIT10
#define B_GPIO_GP_SMI_EN          BIT10


#endif
