/** @file
  Prototype of Intel VT-d (Virtualization Technology for Directed I/O).

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

#ifndef _VT_D_DXE_H_
#define _VT_D_DXE_H_

//
// Include files
//
#include <PiPei.h>
#include <DmaRemappingTable.h>
#include <ScAccess.h>
#include <Uefi.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PeiDxeSmmMmPciLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Guid/EventGroup.h>
#include <CpuRegs.h>
#include <SaRegs.h>
#include <ScRegs/RegsPcu.h>
#include <PlatformBaseAddresses.h>

#define VTD_ECAP_REG            0x10
#define IR                      BIT3
#define VTD_RMRR_USB_LENGTH     0x20000

#define EFI_MSR_XAPIC_BASE      0x1B
#define R_SA_MCHBAR             0x48
//
// VT-d Engine base address.
//
#define R_SA_MCHBAR_VTD1_OFFSET  0x6C88  ///< DMA Remapping HW UNIT1 for IGD
#define R_SA_MCHBAR_VTD2_OFFSET  0x6C80  ///< DMA Remapping HW UNIT2 for all other - PEG, USB, SATA etc

/**
  Locate the VT-d ACPI tables data file and update it based on current configuration and capabilities.

  @retval EFI_SUCCESS        VT-d initialization complete
  @retval EFI_UNSUPPORTED    VT-d is disabled by policy or not supported
**/
EFI_STATUS
VtdInit (
  VOID
  );

/**
  ReadyToBoot callback routine to update DMAR
**/
VOID
UpdateDmarOnReadyToBoot (
  BOOLEAN VtEnable
  );

#endif
