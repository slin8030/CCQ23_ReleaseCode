/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/**@file
  Defines data structure that is the volume header found.
  These data is intent to decouple FVB driver with FV header.
  
Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#include <PiDxe.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Guid/SystemNvDataGuid.h>

//[-start-160504-IB07220073-modify]//
#define FIRMWARE_BLOCK_SIZE         0x1000
#define FVB_MEDIA_BLOCK_SIZE        FIRMWARE_BLOCK_SIZE
//[-end-160504-IB07220073-modify]//

#define FV_RECOVERY_BASE_ADDRESS    FixedPcdGet32(PcdFlashFvRecoveryBase)
#define RECOVERY_BIOS_BLOCK_NUM     (FixedPcdGet32(PcdFlashFvRecoverySize) / FVB_MEDIA_BLOCK_SIZE)

#define FV_MAIN_BASE_ADDRESS        FixedPcdGet32(PcdFlashFvMainBase)
#define MAIN_BIOS_BLOCK_NUM         (FixedPcdGet32(PcdFlashFvMainSize) / FVB_MEDIA_BLOCK_SIZE)

#define NV_STORAGE_BASE_ADDRESS     0xFFF60000
//[-start-160504-IB07220073-modify]//
#define SYSTEM_NV_BLOCK_NUM         ((FixedPcdGet32(PcdFlashNvStorageVariableSize) + FixedPcdGet32(PcdFlashNvStorageFtwWorkingSize) + FixedPcdGet32(PcdFlashNvStorageFtwSpareSize) + FixedPcdGet32(PcdFlashNvStorageFactoryCopySize)) / FVB_MEDIA_BLOCK_SIZE)
//[-end-160504-IB07220073-modify]//




typedef struct {
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  EFI_FIRMWARE_VOLUME_HEADER  FvbInfo;
  //
  //EFI_FV_BLOCK_MAP_ENTRY    ExtraBlockMap[n];//n=0
  //
  EFI_FV_BLOCK_MAP_ENTRY      End[1];
} EFI_FVB2_MEDIA_INFO;

//
// This data structure contains a template of all correct FV headers, which is used to restore
// Fv header if it's corrupted.
//
//[-start-160504-IB07220073-modify]//
EFI_FVB2_MEDIA_INFO mPlatformFvbMediaInfo[] = {
  //
  // Main BIOS FVB
  //
//   {
//     FV_MAIN_BASE_ADDRESS,
//     {
//       {0,}, //ZeroVector[16]
//       EFI_FIRMWARE_FILE_SYSTEM2_GUID,
//       FVB_MEDIA_BLOCK_SIZE * MAIN_BIOS_BLOCK_NUM,
//       EFI_FVH_SIGNATURE,
//       0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
//       sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
//       0,    //CheckSum which will be calucated dynamically.
//       0,    //ExtHeaderOffset
//       {0,}, //Reserved[1]
//       2,    //Revision
//       {
//         {
//           MAIN_BIOS_BLOCK_NUM,
//           FVB_MEDIA_BLOCK_SIZE,
//         }
//       }
//     },
//     {
//       {
//         0,
//         0
//       }
//     }
//   },
  //
  // Systen NvStorage FVB
  //
  {
    0,
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
  },
  //
  // Recovery BIOS FVB
  //
//   {
//     FV_RECOVERY_BASE_ADDRESS,
//     {
//       {0,}, //ZeroVector[16]
//       EFI_FIRMWARE_FILE_SYSTEM2_GUID,
//       FVB_MEDIA_BLOCK_SIZE * RECOVERY_BIOS_BLOCK_NUM,
//       EFI_FVH_SIGNATURE,
//       0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
//       sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
//       0,    //CheckSum which will be calucated dynamically.
//       0,    //ExtHeaderOffset
//       {0,}, //Reserved[1]
//       2,    //Revision
//       {
//         {
//           RECOVERY_BIOS_BLOCK_NUM,
//           FVB_MEDIA_BLOCK_SIZE,
//         }
//       }
//     }, 
//     {
//       {
//         0,
//         0
//       }
//     }
//   }
};
//[-end-160504-IB07220073-modify]//


EFI_STATUS 
GetFvbInfo (
  IN  EFI_PHYSICAL_ADDRESS         FvBaseAddress,
  OUT EFI_FIRMWARE_VOLUME_HEADER   **FvbInfo
  ) 
{
  UINTN                       Index;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;

//[-start-160504-IB07220073-add]//
  if (mPlatformFvbMediaInfo[0].BaseAddress == 0) {
    mPlatformFvbMediaInfo[0].BaseAddress = PcdGet32 (PcdFlashNvStorageVariableBase);
  }
//[-end-160504-IB07220073-add]//

//[-start-160504-IB07220073-modify]//
  for (Index = 0; Index < sizeof (mPlatformFvbMediaInfo) / sizeof (EFI_FVB2_MEDIA_INFO); Index += 1) {
//     if (CompareGuid(&mPlatformFvbMediaInfo[Index].FvbInfo.FileSystemGuid, &((EFI_FIRMWARE_VOLUME_HEADER*)(UINTN)FvBaseAddress)->FileSystemGuid)) {
    if (mPlatformFvbMediaInfo[Index].BaseAddress == FvBaseAddress) {
      FvHeader =  &mPlatformFvbMediaInfo[Index].FvbInfo;
      //
      // Update the checksum value of FV header.
      //
      FvHeader->Checksum = CalculateCheckSum16 ((UINT16 *) FvHeader, FvHeader->HeaderLength);

      *FvbInfo = FvHeader;

      //DEBUG ((EFI_D_INFO, "\nBaseAddr: 0x%lx \n", FvBaseAddress));      
      //DEBUG ((EFI_D_INFO, "FvLength: 0x%lx \n", (*FvbInfo)->FvLength));
      //DEBUG ((EFI_D_INFO, "HeaderLength: 0x%x \n", (*FvbInfo)->HeaderLength));
      //DEBUG ((EFI_D_INFO, "FvBlockMap[0].NumBlocks: 0x%x \n", (*FvbInfo)->BlockMap[0].NumBlocks));
      //DEBUG ((EFI_D_INFO, "FvBlockMap[0].BlockLength: 0x%x \n", (*FvbInfo)->BlockMap[0].Length));
      //DEBUG ((EFI_D_INFO, "FvBlockMap[1].NumBlocks: 0x%x \n",   (*FvbInfo)->BlockMap[1].NumBlocks));
      //DEBUG ((EFI_D_INFO, "FvBlockMap[1].BlockLength: 0x%x \n\n", (*FvbInfo)->BlockMap[1].Length));
      
      return EFI_SUCCESS;
    }
  }
//[-end-160504-IB07220073-modify]//
  return EFI_NOT_FOUND;
}
