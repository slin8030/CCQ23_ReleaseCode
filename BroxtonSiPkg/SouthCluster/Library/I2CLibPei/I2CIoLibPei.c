/** @file
  IO instance for I2C Pei Library.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>


/**
  Reads an 8-bit MMIO register.

  Reads the 8-bit MMIO register specified by Address. The 8-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 8-bit MMIO register operations are not supported, then ASSERT().

  @param[in] Address   - The MMIO register to read.

  @retval UINT8        - The UINT8 value read.
**/
UINT8
EFIAPI
I2CLibPeiMmioRead8 (
  IN      UINTN                     Address
  )
{
  UINT8                             Value;

  Value = *(volatile UINT8*)Address;
  return Value;
}

/**
  Reads a 16-bit MMIO register.

  Reads the 16-bit MMIO register specified by Address. The 16-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param[in] Address   - The MMIO register to read.

  @retval UINT16       - The UINT16 value read.
**/
UINT16
EFIAPI
I2CLibPeiMmioRead16 (
  IN      UINTN                     Address
  )
{
  UINT16                            Value;

  ASSERT ((Address & 1) == 0);
  Value = *(volatile UINT16*)Address;
  return Value;
}

/**
  Writes a 16-bit MMIO register.

  Writes the 16-bit MMIO register specified by Address with the value specified
  by Value and returns Value. This function must guarantee that all MMIO read
  and write operations are serialized.

  If 16-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 16-bit boundary, then ASSERT().

  @param[in]  Address  - The MMIO register to write.
  @param[in]  Value    - The value to write to the MMIO register.

  @retval UINT16       - The UINT16 value written.
**/
UINT16
EFIAPI
I2CLibPeiMmioWrite16 (
  IN      UINTN                     Address,
  IN      UINT16                    Value
  )
{
  ASSERT ((Address & 1) == 0);
  *(volatile UINT16*)Address = Value;
  return Value;
}

/**
  Reads a 32-bit MMIO register.

  Reads the 32-bit MMIO register specified by Address. The 32-bit read value is
  returned. This function must guarantee that all MMIO read and write
  operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param[in] Address   - The MMIO register to read.

  @retval UINT32       - The UINT32 value read.

**/
UINT32
EFIAPI
I2CLibPeiMmioRead32 (
  IN      UINTN                     Address
  )
{
  UINT32                            Value;

  ASSERT ((Address & 3) == 0);
  Value = *(volatile UINT32*)Address;

  return Value;
}

/**
  Writes a 32-bit MMIO register.

  Writes the 32-bit MMIO register specified by Address with the value specified
  by Value and returns Value. This function must guarantee that all MMIO read
  and write operations are serialized.

  If 32-bit MMIO register operations are not supported, then ASSERT().
  If Address is not aligned on a 32-bit boundary, then ASSERT().

  @param[in]  Address  - The MMIO register to write.
  @param[in]  Value    - The value to write to the MMIO register.

  @retval UINT32       - The UINT32 value written.
**/
UINT32
EFIAPI
I2CLibPeiMmioWrite32 (
  IN      UINTN                     Address,
  IN      UINT32                    Value
  )
{
  ASSERT ((Address & 3) == 0);
  *(volatile UINT32*)Address = Value;
  return Value;
}

/**
  Do logical OR operation with the value read from the 32-bit  MMIO register
  and write it back to 32-bit MMIO register.

  @param[in]  Address  - The MMIO register to write.
  @param[in]  OrData   - The value to do logical OR operation with the value read from the MMIO register.

  @retval UINT32       - The final value written to the MMIO register.
**/
UINT32
EFIAPI
I2CLibPeiMmioOr32 (
  IN      UINTN                     Address,
  IN      UINT32                    OrData
  )
{
  return I2CLibPeiMmioWrite32 (Address, I2CLibPeiMmioRead32 (Address) | OrData);
}


