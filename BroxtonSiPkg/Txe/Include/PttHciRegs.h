/** @file
  Register definitions for PTT HCI (Platform Trust Technology - Host Controller Interface).

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#ifndef _PTT_HCI_REGS_H_
#define _PTT_HCI_REGS_H_


///
/// FTPM HCI register base address
///
#define R_PTT_HCI_BASE_ADDRESS             0xFED40000

//
// FTPM HCI Control Area
//
#define R_PTT_HCI_CA_RSVD                  0x00
#define R_PTT_HCI_CA_ERROR                 0x04
#define R_PTT_HCI_CA_CANCEL                0x08
#define R_PTT_HCI_CA_START                 0x0C
#define R_PTT_HCI_CA_INT_RSVD              0x10
#define R_PTT_HCI_CA_CMD_SZ                0x18
#define R_PTT_HCI_CA_CMD                   0x1C
#define R_PTT_HCI_CA_RSP_SZ                0x24
#define R_PTT_HCI_CA_RSP                   0x28

//
// FTPM HCI Private Area
//
#define R_PTT_HCI_CMD                      0x40
#define R_PTT_HCI_STS                      0x44

///
/// FTPM HCI Command and Response Buffer
///
#define R_PTT_HCI_CRB                      0x80

//
// R_PTT_HCI_STS Flags
//
#define B_PTT_HCI_STS_ENABLED              0x00000001 ///< BIT0
#define B_PTT_HCI_STS_READY                0x00000002 ///< BIT1
#define B_PTT_HCI_STS_ACM_AS_CRTM          0x00000004 ///< BIT2
#define B_PTT_HCI_STS_STARTUP_EXEC         0x00000008 ///< BIT3

//
// Value written to R_PTT_HCI_CMD and CA_START
// to indicate that a command is available for processing
//
#define V_PTT_HCI_COMMAND_AVAILABLE_START  0x00000001
#define V_PTT_HCI_COMMAND_AVAILABLE_CMD    0x00000001
#define V_PTT_HCI_BUFFER_ADDRESS_RDY       0x00000003

///
/// Ignore bit setting mask for WaitRegisterBits
///
#define V_PTT_HCI_IGNORE_BITS              0x00000000

///
/// All bits clear mask for WaitRegisterBits
///
#define V_PTT_HCI_ALL_BITS_CLEAR           0xFFFFFFFF
#define V_PTT_HCI_START_CLEAR              0x00000001

///
/// Max FTPM command/reponse buffer length
///
#define S_PTT_HCI_CRB_LENGTH               3968 ///< 0xFED70080:0xFED70FFF = 3968 Bytes

#endif
