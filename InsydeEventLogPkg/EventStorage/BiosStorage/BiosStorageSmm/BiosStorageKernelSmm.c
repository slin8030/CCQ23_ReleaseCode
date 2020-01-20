/** @file

  BIOS Storage Kernel SMM implementation.

;******************************************************************************
;* Copyright (c) 2014-2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/ 

#include <BiosStorageKernelSmm.h>

EFI_EVENT_LOG_INSTANCE                *SmmELPrivate;

/**
 Smart detect current flash device type

 @param[in]         This                
 @param[in]         Buffer           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
FlashFdDetectDevice (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  OUT UINT8                             *Buffer
  )
{
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *ELPrivate;

  ELPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  Status = GetFlashDevice (&(ELPrivate->DevicePtr));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Buffer != NULL) {
    CopyMem (Buffer, ELPrivate->DevicePtr, sizeof (FLASH_DEVICE));
  }
  
  return Status;
}

/**
 Get the HOB of Event log.

 @param[in]         This                          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventLogHob (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL      *This
  )
{
  VOID                                 *HobList;
  VOID                                 *Buffer;
  EFI_STATUS                           Status;
  BIOS_EVENT_LOG_ORGANIZATION          *HobLog;
  BIOS_EVENT_LOG_ORGANIZATION          *CurrentLog;
  EFI_EVENT_LOG_INSTANCE               *EPrivate;
  UINTN                                 Length;

  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);
  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);

    if (((UINTN)CurrentLog > (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength) ||
        (UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase)) ||
        (CurrentLog->Length == 0)) {

      return EFI_SUCCESS;
    }
  }

  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, &HobList);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (;;) {
//[-start-171212-IB08400542-modify]//
    Buffer = GetNextGuidHob (&gH2OBiosEventLogHobGuid, HobList);
//[-end-171212-IB08400542-modify]//
    if (Buffer == NULL) {
      break;
    }    
    HobLog = (BIOS_EVENT_LOG_ORGANIZATION *)((UINT8 *)Buffer + sizeof (EFI_HOB_GUID_TYPE));
    Length = (UINTN)HobLog->Length;
    Status = FlashProgram (
                          (UINT8 *)(UINTN)CurrentLog,
                          Buffer,
                          &Length,
                          (((UINTN)CurrentLog) & ~(0xFFFF))
                          );

    if (EFI_ERROR (Status) ) {
      return Status;
    }
    
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *) ((UINT8*) CurrentLog + Length);
    if (((UINTN)CurrentLog > (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength) ||
        (UINTN)CurrentLog  < (UINTN)(EPrivate->GPNVBase)) ||
        (CurrentLog->Length == 0)) {
      return EFI_SUCCESS;
    }
  }

  return EFI_SUCCESS;
}

/**
 Read function of Event log.

 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiReadNextEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL         *This,
  IN OUT VOID                             **EventListAddress
  )
{
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  BIOS_EVENT_LOG_ORGANIZATION           *CurrentLog;


  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  //
  //Current Log is first log
  //
  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  //
  //Check EventListAddress.
  //If it is NULL, and the first log exist, return first Log
  //
  if (*EventListAddress == NULL) {
    if ((CurrentLog->Type == 0xFF) && (CurrentLog->Length == 0xFF)) {

      return EFI_NOT_FOUND;
    }
    *EventListAddress = (UINT8 *)(UINTN)EPrivate->GPNVBase;
    return EFI_SUCCESS;
  }

  //
  //Check current log
  //
  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)*EventListAddress;

  //
  //If the EventListAddress out of range, return error
  //
  if (((UINTN)CurrentLog > (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength) ||
      (UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase)) ||
      (CurrentLog->Length == 0)){

      return EFI_INVALID_PARAMETER;
  }

  //
  //If the EventListAddress point to the NULL, return
  //
  if ((CurrentLog->Type == 0xFF) && (CurrentLog->Length == 0xFF)) {

    return EFI_NOT_FOUND;
  }

  //
  //Find out and point to the next Event Log
  //
  *EventListAddress = (UINT8 *)( (UINT8*)CurrentLog + CurrentLog->Length);

  return EFI_SUCCESS;

}

/**
 Write function of Event log.s          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiWriteEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  UINT8                             EventLogType,
  IN  UINT32                            PostBitmap1,
  IN  UINT32                            PostBitmap2,
  IN  UINTN                             OptionDataSize,
  IN  UINT8                             *OptionLogData
  )
{
  UINT8                                 *Src;
  UINT8                                 *Dest;
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  UINTN                                 WriteAddress;
  BIOS_EVENT_LOG_ORGANIZATION           *CurrentLog;
  BIOS_EVENT_LOG_ORGANIZATION           *Buffer;
  UINTN                                 NumBytes;
  UINTN                                 DataFormatTypeLength;
  EL_TIME                             ElTime;

  if ((OptionDataSize == 0) || (OptionLogData == NULL)) {
    OptionDataSize = 0;
    OptionLogData = NULL;
  }

  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {
    if (((UINTN)CurrentLog + CurrentLog->Length + BIOS_EVENT_LOG_BASE_LENGTH + OptionDataSize) >= 
         (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength)) {
      //
      // Event Full
      //
      return EFI_OUT_OF_RESOURCES;
    }

    if ((UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase) ||
        (CurrentLog->Length == 0)) {

      return EFI_UNSUPPORTED;
    }
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);
  }

  DataFormatTypeLength = 0;
  switch (EventLogType) {

  case EfiEventLogTypePostError:
    DataFormatTypeLength = BIOS_EVENT_LOG_DATA_FORMAT_POST_RESULT_BITMAP_LENGTH;
    break;

  default:
    DataFormatTypeLength = BIOS_EVENT_LOG_DATA_FORMAT_NONE_LENGTH;
    break;
  }
  NumBytes = BIOS_EVENT_LOG_BASE_LENGTH + DataFormatTypeLength + OptionDataSize;
  if (gSmst == NULL) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    NumBytes,
                    (VOID **)&Buffer
                    );
  } else {
    Status = gSmst->SmmAllocatePool(
                      EfiRuntimeServicesData,
                      NumBytes,
                      (VOID **)&Buffer
                      );
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log allocate pool failed: %r\n", Status));
    return Status;
  }

  ZeroMem (Buffer, NumBytes);

  Status = EventLogGetTimeFromCmos (&ElTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Buffer->Year   = ElTime.Year;
  Buffer->Month  = ElTime.Month;
  Buffer->Day    = ElTime.Day;
  Buffer->Hour   = ElTime.Hour;
  Buffer->Minute = ElTime.Minute;
  Buffer->Second = ElTime.Second;

  Buffer->Type   = EventLogType;
  Buffer->Length = (UINT8)NumBytes;

  if (EventLogType == EfiEventLogTypePostError) {
    *(UINT32 *)(Buffer->Data)            = PostBitmap1;
    *(UINT32 *)(UINTN)(&Buffer->Data[4]) = PostBitmap2;
  }

  if (OptionLogData!= NULL) {
    CopyMem ((UINT8*)((UINTN)Buffer+NumBytes-OptionDataSize), OptionLogData, OptionDataSize);
  }

  WriteAddress = (UINTN)CurrentLog;
  Dest = (UINT8 *)(UINTN)WriteAddress;
  Src = (UINT8 *)(UINTN)Buffer;

  Status = FlashProgram (
                        Dest,
                        Src,
                        &NumBytes,
                        (((UINTN)CurrentLog) & ~(0xFFFF))
                        );

  //
  //  !!WORKARUOUD!! - Force Cache update
  //
  *Dest = 0;
  
  if (gSmst == NULL) {
    gBS->FreePool(Buffer);
  } else {
    gSmst->SmmFreePool (Buffer);
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Write Event Event failed: %r\n", Status));
  }

  return Status;

}

/**
 Clear function of Event log.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiClearEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This
  )
{
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  UINT8                                 *Buffer;
  UINT32                                i;
  UINTN                                 BlockSize;
  UINTN                                 FdSupportEraseSize;
  UINTN                                 EraseCount;
  UINTN                                 EraseStartAddress;

  EPrivate  = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  FdSupportEraseSize = GetFlashBlockSize();  
  
  EraseCount = GET_ERASE_SECTOR_NUM ( 
                                 EPrivate->GPNVBase,
                                 EPrivate->GPNVLength,
                                 EPrivate->GPNVBase & ~(FdSupportEraseSize - 1),
                                 FdSupportEraseSize
                                 );
  EraseStartAddress = EPrivate->GPNVBase & ~(FdSupportEraseSize - 1);
  
  BlockSize = FdSupportEraseSize * EraseCount;

  if (gSmst == NULL) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    BlockSize,
                    (VOID **)&Buffer
                    );
  } else {
    Status = gSmst->SmmAllocatePool(
                      EfiRuntimeServicesData,
                      BlockSize,
                      (VOID **)&Buffer
                      );
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log allocate pool failed: %r\n", Status));
    return Status;
  }

  //
  //Copy whole block data to buffer
  //
  CopyMem(
    Buffer,
    (VOID *)(UINTN)(EPrivate->GPNVBase & (~(FdSupportEraseSize - 1))),
    BlockSize
    );

  //
  //Copy modified GPNV data to buffer
  //
  for (i=0;i<EPrivate->GPNVLength;i++) {
    *(UINT8 *)((Buffer + (EPrivate->GPNVBase) - (EPrivate->GPNVBase & ((UINT32)(~0)) & (~(FdSupportEraseSize - 1))))+i) = 0xFF;
  }

  //
  //Flash GPNV
  //
//[-start-161109-IB04560704-modify]//
  for (i = 0; i < EraseCount; i++) {
    
    Status = FlashErase (
                         (EPrivate->GPNVBase & ((UINT32)(~0)) & (~(FdSupportEraseSize - 1))) + FdSupportEraseSize * i,
                         FdSupportEraseSize
                         );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Event Log FlashErase() failed: %r\n", Status));
      return Status;
    }
  }

  Status = FlashProgram (
                        (UINT8 *)(UINTN)(EPrivate->GPNVBase & ((~(FdSupportEraseSize - 1)))),
                        Buffer,
                        &BlockSize,
                        ((EPrivate->GPNVBase & (UINT32)(~0)) & ~(FdSupportEraseSize - 1))
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log FlashProgram() failed: %r\n", Status));
    return Status;
  }  
//[-end-161109-IB04560704-modify]//
  //
  //  !!WORKARUOUD!! - Force Cache update
  //
  AsmWbinvd ();
  if (gSmst == NULL) {
    gBS->FreePool(Buffer);
  } else {
    gSmst->SmmFreePool (Buffer);
  }

  Status = EPrivate->EventLogService.Write(
                                       &EPrivate->EventLogService,
                                       EfiEventLogTypeLogAreaResetCleared,
                                       0,
                                       0,
                                       0,
                                       NULL
                                       );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Event Log write data failed: %r\n", Status));
  }

  return Status;
}

/**
 Over Write function of Event Log.

 @param[in]         This                
 @param[in]         SelId               
 @param[out]        SelData             
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiOverWriteEventLog (
  IN  EFI_BIOS_EVENT_LOG_PROTOCOL       *This,
  IN  BIOS_EVENT_LOG_ORGANIZATION       *InputBuffer
  )
{
  UINT8                                 *Src;
  UINT8                                 *Dest;
  EFI_STATUS                            Status;
  EFI_EVENT_LOG_INSTANCE                *EPrivate;
  UINTN                                 WriteAddress;
  BIOS_EVENT_LOG_ORGANIZATION           *CurrentLog;
  BIOS_EVENT_LOG_ORGANIZATION           *Buffer;
  UINTN                                 NumBytes;
  UINTN                                 BaseLenght;
  
  if ((This == NULL) || (InputBuffer == NULL)) {
    return EFI_UNSUPPORTED;
  }

  EPrivate = INSTANCE_FROM_EFI_EVENT_LOG_THIS (This);

  CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)(UINTN)EPrivate->GPNVBase;

  while ((CurrentLog->Type != 0xFF) && (CurrentLog->Length != 0xFF)) {

    if (((UINTN)CurrentLog + CurrentLog->Length + InputBuffer->Length) >= 
        (UINTN)(EPrivate->GPNVBase + EPrivate->GPNVLength)) {
      //
      // Event Full
      //
      return EFI_OUT_OF_RESOURCES;
    }

    if ((UINTN)CurrentLog< (UINTN)(EPrivate->GPNVBase) ||
        (CurrentLog->Length == 0)) {

      return EFI_UNSUPPORTED;
    }
    CurrentLog = (BIOS_EVENT_LOG_ORGANIZATION *)( (UINT8*)CurrentLog + CurrentLog->Length);
  }

  NumBytes = InputBuffer->Length;

  if (gSmst == NULL) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    NumBytes,
                    (VOID **)&Buffer
                    );
  } else {
    Status = gSmst->SmmAllocatePool(
                      EfiRuntimeServicesData,
                      NumBytes,
                      (VOID **)&Buffer
                      );
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (Buffer, NumBytes);

  Buffer->Year   = InputBuffer->Year;
  Buffer->Month  = InputBuffer->Month;
  Buffer->Day    = InputBuffer->Day;
  Buffer->Hour   = InputBuffer->Hour;
  Buffer->Minute = InputBuffer->Minute;
  Buffer->Second = InputBuffer->Second;

  Buffer->Type   = InputBuffer->Type;;
  Buffer->Length = InputBuffer->Length;;

  BaseLenght = BIOS_EVENT_LOG_BASE_LENGTH;
  
  if (InputBuffer->Length > BIOS_EVENT_LOG_BASE_LENGTH) {
    CopyMem ((UINT8*)((UINTN)Buffer+NumBytes-(InputBuffer->Length - BaseLenght)), 
                &InputBuffer->Data[0], 
                InputBuffer->Length - BaseLenght);
  }

  WriteAddress = (UINTN)CurrentLog;
  Dest = (UINT8 *)(UINTN)WriteAddress;
  Src = (UINT8 *)(UINTN)Buffer;

  Status = FlashProgram (
                      Dest,
                      Src,
                      &NumBytes,
                      (WriteAddress & ~(0xFFFF))
                      );
  //
  //  !!WORKARUOUD!! - Force Cache update
  //
  *Dest = 0;
  
  if (gSmst == NULL) {
    gBS->FreePool(Buffer);
  } else {
    gSmst->SmmFreePool (Buffer);
  }

  return Status;

}

/**
 A function for Clear and Write event log in SMM via SW SMI.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EventLogToSmm (
  IN     EFI_HANDLE                     DispatchHandle,
  IN     CONST VOID                     *RegisterContext,
  IN OUT VOID                           *CommBuffer,
  IN OUT UINTN                          *CommBufferSize
)
{
  EFI_STATUS                            Status;
  EFI_BIOS_EVENT_LOG_PROTOCOL           *EventLog;  
  EFI_EVENT_LOG_BUF                     *EventLogBuf;
  
  EventLogBuf = (EFI_EVENT_LOG_BUF *)(UINTN) CommBuffer;

//[-start-171212-IB08400542-modify]//
  Status = gSmst->SmmLocateProtocol (&gH2OBiosSmmEventLogProtocolGuid, NULL, (VOID **)&EventLog);
//[-end-171212-IB08400542-modify]//
  if (EFI_ERROR (Status)) {
    EventLogBuf->ReturnStatus = Status;
    return EFI_SUCCESS;
  }
  
  switch (EventLogBuf->ActionType) {
  case EVENT_LOG_CLEAR_LOG:
    Status = EfiClearEventLog (EventLog);
    EventLogBuf->ReturnStatus = Status;
    break;
  case EVENT_LOG_WRITE_LOG:
    Status = EfiWriteEventLog (
                  EventLog, 
                  EventLogBuf->EventLogType, 
                  EventLogBuf->PostBitmap1, 
                  EventLogBuf->PostBitmap2, 
                  EventLogBuf->OptionDataSize, 
                  EventLogBuf->OptionLogData
                  );
    EventLogBuf->ReturnStatus = Status;
    break;
  case EVENT_LOG_OVERWRITE_LOG:
    Status = EfiOverWriteEventLog(EventLog, (BIOS_EVENT_LOG_ORGANIZATION *)EventLogBuf->RawData); 
    EventLogBuf->ReturnStatus = Status;
    break;
  default:
    EventLogBuf->ReturnStatus = EFI_UNSUPPORTED;
    break;
  }

  return EFI_SUCCESS;
}

/**
 Entry point of this driver. Install Event Log protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
BiosStorageKernelEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                          Status;
  EFI_SMM_SW_DISPATCH2_PROTOCOL       *SwDispatch;
  EFI_HANDLE                          DispatchHandle;

  //
  // Make sure the NV space is available for BIOS Storage.
  // 
//[-start-171212-IB08400542-modify]//
  if ((!PcdGet32 (PcdH2OBiosEventStorageBase)) || (!PcdGet32 (PcdH2OBiosEventStorageSize))) {
    return EFI_NOT_FOUND;
  }
//[-end-171212-IB08400542-modify]//

  Status = gSmst->SmmAllocatePool (
                       EfiRuntimeServicesData,
                       sizeof (EFI_EVENT_LOG_INSTANCE),
                       (VOID **)&SmmELPrivate
                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SmmELPrivate->Signature                      = EVENT_LOG_SIGNATURE;
  SmmELPrivate->DevicePtr                      = NULL;
//[-start-171212-IB08400542-modify]//
  SmmELPrivate->GPNVBase                       = PcdGet32 (PcdH2OBiosEventStorageBase);
  SmmELPrivate->GPNVLength                     = PcdGet32 (PcdH2OBiosEventStorageSize);
//[-end-171212-IB08400542-modify]//
  SmmELPrivate->EventLogService.Write          = EfiWriteEventLog;
  SmmELPrivate->EventLogService.Clear          = EfiClearEventLog;
  SmmELPrivate->EventLogService.ReadNext       = EfiReadNextEventLog;
  SmmELPrivate->EventLogService.DetectDevice   = FlashFdDetectDevice;
  SmmELPrivate->EventLogService.OverWrite      = EfiOverWriteEventLog;
  
  SmmELPrivate->Handle = NULL;
//[-start-171212-IB08400542-modify]//
  Status = gSmst->SmmInstallProtocolInterface (
                                           &SmmELPrivate->Handle,
                                           &gH2OBiosSmmEventLogProtocolGuid,
                                           EFI_NATIVE_INTERFACE,
                                           &(SmmELPrivate->EventLogService)
                                           );
//[-end-171212-IB08400542-modify]//

  if (!EFI_ERROR (Status)) {
    if (SmmELPrivate->DevicePtr == NULL) {
      Status = FlashFdDetectDevice (&SmmELPrivate->EventLogService, NULL);
      ASSERT_EFI_ERROR (Status);
    }
    //
    // Register an API for Clear and Write Event log with SW SMI callback function(0x99).
    //
    Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID **)&SwDispatch);
    if (EFI_ERROR (Status)) {
      return Status;
    } else {
      //
      // For Bios Storage function in SMM after exiting Boot Service.
      //
      Status = gSmst->SmiHandlerRegister (
                        EventLogToSmm,
                        &gH2OBiosStorageCommunicationGuid,
                        &DispatchHandle
                        );
      if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_ERROR, "Register EventLogToSmm(): %r\n", Status));
      } else {
//[-start-180724-IB08400617-modify]//
//[-start-180823-IB07400999-modify]//
        Status = PcdSetBool (PcdH2OSmmBiosStorageSupportReady, TRUE);
//[-end-180823-IB07400999-modify]//
        if (EFI_ERROR (Status)) {
          DEBUG((EFI_D_ERROR, "Set PcdH2OSmmBiosStorageSupportReady failure: %r\n", Status));
          return Status;
        }
//[-end-180724-IB08400617-modify]//
      }  
    }
  }

  return EFI_SUCCESS;
}

