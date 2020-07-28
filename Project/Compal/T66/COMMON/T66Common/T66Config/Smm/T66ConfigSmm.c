/** @file
  SMM driver implementation for the IHISI Services

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <T66ConfigSmm.h>

EFI_STATUS
EFIAPI
TXE_UnlockCallBack (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  DEBUG ((EFI_D_ERROR, "TXE_UnlockCallBack Now!!!"));
  CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, TXE_UNLOCK);
  CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_PLATFORM_NOTIFICATION, EC_CMD_PLATFORM_NOTIFICATION_SYSTEM_REBOOT);

  return EFI_SUCCESS;
}


/**
  IhisiSmm module entry point

  @param ImageHandle            A handle for the image that is initializing this driver
  @param SystemTable            A pointer to the EFI system table

  @retval EFI_SUCCESS:          Module initialized successfully
  @retval Others                Module initialized unsuccessfully
**/
EFI_STATUS
EFIAPI
T66ConfigSmmEntry(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_HANDLE                                Handle;
  EFI_SMM_SW_DISPATCH2_PROTOCOL              *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT                SwContext;

  DEBUG ((EFI_D_ERROR, "T66ConfigSmmEntry ++ Now!!!"));

  if (!InSmm()) {
    return EFI_SUCCESS;
  }
  //
  // Get the Sw dispatch protocol and register SMI callback functions.
  //
  Status = gSmst->SmmLocateProtocol (
                  &gEfiSmmSwDispatch2ProtocolGuid,
                  NULL,
                  (VOID **)&SwDispatch)
                  ;
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SwContext.SwSmiInputValue = Compal_EC_ENTRY_CALLBACK_SMI;
  Status = SwDispatch->Register (
                           SwDispatch,
                           TXE_UnlockCallBack,
                           &SwContext,
                           &Handle
                           );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_ERROR, "T66ConfigSmmEntry-- Now!!!"));
  return Status;
}
