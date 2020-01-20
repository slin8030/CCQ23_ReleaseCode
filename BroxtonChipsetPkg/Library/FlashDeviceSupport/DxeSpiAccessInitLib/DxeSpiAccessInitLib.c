/** @file
  SPI Access Init routines

;******************************************************************************
;* Copyright (c) 2012-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Guid/EventGroup.h>
#include <ScAccess.h>
#include <Protocol/SmmBase2.h>

static EFI_EVENT              mEfiExitBootServicesEvent;
static BOOLEAN                mEfiAtRuntime           = FALSE;



/**
  Set AtRuntime flag as TRUE after ExitBootServices.

  @param[in]  Event   The Event that is being processed.
  @param[in]  Context The Event Context.

**/
VOID
EFIAPI
DxeSpiAccessInitLibExitBootServicesEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  mEfiAtRuntime = TRUE;
}


/**
  Detect whether the system is at SMM mode.

  @retval TRUE                  System is at SMM mode.
  @retval FALSE                 System is not at SMM mode.
**/
STATIC
BOOLEAN
IsInSmm (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_SMM_BASE2_PROTOCOL         *SmmBase;
  BOOLEAN                        InSmm;

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  InSmm = FALSE;
  SmmBase->InSmm (SmmBase, &InSmm);
  return InSmm;
}


/**
  Detect whether the system is at EFI runtime or not

  @param  None

  @retval TRUE                  System is at EFI runtime
  @retval FALSE                 System is not at EFI runtime

**/
BOOLEAN
EFIAPI
AtRuntime (
  VOID
  )
{
  return mEfiAtRuntime;
}

/**
  Initialization routine for SpiAccessLib

  @param  None

  @retval EFI_SUCCESS           SpiAccessLib successfully initialized
  @return Others                SpiAccessLib initialization failed

**/
EFI_STATUS
EFIAPI
SpiAccessInit (
  VOID
  )
{
  EFI_PHYSICAL_ADDRESS BaseAddress;
  UINTN                Length;
  EFI_STATUS           Status;

  Status = EFI_SUCCESS;
  if (!IsInSmm ()) {
    //
    // Only create exit boot services event in protected mode.
    // Otherwise, system will trigger exception if event is signaled and callback to function
    // which is located in SMM RAM.
    //
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    DxeSpiAccessInitLibExitBootServicesEvent,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &mEfiExitBootServicesEvent
                    );
  
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Reserve RCBA space in GCD, which will be reported to OS by E820
  // It will assert if RCBA Memory Space is not allocated
  // The caller is responsible for the existence and allocation of the RCBA Memory Spaces
  //
  BaseAddress = (EFI_PHYSICAL_ADDRESS) MmioRead32 (
                                         MmPciAddress (0,
                                           DEFAULT_PCI_BUS_NUMBER_SC,
                                           PCI_DEVICE_NUMBER_SPI,
                                           PCI_FUNCTION_NUMBER_SPI,
                                           R_SPI_BASE
                                         )
                                       ) & B_SPI_BASE_BAR;
  Length      = 0x1000;
  Status = gDS->SetMemorySpaceAttributes (
                    BaseAddress,
                    Length,
                    EFI_MEMORY_RUNTIME
                    );
  return Status;
}

/**
  This routine uses to free the allocated resource by SpiAccessInit ().

  @retval EFI_SUCCESS    Free allocated resource successful.
  @return Others         Free allocated resource failed.
**/
EFI_STATUS
EFIAPI
SpiAccessDestroy (
  VOID
  )
{
  EFI_STATUS      Status;

  if (mEfiExitBootServicesEvent != NULL) {
    Status = gBS->CloseEvent (mEfiExitBootServicesEvent);
    ASSERT_EFI_ERROR (Status);
  }
  return EFI_SUCCESS;
}
