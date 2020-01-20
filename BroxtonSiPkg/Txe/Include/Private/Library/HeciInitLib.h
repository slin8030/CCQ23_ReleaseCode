/** @file
  Header File for HECI Init Lib functionality

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#ifndef _HECI_INIT_LIB_H
#define _HECI_INIT_LIB_H


#define B_EXCLUSION BIT8

UINT32
MmIoReadDword (
  UINTN a
);

VOID
MmIoWriteDword (
  UINTN  a,
  UINT32 b
);

//-------------------------------------------------------------------------------------------------------
// Function Prototype declarations
//-------------------------------------------------------------------------------------------------------

EFI_STATUS
WaitForSECReady(
VOID
);

EFI_STATUS
EFIAPI
HeciReset(
IN      HECI_DEVICE      HeciDev
);

EFI_STATUS
HeciTakeOwnerShip(
);

/**
  Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  @param[in]  HECI Device

  @retval  EFI_STATUS

**/
EFI_STATUS
InitializeHeciPrivate(
IN HECI_DEVICE                  HeciDev
)
;


/**
CheckAndFixHeciForAccess -
This function gets HECI device PCI base address and checks for HECI device availability and provides HECI Device MBAR
after enabling Device specific BME, MSE and SERR.

@param[in]  HECI Device Number

@retval Corresponding HECI Device MBAR

**/
UINTN
CheckAndFixHeciForAccess(
IN HECI_DEVICE       HeciDev
)
;

EFI_STATUS
EFIAPI
Heci2SendwACK(
IN OUT  UINT32  *Message,
IN    UINT32  Length,
IN OUT  UINT32  *RecLength,
IN    UINT8   HostAddress,
IN    UINT8   SeCAddress
);

EFI_STATUS
EFIAPI
Heci2SendwoACK(
IN OUT  UINT32  *Message,
IN    UINT32  Length,
IN OUT  UINT32  *RecLength,
IN    UINT8   HostAddress,
IN    UINT8   SeCAddress
);

EFI_STATUS
EFIAPI
HeciSendwoACK(
IN OUT  UINT32  *Message,
IN    UINT32  Length,
IN OUT  UINT32  *RecLength,
IN    UINT8   HostAddress,
IN    UINT8   SeCAddress
);

EFI_STATUS
HeciGetBootDevice(
MBP_CURRENT_BOOT_MEDIA    *BootMediaData
);

/**
Checks if HECI reset has occured.

@param[in]  HECI Device Number

@retval TRUE - HECI reset occurred
@retval  FALSE - No HECI reset occurred
**/
BOOLEAN
CheckForHeciReset(
IN HECI_DEVICE                  HeciDev
);

/**

  DumpBuffer_HECI

  @param[in] Buffer1

  @return None

**/
VOID
DumpBuffer_HECI (
  VOID *Buffer1,
  UINT8 Buffersize
  );
//-------------------------------------------------------------------------------------------------------
// DXE-HECI Protocol Function Prototype declarations
//-------------------------------------------------------------------------------------------------------
/**
  Function sends one messsage through the HECI circular buffer and waits
    for the corresponding ACK message.

  @param[in,out]  Message     - Pointer to the message buffer.
  @param[in]      SendLength  - Length of the message in bytes.
  @param[in,out]  RecLength   - Length of the message response in bytes.
  @param[in]      HostAddress - Address of the sending entity.
  @param[in]      SeCAddress   - Address of the ME entity that should receive the message.

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciSendwACK(
IN      HECI_DEVICE   HeciDev,
IN OUT  UINT32        *Message,
IN OUT  UINT32        Length,
IN OUT  UINT32        *RecLength,
IN      UINT8         HostAddress,
IN      UINT8         SeCAddress
)
;


/**
  Reads a message from the SEC across HECI.

  @param[in]      Blocking    - Used to determine if the read is BLOCKING or NON_BLOCKING.
  @param[out]     MessageData - Pointer to a buffer used to receive a message.
  @param[in,out]  Length      - Pointer to the length of the buffer on input and the length
                                of the message on return. (in bytes)

  @retval  EFI_STATUS

**/

EFI_STATUS
EFIAPI
HeciReceive(
IN      HECI_DEVICE      HeciDev,
IN      UINT32           Blocking,
IN OUT  UINT32           *MessageBody,
IN OUT  UINT32           *Length
)
;
/**
  Function sends one messsage (of any length) through the HECI circular buffer.

  @param[in]  Message     - Pointer to the message data to be sent.
  @param[in]  Length      - Length of the message in bytes.
  @param[in]  HostAddress - The address of the host processor.
  @param[in]  SeCAddress   - Address of the ME subsystem the message is being sent to.

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciSend(
IN     HECI_DEVICE    HeciDev,
IN     UINT32        *Message,
IN     UINT32         Length,
IN     UINT8          HostAddress,
IN     UINT8          SeCAddress
)

;
/**
  Function forces a reinit of the heci interface by following the reset heci interface via host algorithm
    in HPS 0.90 doc 4-17-06 njy

  @param[in]  none

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
ResetHeciInterface(
IN      HECI_DEVICE      HeciDev
);

///
/// Prototypes
///
/**
  Determines if the HECI device is present and, if present, initializes it for
  use by the BIOS.

  @param[in] None

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciInitialize(
IN  HECI_DEVICE      HeciDev
);

/**
  Heci Re-initializes it for Host

  @param[in] None

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciReInitialize(
IN      HECI_DEVICE      HeciDev
);

/**
  SeC reset and waiting for ready

  @param[in]  Delay - The biggest waiting time

  @retval  EFI_TIMEOUT - Time out
  @retval  EFI_SUCCESS - SeC ready

**/
EFI_STATUS
EFIAPI
SeCResetWait(
IN  HECI_DEVICE   HeciDev,
IN  UINT32        Delay
)
;
/**
  Return SEC Status

  @param[in]  SeCStatus pointer for status report

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciGetSeCStatus(
IN UINT32                     *SeCStatus
);

/**
  Return SEC Status

  @param[in]  SeCStatus pointer for status report

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciGetSeCMode(
IN  HECI_DEVICE               HeciDev,
IN UINT32                     *SeCMode
);

/**
Function sends bios2ish bin file to CSE through HECI circular buffer and waits
for the corresponding ACK message.

@param[in]  Message     - Pointer to the send message buffer.
@param[out]  Message     - Pointer to the receive message buffer.
@param[in]      SendLength  - Length of the message in bytes.
@param[in,out]  RecLength   - Length of the message response in bytes.
@param[in]      HostAddress - Address of the sending entity.
@param[in]      SeCAddress   - Address of the ME entity that should receive the message.

@retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciIshSendwAck(
IN   HECI_DEVICE   HeciDev,
IN   VOID          *SendMessage,
OUT  VOID          *ReceiveMessage,
IN   UINT32        SendLength,
IN OUT  UINT32     *RecLength,
IN      UINT8      HostAddress,
IN      UINT8      SeCAddress
);

//[-start-160804-IB07400769-add]//
BOOLEAN
IsTxeDataRegionAvailable (
  VOID
  );
//[-end-160804-IB07400769-add]//

#endif
