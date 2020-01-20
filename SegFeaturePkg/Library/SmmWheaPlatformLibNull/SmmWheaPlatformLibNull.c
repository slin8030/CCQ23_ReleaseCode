/** @file

  WheaPlatform driver functions.

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmServicesTableLib.h>
#include <Library/SmmWheaPlatformLib.h>

EFI_WHEA_PLATFORM_PROTOCOL     *mWheaPlatform = NULL;

/**
  Clear all error data in all Error status blocks

  @param [in] This                A pointer of WheaPlatformSuppourt protocol interface.

  @retval EFI_SUCCESS             Clear success.
  @retval EFI_ABORTED             WHEA is not enabled.
  @retval EFI_NOT_FOUND           ErrorStatusBlock is not ready.

**/
EFI_STATUS
WheaPlatformLibRestartAllElog(
  VOID
  )
{
 return EFI_UNSUPPORTED;
}

/**
  To clean an error record specificed by Error type

  @param [in] This                A pointer of WheaPlatformSuppourt protocol interface
  @param [in] ErrorType           Error type record to clean

  @retval EFI_SUCCESS             Error record clean success.
  @retval EFI_ABORTED             WHEA is not enabled.

**/
EFI_STATUS
WheaPlatformLibRestartElog(
  IN UINT16                            ErrorType
  )
{
  return EFI_UNSUPPORTED;
}

/**
  To trigger a SCI if it is a correct error

  @param [in] This                A pointer of WheaPlatformSuppourt protocol interface

  @retval EFI_SUCCESS             Trigger SCI success.
  @retval EFI_ABORTED             Driver is not ready.

**/
EFI_STATUS
WheaPlatformLibEndElog (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

/**
  To prepare Memory error record

  @param [in] This                A pointer of WheaPlatformSuppourt protocol interface
  @param [in] ErrorType           Error type to inject
  @param [in] MemInfo             Memory information

  @retval EFI_SUCCESS             Error record prepared.
  @retval EFI_ABORTED             WHEA is not enabled.

**/
EFI_STATUS
WheaPlatformLibElogMemory(
  IN UINT16                            ErrorType,
//[-start-150923-IB04930584-modify]//
  IN VOID                              *MemInfo
//[-end-150923-IB04930584-modify]//
  )
{
  return EFI_UNSUPPORTED;
}

/**
  To prepare Pci root bridge Aer error record

  @param [in] This                A pointer of WheaPlatformSuppourt protocol interface.
  @param [in] ErrorType           Error type to inject.
  @param [in] ErrPcieDev          Pcie device information.

  @retval EFI_SUCCESS             Error record prepared.
  @retval EFI_ABORTED             WHEA is not enabled.

**/
EFI_STATUS
WheaPlatformLibElogPcieRootDevBridge(
  IN UINT16                            ErrorType,
//[-start-150923-IB04930584-modify]//
  IN VOID                              *ErrPcieDev
//[-end-150923-IB04930584-modify]//
  )
{
  return EFI_UNSUPPORTED;
}

/**
  To prepare Pci Device Aer error record

  @param [in] This                A pointer of WheaPlatformSuppourt protocol interface.
  @param [in] ErrorType           Error type to inject.
  @param [in] ErrPcieDev          Pcie device information.

  @retval EFI_SUCCESS             Error record prepared.
  @retval EFI_ABORTED             WHEA is not enabled.

**/
EFI_STATUS
WheaPlatformLibElogPciDev(
  IN UINT16                            ErrorType,
//[-start-150923-IB04930584-modify]//
  IN VOID                              *ErrPcieDev
//[-end-150923-IB04930584-modify]//
  )
{
  return EFI_UNSUPPORTED;
}
