//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c) 1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformDefinitions.h

Abstract:

  This header file provides platform specific definitions used by other modules
  for platform specific initialization.

  THIS FILE SHOULD ONLY CONTAIN #defines BECAUSE IT IS CONSUMED BY NON-C MODULES
  (ASL and VFR)

  This file should not contain addition or other operations that an ASL compiler or
  VFR compiler does not understand.

--*/

#ifndef _PLATFORM_DEFINITIONS_H_
#define _PLATFORM_DEFINITIONS_H_

//#include "Platform.h"
//#include "Hpet.h"

//
// Platform Base Address definitions
//
#ifndef PCIEX_BASE_ADDRESS
#define PCIEX_BASE_ADDRESS          EDKII_GLUE_PciExpressBaseAddress // Pci Express Configuration Space Base Address
#endif
#define PCIEX_LENGTH                PLATFORM_PCIEXPRESS_LENGTH

#define THERMAL_BASE_ADDRESS        0xFED08000

#ifndef MCH_BASE_ADDRESS
#define MCH_BASE_ADDRESS            0xFED10000  // MCH  Register Base Address
#endif

//
// GPIO Register Offsets from GBASE
// Obsoleted !!!
//
#define R_PCH_GPIO_SC_USE_SEL                     0x00  // GPIO South Usage Select [31:0]
#define R_PCH_GPIO_SC_IO_SEL                      0x04  // GPIO South Input / Output Select [31:0]
#define R_PCH_GPIO_SC_LVL                         0x08  // GPIO South Level for Input or Output [31:0]

#define R_PCH_GPIO_SC_TPE                         0x0C  // GPIO South Trigger Positive Edge Enable [31:0]
#define R_PCH_GPIO_SC_TNE                         0x10  // GPIO South Trigger Negative Edge Enable [31:0]
#define R_PCH_GPIO_SC_TS                          0x14  // GPIO South Trigger Status [31:0]

#define R_PCH_GPIO_SC_USE_SEL2                    0x20  // GPIO South Usage Select 2 [63:32]
#define R_PCH_GPIO_SC_IO_SEL2                     0x24  // GPIO South Input / Output Select 2 [63:32]
#define R_PCH_GPIO_SC_LVL2                        0x28  // GPIO South Level for Input or Output 2 [63:32]

#define R_PCH_GPIO_SC_TPE2                        0x2C  // GPIO South Trigger Positive Edge Enable 2 [63:32]
#define R_PCH_GPIO_SC_TNE2                        0x30  // GPIO South Trigger Negative Edge Enable 2 [63:32]
#define R_PCH_GPIO_SC_TS2                         0x34  // GPIO South Trigger Status 2 [63:32]

#define R_PCH_GPIO_SC_USE_SEL3                    0x40  // GPIO South Usage Select 3 [95:64]
#define R_PCH_GPIO_SC_IO_SEL3                     0x44  // GPIO South Input / Output Select 3 [95:64]
#define R_PCH_GPIO_SC_LVL3                        0x48  // GPIO South Level for Input or Output 3 [95:64]

#define R_PCH_GPIO_SC_TPE3                        0x4C  // GPIO South Trigger Positive Edge Enable 3 [95:64]
#define R_PCH_GPIO_SC_TNE3                        0x50  // GPIO South Trigger Negative Edge Enable 3 [95:64]
#define R_PCH_GPIO_SC_TS3                         0x54  // GPIO South Trigger Status 3 [95:64]

#define R_PCH_GPIO_SC_USE_SEL4                    0x60  // GPIO South Usage Select 4 [127:96]
#define R_PCH_GPIO_SC_IO_SEL4                     0x64  // GPIO South Input / Output Select 4 [127:96]
#define R_PCH_GPIO_SC_LVL4                        0x68  // GPIO South Level for Input or Output 4 [127:96]

#define R_PCH_GPIO_SC_TPE4                        0x6C  // GPIO South Trigger Positive Edge Enable 4 [127:96]
#define R_PCH_GPIO_SC_TNE4                        0x70  // GPIO South Trigger Negative Edge Enable 4 [127:96]
#define R_PCH_GPIO_SC_TS4                         0x74  // GPIO South Trigger Status 4 [127:96]

#define R_PCH_GPIO_SUS_USE_SEL                    0x80  // GPIO Suspend Use Select [31:0]
#define R_PCH_GPIO_SUS_IO_SEL                     0x84  // GPIO Suspend Input / Output Select [31:0]
#define R_PCH_GPIO_SUS_LVL                        0x88  // GPIO Suspend Level for Input or Output [31:0]

#define R_PCH_GPIO_SUS_TPE                        0x8C  // GPIO Suspend Trigger Positive Edge Enable [31:0]
#define R_PCH_GPIO_SUS_TNE                        0x90  // GPIO Suspend Trigger Negative Edge Enable [31:0]
#define R_PCH_GPIO_SUS_TS                         0x94  // GPIO Suspend Trigger Status [31:0]

#define R_PCH_GPIO_SUS_WAKE_EN                    0x98  // GPIO Suspend Wake Enable [31:0]

#define R_PCH_GPIO_SUS_USE_SEL2                   0x100 // GPIO Suspend Use Select 2 [42:32]
#define R_PCH_GPIO_SUS_IO_SEL2                    0x104 // GPIO Suspend Input / Output Select 2 [42:32]
#define R_PCH_GPIO_SUS_LVL2                       0x108 // GPIO Suspend Level for Input or Output 2 [42:32]

#define R_PCH_GPIO_SUS_TPE2                       0x10C // GPIO Suspend Trigger Positive Edge Enable [42:32]
#define R_PCH_GPIO_SUS_TNE2                       0x110 // GPIO Suspend Trigger Negative Edge Enable [42:32]
#define R_PCH_GPIO_SUS_TS2                        0x114 // GPIO Suspend Trigger Status [42:32]

#define R_PCH_GPIO_SUS_WAKE_EN2                   0x118 // GPIO Suspend Wake Enable 2 [42:32]

#endif
