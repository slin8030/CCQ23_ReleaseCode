/**@file

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/
//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c) 1999 - 2015, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SscRegs.h

Abstract:



--*/

#ifndef _SSC_REG_H_
#define _SSC_REG_H_

// selection ID of the SSC selection table
typedef enum {
  No_SSC=0,
  M01_SSC,
  M02_SSC,
  M03_SSC,
  M04_SSC,
  M05_SSC
}SSC_SELECTION_TABLE;

// selection ID of the Clock bending selection table
typedef enum {
  Clk_Bending_13=0,
  Clk_Bending_06,
  No_Clk_Bending,
  Clk_Bending_M09
}CLK_BENDING_SELECTION_TABLE;

#define SSC_DEFAULT_SETTING          M05_SSC
#define CLK_BENDING_DEFAULT_SETTING  No_Clk_Bending

typedef struct _SSC_SETTING {
  UINT8 Ssc_Selection_ID;
  UINT32 Ssc_Cyc_To_Peak;
  UINT32 Ffs_Frac_Step;
} SSC_SETTING;

typedef struct _CLOCK_BENDING_SETTING {
  UINT8 Clk_Bending_Selection_ID;
  UINT32 Pll_Ratio_Frac;
  UINT32 Pll_Ratio_Int;
} CLOCK_BENDING_SETTING;

//default value of the LJ1PLL control registers (IPC command 0xE8)
#define LJ1PLL_CR_RW_CONTROL_1_DEFAULT  0x00
#define LJ1PLL_CR_RW_CONTROL_2_DEFAULT  0x0888812B      //downspread, ssc_frac_step: 0x8888, ssc_cyc_to_peak_m1: 0x12B
#define LJ1PLL_CR_RW_CONTROL_3_DEFAULT  0x7D000000      //ljpll_fb_int: 0x7D, others are 0
#define LJ1PLL_CR_RW_CONTROL_5_DEFAULT  0x7D000000      //pll_ratio_int: 0x7D, pll_ratio_frac: 0x00

#define SSC_ENABLE         0x1
#define SSC_DISABLE        0x0

#define SSC_MODE_DOWNSPREAD     0x0
#define SSC_MODE_UPSPREAD       0x1
#define SSC_MODE_CENTERSPREAD_DOWNFIRST   0x2
#define SSC_MODE_CENTERSPREAD_UPFIRST     0x3

typedef union _LJ1PLL_CR_RW_CONTROL_1 {
  UINT32  Data;
  struct {
    UINT32  ssc_en_ovr : 1;        ///< [0] SSC enable override
    UINT32  ssc_en : 1;          ///< [1] SSC Enable
    UINT32  ssc_b_cycles : 5;         ///< [6:2] ssc_b_cycles
    UINT32  ssc_b_step : 10;         ///< [16:7] ssc_b_step
    UINT32  ssc_a_cycles : 5;         ///< [21:17] ssc_a_cycles
    UINT32  ssc_a_step : 10;         ///< [31:22] ssc_a_step
  } Fields;
} LJ1PLL_CR_RW_CONTROL_1;

typedef union _LJ1PLL_CR_RW_CONTROL_2 {
  UINT32  Data;
  struct {
    UINT32  ssc_cyc_to_peak_m1 : 9;        ///< [8:0] cycle to peak
    UINT32  ssc_mode : 2;          ///< [10:9] SSC Mode
    UINT32  spare : 1;         ///< [11] Spare
    UINT32  ssc_frac_step : 20;         ///< [31:12] fractional step configuration
  } Fields;
} LJ1PLL_CR_RW_CONTROL_2;

typedef union _LJ1PLL_CR_RW_CONTROL_5 {
  UINT32  Data;
  struct {
    UINT32  pll_ratio_frac : 24;        ///< [23:0] Clock Bending: fractional frequency multiplier
    UINT32  pll_ratio_int : 8;          ///< [31:24] Clock Bending: integer frequency multiplier
  } Fields;
} LJ1PLL_CR_RW_CONTROL_5;

//The IPC command  buffer for SSC
typedef struct _SSC_IPC_BUFFER {
  LJ1PLL_CR_RW_CONTROL_1 LJ1PLL_CTRL_1;
  LJ1PLL_CR_RW_CONTROL_2 LJ1PLL_CTRL_2;
  UINT32                 LJ1PLL_CTRL_3;
  LJ1PLL_CR_RW_CONTROL_5 LJ1PLL_CTRL_5;
} SSC_IPC_BUFFER;


#define SB_ICLK_PORT      0x99   /// ICLK VNN Port
#define R_SB_LCPLL_1      0x9910 /// LCPLL_RW_CONTROL_1 SSC Configuration for High Speed IO domains
#define R_SB_LCPLL_2      0x9914 /// LCPLL_RW_CONTROL_2 SSC Configuration for High Speed IO domains
#define R_SB_LCPLL_5      0x9920 /// LCPLL_RW_CONTROL_5 RFI freq Configuration for High Speed IO domains

//default value of the LJ1PLL control registers (IPC command 0xE8)
#define LCPLL_CR_RW_CONTROL_1_DEFAULT  0x00
#define LCPLL_CR_RW_CONTROL_2_DEFAULT  0x7D9C12B

typedef union _LCPLL_CR_RW_CONTROL_1 {
    UINT32  Data;
    struct {
        UINT32  ssc_en_ovr : 1;        /// [0] SSC enable override
        UINT32  ssc_en : 1;            /// [1] SSC Enable
        UINT32  RSVD : 30;             /// [31:2] Reserved
    } Fields;
} LCPLL_CR_RW_CONTROL_1;

typedef union _LCPLL_CR_RW_CONTROL_2 {
    UINT32  Data;
    struct {
        UINT32  ssc_cyc_to_peak_m1 : 9;     /// [8:0] cycle to peak
        UINT32  ssc_mode : 2;               /// [10:9] SSC Mode
        UINT32  spare : 1;                  /// [11] Spare
        UINT32  ssc_frac_step : 20;         /// [31:12] fractional step configuration
    } Fields;
} LCPLL_CR_RW_CONTROL_2;

typedef union _LCPLL_CR_RW_CONTROL_5 {
    UINT32  Data;
    struct {
        UINT32  pll_ratio_frac : 24;        /// [23:0] Clock Bending: fractional frequency multiplier
        UINT32  pll_ratio_int : 8;          /// [31:24] Clock Bending: integer frequency multiplier
    } Fields;
} LCPLL_CR_RW_CONTROL_5;

//The IPC command  buffer for SSC
typedef struct _SSC_LCPLL_IPC_BUFFER {
  LCPLL_CR_RW_CONTROL_1 LCPLL_CTRL_1;
  LCPLL_CR_RW_CONTROL_2 LCPLL_CTRL_2;
} SSC_LCPLL_IPC_BUFFER;

#endif
