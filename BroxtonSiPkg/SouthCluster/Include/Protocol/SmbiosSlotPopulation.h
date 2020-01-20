/** @file
    EFI SMBIOS slot structure control code.

GUID:
    {EF7BF7D6-F8FF-4a76-8247-C0D0D1CC49C0}
    0xef7bf7d6, 0xf8ff, 0x4a76, 0x82, 0x47, 0xc0, 0xd0, 0xd1, 0xcc, 0x49, 0xc0

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

@par Specification
**/

#ifndef _EFI_SMBIOS_SLOT_POPULATION_H_
#define _EFI_SMBIOS_SLOT_POPULATION_H_

//
// Slot Population Protocol GUID
//
#define EFI_SMBIOS_SLOT_POPULATION_GUID \
  { 0xef7bf7d6, 0xf8ff, 0x4a76, 0x82, 0x47, 0xc0, 0xd0, 0xd1, 0xcc, 0x49, 0xc0 }

typedef struct {
  UINT16      SmbiosSlotId;   ///< SMBIOS Slot ID
  BOOLEAN     InUse;          ///< Does the slot have a card in it
  BOOLEAN     Disabled;       ///< Should the slot information be in SMBIOS
} EFI_SMBIOS_SLOT_ENTRY;

typedef struct {
  UINT32                NumberOfEntries;
  EFI_SMBIOS_SLOT_ENTRY *SlotEntries;
} EFI_SMBIOS_SLOT_POPULATION_INFO;

extern EFI_GUID gEfiSmbiosSlotPopulationGuid;

#endif
