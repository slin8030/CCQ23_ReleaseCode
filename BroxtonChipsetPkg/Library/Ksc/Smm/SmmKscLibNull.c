/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SmmKscLib.c
    
Abstract:

  SMM KSC library implementation.

  These functions need to be SMM safe.
  
  These functions require the SMM IO library (SmmIoLib) to be present.
  Caller must link those libraries and have the proper include path.

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/

#include "SmmKscLib.h"

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
  The SMM library only requires SMM IO library and has no initialization.
  However, this must be called prior to use of any other KSC library functions
  for future compatibility.

Arguments:

  None.

Returns: 

  EFI_SUCCESS       - KscLib is successfully initialized.

--*/ 
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SendKscCommand (
  UINT8   Command
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
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
ReceiveKscStatus (
  UINT8   *KscStatus
  )
/*++

Routine Description:

  Receives status from Keyboard System Controller.

Arguments:

  KscStatus  - Status byte to receive
  
Returns: 

  EFI_SUCCESS       - Always success

--*/
{
  *KscStatus = 0xFF;
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
SendKscData (
  UINT8   Data
  )
/*++

Routine Description:

  Sends data to Keyboard System Controller.

Arguments:

  Data  - Data byte to send

Returns: 

  EFI_SUCCESS       - Success
  EFI_DEVICE_ERROR  - Error

--*/ 
{
  return EFI_DEVICE_ERROR;
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
  *Data = 0;
  return EFI_DEVICE_ERROR;
}
