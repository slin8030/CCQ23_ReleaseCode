/** @file
  Definitions for S3 Resume.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#ifndef _CPU_S3_
#define _CPU_S3_

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/SynchronizationLib.h>
#include <Library/TimerLib.h>
#include <Library/HobLib.h>
#include <Library/BootGuardLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/CpuCommonLib.h>
#include <Ppi/SmmAccess.h>
#include <Ppi/MasterBootMode.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/CpuPolicy.h>
#include "CpuAccess.h"

#define VacantFlag          0x00
#define NotVacantFlag       0xff

#define MICROSECOND         10

#define MAXIMUM_CPU_NUMBER  0x40
#define STACK_SIZE_PER_PROC 0x8000

#define MAXIMUM_CPU_S3_TABLE_SIZE     0x1000

#define IO_APIC_INDEX_REGISTER        0xFEC00000
#define IO_APIC_DATA_REGISTER         0xFEC00010
extern UINTN FixedMtrrNumber;
extern UINTN MtrrDefTypeNumber;
extern UINTN VariableMtrrNumber;

typedef struct {
  UINT16 Index;
  UINT64 Value;
} EFI_MTRR_VALUES;

typedef struct {
  UINT32 ApicId;
  UINT32 MsrIndex;
  UINT64 MsrValue;
} MP_CPU_S3_SCRIPT_DATA;

typedef struct {
  UINT32 S3BootScriptTable;
  UINT32 S3BspMtrrTable;
  UINT32 VirtualWireMode;
} MP_CPU_S3_DATA_POINTER;

typedef struct {
  UINT32                Lock;
  UINT32                StackStart;
  UINT32                StackSize;
  UINT32                ApFunction;
  IA32_DESCRIPTOR       GdtrProfile;
  IA32_DESCRIPTOR       IdtrProfile;
  UINT32                BufferStart;
  UINT32                PmodeOffset;
  UINT32                AcpiCpuDataAddress;
  UINT32                MtrrValuesAddress;
  UINT32                FinishedCount;
  UINT32                WakeupCount;
  UINT32                SerializeLock;
  MP_CPU_S3_SCRIPT_DATA *S3BootScriptTable;
  UINT32                StartState;
  UINT32                VirtualWireMode;
  VOID (*SemaphoreCheck)(
          UINT32 *SemaphoreAddress
          );
  UINT32                      McuLoadCount;
  IN CONST EFI_PEI_SERVICES   **PeiServices;
  UINT64                      CpuPerfCtrlValue;
  SI_CPU_POLICY_PPI           *SiCpuPolicyPpi;
} MP_CPU_EXCHANGE_INFO;

/**
  Get protected mode code offset

  @retval Offset of protected mode code
**/
VOID *
S3AsmGetPmodeOffset (
  VOID
  );

/**
  Get code offset of SemaphoreCheck

  @retval Offset of SemaphoreCheck
**/
UINT32
S3AsmGetSemaphoreCheckOffset (
  VOID
  );

/**
  Read MTRR settings

  @param[in] MtrrValues  - buffer to store MTRR settings
**/
VOID
ReadMtrrRegisters (
  UINT64              *MtrrValues
  );

/**
  Syncup MTRR settings between all processors

  @param[in] MtrrValues  - buffer to store MTRR settings
**/
VOID
MpMtrrSynchUp (
  UINT64 *MtrrValues
  );

/**
  Set MTRR registers

  @param[in] MtrrArray   - buffer with MTRR settings
**/
VOID
SetMtrrRegisters (
  IN EFI_MTRR_VALUES  *MtrrArray
  );

#ifdef EFI_DEBUG
/**
  Print MTRR settings in debug build BIOS

  @param[in] MtrrArray   - buffer with MTRR settings
**/
VOID
ShowMtrrRegisters (
  IN EFI_MTRR_VALUES  *MtrrArray
  );
#endif

/**
  This will check if the microcode address is valid for this processor, and if so, it will
  load it to the processor.

  @param[in]  ExchangeInfo     - Pointer to the exchange info buffer for output.
  @param[in]  MicrocodeAddress - The address of the microcode update binary (in memory).
  @param[out] FailedRevision   - The microcode revision that fails to be loaded.

  @retval EFI_SUCCESS           - A new microcode update is loaded.
  @retval Other                 - Due to some reason, no new microcode update is loaded.
**/
EFI_STATUS
InitializeMicrocode (
  IN  MP_CPU_EXCHANGE_INFO *ExchangeInfo,
  IN  CPU_MICROCODE_HEADER *MicrocodeAddress,
  OUT UINT32               *FailedRevision
  );

///
/// Functions shared in MP/HT drivers
///
/**
  Send interrupt to CPU

  @param[in] BroadcastMode - interrupt broadcast mode
  @param[in] ApicID        - APIC ID for sending interrupt
  @param[in] VectorNumber  - Vector number
  @param[in] DeliveryMode  - Interrupt delivery mode
  @param[in] TriggerMode   - Interrupt trigger mode
  @param[in] Assert        - Interrupt pin polarity

  @retval EFI_INVALID_PARAMETER - input parameter not correct
  @retval EFI_NOT_READY         - there was a pending interrupt
  @retval EFI_SUCCESS           - interrupt sent successfully
**/
EFI_STATUS
SendInterrupt (
  IN UINT32           BroadcastMode,
  IN UINT32           ApicID,
  IN UINT32           VectorNumber,
  IN UINT32           DeliveryMode,
  IN UINT32           TriggerMode,
  IN BOOLEAN          Assert
  );

/**
  Programs Local APIC registers for virtual wire mode.

  @param[in] BSP             - Is this BSP?
**/
VOID
ProgramXApic (
  BOOLEAN BSP
  );

/**
  Get address map of S3RendezvousFunnelProc.

  @retval AddressMap  - Output buffer for address map information
**/
VOID *
S3AsmGetAddressMap (
  VOID
  );

#endif
