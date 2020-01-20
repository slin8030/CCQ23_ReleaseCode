/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
/*++
Module Name:

  SeCUma.h

Abstract:

  Framework PEIM to SeCUma

--*/
#ifndef _SEC_UMA_H_
#define _SEC_UMA_H_


#include <HeciRegs.h>
#include <SeCAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PerformanceLib.h>
#include <Library/TimerLib.h>
#include <Ppi/SeCUma.h>
#include <Ppi/Stall.h>
#include "PlatformBaseAddresses.h"
#include <ScRegs/RegsPmc.h>

#define R_MESEG_BASE                0x70  // Removed in BXT
#define B_EXCLUSION                 BIT8

//
// SEC FW communication timeout value definitions
//
#define DID_TIMEOUT_MULTIPLIER      0x1388

//
// SEC FW HOST ALIVENESS RESP timeout
//
#define HOST_ALIVENESS_RESP_TIMEOUT_MULTIPLIER  0x1388
#define MUSZV_TIMEOUT_MULTIPLIER  0x1388
#define CPURV_TIMEOUT_MULTIPLIER  0x32
#define STALL_1_MILLISECOND       1000
#define STALL_100_MICROSECONDS    100

//
// Function Prototype(s)
//
/**

  This procedure will read and return the amount of SEC UMA requested
  by SEC ROM from the HECI device.


  @param PeiServices     - General purpose services available to every PEIM.

  @return Return SEC UMA Size in KBs

**/
EFI_STATUS
SeCSendUmaSize (
  IN EFI_PEI_SERVICES **PeiServices
  )
;

/**

  This procedure will configure the SEC Host General Status register,
  indicating that DRAM Initialization is complete and SEC FW may
  begin using the allocated SEC UMA space.


  @param PeiServices     - General purpose services available to every PEIM.
  @param MrcBootMode     - MRC BootMode
  @param InitStat        - H_GS[27:24] Status
  @param SeCUmaBase      - Memory Location ** must be with in 4GB range
  @param SeCUmaSize      EDES_TODO: Add parameter description

  @return EFI_SUCCESS

**/
EFI_STATUS
SeCConfigDidReg (
  IN CONST EFI_PEI_SERVICES **PeiServices,
  MRC_BOOT_MODE_T           MrcBootMode,
  UINT8                     InitStat,
  UINT32                    SeCUmaBase,
  UINT32                    *SeCUmaSize
  )
;

/**

  This procedure will enforce the BIOS Action that was requested by SEC FW
  as part of the DRAM Init Done message.


  @param PeiServices     - General purpose services available to every PEIM.
  @param BiosAction      -  SeC requests BIOS to act

  @return Return EFI_SUCCESS

**/
EFI_STATUS
SeCTakeOwnerShip (
  )
;

/**

  This procedure will issue a Non-Power Cycle, Power Cycle, or Global Rest.


  @param PeiServices     - General purpose services available to every PEIM.
  @param ResetType       -  Type of reset to be issued.

  @return Return EFI_SUCCESS

**/
EFI_STATUS
PerformReset (
  UINT8                     ResetType
  )
;

/**

  This procedure will clear the DISB.


  @param VOID            EDES_TODO: Add parameter description

  @return Return EFI_SUCCESS

**/
EFI_STATUS
ClearDISB (
  VOID
  )
;
/**

  This procedure will check the exposure of SeC device.


  @param PeiServices     EDES_TODO: Add parameter description

  @return Return EFI_SUCCESS

**/
EFI_STATUS
isSeCExpose(
  IN EFI_PEI_SERVICES **PeiServices
  )
;
#endif
