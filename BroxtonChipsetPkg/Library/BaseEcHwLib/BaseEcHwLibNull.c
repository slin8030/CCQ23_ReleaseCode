/** @file
  BaseEcHw library implementation.

  This library provides basic Embedded Controller Hardware interface.
  It is keeped simple enough and used by other BaseEc libraries to abstacrt the usage

@copyright
 Copyright (c) 2014 - 2015 Intel Corporation. All rights reserved
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
#include <Library/EcHwLib.h>
#include <Library/EcLib.h>
#include <Library/DebugLib.h>


/**
  Detect EC through LPC interface

  @param[in][out] DataSize    Size of data to send to the EC.
                              If the command retuned data - size of buffer returned by the EC.
                              Be aware of the DataSize must euqal to size of DataBuffer and cannot smaller
                              than number of send data or number of receive data, whichever is the grater.
  @param[in][out] DataBuffer  Pointer to the data buffer including data to be sent to the EC.
                              If the command returned data - pointer to the buffer including the data.
                              The buffer size should be the max of receive and transmit data.

  @retval  EFI_SUCCESS
  @retval  EFI_NOT_FOUND
**/
EFI_STATUS
LpcEcGetModeCommand (
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  )
{
  *DataSize = 0;
  *DataBuffer = 0;
  return EFI_NOT_FOUND;
}


/**
  Send data to EC through LPC interface

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
LpcEcInterface (
  IN UINT8                     Command,
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  )
{
  *DataSize = 0;
  *DataBuffer = 0;
  return EFI_UNSUPPORTED;
}


/**
  Send data to EC through SPI interface

  @param[in]      Command         Command value to send to the EC
  @param[in][out] DataSize        Size of data to send to the EC.
                                  If the command retuned data - size of buffer returned by the EC.
                                  Be aware of the DataSize must euqal to size of DataBuffer and cannot smaller
                                  than number of send data or number of receive data, whichever is the grater.
  @param[in][out] DataBuffer      Pointer to the data buffer including data to be sent to the EC.
                                  If the command returned data - pointer to the buffer including the data.
                                  The buffer size should be the max of receive and transmit data.

  @retval         EFI_UNSUPPORTED
**/
EFI_STATUS
SpiEcInterface (
  IN UINT8                     Command,
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Send data to EC through ESPI interface

  @param[in]      Command         Command value to send to the EC
  @param[in][out] DataSize        Size of data to send to the EC.
                                  If the command retuned data - size of buffer returned by the EC.
                                  Be aware of the DataSize must euqal to size of DataBuffer and cannot smaller
                                  than number of send data or number of receive data, whichever is the grater.
  @param[in][out] DataBuffer      Pointer to the data buffer including data to be sent to the EC.
                                  If the command returned data - pointer to the buffer including the data.
                                  The buffer size should be the max of receive and transmit data.

  @retval         EFI_UNSUPPORTED
**/
EFI_STATUS
EspiEcInterface (
  IN UINT8                     Command,
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Send data to EC through I2C interface

  @param[in]      Command         Command value to send to the EC
  @param[in][out] DataSize        Size of data to send to the EC.
                                  If the command retuned data - size of buffer returned by the EC.
                                  Be aware of the DataSize must euqal to size of DataBuffer and cannot smaller
                                  than number of send data or number of receive data, whichever is the grater.
  @param[in][out] DataBuffer      Pointer to the data buffer including data to be sent to the EC.
                                  If the command returned data - pointer to the buffer including the data.
                                  The buffer size should be the max of receive and transmit data.

  @retval         EFI_UNSUPPORTED
**/
EFI_STATUS
I2cEcInterface (
  IN UINT8                     Command,
  IN OUT UINT8                 *DataSize,
  IN OUT UINT8                 *DataBuffer
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Dispatch EC interface that depends EC interface type

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
DispatchEcInterface (
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
