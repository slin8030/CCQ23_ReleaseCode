/** @file
  I2C Touch Panel driver

;******************************************************************************
;* Copyright (c) 2014 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "I2cTouchPanel.h"
#include <Library/TimerLib.h>

#define I2C_TP_EVENT_EXPIRD_PERIOD_TIME 100 // 100 ms

/**
  Get HID Descriptor

  @param [in]  I2cIo            Protocol instance pointer.
  @param [in]  HidDescReg       HID descriptor register
  @param [out] HidDescriptor    Point to HID Descriptor

  @retval EFI_SUCCESS           This driver supports this device.
  @retval EFI_DEVICE_ERROR      No ack for this command

**/
STATIC
EFI_STATUS
GetHidDescriptor (
  IN   EFI_I2C_IO_PROTOCOL      *I2cIo,
  IN   UINT32                   HidDescReg,
  OUT  HID_DESCRIPTOR           *HidDescriptor
  )
{
  EFI_STATUS                    Status;
  EFI_I2C_REQUEST_PACKET        *RequestPacket;
  EFI_I2C_OPERATION             *Operation;
  UINT8                         Buffer[sizeof(EFI_I2C_REQUEST_PACKET) + sizeof(EFI_I2C_OPERATION)];

  RequestPacket = (EFI_I2C_REQUEST_PACKET*)Buffer;
  Operation   = RequestPacket->Operation;
  RequestPacket->OperationCount = 2;
  Operation[0].Flags            = 0;
  Operation[0].LengthInBytes    = sizeof(UINT16);
  Operation[0].Buffer           = (UINT8*)&HidDescReg;
  Operation[1].Flags            = I2C_FLAG_READ;
  Operation[1].LengthInBytes    = sizeof(HID_DESCRIPTOR);
  Operation[1].Buffer           = (UINT8*)HidDescriptor;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    RequestPacket,
                    NULL
                    );
  return Status;
}

/**
  Get Report Descriptor

  @param [in]  I2cIo            Protocol instance pointer.
  @param [in]  HidDescriptor    Point to HID Descriptor
  @param [out] ReportDesc       Point to Report Descriptor

  @retval EFI_SUCCESS           This driver supports this device.
  @retval EFI_DEVICE_ERROR      No ack for this command
  @retval EFI_INVALID_PARAMETER ReportDesc == NULL
  @retval EFI_OUT_OF_RESOURCES  Memory Overflow

**/
STATIC
EFI_STATUS
GetReportDescriptor (
  IN   EFI_I2C_IO_PROTOCOL      *I2cIo,
  IN   HID_DESCRIPTOR           *HidDescriptor,
  OUT  UINT8                    **ReportDesc
  )
{
  EFI_I2C_REQUEST_PACKET        *RequestPacket;
  EFI_I2C_OPERATION             *Operation;
  UINT8                         Buffer[sizeof(EFI_I2C_REQUEST_PACKET) + sizeof(EFI_I2C_OPERATION)];
  UINT16                        ReportDescReg;
  EFI_STATUS                    Status;

  if (ReportDesc == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *ReportDesc = AllocateZeroPool (HidDescriptor->ReportDescLength);

  if (*ReportDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get the Report Descriptor
  //
  RequestPacket = (EFI_I2C_REQUEST_PACKET*)Buffer;
  Operation                     = RequestPacket->Operation;
  ReportDescReg                 = HidDescriptor->ReportDescRegister;
  RequestPacket->OperationCount = 2;
  Operation[0].Flags            = 0;
  Operation[0].LengthInBytes    = sizeof(UINT16);
  Operation[0].Buffer           = (UINT8*)&ReportDescReg;
  Operation[1].Flags            = I2C_FLAG_READ;
  Operation[1].LengthInBytes    = HidDescriptor->ReportDescLength;
  Operation[1].Buffer           = *ReportDesc;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    RequestPacket,
                    NULL
                    );
  return Status;
}

/**

  Set the power state

  @param  TpDev                 Touch panel instance
  @param  PowerState            Power state

  @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
SetPower (
  IN  I2C_TOUCH_PANEL_DEV       *TpDev,
  IN  UINT16                    PowerState
  )
{
  UINT16                        Request[2];
  EFI_STATUS                    Status;
  EFI_I2C_REQUEST_PACKET        RequestPacket;
  EFI_I2C_IO_PROTOCOL           *I2cIo;

  if ((TpDev == NULL) ||
      ((I2cIo = TpDev->I2cIo) == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Set the power state
  //
  Request[0] = TpDev->HidDescriptor.CommandRegister;
  Request[1] = SET_POWER | PowerState;
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = 0;
  RequestPacket.Operation[0].LengthInBytes = 4;
  RequestPacket.Operation[0].Buffer        = (UINT8*)Request;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    &RequestPacket,
                    NULL
                    );
  return Status;
}

/**

  Reset the device

  @param  TpDev                 Touch panel instance

  @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
ResetDevice (
  IN  I2C_TOUCH_PANEL_DEV       *TpDev
  )
{
  UINT16                        Request[2];
  EFI_STATUS                    Status;
  EFI_I2C_REQUEST_PACKET        RequestPacket;
  EFI_I2C_IO_PROTOCOL           *I2cIo;

  if ((TpDev == NULL) ||
      ((I2cIo = TpDev->I2cIo) == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Reset the device
  //
  Request[0] = TpDev->HidDescriptor.CommandRegister;
  Request[1] = RESET;
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = 0;
  RequestPacket.Operation[0].LengthInBytes = 4;
  RequestPacket.Operation[0].Buffer        = (UINT8*)Request;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    &RequestPacket,
                    NULL
                    );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Receive the status with flush the transmission buffer
  //
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = I2C_FLAG_READ;
  RequestPacket.Operation[0].LengthInBytes = 2;
  RequestPacket.Operation[0].Buffer        = (UINT8*)Request;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    &RequestPacket,
                    NULL
                    );
  return Status;
}

/**

  Flush data buffer before operation

  @param  TpDev                 Touch panel instance

  @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
FlushDataBuffer (
  IN  I2C_TOUCH_PANEL_DEV       *TpDev
  )
{
  EFI_STATUS                    Status;
  EFI_I2C_REQUEST_PACKET        RequestPacket;
  EFI_I2C_IO_PROTOCOL           *I2cIo;

  if ((TpDev == NULL) ||
      ((I2cIo = TpDev->I2cIo) == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Receive the status with flush the transmission buffer
  //
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = I2C_FLAG_READ;
  RequestPacket.Operation[0].LengthInBytes = TpDev->HidDescriptor.MaxInputLength;
  RequestPacket.Operation[0].Buffer        = TpDev->ReportData;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    &RequestPacket,
                    NULL
                    );
  return Status;
}

/**

  I2C touch panel driver need receive next event in fixed time when
  receive a touch event and user still touch.
  Otherwise it will take as finger up, but no receive release event,
  I2C touch panel driver will simulator a release event.

  @param  TpDev                 Touch panel instance

**/
VOID
I2cTouchPanelEventExpiredHandler (
  I2C_TOUCH_PANEL_DEV *TpDev
  )
{
  EFI_TPL                       OldTpl;
  UINT64                        Tick;
  UINTN                         BufferIn;


  if (TpDev->EventExpiredTime == 0) {
    return ;
  }

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  Tick = GetPerformanceCounter();
  if (Tick < TpDev->EventExpiredTime) {
    gBS->RaiseTPL (OldTpl);
    return ;
  }

  TpDev->EventExpiredTime = 0;
  if (TpDev->State[TpDev->BufferIn].ActiveButtons == 0) {
    gBS->RaiseTPL (OldTpl);
    return ;
  }

  TpDev->StateChanged = TRUE;
  TpDev->EventExpiredTime = 0;
  TpDev->State[TpDev->BufferIn].ActiveButtons = 0;

  BufferIn = TpDev->BufferIn + 1;
  if (BufferIn == STATE_BUFFER_SIZE) BufferIn = 0;
  if (BufferIn != TpDev->BufferOut) {
    //
    // Copy last state to new position for use of check procedure of last button state
    //
    CopyMem (
      &TpDev->State[BufferIn],
      &TpDev->State[TpDev->BufferIn],
      sizeof (EFI_ABSOLUTE_POINTER_STATE)
      );
    TpDev->BufferIn = BufferIn;
  }

  gBS->RestoreTPL(OldTpl);
}

/**

  It is called whenever there is data received from async interrupt
  transfer.

  @param  Event                 Wait Event
  @param  Context               Passed parameter to event handler

**/
STATIC
VOID
EFIAPI
I2cTouchPanelInterruptOnCompleted (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  I2C_TOUCH_PANEL_DEV           *TpDev;
  REPORT_GROUP                  *ReportGroup;
  UINTN                         DataLength;
  UINTN                         Index;
  EFI_ABSOLUTE_POINTER_STATE    AbsPtrState;
  UINT8                         *ReportData;
  EFI_I2C_IO_PROTOCOL           *I2cIo;
  EFI_I2C_REQUEST_PACKET        RequestPacket;
  UINTN                         BufferIn;

  TpDev      = (I2C_TOUCH_PANEL_DEV*)Context;
  I2cIo      = TpDev->I2cIo;
  DataLength = *(UINT16*)TpDev->ReportData;
  if (TpDev->TransferResult != EFI_SUCCESS || DataLength == 0) goto Exit;
  ReportData = TpDev->ReportData + sizeof (UINT16);
  DataLength -= sizeof (UINT16);
  //
  // Since Report ID is optional, our saved value of Report ID can be 0 and it means that no
  // Report ID item tags were present in the report descriptor.  If no Report ID was declared,
  // we assume only one report group exists and continue using the first report group.
  //
  if (TpDev->ReportId == 0) {
    ReportGroup = &(TpDev->ReportFieldInfo.ReportGroup[0]);
  } else {
    ReportGroup = NULL;
    for (Index = 0; Index < TpDev->ReportFieldInfo.Total; Index++) {
      if ((TpDev->ReportFieldInfo.ReportGroup[Index].Id == (*(UINT8*)ReportData)) &&
          (TpDev->ReportFieldInfo.ReportGroup[Index].DataValid)) {
        ReportGroup = &TpDev->ReportFieldInfo.ReportGroup[Index];
        break;
      }
    }
    if (!ReportGroup) goto Exit;
    if (TpDev->ReportId != ReportGroup->Id) {
      //
      // Reset the Mode parameters due to report ID changed
      //
      TpDev->ReportId = ReportGroup->Id;
      //
      // Set limit by report descriptor
      //
      TpDev->Mode.AbsoluteMinX = ReportGroup->Data.TouchPanel.FieldX.Min;
      TpDev->Mode.AbsoluteMinY = ReportGroup->Data.TouchPanel.FieldY.Min;
      TpDev->Mode.AbsoluteMaxX = ReportGroup->Data.TouchPanel.FieldX.Max;
      TpDev->Mode.AbsoluteMaxY = ReportGroup->Data.TouchPanel.FieldY.Max;
      TpDev->Mode.Attributes   = 0;
      if (ReportGroup->Data.TouchPanel.FieldAltActive.DataValid) {
        TpDev->Mode.Attributes |= EFI_ABSP_SupportsAltActive;
      }
    }
  }
  if (DataLength < ReportGroup->DataSize) {
    goto Exit;
  }
  //
  // Get TouchPanelData data
  //
  AbsPtrState.CurrentX = (UINT32) GetReportFieldValue (ReportData, DataLength, &ReportGroup->Data.TouchPanel.FieldX);
  AbsPtrState.CurrentY = (UINT32) GetReportFieldValue (ReportData, DataLength, &ReportGroup->Data.TouchPanel.FieldY);
  AbsPtrState.CurrentZ = 0;
  AbsPtrState.ActiveButtons = 0;
  if (GetReportFieldValue (ReportData, DataLength, &ReportGroup->Data.TouchPanel.FieldTouchActive)) {
    AbsPtrState.ActiveButtons |= EFI_ABSP_TouchActive;
  }
  if (GetReportFieldValue (ReportData, DataLength, &ReportGroup->Data.TouchPanel.FieldAltActive)) {
    AbsPtrState.ActiveButtons |= EFI_ABS_AltActive;
  }
  TpDev->State[TpDev->BufferIn].CurrentX = AbsPtrState.CurrentX;
  TpDev->State[TpDev->BufferIn].CurrentY = AbsPtrState.CurrentY;
  TpDev->State[TpDev->BufferIn].CurrentZ = AbsPtrState.CurrentZ;

  I2cTouchPanelEventExpiredHandler (TpDev);

  if (AbsPtrState.ActiveButtons != 0) {
    TpDev->EventExpiredTime = GetPerformanceCounter() + TpDev->EventExpiredPeriodTime;
  } else {
    TpDev->EventExpiredTime = 0;
  }

  if ((TpDev->State[TpDev->BufferIn].ActiveButtons != 0) || (AbsPtrState.ActiveButtons != 0)) {
    TpDev->StateChanged = TRUE;
    if (TpDev->State[TpDev->BufferIn].ActiveButtons != AbsPtrState.ActiveButtons) {
      TpDev->State[TpDev->BufferIn].ActiveButtons = AbsPtrState.ActiveButtons;
      //
      // Advance the internal buffer only when button state changed to reduce the dragging phenomenon
      //
      BufferIn = TpDev->BufferIn + 1;
      if (BufferIn == STATE_BUFFER_SIZE) BufferIn = 0;
      if (BufferIn != TpDev->BufferOut) {
        //
        // Copy last state to new position for use of check procedure of last button state
        //
        CopyMem (
          &TpDev->State[BufferIn],
          &TpDev->State[TpDev->BufferIn],
          sizeof (EFI_ABSOLUTE_POINTER_STATE)
          );
        TpDev->BufferIn = BufferIn;
      }
    }
  }
Exit:
  //
  // Starting next transmission
  //
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = I2C_FLAG_READ;
  RequestPacket.Operation[0].LengthInBytes = TpDev->HidDescriptor.MaxInputLength;
  RequestPacket.Operation[0].Buffer        = TpDev->ReportData;

  I2cIo->QueueRequest (
           I2cIo,
           0,
           TpDev->OnCompleteEvent,
           &RequestPacket,
           &TpDev->TransferResult
           );
}

/**

  Event notification function for ABSOLUTE_POINTER.WaitForInput event
  Signal the event if there is input from Touch Panel

  @param  Event                 Wait Event
  @param  Context               Passed parameter to event handler

**/
STATIC
VOID
EFIAPI
I2cTouchPanelWaitForInput (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  I2C_TOUCH_PANEL_DEV *TpDev;

  TpDev = (I2C_TOUCH_PANEL_DEV *) Context;

  //
  // Someone is waiting on the TouchPanel event, if there's
  // input from Touch Panel, signal the event
  //
  if (TpDev->StateChanged) {
    gBS->SignalEvent (Event);
  }
}

/**

  Get the Touch Panel state, see ABSOLUTE POINTER PROTOCOL.

  @param  This                  Protocol instance pointer.
  @param  Touch PanelState      Current Touch Panel state

  @retval EFI_SUCCESS
  @retval EFI_DEVICE_ERROR
  @retval EFI_NOT_READY

**/
STATIC
EFI_STATUS
EFIAPI
I2cGetTouchPanelState (
  IN   EFI_ABSOLUTE_POINTER_PROTOCOL    *This,
  OUT  EFI_ABSOLUTE_POINTER_STATE       *TouchPanelState
  )
{
  I2C_TOUCH_PANEL_DEV *TpDev;
  EFI_TPL             OldTpl;

  if (TouchPanelState == NULL) {
    return EFI_DEVICE_ERROR;
  }

  TpDev = I2C_TOUCH_PANEL_DEV_FROM_ABSOLUTE_POINTER_PROTOCOL (This);

  I2cTouchPanelEventExpiredHandler (TpDev);

  if (!TpDev->StateChanged) {
    return EFI_NOT_READY;
  }

  CopyMem (
    TouchPanelState,
    &TpDev->State[TpDev->BufferOut],
    sizeof (EFI_ABSOLUTE_POINTER_STATE)
    );

  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  if (TpDev->BufferOut != TpDev->BufferIn) {
    TpDev->BufferOut ++;
    if (TpDev->BufferOut == STATE_BUFFER_SIZE) TpDev->BufferOut = 0;
  }
  if (TpDev->BufferOut == TpDev->BufferIn) {
    TpDev->StateChanged = FALSE;
  }

  gBS->RestoreTPL (OldTpl);

  return EFI_SUCCESS;
}

/**

  Reset the Touch Panel device, see ABSOLUTE POINTER PROTOCOL.

  @param  This                  Protocol instance pointer.
  @param  ExtendedVerification  Ignored here

  @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
EFIAPI
I2cTouchPanelReset (
  IN EFI_ABSOLUTE_POINTER_PROTOCOL      *This,
  IN BOOLEAN                            ExtendedVerification
  )
{
  I2C_TOUCH_PANEL_DEV       *TpDev;

  TpDev = I2C_TOUCH_PANEL_DEV_FROM_ABSOLUTE_POINTER_PROTOCOL (This);
  ZeroMem (
    TpDev->State,
    sizeof (EFI_ABSOLUTE_POINTER_STATE) * STATE_BUFFER_SIZE
    );
  TpDev->StateChanged = FALSE;
  TpDev->BufferIn     = 0;
  TpDev->BufferOut    = 0;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
I2cGetReportRequest (
  IN I2C_TOUCH_PANEL_DEV     *TpDev,
  IN UINT8                   ReportId,
  IN UINT16                  ReportType,
  IN UINTN                   ReportLen,
  IN UINT8                   *Report
  )
{
  UINT8                      Buffer[sizeof(EFI_I2C_REQUEST_PACKET) + sizeof(EFI_I2C_OPERATION)];
  UINT8                      Request[8];
  EFI_STATUS                 Status;
  EFI_I2C_OPERATION          *Operation;
  EFI_I2C_REQUEST_PACKET     *RequestPacket;
  UINT32                     Length;
  EFI_I2C_IO_PROTOCOL        *I2cIo;

  if ((TpDev == NULL) ||
      ((I2cIo = TpDev->I2cIo) == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *(UINT16*)Request = TpDev->HidDescriptor.CommandRegister;
  if (ReportId < 0x0f) {
    *(UINT16*)(Request + 2) = ReportType | ReportId;
    *(UINT16*)(Request + 4) = TpDev->HidDescriptor.DataRegister;
    Length  = 6;
  } else {
    *(UINT16*)(Request + 2) = ReportType | 0x0f;
    Request[4] = ReportId;
    *(UINT16*)(Request + 5) = TpDev->HidDescriptor.DataRegister;
    Length  = 7;
  }

  RequestPacket                 = (EFI_I2C_REQUEST_PACKET*)Buffer;
  Operation                     = RequestPacket->Operation;
  RequestPacket->OperationCount = 2;
  Operation[0].Flags            = 0;
  Operation[0].LengthInBytes    = Length;
  Operation[0].Buffer           = Request;
  Operation[1].Flags            = I2C_FLAG_READ;
  Operation[1].LengthInBytes    = (UINT32)(ReportLen + 2);
  Operation[1].Buffer           = Report;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    RequestPacket,
                    NULL
                    );

  return Status;
}

EFI_STATUS
I2cHidInitReports (
  IN  I2C_TOUCH_PANEL_DEV               *TpDev
  )
{
  EFI_STATUS            Status;
  UINT8                 *ReportData;
  UINTN                 Index;
  REPORT_GROUP          *ReportGroup;

  if ((TpDev == NULL) || (TpDev->I2cIo == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  ReportData = AllocatePool (TpDev->ReportFieldInfo.MaximumFeatureLength + 16);
  if (ReportData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }


  //
  // Accroding to Linux source code,
  // get Feature Report to make some touch panel device work
  //
  for (Index = 0; Index < TpDev->ReportFieldInfo.Total; Index++) {
    ReportGroup = &TpDev->ReportFieldInfo.ReportGroup[Index];
    if (ReportGroup->DataValid &&
        ReportGroup->DataClass == TOUCH_PANEL_CLASS &&
        ReportGroup->DataType == HID_MAIN_ITEM_TAG_FEATURE &&
        ReportGroup->DataSize > 0 &&
        ReportGroup->DataAttr == ATTR_VENDOR_FEATURE) {
      Status = I2cGetReportRequest (
                 TpDev,
                 (UINT8) ReportGroup->Id,
                 GET_REPORT_FEATURE,
                 ReportGroup->DataSize,
                 ReportData
                 );
    }
  }

  FreePool (ReportData);

  return EFI_SUCCESS;
}

/**

  Set device into single input mode

  @param  TpDev                 Touch panel instance

  @retval EFI_SUCCESS
  @retval others

**/
STATIC
EFI_STATUS
SetDeviceMode (
  IN  I2C_TOUCH_PANEL_DEV       *TpDev
  )
{
  EFI_STATUS                    Status;
  REPORT_GROUP                  *ReportGroup;
  UINTN                         Length;
  UINT8                         *ReportData;
  EFI_I2C_REQUEST_PACKET        RequestPacket;
  EFI_I2C_IO_PROTOCOL           *I2cIo;
  UINT8                         Request[8];

  if ((TpDev == NULL) ||
      ((I2cIo = TpDev->I2cIo) == NULL) ||
      ((ReportGroup = TpDev->ReportFieldInfo.DeviceConfigurationReportGroup) == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Since Device Configuration must have its own report (see HUTRR34.pdf), Report ID must be
  // reported by the USB touch device and can not be the reserved value of 0.
  //
  if ((ReportGroup->Id == 0) || (!(ReportGroup->DataValid))) {
    return EFI_UNSUPPORTED;
  }
  ReportData = AllocatePool (ReportGroup->DataSize + 16);
  if (ReportData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Get the current mode
  //
  *(UINT16*)ReportData = TpDev->HidDescriptor.CommandRegister;

  if (ReportGroup->Id < 0x0f) {
    *(UINT16*)(ReportData + 2) = GET_REPORT_FEATURE | ReportGroup->Id;
    *(UINT16*)(ReportData + 4) = TpDev->HidDescriptor.DataRegister;
    Length  = 6;
  } else {
    *(UINT16*)(ReportData + 2) = GET_REPORT_FEATURE | 0x0f;
    ReportData[4] = ReportGroup->Id;
    *(UINT16*)(ReportData + 5) = TpDev->HidDescriptor.DataRegister;
    Length  = 7;
  }

  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = 0;
  RequestPacket.Operation[0].LengthInBytes = (UINT32)Length;
  RequestPacket.Operation[0].Buffer        = ReportData;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    &RequestPacket,
                    NULL
                    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length  = ReportGroup->DataSize + 2;
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = I2C_FLAG_READ;
  RequestPacket.Operation[0].LengthInBytes = (UINT32)Length;
  RequestPacket.Operation[0].Buffer        = ReportData;

  if (GetReportFieldValue (ReportData + 2, ReportGroup->DataSize, &ReportGroup->Data.TouchPanelMode.FieldDeviceMode) == HID_DIGITIZERS_DEVICE_MODE_SINGLE_INPUT) {
    //
    // Exit if current mode is in single input mode
    //
    goto EXIT;
  }
  Status = SetReportFieldValue (
             ReportData + 2,
             ReportGroup->DataSize,
             &ReportGroup->Data.TouchPanelMode.FieldDeviceMode,
             HID_DIGITIZERS_DEVICE_MODE_SINGLE_INPUT
             );
  if (EFI_ERROR (Status)) {
    goto EXIT;
  }
  if (ReportGroup->Data.TouchPanelMode.FieldDeviceId.DataValid) {
    //
    // Use a Device ID of 0 to configure all the report groups (i.e. top-level collections)
    // at the same time regardless of Device ID (see HUTRR34.pdf).
    //
    Status = SetReportFieldValue (
               ReportData + 2,
               ReportGroup->DataSize,
               &ReportGroup->Data.TouchPanelMode.FieldDeviceId,
               0
               );
    if (EFI_ERROR (Status)) {
      goto EXIT;
    }
  }
  //
  // Set the mode to single input mode
  //
  *(UINT16*)Request = TpDev->HidDescriptor.CommandRegister;

  if (ReportGroup->Id < 0x0f) {
    Length  = 6;
    *(UINT16*)(Request + 2) = SET_REPORT_FEATURE | ReportGroup->Id;
    *(UINT16*)(Request + 4) = TpDev->HidDescriptor.DataRegister;
  } else {
    Length  = 7;
    *(UINT16*)(Request + 2) = SET_REPORT_FEATURE | 0x0f;
    Request[4] = ReportGroup->Id;
    *(UINT16*)(Request + 5) = TpDev->HidDescriptor.DataRegister;
  }
  CopyMem (ReportData + Length, ReportData, *(UINT16*)ReportData);
  CopyMem (ReportData, Request, Length);
  Length += *(UINT16*)(ReportData + Length);

  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = 0;
  RequestPacket.Operation[0].LengthInBytes = (UINT32)Length;
  RequestPacket.Operation[0].Buffer        = ReportData;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    NULL,
                    &RequestPacket,
                    NULL
                    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

EXIT:
  FreePool (ReportData);
  return Status;
}

/**

  Free the resource allocated by I2C Touch Panel instance

  @param  TpDev                 I2C Touch panel instance
  @param  Controller            I2C I/O Controller Handle

**/
STATIC
VOID
FreeResource (
  IN  I2C_TOUCH_PANEL_DEV       *TpDev,
  IN  EFI_HANDLE                Controller
  )
{
  //
  // Always close protocol even the protocol has not being opened
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         gI2cTouchPanelDriverBinding.DriverBindingHandle,
         Controller
         );
  //
  // Free resources in the instance
  //
  if (TpDev) {
    if (TpDev->OnCompleteEvent) {
      gBS->CloseEvent (TpDev->OnCompleteEvent);
    }
    if (TpDev->AbsolutePointerProtocol.WaitForInput) {
      gBS->CloseEvent (TpDev->AbsolutePointerProtocol.WaitForInput);
    }
    gBS->UninstallProtocolInterface (
           Controller,
           &gEfiAbsolutePointerProtocolGuid,
           &TpDev->AbsolutePointerProtocol
           );
    if (TpDev->ReportData) {
      FreePool (TpDev->ReportData);
    }
    if (TpDev->ControllerNameTable) {
      FreeUnicodeStringTable (TpDev->ControllerNameTable);
    }
    FreePool (TpDev);
  }
}

/**
  Verify the controller type

  This routine determines if an Touch Panel is available.

  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding             Protocol instance pointer.
  @param [in] Controller                Handle of device to test.
  @param [in] RemainingDevicePath       Not used.

  @retval EFI_SUCCESS                   This driver supports this device.
  @retval other                         This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cTouchPanelDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL        *This,
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
{
  EFI_STATUS                            Status;
  EFI_I2C_IO_PROTOCOL                   *I2cIo;
  EFI_ADAPTER_INFORMATION_PROTOCOL      *Aip;
  H2O_I2C_HID_DEVICE                    *I2cHidDevice;
  UINTN                                 DataLength;
  //
  // Determine if the I2C I/O is available
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cIoProtocolGuid,
                  (VOID**) &I2cIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR ( Status )) {
    return Status;
  }
  //
  //  The I2C I/O is available
  //
  gBS->CloseProtocol (
         Controller,
         &gEfiI2cIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );
  //
  // Inspect the H2O I2C HID specific data in the controller
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiAdapterInformationProtocolGuid,
                  (VOID**) &Aip,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Aip->GetInformation (
                  Aip,
                  &gI2cHidDeviceInfoGuid,
                  (VOID**) &I2cHidDevice,
                  &DataLength
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (I2cHidDevice->ClassType != TOUCH_PANEL_CLASS) {
    Status = EFI_NOT_FOUND;
  }
  FreePool (I2cHidDevice);
  return Status;
}

/**
  Connect to the I2C Touch Panel

  This routine initializes an instance of the I2C driver for this
  controller.

  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in I2cBusDriverSupported.

  @param [in] DriverBinding             Protocol instance pointer.
  @param [in] Controller                Handle of device to work with.
  @param [in] RemainingDevicePath       Not used, always produce all possible children.

  @retval EFI_SUCCESS                   This driver is added to Controller.
  @retval other                         This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cTouchPanelDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL        *This,
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
{
  EFI_STATUS                            Status;
  I2C_TOUCH_PANEL_DEV                   *TpDev;
  EFI_I2C_IO_PROTOCOL                   *I2cIo;
  EFI_ADAPTER_INFORMATION_PROTOCOL      *Aip;
  H2O_I2C_HID_DEVICE                    *I2cHidDevice;
  UINTN                                 DataLength;
  UINT8                                 *ReportDescriptor;
  UINTN                                 Index;
  REPORT_GROUP                          *ReportGroup;
  EFI_I2C_REQUEST_PACKET                RequestPacket;
  UINT64                                CpuFreq;

  TpDev = NULL;
  //
  // Occupy the I2C I/O protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cIoProtocolGuid,
                  (VOID**) &I2cIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Get the H2O I2C HID specific data in the controller
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiAdapterInformationProtocolGuid,
                  (VOID**) &Aip,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  Status = Aip->GetInformation (
                  Aip,
                  &gI2cHidDeviceInfoGuid,
                  (VOID**) &I2cHidDevice,
                  &DataLength
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  TpDev = AllocateZeroPool (sizeof (I2C_TOUCH_PANEL_DEV));
  if (TpDev == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  TpDev->Signature   = I2C_TOUCH_PANEL_DEV_SIGNATURE;
  TpDev->I2cIo       = I2cIo;
  CopyMem (&TpDev->I2cHid, I2cHidDevice, I2cHidDevice->Length);
  FreePool (I2cHidDevice);
  //
  // Get HID descriptor
  //
  Status = GetHidDescriptor (I2cIo, TpDev->I2cHid.DescReg, &TpDev->HidDescriptor);
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  if (TpDev->HidDescriptor.HIDDescLength != sizeof(HID_DESCRIPTOR)) {
    Status = EFI_DEVICE_ERROR;
    goto Error;
  }
  TpDev->I2cHid.VendorID  = TpDev->HidDescriptor.VendorID;
  TpDev->I2cHid.ProductID = TpDev->HidDescriptor.ProductID;
  TpDev->I2cHid.VersionID = TpDev->HidDescriptor.VersionID;
  //
  // Set the VID/PID/Revision into AIP for use of OEM specific usage
  //
  Aip->SetInformation (
         Aip,
         &gI2cHidDeviceInfoGuid,
         (VOID*) &TpDev->I2cHid,
         DataLength
         );
  TpDev->ReportData = AllocateZeroPool (TpDev->HidDescriptor.MaxInputLength);
  if (TpDev->ReportData == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }
  //
  // Power on the device
  //
  SetPower (TpDev, POWER_ON);
  //
  // Reset the device
  //
  ResetDevice (TpDev);
  //
  // Flush the data buffer before operation
  //
  FlushDataBuffer (TpDev);
  //
  // Get report descriptor
  //
  Status = GetReportDescriptor ( I2cIo, &TpDev->HidDescriptor, &ReportDescriptor);
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  Status = ParseReportDescriptor (
             ReportDescriptor,
             TpDev->HidDescriptor.ReportDescLength,
             &TpDev->ReportFieldInfo
             );
  FreePool (ReportDescriptor);
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  //
  // Searching for touch panel report
  //
  for (Index = 0, ReportGroup = NULL; Index < TpDev->ReportFieldInfo.Total; Index ++) {
    if (TpDev->ReportFieldInfo.ReportGroup[Index].DataValid &&
        TpDev->ReportFieldInfo.ReportGroup[Index].DataClass == TOUCH_PANEL_CLASS &&
        TpDev->ReportFieldInfo.ReportGroup[Index].DataType == HID_MAIN_ITEM_TAG_INPUT &&
        TpDev->ReportFieldInfo.ReportGroup[Index].DataSize > 0) {
      ReportGroup =  &TpDev->ReportFieldInfo.ReportGroup[Index];
      break;
    }
  }
  if (ReportGroup == NULL) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }

  TpDev->ReportId          = ReportGroup->Id;
  TpDev->Mode.AbsoluteMinX = ReportGroup->Data.TouchPanel.FieldX.Min;
  TpDev->Mode.AbsoluteMinY = ReportGroup->Data.TouchPanel.FieldY.Min;
  TpDev->Mode.AbsoluteMaxX = ReportGroup->Data.TouchPanel.FieldX.Max;
  TpDev->Mode.AbsoluteMaxY = ReportGroup->Data.TouchPanel.FieldY.Max;
  TpDev->Mode.Attributes   = 0;
  if (ReportGroup->Data.TouchPanel.FieldAltActive.DataValid) {
    TpDev->Mode.Attributes |= EFI_ABSP_SupportsAltActive;
  }

  //
  // If Device Configuration feature report is found, this touch device has multiple operating
  // modes (mouse, single-input, multi-input, etc.).  Some devices only send "correct" report data
  // (i.e. absolute coordinates for digitizer as opposed to relative displacements for mouse) when
  // they are operating in single-input mode.  However, some other devices do not change operating
  // mode after SetDeviceMode (), but can still work with their report data; thus, we do not check
  // the return status of SetDeviceMode ().
  //
  if (TpDev->ReportFieldInfo.DeviceConfigurationReportGroup != NULL) {
    SetDeviceMode (TpDev);
  }

  I2cHidInitReports (TpDev);

  //
  // Install AbsolutePointer protocol
  //
  TpDev->AbsolutePointerProtocol.GetState  = I2cGetTouchPanelState;
  TpDev->AbsolutePointerProtocol.Reset     = I2cTouchPanelReset;
  TpDev->AbsolutePointerProtocol.Mode      = &TpDev->Mode;
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  I2cTouchPanelWaitForInput,
                  TpDev,
                  &TpDev->AbsolutePointerProtocol.WaitForInput
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  //
  // Install Absolute Pointer Protocol for the I2C touch panel device.
  //
  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &gEfiAbsolutePointerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &TpDev->AbsolutePointerProtocol
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }
  //
  // Create event for interrupt on completed handler
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  I2cTouchPanelInterruptOnCompleted,
                  TpDev,
                  &TpDev->OnCompleteEvent
                  );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  //
  // calcaute I2C_TP_CHECK_RELEASE_STATE_PERIOD ms
  //
  CpuFreq = GetPerformanceCounterProperties (NULL, NULL);
  TpDev->EventExpiredPeriodTime = DivU64x32 (MultU64x32 (CpuFreq, I2C_TP_EVENT_EXPIRD_PERIOD_TIME), 1000u);

  //
  // Allocate and setup controller name table for ComponentName/2 protocol
  //
  AddUnicodeString2 (
    LANGUAGE_CODE_ENGLISH_ISO639,
    gI2cTouchPanelComponentName.SupportedLanguages,
    &TpDev->ControllerNameTable,
    CONTROLLER_DRIVER_NAME,
    TRUE
    );
  AddUnicodeString2 (
    LANGUAGE_CODE_ENGLISH_RFC4646,
    gI2cTouchPanelComponentName2.SupportedLanguages,
    &TpDev->ControllerNameTable,
    CONTROLLER_DRIVER_NAME,
    FALSE
    );
  //
  // Issue async I2C request for data transfer
  //
  RequestPacket.OperationCount             = 1;
  RequestPacket.Operation[0].Flags         = I2C_FLAG_READ;
  RequestPacket.Operation[0].LengthInBytes = TpDev->HidDescriptor.MaxInputLength;
  RequestPacket.Operation[0].Buffer        = TpDev->ReportData;

  Status = I2cIo->QueueRequest (
                    I2cIo,
                    0,
                    TpDev->OnCompleteEvent,
                    &RequestPacket,
                    &TpDev->TransferResult
                    );
Error:
  if (EFI_ERROR (Status)) {
    FreeResource (TpDev, Controller);
  }
  return Status;
}


/**
  Disconnect from the I2C host controller.

  This routine disconnects from the Touch Panel

  This routine is called by DriverUnload when the Touch Panel
  is being unloaded.

  @param [in] DriverBinding     Protocol instance pointer.
  @param [in] Controller        Handle of device to stop driver on.
  @param [in] NumberOfChildren  How many children need to be stopped.
  @param [in] ChildHandleBuffer Not used.

  @retval EFI_SUCCESS           This driver is removed Controller.
  @retval EFI_DEVICE_ERROR      The device could not be stopped due to a device error.
  @retval other                 This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
I2cTouchPanelDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL       *This,
  IN  EFI_HANDLE                        Controller,
  IN  UINTN                             NumberOfChildren,
  IN  EFI_HANDLE                        *ChildHandleBuffer
  )
{
  EFI_STATUS                    Status;
  I2C_TOUCH_PANEL_DEV           *TpDev;
  EFI_ABSOLUTE_POINTER_PROTOCOL *AbsolutePointerProtocol;
  //
  //  Disconnect any connected drivers and locate the context
  //  structure
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiAbsolutePointerProtocolGuid,
                  (VOID**) &AbsolutePointerProtocol,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TpDev = I2C_TOUCH_PANEL_DEV_FROM_ABSOLUTE_POINTER_PROTOCOL (AbsolutePointerProtocol);
  FreeResource (TpDev, Controller);
  return Status;
}

/**

  Driver binding protocol support

**/
EFI_DRIVER_BINDING_PROTOCOL gI2cTouchPanelDriverBinding = {
  I2cTouchPanelDriverSupported,
  I2cTouchPanelDriverStart,
  I2cTouchPanelDriverStop,
  0x12,
  NULL,
  NULL
};

/**

  Entry point for EFI drivers.

  @param  ImageHandle           EFI_HANDLE
  @param  SystemTable           EFI_SYSTEM_TABLE

  @retval EFI_SUCCESS
  @retval others

**/
EFI_STATUS
EFIAPI
I2cTouchPanelEntryPoint (
  IN EFI_HANDLE                 ImageHandle,
  IN EFI_SYSTEM_TABLE           *SystemTable
  )
{
  //
  // Install binding protocols
  //
  return EfiLibInstallDriverBindingComponentName2 (
           ImageHandle,
           SystemTable,
           &gI2cTouchPanelDriverBinding,
           ImageHandle,
           &gI2cTouchPanelComponentName,
           &gI2cTouchPanelComponentName2
           );
}


/**
  This is the default unload handle for all the I2C tocuh panel drivers.

  Disconnect the driver specified by ImageHandle from all the devices in the handle database.
  Uninstall all the protocols installed in the driver entry point.

  @param[in]  ImageHandle       The drivers' driver image.

  @retval EFI_SUCCESS           The image is unloaded.
  @retval Others                Failed to unload the image.

**/
EFI_STATUS
EFIAPI
I2cTouchPanelUnload (
  IN EFI_HANDLE             ImageHandle
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *DeviceHandleBuffer;
  UINTN                             DeviceHandleCount;
  UINTN                             Index;
  UINTN                             Index2;
  EFI_DRIVER_BINDING_PROTOCOL       *DriverBinding;
  EFI_COMPONENT_NAME_PROTOCOL       *ComponentName;
  EFI_COMPONENT_NAME2_PROTOCOL      *ComponentName2;

  //
  // Get the list of all the handles in the handle database.
  // If there is an error getting the list, then the unload
  // operation fails.
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &DeviceHandleCount,
                  &DeviceHandleBuffer
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiDriverBindingProtocolGuid,
                    (VOID **) &DriverBinding
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (DriverBinding->ImageHandle != ImageHandle) {
      continue;
    }

    //
    // Disconnect the driver specified by ImageHandle from all
    // the devices in the handle database.
    //
    for (Index2 = 0; Index2 < DeviceHandleCount; Index2++) {
      Status = gBS->DisconnectController (
                      DeviceHandleBuffer[Index2],
                      DriverBinding->DriverBindingHandle,
                      NULL
                      );
    }

    //
    // Uninstall all the protocols installed in the driver entry point
    //
    gBS->UninstallProtocolInterface (
           DriverBinding->DriverBindingHandle,
           &gEfiDriverBindingProtocolGuid,
           DriverBinding
           );

    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiComponentNameProtocolGuid,
                    (VOID **) &ComponentName
                    );
    if (!EFI_ERROR (Status)) {
      gBS->UninstallProtocolInterface (
             DriverBinding->DriverBindingHandle,
             &gEfiComponentNameProtocolGuid,
             ComponentName
             );
    }

    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiComponentName2ProtocolGuid,
                    (VOID **) &ComponentName2
                    );
    if (!EFI_ERROR (Status)) {
      gBS->UninstallProtocolInterface (
             DriverBinding->DriverBindingHandle,
             &gEfiComponentName2ProtocolGuid,
             ComponentName2
             );
    }
  }

  //
  // Free the buffer containing the list of handles from the handle database
  //
  if (DeviceHandleBuffer != NULL) {
    FreePool (DeviceHandleBuffer);
  }

  return EFI_SUCCESS;
}

