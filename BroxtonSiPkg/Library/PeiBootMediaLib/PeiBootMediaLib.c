/** @file
  PEI Boot Media Library.

@copyright
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

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BootMediaLib.h>
#include <Library/HeciMsgLib.h>     // Needed for MBP_CURRENT_BOOT_MEDIA

//[-start-160818-IB03090432-add]//
extern  EFI_GUID gEfiBootMediaHobGuid;
//[-end-160818-IB03090432-add]//

/**
  Determines the boot media that the platform firmware is booting from

  @param[out] BootMedia       The Boot Media the platform is booting from

  @retval     EFI_SUCCESS     The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetBootMediaType (
  OUT BOOT_MEDIA_TYPE   *BootMediaType
  )
{
  EFI_STATUS                Status;
  UINT8                     BootMedia;
  EFI_HOB_GUID_TYPE         *GuidHobPtr;
  MBP_CURRENT_BOOT_MEDIA    *BootMediaData;

  Status = EFI_SUCCESS;
  GuidHobPtr  = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  if (GuidHobPtr == NULL) {
    DEBUG ((DEBUG_ERROR, "BootMediaLib Error: Boot Media HOB does not exist!\n"));
    ASSERT (GuidHobPtr != NULL);
    return EFI_NOT_FOUND;
  }

  BootMediaData = (MBP_CURRENT_BOOT_MEDIA *) GET_GUID_HOB_DATA (GuidHobPtr);
  BootMedia = (UINT8) BootMediaData->PhysicalData;
  switch (BootMedia) {
    case BOOT_FROM_EMMC:
      *BootMediaType = BootMediaEmmc;
      break;
    case BOOT_FROM_UFS:
      *BootMediaType = BootMediaUfs;
      break;
    case BOOT_FROM_SPI:
      *BootMediaType = BootMediaSpi;
      break;
    default:
//[-start-160825-IB07220135-modify]//
//       Status = EFI_NOT_FOUND;
      *BootMediaType = BootMediaSpi;
//[-end-160825-IB07220135-modify]//
      break;
  }

  return EFI_SUCCESS;
}

/**
  Determines if the platform firmware is booting from SPI or not

  @retval TRUE        Platform firmware is booting from SPI
  @retval FALSE       Platform firmware is booting from a non-SPI device (eMMC, UFS, etc.)
**/
BOOLEAN
EFIAPI
BootMediaIsSpi (
  VOID
  )
{
  EFI_STATUS      Status;
  BOOT_MEDIA_TYPE BootMedia;

  Status = GetBootMediaType (&BootMedia);
  if (EFI_ERROR (Status) || BootMedia != BootMediaSpi) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Determines if the platform firmware is booting from eMMC or not

  @retval TRUE        Platform firmware is booting from eMMC
  @retval FALSE       Platform firmware is booting from a non-eMMC device (SPI, UFS, etc.)
**/
BOOLEAN
EFIAPI
BootMediaIsEmmc (
  VOID
  )
{
  EFI_STATUS      Status;
  BOOT_MEDIA_TYPE BootMedia;

  Status = GetBootMediaType (&BootMedia);
  if (EFI_ERROR (Status) || BootMedia != BootMediaEmmc) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Determines if the platform firmware is booting from UFS or not

  @retval TRUE        Platform firmware is booting from UFS
  @retval FALSE       Platform firmware is booting from a non-UFS device (eMMC, SPI, etc.)
**/
BOOLEAN
EFIAPI
BootMediaIsUfs (
  VOID
  )
{
  EFI_STATUS      Status;
  BOOT_MEDIA_TYPE BootMedia;

  Status = GetBootMediaType (&BootMedia);
  if (EFI_ERROR (Status) || BootMedia != BootMediaUfs) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Determines if the platform firmware is booting from eMMC or UFS

  @retval TRUE        Platform firmware is booting from eMMC or UFS
  @retval FALSE       Platform firmware is not booting from eMMC or UFS
**/
BOOLEAN
EFIAPI
BootMediaIsEmmcOrUfs (
  VOID
  )
{
  EFI_STATUS      Status;
  BOOT_MEDIA_TYPE BootMedia;

  Status = GetBootMediaType (&BootMedia);
  if (EFI_ERROR (Status)) {
    return FALSE;
  } else if (BootMedia == BootMediaEmmc || BootMedia == BootMediaUfs) {
    return TRUE;
  } else {
    return FALSE;
  }
}
