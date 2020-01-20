/** @file
  Header file for Cpu Common Lib implementation.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#ifndef _CPU_COMMON_LIB_H_
#define _CPU_COMMON_LIB_H_

typedef UINT32 CPU_RESET_TYPE;

#define NO_RESET                              0
#define WARM_RESET                            BIT0
#define COLD_RESET                            (BIT0 | BIT1)

#define RESET_PPI_WARM_RESET                  0
#define RESET_PPI_COLD_RESET                  3

#define POWER_LEVEL_DATA                      UINT8
#define MAILBOX_PL_WIDTH                      6
#define MAILBOX_PL_WIDTH_MASK                 0x3F

#define READ_PCH_POWER_LEVELS_CMD             0x8000000A
#define NUM_PL_SUPP_PCH_POWER_LEVELS_CMD      4
#define READ_EXT_PCH_POWER_LEVELS_CMD         0x8000000B
#define NUM_PL_SUPP_EXT_PCH_POWER_LEVELS_CMD  3

///
/// Enums for Time Window Convert Type
///
typedef enum {
  PL1TimeWindowConvert = 1,
  PL3TimeWindowConvert,
  TccOffsetTimeWindowConvert,
  TimeWindowConvertMaximum
} TIME_WINDOW_CONV;

/**
  Initialize prefetcher settings

  @param[in] MlcStreamerprefecterEnabled - Enable/Disable MLC streamer prefetcher
  @param[in] MlcSpatialPrefetcherEnabled - Enable/Disable MLC spatial prefetcher
**/
VOID
ProcessorsPrefetcherInitialization (
  IN UINTN MlcStreamerprefecterEnabled,
  IN UINTN MlcSpatialPrefetcherEnabled
  );

/**
  Set up flags in CR4 for XMM instruction enabling
**/
VOID
EFIAPI
XmmInit (
  VOID
  );

/**
  Enable "Machine Check Enable"
**/
VOID
EFIAPI
EnableMce (
  VOID
  );

/**
  Mtrr Synch Up Entry
**/
UINTN
EFIAPI
MpMtrrSynchUpEntry (
  VOID
  );

/**
  Mtrr Synch Up Exit
**/
VOID
EFIAPI
MpMtrrSynchUpExit (
  UINTN Cr4
  );

/**
  This procedure sends an IPI to the designated processor in
  the requested delivery mode with the requested vector.

  @param[in] ApicID          - APIC ID of processor.
  @param[in] VectorNumber    - Vector number.
  @param[in] DeliveryMode    - I/O APIC Interrupt Deliver Modes

  @retval EFI_INVALID_PARAMETER - Input paramters were not correct.
  @retval EFI_NOT_READY         - There was a pending interrupt
  @retval EFI_SUCCESS           - Interrupt sent successfully
**/
EFI_STATUS
EFIAPI
CpuSendIpi (
  IN UINT32            ApicID,
  IN UINTN             VectorNumber,
  IN UINTN             DeliveryMode
  );

/**
  Private helper function to convert various Turbo Power Limit Time from Seconds to CPU units

  @param[in] TimeInSeconds       Time in seconds
  @param[in] TimeWindowConvType  Time Window Convert Type

  @retval UINT8 Converted time in CPU units
**/
UINT8
GetConvertedTime (
  IN UINT32            TimeInSeconds,
  IN TIME_WINDOW_CONV  TimeWindowConvType
  );

/**
  Get APIC ID of processor

  @retval APIC ID of processor
**/
UINT32
GetCpuApicId (
  VOID
  );

/**
  Programs XAPIC registers.

  @param[in] Bsp             - Is this BSP?
**/
VOID
ProgramXApic (
  BOOLEAN Bsp
  );

/**
  This function is to disable BIOS Write Protect in SMM phase.
**/
VOID
EFIAPI
CpuSmmDisableBiosWriteProtect (
  VOID
  );

/**
  This function is to enable BIOS Write Protect in SMM phase.
**/
VOID
EFIAPI
CpuSmmEnableBiosWriteProtect (
  VOID
  );

/**
  This function returns the maximum number of cores supported in this physical processor package.

  @retval Maximum number of supported cores in the package.
**/
UINT8
GetMaxSupportedCoreCount (
  VOID
  );

/**
  This function returns the actual factory-configured number of threads per core,
  and actual factory-configured number of cores in this physical processor package.

  @param[out] NumberOfEnabledThreadsPerCore  - variable that will store Maximum enabled threads per core
  @param[out] NumberOfEnabledCoresPerDie     - variable that will store Maximum enabled cores per die
**/
VOID
GetSupportedCount (
  OUT UINT16 *ThreadsPerCore,  OPTIONAL
  OUT UINT16 *NumberOfCores    OPTIONAL
  );

/**
  This function returns the maximum enabled Core per die, maximum enabled threads per core,
  maximum number of dies and packages

  @param[out] *NumberOfEnabledThreadsPerCore  - variable that will store Maximum enabled threads per core
  @param[out] *NumberOfEnabledCoresPerDie     - variable that will store Maximum enabled cores per die
  @param[out] *NumberOfDiesPerPackage         - variable that will store Maximum dies per package
  @param[out] *NumberOfPackages               - variable that will store Maximum Packages
**/
VOID
GetEnabledCount (
  OUT UINT16 *NumberOfEnabledThreadsPerCore,
  OUT UINT16 *NumberOfEnabledCoresPerDie,
  OUT UINT16 *NumberOfDiesPerPackage,
  OUT UINT16 *NumberOfPackages
  );

/**
  Check to see if the executing thread is BSP

  @retval TRUE   Executing thread is BSP
  @retval FALSE  Executing thread is AP
**/
BOOLEAN
IsBsp (
  VOID
  );

#endif
