/** @file
  This file defines the Info Protocol.

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
#ifndef _SC_INFO_H_
#define _SC_INFO_H_


#define EFI_INFO_PROTOCOL_GUID \
  { \
    0xd31f0400, 0x7d16, 0x4316, 0xbf, 0x88, 0x60, 0x65, 0x88, 0x3b, 0x40, 0x2b \
  }
extern EFI_GUID                       gEfiInfoProtocolGuid;

///
/// Forward reference for ANSI C compatibility
///
typedef struct _EFI_INFO_PROTOCOL EFI_SC_INFO_PROTOCOL;

///
/// Protocol revision number
/// Any backwards compatible changes to this protocol will result in an update in the revision number
/// Major changes will require publication of a new protocol
///
/// Revision 1:  Original version
///
#define INFO_PROTOCOL_REVISION_1  1
#define INFO_PROTOCOL_REVISION_2  2

///
/// RCVersion[7:0] is the release number.
/// For example:
/// VlvFramework 0.6.0-01 should be 00 06 00 01 (0x00060001)
/// VlvFramework 0.6.2    should be 00 06 02 00 (0x00060200)
///
#define SC_RC_VERSION                0x01000000

/**
  This protocol is used to provide the information of South Cluster controller.
**/
struct _EFI_INFO_PROTOCOL {
  UINT8   Revision;
  UINT8   BusNumber;
  UINT32  RCVersion;
};

#endif
