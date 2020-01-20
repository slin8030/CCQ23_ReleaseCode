/** @file
  This library will determine memory configuration information from the chipset
  and memory and create SMBIOS memory structures appropriately.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
#include "SmbiosMemory.h"

///
/// Memory Array Mapped Address (Type 19) data
///
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TABLE_TYPE19 SmbiosTableType19Data = {
  { EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS, sizeof (SMBIOS_TABLE_TYPE19), 0 },
  0,            ///< StartingAddress
  TO_BE_FILLED, ///< EndingAddress
  TO_BE_FILLED, ///< MemoryArrayHandle
  TO_BE_FILLED, ///< PartitionWidth
  0,            ///< ExtendedStartingAddress
  0,            ///< ExtendedEndingAddress
};


/**
  This function installs SMBIOS Table Type 19 (Physical Memory Array).

  @retval EFI_SUCCESS          - if the data is successfully reported.
  @retval EFI_OUT_OF_RESOURCES - if not able to get resources.
**/
EFI_STATUS
InstallSmbiosType19 (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT64                          TotalMemorySizeInKB;
  UINT8                           Dimm;
  BOOLEAN                         SlotPresent;
  EFI_SMBIOS_HANDLE               SmbiosHandle;

  TotalMemorySizeInKB = 0;

  //
  // Get Memory size parameters for each rank from the chipset registers
  //
  Status = gBS->LocateProtocol (&gMemInfoProtocolGuid, NULL, (VOID **) &mMemInfoHob);

  ///
  /// Calculate the TotalMemorySizeInKB by adding the size of all populated sockets
  ///
  for (Dimm = 0; Dimm < MAX_SOCKETS; Dimm++) {
    ///
    /// Use channel slot map to check whether the Socket is supported in this SKU, some SKU only has 2 Sockets totally
    ///
    SlotPresent = FALSE;
    if (mMemInfoHob->MemInfoData.DimmPresent[Dimm]) {
      SlotPresent = TRUE;
    }
    if (SlotPresent) {
      TotalMemorySizeInKB += LShiftU64 (mMemInfoHob->MemInfoData.dimmSize[Dimm], 10);
    }
  }

  if (TotalMemorySizeInKB > SMBIOS_TYPE19_USE_EXTENDED_ADDRESSES) {
    SmbiosTableType19Data.StartingAddress = SMBIOS_TYPE19_USE_EXTENDED_ADDRESSES;
    SmbiosTableType19Data.EndingAddress = SMBIOS_TYPE19_USE_EXTENDED_ADDRESSES;
    SmbiosTableType19Data.ExtendedEndingAddress = TotalMemorySizeInKB - 1;
  } else {
    SmbiosTableType19Data.EndingAddress = (UINT32) (TotalMemorySizeInKB - 1);
  }
  SmbiosTableType19Data.MemoryArrayHandle = mSmbiosType16Handle;
  SmbiosTableType19Data.PartitionWidth = MAX_SOCKETS;

  ///
  /// Generate Memory Array Mapped Address info (TYPE 19)
  ///
  Status = AddSmbiosEntry ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTableType19Data, NULL, 0, &SmbiosHandle);
  DEBUG ((DEBUG_INFO, "\nInstall SMBIOS Table Type 19"));
  return Status;
}
