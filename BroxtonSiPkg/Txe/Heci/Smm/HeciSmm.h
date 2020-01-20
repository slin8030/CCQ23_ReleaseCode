/** @file
  Definitions for HECI SMM driver

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

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

#ifndef _HECI_SMM_H_
#define _HECI_SMM_H_

#define SMM_HECI_FUNCTION_SEND_MESSAGE_WITH_ACK    1
#define SMM_HECI_FUNCTION_READ_MESSAGE             2
#define SMM_HECI_FUNCTION_SEND_MESSAGE             3
#define SMM_HECI_FUNCTION_GET_STATUS               8

#define SMM_HECI_TRUSTED_CHANNEL_ERROR             5

#define SMM_HECI_MESSAGE_END_OF_POST               0x100
#define SMM_HECI_MESSAGE_END_OF_SERVICES           0x101

#define PCI_CFG_GS3                 0x74
#define PCI_CFG_GS3_PANIC_SUPPORTED 0x00080000
#define PCI_CFG_GS3_PANIC           0x00000008

typedef struct {
  UINTN       Function;
  EFI_STATUS  ReturnStatus;
  UINT8       Data[1];
} SMM_HECI_COMMUNICATE_HEADER;

typedef struct {
  UINT32      Length;
  UINT32      HostAddress;
  UINT32      CSEAddress;
  UINT8       MessageData[1];
} SMM_HECI_SEND_MESSAGE_BODY;

typedef struct {
  UINT32      Length;
  UINT8       MessageData[1];
} SMM_HECI_READ_MESSAGE_BODY;

typedef struct {
  UINT32      Length;
  UINT32      RecLength;
  UINT32      HostAddress;
  UINT32      CSEAddress;
  UINT8       MessageData[1];
} SMM_HECI_SEND_MESSAGE_W_ACK_BODY;

#define MAX_HECI_BUFFER_SIZE   0x10000

//[-start-190315-IB07401091-add]//
#define  WaitForDevIdleRegCipToClear(Addr, Data)   \
  do {                                             \
    Data = Mmio32(Addr, R_HECI_DEVIDLEC);          \
    if ((Data & B_HECI_DEVIDLEC_CIP) == 0) break;  \
  }while(TRUE)
//[-end-190315-IB07401091-add]//

#endif
