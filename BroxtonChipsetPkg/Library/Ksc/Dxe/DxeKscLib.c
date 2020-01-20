/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DxeKscLib.c
    
Abstract:

  Boot service DXE KSC library implementation.

  These functions in this file can be called during DXE and cannot be called during runtime 
  or in SMM which should use a RT or SMM library.

  This library uses the CPU IO protocol, and will assert if the protocol cannot be found.
  Caller must ensure that CPU IO protocol is available.

--*/

#include "KscLib.h"
#include "DxeKscLib.h"

EFI_CPU_IO2_PROTOCOL	*mDxeKscLibCpuIo;

BOOLEAN mDxeKscLibInitialized = FALSE;

//
// Function implemenations
//
EFI_STATUS
InitializeKscLib (
  VOID
  )
/*++

Routine Description:

  Initialize the library.
  The DXE library only requires CPU IO protocol, so this will locate CPU IO protocol
  and save it for future use.

Arguments:

  None.

Returns: 

  EFI_SUCCESS       - KscLib is successfully initialized.

--*/ 
{
  EFI_STATUS                  Status;
  UINT8                       Data;

  //
  // Locate CpuIo protocol
  //
  Status = gBS->LocateProtocol (&gEfiCpuIo2ProtocolGuid, NULL, &mDxeKscLibCpuIo);
  ASSERT_EFI_ERROR (Status);

  //
  // Fail if EC doesn't exist.
  //
  mDxeKscLibCpuIo->Io.Read (mDxeKscLibCpuIo, EfiCpuIoWidthUint8, KSC_C_PORT, 1, &Data);
  if(Data == 0xff){
    mDxeKscLibInitialized = FALSE;
    return EFI_DEVICE_ERROR;
  }

  mDxeKscLibInitialized = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
SendKscCommand (
  UINT8                 Command
  )
/*++

Routine Description:

  Sends command to Keyboard System Controller.

Arguments:

  Command  - Command byte to send

Returns: 

  EFI_SUCCESS       - Command success
  EFI_DEVICE_ERROR  - Command error

--*/ 
{
  UINTN         Index;
  UINT8         KscStatus;
  EFI_STATUS    Status;

  //
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if (mDxeKscLibInitialized == FALSE){
    return EFI_DEVICE_ERROR;
  }

  KscStatus = 0;
  //
  // Verify library has been initialized properly
  //
  ASSERT (mDxeKscLibCpuIo);

  Index = 0;

  //
  // Wait for KSC to be ready (with a timeout)
  //
  ReceiveKscStatus (&KscStatus); 
  while (((KscStatus & KSC_S_IBF) != 0) && (Index < KSC_TIME_OUT)) {
    gBS->Stall (15);
    ReceiveKscStatus (&KscStatus);
    Index++;
  }
  if (Index >= KSC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Send the KSC command
  //
  Status = mDxeKscLibCpuIo->Io.Write (
                                 mDxeKscLibCpuIo,
                                 EfiCpuIoWidthUint8,
                                 KSC_C_PORT,
                                 1,
                                 &Command
                                 );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
ReceiveKscStatus (
  UINT8                 *KscStatus
  )
/*++

Routine Description:

  Receives status from Keyboard System Controller.

Arguments:

  Status  - Status byte to receive
  
Returns: 

  EFI_SUCCESS       - Always success

--*/
{ 
  EFI_STATUS Status = EFI_SUCCESS;

  // 
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE){
    return EFI_DEVICE_ERROR;
  }

  //
  // Verify library has been initialized properly
  //
  ASSERT (mDxeKscLibCpuIo);   

  //
  // Read and return the status 
  //
  Status = mDxeKscLibCpuIo->Io.Read (
                                 mDxeKscLibCpuIo,
                                 EfiCpuIoWidthUint8,
                                 KSC_C_PORT,
                                 1,
                                 KscStatus
                                 );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
SendKscData (
  UINT8                 Data
  )
/*++

Routine Description:

  Sends data to Keyboard System Controller.

Arguments:

  Data  - Data byte to send

Returns: 

  EFI_SUCCESS       - Success
  EFI_TIMEOUT       - Timeout
  Other             - Failed

--*/ 
{
  UINTN         Index;
  UINT8         KscStatus;
  EFI_STATUS    Status = EFI_SUCCESS;

  // 
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE){
    return EFI_DEVICE_ERROR;
  }

  //
  // Verify library has been initialized properly
  //
  ASSERT (mDxeKscLibCpuIo);

  Index = 0;

  //
  // Wait for KSC to be ready (with a timeout)
  //
  ReceiveKscStatus (&KscStatus);
  while (((KscStatus & KSC_S_IBF) != 0) && (Index < KSC_TIME_OUT)) {
    gBS->Stall (15);
    ReceiveKscStatus (&KscStatus);
    Index++;
  }
  if (Index >= KSC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Send the data and return
  //
  Status = mDxeKscLibCpuIo->Io.Write (
                                 mDxeKscLibCpuIo,
                                 EfiCpuIoWidthUint8,
                                 KSC_D_PORT,
                                 1,
                                 &Data
                                 );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
ReceiveKscData (
  UINT8   *Data
 )
/*++

Routine Description:

  Receives data from Keyboard System Controller.

Arguments:

  Data  - Data byte received

Returns: 

  EFI_SUCCESS       - Read success
  EFI_DEVICE_ERROR  - Read error

--*/ 
{
  UINTN         Index;
  UINT8         KscStatus;  
  EFI_STATUS    Status = EFI_SUCCESS;

  // 
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE){
    return EFI_DEVICE_ERROR;
  }

  //
  // Verify library has been initialized properly
  //
  ASSERT (mDxeKscLibCpuIo);

  Index = 0;

  //
  // Wait for KSC to be ready (with a timeout)
  //
  ReceiveKscStatus (&KscStatus);
  while (((KscStatus & KSC_S_OBF) == 0) && (Index < KSC_TIME_OUT)) {
    gBS->Stall (15);
    ReceiveKscStatus (&KscStatus);
    Index++;
  }
  if (Index >= KSC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Read KSC data and return
  //
  Status = mDxeKscLibCpuIo->Io.Read (
                                 mDxeKscLibCpuIo,
                                 EfiCpuIoWidthUint8,
                                 KSC_D_PORT,
                                 1,
                                 Data
                                 );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
