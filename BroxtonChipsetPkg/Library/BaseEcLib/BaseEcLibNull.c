/** @file
  Base EC library implementation for H/W layer.

@copyright
  Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains 'Framework Code' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may not be modified, except as allowed by
 additional terms of your license agreement.

@par Specification Reference:
**/

#include <Base.h>
#include <Uefi.h>
#include <Library/EcHwLib.h>
#include <Library/EcLib.h>
#include <Library/DebugLib.h>

EFI_STATUS
DispatchEcInterface (
  IN EC_INTERFACE_TYPE         Interface,
  IN UINT8                     Command,
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  );

//
// Function implementations
//
/**
  Sends command to EC.

  @param[in] Command           Command byte to send
  @param[in] Timeout           Timeout in microseonds

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
SendEcCommandTimeout (
  IN UINT8                  Command,
  IN UINT32                 Timeout
  )
{
  return EFI_DEVICE_ERROR;
}


/**
  Sends command to EC.

  @param[in] Command           Command byte to send

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
SendEcCommand (
  IN UINT8                  Command
  )
{
  return EFI_DEVICE_ERROR;
}


/**
  Receives status from EC.

  @param[out] EcStatus       Status byte to receive

  @retval     EFI_SUCCESS
  @retval     EFI_DEVICE_ERROR
**/
EFI_STATUS
ReceiveEcStatus (
  OUT UINT8                 *EcStatus
  )
{
  *EcStatus = 0xFF;
  return EFI_DEVICE_ERROR;
}


/**
  Sends data to EC.

  @param[in] Data          Data byte to send

  @retval    EFI_SUCCESS
  @retval    EFI_DEVICE_ERROR
**/
EFI_STATUS
SendEcData (
  IN UINT8                 Data
  )
{
  return EFI_DEVICE_ERROR;
}


/**
  Receives data from EC.

  @param[out] Data              Data byte received
  @param[in]  Timeout           Timeout in microseonds

  @retval     EFI_SUCCESS       Read success
  @retval     EFI_DEVICE_ERROR  Read error
  @retval     EFI_TIMEOUT       Command timeout
--*/
EFI_STATUS
ReceiveEcDataTimeout (
  OUT UINT8                  *Data,
  IN  UINT32                 Timeout
 )
{
  *Data = 0;
  return EFI_DEVICE_ERROR;
}
/**
  Receives data from EC.

  @param[out] Data              Data byte received

  @retval     EFI_SUCCESS       Read success
  @retval     EFI_DEVICE_ERROR  Read error
  @retval     EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
ReceiveEcData (
  OUT UINT8                  *Data
 )
{
  *Data = 0;
  return EFI_DEVICE_ERROR;
}

/**
  Detect EC

  @param[out] EcMajorRevision
  @param[out] EcMinorRevision

  @retval     TRUE            EC is present on the system.
  @retval     FALSE           EC is NOT present on the system.
**/
BOOLEAN
DetectEc (
  OUT UINT8           *EcMajorRevision,
  OUT UINT8           *EcMinorRevision
  )
{
  *EcMajorRevision = 0;
  *EcMinorRevision = 0;
  return FALSE;
}


/**
  Sends data to the Embedded Controler.
    Four use-cases:
    1.Send command to EC (DataSize and DataBuffer can be NULL)
    2.Send command to EC and send data to EC
    3.Send command to EC and receive data from EC
    4.Send command to EC, send data to EC and receive data from EC

  @param[in]      EcId        Embedded Controller identification - in case there is more then one.
  @param[in]      Interface   Interface used to access the EC HW
  @param[in]      Command     Command value to send to the EC
  @param[in][out] DataSize    Size of data to send to the EC.
                              If the command retuned data - size of buffer returned by the EC.
                              Be aware of the DataSize must euqal to size of DataBuffer and cannot smaller
                              than number of send data or number of receive data, whichever is the grater.
  @param[in][out] DataBuffer  Pointer to the data buffer including data to be sent to the EC.
                              If the command returned data - pointer to the buffer including the data.
                              The buffer size should be the max of receive and transmit data.

  @retval         EFI_SUCCESS Success
  @retval         Other       Failed - EFI_TIMEOUT, EFI_INVALID_PARAMETER, EFI_UNSUPPORTED,
                                       EFI_BUFFER_TOO_SMALL, etc.
**/
EFI_STATUS
EcInterface (
  IN EC_ID                     EcId,
  IN EC_INTERFACE_TYPE         Interface,
  IN UINT8                     Command,
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  )
{
  *DataSize = 0;
  *DataBuffer = 0;
  return EFI_UNSUPPORTED;
}
