/** @file
  Routines for displaying virtual keyboard in bitlocker

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <PiDxe.h>
#include <Guid/ReturnFromImage.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SetupMouse.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/AbsolutePointer.h>
#include "SetupMouse.h"

#define TICKS_PER_MS            10000U

extern PRIVATE_MOUSE_DATA              *mPrivate;

BOOLEAN
EFIAPI
IsWindows (
  EFI_HANDLE                    Handle,
  UINT32                        *VersionMS,
  UINT32                        *VersionLS
  );

VOID
EFIAPI
OutputDevicePluginCallback (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  );
/**

 Close check read key event

 @param [in] Event              Event
 @param [in] Context            Passed parameter to event handler

**/
VOID
EFIAPI
DisableCheckReadKeyHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  PRIVATE_MOUSE_DATA              *Private;

  Private = mPrivate;

  gBS->CloseEvent (Private->DisableCheckReadKeyEvent);
  Private->DisableCheckReadKeyEvent = NULL;

  if (Private->CheckReadKeyEvent != NULL) {
    gBS->CloseEvent (Private->CheckReadKeyEvent);
    Private->CheckReadKeyEvent = NULL;
  }
}

/**

 Close setup mouse if no read key at a period time

 @param [in] Event              Event
 @param [in] Context            Passed parameter to event handler

**/
VOID
EFIAPI
CheckReadKeyHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  PRIVATE_MOUSE_DATA              *Private;

  Private = mPrivate;

  if (!Private->ReadKeyFlag) {
    Private->NoReadKeyCount++;
  } else {
    Private->NoReadKeyCount = 0;
  }

  Private->ReadKeyFlag = FALSE;

  if (Private->NoReadKeyCount > 2) {
    gBS->CloseEvent (Private->CheckReadKeyEvent);
    Private->CheckReadKeyEvent = NULL;

    if (Private->DisableCheckReadKeyEvent != NULL) {
      gBS->CloseEvent (Private->DisableCheckReadKeyEvent);
      Private->DisableCheckReadKeyEvent = NULL;
    }

    Private->SetupMouse.Close (&Private->SetupMouse);
  }
}




/**

 Monitor readkeystroke function to create timer event of bitlocker.

 @param [in]  This              Protocol instance pointer of SimpleTextInputEx.
 @param [out] KeyData           EFI key data.

 @retval                        Origianl ReadKeyStrokeEx function return status

**/
EFI_STATUS
EFIAPI
BitlockerKeyboardReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_POINTER_STATE      SimplePointerState;
  EFI_ABSOLUTE_POINTER_STATE    AbsolutePointerState;
  KEYBOARD_ATTRIBUTES           KeyboardAttrs;
  PRIVATE_MOUSE_DATA            *Private;
  UINTN                         X;
  UINTN                         Y;
  BOOLEAN                       LeftButton;
  BOOLEAN                       RightButton;
  EFI_SETUP_MOUSE_PROTOCOL      *SetupMouse;
  BOOLEAN                       ShowVirtualKeyboard;
  BOOLEAN                       StartSetupMouse;

  Private = mPrivate;

  if (!Private->InStartImage) {
    return Private->OrgReadKeyStrokeEx (This, KeyData);
  }

  if (Private->FirstIn) {
    Private->FirstIn = FALSE;

    Status = gBS->HandleProtocol (
                    gST->ConsoleInHandle,
                    &gEfiSimplePointerProtocolGuid,
                    (VOID **) &Private->SimplePointer
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      goto CheckReadKey;
    }


    Status = gBS->HandleProtocol (
                    gST->ConsoleInHandle,
                    &gEfiAbsolutePointerProtocolGuid,
                    (VOID **) &Private->AbsolutePointer
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      goto CheckReadKey;
    }
    Private->SimplePointer->Reset (Private->SimplePointer, TRUE);
    Private->AbsolutePointer->Reset (Private->AbsolutePointer, TRUE);
  }

  ShowVirtualKeyboard = FALSE;
  StartSetupMouse = FALSE;
  SetupMouse = &Private->SetupMouse;
  if (!Private->IsStart) {
    Status = Private->SimplePointer->GetState(Private->SimplePointer, &SimplePointerState);
    if (!EFI_ERROR (Status)) {
      StartSetupMouse = TRUE;
      if (SimplePointerState.LeftButton) {
        ShowVirtualKeyboard = TRUE;
      }
    }

    Status = Private->AbsolutePointer->GetState(Private->AbsolutePointer, &AbsolutePointerState);
    if (!EFI_ERROR (Status)) {
      StartSetupMouse = TRUE;
      if ((AbsolutePointerState.ActiveButtons & EFI_ABSP_TouchActive) == EFI_ABSP_TouchActive) {
        ShowVirtualKeyboard = TRUE;
      }
    }
    if (StartSetupMouse) {
      SetupMouse->Start (SetupMouse);
    }
  }

  if (Private->IsStart) {

    if (!ShowVirtualKeyboard) {
      LeftButton = FALSE;
      Status = SetupMouse->QueryState (SetupMouse, &X, &Y, &LeftButton, &RightButton);
      if (!EFI_ERROR (Status) && LeftButton) {
        ShowVirtualKeyboard = TRUE;
      }
    }

    if (ShowVirtualKeyboard) {
      SetupMouse->GetKeyboardAttributes (SetupMouse, &KeyboardAttrs);
      if (Private->FirstDisplayKB) {
        Private->FirstDisplayKB = FALSE;
        SetupMouse->StartKeyboard (SetupMouse, (UINTN) 10000, (UINTN) 10000); // right-bottom
      } else {
        SetupMouse->StartKeyboard (SetupMouse, (UINTN) KeyboardAttrs.X, (UINTN) KeyboardAttrs.Y);
      }
    }
  }

CheckReadKey:

  Private->ReadKeyFlag = TRUE;
  Status = Private->OrgReadKeyStrokeEx (This, KeyData);
  if (EFI_ERROR (Status) || !Private->IsStart) {
    return Status;
  }

  if (Private->DisableCheckReadKeyEvent == NULL &&
      Private->CheckReadKeyEvent == NULL &&
      (KeyData->Key.ScanCode == SCAN_ESC || KeyData->Key.UnicodeChar == CHAR_CARRIAGE_RETURN)) {

    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    DisableCheckReadKeyHandler,
                    Private,
                    &Private->DisableCheckReadKeyEvent
                    );
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer(Private->DisableCheckReadKeyEvent, TimerRelative, 500 * TICKS_PER_MS);
    }

    Private->NoReadKeyCount = 0;
    Status = gBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    CheckReadKeyHandler,
                    Private,
                    &Private->CheckReadKeyEvent
                    );
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer(Private->CheckReadKeyEvent, TimerPeriodic, 50 * TICKS_PER_MS);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
StartImageSupportVirtualKeyboard (
  IN  EFI_HANDLE                  ImageHandle,
  OUT UINTN                       *ExitDataSize,
  OUT CHAR16                      **ExitData    OPTIONAL
  )
{
  EFI_STATUS                    Status;
  UINT32                        VersionMS;
  UINT32                        VersionLS;
  PRIVATE_MOUSE_DATA            *Private;
  VOID                          *Registration;

  Private = mPrivate;
  if (!IsWindows (ImageHandle, &VersionMS, &VersionLS)) {
    goto Done;
  }

  Status = gBS->HandleProtocol (
                  gST->ConsoleInHandle,
                  &gEfiSimpleTextInputExProtocolGuid,
                  (VOID **) &Private->SimpleTextInputEx
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Private->FirstIn        = TRUE;
  Private->FirstDisplayKB = TRUE;

  if (Private->GopHotplugEvent == NULL) {
    AcquireSetupMouseLock (Private);
    Private->GopHotplugEvent = EfiCreateProtocolNotifyEvent (
                                 &gEfiSimpleTextOutProtocolGuid,
                                 TPL_NOTIFY - 1,
                                 OutputDevicePluginCallback,
                                 NULL,
                                 &Registration
                                 );
    ReleaseSetupMouseLock (Private);
  }

  //
  // hook ReadKeyStrokeEx of SimpleTextInEx
  //
  if (Private->OrgReadKeyStrokeEx == NULL) {
    Private->OrgReadKeyStrokeEx = Private->SimpleTextInputEx->ReadKeyStrokeEx;
    Private->SimpleTextInputEx->ReadKeyStrokeEx = BitlockerKeyboardReadKeyStrokeEx;
  }
  Private->InStartImage = TRUE;

Done:
  Status = Private->OrgStartImage (ImageHandle, ExitDataSize, ExitData);

  if (Private->OrgReadKeyStrokeEx != NULL) {
    Private->SimpleTextInputEx->ReadKeyStrokeEx = Private->OrgReadKeyStrokeEx;
  }

  Private->InStartImage = FALSE;

  if (Private->CheckReadKeyEvent != NULL) {
    gBS->CloseEvent (Private->CheckReadKeyEvent);
    Private->CheckReadKeyEvent = NULL;
  }

  if (Private->DisableCheckReadKeyEvent != NULL) {
    gBS->CloseEvent (Private->DisableCheckReadKeyEvent);
    Private->DisableCheckReadKeyEvent = NULL;
  }

  if (Private->GopHotplugEvent != NULL) {
    gBS->CloseEvent (Private->GopHotplugEvent);
    Private->GopHotplugEvent = NULL;
  }

  if (Private->IsStart) {
    Private->SetupMouse.Close (&Private->SetupMouse);
  }

  return Status;
}

