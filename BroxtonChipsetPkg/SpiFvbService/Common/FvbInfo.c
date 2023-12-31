/**@file
  Defines data structure that is the volume header found.
  These data is intent to decouple FVB driver with FV header.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/

#include "SpiFvbServiceCommon.h"

#define FIRMWARE_BLOCK_SIZE         0x10000
#define FVB_MEDIA_BLOCK_SIZE        FIRMWARE_BLOCK_SIZE

#define SYSTEM_NV_BLOCK_NUM         ((FixedPcdGet32(PcdFlashNvStorageVariableSize) + FixedPcdGet32(PcdFlashNvStorageFtwWorkingSize) + FixedPcdGet32(PcdFlashNvStorageFtwSpareSize)) / FVB_MEDIA_BLOCK_SIZE)

typedef struct {
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  EFI_FIRMWARE_VOLUME_HEADER  FvbInfo;
  EFI_FV_BLOCK_MAP_ENTRY      End[1];
} EFI_FVB2_MEDIA_INFO;

//
// This data structure contains a template of all correct FV headers, which is used to restore
// Fv header if it's corrupted.
//
EFI_FVB2_MEDIA_INFO mPlatformFvbMediaInfo[] = {
  //
  // System NvStorage FV
  //
  {
    0, // PcdFlashNvStorageVariableBase will be set here at runtime
    {
      {0,}, //ZeroVector[16]
      EFI_SYSTEM_NV_DATA_FV_GUID,
      FVB_MEDIA_BLOCK_SIZE * SYSTEM_NV_BLOCK_NUM,
      EFI_FVH_SIGNATURE,
      0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum which will be calucated dynamically.
      0,    //ExtHeaderOffset
      {0,}, //Reserved[1]
      2,    //Revision
      {
        {
          SYSTEM_NV_BLOCK_NUM,
          FVB_MEDIA_BLOCK_SIZE,
        }
      }
    },
    {
      {
        0,
        0
      }
    }
  }
};

EFI_STATUS
GetFvbInfo (
  IN  EFI_PHYSICAL_ADDRESS         FvBaseAddress,
  OUT EFI_FIRMWARE_VOLUME_HEADER   **FvbInfo
  )
{
  UINTN                       Index;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;

  mPlatformFvbMediaInfo[0].BaseAddress = PcdGet32 (PcdFlashNvStorageVariableBase);

  for (Index = 0; Index < sizeof (mPlatformFvbMediaInfo) / sizeof (EFI_FVB2_MEDIA_INFO); Index++) {
    if (mPlatformFvbMediaInfo[Index].BaseAddress == FvBaseAddress) {
      FvHeader = &mPlatformFvbMediaInfo[Index].FvbInfo;

      //
      // Update the checksum value of FV header.
      //
      FvHeader->Checksum = CalculateCheckSum16 ( (UINT16 *) FvHeader, FvHeader->HeaderLength);

      *FvbInfo = FvHeader;

      DEBUG ((DEBUG_INFO, "BaseAddr: 0x%lx \n", FvBaseAddress));
      DEBUG ((DEBUG_INFO, "FvLength: 0x%lx \n", (*FvbInfo)->FvLength));
      DEBUG ((DEBUG_INFO, "HeaderLength: 0x%x \n", (*FvbInfo)->HeaderLength));
      DEBUG ((DEBUG_INFO, "Header Checksum: 0x%X\n", (*FvbInfo)->Checksum));
      DEBUG ((DEBUG_INFO, "FvBlockMap[0].NumBlocks: 0x%x \n", (*FvbInfo)->BlockMap[0].NumBlocks));
      DEBUG ((DEBUG_INFO, "FvBlockMap[0].BlockLength: 0x%x \n", (*FvbInfo)->BlockMap[0].Length));
      DEBUG ((DEBUG_INFO, "FvBlockMap[1].NumBlocks: 0x%x \n", (*FvbInfo)->BlockMap[1].NumBlocks));
      DEBUG ((DEBUG_INFO, "FvBlockMap[1].BlockLength: 0x%x \n\n", (*FvbInfo)->BlockMap[1].Length));

      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}
