/** @file
  Header file of CPU feature control module

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
#ifndef _FEATURES_H_
#define _FEATURES_H_

#include <Private/Library/MpServiceLib.h>

///
/// Processor feature definitions.
///
#define TXT_SUPPORT        1
#define VMX_SUPPORT        (1 << 1)
#define XD_SUPPORT         (1 << 2)
#define DCA_SUPPORT        (1 << 3)
#define X2APIC_SUPPORT     (1 << 4)
#define AES_SUPPORT        (1 << 5)
#define HT_SUPPORT         (1 << 6)
#define DEBUG_SUPPORT      (1 << 7)
#define DEBUG_LOCK_SUPPORT (1 << 8)
#define PROC_TRACE_SUPPORT (1 << 9)

#define OPTION_FEATURE_RESERVED_MASK        0xFFFB00F8  ///< bits 30:16, 18, 7:3
#define OPTION_FEATURE_CONFIG_RESERVED_MASK 0xFFFFFFFC  ///< bits 2:31

#define MAX_TOPA_ENTRY_COUNT 2

typedef struct {
  UINT64   TopaEntry[MAX_TOPA_ENTRY_COUNT];
} PROC_TRACE_TOPA_TABLE;

/**
  Create feature control structure which will be used to program each feature on each core.
**/
VOID
InitializeFeaturePerSetup (
  VOID
  );

/**
  Program all processor features basing on desired settings
**/
VOID
EFIAPI
ProgramProcessorFeature (
  VOID
  );

/**
  Program CPUID Limit before booting to OS
**/
VOID
EFIAPI
ProgramCpuidLimit (
   VOID
  );

/**
  Initialize prefetcher settings

  @param[in] MlcStreamerprefecterEnabled - Enable/Disable MLC streamer prefetcher
  @param[in] MlcSpatialPrefetcherEnabled - Enable/Disable MLC spatial prefetcher
**/
VOID
InitializeProcessorsPrefetcher (
  IN UINTN MlcStreamerprefecterEnabled,
  IN UINTN MlcSpatialPrefetcherEnabled
  );

/**
  Detect each processor feature and log all supported features
**/
VOID
EFIAPI
CollectProcessorFeature (
  VOID
  );

/**
  Lock VMX/TXT feature bits on the processor.
  Set "CFG Lock" (MSR 0E2h Bit[15]
**/
VOID
LockFeatureBit (
  VOID
  );


/**
  Provide access to the CPU misc enables MSR

  @param[in] Enable  - Enable or Disable Misc Features
  @param[in] BitMask - The register bit offset of MSR MSR_IA32_MISC_ENABLE
**/
VOID
CpuMiscEnable (
  BOOLEAN Enable,
  UINT64  BitMask
  );
#endif
