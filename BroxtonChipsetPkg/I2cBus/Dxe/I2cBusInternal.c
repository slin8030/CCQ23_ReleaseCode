/** @file
  For I2C Master

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "I2cMasterDxe.h"

UINT64                         mRdscFreq = 0;

UINT8
I2cHcRead8 (
  IN I2C_BUS_INSTANCE    *I2cPort,
  IN UINT16              Offset
  )
/*++

Routine Description:

  This function provides a standard way to read ICH Smbus IO registers.

Arguments:

  I2cPort:  Pointer to I2cPort data structure.
  Offset:   Register offset from Smbus base IO address.

Returns:

  Returns data read from IO.

--*/
{
  return MmioRead8(I2cPort->I2cBase + Offset);
}

UINT16
I2cHcRead16 (
  IN I2C_BUS_INSTANCE    *I2cPort,
  IN UINT16              Offset
  )
/*++

Routine Description:

  This function provides a standard way to read ICH Smbus IO registers.

Arguments:

  I2cPort:  Pointer to private data structure.
  Offset:   Register offset from Smbus base IO address.

Returns:

  Returns data read from IO.

--*/
{
  return MmioRead16(I2cPort->I2cBase + Offset);
}

UINT32
I2cHcRead32 (
  IN I2C_BUS_INSTANCE    *I2cPort,
  IN UINT16              Offset
  )
/*++

Routine Description:

  This function provides a standard way to read ICH Smbus IO registers.

Arguments:

  I2cPort:  Pointer to private data structure.
  Offset:   Register offset from Smbus base IO address.

Returns:

  Returns data read from IO.

--*/
{
  return MmioRead32(I2cPort->I2cBase + Offset);
}

VOID
I2cHcWrite8 (
  IN I2C_BUS_INSTANCE    *I2cPort,
  IN UINT16              Offset,
  IN UINT8               Data
  )
/*++

Routine Description:

  This function provides a standard way to write ICH Smbus IO registers.

Arguments:

  I2cPort:  Pointer to private data structure.
  Offset:   Register offset from Smbus base IO address.
  Data:     Data to write to register.

Returns:

  None.

--*/
{
  //
  // Write New Value
  //
  MmioWrite8 (I2cPort->I2cBase + Offset, Data);  
}

VOID
I2cHcWrite16 (
  IN I2C_BUS_INSTANCE    *I2cPort,
  IN UINT16              Offset,
  IN UINT16               Data
  )
/*++

Routine Description:

  This function provides a standard way to write ICH Smbus IO registers.

Arguments:

  I2cPort:  Pointer to private data structure.
  Offset:   Register offset from Smbus base IO address.
  Data:     Data to write to register.

Returns:

  None.

--*/
{
  //
  // Write New Value
  //
  MmioWrite16 (I2cPort->I2cBase + Offset, Data);  
}

VOID
I2cHcWrite32 (
  IN I2C_BUS_INSTANCE    *I2cPort,
  IN UINT16              Offset,
  IN UINT32              Data
  )
/*++

Routine Description:

  This function provides a standard way to write ICH Smbus IO registers.

Arguments:

  I2cPort:  Pointer to private data structure.
  Offset:   Register offset from Smbus base IO address.
  Data:     Data to write to register.

Returns:

  None.

--*/
{
  //
  // Write New Value
  //
  MmioWrite32 (I2cPort->I2cBase + Offset, Data);  
}

EFI_STATUS
I2cEnable(
  IN  I2C_BUS_INSTANCE             *I2cPort
  )
{
  UINT32 NumTries;

  NumTries = 10000;
  
  I2cPort->I2cHcWrite16(I2cPort, R_IC_ENABLE, 1);

  while ( 0 == ( I2cPort->I2cHcRead16 (I2cPort, R_IC_ENABLE_STATUS) & 1 )) {
    gBS->Stall(10);
    NumTries --;
    if (0 == NumTries) {
      return RETURN_NOT_READY;
    }
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
I2cDisable (
  IN  I2C_BUS_INSTANCE             *I2cPort
  )
{
  I2cPort->I2cHcWrite16(I2cPort, R_IC_ENABLE, 0);

  while ( 0 != ( I2cPort->I2cHcRead16 ( I2cPort, R_IC_ENABLE ) & 1 )) {
    gBS->Stall(1);
  }
  
  return EFI_SUCCESS;
}

BOOLEAN
I2cHardwareActive (
  IN  I2C_BUS_INSTANCE                 *I2cPort
  )
{
  if (I2cPort->I2cHcRead32(I2cPort, R_IC_STATUS) & STAT_MST_ACTIVITY) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
TxAbortChk(
  IN  I2C_BUS_INSTANCE                 *I2cPort
  )
{
  UINT16 RawIntrValue;
  
  RawIntrValue = I2cPort->I2cHcRead16(I2cPort, R_IC_RAW_INTR_STAT);
  if ((RawIntrValue & B_I2C_INTR_TX_ABRT) == B_I2C_INTR_TX_ABRT){

    I2cPort->I2cHcRead16(I2cPort, R_IC_CLR_TX_ABRT);
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
CalibrateTscFrequency (
  OUT UINT64                        *Frequency
  )
{
  UINT64  BeginValue;
  UINT64  EndValue;

  if (Frequency == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BeginValue = AsmReadTsc ();
  gBS->Stall (100);
  EndValue   = AsmReadTsc ();
  
  *Frequency  = MultU64x32(EndValue - BeginValue, 10000);    

  return EFI_SUCCESS;
}

/**
  Reset the I2C controller and configure it for use

  The controller's I2C bus frequency is set to 100 KHz.

  @param[in] This          I2C bus I2cPort info
  @param[in] Slave         Slave Address

**/
VOID
I2cReset (
  IN  I2C_BUS_INSTANCE                 *I2cPort
  )
{
  UINT8        Index;
  UINT8        SpeedIndex;
  EFI_STATUS   Status;
  
  if (mRdscFreq == 0) {
    CalibrateTscFrequency(&mRdscFreq);
  }

  if (I2cPort->Info.Init){
    I2cPort->Info.Init(I2cPort);
  }
  
  Status = I2cDisable ( I2cPort );

  SpeedIndex = (UINT8)((I2cPort->I2cBusSpeed>>1) - 1);

  for (Index = 0; Index < 3; Index++) {
    I2cPort->I2cHcWrite16(I2cPort, 0x14 + (Index * 8), I2cPort->Info.QkR[Index].H);
    I2cPort->I2cHcWrite16(I2cPort, 0x18 + (Index * 8), I2cPort->Info.QkR[Index].L);
  }

  Status = EFI_SUCCESS;

  I2cPort->I2cHcWrite16(I2cPort, I2cPort->Info.SHR, I2cPort->Info.QkR[SpeedIndex].S);
  
  Status = I2cEnable ( I2cPort );
}

RETURN_STATUS
I2cStartRequest (
  IN  I2C_BUS_INSTANCE    *I2cPort,
  IN  UINTN               SlaveAddress,
  IN  UINTN               *WriteBytes,
  IN  UINT8               *WriteBuffer,
  IN  UINTN               *ReadBytes,
  OUT UINT8               *ReadBuffer,
  IN  BOOLEAN             P
  )
{
  UINT8        *ReceiveDataEnd;
  UINT8        *ReceiveRequest;
  UINT8        *TransmitEnd;
  UINT16       ReceiveData;
  UINT16       RawIntrStat;
  UINT32       I2cStatus;
  UINT32       NumTries;
  UINT32       Eflags;
  UINT64       RdTstStart;
  UINT64       RdTscEnd;
  UINT64       RdTscInterval;
  EFI_STATUS   Status;
  CONST UINT32 Timeout = 1000;

  RawIntrStat    = 0;
  NumTries       = 0;
  ReceiveRequest = NULL;
  
  //
  //  Verify the parameters
  //
  Status = RETURN_SUCCESS;
  
  if ( 1023 < SlaveAddress ) {
    Status =  RETURN_INVALID_PARAMETER;
    goto ExitI2cStartRequest;
  }

  NumTries = 100 * 1000;
  
  while ( I2cHardwareActive ( I2cPort )) {
    gBS->Stall(10);
    NumTries --;
    if(0 == NumTries) {
      Status = EFI_DEVICE_ERROR;
      goto ExitI2cStartRequest;
    }
  }

  Status = I2cDisable ( I2cPort );
  
  I2cPort->I2cHcWrite16 ( I2cPort, I2cPort->Info.CRG, 1);
  I2cPort->I2cHcWrite16 ( I2cPort, R_IC_INTR_MASK, 0x0);
  I2cPort->I2cHcWrite16 ( I2cPort, R_IC_TAR, (UINT16) SlaveAddress );
  I2cPort->I2cHcWrite16 ( I2cPort, R_IC_RX_TL, 0);
  I2cPort->I2cHcWrite16 ( I2cPort, R_IC_TX_TL, 0 );

  //Setup clock frequency and speed mode
  //enable master FSM, disable slave FSM
  I2cPort->I2cHcWrite32( I2cPort, R_IC_CON, B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE | I2cPort->I2cBusSpeed);
  
  Status = I2cEnable(I2cPort);
  
  if( EFI_ERROR(Status) ) {
    Status = EFI_DEVICE_ERROR;
    goto ExitI2cStartRequest;
  }

  I2cPort->I2cHcRead16 ( I2cPort, R_IC_CLR_TX_ABRT);

  ReceiveDataEnd = &ReadBuffer [ *ReadBytes ];
  TransmitEnd    = &WriteBuffer [ *WriteBytes ];
  RdTstStart     = AsmReadTsc();
  RdTscEnd       = RdTstStart + MultU64x32(DivU64x32Remainder(mRdscFreq, 1000, NULL), Timeout) ;
  
  if (RdTscEnd < RdTstStart) {
    DEBUG((EFI_D_ERROR, "Rdtsc overflow\n"));
    Status = RETURN_INVALID_PARAMETER;
    goto ExitI2cStartRequest;
  }

  Status = EFI_SUCCESS;
  
  if (WriteBytes) {
    
    LocalIrqSave(&Eflags);
    
    *WriteBytes = 0;
    while ( TransmitEnd > WriteBuffer ) {
      
      I2cStatus = I2cPort->I2cHcRead16 ( I2cPort, R_IC_STATUS);

      Status = TxAbortChk(I2cPort);
      
      if (EFI_ERROR(Status)) {
        break;
      }
      
      if ( AsmReadTsc() > RdTscEnd ) {
        // this shouldn't happen , the timeout value should be enough to transfer all the bytes. 
        // if timeout value isn't enough, do the following workaround   
        RdTscEnd += 1000000;
        while( (0 == (I2cPort->I2cHcRead16 ( I2cPort, R_IC_STATUS ) & STAT_TFNF) ) && 
               ( AsmReadTsc() < RdTscEnd ));
        
        if( I2cPort->I2cHcRead16 ( I2cPort, R_IC_STATUS ) & STAT_TFNF ) {
          I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, *WriteBuffer | B_STOP_CMD );
          gBS->Stall(2000);  //finish the data transmission in fifo
        } else {
        }          
        Status = EFI_TIMEOUT;
        break;
      }
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        continue;
      }
      
      if( (TransmitEnd == (WriteBuffer + 1)) && !P) {
        I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, *WriteBuffer);
        I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, B_STOP_CMD );                
        WriteBuffer ++;
      } else {
        I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, *WriteBuffer++ );
      }
      (*WriteBytes)++;
      // Add a small delay to work around some odd behavior being seen.  Without
      // this delay bytes get dropped.
      gBS->Stall (I2cPort->IntervalTime);
    }
    
    LocalIrqRestore(Eflags);
    
    if (EFI_ERROR(Status)) {
      goto ExitI2cStartRequest;
    }
    
    //Wait for bytes to go
    //Write into Tx fifo doesn't mean the dat will go correctly on the SDA data line
    while(1) {
        
      Status = TxAbortChk(I2cPort);
      
      if (EFI_ERROR(Status)) {
        goto ExitI2cStartRequest;
      }

      if( 0 == I2cPort->I2cHcRead16(I2cPort, R_IC_TXFLR)){
        break;      
      }
      
      if( AsmReadTsc() > RdTscEnd ) {
        Status = EFI_TIMEOUT;
        goto ExitI2cStartRequest;
      }
    }    
  }

  if(EFI_ERROR(Status)) 
    goto ExitI2cStartRequest;


  if ( ReadBytes) {
    *ReadBytes = 0;
    ReceiveRequest = ReadBuffer;
    LocalIrqSave(&Eflags);
    
    while ( (ReceiveDataEnd > ReceiveRequest) || (ReceiveDataEnd > ReadBuffer)) {

      //  Check for NACK
      Status = TxAbortChk(I2cPort);
      
      if (EFI_ERROR(Status)) {
        break;
      }
      
      //  Determine if another byte was received
      I2cStatus = I2cPort->I2cHcRead16 ( I2cPort, R_IC_STATUS );
      if ( 0 != ( I2cStatus & STAT_RFNE )) {
        ReceiveData = I2cPort->I2cHcRead16 ( I2cPort, R_IC_DATA_CMD );

        *ReadBuffer++ = (UINT8)ReceiveData;
        (*ReadBytes)++;
      }
      
      if ( AsmReadTsc() > RdTscEnd ) {        
        // this shouldn't happen , the timeout value should be enough to transfer all the bytes. 
        // if timeout value isn't enough, do the following workaround   
        if( ReceiveDataEnd > ReceiveRequest ) {
          RdTscEnd += 1000000;
          while( (0 == (I2cPort->I2cHcRead16 ( I2cPort, R_IC_STATUS ) & STAT_TFNF) ) && 
                 ( AsmReadTsc() < RdTscEnd ) 
               );
          if( I2cPort->I2cHcRead16 ( I2cPort, R_IC_STATUS ) & STAT_TFNF ) {
            I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, B_READ_CMD | B_STOP_CMD );
            gBS->Stall(2000);  //finish the data transmission in fifo
          } else {
            DEBUG((EFI_D_INFO, "Tricky things happen!!!\r\n\r\n\r\n"));
          }          
        } else {
          DEBUG((EFI_D_INFO, "All read command has been transferred\r\n"));
        }
        Status = EFI_TIMEOUT;
        break;
      }

      if (ReceiveDataEnd == ReceiveRequest) {
        continue;
      }
      
      //  Wait until a read request will fit      
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        gBS->Stall (10);
        continue;
      }
      //  Issue the next read request
      if(ReceiveDataEnd == ( ReceiveRequest + 1 ) ) {
        I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, B_READ_CMD | B_STOP_CMD );
      } else {
        I2cPort->I2cHcWrite16 ( I2cPort, R_IC_DATA_CMD, B_READ_CMD );
      }
      
      ReceiveRequest += 1;
    }
    LocalIrqRestore(Eflags);
  }

  RdTscInterval = AsmReadTsc() - RdTstStart;

ExitI2cStartRequest:
    
  return Status;
}

EFI_STATUS
LocateI2cBase(
  IN  I2C_BUS_INSTANCE      *I2cPort,
  OUT EFI_PHYSICAL_ADDRESS  *Base0BackUp,
  OUT UINT32                *PciCmdBackUp
  )
{
  UINTN                 PciI2cDevBase;
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  MmioBase0;

  MmioBase0     = 0;

  if (I2cPort->Info.GetDeviceMode (I2cPort) != I2C_DEVICE_MODE_PCI) {
    return EFI_SUCCESS;
  }
  
  PciI2cDevBase = MmPciAddress (
                  0,
                  I2cPort->PciI2c.Bus,
                  I2cPort->PciI2c.Dev,
                  I2cPort->PciI2c.Func,
                  0
                  );

  if (MmioRead16(PciI2cDevBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    return EFI_ALREADY_STARTED;
  }

  *Base0BackUp     = MmioRead32(PciI2cDevBase + 0x10);
  *PciCmdBackUp    = MmioRead32 (PciI2cDevBase + PCI_COMMAND_OFFSET);

  if (*Base0BackUp == 0) {
    Status = gDS->AllocateMemorySpace (
                   EfiGcdAllocateAnySearchBottomUp,
                   EfiGcdMemoryTypeMemoryMappedIo,
                   12,
                   0x1000,
                   &MmioBase0,
                   gImageHandle,
                   NULL
                   );
  
    if (EFI_ERROR(Status)) {
      return Status;
    }
  } else {
    MmioBase0 = *Base0BackUp & 0xFFFFF000;
  }
  
  I2cPort->I2cBase = (UINT32)(UINTN)MmioBase0;
  MmioAnd32 (PciI2cDevBase + PCI_COMMAND_OFFSET, (UINT32)~(EFI_PCI_COMMAND_MEMORY_SPACE|EFI_PCI_COMMAND_BUS_MASTER));
  MmioWrite32 (PciI2cDevBase + 0x10, I2cPort->I2cBase|BIT2);
  MmioOr32 (PciI2cDevBase + PCI_COMMAND_OFFSET, (EFI_PCI_COMMAND_MEMORY_SPACE|EFI_PCI_COMMAND_BUS_MASTER));

  return EFI_SUCCESS;
}

VOID
FreeI2cBase(
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN EFI_PHYSICAL_ADDRESS  Base0Org,
  IN UINT32                PciCmdOrg
  )
{
  UINTN                 PciI2cDevBase;

  if (I2cPort->Info.GetDeviceMode (I2cPort) != I2C_DEVICE_MODE_PCI) {
    return;
  }
  
  PciI2cDevBase = MmPciAddress (
                  0,
                  I2cPort->PciI2c.Bus,
                  I2cPort->PciI2c.Dev,
                  I2cPort->PciI2c.Func,
                  0
                  );

  if (MmioRead16(PciI2cDevBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    return;
  }

  if (MmioRead32(PciI2cDevBase + 0x10) != Base0Org) {
    MmioAnd32 (PciI2cDevBase + PCI_COMMAND_OFFSET, (UINT32)~(EFI_PCI_COMMAND_MEMORY_SPACE|EFI_PCI_COMMAND_BUS_MASTER));
    MmioWrite32 (PciI2cDevBase + 0x10, (UINT32)(UINTN)Base0Org);  
    
    gDS->FreeMemorySpace (I2cPort->I2cBase, (UINT64) 0x1000);
  }
  
  MmioWrite32 (PciI2cDevBase + PCI_COMMAND_OFFSET, PciCmdOrg);  
}

VOID
PollI2cNotify (
  IN  EFI_EVENT                   Event,
  IN  VOID                        *Context
  )
/*++

Routine Description:

  Function to be called every time periodic event happens. This will check if
  the SMBus Host Controller has received a Host Notify command. If so, it will
  see if a notification has been reqested on that event and make any callbacks
  that may be necessary.

Arguments:

  Event:    The periodic event that occured and got us into this callback.
  Context:  Event context. Will be NULL in this case, since we already have our
              I2cPort data in a module global variable.

Returns:


--*/
{
  EFI_STATUS                      Status;
  I2C_ASYNC_NODE                  *Node;
  I2C_BUS_INSTANCE                *I2cContext = NULL;
  LIST_ENTRY                      *TempList  = NULL;

  I2cContext = (I2C_BUS_INSTANCE*)Context;

  if (IsListEmpty(&I2cContext->AsyncQueue)) {
    return;
  }
  
  for (TempList = GetFirstNode(&I2cContext->AsyncQueue); !IsNull(&I2cContext->AsyncQueue, TempList);
       TempList = GetNextNode( &I2cContext->AsyncQueue, TempList)){

    Node = I2C_ASYNC_CONTEXT_FROM_LINK (TempList);

    if (I2cContext->Info.GpioArrived (I2cContext, Node->I2cDev->GpioHc, Node->I2cDev->GpioPin, Node->I2cDev->GpioLevel)) {
    
      I2cContext->CacheMode     = TRUE;
      I2cContext->CacheAvailable = 0;
              
      //
      // We have a match, notify the requested function
      //
      Status =  I2cContext->MasterApi.StartRequest (
            &I2cContext->MasterApi,
            Node->I2cDev->I2cDevice->SlaveAddressArray[0],
            Node->RequestPacket,
            NULL,
            Node->I2cStatus
            );


      gBS->SignalEvent (Node->Event);
      
      RemoveEntryList (TempList);

      if (Node->I2cDev) {
        FreePool (Node->I2cDev);
      }

      if (Node->RequestPacket) {
        FreePool (Node->RequestPacket);
      }

      if (Node) {
        FreePool (Node);
      }
      
      //
      // Restore the mode back to normal
      //
      I2cContext->CacheMode = FALSE;
    }
  }  
}

EFI_STATUS
InitializePeriodicEvent (
  IN I2C_BUS_INSTANCE            *I2cContext
  )
{
  EFI_STATUS                  Status;
  
  Status = gBS->CreateEvent (
                  (EVT_TIMER | EVT_NOTIFY_SIGNAL),
                  TPL_NOTIFY,
                  PollI2cNotify,
                  I2cContext,
                  &I2cContext->TimEvent
                  );
  
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = gBS->SetTimer (
                  I2cContext->TimEvent,
                  TimerPeriodic,
                  10 * MILLISECOND
                  );
  
  if (EFI_ERROR(Status)) {
    gBS->CloseEvent (I2cContext->TimEvent);
  }
  
  return Status;
}

EFI_STATUS
FindI2cDev (
  IN   EFI_HANDLE         Handle,
  IN   UINTN              SlaveAddress,
  OUT  EFI_I2C_DEVICE     **I2cDev
  )
{
  UINTN                       Index;
  EFI_STATUS                  Status;
  EFI_I2C_ENUMERATE_PROTOCOL  *I2cEnumerate;

  if (I2cDev == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiI2cEnumerateProtocolGuid,
                  (VOID**) &I2cEnumerate
                  );
  ASSERT ( EFI_SUCCESS == Status );

  if (EFI_ERROR(Status)) {
    return Status;
  }

  *I2cDev    = NULL;

  do {
    Status = I2cEnumerate->Enumerate (I2cEnumerate, I2cDev);

    if (EFI_ERROR(Status)) {
      break;
    }

    ASSERT ( *I2cDev != NULL );

    for (Index = 0; Index < (*I2cDev)->SlaveAddressCount; Index++) {
      if ((*I2cDev)->SlaveAddressArray[Index] == SlaveAddress) {
        return EFI_SUCCESS;
      }
    }

  } while (1);

  *I2cDev    = NULL;

  return EFI_NOT_FOUND;
}

EFI_STATUS
FindI2cHidDev (
  IN   EFI_I2C_DEVICE      *I2cDev,
  OUT  H2O_I2C_HID_DEVICE  **I2cHidDev
  )
{
  UINTN                              DeviceHandleCount;
  UINTN                              Index;
  BOOLEAN                            Found;
  UINTN                              DataLength;
  EFI_STATUS                         Status;
  EFI_HANDLE                         *DeviceHandleBuffer;
  EFI_ADAPTER_INFORMATION_PROTOCOL   *Aip;
  H2O_I2C_HID_DEVICE                 *I2cHidDevice;

  Aip                = NULL;
  DeviceHandleBuffer = NULL;
  Found              = FALSE;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiAdapterInformationProtocolGuid,
                  NULL,
                  &DeviceHandleCount,
                  &DeviceHandleBuffer
                  );

  if (EFI_ERROR(Status)) {
    Status = EFI_NOT_FOUND;
    goto ExitFindI2cHideDev;
  }

  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    DeviceHandleBuffer[Index],
                    &gEfiAdapterInformationProtocolGuid,
                    &Aip
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = Aip->GetInformation (
                    Aip,
                    &gI2cHidDeviceInfoGuid,
                    (VOID**) &I2cHidDevice,
                    &DataLength
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    if (CompareGuid (I2cHidDevice->I2cDevice->DeviceGuid, I2cDev->DeviceGuid)) {
      *I2cHidDev = I2cHidDevice;
      Found = TRUE;
      break;
    }

    FreePool (I2cHidDevice);
  }

  if (Found) {
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_FOUND;
  }

ExitFindI2cHideDev:

  if (DeviceHandleBuffer) {
    FreePool (DeviceHandleBuffer);
  }
  return Status;
}


EFI_STATUS
I2cPusAsyncQueue (
  IN  I2C_BUS_INSTANCE            *I2cContext,
  IN  UINTN                       SlaveAddress,
  IN  EFI_I2C_REQUEST_PACKET      *RequestPacket,
  IN  EFI_EVENT                   Event,
  OUT EFI_STATUS                  *I2cStatus
  )
/*++

  Routine Description:
  
    Register a callback in the event of a Host Notify command being sent by a
    specified Slave Device.
  
  Arguments:
  
    This:           Pointer to the instance of the EFI_SMBUS_HC_PROTOCOL.
    SlaveAddress:   Address of the device whose Host Notify command we want to
                      trap.
    Data:           Data of the Host Notify command we want to trap.
    NotifyFunction: Function to be called in the event the desired Host Notify
                      command occurs.
  
  Returns:
  
    EFI_INVALID_PARAMETER:  NotifyFunction was NULL.
    EFI_OUT_OF_RESOURCES:   Unable to allocate space to register the notification.
    EFI_UNSUPPORTED:        Unable to create the event needed for notifications.
    EFI_SUCCESS:            Function completed successfully

-*/
{
  EFI_STATUS             Status;
  UINTN                  Length;
  I2C_ASYNC_NODE         *NewNode;
  EFI_I2C_DEVICE         *I2cDev;
  H2O_I2C_HID_DEVICE     *I2cHidDev;
  
  if (Event == NULL || RequestPacket == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FindI2cDev ( I2cContext->Handle, SlaveAddress, &I2cDev);

  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = FindI2cHidDev (I2cDev, &I2cHidDev);

  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  NewNode = AllocateZeroPool(sizeof(I2C_ASYNC_NODE));
  
  //
  // If this is the first notification request, start an event to periodically
  // check for a Notify master command.
  //
  if (!I2cContext->TimEvent) {
    Status = InitializePeriodicEvent (I2cContext);
    if (EFI_ERROR(Status)) {
      FreePool (NewNode);
      return EFI_UNSUPPORTED;
    }
  }

  if (RequestPacket->OperationCount == 2) {
    Length = sizeof(EFI_I2C_REQUEST_PACKET) + sizeof(EFI_I2C_OPERATION);
  } else {
    Length = sizeof(EFI_I2C_REQUEST_PACKET);
  }

  
  NewNode->Signature     = I2C_PRIVATE_DATA_SIGNATURE;
  NewNode->I2cDev        = I2cHidDev;
  NewNode->Event         = Event;
  NewNode->I2cStatus     = I2cStatus;
  NewNode->RequestPacket = AllocateZeroPool(Length);
  CopyMem ( NewNode->RequestPacket, RequestPacket, Length);  
      
  InsertTailList (&I2cContext->AsyncQueue, &NewNode->Link);

  return EFI_SUCCESS;
}

