/** @file
  This protocol provides the memory information data, such as
  total physical memory size, memory frequency, memory size
  of each dimm and rank.

  This protocol is EFI compatible.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _MEM_INFO_PROTOCOL_H_
#define _MEM_INFO_PROTOCOL_H_

//
// Define the  protocol GUID
//
#define MEM_INFO_PROTOCOL_GUID \
  { \
    0x6f20f7c8, 0xe5ef, 0x4f21, 0x8d, 0x19, 0xed, 0xc5, 0xf0, 0xc4, 0x96, 0xae \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                   gMemInfoProtocolGuid;

//
// Forward reference for ANSI C compatibility
//
typedef struct _MEM_INFO_PROTOCOL MEM_INFO_PROTOCOL;

//
// Protocol definitions
//

#define CH_NUM    4
#define DIMM_NUM  1
#define RANK_NUM  2
#define MAX_SOCKETS  (CH_NUM * DIMM_NUM)

/**
  memSize         Total physical memory size
  ddrFreq         DDR Frequency
  EccSupport      ECC Support
  dimmSize        Dimm Size
  DimmExist       Dimm Present or not
  RankInDimm      No. of ranks in a dimm
**/
#pragma pack(1)
typedef struct {
  UINT32  memSize;
  UINT16   ddrFreq;
  UINT16   ddrType;
  BOOLEAN EccSupport;
  UINT32  dimmSize[CH_NUM * DIMM_NUM];
  UINT8   reserved;
  UINT16   reserved2;
  UINT8   DimmPresent[CH_NUM * DIMM_NUM];
  UINT8   *DimmsSpdData[CH_NUM * DIMM_NUM];
  UINT8   BusWidth;
} MEMORY_INFO_DATA;
#pragma pack()

///
/// Memory Information Protocol
///
struct _MEM_INFO_PROTOCOL {
  MEMORY_INFO_DATA  MemInfoData;
};

#endif
