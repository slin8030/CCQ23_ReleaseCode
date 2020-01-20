/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/
/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PeiKscLib.c
  
Abstract: 

  Ksc settings in PEI phase.

--*/
#include "PeiKsc.h"

//
// Function implemenations
//
EFI_STATUS
InitializeKscLib ( )
/*++

Routine Description:
  The PEI function requires CPU IO protocol, through which it reads KSC Command port
  and ensures that EC exists or not.
  
Arguments:

  --

Returns: 
  EFI_SUCCESS       - EC found and KscLib is successfully initialized.
  EFI_DEVICE_ERROR  - EC is NOT present on the system.

--*/ 
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SendKscCommand (
  IN UINT8						Command
  )
/*++

Routine Description:
  Sends command to Keyboard System Controller.

Arguments:
  Command     - Command byte to send

Returns: 
  EFI_SUCCESS       - Command success
  EFI_DEVICE_ERROR  - Command error
  EFI_TIMEOUT       - Command timeout

--*/ 
{
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
ReceiveKscStatus (
  OUT UINT8		*KscStatus
  )
/*++

Routine Description:
  Receives status from Keyboard System Controller.

Arguments:
  KscStatus   - Status byte to receive
  
Returns: 
  EFI_DEVICE_ERROR  - Ksc library has not initialized yet or KSC not present
  EFI_SUCCESS       - Get KSC status successfully

--*/
{
  *KscStatus = 0xFF;
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
SendKscData (
  IN UINT8						Data
  )
/*++

Routine Description:
  Sends data to Keyboard System Controller.

Arguments:
  Data        - Data byte to send

Returns: 
  EFI_SUCCESS       - Success
  EFI_DEVICE_ERROR  - Error
  EFI_TIMEOUT       - Command timeout

--*/ 
{
  return EFI_DEVICE_ERROR;
}

EFI_STATUS
ReceiveKscData (
  OUT UINT8						*Data
 )
/*++

Routine Description:
  Receives data from Keyboard System Controller.

Arguments:
  Data        - Data byte received

Returns: 
  EFI_SUCCESS       - Read success
  EFI_DEVICE_ERROR  - Read error
  EFI_TIMEOUT       - Command timeout

--*/ 
{
  *Data = 0;
  return EFI_DEVICE_ERROR;
}

