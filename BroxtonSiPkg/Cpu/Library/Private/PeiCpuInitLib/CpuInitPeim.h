/** @file
  Describes the functions visible to the rest of the CpuPeim.

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

#ifndef _CPU_INIT_PEIM_H_
#define _CPU_INIT_PEIM_H_

#include <Ppi/CpuPolicy.h>
#include <CpuRegs.h>

typedef struct {
  UINT64  MsrValue;
  BOOLEAN Changed;
} MTRR_VALUE;


#if defined (__GNUC__)
#define IA32API                     _EFIAPI
#else
#define IA32API                     __cdecl
#endif


#define RESET_PORT            0x0CF9
#define CLEAR_RESET_BITS      0x0F1

/**
  Set up flags in CR4 for XMM instruction enabling
**/
VOID
XmmInit (
  VOID
  );

/**
  Install CacheInitPpi

  @retval EFI_OUT_OF_RESOURCES - failed to allocate required pool
**/
EFI_STATUS
CacheInitPpiInit (
  VOID
  );
#ifndef FSP_FLAG

/**
  Build BIST HOB

  @param[in] PeiServices       - Indirect reference to the PEI Services Table.
  @param[in] NotifyDescriptor  - Address of the notification descriptor data structure. Type
                                 EFI_PEI_NOTIFY_DESCRIPTOR is defined above.
  @param[in] Ppi               - Address of the PPI that was installed.

  @retval EFI_SUCCESS          - Hob built or not necessary
**/
EFI_STATUS
EFIAPI
BuildBistHob (
  IN CONST EFI_PEI_SERVICES          **PeiServices
  );
#endif // FSP_FLAG
#endif
