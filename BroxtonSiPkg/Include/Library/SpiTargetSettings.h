/** @file
  MMIO device protocol as defined in the UEFI 2.x.x specification.

  The MMIO device protocol defines a memory mapped I/O device
  for use by the system.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

@par Specification
**/

#ifndef __SPI_TARGET_SETTINGS_H__
#define __SPI_TARGET_SETTINGS_H__

///
///  SPI TARGET SETTINGS
///
typedef struct{
  ///
  /// ID of this structure
  ///
  UINT32 WireMode;
  UINT32 ChipSelectLine;
  UINT32 ChipSelectPolarity;
  UINT32 SerialClockPolarity;
  UINT32 SerialClockPhase;
  UINT32 DataFrameSize;
  UINT32 ClockSpeed;
  UINT32 LoopbackMode;
  UINT8   BytesPerEntry;
  UINT32 Reserved;
} SPI_TARGET_SETTINGS;
//extern EFI_GUID gEfiMmioDeviceProtocolGuid;

#endif  //  __SPI_TARGET_SETTINGS_H__
