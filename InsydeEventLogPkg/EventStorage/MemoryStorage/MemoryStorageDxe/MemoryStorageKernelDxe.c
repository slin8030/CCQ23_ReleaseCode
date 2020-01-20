/** @file

  Memory Storage Kernel DXE implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <MemoryStorageKernelDxe.h>

EFI_EVENT                             gReadyToBootEvent;
//
// Memory Storage Area Address, included Head size
//
EFI_PHYSICAL_ADDRESS                  gMemStorageHeadAddr;
//
// Memory Storage Area Address, NO included Head size
//
EFI_PHYSICAL_ADDRESS                  gMemStorageEntryAddr;
EFI_EVENT                             mVarEvent;

EFI_MEMORY_STORAGE_PROTOCOL           gMemoryStorageProtocol = {
                                                          WriteEventToMemStorage,
                                                          EfiClearMemStorageEvent,
                                                          EfiReadNextMemStorageEvent,
                                                          OverWriteEventToMemStorage
                                                          };

/**
 
 Notification function when Variable Architecture Protocol Installed
            
 @param[in] Event   - The Event that is being processed
 @param[in] Context - Event Context
  
 @retval EFI Status                  
*/
VOID
EFIAPI
InstalledVarNotificationFunction (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  )
{
  EFI_STATUS                              Status;
  H2O_EVENT_STORAGE_VARIABLE              H2OEventStorageVar;  
  UINTN                                   VarSize;
  CHAR16                                  *VarName = H2O_EVENT_STORAGE_VARIABLE_NAME;
  
  //
  // Store "Memory Storage Head Address" for SMM driver to refer.
  //
  gRT->SetVariable (
                L"MemoryStorage",
                &gH2OMemoryStorageVariableGuid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof (EFI_PHYSICAL_ADDRESS),
                &gMemStorageHeadAddr
                );

  VarSize = sizeof(H2O_EVENT_STORAGE_VARIABLE);
  Status = gRT->GetVariable (
                         H2O_EVENT_STORAGE_VARIABLE_NAME,
                         &gH2OEventStorageVariableGuid,
                         NULL,
                         &VarSize,
                         &H2OEventStorageVar
                         );
//[-start-160202-IB05080551-modify]//
  if (!EFI_ERROR (Status)) { 
    H2OEventStorageVar.MemoryEventStorageExist = TRUE;
    H2OEventStorageVar.MemoryEventStorageBaseAddress = (UINT32) gMemStorageHeadAddr;
    H2OEventStorageVar.MemoryEventStorageLength = (UINT32)MEMORY_STORAGE_SIZE;
  
    VarSize = sizeof(H2O_EVENT_STORAGE_VARIABLE);
    gRT->SetVariable (
           VarName,
           &gH2OEventStorageVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           VarSize,
           &H2OEventStorageVar
           );  
  }
//[-end-160202-IB05080551-modify]//
  gBS->CloseEvent (mVarEvent);

  return;

}

//[-start-180729-IB08400617-add]//
//[-start-180912-IB07401006-modify]//
void *
//[-end-180912-IB07401006-modify]//
DummyPointerConvert (
  IN void *Ptr
  )
{
  return Ptr;
}

//[-end-180729-IB08400617-add]//

/**
 Get F-Segment Table Address

 @param[in]         Size - Require Size
 
 @retval EFI Status                  
*/
UINT32
EFIAPI
GetFSegmentTableAddr (
  IN  UINT16      Size
  ) 
{
  EFI_STATUS                  Status;
  EFI_LEGACY_BIOS_PROTOCOL    *LegacyBios;
//[-start-171207-IB08400539-modify]//
  EFI_LEGACY_REGION2_PROTOCOL  *LegacyRegion2;
//[-end-171207-IB08400539-modify]//
  EFI_IA32_REGISTER_SET       Regs;
  EFI_COMPATIBILITY16_TABLE   *Table;
  UINT32                      BufferAddr;
  UINT8                       *Ptr;
//[-start-180729-IB08400617-add]//
  UINT32                      *Ptr32;
//[-end-180729-IB08400617-add]//

  Table = NULL;
  BufferAddr = 0;

  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&LegacyBios);
  if(EFI_ERROR (Status)) {
    return 0;
  }
  
//[-start-171207-IB08400539-modify]//
  Status = gBS->LocateProtocol(&gEfiLegacyRegion2ProtocolGuid, NULL, (VOID **)&LegacyRegion2);
  if(EFI_ERROR (Status)) {
    return 0;
  }
//[-end-171207-IB08400539-modify]//

  for (Ptr = (UINT8 *)((UINTN)0xFE000); Ptr < (UINT8 *) ((UINTN) 0x100000); Ptr += 0x10) {
//[-start-180724-IB08400617-modify]//
    if (Ptr != NULL) {
      Ptr32 = DummyPointerConvert (Ptr);
      if (*Ptr32 == SIGNATURE_32 ('I', 'F', 'E', '$')) {
        Table = (EFI_COMPATIBILITY16_TABLE *) Ptr;
        break; 
      }
    } 
//[-end-180724-IB08400617-modify]//
  }

  if(Table == NULL) {
    return 0;
  }

//[-start-171207-IB08400539-modify]//
  LegacyRegion2->UnLock (LegacyRegion2, 0xE0000, 0x20000, NULL);
//[-end-171207-IB08400539-modify]//

  ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));     // aligned in 0x10 boundry
  Regs.X.AX = 0x06;                                    // Legacy16GetTableAddress    allocate legacy memory
  Regs.X.BX = 0x01;                                    // memory resident in F000 segment
  Regs.X.CX = (UINT16) Size;                           // allocate size
  Status = LegacyBios->FarCall86 (
                              LegacyBios,
                              Table->Compatibility16CallSegment,
                              Table->Compatibility16CallOffset,
                              &Regs,
                              NULL,
                              0
                              );

  BufferAddr = (UINT32) (Regs.X.DS *16 + Regs.X.BX);

//[-start-171207-IB08400539-modify]//
  LegacyRegion2->Lock (LegacyRegion2, 0xE0000, 0x20000, NULL);
//[-end-171207-IB08400539-modify]//

  return BufferAddr;
}

/**
 Publish Memory Storage Signature to F-Segment

 @param[in] Event   - The Event that is being processed
 @param[in] Context - Event Context           
 
 @retval None                  
*/
VOID
EFIAPI
PublishMemStorageSignature (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )  
{
  EFI_STATUS                              Status;
  MEMORY_STORAGE_F_SEGMENT_STRUCTURE      *MemoryStorageEntry;
//[-start-171207-IB08400539-modify]//
  EFI_LEGACY_REGION2_PROTOCOL              *LegacyRegion2;
//[-end-171207-IB08400539-modify]//

//[-start-171207-IB08400539-modify]//
  Status = gBS->LocateProtocol(&gEfiLegacyRegion2ProtocolGuid, NULL, (VOID **)&LegacyRegion2);
  if(EFI_ERROR (Status)) {
    return;
  }
//[-end-171207-IB08400539-modify]//

  //
  // 1. Create a "_MEMS_" signature on F-Segment to point prior allocated reserved memory area(32Kb).
  // 
  MemoryStorageEntry = 0;
  MemoryStorageEntry = (MEMORY_STORAGE_F_SEGMENT_STRUCTURE *)(UINTN)GetFSegmentTableAddr (sizeof(MEMORY_STORAGE_F_SEGMENT_STRUCTURE));
  if(MemoryStorageEntry != 0) {
//[-start-171207-IB08400539-modify]//
    LegacyRegion2->UnLock (LegacyRegion2, 0xE0000, 0x20000, NULL);
    MemoryStorageEntry->MemStorageSignature = MEMORY_STORAGE_INSTANCE_SIGNATURE;
    MemoryStorageEntry->MemStorageHeadAddr = gMemStorageHeadAddr;
    LegacyRegion2->Lock (LegacyRegion2, 0xE0000, 0x20000, NULL);
//[-end-171207-IB08400539-modify]//
  }
  //
  // 2. Close Evnet
  //
  gBS->CloseEvent(gReadyToBootEvent);

}

/**
 Write (Log) Event to Memory Storage
            
 @param[in] SensorType      - Event Sensor Type
 @param[in] SensorNum       - Event Sensor Number
 @param[in] EventType       - Event Event Type
 @param[in] OptionDataSize  - Option Data Size
 @param[in] *OptionLogData  - Option Log Data
  
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
WriteEventToMemStorage (
  IN  UINT8                             SensorType,
  IN  UINT8                             SensorNum,
  IN  UINT8                             EventType,
  IN  UINTN                             OptionDataSize,
  IN  UINT8                             *OptionLogData
  )     
{
  EFI_STATUS                            Status;
  MEMORY_STORAGE_ORGANIZATION           *CurrentLog;
  MEMORY_STORAGE_ORGANIZATION           *Buffer;
  UINTN                                 NumBytes;
  EFI_PHYSICAL_ADDRESS                  *MemStorageCurrentLogAddr;
  EL_TIME                               ElTime;

  if ((OptionDataSize == 0) || (OptionLogData == NULL)) {
    OptionDataSize = 0;
    OptionLogData = NULL;
  }

  MemStorageCurrentLogAddr = (EFI_PHYSICAL_ADDRESS *)((UINTN)gMemStorageHeadAddr + sizeof(gMemStorageHeadAddr));
  CurrentLog = (MEMORY_STORAGE_ORGANIZATION *) *MemStorageCurrentLogAddr;

  if (((UINTN)CurrentLog + CurrentLog->Length + MEMORY_STORAGE_BASE_LENGTH + OptionDataSize) >= 
       (UINTN)(gMemStorageEntryAddr + MEMORY_STORAGE_SIZE - sizeof(MEMORY_STORAGE_HEAD_STRUCTURE))) {
    //
    // Event Full
    //
    return EFI_OUT_OF_RESOURCES;
  }

  NumBytes = MEMORY_STORAGE_BASE_LENGTH + OptionDataSize;
  

  Status = gBS->AllocatePool (
                          EfiRuntimeServicesData,
                          NumBytes,
                          (VOID **)&Buffer
                          );
  if (EFI_ERROR (Status)) {
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

  Buffer->SensorType = SensorType;
  Buffer->SensorNum  = SensorNum;
  Buffer->EventType  = EventType;
  Buffer->Length     = (UINT8)NumBytes;

  if (OptionLogData!= NULL) {
    CopyMem ((UINT8*)((UINTN)Buffer+NumBytes-OptionDataSize), OptionLogData, OptionDataSize);
  }

  //
  // Copy Event to Memory Storage
  //
  CopyMem ((UINT8*)((MEMORY_STORAGE_ORGANIZATION *)CurrentLog), (UINT8*)((MEMORY_STORAGE_ORGANIZATION *)Buffer), NumBytes);
  *MemStorageCurrentLogAddr =  (EFI_PHYSICAL_ADDRESS)((UINTN)CurrentLog + NumBytes);

  AsmWbinvd ();

  Status = gBS->FreePool(Buffer);

  return Status;

}

/**
 Clear Event from Memory Storage          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiClearMemStorageEvent (
  VOID
  )       
{
  EFI_PHYSICAL_ADDRESS                  *MemStorageCurrentLogAddr;

  //
  // Set Memory Storage Area data to zero. DON'T clear Memory Storage Head Information
  //
  ZeroMem ((VOID *)gMemStorageEntryAddr, MEMORY_STORAGE_SIZE-sizeof(MEMORY_STORAGE_HEAD_STRUCTURE));
  //
  // Reset the Current Log Address of Memory Storage.
  //
  MemStorageCurrentLogAddr = (EFI_PHYSICAL_ADDRESS *)((UINTN)gMemStorageHeadAddr + sizeof(gMemStorageHeadAddr));
  *MemStorageCurrentLogAddr = (EFI_PHYSICAL_ADDRESS)gMemStorageEntryAddr; 
  AsmWbinvd ();

  return EFI_SUCCESS;
}

/**
 Read Next Event from Memory Storage

 @param[in]         MemStorageListAddress - Next Event Address            
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EfiReadNextMemStorageEvent (
  IN OUT VOID                             **MemStorageListAddress
  )
{
  MEMORY_STORAGE_ORGANIZATION                *CurrentLog;
  MEMORY_STORAGE_HEAD_STRUCTURE              *MemStorageHead = NULL;

  //
  // Current Log is first log
  //
  CurrentLog = (MEMORY_STORAGE_ORGANIZATION *)(EFI_PHYSICAL_ADDRESS)gMemStorageEntryAddr;

  //
  // Check EventListAddress.
  // If it is NULL, and the first log exist, return first Log
  //
  if (*MemStorageListAddress == NULL) {
    if ((CurrentLog->SensorType == 0x00) && 
        (CurrentLog->SensorNum  == 0x00) &&
        (CurrentLog->EventType  == 0x00)) {

      return EFI_NOT_FOUND;
    }
    
    *MemStorageListAddress = (UINT8 *)(UINTN)gMemStorageEntryAddr;

    return EFI_SUCCESS;
  }
  
  //
  //Check current log
  //
  CurrentLog = (MEMORY_STORAGE_ORGANIZATION *)*MemStorageListAddress;
  
  MemStorageHead = (MEMORY_STORAGE_HEAD_STRUCTURE *)(gMemStorageEntryAddr - sizeof(MEMORY_STORAGE_HEAD_STRUCTURE));
  if (((UINTN)((UINTN)CurrentLog + CurrentLog->Length)) >= (UINTN)(MemStorageHead->MemStorageCurrentLogAddr)) {
    return EFI_NOT_FOUND;
  }    
  
  //
  // If the MemStorageListAddress out of range, return error
  //
  if ((((UINTN)CurrentLog + CurrentLog->Length) >= (UINTN)(gMemStorageEntryAddr + (MEMORY_STORAGE_SIZE -sizeof(MEMORY_STORAGE_HEAD_STRUCTURE)))) ||
      ((UINTN)CurrentLog < (UINTN)(gMemStorageEntryAddr)) ||
      (CurrentLog->Length == 0)){  
    return EFI_UNSUPPORTED;
  }

  //
  // If the MemStorageListAddress had not event exist, return
  //
  if ((CurrentLog->SensorType == 0x00) && 
      (CurrentLog->SensorNum  == 0x00) &&
      (CurrentLog->EventType  == 0x00)) {
  
    return EFI_NOT_FOUND;
  }

  //
  // Find out and point to the next Event Log
  //
  *MemStorageListAddress = (UINT8 *)( (UINT8*)CurrentLog + CurrentLog->Length);

  return EFI_SUCCESS;

}

/**
 Over Write (Log) Event to Memory Storage

 @param[in]         InputBuffer    - All Event Data format which included timestamped.           
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
OverWriteEventToMemStorage (
  IN  MEMORY_STORAGE_ORGANIZATION             *InputBuffer
  )
{
  EFI_STATUS                            Status;
  MEMORY_STORAGE_ORGANIZATION           *CurrentLog;
  MEMORY_STORAGE_ORGANIZATION           *Buffer;
  UINTN                                 NumBytes;
  UINTN                                 BaseLenght;
  EFI_PHYSICAL_ADDRESS                  *MemStorageCurrentLogAddr;

  if ((InputBuffer == NULL)) {
    return EFI_UNSUPPORTED;
  }

  MemStorageCurrentLogAddr = (EFI_PHYSICAL_ADDRESS *)((UINTN)gMemStorageHeadAddr + sizeof(gMemStorageHeadAddr));
  CurrentLog = (MEMORY_STORAGE_ORGANIZATION *) *MemStorageCurrentLogAddr;
  if (((UINTN)CurrentLog + CurrentLog->Length + InputBuffer->Length) >= 
      (UINTN)(gMemStorageEntryAddr + MEMORY_STORAGE_SIZE - sizeof(MEMORY_STORAGE_HEAD_STRUCTURE))) {
    //
    // Event Full
    //
    return EFI_OUT_OF_RESOURCES;
  }

  NumBytes = InputBuffer->Length;

  Status = gBS->AllocatePool (
                          EfiRuntimeServicesData,
                          NumBytes,
                          (VOID **)&Buffer
                          );
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

  Buffer->SensorType = InputBuffer->SensorType;
  Buffer->SensorNum  = InputBuffer->SensorNum;
  Buffer->EventType  = InputBuffer->EventType;
  Buffer->Length     = InputBuffer->Length;

  BaseLenght = MEMORY_STORAGE_BASE_LENGTH;

  if (InputBuffer->Length > MEMORY_STORAGE_BASE_LENGTH) {
    CopyMem ((UINT8*)((UINTN)Buffer+NumBytes-(InputBuffer->Length - BaseLenght)), 
                &InputBuffer->Data[0], 
                InputBuffer->Length - BaseLenght);
  }

  //
  // Copy Event to Memory Storage
  //
  CopyMem ((UINT8*)((MEMORY_STORAGE_ORGANIZATION *)CurrentLog), (UINT8*)((MEMORY_STORAGE_ORGANIZATION *)Buffer), NumBytes);
  *MemStorageCurrentLogAddr =  (EFI_PHYSICAL_ADDRESS)((UINTN)CurrentLog + NumBytes);
  
  Status = gBS->FreePool(Buffer);


  return Status;

}


/**
 Memory Storage Kernel Initial Entry.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
MemoryStorageKernelDxeEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                              Status;
  EFI_HANDLE                              Handle;
  UINTN                                   TableSize;
  VOID                                    *EventRegistration; 
  EFI_PHYSICAL_ADDRESS                    MemStorageBuffer;  
  MEMORY_STORAGE_HEAD_STRUCTURE           *MemoryStorageHead;

  Handle = NULL;  
  MemoryStorageHead = NULL;
  //
  // Allocate reserved memory for Memory Storage.
  //
  MemStorageBuffer = 0xFFFFFFFF;
  TableSize = MEMORY_STORAGE_SIZE;
  Status = gBS->AllocatePages (
                           AllocateMaxAddress,
                           EfiReservedMemoryType,
                           EFI_SIZE_TO_PAGES(TableSize),
                           &MemStorageBuffer
                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Store Memory Storage Head address and add signature '_MEMSE_', Entry Address and total size information.
  //
  gMemStorageHeadAddr = MemStorageBuffer;
  MemoryStorageHead = (MEMORY_STORAGE_HEAD_STRUCTURE *)(UINTN)MemStorageBuffer;
  if (MemoryStorageHead->MemStorageEntrySignature != MEMORY_STORAGE_ENTRY_SIGNATURE) {
    //
    // Memory didn't include previous information. Create Memory Storage Head Information.
    // *** NO GOOD IDEA TO GET PREVIOUS MEMORY DATA ***
    //
    ZeroMem ((VOID *)MemStorageBuffer, TableSize);
    MemoryStorageHead->MemStorageEntrySignature = MEMORY_STORAGE_ENTRY_SIGNATURE;
    MemoryStorageHead->MemStorageCurrentLogAddr = MemStorageBuffer + sizeof(MEMORY_STORAGE_HEAD_STRUCTURE);
    MemoryStorageHead->MemStorageEntryAddr = MemStorageBuffer + sizeof(MEMORY_STORAGE_HEAD_STRUCTURE);
    MemoryStorageHead->MemStorageSize = MEMORY_STORAGE_SIZE;
  }
  gMemStorageEntryAddr = gMemStorageHeadAddr + sizeof(MEMORY_STORAGE_HEAD_STRUCTURE);
  
  //
  // Install Memory Storage protocol for user to Log Event
  //
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gH2OMemoryStorageProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gMemoryStorageProtocol
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Create ready to boot event, to publish F1F2 Error Message Table
    //       
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               PublishMemStorageSignature,
               NULL,
               &gReadyToBootEvent
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Create an evnet - Wait for Variable Write Architecture Protocol
    //
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    InstalledVarNotificationFunction,
                    NULL,
                    &mVarEvent
                    );

    if (!EFI_ERROR (Status)) {
      gBS->RegisterProtocolNotify (
             &gEfiVariableWriteArchProtocolGuid,
             mVarEvent,
             &EventRegistration
             );
    }
  }
  
  return Status;
}
