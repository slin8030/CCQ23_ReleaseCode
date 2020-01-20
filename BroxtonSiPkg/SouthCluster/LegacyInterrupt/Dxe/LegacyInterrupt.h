/** @file
  @brief
  This code supports a the private implementation
  of the Legacy Interrupt protocol

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

@par Specification
**/
#ifndef LEGACY_INTERRUPT_H_
#define LEGACY_INTERRUPT_H_

#include <PiDxe.h>

#include <Protocol/LegacyInterrupt.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include "ScAccess.h"
//[-start-160704-IB07400752-add]//
#include <Library/SideBandLib.h>
//[-end-160704-IB07400752-add]//

//[-start-160704-IB07400752-remove]//
//#define PIRQN           0x00  /// PIRQ Null
//#define PIRQA           R_ILB_PIRQA_ROUT
//#define PIRQB           R_ILB_PIRQB_ROUT
//#define PIRQC           R_ILB_PIRQC_ROUT
//#define PIRQD           R_ILB_PIRQD_ROUT
//#define PIRQE           R_ILB_PIRQE_ROUT
//#define PIRQF           R_ILB_PIRQF_ROUT
//#define PIRQG           R_ILB_PIRQG_ROUT
//#define PIRQH           R_ILB_PIRQH_ROUT
//
//#define R_ILB_PIRQA_ROUT           0x8
//#define R_ILB_PIRQB_ROUT           0x9
//#define R_ILB_PIRQC_ROUT           0xA
//#define R_ILB_PIRQD_ROUT           0xB
//#define R_ILB_PIRQE_ROUT           0xC
//#define R_ILB_PIRQF_ROUT           0xD
//#define R_ILB_PIRQG_ROUT           0xE
//#define R_ILB_PIRQH_ROUT           0xF
//
//#define R_LPC_ILB_BASE                        0x50  //< IBASE, 32bit
//#define B_LPC_ILB_BASE_BAR                    0xFFFFFE00 //< Base Address, 512 bytes
//#define B_LPC_ILB_BASE_PREF                   BIT3  //< Prefetchable
//#define B_LPC_ILB_BASE_ADDRNG                 BIT2  //< Address Range
//#define B_LPC_ILB_BASE_EN                     BIT1  //< Enable Bit
//#define B_LPC_ILB_BASE_MEMI                   BIT0  //< Memory Space Indication
//[-end-160704-IB07400752-remove]//


#define MAX_PIRQ_NUMBER 8

/**

  @brief
  Return the number of PIRQs supported by this chipset.

  @param[in]  This                 Pointer to LegacyInterrupt Protocol
  @param[out] NumberPirqs          The pointer which point to the max IRQ number supported by this SC.

  @retval EFI_SUCCESS             Legacy BIOS protocol installed

**/
EFI_STATUS
EFIAPI
GetNumberPirqs (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *NumberPirqs
  )
;


/**

  @brief
  Return PCI location of this device. $PIR table requires this info.

  @param[in]  This                 Protocol instance pointer.
  @param[out] Bus                  PCI Bus
  @param[out] Device               PCI Device
  @param[out] Function             PCI Function

  @retval EFI_SUCCESS             Bus/Device/Function returned

**/
EFI_STATUS
EFIAPI
GetLocation (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *Bus,
  OUT UINT8                          *Device,
  OUT UINT8                          *Function
  )
;


/**

  @brief
  Read the given PIRQ register

  @param[in]  This                 Pointer to LegacyInterrupt Protocol
  @param[in]  PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[out] PirqData             Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number

**/
EFI_STATUS
EFIAPI
ReadPirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  OUT UINT8                          *PirqData
  )
;


/**

  @brief
  Read the given PIRQ register

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[in] PirqData             Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number

**/
EFI_STATUS
EFIAPI
WritePirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  IN  UINT8                          PirqData
  )
;

#endif
