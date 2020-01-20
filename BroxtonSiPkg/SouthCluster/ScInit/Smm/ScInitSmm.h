/** @file
  Header file for SC Init SMM Handler

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

@par Specification Reference:
**/

#ifndef _SC_INIT_SMM_H_
#define _SC_INIT_SMM_H_

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmIoTrapDispatch2.h>
#include <Library/S3BootScriptLib.h>
#include <Library/HobLib.h>
#include <Protocol/SmmCpu.h>

#include <IndustryStandard/Pci30.h>
#include <ScAccess.h>
#include <Library/ScPlatformLib.h>
#include <Library/MmPciLib.h>
#include <Private/Library/PeiDxeSmmScPciExpressHelpersLib.h>
#include <Private/Guid/ScPolicyHobGuid.h>
#include <Ppi/ScPcieDeviceTable.h>
#include <Protocol/ScPcieSmiDispatch.h>
#include <Protocol/ScSmmIoTrapControl.h>
#include <Private/Protocol/ScNvs.h>
#include <Private/Protocol/PcieIoTrap.h>
#include <Include/SiPolicyHob.h>
#include <Library/ConfigBlockLib.h>

extern EFI_SMM_SW_DISPATCH2_PROTOCOL *mSwDispatch;
extern EFI_SMM_SX_DISPATCH2_PROTOCOL *mSxDispatch;

extern SC_NVS_AREA                   *mScNvsArea;
extern UINT16                        mAcpiBaseAddr;

extern SC_POLICY_HOB                 *mScPolicy;
extern SC_PCIE_CONFIG                *mPcieRpConfig;
extern EFI_PHYSICAL_ADDRESS          mResvMmioBaseAddr;
extern UINTN                         mResvMmioSize;
extern UINT32                        mAspmSwSmiNumber;

#define EFI_PCI_CAPABILITY_ID_PCIPM 0x01

#define DeviceD0 0x00
#define DeviceD3 0x03

#define ARRAY_SIZE(data) (sizeof (data) / sizeof (data[0]))

typedef enum {
  PciCfg,
  PciMmr
} SC_PCI_ACCESS_TYPE;

typedef enum {
  Acpi,
  Rcrb,
  Tco
} SC_ACCESS_TYPE;

typedef struct {
  SC_ACCESS_TYPE AccessType;
  UINT32          Address;
  UINT32          Data;
  UINT32          Mask;
  UINT8           Width;
} SC_SAVE_RESTORE_REG;

typedef struct {
  SC_SAVE_RESTORE_REG*  ScSaveRestoreReg;
  UINT8                 size;
} SC_SAVE_RESTORE_REG_WRAP;

struct _SC_SAVE_RESTORE_PCI;

typedef struct _SC_SAVE_RESTORE_PCI{
  SC_PCI_ACCESS_TYPE AccessType;
  UINT8            Device;
  UINT8            Function;
  UINT8            BarOffset;
  UINT16           Offset;
  UINT32           Data;
  UINT32           Mask;
  UINT8            Width;
  VOID            (*RestoreFunction)(struct _SC_SAVE_RESTORE_PCI *ScSaveRestorePci);
} SC_SAVE_RESTORE_PCI;

typedef struct {
  SC_SAVE_RESTORE_PCI*  ScSaveRestorePci;
  UINT8                 size;
} Sc_SAVE_RESTORE_PCI_WRAP;

typedef struct {
  UINT8           Device;
  UINT8           Function;
  UINT8           PowerState;
} DEVICE_POWER_STATE;

/**
  Register PCIE Hotplug SMI dispatch function to handle Hotplug enabling

  @param[in] ImageHandle          The image handle of this module
  @param[in] SystemTable          The EFI System Table

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
EFIAPI
InitializeScPcieSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  );

/**
  Program Common Clock and ASPM of Downstream Devices

  @param[in] PortIndex                  Pcie Root Port Number
  @param[in] RpDevice                   Pcie Root Pci Device Number
  @param[in] RpFunction                 Pcie Root Pci Function Number

  @retval EFI_SUCCESS                   Root port complete successfully
  @retval EFI_UNSUPPORTED               PMC has invalid vendor ID
**/
EFI_STATUS
ScPcieSmi (
  IN  UINT8                             PortIndex,
  IN  UINT8                             RpDevice,
  IN  UINT8                             RpFunction
  );

/**
  PCIE Hotplug SMI call back function for each Root port

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.
  @retval  None
**/
VOID
EFIAPI
ScPcieSmiRpHandlerFunction (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  SC_PCIE_SMI_RP_CONTEXT            *RpContext
  );

/**
  PCIE Link Active State Change Hotplug SMI call back function for all Root ports

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.

  @retval  None
**/
VOID
EFIAPI
ScPcieLinkActiveStateChange (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  SC_PCIE_SMI_RP_CONTEXT            *RpContext
  );

/**
  PCIE Link Equalization Request SMI call back function for all Root ports

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.

  @retval  None
**/
VOID
EFIAPI
ScPcieLinkEqHandlerFunction (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  SC_PCIE_SMI_RP_CONTEXT            *RpContext
  );

/**
  The SW SMI callback to config PCIE power management settings

  @param[in]      DispatchHandle  The handle of this callback, obtained when registering
  @param[in]      CallbackContext Pointer to the EFI_SMM_SW_REGISTER_CONTEXT
  @param[in, out] CommBuffer      Point to the CommBuffer structure
  @param[in, out] CommBufferSize  Point to the Size of CommBuffer structure
**/
VOID
ScPciePmSwSmiCallback (
  IN     EFI_HANDLE              DispatchHandle,
  IN EFI_SMM_SW_REGISTER_CONTEXT *DispatchContext,
  IN OUT VOID                    *CommBuffer,
  IN OUT UINTN                   *CommBufferSize
  );

/**
  Initializes the PCH SMM handler for PCH save and restore

  @param[in] ImageHandle - Handle for the image of this driver
  @param[in] SystemTable - Pointer to the EFI System Table

  @retval EFI_SUCCESS    - PCH SMM handler was installed
**/
EFI_STATUS
EFIAPI
ScInitLateSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  );

/**
  Locate required protocol and register the 61h IO trap

  @param[in] ImageHandle - Handle for the image of this driver
  @param[in] SystemTable - Pointer to the EFI System Table

  @retval EFI_SUCCESS    - PCH SMM handler was installed
**/
EFI_STATUS
EFIAPI
InstallIoTrapPort61h (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

/**
  Initialize PCH Sx entry SMI handler.

  @param[in] ImageHandle - Handle for the image of this driver

  @retval  None
**/
VOID
InitializeSxHandler (
  IN EFI_HANDLE        ImageHandle
  );

/**
  PCH Sx entry SMI handler.

  @param[in]     Handle          Handle of the callback
  @param[in]     Context         The dispatch context
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS
**/

EFI_STATUS
EFIAPI
ScSxHandler (
  IN  EFI_HANDLE                   Handle,
  IN CONST VOID                    *Context OPTIONAL,
  IN OUT VOID                      *CommBuffer OPTIONAL,
  IN OUT UINTN                     *CommBufferSize OPTIONAL
  );

/**
  GbE Sx entry handler

  @param[in]  None

  @retval  None
**/
VOID
ScLanSxCallback (
  VOID
  );

/**
  This function performs GPIO Sx Isolation steps.

  @param[in]  None

  @retval  None
**/
VOID
ScGpioSxIsolationCallBack (
  VOID
  );

/**
  Register dispatch function to handle GPIO pads Sx isolation

  @param[in]  None

  @retval  None
**/
VOID
InitializeGpioSxIsolationSmm (
  VOID
  );

/**
  Entry point for Sc Bios Write Protect driver.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
**/
EFI_STATUS
EFIAPI
InstallScBiosWriteProtect (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  );

#endif
