/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file

Copyright (c) 2007 - 2014, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  IdeBus.h

Abstract:

  System reset Library Services.  This library class provides a set of
  methods to reset whole system with manipulate ICH.

**/


#include <Base.h>


#include <Library/ResetSystemLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PciLib.h>

#include <ScAccess.h>

VOID
EFIAPI
PlatformResetHook (
  UINT8 ResetType
  )
{
  //
  // Platform need to save OS reset request/types for next Android boot
  //  
  IoWrite8 (0x72, EFI_CMOS_RESET_TYPE_BY_OS);
  IoWrite8 (0x73, ResetType);
}
/**
  Calling this function causes a system-wide reset. This sets
  all circuitry within the system to its initial state. This type of reset
  is asynchronous to system operation and operates without regard to
  cycle boundaries.

  System reset should not return, if it returns, it means the system does
  not support cold reset.
**/
VOID
EFIAPI
ResetCold (
  VOID
  )
{
  PlatformResetHook(COLD_RESET);
  IoWrite8 (R_PCH_RST_CNT, 0x2);
  IoWrite8 (R_PCH_RST_CNT, 0x6);
}

/**
  Calling this function causes a system-wide initialization. The processors
  are set to their initial state, and pending cycles are not corrupted.

  System reset should not return, if it returns, it means the system does
  not support warm reset.
**/
VOID
EFIAPI
ResetWarm (
  VOID
  )
{
  PlatformResetHook(WARM_RESET);
  IoWrite8 (R_PCH_RST_CNT, 0x0);
  IoWrite8 (R_PCH_RST_CNT, 0x4);
}

/**
  Calling this function causes the system to enter a power state equivalent
  to the ACPI G2/S5 or G3 states.

  System shutdown should not return, if it returns, it means the system does
  not support shut down reset.
**/
VOID
EFIAPI
ResetShutdown (
  VOID
  )
{
  UINT16  PchPmioBase;
  UINT16  Data16;
  UINT32  Data32;

  //
  // Firstly, ACPI decode must be enabled
  //
  PciOr8 (
    PCI_LIB_ADDRESS(PCI_BUS_NUMBER_ICH, PCI_DEVICE_NUMBER_ICH_LPC, 0, R_ICH_LPC_ACPI_CNT),
    (UINT8) (B_ICH_LPC_ACPI_CNT_ACPI_EN)
    );

  PchPmioBase = (UINT16) (PciRead16 (PCI_LIB_ADDRESS(PCI_BUS_NUMBER_ICH, PCI_DEVICE_NUMBER_ICH_LPC, 0, R_ICH_LPC_ACPI_BASE)) & ~BIT0);

  //
  // Then, GPE0_EN should be disabled to avoid any GPI waking up the system from S5
  //
  Data16 = 0;
  IoWrite16 (
    (UINTN)(PchPmioBase + R_ACPI_GPE0a_EN),
    (UINT16)Data16
    );

  //
  // Clear Sleep SMI Status
  //
  IoWrite16 (PchPmioBase + R_SMI_STS,
             (UINT16)(IoRead16 (PchPmioBase + R_SMI_STS) | B_SMI_STS_ON_SLP_EN));
  //
  // Clear Sleep Type Enable
  //
  IoWrite16 (PchPmioBase + R_SMI_EN,
             (UINT16)(IoRead16 (PchPmioBase + R_SMI_EN) & (~B_SMI_EN_ON_SLP_EN)));
  //
  // Clear Power Button Status
  //
  IoWrite16(PchPmioBase + R_ACPI_PM1_STS, B_ACPI_PM1_STS_PWRBTN);
  
  //
  // Secondly, Power Button Status bit must be cleared
  //
  // Write a "1" to bit[8] of power button status register at
  // (ABASE + PM1_STS) to clear this bit
  // Clear it through SMI Status register
  //
  Data16 = B_SMI_STS_PM1_STS_REG;
  IoWrite16 ((UINTN) (PchPmioBase + R_SMI_STS), Data16);
  
  //
  // Finally, transform system into S5 sleep state
  //
  Data32 = IoRead32 ((UINTN)(PchPmioBase + R_ACPI_PM1_CNT));

  Data32  = (UINT32) ((Data32 & ~(B_ACPI_PM1_CNT_SLP_TYP + B_ACPI_PM1_CNT_SLP_EN)) | V_ACPI_PM1_CNT_S5);

  IoWrite32 (
    (UINTN) (PchPmioBase + R_ACPI_PM1_CNT),
    (UINT32)Data32
    );

  Data32 = Data32 | B_ACPI_PM1_CNT_SLP_EN;

  IoWrite32 (
    (UINTN) (PchPmioBase + R_ACPI_PM1_CNT),
    (UINT32)Data32
    );
 
 return;

}

/**
  Calling this function causes the system to enter a power state for capsule
  update.

  Reset update should not return, if it returns, it means the system does
  not support capsule update.

**/
VOID
EFIAPI
EnterS3WithImmediateWake (
  VOID
  )
{
  ASSERT (FALSE);
}

