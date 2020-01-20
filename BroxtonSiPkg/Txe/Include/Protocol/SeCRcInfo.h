/** @file
  This file defines the SEC RC Info Protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
#ifndef _SEC_RC_INFO_H_
#define _SEC_RC_INFO_H_

//
// Define SEC RC INFO protocol GUID
//
#define EFI_SEC_RC_INFO_PROTOCOL_GUID \
  { \
    0x11fbfdfb, 0x10d2, 0x43e6, 0xb5, 0xb1, 0xb4, 0x38, 0x6e, 0xdc, 0xcb, 0x9a \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                       gEfiSeCRcInfoProtocolGuid;

//
// Revision 1:  Original version
//
#define SEC_RC_INFO_PROTOCOL_REVISION_1  1
#define SEC_RC_VERSION                   0x00050000

typedef union _RC_VERSION {
  UINT32  Data;
  struct {
    UINT32  RcBuildNo : 8;
    UINT32  RcRevision : 8;
    UINT32  RcMinorVersion : 8;
    UINT32  RcMajorVersion : 8;
  } Fields;
} RC_VERSION;

///
/// This interface defines the SEC RC Info Protocol.
///
typedef struct _EFI_SEC_RC_INFO_PROTOCOL {
  UINT8       Revision;
  RC_VERSION  RCVersion;
} EFI_SEC_RC_INFO_PROTOCOL;
#endif
