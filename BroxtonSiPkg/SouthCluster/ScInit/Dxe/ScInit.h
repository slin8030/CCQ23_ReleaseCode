/** @file
  Header file for Initialization Driver.

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

#ifndef _SC_INITIALIZATION_DRIVER_H_
#define _SC_INITIALIZATION_DRIVER_H_

#ifndef FSP_FLAG
#include <Library/UefiLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Protocol/FirmwareVolume2.h>
#endif
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/EventGroup.h>
#include <Library/ConfigBlockLib.h>
//
// Driver Consumed Protocol Prototypes
//
#ifndef FSP_FLAG
#include <Protocol/GlobalNvsArea.h>
#include <Protocol/BootScriptSave.h>
#include <Protocol/ExitPmAuth.h>
#include <Protocol/ScS3Support.h>
#include <Library/ScAslUpdateLib.h>
#include <Protocol/BlockIo.h>
#endif
#include <Protocol/ScInfo.h>

#include <ScAccess.h>
#include <Library/ScPlatformLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/SideBandLib.h>
#include <PlatformBaseAddresses.h>
#include <Private/Library/UsbCommonLib.h>
#include <Private/Library/SataInitLib.h>
#include <Private/Library/ScInitCommonLib.h>
#include <Private/Guid/ScPolicyHobGuid.h>
#include <Library/MmPciLib.h>
#include <Protocol/ScEmmcTuning.h>
#include <IndustryStandard/Pci30.h>
#include <Library/SteppingLib.h>

// Using SC_PM_ENABLE macro to add PM supporting for specific HSLE modules. Don't enable it for normal release, which will cause driver
// issue (PCI header reset) around WW23'14 modules.
#ifndef FSP_FLAG
#define SC_PM_ENABLE
#endif

typedef struct {
  EFI_SC_INFO_PROTOCOL ScInfo;
} SC_INSTANCE_PRIVATE_DATA;

//
// Data definitions
//
extern EFI_HANDLE mImageHandle;

///
///  IRQ routing init table entry
///
typedef struct {
  UINTN BusNumber;
  UINTN DeviceNumber;
  UINTN FunctionNumber;
  UINTN Irq;
} SC_IRQ_INIT_ENTRY;

/**
  Initialize the SC device according to the SC Platform Policy

  @param[in] ScInstance                 SC instance private data. May get updated by this function
  @param[in] ScPolicy                   The SC Policy instance
  @param[in] PmcBase                    PMC base address of this SC device
  @param[in] AcpiBaseAddr               ACPI IO base address of this SC device

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
InitializeScDevice (
  IN OUT SC_INSTANCE_PRIVATE_DATA       *ScInstance,
  IN     SC_POLICY_HOB                  *ScPolicy,
  IN     UINT32                         PmcBase,
  IN     UINT16                         AcpiBaseAddr
  );

/**
  Program Sc devices dedicated IRQ#.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigureIrqAtBoot (
  IN      SC_POLICY_HOB                 *ScPolicy
  );

/**
  Configure and Enable Direct IRQs (IOAPIC) for PMC

  Note: These settings are OS-agnostic.

  If the IP never enters D0i3, then Direct IRQ has no impact
  and is never triggered.

  @param[in]     AcpiBaseAddr        ACPI IO base address

  @retval EFI_SUCCESS                The function completed successfully
**/
VOID
ConfigurePlatformDirectIrqs (
  IN     UINT16            AcpiBaseAddr
  );

/**
  SC initialization triggered by ExitBootServices events
  Useful for operations which must happen later than at the end of post

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScExitBootServicesEvent (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  );

  /**
  SC initialization triggered by ExitBootServices for LPSS events

  This should be at a higher TPL than the callback that sets Untrusted Mode.

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScLpssExitBootServicesEvent (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  );

/**
  SC initialization before Boot Script Table is closed

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScOnEndOfDxe (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  );

/**
  Hide PCI config space of HD-Audio device and do any final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
  @retval EFI_UNSUPPORTED               Device non-exist or Dsp is disabled
**/
EFI_STATUS
ConfigureHdaAtBoot (
  IN SC_POLICY_HOB                      *ScPolicy
  );

/**
  Perform the remaining configuration on SC SATA to perform device detection
  at end of Dxe, then set the SATA SPD and PxE corresponding, and set the Register Lock

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigureSataDxe (
  IN SC_POLICY_HOB                      *ScPolicy
  );

/**
  Hide PCI config space of Otg device and do any final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
  @retval EFI_NOT_FOUND                 GNVS Protocol interface not found
**/
EFI_STATUS
ConfigureOtgAtBoot (
  IN SC_POLICY_HOB                      *ScPolicy
  );

/**
  Hide PCI config space of Lpss devices and do any final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
  @retval EFI_NOT_FOUND                 GNVS Protocol interface not found
**/
EFI_STATUS
ConfigureLpssAtBoot (
  IN SC_POLICY_HOB                      *ScPolicy
  );

/**
  Stop Lpss devices[I2C and SPI] if any is running.

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
StopLpssAtBoot (
  VOID
  );

#ifdef FSP_FLAG
/**
  Pmc devices final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigurePmcAtBoot (
  IN SC_POLICY_HOB     *ScPolicy
  );
#endif

/**
  Register notification function for PM configuration after PCI enumeration

  @param[in]  ScPolicy                  The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigurePmcAfterPciEnum (
  IN SC_POLICY_HOB                      *ScPolicy
  );


/**
  Dump whole SC_POLICY_HOB and serial out.

  @param[in] ScPolicy                   The SC Policy instance
**/
VOID
ScDumpPolicy (
  IN  SC_POLICY_HOB                     *ScPolicy
  );

/**
  Configures ports of the SC USB3 (xHCI) controller before entering OS.     the SC USB3 (xHCI) controller

  @param[in] ScPolicy                   The SC Policy instance
**/
VOID
ConfigureXhciAtBoot (
  IN SC_POLICY_HOB                      *ScPolicy
  );

/**
  Update ASL object before Boot

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_STATUS
  @retval EFI_NOT_READY         The Acpi protocols are not ready.
**/
EFI_STATUS
UpdateNvsArea (
  IN     SC_POLICY_HOB         *ScPolicy
  );

#endif
