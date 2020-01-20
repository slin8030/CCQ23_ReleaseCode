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
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
//[-start-170516-IB08450375-add]//
#if BXTI_PF_ENABLE
#include <Library/PcdLib.h>
#endif
//[-end-170516-IB08450375-add]//

//[-start-151228-IB03090424-modify]//
#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND  1
#endif
#ifndef STALL_ONE_MILLI_SECOND
#define STALL_ONE_MILLI_SECOND  1000
#endif
//[-end-151228-IB03090424-modify]//

#ifndef EC_INIT_TIME_OUT
#define EC_INIT_TIME_OUT       0x200
#endif

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
  UINTN         Index;
  UINT8         EcStatus;

//[-start-170516-IB08450375-add]//
//[-start-171220-IB07400935-remove]//
//#if BXTI_PF_ENABLE
//  if (!PcdGetBool(PcdEcEnable)) {
//    DEBUG ((DEBUG_INFO, "Non-EC, return.\n"));
//    return EFI_SUCCESS;
//  }
//#endif
//[-end-171220-IB07400935-remove]//
//[-end-170516-IB08450375-add]//

  Index = 0;
  EcStatus = 0;
  //
  // Wait for EC to be ready (with a timeout)
  //
  ReceiveEcStatus (&EcStatus);
  while (((EcStatus & EC_S_IBF) != 0) && (Index < Timeout)) {
    MicroSecondDelay (15 * STALL_ONE_MICRO_SECOND);
    ReceiveEcStatus (&EcStatus);
    Index++;
  }

  if (Index >= Timeout) {
    return EFI_TIMEOUT;
  }

  //Printing EC Command Sent
  DEBUG ((DEBUG_INFO, "Sending EC Command: %02X\n", Command));

  //
  // Send the EC command
  //
  IoWrite8 (EC_C_PORT, Command);

  return EFI_SUCCESS;
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
  return SendEcCommandTimeout (Command, EC_TIME_OUT);
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
  //
  // Read and return the status
  //
  *EcStatus = IoRead8 (EC_C_PORT);

  return EFI_SUCCESS;
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
  UINTN         Index;
  UINT8         EcStatus;

  Index = 0;

  //
  // Wait for EC to be ready (with a timeout)
  //
  ReceiveEcStatus (&EcStatus);
  while (((EcStatus & EC_S_IBF) != 0) && (Index < EC_TIME_OUT)) {
    MicroSecondDelay (15);
    ReceiveEcStatus (&EcStatus);
    Index++;
  }
  if (Index >= EC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  //Printing EC Data Sent
  //
  DEBUG ((DEBUG_INFO, "Sending EC Data: %02X\n", Data));

  //
  // Send the data and return
  //
  IoWrite8 (EC_D_PORT, Data);

  return EFI_SUCCESS;
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
  UINTN         Index;
  UINT8         EcStatus;

//[-start-170516-IB08450375-add]//
//[-start-171220-IB07400935-remove]//
//#if BXTI_PF_ENABLE
//  if (!PcdGetBool(PcdEcEnable)) {
//    DEBUG ((DEBUG_INFO, "Non-EC, return.\n"));
//    return EFI_SUCCESS;
//  }
//#endif
//[-end-171220-IB07400935-remove]//
//[-end-170516-IB08450375-add]//

  Index = 0;

  //
  // Wait for EC to be ready (with a timeout)
  //
  ReceiveEcStatus (&EcStatus);
  while (((EcStatus & EC_S_OBF) == 0) && (Index < Timeout)) {
    MicroSecondDelay (15 * STALL_ONE_MICRO_SECOND);
    ReceiveEcStatus (&EcStatus);
    Index++;
  }

  if (Index >= Timeout) {
    return EFI_TIMEOUT;
  }
  //
  // Read EC data and return
  //
  *Data = IoRead8 (EC_D_PORT);

  //Printing EC Data Received
  DEBUG ((DEBUG_INFO, "Receiving EC Data: %02X\n", *Data));

  return EFI_SUCCESS;
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
  return ReceiveEcDataTimeout (Data, EC_TIME_OUT);
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
//[-start-161004-IB08450356-modify]//
  EFI_STATUS          Status;
  UINT8               EcVer[2];
  BOOLEAN             EcPresent;
  UINTN               Count;
  UINT8               EcVersionOffset[] = {EC_MEM_OFFSET_EC_VER_MAIN, EC_MEM_OFFSET_EC_VER_SUB};


  EcVer[0] = 0;
  EcVer[1] = 0;
  for (Count = 0; Count < (sizeof(EcVersionOffset) / sizeof(UINT8)); Count ++){
    Status = SendEcCommandTimeout (EC_C_READ_MEM, EC_INIT_TIME_OUT);
    if (Status != EFI_SUCCESS) {
      return FALSE;
    }
    Status = SendEcData (EcVersionOffset[Count]);
    if (Status != EFI_SUCCESS) {
      return FALSE;
    }
    Status = ReceiveEcDataTimeout (EcVer + Count, EC_INIT_TIME_OUT);
    if (Status != EFI_SUCCESS) {
      return FALSE;
    }
  }

  EcPresent = TRUE;
  *EcMajorRevision = EcVer[0];
  *EcMinorRevision = EcVer[1];
  if (EcPresent) {
    DEBUG ((DEBUG_ERROR, "+==============================================+\n"));
    DEBUG ((DEBUG_ERROR, "| EC Major Revision: %02X  EC Minor Revision: %02X |\n", *EcMajorRevision, *EcMinorRevision));
    DEBUG ((DEBUG_ERROR, "+==============================================+\n"));
  }
//[-end-161004-IB08450356-modify]//

  return EcPresent;
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
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

//[-start-170516-IB08450375-add]//
//[-start-171220-IB07400935-remove]//
//#if BXTI_PF_ENABLE
//  if (!PcdGetBool(PcdEcEnable)) {
//    DEBUG ((DEBUG_INFO, "Non-EC, return.\n"));
//    return EFI_SUCCESS;
//  }
//#endif
//[-end-171220-IB07400935-remove]//
//[-end-170516-IB08450375-add]//

  if (Interface >= EcInterfaceMax || EcId >= EcIdMax) {
    return EFI_INVALID_PARAMETER;
  }

  switch (EcId) {

    case EcId0:
      Status = DispatchEcInterface (Interface, Command, DataSize, DataBuffer);
      break;

    default:
      Status = EFI_INVALID_PARAMETER;
      break;
  }

  return Status;
}
