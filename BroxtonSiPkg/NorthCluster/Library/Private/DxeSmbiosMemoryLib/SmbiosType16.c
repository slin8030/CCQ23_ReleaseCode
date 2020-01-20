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
/// Physical Memory Array (Type 16) data
///
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TABLE_TYPE16 SmbiosTableType16Data = {
  { EFI_SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY, sizeof (SMBIOS_TABLE_TYPE16), 0 },
  MemoryArrayLocationSystemBoard, ///< Location
  MemoryArrayUseSystemMemory,     ///< Use
  TO_BE_FILLED,                   ///< MemoryErrorCorrection
  TO_BE_FILLED,                   ///< MaximumCapacity
  0xFFFE,                         ///< MemoryErrorInformationHandle
  TO_BE_FILLED,                   ///< NumberOfMemoryDevices
  0,                              ///< ExtendedMaximumCapacity
};


/**
  This function installs SMBIOS Table Type 16 (Physical Memory Array).

  @param[in] SmbiosProtocol    - Instance of Smbios Protocol

  @retval EFI_SUCCESS          - if the data is successfully reported.
  @retval EFI_OUT_OF_RESOURCES - if not able to get resources.
**/
EFI_STATUS
InstallSmbiosType16 (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT8                           ChannelASlotMap;
  UINT8                           ChannelBSlotMap;
  UINT8                           BitIndex;
  UINT16                          MaxSockets;
  UINT8                           ChannelASlotNum;
  UINT8                           ChannelBSlotNum;

  //
  // Get Memory size parameters for each rank from the chipset registers
  //
  Status = gBS->LocateProtocol (&gMemInfoProtocolGuid, NULL, (VOID **) &mMemInfoHob);

  ///
  /// Configure the data for TYPE 16 SMBIOS Structure
  ///
  ///
  /// Create physical array and associated data for all mainboard memory
  ///

  ///
  /// Update ECC Support
  ///
 // if (mMemInfoHob->MrcData.Outputs.EccSupport) {
 //   SmbiosTableType16Data.MemoryErrorCorrection = MemoryErrorCorrectionSingleBitEcc;
 // } else {
    SmbiosTableType16Data.MemoryErrorCorrection = MemoryErrorCorrectionNone;
 // }
  ///
  /// Get the Memory DIMM info from policy protocols
  ///
  ChannelASlotMap = 0x01;
  ChannelBSlotMap = 0x01;
  ChannelASlotNum = 0;
  ChannelBSlotNum = 0;
  for (BitIndex = 0; BitIndex < 8; BitIndex++) {
    if ((ChannelASlotMap >> BitIndex) & BIT0) {
      ChannelASlotNum++;
    }

    if ((ChannelBSlotMap >> BitIndex) & BIT0) {
      ChannelBSlotNum++;
    }
  }
  MaxSockets = ChannelASlotNum + ChannelBSlotNum;
  if (mMemInfoHob->MemInfoData.ddrType == 0) {
    if ((MAX_RANK_CAPACITY_DDR4 * SA_MC_MAX_SIDES * MaxSockets) < SMBIOS_TYPE16_USE_EXTENDED_MAX_CAPACITY) {
      SmbiosTableType16Data.MaximumCapacity = MAX_RANK_CAPACITY_DDR4 * SA_MC_MAX_SIDES * MaxSockets;
    } else {
      SmbiosTableType16Data.MaximumCapacity = SMBIOS_TYPE16_USE_EXTENDED_MAX_CAPACITY;
      SmbiosTableType16Data.ExtendedMaximumCapacity = ((UINT64) MAX_RANK_CAPACITY_DDR4) * SA_MC_MAX_SIDES * MaxSockets * 1024; // Convert from KB to Byte
    }
  } else {
    if ((MAX_RANK_CAPACITY * SA_MC_MAX_SIDES * MaxSockets) < SMBIOS_TYPE16_USE_EXTENDED_MAX_CAPACITY) {
      SmbiosTableType16Data.MaximumCapacity = MAX_RANK_CAPACITY * SA_MC_MAX_SIDES * MaxSockets;
    } else {
      SmbiosTableType16Data.MaximumCapacity = SMBIOS_TYPE16_USE_EXTENDED_MAX_CAPACITY;
      SmbiosTableType16Data.ExtendedMaximumCapacity = ((UINT64) MAX_RANK_CAPACITY) * SA_MC_MAX_SIDES * MaxSockets * 1024; // Convert from KB to Byte
    }
  }
  SmbiosTableType16Data.NumberOfMemoryDevices = MaxSockets;

  ///
  /// Install SMBIOS Table Type 16
  ///
  Status = AddSmbiosEntry ((EFI_SMBIOS_TABLE_HEADER *) &SmbiosTableType16Data, NULL, 0, &mSmbiosType16Handle);
  DEBUG ((DEBUG_INFO, "\nInstall SMBIOS Table Type 16"));
  return Status;
}
