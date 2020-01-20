/** @file
  Definitions for HECI driver

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

#ifndef _HECI_CORE_H
#define _HECI_CORE_H

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/PerformanceLib.h>
#include <Library/TimerLib.h>
#include <SeCState.h>
#include <CoreBiosMsg.h>
#include <HeciRegs.h>
#include <SeCAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Library/MmPciLib.h>

#include <Library/SeCLib.h>

///
/// HECI bus function version
///
#define HBM_MINOR_VERSION 0
#define HBM_MAJOR_VERSION 1


#define STALL_1_MILLISECOND       1000
#define CSE_WAIT_TIMEOUT          50

///
/// Local/Private functions not part of EFIAPI for HECI
///



/**
  Waits for the ME to report that it is ready for communication over the HECI
    interface.

  @param[in]  None

  @retval  EFI_STATUS

**/
EFI_STATUS
WaitForSECInputReady (
  VOID
  )
;

/**
  Calculate if the circular buffer has overflowed.
    Corresponds to HECI HPS (part of) section 4.2.1

  @param[in]  ReadPointer  - Location of the read pointer.
  @param[in]  WritePointer - Location of the write pointer.

  @retval  Number of filled slots.

**/
UINT8
FilledSlots (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer
  )
;

/**
  Calculate if the circular buffer has overflowed.
    Corresponds to HECI HPS (part of) section 4.2.1

  @param[in]  ReadPointer - Value read from host/me read pointer
  @param[in]  WritePointer - Value read from host/me write pointer
  @param[in]  BufferDepth - Value read from buffer depth register

  @retval  EFI_STATUS

**/
EFI_STATUS
OverflowCB (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer,
  IN      UINT32                    BufferDepth
  )
;


/**
  SeCAlivenessRequest

  @param[in] HeciMemBar
  @param[in] Request

  @retval  EFI_STATUS

**/
EFI_STATUS
SeCAlivenessRequest (
  IN      UINTN                       *HeciMemBar,
  IN      UINT32                       Request
  );

#endif // _HECI_CORE_H
