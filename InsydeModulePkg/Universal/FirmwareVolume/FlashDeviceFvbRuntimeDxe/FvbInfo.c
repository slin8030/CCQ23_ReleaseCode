/** @file
  Defines data structure that is the volume header found. These data is
  intented to decouple FVB driver with FV header.

;******************************************************************************
;* Copyright (c) 2013 -2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiDxe.h>

//
// The protocols, PPI and GUID defintions for this module
//
#include <Guid/EventGroup.h>
#include <Guid/FirmwareFileSystem2.h>
#include <Guid/SystemNvDataGuid.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Protocol/DevicePath.h>
//
// The Library classes this module consumes
//
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/FlashRegionLib.h>
#include <Guid/IrsiFeature.h>
#include <Protocol/FvRegionInfo.h>


FV_REGION_INFO mFvRegionInfo[] =
{
  //
  // FV region information
  //
  {
    IRSI_BIOS_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  },
  //
  // The end of FVB Image Info list must be with a NULL GUID
  //
  {
    IRSI_NULL_IMAGE_GUID,
    0,
    0
  }
};


//
// Flash Device FVB Information
//
FVB_MEDIA_INFO  mPlatformFvbMediaInfo = {

    0,
    mFvRegionInfo,
    {
      {
        0,
      },        // ZeroVector[16]
      EFI_FIRMWARE_FILE_SYSTEM2_GUID,
      0,
      EFI_FVH_SIGNATURE,
      EFI_FVB2_MEMORY_MAPPED |  
        EFI_FVB2_READ_ENABLED_CAP |
        EFI_FVB2_READ_STATUS |
        EFI_FVB2_WRITE_ENABLED_CAP |
        EFI_FVB2_WRITE_STATUS |
        EFI_FVB2_ERASE_POLARITY |
        EFI_FVB2_ALIGNMENT_16,
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
        0,      // CheckSum
        0,      // ExtHeaderOffset
      {
        0,
      },        // Reserved[1]
        2,      // Revision
      {
        0,
        FixedPcdGet32 (PcdFirmwareBlockSize),
      }
    },
    {
      0,
      0
    }
};

UINT32 mFvRegionInfoTableCount = sizeof(mFvRegionInfo)/sizeof(FV_REGION_INFO);

FVB_MEDIA_INFO *
EFIAPI
GetFvbMediaInfo (
  VOID
  )
{
  return &mPlatformFvbMediaInfo;
}


