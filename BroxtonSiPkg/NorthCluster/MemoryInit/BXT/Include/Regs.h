/** @file
  This header file provides common definitions just for MCH using to avoid including extra module's file.

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

#ifndef  __REGS_H__
#define  __REGS_H__

#ifndef MINIBIOS
#ifndef  SIM
#ifdef APTIO_BUILD
#include "Token.h"
#endif
#endif
#endif

#ifndef _MC_H_INCLUDED_
#define _MC_H_INCLUDED_
/**
   Extended Configuration Base Address.  Port to value enabled by calling code,
   if necessary.
   If the EC space is not enabled by code calling the MRC, then enable the
   EC space to this value via the instructions in the function header of
   LincroftMemInit (mrc.c) at the place marked '?? OEM PORTING REQUIRED ??'.
**/

#ifndef MC_BUS
#define MC_BUS                    0x00  // Bus 0
#endif
#define MC_FUN                    0x00  // Function 0

#define MC_DEV_FUN                (MC_DEV << 3)  // Device 0, function 0
//
// Message Bus Port IDs and registers needed before ECBASE is programmed.
//
#define AUNIT_PORT_ID 0x00
#define TUNIT_PORT_ID 0x02
#define BUNIT_PORT_ID 0x03
#define BUNIT_BECREG  0x27


///
/// CMOS ADDRESS
///
#define CMOS_UPPER      0x72
#define RESET_LOOP_ADDR 0x41

///
/// Copied from dunit.h but expanding bit fields for more readable use in code.
///
typedef union {
#ifndef ASM_INC
  struct {
    /** fusestat - Bits[31:0], RO, default = 0h
       Dunit/DDRIO Fuse Status: Dunit fuse bits are captured into this register and are
       available for read.     **/
    UINT32 fus_dun_ecc_dis : 1;
    UINT32 fus_dun_max_supported_memory : 3;
    UINT32 fus_dun_max_devden : 2;
    UINT32 fus_dun_dimm2_dis : 1;
    UINT32 fus_dun_rank2_dis : 1;
    UINT32 fus_dun_ooo_dis : 1;
    UINT32 fus_dun_memx8_dis : 1;
    UINT32 fus_dun_memx16_dis : 1;
    UINT32 fus_dun_bc4_dis : 1;
    UINT32 fus_dun_1n_dis : 1;
    UINT32 fus_dun_dq_scrambler_dis : 1;
    UINT32 fus_dun_rd_scrambler_key_dis : 1;
    UINT32 Reserved : 2;
    UINT32 fus_ispid_mpll_max_freq : 3;
    UINT32 fus_ispid_mpll_default_freq : 3;
    UINT32 fus_ispid_mpllrefclk_freqsel : 2;
    UINT32 fus_ispid_mpll_default_en : 1;
    UINT32 fus_iddr3_pllsfrtrim : 4;
    UINT32 fus_ispid_mpllrefclk_sel : 2;

  } Bits;
#endif /* ASM_INC */
  UINT32 Data;
} DFUSESTAT_DUNIT_BOXTYPE_STRUCT_WITH_BITS;

typedef union {
#ifndef ASM_INC
  struct {
    UINT32 NumChannelsPopulated : 4;
    UINT32 Dimm0Density : 4;
    UINT32 Dimm0NumRanks : 4;
    UINT32 Dimm1Density : 4;
    UINT32 Dimm1NumRanks : 4;
    UINT32 Rsvd : 12;
  } Bits;
#endif /* ASM_INC */
  UINT32 Data;
} DIMM_CONFIG;

typedef union {
#ifndef ASM_INC
  struct {
    UINT32 MrcFlowStatus        : 3;    ///< Define if the MRC is started, not started or finished (NV)
    UINT32 Reserved0            : 1;
    UINT32 MrcSpdFrequency      : 3;    ///< Indicates the frequency when the last SPD reset was issued (NV)
    UINT32 MrcSpdReset          : 1;    ///< Indicates if an SPD reset has been executed (NV)
    UINT32 MrcVoltage           : 3;    ///< Indicates the current voltage in VDDQ, the voltage specification complies with the mailbox standards (NV)
    UINT32 Reserved1            : 1;
    UINT32 MrcFineVoltage       : 8;    ///< Each bit indicates a change of +-5mV in VDDQ. Sign is represented by MSB (NV)
    UINT32 MrcChannel           : 2;    ///< Indicate the number of channels active 0: Invalid, 1: One Channel, 2: Two Channels ....(NV)
    UINT32 Reserved2            : 10;
  } Bits;
#endif /* ASM_INC */
  UINT32 Data;
} MRC_FLOW_FLAGS;

typedef struct {
  BOOLEAN   FrequencyResetRequired;
  BOOLEAN   VoltageResetRequired;
  BOOLEAN   ChannelResetRequired;
} MRC_RESET_FLAGS;


#endif

#endif

