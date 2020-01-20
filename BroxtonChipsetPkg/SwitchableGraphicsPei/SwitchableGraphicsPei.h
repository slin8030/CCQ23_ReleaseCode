/** @file
  This header file is for Switchable Graphics Feature PEI module.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SWITCHABLE_GRAPHICS_PEI_H_
#define _SWITCHABLE_GRAPHICS_PEI_H_

#include <Guid/SgInfoHob.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiOemSvcChipsetLib.h>
#include <Library/PeiServicesLib.h>
#include <ScAccess.h>
#include <PlatformBaseAddresses.h>
#include <Ppi/SwitchableGraphics.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/SaPolicy.h>
#include <SetupConfig.h>
#include <SwitchableGraphicsDefine.h>

#define ACPI_TIMER_ADDR                       (FixedPcdGet16 (PcdPerfPkgAcpiIoPortBaseAddress) + 0x08)
#define ACPI_TIMER_MAX_VALUE                  0x1000000


//
// Function Prototypes
//
STATIC
EFI_STATUS
SwitchableGraphicsPeiInitialize (
  IN OUT SI_SA_POLICY_PPI                        **ChvPolicyPpi,
  IN OUT SYSTEM_CONFIGURATION                 *SystemConfiguration
  );

STATIC
VOID
SetSgRelatedSaPlatformPolicy (
  IN OUT SI_SA_POLICY_PPI                        *ChvPolicyPpi,
  IN SYSTEM_CONFIGURATION                     SystemConfiguration
  );

STATIC
EFI_STATUS
CreateSgInfoDataHob (
  IN OUT SG_INFORMATION_DATA_HOB              **SgInfoDataHob,
  IN SYSTEM_CONFIGURATION                     SystemConfiguration
  );

STATIC
SG_MODE_SETTING
SetSgModeValue (
  IN SYSTEM_CONFIGURATION                     SystemConfiguration
  );

STATIC
VOID
MxmDgpuPowerEnableSequence (
  IN H2O_SWITCHABLE_GRAPHICS_PPI              SgPpi,
  IN OUT SG_INFORMATION_DATA_HOB              *SgInfoDataHob
  );

//
// PEI Switchable Graphics PPI Function Prototypes
//
BOOLEAN
EFIAPI
SwitchableGraphicsGpioRead (
  IN UINT16                                   GpioData
  );

VOID
EFIAPI
SwitchableGraphicsGpioWrite (
  IN       UINT32                             CommunityOffset,
  IN       UINT32                             PinOffset,
  IN       BOOLEAN                            Active,
  IN       BOOLEAN                            Level
  );

EFI_STATUS
EFIAPI
SwitchableGraphicsStall (
  IN UINTN                                    Microseconds
  );

#endif
