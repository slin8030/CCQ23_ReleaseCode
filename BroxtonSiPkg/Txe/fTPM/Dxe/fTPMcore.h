/** @file
  Definitions for HECI driver

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

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
#ifndef _HECI_CORE_H
#define _HECI_CORE_H

#include "CoreBiosMsg.h"

//
// HECI bus function version
//
#define HBM_MINOR_VERSION 0
#define HBM_MAJOR_VERSION 1



typedef union {
    UINT8 ul[12];
    struct {
      UINT8     byte1;      // 0
      UINT8     byte2;      // 0
      UINT8     byte3;      // 0
      UINT8     byte4;      // 0
      UINT8     byte5;      // 0
      UINT8     byte6;      // 0
      UINT8     byte7;      // 0
      UINT8     byte8;      // 0
      UINT8     byte9;      // 0
      UINT8     byte10;      // 0
      UINT8     byte11;      // 0
    } r;
} FTPM_START_CMD;

typedef union {
    UINT8 ul[12];
} FTPM_CMD;


//
// Prototypes
//
/**

  Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  @param[in]  none

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
fTPMInitialize (
  VOID
  )
;

/**

  Heci Re-initializes it for Host

  @param[in]  none

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
fTPMReInitialize (
  VOID
  )
;

/**

  Reads a message from the ME across HECI.

  @param[in]     Blocking    - Used to determine if the read is BLOCKING or NON_BLOCKING.
  @param[out]    MessageData - Pointer to a buffer used to receive a message.
  @param[in,out] Length      - Pointer to the length of the buffer on input and the length
                               of the message on return. (in bytes)

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciReceive (
  IN      UINT32                    Blocking,
  OUT     UINT32                    *MessageData,
  IN OUT  UINT32                    *Length
  )
;
/**

  Function sends one command (of any length) through the HCI buffer.

  @param[in]     Message     - Pointer to the message data to be sent.

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
fTPMCmdSend (
  IN      UINT8                    *Message
  )
;
/**

  Function sends one messsage through the HECI circular buffer and waits
    for the corresponding ACK message.

  @param[in,out]     Message     - Pointer to the message buffer.
  @param[in]         SendLength  - Length of the message in bytes.
  @param[in,out]     RecLength   - Length of the message response in bytes.
  @param[in]         HostAddress - Address of the sending entity.
  @param[in]         SeCAddress   - Address of the ME entity that should receive the message.

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciSendwACK (
  IN OUT  UINT32                    *Message,
  IN      UINT32                    Length,
  IN OUT  UINT32                    *RecLength,
  IN      UINT8                     HostAddress,
  IN      UINT8                     SeCAddress
  )
;
/**

  SeC reset and waiting for ready

  @param[in]     Delay - The biggest waiting time

  @retval  EFI_TIMEOUT - Time out
  @retval  EFI_SUCCESS - SeC ready

**/
EFI_STATUS
EFIAPI
SeCResetWait (
  IN  UINT32  Delay
  )
;

/**

  Function forces a reinit of the heci interface by following the reset heci interface via host algorithm
    in HPS 0.90 doc 4-17-06 njy

  @param[in]     None

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
ResetfTPM (
  VOID
  )
;

/**

  Return ME Status

  @param[in]     SeCStatus pointer for status report

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciGetSeCStatus (
  IN UINT32                     *SeCStatus
  )
;
/**

  Return ME Mode

  @param[in]     SeCMode pointer for ME Mode report

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciGetSeCMode (
  IN UINT32                     *SeCMode
  )
;

///
/// Local/Private functions not part of EFIAPI for HECI
///
/**

  HECI driver entry point used to initialize support for the HECI device.

  @param[in]    ImageHandle - Standard entry point parameter.
  @param[in]    SystemTable - Standard entry point parameter.

  @retval  EFI_STATUS

**/
EFI_STATUS
InitializeHECI (
  IN      EFI_HANDLE                ImageHandle,
  IN      EFI_SYSTEM_TABLE          *SystemTable
  )
;

/**

  Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  @param[in]   None

  @retval  EFI_STATUS

**/
EFI_STATUS
InitializeHeciPrivate (
  VOID
  )
;
/**

  This function provides a standard way to verify the HECI cmd and MBAR regs
  in its PCI cfg space are setup properly and that the local mHeciContext
  variable matches this info.

  @param[in]   None

  @retval  UINT32

**/
UINT32
CheckAndFixHeciForAccess (
  VOID
  )
;
/**

  Waits for the ME to report that it is ready for communication over the HECI
    interface.

  @param[in]   None

  @retval  EFI_STATUS

**/
EFI_STATUS
WaitForSECInputReady (
  VOID
  )
;
/**

  Calculate if the circular buffer has overflowed.
    Corresponds to HECI HPS (part of) section 4.2.1

  @param[in]   ReadPointer  - Location of the read pointer.
  @param[in]  WritePointer - Location of the write pointer.

  @retval  Number of filled slots.

**/
UINT8
FilledSlots (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer
  )
;


/**

  Calculate if the circular buffer has overflowed
    Corresponds to HECI HPS (part of) section 4.2.1

  @param[in]   ReadPointer - Value read from host/me read pointer
  @param[in]  WritePointer - Value read from host/me write pointer
  @param[in]  BufferDepth - Value read from buffer depth register

  @retval  EFI_STATUS

**/
EFI_STATUS
OverflowCB (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer,
  IN      UINT32                    BufferDepth
  )
;

/**

  Function to pull one message packet off.
    BWG0.7

  @param[in]  Message   - Pointer to a buffer to receive the message in.

  @retval  EFI_STATUS

**/
EFI_STATUS
PTTHciPacketRead (
  OUT     UINT8                    *Message
  )
;

/**

  Function sends one message packet through the HCI buffer
   for reference see BWG0.7

  @param[in]  Message   - Pointer to the actual message data.

  @retval  EFI_STATUS

**/
EFI_STATUS
PTTHciPacketWrite (
  IN      UINT8                    *Message
  )
;

/**

  SeCAlivenessRequest

  @param[in]  HeciMemBar
  @param[in]  Request

  @retval  EFI_STATUS

**/
EFI_STATUS
SeCAlivenessRequest (
  IN      UINT32                       *HeciMemBar,
  IN      UINT32            Request
  );

/**

  Send start command for fTPM

  @param[in]  type - Start command type

  @retval  EFI_STATUS

**/
EFI_STATUS
EFIAPI
StartCmd (
  IN UINT32                     *type
  )
;

/**

  getfTPMBaseAddr

  @param[in]  None

  @retval   UINT32

**/
UINT32
getfTPMBaseAddr(void)
/*
 * return base address of FTPM
 */
;
#endif // _FTPM_CORE_H
