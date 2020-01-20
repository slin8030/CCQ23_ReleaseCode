/** @file
  SC General policy

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
#ifndef _SC_GENERAL_CONFIG_H_
#define _SC_GENERAL_CONFIG_H_

#define SC_GENERAL_CONFIG_REVISION 1
extern EFI_GUID gScGeneralConfigGuid;

#pragma pack (push,1)

typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header
  /**
    Subsystem Vendor ID and Subsystem ID of the SC devices.
    This fields will be ignored if the value of SubSystemVendorId and SubSystemId
    are both 0.
  **/
  UINT16    SubSystemVendorId;          ///< Default Subsystem Vendor ID of the SC devices. Default is <b>0x8086</b>
  UINT16    SubSystemId;                ///< Default Subsystem ID of the SC devices. Default is <b>0x7270</b>
  UINT16    AcpiBase;                   ///< Power management I/O base address. Default is <b>0x400</b>.
  UINT32    PmcBase;                    ///< PMC Base Address.  Default is <b>0xD1001000</b>.
  UINT32    P2sbBase;                   ///< P2SB base Address. Default is <b>0xD0000000</b>.
  /**
    This member describes whether or not the Compatibility Revision ID (CRID) feature
    of SC should be enabled. <b>0: Disable</b>; 1: Enable
  **/
  UINT32    Crid            :  1;
  UINT32    S0ixSupport     :  1;
  UINT32    ResetSelect     :  4;
  UINT32    RsvdBits0       : 26;       ///< Reserved bits
} SC_GENERAL_CONFIG;

#pragma pack (pop)

#endif // _SC_GENERAL_CONFIG_H_
