/** @file
  IoApic policy

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
#ifndef _IOAPIC_CONFIG_H_
#define _IOAPIC_CONFIG_H_

#define IOAPIC_CONFIG_REVISION 1
extern EFI_GUID gIoApicConfigGuid;

#pragma pack (push,1)

/**
  This structure contains the policies which are related to IO Apic.
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;         ///< Config Block Header
  UINT32  BdfValid            :  1;     ///< Set to 1 if BDF value is valid, SC code will not program these fields if this bit is not TRUE. <b>0: Disable</b>; 1: Enable.
  UINT32  RsvdBits0           :  7;     ///< Reserved bits
  UINT32  BusNumber           :  8;     ///< Bus/Device/Function used as Requestor / Completer ID. Default is <b>0xFA</b>.
  UINT32  DeviceNumber        :  5;     ///< Bus/Device/Function used as Requestor / Completer ID. Default is <b>0x0F</b>.
  UINT32  FunctionNumber      :  3;     ///< Bus/Device/Function used as Requestor / Completer ID. Default is <b>0x00</b>.
  UINT32  IoApicEntry24_119   :  1;     ///< 0: Disable; <b>1: Enable</b> IOAPIC Entry 24-119
  UINT32  RsvdBits1           :  7;     ///< Reserved bits
  UINT8   IoApicId;                     ///< This member determines IOAPIC ID.
  UINT8   ApicRangeSelect;              ///< Define address bits 19:12 for the IOxAPIC range. Default is <b>0</b>
  UINT8   Rsvd0[2];                     ///< Reserved bytes
} SC_IOAPIC_CONFIG;

#pragma pack (pop)

#endif // _IOAPIC_CONFIG_H_
