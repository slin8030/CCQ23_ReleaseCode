/** @file
  Utility MRC function.

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

#include "McFunc.h"
#include "MrcFunc.h"
#include "ConfigMem.h"
#include "OemHooks.h"
#include "MrcVersion.h"

const MrcVersion mIafwVersion = {{IAFW_VERSION_STRUCT_VALUE}};

#ifdef __GNUC__
unsigned char _BitScanForward (
  UINT32 *Index,
  unsigned long Mask
)
{
  __asm__ (
  "bsfl %0, %0;"
  :"=a"(*Index)
  :"0" (Mask)
  :
  );

  return (unsigned char)(0 != Mask);
}
#else
#pragma intrinsic(_BitScanForward)
#endif
/**
  Returns the position of the least significant bit set in the input parameter.
  If the input value is zero, the output is undefined.

  @param[in]  Input    The value to be scanned

  @retval  UINT8    bit position
**/
UINT8
BitScanForward8 (
  IN        UINT8   Input
)
{
#ifdef SIM
  UINT8 i;
  for (i = 0; (i < 8) && ((Input & (1 << i) ) == 0); i++);
  return i;
#else
  UINT32 Index;
  _BitScanForward (&Index, Input);
  return (UINT8) Index;
#endif
}

#ifdef __GNUC__
unsigned char _BitScanReverse(
  UINT32 *Index,
  unsigned long Mask
)
{
  __asm__ (
  "bsrl %0, %0;"
  :"=a"(*Index)
  :"0" (Mask)
  :
  );

  return (unsigned char)(0 != Mask);
}
#else
#pragma intrinsic(_BitScanReverse)
#endif
/**
  Returns the position of the most significant bit set in the input parameter.
  If the input value is zero, the output is undefined.

  @param[in]  Input    The value to be scanned

  @retval  UINT8    bit position
**/
INT8
BitScanReverse16 (
  IN        UINT16   Input
)
{
#ifdef SIM
  INT16 i;
  for (i = 15; (i >= 0) && ((Input & (1 << i) ) == 0); i--);
  return (INT8) i;
#else
  UINT32 Index;
  _BitScanReverse (&Index, Input);
  return (UINT8) Index;
#endif


}
/**
  FillInputStructure

  @param[in,out]  MrcData

  @retval  MMRC_STATUS
**/
MMRC_STATUS
FillInputStructure (
  IN  OUT   MMRC_DATA   *MrcData
)
{
#if DDR3_SUPPORT || DDR4_SUPPORT
  UINT8     CurrentDimm;
  SPD_DATA  *CurrentSpdData;
  UINT8     i;
  UINT8     *pData8;
  UINT8     Channel;
#endif

  //MmrcDebugPrint ((MMRC_DBG_MIN, "BootMode = %d\n", MrcData->BootMode));

#if DDR3_SUPPORT || DDR4_SUPPORT
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm]) {
          continue;
        }
        CurrentSpdData = & (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
        pData8 = (UINT8 *) (&(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FastBootData[CurrentDimm]) );
        for (i = 0; i < (sizeof (FASTBOOTDATA) - 1); i++) {
          *(pData8 + i) = CurrentSpdData->Buffer[SPD_DDR3_MANUFACTURER_ID_LO + i] ;
        }
        *(pData8 + sizeof (FASTBOOTDATA) - 1) = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount;
      } // End of for CurrentDimm
    } // if channel enabled
  } // for channel
#endif

  return MMRC_SUCCESS;
}
/**
  FillOutputStructure

  @param[in,out]  MrcData

  @retval  MMRC_STATUS
**/
MMRC_STATUS
FillOutputStructure (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  return MMRC_SUCCESS;
}

/**
  Returns the IAFW version.

  @param[in,out]      MrcData        MRC Data Structure
  @param[out]         IafwVersion    The IAFW version

**/
VOID
GetIafwVersion (
  IN  OUT   MMRC_DATA   *MrcData,
  OUT       MrcVersion  *IafwVersion
)
{
  IafwVersion->Version.Major = mIafwVersion.Version.Major;
  IafwVersion->Version.Minor = mIafwVersion.Version.Minor;
  IafwVersion->Version.Rev = mIafwVersion.Version.Rev;
  IafwVersion->Version.Build = mIafwVersion.Version.Build;
  IafwVersion->Bits.TestBuild = TEST_BIOS;
  IafwVersion->Bits.DevId     = DEV_ID;
}

/**
  Returns a UINT32 value representing the MRC and MMRC versions

  @param[in,out]      MrcData        MRC Data Structure

  @retval             MrcVersion     A UINT32 representation of the MRC Version
                                     [31:16] = Mrc Version
                                     [15:0]  = MMRC Version
**/
UINT32
GetMrcVersions (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  MrcVersion                IafwVersion;
  GetIafwVersion(MrcData, &IafwVersion);
  return (IafwVersion.Version.Major << 24) + (IafwVersion.Version.Minor << 16) + (VERSION_MAJOR << 8) + (VERSION_MINOR);
}

/**
  Calculates a CRC-32 of the specified data buffer.

  @param[in] Data     - Pointer to the data buffer.
  @param[in] DataSize - Size of the data buffer, in bytes.

  @retval The CRC-32 value.
**/
UINT32
MrcCalculateCrc32 (
  IN     const UINT8       *const Data,
  IN     const UINT32      DataSize
  )
{
  UINT32 i;
  UINT32 j;
  UINT32 crc;
  UINT32 CrcTable[256];

  crc = (UINT32) (-1);

  //
  // Initialize the CRC base table.
  //
  for (i = 0; i < 256; i++) {
    CrcTable[i] = i;
    for (j = 8; j > 0; j--) {
      CrcTable[i] = (CrcTable[i] & 1) ? (CrcTable[i] >> 1) ^ 0xEDB88320 : CrcTable[i] >> 1;
    }
  }
  //
  // Calculate the CRC.
  //
  for (i = 0; i < DataSize; i++) {
    crc = (crc >> 8) ^ CrcTable[(UINT8) crc ^ (Data)[i]];
  }

  return ~crc;
}