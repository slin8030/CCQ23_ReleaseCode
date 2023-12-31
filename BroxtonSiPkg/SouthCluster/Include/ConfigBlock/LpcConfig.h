/** @file
  Lpc policy

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
#ifndef _LPC_CONFIG_H_
#define _LPC_CONFIG_H_

#define LPC_PREMEM_CONFIG_REVISION 1
extern EFI_GUID gLpcPreMemConfigGuid;

#pragma pack (push,1)

/**
  This structure contains the policies which are related to LPC.
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header
  /**
    Enhance the port 8xh decoding.
    Original LPC only decodes one byte of port 80h, with this enhancement LPC can decode word or dword of port 80h-83h.
    @note: this will occupy one LPC generic IO range register. While this is enabled, read from port 80h always return 0x00.
    0: Disable, <b>1: Enable</b>
  **/
  UINT32    EnhancePort8xhDecoding      :  1;
  UINT32    RsvdBits                    : 31;     ///< Reserved bits
} SC_LPC_PREMEM_CONFIG;

#pragma pack (pop)

#endif // _LPC_CONFIG_H_
