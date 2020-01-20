/** @file
  Register names for HSIO devices.

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

#ifndef _REGS_HSIO_H_
#define _REGS_HSIO_H_

#define B_SC_HSIO_ACCESS_TYPE            (BIT15 | BIT14)
#define N_SC_HSIO_ACCESS_TYPE            14
#define V_SC_HSIO_ACCESS_TYPE_BDCAST     BIT15
#define V_SC_HSIO_ACCESS_TYPE_MULCAST    (BIT15 | BIT14)
#define B_SC_HSIO_LANE_GROUP_NO          (BIT13 | BIT12 | BIT11 | BIT10 | BIT9)
#define B_SC_HSIO_FUNCTION_NO            (BIT8  | BIT7)
#define N_SC_HSIO_FUNCTION_NO            7
#define B_SC_HSIO_REG_OFFSET             (BIT6  | BIT5  | BIT4  | BIT3  | BIT2  | BIT1  | BIT0)

#define V_SC_HSIO_ACCESS_TYPE_MCAST      0x03
#define V_SC_HSIO_ACCESS_TYPE_BCAST      0x02
#define V_SC_HSIO_ACCESS_TYPE_UCAST      0x00

#define V_SC_HSIO_LANE_GROUP_NO_CMN_LANE 0x00

#define V_SC_HSIO_FUNCTION_NO_PCS        0x00
#define V_SC_HSIO_FUNCTION_NO_TX         0x01
#define V_SC_HSIO_FUNCTION_NO_RX         0x02

#define V_SC_HSIO_FUNCTION_NO_CMNDIG     0x00
#define V_SC_HSIO_FUNCTION_NO_CMNANA     0x01
#define V_SC_HSIO_FUNCTION_NO_PLL        0x02

#define R_PCS_DWORD1                     0x0004
#define R_PCS_DWORD9                     0x0024
#define R_PCS_DWORD10                    0x0028

#define R_CMN_DIG_DWORD12                0x0030
#define R_CMN_DIG_DWORD13                0x0035
#define R_CMN_DIG_DWORD15                0x003C

#define R_SC_PCR_MODPHY0_COM0_FBDIVRATIO_7_0 0x8100

///
/// xHCI SSIC Private Configuration Register
///
#define R_SC_PCR_MMP0_LANE_0_OFFSET 0x0
#define R_SC_PCR_MMP0_LANE_1_OFFSET 0x2000
#define R_SC_PCR_MMP0_LANE_2_OFFSET 0x4000
#define R_SC_PCR_MMP0_CMNREG1       0xF000
#define R_SC_PCR_MMP0_CMNREG2       0xF004
#define R_SC_PCR_MMP0_CMNREG3       0xF008
#define R_SC_PCR_MMP0_CMNREG4       0xF00C
#define R_SC_PCR_MMP0_CMNREG5       0xF010
#define R_SC_PCR_MMP0_CMNREG6       0xF014
#define R_SC_PCR_MMP0_CMNREG7       0xF018
#define R_SC_PCR_MMP0_CMNREG8       0xF01C
#define R_SC_PCR_MMP0_CMNREG9       0xF020
#define R_SC_PCR_MMP0_CMNREG10      0xF024
#define R_SC_PCR_MMP0_CMNREG11      0xF028
#define R_SC_PCR_MMP0_CMNREG12      0xF02C
#define R_SC_PCR_MMP0_CMNREG13      0xF030
#define R_SC_PCR_MMP0_CMNREG14      0xF034
#define R_SC_PCR_MMP0_CMNREG15      0xF038
#define R_SC_PCR_MMP0_CMNREG16      0xF03C
///
///DFXREG0 - DFXREG12 are in the gap of CMNREG16 and CMNREG17
///
#define R_SC_PCR_MMP0_CMNREG17      0xF074
#define R_SC_PCR_MMP0_CMNREG18      0xF078
#define R_SC_PCR_MMP0_CMNREG19      0xF07C
#define R_SC_PCR_MMP0_CMNREG20      0xF080
#define R_SC_PCR_MMP0_CMNREG21      0xF084
#define R_SC_PCR_MMP0_CMNREG22      0xF088
#define R_SC_PCR_MMP0_IMPREG5       0x1010
#define R_SC_PCR_MMP0_IMPREG8       0x101C
#define R_SC_PCR_MMP0_IMPREG16      0x103C
#define R_SC_PCR_MMP0_IMPREG21      0x1050
#define R_SC_PCR_MMP0_IMPREG22      0x1054
#define R_SC_PCR_MMP0_IMPREG23      0x1058
#define R_SC_PCR_MMP0_IMPREG25      0x1060
#define R_SC_PCR_MMP0_IMPREG27      0x1068
#define R_SC_PCR_MMP0_DFXREG0       0xF040
#define R_SC_PCR_MMP0_DFXREG1       0xF044
#define R_SC_PCR_MMP0_DFXREG2       0xF048
#define R_SC_PCR_MMP0_DFXREG3       0xF04C
#define R_SC_PCR_MMP0_DFXREG4       0xF050
#define R_SC_PCR_MMP0_DFXREG5       0xF054
#define R_SC_PCR_MMP0_DFXREG6       0xF058
#define R_SC_PCR_MMP0_DFXREG7       0xF05C
#define R_SC_PCR_MMP0_DFXREG8       0xF060
#define R_SC_PCR_MMP0_DFXREG9       0xF064
#define R_SC_PCR_MMP0_DFXREG10      0xF068
#define R_SC_PCR_MMP0_DFXREG11      0xF06C
#define R_SC_PCR_MMP0_DFXREG12      0xF070
#define R_SC_PCR_MMP0_FUSEREG1      0xF0FC
#endif

