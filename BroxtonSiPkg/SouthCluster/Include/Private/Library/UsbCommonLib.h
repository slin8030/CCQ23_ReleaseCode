/** @file
  Header file for USB Common Lib

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

@par Specification Reference:
**/

#ifndef _USB_COMMON_H_
#define _USB_COMMON_H_

#include <Library/ScPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MmPciLib.h>
#include <Library/SideBandLib.h>
#include <Ppi/ScPolicy.h>
#include <Library/S3BootScriptLib.h>
#include <Pi/PiBootMode.h>
#include <Library/TimerLib.h>
#include <SiPolicyHob.h>
#include <Private/Guid/ScPolicyHobGuid.h>

typedef struct {
  UINT8   Device;
  UINT8   Function;
} USB_CONTROLLER;

#define INIT_COMMON_SCRIPT_IO_WRITE(TableName, Width, Address, Count, Buffer)

#define INIT_COMMON_SCRIPT_IO_READ_WRITE(TableName, Width, Address, Data, DataMask)

#define INIT_COMMON_SCRIPT_MEM_WRITE(TableName, Width, Address, Count, Buffer)

#define INIT_COMMON_SCRIPT_MEM_READ_WRITE(TableName, Width, Address, Data, DataMask)

#define INIT_COMMON_SCRIPT_PCI_CFG_WRITE(TableName, Width, Address, Count, Buffer)

#define INIT_COMMON_SCRIPT_PCI_CFG_READ_WRITE(TableName, Width, Address, Data, DataMask)

#define INIT_COMMON_SCRIPT_STALL(TableName, Duration)

/**
  Configures SC USB controller

  @param[in] UsbConfig            The SC Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] BusNumber            PCI Bus Number of the SC device
  @param[in] FuncDisableReg       Function Disable Register
  @param[in] BootMode             current boot mode

  @retval EFI_INVALID_PARAMETER   The parameter of ScPolicy is invalid
  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
EFIAPI
CommonUsbInit (
  IN  SC_USB_CONFIG               *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINT8                       BusNumber,
  IN OUT UINT32                   *FuncDisableReg,
  IN  EFI_BOOT_MODE               BootMode
  );

/**
  Performs basic configuration of SC USB3 (xHCI) controller.

  @param[in] UsbConfig            The SC Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of xHCI Controller
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None
**/
VOID
CommonXhciHcInit (
  IN  SC_USB_CONFIG               *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINTN                       XhciPciMmBase,
  IN  EFI_BOOT_MODE               BootMode
  );

/**
  Setup XHCI Over-Current Mapping

  @param[in] UsbConfig            The SC Platform Policy for USB configuration
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None
**/
VOID
XhciOverCurrentMapping (
  IN  SC_USB_CONFIG               *UsbConfig,
  IN  UINTN                       XhciPciMmBase
  );


/**
  Program and enable XHCI Memory Space

  @param[in] UsbConfig            The SC Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None
**/
VOID
XhciMemorySpaceOpen (
  IN  SC_USB_CONFIG               *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINTN                       XhciPciMmBase
  );

/**
  Clear and disable XHCI Memory Space

  @param[in] UsbConfig            The SC Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None
**/
VOID
XhciMemorySpaceClose (
  IN  SC_USB_CONFIG               *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINTN                       XhciPciMmBase
  );

/**
  Lock USB registers before boot

  @param[in] ScPolicy    The SC Platform Policy

  @retval None
**/
VOID
UsbInitBeforeBoot (
  IN SI_POLICY_HOB *SiPolicy,
  IN SC_POLICY_HOB *ScPolicy
);

/**
  Initialization USB Clock Gating registers

  @retval None
**/
VOID
ConfigureUsbClockGating (
  VOID
  );

#endif
