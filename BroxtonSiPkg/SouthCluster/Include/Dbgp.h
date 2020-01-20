/** @file
  Inlcude file for debug port table.

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

#ifndef _DBGP_H_
#define _DBGP_H_

//
// Statements that include other files
//

#include <IndustryStandard/Acpi30.h>

//
// Ensure proper structure formats
//
#pragma pack(1)

//
// Debug Port Table definition.
//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER               Header;
  UINT8                                     InterfaceType;
  UINT8                                     Reserved_37[3];
  EFI_ACPI_3_0_GENERIC_ADDRESS_STRUCTURE    BaseAddress;
} EFI_ACPI_DEBUG_PORT_DESCRIPTION_TABLE;

#pragma pack()

//
// DBGP Revision (defined in spec)
//
#define EFI_ACPI_DEBUG_PORT_TABLE_REVISION  0x01

//
// Interface Type
//
#define EFI_ACPI_DBGP_INTERFACE_TYPE_FULL_16550                 0
#define EFI_ACPI_DBGP_INTERFACE_TYPE_16550_SUBSET_COMPATIBLE_WITH_MS_DBGP_SPEC  1


//
// DBGP Definitions
//
#define EFI_ACPI_DBGP_TABLE_REVISION  0x00000001

#define EFI_ACPI_OEM_DBGP_REVISION    0x00000000

#endif
