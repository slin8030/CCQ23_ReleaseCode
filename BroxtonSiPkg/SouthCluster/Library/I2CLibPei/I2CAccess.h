/** @file
  Macros that simplify accessing I2C device's registers.

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

#ifndef _I2C_ACCESS_H_
#define _I2C_ACCESS_H_

///
/// Memory Mapped PCI Access macros
///

#include "I2CIoLibPei.h"

#define DEFAULT_PCI_BUS_NUMBER_SC   0

#define PCI_DEVICE_NUMBER_LPC       31
#define PCI_FUNCTION_NUMBER_LPC     0

#define R_LPC_ACPI_BASE             0x40        ///< ABASE, 16bit
#define R_LPC_ACPI_BASEADR          0x400       ///< ABASE, 16bit
#define B_LPC_ACPI_BASE_EN          BIT1        ///< Enable Bit
#define B_LPC_ACPI_BASE_BAR         0x0000FF80  ///< Base Address, 128 Bytes
#define V_ACPI_PM1_TMR_MAX_VAL      0x1000000   ///< The timer is 24 bit overflow
#define B_ACPI_PM1_TMR_VAL          0xFFFFFF    ///< The timer value mask

#define R_ACPI_PM1_TMR              0x08        ///< Power Management 1 Timer
#define V_ACPI_PM1_TMR_FREQUENCY    3579545     ///< Timer Frequency


#define ScLpcPciCfg8(Register)      I2CLibPeiMmioRead8 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_LPC, 0, Register))

#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PcdGet64 (PcdPciExpressBaseAddress)+ \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#endif
