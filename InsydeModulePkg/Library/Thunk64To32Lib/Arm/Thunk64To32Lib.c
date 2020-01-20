 /** @file
 Arm specifc functionality for Thunk 32 To 64 Library
  
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

// #include <Library/UefiBootServicesTableLib.h>
// #include <Library/BaseLib.h>
// #include <Library/PciLib.h>
// #include <Library/BaseMemoryLib.h>
// #include <Library/MemoryAllocationLib.h>
// #include <Library/HobLib.h>
// #include <Library/DebugLib.h>

/**
  The constructor function caches the PCI Express Base Address and creates a 
  Set Virtual Address Map event to convert physical address to virtual addresses.
  
  @param[in]  ImageHandle   The firmware allocated handle for the EFI image.
  @param[in]  SystemTable   A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS   The constructor completed successfully.
  @retval Other value   The constructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
Thunk64To32LibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS; 
}

/**
  Do thunk 64 to 32 and jmp to run code.

  @param [in]   PeiServicesPoint   Pei Services Point
  @param [in]   FunctionPoint      Function Point
  
  @retval EFI_SUCCESS     The thunk completed successfully.
  @retval EFI_NOT_READY   The thunk constructor function has not been performed
  
**/
EFI_STATUS
EFIAPI
Thunk64To32 (
  IN UINT32        PeiServicesPoint,
  IN UINT32        FunctionPoint
  )
{
  return EFI_UNSUPPORTED;
}