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
/// Memory Device (Type 17) data
///
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TABLE_TYPE17 SmbiosTableType17Data = {
  { EFI_SMBIOS_TYPE_MEMORY_DEVICE, sizeof (SMBIOS_TABLE_TYPE17), 0 },
  TO_BE_FILLED, ///< MemoryArrayHandle
  0xFFFE,       ///< MemoryErrorInformationHandle
  TO_BE_FILLED, ///< TotalWidth
  TO_BE_FILLED, ///< DataWidth
  TO_BE_FILLED, ///< Size
  TO_BE_FILLED, ///< FormFactor
  0,            ///< DeviceSet
  STRING_1,     ///< DeviceLocator
  STRING_2,     ///< BankLocator
  TO_BE_FILLED, ///< MemoryType
  {             ///< TypeDetail
    0,            ///< Reserved     :1;
    0,            ///< Other        :1;
    0,            ///< Unknown      :1;
    0,            ///< FastPaged    :1;
    0,            ///< StaticColumn :1;
    0,            ///< PseudoStatic :1;
    TO_BE_FILLED, ///< Rambus       :1;
    TO_BE_FILLED, ///< Synchronous  :1;
    0,            ///< Cmos         :1;
    0,            ///< Edo          :1;
    0,            ///< WindowDram   :1;
    0,            ///< CacheDram    :1;
    0,            ///< Nonvolatile  :1;
    0,            ///< Registered   :1;
    0,            ///< Unbuffered   :1;
    0,            ///< Reserved1    :1;
  },
  TO_BE_FILLED, ///< Speed
  TO_BE_FILLED, ///< Manufacturer
  TO_BE_FILLED, ///< SerialNumber
  TO_BE_FILLED, ///< AssetTag
  TO_BE_FILLED, ///< PartNumber
  TO_BE_FILLED, ///< Attributes
  TO_BE_FILLED, ///< ExtendedSize
  TO_BE_FILLED, ///< ConfiguredMemoryClockSpeed
  TO_BE_FILLED, ///< MinimumVoltage
  TO_BE_FILLED, ///< MaximumVoltage
  TO_BE_FILLED, ///< ConfiguredVoltage
};
GLOBAL_REMOVE_IF_UNREFERENCED SMBIOS_TYPE17_STRING_ARRAY SmbiosTableType17Strings = {
  TO_BE_FILLED_STRING,  ///< DeviceLocator
  TO_BE_FILLED_STRING,  ///< BankLocator
  TO_BE_FILLED_STRING,  ///< Manufacturer
  TO_BE_FILLED_STRING,  ///< SerialNumber
#ifdef MEMORY_ASSET_TAG ///< AssetTag
  MEMORY_ASSET_TAG,
#else
  "0123456789",
#endif
  TO_BE_FILLED_STRING   ///< PartNumber
};


///
/// Even SPD Addresses only as we read Words
///
const UINT8
  SpdAddress[] = { 2, 8, 116, 118, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 142, 144 };


/**
  This function installs SMBIOS Table Type 17 (Memory Device).
  This function installs one table per memory device slot, whether populated or not.

  @param[in] SmbiosProtocol     - Instance of Smbios Protocol

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
InstallSmbiosType17 (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT16                          Index;
  UINT32                          DimmMemorySizeInMB;
  UINT8                           Dimm;
  UINT8                           *SmbusBuffer;
  UINTN                           SmbusBufferSize;
  UINTN                           SmbusOffset;
  CHAR8                           *StringBuffer;
  CHAR8                           *StringBufferStart;
  UINTN                           StringBufferSize;
  CHAR8                           StringBuffer2[4];
  UINT8                           IndexCounter;
  UINTN                           IdListIndex;
  BOOLEAN                         SlotPresent;
  UINT16                          MemoryTotalWidth;
  UINT16                          MemoryDataWidth;
  UINT8                           i;
  BOOLEAN                         FoundManufacturer;
  EFI_SMBIOS_HANDLE               SmbiosHandle;
  UINTN                           StrBufferLen;

  Status = EFI_SUCCESS;

  ///
  /// StringBuffer should only use around 50 to 60 characters max.
  /// Therefore, allocate around double that, as a saftey margin
  ///
  StringBufferSize  = (sizeof (CHAR8)) * 100;
  StringBufferStart = AllocateZeroPool (StringBufferSize);
  if (StringBufferStart == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SmbusBuffer     = NULL;
  SmbusBufferSize = 0x100; ///< SPD data section is exactly 256 bytes.
  SmbusBuffer     = AllocatePool (SmbusBufferSize);
  if (SmbusBuffer == NULL) {
    FreePool (StringBufferStart);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get Memory size parameters for each rank from the chipset registers
  //
  Status = gBS->LocateProtocol (&gMemInfoProtocolGuid, NULL, (VOID **) &mMemInfoHob);

  ///
  /// Each instance of table type 17 has the same MemoryArrayHandle
  ///
  SmbiosTableType17Data.MemoryArrayHandle = mSmbiosType16Handle;

  /**
    For each socket that exists, whether it is populated or not, generate Type 17.

    The Desktop and mobile only support 2 channels * 1 slots per channel = 2 sockets totally
    So there is rule here for Desktop and mobile that there are no more 2 DIMMS totally in a system:
     - Channel A/ Slot 0 --> SpdAddressTable[0] --> DimmToDevLocator[0] --> MemInfoHobProtocol.MemInfoData.dimmSize[0]
     - Channel B/ Slot 0 --> SpdAddressTable[2] --> DimmToDevLocator[2] --> MemInfoHobProtocol.MemInfoData.dimmSize[1]
  **/
  for (Dimm = 0; Dimm < SA_MC_MAX_SOCKETS; Dimm++) {
    ///
    /// Use channel slot map to check whether the Socket is supported in this SKU, some SKU only has 2 Sockets totally
    ///
    SlotPresent = FALSE;
    if (mMemInfoHob->MemInfoData.DimmPresent[Dimm]) {
      SlotPresent = TRUE;
    }
    ///
    /// Don't create Type 17 tables for sockets that don't exist
    ///
    if (!SlotPresent) {
      continue;
    }
    ///
    /// Generate Memory Device info (Type 17)
    ///
    ZeroMem (SmbusBuffer, SmbusBufferSize);

    ///
    /// Only read the SPD data if the DIMM is populated in the slot.
    ///
    if (SlotPresent) {
      for (i = 0; i < sizeof SpdAddress; i++) {
        SmbusOffset = SpdAddress[i];
        *(UINT16 *) (SmbusBuffer + SmbusOffset) = *(UINT16 *) (mMemInfoHob->MemInfoData.DimmsSpdData[Dimm] + SmbusOffset);
      }
    }

    ///
    /// Use SPD data to generate Device Type info
    ///
    SmbiosTableType17Strings.DeviceLocator = DimmToDevLocator[Dimm];
    SmbiosTableType17Strings.BankLocator = DimmToBankLocator[Dimm];

    ///
    /// According to SMBIOS 2.7.1 Specification - Appendix A Conformance Guidelines
    /// 4.8.7:  Form Factor is not 00h (Reserved) or 02h (Unknown).
    ///
    if (SlotPresent) {
      ///
      /// Reset StringBuffer
      ///
      StringBuffer = StringBufferStart;

      ///
      /// Show name for known manufacturer or ID for unknown manufacturer
      ///
      FoundManufacturer = FALSE;

      ///
      /// Calculate index counter
      /// Clearing Bit7 as it is the Parity Bit for Byte 117
      ///
      IndexCounter = SmbusBuffer[117] & (~0x80);

      ///
      /// Convert memory manufacturer ID to string
      ///
      for (IdListIndex = 0; MemoryModuleManufactureList[IdListIndex].Index != 0xff; IdListIndex++) {
        if (MemoryModuleManufactureList[IdListIndex].Index == IndexCounter &&
            MemoryModuleManufactureList[IdListIndex].ManufactureId == SmbusBuffer[118]
            ) {
          SmbiosTableType17Strings.Manufacturer = MemoryModuleManufactureList[IdListIndex].ManufactureName;
          FoundManufacturer = TRUE;
          break;
        }
      }
      ///
      /// Use original data if no conversion information in conversion table
      ///

      StrBufferLen = StringBufferSize / sizeof (CHAR8);

      if (!(FoundManufacturer)) {
        AsciiStrCpyS (StringBuffer, StrBufferLen, "");
        for (Index = 117; Index < 119; Index++) {
          AsciiValueToString (StringBuffer2, PREFIX_ZERO, SmbusBuffer[Index], 2);
          AsciiStrCatS (StringBuffer, StrBufferLen, StringBuffer2);
        }
        SmbiosTableType17Strings.Manufacturer = StringBuffer;
        StringBuffer += AsciiStrSize (StringBuffer);
      }

      AsciiStrCpyS (StringBuffer, StrBufferLen, "");
      for (Index = 122; Index < 126; Index++) {
        AsciiValueToString (StringBuffer2, PREFIX_ZERO, SmbusBuffer[Index], 2);
        AsciiStrCatS (StringBuffer, StrBufferLen, StringBuffer2);
      }
      SmbiosTableType17Strings.SerialNumber = StringBuffer;
      StringBuffer += AsciiStrSize (StringBuffer);

      AsciiStrCpyS (StringBuffer, StrBufferLen, "");
      for (Index = 128; Index < 146; Index++) {
        AsciiSPrint (StringBuffer2, 4, "%c", SmbusBuffer[Index]);
        AsciiStrCatS (StringBuffer, StrBufferLen, StringBuffer2);
      }
      SmbiosTableType17Strings.PartNumber = StringBuffer;

      ASSERT ((StringBuffer + AsciiStrSize (StringBuffer)) < (StringBufferStart + StringBufferSize));

      SmbiosTableType17Data.Manufacturer = STRING_3;
      SmbiosTableType17Data.SerialNumber = STRING_4;
      SmbiosTableType17Data.AssetTag     = STRING_5;
      SmbiosTableType17Data.PartNumber   = STRING_6;

      ///
      /// Get the Memory TotalWidth and DataWidth info for DDR3
      /// refer to DDR3 SPD 1.0 spec, Byte 8: Module Memory Bus Width
      /// SPD Offset 8 Bits [2:0] DataWidth aka Primary Bus Width
      /// SPD Offset 8 Bits [4:3] Bus Width extension for ECC
      ///
      MemoryDataWidth = 8 * (1 << mMemInfoHob->MemInfoData.BusWidth);
      MemoryTotalWidth = MemoryDataWidth;
      SmbiosTableType17Data.TotalWidth = MemoryTotalWidth;
      SmbiosTableType17Data.DataWidth = MemoryDataWidth;

      DimmMemorySizeInMB = mMemInfoHob->MemInfoData.dimmSize[Dimm];
      ///
      /// See the DMTF SMBIOS Specification 2.7.1, section 7.18.4
      /// regarding use of the ExtendedSize field.
      ///
      if (DimmMemorySizeInMB < SMBIOS_TYPE17_USE_EXTENDED_SIZE) {
        SmbiosTableType17Data.Size = (UINT16) DimmMemorySizeInMB;
        SmbiosTableType17Data.ExtendedSize = 0;
      } else {
        SmbiosTableType17Data.Size = SMBIOS_TYPE17_USE_EXTENDED_SIZE;
        SmbiosTableType17Data.ExtendedSize = DimmMemorySizeInMB;
      }

      switch (SmbusBuffer[DDR_MTYPE_SPD_OFFSET] & DDR_MTYPE_SPD_MASK) {
        case DDR_MTYPE_SODIMM:
          SmbiosTableType17Data.FormFactor = MemoryFormFactorSodimm;
          break;

        case DDR_MTYPE_RDIMM:
        case DDR_MTYPE_MINI_RDIMM:
          SmbiosTableType17Data.FormFactor = MemoryFormFactorRimm;
          break;

        case DDR_MTYPE_UDIMM:
        case DDR_MTYPE_MICRO_DIMM:
        case DDR_MTYPE_MINI_UDIMM:
        default:
          SmbiosTableType17Data.FormFactor = MemoryFormFactorDimm;
      }

      //
      // Memory Type
      //
      switch  (mMemInfoHob->MemInfoData.ddrType) {
        case DDRType_DDR3:
        case DDRType_DDR3L:
        case DDRType_DDR3U:
        case DDRType_LPDDR3:
            SmbiosTableType17Data.MemoryType = MemoryTypeDdr3;
            break;
        default:
            SmbiosTableType17Data.MemoryType = 0x1E;
            break;
      }

      if (SmbiosTableType17Data.FormFactor == MemoryFormFactorRimm) {
        SmbiosTableType17Data.TypeDetail.Rambus = 1;
      } else {
        SmbiosTableType17Data.TypeDetail.Rambus = 0;
      }
      SmbiosTableType17Data.TypeDetail.Synchronous  = 1;

      //
      // Memory Freq
      //
      switch (mMemInfoHob->MemInfoData.ddrFreq){
          case FREQ_800:
             SmbiosTableType17Data.Speed = 800;
              break;
          case FREQ_1066:
              SmbiosTableType17Data.Speed = 1066;
              break;
          case FREQ_1333:
              SmbiosTableType17Data.Speed = 1333;
              break;
          case FREQ_1600:
              SmbiosTableType17Data.Speed = 1600;
              break;
          case FREQ_1866:
              SmbiosTableType17Data.Speed = 1866;
              break;
          case FREQ_2133:
              SmbiosTableType17Data.Speed = 2133;
              break;
//[-start-170109-IB07400830-add]//
          case FREQ_2400:
              SmbiosTableType17Data.Speed = 2400;
              break;
//[-end-170109-IB07400830-add]//
          case FREQ_2666:
              SmbiosTableType17Data.Speed = 2666;
              break;
          case FREQ_3200:
              SmbiosTableType17Data.Speed = 3200;
              break;
          default:
              SmbiosTableType17Data.Speed = 0;
              break;
      }
      SmbiosTableType17Data.ConfiguredMemoryClockSpeed = SmbiosTableType17Data.Speed;
    } else {
      ///
      /// Memory is not Populated in this slot.
      ///
      SmbiosTableType17Strings.DeviceLocator       = DimmToDevLocator[Dimm];
      SmbiosTableType17Strings.BankLocator         = DimmToBankLocator[Dimm];

      SmbiosTableType17Strings.Manufacturer        = NULL;
      SmbiosTableType17Strings.SerialNumber        = NULL;
      SmbiosTableType17Strings.PartNumber          = NULL;

      SmbiosTableType17Data.Manufacturer           = NO_STRING_AVAILABLE;
      SmbiosTableType17Data.SerialNumber           = NO_STRING_AVAILABLE;
      SmbiosTableType17Data.AssetTag               = STRING_3;
      SmbiosTableType17Data.PartNumber             = NO_STRING_AVAILABLE;

      SmbiosTableType17Data.TotalWidth             = 0;
      SmbiosTableType17Data.DataWidth              = 0;
      SmbiosTableType17Data.Size                   = 0;
      SmbiosTableType17Data.FormFactor             = MemoryFormFactorDimm;
      SmbiosTableType17Data.MemoryType             = MemoryTypeUnknown;
      SmbiosTableType17Data.TypeDetail.Rambus      = 0;
      SmbiosTableType17Data.TypeDetail.Synchronous = 0;
      SmbiosTableType17Data.Speed                  = 0;
      SmbiosTableType17Data.Attributes             = 0;
      SmbiosTableType17Data.ExtendedSize           = 0;
    }

    ///
    /// Generate Memory Device info (Type 17)
    ///
    Status = AddSmbiosEntry (
               (EFI_SMBIOS_TABLE_HEADER *) &SmbiosTableType17Data,
               (CHAR8 **) &SmbiosTableType17Strings,
               SMBIOS_TYPE17_NUMBER_OF_STRINGS,
               &SmbiosHandle);
    if (EFI_ERROR (Status)) {
      goto CleanAndExit;
    }
  }
CleanAndExit:
  FreePool (SmbusBuffer);
  FreePool (StringBufferStart);
  DEBUG ((DEBUG_INFO, "\nInstall SMBIOS Table Type 17"));
  return Status;
}
