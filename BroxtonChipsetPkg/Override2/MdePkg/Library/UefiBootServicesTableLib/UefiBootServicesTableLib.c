/** @file
  This library retrieve the EFI_BOOT_SERVICES pointer from EFI system table in 
  library's constructor.

  Copyright (c) 2006 - 2008, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
//[-start-140724-IB07400494-remove]//
//#include <Library/DebugLib.h>
//[-end-140724-IB07400494-remove]//

EFI_HANDLE         gImageHandle = NULL;
EFI_SYSTEM_TABLE   *gST         = NULL;
EFI_BOOT_SERVICES  *gBS         = NULL;

/**
  The constructor function caches the pointer of Boot Services Table.
    
  The constructor function caches the pointer of Boot Services Table through System Table.
  It will ASSERT() if the pointer of System Table is NULL.
  It will ASSERT() if the pointer of Boot Services Table is NULL.
  It will always return EFI_SUCCESS.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
UefiBootServicesTableLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  // Cache the Image Handle
  //
  gImageHandle = ImageHandle;
//[-start-140724-IB07400494-remove]//
//  ASSERT (gImageHandle != NULL);
//[-end-140724-IB07400494-remove]//

  //
  // Cache pointer to the EFI System Table
  //
  gST = SystemTable;
//[-start-140724-IB07400494-remove]//
//  ASSERT (gST != NULL);
//[-end-140724-IB07400494-remove]//

  //
  // Cache pointer to the EFI Boot Services Table
  //
  gBS = SystemTable->BootServices;
//[-start-140724-IB07400494-remove]//
//  ASSERT (gBS != NULL);
//[-end-140724-IB07400494-remove]//

  return EFI_SUCCESS;
}
