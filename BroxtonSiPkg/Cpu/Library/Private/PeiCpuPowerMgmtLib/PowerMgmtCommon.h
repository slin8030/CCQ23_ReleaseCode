/** @file
  This header file contains the processor power management definitions.

  Acronyms:
   - PPM   Processor Power Management
   - TM    Thermal Monitor
   - IST   Intel(R) Speedstep technology
   - HT    Hyper-Threading Technology

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

#ifndef _POWER_MGMT_COMMON_H_
#define _POWER_MGMT_COMMON_H_

#include <Library/PeiServicesLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/TimerLib.h>
#include <ScAccess.h>
#include <SaAccess.h>
#include <CpuAccess.h>
#include <Ppi/CpuPolicy.h>
#include <Library/CpuPlatformLib.h>
#include <Private/PowerMgmtNvsStruct.h>
#include <Library/MmPciLib.h>
#include <Private/CpuInitDataHob.h>
#include <Private/Library/MpServiceLib.h>
#include <Library/CpuMailboxLib.h>
#include <Library/PostCodeLib.h>

#define PM_CST_LVL2                     0x14

#define FADT_C3_LATENCY                 57
#define FADT_C3_LATENCY_DISABLED        1001

#define NATIVE_PSTATE_LATENCY           10
#define PSTATE_BM_LATENCY               10

#define MP_TIMEOUT_FOR_STARTUP_ALL_APS  0 ///< Set 0 for BSP always wait for APs

#define CTDP_LEVEL_ONE                     1
#define CTDP_LEVEL_TWO                     2

///
/// Limit the number of P-states to 16. Up to Windows 7, the OS allocates 1KB buffer for the PSS package.
/// So the maximum number of P-state OS can handle is 19. This is not an OS issue. Having too many P-states
/// is not good for the system performance.
///
#define FVID_MAX_STATES                 20
#define FVID_MIN_STEP_SIZE              1

///
/// Cpu Brandstring length
///
#define MAXIMUM_CPU_BRAND_STRING_LENGTH       48

#define EFI_FIELD_OFFSET(TYPE,Field) ((UINTN)(&(((TYPE *) 0)->Field)))
#define EFI_IDIV_ROUND(r, s)  ((r) / (s) + (((2 * ((r) % (s))) < (s)) ? 0 : 1))

//
// Below function is defined in MdePkg but not published by it's .h file.
//

INT64
EFIAPI
InternalMathDivRemS64x64 (
  IN      INT64                     Dividend,
  IN      INT64                     Divisor,
  OUT     INT64                     *Remainder  OPTIONAL
  );

//
// Global variables
//
///
/// SiCpuPolicy Revision
///
extern UINT8 mCpuPolicyRevision;
///
/// Power Managment policy configurations
///
extern POWER_MGMT_CONFIG           *gPowerMgmtConfig;
extern CPU_CONFIG                  *mCpuConfig;
extern EFI_CPUID_REGISTER          mCpuid01;            ///< CPUID 01 values
//
// Values for FVID table calculate.
//
extern UINT16 mTurboBusRatio;
extern UINT16 mMaxBusRatio;
extern UINT16 mMinBusRatio;
extern UINT16 mProcessorFlavor;
extern UINT16 mBspBootRatio;
extern UINT16 mPackageTdp;
extern UINT16 mPackageTdpWatt;
extern UINT16 mCpuConfigTdpBootRatio;
extern UINT16 mCustomPowerUnit;
///
/// Fractional part of Processor Power Unit in Watts. (i.e. Unit is 1/mProcessorPowerUnit)
///
extern UINT8 mProcessorPowerUnit;
///
/// Fractional part of Processor Time Unit in seconds. (i.e Unit is 1/mProcessorTimeUnit)
///
extern UINT8 mProcessorTimeUnit;
///
/// Maximum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
/// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
///
extern UINT16 mPackageMaxPower;
///
/// Minimum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
/// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
///
extern UINT16 mPackageMinPower;
extern UINT8  mRatioLimitProgrammble;                        ///< Programmable Ratio Limit
extern UINT8  mTdpLimitProgrammble;                          ///< Programmable TDP Limit

extern CPU_GLOBAL_NVS_AREA_CONFIG *gCpuGlobalNvsAreaConfig;  ///< CPU GlobalNvs pointer

typedef struct _ENABLE_CSTATE_PARAMS {
  UINT16 C3IoAddress;
} ENABLE_CSTATE_PARAMS;

typedef struct _ENABLE_EMTTM_PARAMS {
  FVID_TABLE *FvidPointer;
} ENABLE_EMTTM_PARAMS;

typedef struct _PCODE_BCLK_CALIBRATION_MAILBOX {
  UINT32 PCalFactor;
  UINT32 TSC24_L1;
  UINT32 TSC24_L2;
  UINT32 TSC24_U1;
  UINT32 TSC24_U2;
  UINT32 TSC100_L1;
  UINT32 TSC100_L2;
  UINT32 TSC100_U1;
  UINT32 TSC100_U2;
} PCODE_BCLK_CALIBRATION_MAILBOX;

typedef struct {
  UINT16 SkuPackageTdp;
  UINTN  MsrCtdpPowerLimit1;
  UINTN  MsrCtdpPowerLimit2;
  UINTN  CtdpUpPowerLimit1;
  UINTN  CtdpUpPowerLimit2;
  UINTN  CtdpNominalPowerLimit1;
  UINTN  CtdpNominalPowerLimit2;
  UINTN  CtdpDownPowerLimit1;
  UINTN  CtdpDownPowerLimit2;
  UINTN  Reserved;
} PPM_CTDP_OVERRIDE_TABLE;

///
/// PL1 Thermal Control structure layout
///
typedef union {
  struct {
   UINT8 Disable:1;      ///< [0] Disable
   UINT8 Reserved:7;     ///< [7:2] Reserved
   UINT8 FloorIa:8;        ///< [15:8] Percent Throttle for IA component 255-0; 255=0%, 0=100%
   UINT8 FloorGt:8;        ///< [23:16] Percent Throttle for GT component 255-0; 255=0%, 0=100%
   UINT8 FloorPch:8;       ///< [31:24] Percent Throttle for PCH component 255-0; 255=0%, 0=100%
  } Bits;
  UINT32 Uint32;
 } PL1_THERMAL_CONTROL;

//
// FVID Table Information
// Default FVID table
// One header field plus states
//
extern UINT16     mNumberOfStates;
extern FVID_TABLE *mFvidPointer;

//
// Function prototypes
//
/**
  Initializes P States and Turbo Power management features
**/
VOID
InitPStates (
  VOID
  );

/**
  Initializes XE support in the processor.
**/
VOID
InitTurboRatioLimits (
  VOID
  );

/**
  Initializes required structures for P-State table creation and enables EIST
  support in the processor.

  @param[in out] FvidPointer  Table to update, must be initialized.
**/
VOID
InitEist (
  IN OUT FVID_TABLE        *FvidPointer
  );

/**
  Enables EIST support in a logical processor.

  This function must be MP safe.

  @param[in out] Buffer  Pointer to the function parameters passed in.
**/
VOID
EFIAPI
ApSafeEnableEist (
  IN OUT VOID *Buffer
  );

/**
  This function updates the table provided with the FVID data for the processor.
  If CreateDefaultTable is TRUE, a minimam FVID table will be provided.
  The maximum number of states must be greater then or equal to two.
  The table should be initialized in such a way as for the caller to determine if the
  table was updated successfully.  This function should be deprecated in the future when
  Release 8 is integrated in favor of the EIST protocol calculating FVID information.

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in] MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer
  @param[in] MinStepSize         Minimum step size for generating the FVID table
  @param[in] CreateDefaultTable  Create default FVID table rather then full state support
**/
VOID
InitFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates,
  IN UINT16         MinStepSize,
  IN BOOLEAN        CreateDefaultTable
  );

/**
  Create an FVID table based on the algorithm provided by the BIOS writer's guide.

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in] MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer

  @retval EFI_SUCCESS            FVID table created successfully.
  @retval EFI_INVALID_PARAMETER  The bus ratio range don't permit FVID table calculation;
                                 a default FVID table should be constructed.
**/
EFI_STATUS
CreateFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates
  );

/**
  Create default FVID table with max and min states only.

  @param[in out] FvidPointer  Pointer to a table to be updated
**/
VOID
CreateDefaultFvidTable (
  IN OUT FVID_TABLE *FvidPointer
  );

/**
  Completes platform power management initialization
    (1) Initializes the TSC update variables.
    (2) Initializes the GV state for processors.
    (3) Saves MSR state for S3
    (4) Adds a callback (SMI) in S3 resume script to restore the MSR
**/
VOID
InitPpmPost (
  VOID
  );

/**
  Set processor P state to HFM or LFM
**/
VOID
SetBootPState (
  VOID
  );

/**
  Set processor P state to HFM or LFM.

  @param[in out] Buffer    Unused

  @retval EFI_SUCCESS  Processor MSR setting is saved.
**/
VOID
EFIAPI
ApSafeSetBootPState (
  IN OUT VOID *Buffer
  );

/**
  Initializes C States Power management features
**/
VOID
InitCState (
  VOID
  );

/**
  Enables C-State support as specified by the input flags on all logical
  processors and sets associated timing requirements in the chipset.

  @param[in] This          Pointer to the protocol instance
  @param[in] C3IoAddress   IO address to generate C3 states (PM base + 014 usually)
**/
VOID
EnableCStates (
  IN UINT16 C3IoAddress
  );

/**
  Enable C-State support as specified by the input flags on a logical processor.
    Configure BIOS C1 Coordination (SMI coordination)
    Enable IO redirection coordination
    Choose proper coordination method
    Configure extended C-States

  This function must be MP safe.

  @param[in out] Buffer        Pointer to a ENABLE_CSTATE_PARAMS containing the necessary
                           information to enable C-States

  @retval EFI_SUCCESS   Processor C-State support configured successfully.
**/
VOID
EFIAPI
ApSafeEnableCStates (
  IN OUT VOID *Buffer
  );

/**
  This will perform Miscellaneous Power Management related programming.

  @param[in] CtdpSupport   Status of InitMiscFeatures funtion
**/
VOID
InitMiscFeatures (
  );

/**
  This will perform general thermal initialization other then TM1, TM2, or
  PROCHOT# on all logical processors.
**/
VOID
InitThermal (
  VOID
  );

/**
  This will perform enable thermal initialization. TM1, TM2 and adaptive thermal
  throttling are enabled/disabled together.

  This function must be MP safe.

  @param[in out] Buffer    Pointer to the function parameters passed in.

  @retval EFI_SUCCESS  General thermal initialization completed successfully
**/
VOID
EFIAPI
ApSafeInitThermal (
  IN OUT VOID *Buffer
  );

/**
  Enables the bi-directional PROCHOT# signal.

  @retval EFI_SUCCESS  PROCHOT# configured successfully
**/
EFI_STATUS
EnableProcHot (
  VOID
  );

/**
  Locks down all settings.
**/
VOID
PpmLockDown (
  VOID
  );

/**
  Lock MSR_PMG_CST_CONFIG
  This function must be MP safe.

  @param[in out] Buffer    Not used (needed for API compatibility)

  @retval EFI_SUCCESS  Processor C-State locked successfully.
**/
VOID
EFIAPI
ApSafeLockDown (
  IN OUT VOID *Buffer
  );

/**
  Runs the specified procedure on all logical processors, passing in the
  parameter buffer to the procedure.

  @param[in out] Procedure  The function to be run.
  @param[in out] Buffer     Pointer to a parameter buffer.

  @retval EFI_SUCCESS
**/
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE Procedure,
  IN OUT VOID             *Buffer
  );

/**
  Configures following fields of MSR 0x610 based on user configuration:
    Configures Long duration Turbo Mode (power limit 1) power level and time window
    Configures Short duration Turbo mode (power limit 2)
**/
VOID
ConfigurePowerLimits (
  VOID
  );

/**
  Configures following fields of PL3 MSR 0x615 based on user configuration:
    Configures PL 3 and PL4 power level and time window
**/
VOID
ConfigurePl3AndPl4PowerLimits (
  VOID
  );

/**
  Configure VR Current Config of MSR 601
**/
VOID
ConfigureVrCurrentConfig (
  VOID
  );

/**
  Configures following fields of MSR 0x618 based on user configuration:
    Configures Long duration Turbo Mode (power limit 1) power level and time window for DDR domain
    Configures Short duration Turbo mode (power limit 2)
**/
VOID
ConfigureDdrPowerLimits (
  VOID
  );

/**
  Configures MSR 0x65C platform power limits (PSys)
    -Configures Platform Power Limit 1 Enable, power and time window
    -Configures Platform Power Limit 2 Enable, power
**/
VOID
ConfigurePlatformPowerLimits (
  VOID
  );

/**
  Configures following fields of MSR 0x610
    Configures Long duration Turbo Mode (power limit 1) power level and time window
    Configures Short duration turbo mode (power limit 2)
**/
VOID
ConfigureTdpPowerLimits (
  VOID
  );

/**
  Patch the native _PSS package with the EIST values
  Uses ratio/VID values from the FVID table to fix up the control values in the _PSS.

  (1) Find _PSS package:
    (1.1) Find the _PR_CPU0 scope.
    (1.2) Save a pointer to the package length.
    (1.3) Find the _PSS AML name object.
  (2) Resize the _PSS package.
  (3) Fix up the _PSS package entries
    (3.1) Check Turbo mode support.
    (3.2) Check Dynamic FSB support.
  (4) Fix up the Processor block and \_PR_CPU0 Scope length.
  (5) Update SSDT Header with new length.

  @retval EFI_SUCCESS
  @retval EFI_NOT_FOUND - If _PR_.CPU0 scope is not foud in the ACPI tables
**/
EFI_STATUS
AcpiPatchPss (
  VOID
  );

/**
  Configure the FACP for C state support
**/
VOID
ConfigureFadtCStates (
  VOID
  );

/**
  Locate the CPU ACPI tables data file and read ACPI SSDT tables.
  Publish the appropriate SSDT based on current configuration and capabilities.

  @param[in] This                Pointer to the protocol instance

  @retval EFI_SUCCESS - on success
  @retval Appropiate failure code on error
**/
EFI_STATUS
InitCpuAcpiTable (
  VOID
  );
#endif
