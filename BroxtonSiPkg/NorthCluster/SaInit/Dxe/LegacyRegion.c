/** @file
This code provides a private implementation of the Legacy Region protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#include "LegacyRegion.h"
#include "SaRegs.h"
#include "SaAccess.h"

EFI_HANDLE   mLegacyRegion2Handle = NULL;
EFI_LEGACY_REGION2_PROTOCOL  mLegacyRegion2 = {
  LegacyRegion2Decode,
  LegacyRegion2Lock,
  LegacyRegion2BootLock,
  LegacyRegion2Unlock,
  LegacyRegionGetInfo
};

/**
  Modify PAM registers for region specified to MODE state.

  @param[in]  This         Pointer to EFI_LEGACY_REGION_PROTOCOL instance.
  @param[in]  Start        Starting address of a memory region covered by HUNIT HMISC2 registers (C0000h - FFFFFh).
  @param[in]  Length       Memory region length.
  @param[in]  Mode         Action to perform on a PAM region: UNLOCK, LOCK or BOOTLOCK.
  @param[out] Granularity  Granularity of region in bytes.

  @retval  EFI_SUCCESS - PAM region(s) state modified as requested.
**/
EFI_STATUS
LegacyRegionManipulation (
  IN  EFI_LEGACY_REGION2_PROTOCOL  *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  IN  UINT32                       Mode,
  OUT UINT32                       *Granularity  OPTIONAL
  )
{
  ///
  /// UNLOCK = normal operation (read/write to DRAM)
  /// LOCK = reads to DRAM, writes to DMI
  /// BOOTLOCK = LOCK = reads to DRAM, writes to DMI

  /// Since there is no concept of write routing for legacy
  /// regions in CDV, we will always assume that the caller
  /// is requesting reads to be directed to DRAM.  The Mode parameter
  /// is therefore ignored.
  ///
  if ((Start < 0xF0000) && ((Start + Length - 1) >= 0xE0000)) {
    //
    //F-segment are routed to DRAM
    //
    Mmio32Or(MCH_BASE_ADDRESS, BUNIT_BMISC_MCHBAR_OFFSET, B_BMISC_RFSDRAM);
  }

  if ((Start < 0x100000) && ((Start + Length - 1) >= 0xF0000)) {
    //
    //E-segment are routed to DRAM
    //
    Mmio32Or(MCH_BASE_ADDRESS, BUNIT_BMISC_MCHBAR_OFFSET, B_BMISC_RESDRAM);
  }

  if (Granularity) {
    *Granularity = 64 * 1024;   // All regions are 64K.
  }

  return EFI_SUCCESS;
}
///
/// Module Global:
///  Since this driver will only ever produce one instance of the Private Data
///  protocol you are not required to dynamically allocate the PrivateData.
///

/**
  Enable/Disable decoding of the given region

  @param[in]  This
  @param[in]  Start       Starting address of region.
  @param[in]  Length      Length of region in bytes.
  @param[out] Granularity
  @param[in]  On          0 = Disable decoding, 1 = Enable decoding.

  @retval  EFI_SUCCESS  Decoding change affected.
**/
EFI_STATUS
EFIAPI
LegacyRegion2Decode (
  IN  EFI_LEGACY_REGION2_PROTOCOL  *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity,
  IN  BOOLEAN                      *On
  )
{
  return EFI_SUCCESS;
}

/**
  Make the indicated region read from RAM / write to ROM.


  @param[in]  This
  @param[in]  Start        Starting address of region.
  @param[in]  Length       Length of region in bytes.
  @param[out] Granularity  Granularity of region in bytes.

  @retval  EFI_SUCCESS - Region locked or made R/O.
**/
EFI_STATUS
EFIAPI
LegacyRegion2BootLock (
  IN  EFI_LEGACY_REGION2_PROTOCOL  *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity  OPTIONAL
  )
{
  return LegacyRegionManipulation(This, Start, Length, BOOTLOCK, Granularity);
}

/**
  Make the indicated region read from RAM / write to ROM.


  @param[in]  This
  @param[in]  Start        Starting address of region.
  @param[in]  Length       Length of region in bytes.
  @param[out] Granularity  Granularity of region in bytes.

  @retval  EFI_SUCCESS - Region locked or made R/O.
**/
EFI_STATUS
EFIAPI
LegacyRegion2Lock (
  IN  EFI_LEGACY_REGION2_PROTOCOL  *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity  OPTIONAL
  )

{
  return LegacyRegionManipulation(This, Start, Length, LOCK, Granularity);
}

/**
  Make the indicated region read from RAM / write to RAM.


  @param[in]  This
  @param[in]  Start        Starting address of region.
  @param[in]  Length       Length of region in bytes.
  @param[out] Granularity  Granularity of region in bytes.

  @retval  EFI_SUCCESS     Region unlocked or made R/W.
**/
EFI_STATUS
EFIAPI
LegacyRegion2Unlock (
  IN  EFI_LEGACY_REGION2_PROTOCOL  *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity  OPTIONAL
  )
{
  return LegacyRegionManipulation(This, Start, Length, UNLOCK, Granularity);
}

/**
  Get region information for the attributes of the Legacy Region.

  This function is used to discover the granularity of the attributes for the memory in the legacy
  region. Each attribute may have a different granularity and the granularity may not be the same
  for all memory ranges in the legacy region.

  @param  This[in]              Indicates the EFI_LEGACY_REGION2_PROTOCOL instance.
  @param  DescriptorCount[out]  The number of region descriptor entries returned in the Descriptor
                                buffer.
  @param  Descriptor[out]       A pointer to a pointer used to return a buffer where the legacy
                                region information is deposited. This buffer will contain a list of
                                DescriptorCount number of region descriptors.  This function will
                                provide the memory for the buffer.

  @retval EFI_SUCCESS           The information structure was returned.
  @retval EFI_UNSUPPORTED       This function is not supported.

**/
EFI_STATUS
EFIAPI
LegacyRegionGetInfo (
  IN  EFI_LEGACY_REGION2_PROTOCOL   *This,
  OUT UINT32                        *DescriptorCount,
  OUT EFI_LEGACY_REGION_DESCRIPTOR  **Descriptor
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Install Driver to produce Legacy Region protocol.


  @param[in]  ImageHandle
  @param[in]  SystemTable

  @retval  EFI_SUCCESS  Legacy Region protocol installed
  @retval  Other        No protocol installed, unload driver.
**/
EFI_STATUS
LegacyRegionInstall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  return gBS->InstallProtocolInterface (
                &mLegacyRegion2Handle,
                &gEfiLegacyRegion2ProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &mLegacyRegion2
                );
}
