//;******************************************************************************
//;* Copyright (c) 2013-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; 
//; 
//; 
//;------------------------------------------------------------------------------
//; 
//; Abstract: Provide main functions of Bios Region Lock protocol
//;
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <ChipsetSmiTable.h>
#include <ScPolicyCommon.h>
#include <SmiTable.h>
#include "BiosRegionLockInfo.h"
#include "BiosRegionLockHelpFun.h"

EFI_STATUS
SetRegionByType (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN BIOS_REGION_TYPE            Type
  )
/*++

Routine Description:

  This function set the BIOS region by requested type to the BIOS protect region register.

Arguments:

  This - Protocol instance pointer.
  Type - The BIOS Region type which need to be locked.

Returns:

  EFI_ACCESS_DENIED      - The BIOS protect region register has been locked.
  EFI_INVALID_PARAMETER  - The input Type is invalid.
  EFI_OUT_OF_RESOURCES   - All BIOS protect region registers have been set and the requested region 
                           can not be merge to existing protected region.
  EFI_SUCCESS            - Set BIOS protect region register successfully

--*/
{
#if 0
  UINTN      Base;
  UINTN      Length;
  EFI_STATUS Status;

  if (MmSpi16 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) {
    DEBUG ((EFI_D_ERROR, "SPI has been locked - Access Denied!\n"));
    return EFI_ACCESS_DENIED;
  }

  Status = GetBiosRegionAddress (Type, &Base, &Length);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return SetRegionByAddress (This, Base, Length);
#else
  return EFI_UNSUPPORTED;
#endif
}


EFI_STATUS
SetRegionByAddress (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN UINTN                       BaseAddress,
  IN UINTN                       Length 
  )
/*++

Routine Description:

  This function sets the BIOS region by requested base address and legnth to the BIOS protect region register.

Arguments:

  This        - Protocol instance pointer.
  BaseAddress - The start address of the BIOS region which need to be locked.
  Length      - The Length of the BIOS region which need to be locked.

Returns:

  EFI_ACCESS_DENIED      - The BIOS protect region register has been locked.
  EFI_OUT_OF_RESOURCES   - All BIOS protect region registers have been set and the requested region 
                           can not be merge to existing protected region.
  EFI_SUCCESS            - Set BIOS protect region register successfully

--*/
{
  BIOS_REGION_LOCK_INSTANCE  *Private;
  EFI_STATUS                  Status;

  if (MmSpi16 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) {
    DEBUG ((EFI_D_ERROR, "SPI has been locked - Access Denied!\n"));
    return EFI_ACCESS_DENIED;
  }

  Private = BIOS_REGION_LOCK_INSTANCE_FROM_THIS (This);
  Status  = MergeToBase (Private->BiosLock, BaseAddress, Length);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ClearRegionByType (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN BIOS_REGION_TYPE            Type
  )
/*++

Routine Description:

  This function removes the BIOS region by requested type from the BIOS protect region register.

Arguments:

  This - Protocol instance pointer.
  Type - The BIOS Region type which need to be locked.

Returns:

  EFI_ACCESS_DENIED      - The BIOS protect region register has been locked.
  EFI_INVALID_PARAMETER  - The input Type is invalid.
  EFI_OUT_OF_RESOURCES   - The BIOS protect region registers are not enough to set for all discontinuous BIOS region
  EFI_SUCCESS            - Set BIOS protect region register successfully

--*/
{
#if 0
  UINTN                       Base;
  UINTN                       Length;
  EFI_STATUS                  Status;

  if (MmSpi16 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) {
    DEBUG ((EFI_D_ERROR, "SPI has been locked - Access Denied!\n"));
    return EFI_ACCESS_DENIED;
  }
  
  Status = GetBiosRegionAddress (Type, &Base, &Length);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return ClearRegionByAddress (This, Base, Length);
#else
  return EFI_UNSUPPORTED;
#endif
}


EFI_STATUS
ClearRegionByAddress (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN UINTN                       BaseAddress,
  IN UINTN                       Length 
  )
/*++

Routine Description:

  This function removes the BIOS region by requested base address and legnth from the BIOS protect region register.

Arguments:

  This        - Protocol instance pointer.
  BaseAddress - The start address of the BIOS region which need to be locked.
  Length      - The Length of the BIOS region which need to be locked.

Returns:

  EFI_ACCESS_DENIED      - The BIOS protect region register has been locked.
  EFI_OUT_OF_RESOURCES   - The BIOS protect region registers are not enough to set for all discontinuous BIOS region
  EFI_SUCCESS            - Set BIOS protect region register successfully

--*/
{
  BIOS_REGION_LOCK_INSTANCE  *Private;
  EFI_STATUS                  Status;

  if (MmSpi16 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) {
    DEBUG ((EFI_D_ERROR, "SPI has been locked - Access Denied!\n"));
    return EFI_ACCESS_DENIED;
  }

  Private = BIOS_REGION_LOCK_INSTANCE_FROM_THIS (This);
  Status = RemoveFromBase (Private->BiosLock, BaseAddress, Length);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Lock (
  IN BIOS_REGION_LOCK_PROTOCOL  *This
  )
/*++

Routine Description:

  This function locks BIOS region described by PR0 to PR5 and trigger software SMI to enable BIOS Region SMM Protection.

Arguments:

  This        - Protocol instance pointer.

Returns:

  EFI_SUCCESS

--*/
{
//[-start-160825-IB11270160-modify]//
//
// BIOS protect region will be notify by ExitPmAuth protocol call back event in RC code "ScOnEndOfDxe",
// thus do not do the task in this function.
//
#if 0
  BIOS_REGION_LOCK_INSTANCE  *Private;
  EFI_STATUS                  Status;
//  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;

  ///
  /// Locate PCH Platform Policy protocol
  ///
//  Status = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **)&PchPlatformPolicy);
//  ASSERT_EFI_ERROR (Status);

//  if (PchPlatformPolicy->LockDownConfig->BiosLock == PCH_DEVICE_DISABLE) {
//    return EFI_SUCCESS;
//  }
  if (MmSpi16 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) {
    DEBUG ((EFI_D_ERROR, "SPI has been locked - Access Denied!\n"));
    return EFI_ACCESS_DENIED;
  }

  Private = BIOS_REGION_LOCK_INSTANCE_FROM_THIS (This);

  //
  // Program PR registers
  //
  Status = ProgramRegister (Private->BiosLock);
  ASSERT_EFI_ERROR (Status);

  //
  // Lock the Protected Range registers
  //
  if ((MmSpi32 (R_SPI_HSFS) & B_SPI_HSFS_FLOCKDN) == 0) {
    MmSpi16Or (R_SPI_HSFS, B_SPI_HSFS_FLOCKDN);
    // 
    // Invoke PchBiosLockSwSmiCallback to enable BIOS Region SMM Protection
    //
//    IoWrite8 (SW_SMI_PORT, ScPolicy->LockDownConfig.ScBiosLockSwSmiNumber);
    DEBUG ((EFI_D_ERROR, "SPI is locked now!\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "SPI is already locked. Can not lock again!\n"));
    CpuDeadLoop ();
  }
#endif
  return EFI_UNSUPPORTED;
//[-end-160825-IB11270160-modify]//

}

