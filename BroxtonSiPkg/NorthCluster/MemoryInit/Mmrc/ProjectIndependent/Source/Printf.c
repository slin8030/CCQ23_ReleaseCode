/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

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

@par Specification Reference:
**/

#include "Printf.h"
#include "Mmrc.h"
#ifdef MINIBIOS
#if defined (CAR)
  int rcExternalPrintf(const char *fmt, va_list args);
#else
#include "Uart.h"
#endif
#endif

#if defined SIM || defined JTAG
#include <stdio.h>
#endif

#if !defined SIM && !defined JTAG
#ifndef EFI_D_INFO
#define EFI_D_INFO        0x00000040          // Verbose
#define EFI_D_ERROR       0x80000000          // Error
#endif
#ifndef MINIBIOS
#ifndef __GNUC__
int printf (UINT32 PeiServices, UINT32 ErrorLevel, CONST char *Format, ...);
#endif
#endif
#endif
#define MAX_DEBUG_MESSAGE_LENGTH  0x200

#if defined SIM || defined JTAG
FILE  *gLogFile;
#endif

/**
  MmrcDebugPrint with stdargs varargs stack frame in place of .... Limited
  support for sizes other than UINT32 to save code space

  @param[in]  Format  String containing characters to print and formating data.
  @param[in]  Marker  va_list that points to the arguments for Format that are on
                      the stack.

  @return     None
**/
VOID
rcVprintf (
  CONST INT8 *Format,
  va_list    Marker
)
{
#if DEBUG_MSG || ERROR_MESSAGES

#if defined SIM || defined JTAG
  if (gLogFile != NULL) {
    vfprintf (gLogFile, Format, Marker);
    fflush (gLogFile);
  } else {
    vprintf (Format, Marker);
    fflush (stdout);
  }
#else
#ifdef CAR
  rcPrintf (EFI_D_INFO, Format, Marker);
//#else if defined MINIBIOS
//  vprintf (EFI_D_INFO, Format, Marker);
#endif
#endif //SIM

#endif // DEBUG_MSG
}


BOOLEAN SearchForDigit (INT8 c) {
  if (c >= 0x30 && c <= 0x39) {
    return TRUE;
  }
  return FALSE;
}

/**
  Prints string to serial output

  @param[in]  MsgLevel   Defines the level at which the message should be displayed
  @param[in]  Format     Format string for output

  @return     None
**/
VOID
rcPrintf (
  IN        UINT8     MsgLevel,
  IN        INT8      *Format,
  ...
)
{
//[-start-160329-IB07400711-modify]//
//[-start-160407-IB07400715-modify]//
//[-start-171023-IB07400920-modify]//
#if (MRC_EV_RMT_BUILD == 1)
//[-end-171023-IB07400920-modify]//
//[-end-160407-IB07400715-modify]//
  va_list Marker;
  INT8    *Ptr;
  MMRC_DATA *MrcData;
  UINT8   Buffer[MAX_DEBUG_MESSAGE_LENGTH];
  INT8    CopyStr[MAX_DEBUG_MESSAGE_LENGTH];
  UINT16  Index;
  MARKER_STATES State;

  MrcData = GetMrcHostStructureAddress ();

  if ((MrcData->MrcDebugMsgLevel & MsgLevel) != 0) {
    CopyStr[0]  = '\0';
    Buffer[0]   = '\0';
    Index       = 0;
    State       = MarkerInit;
    Ptr = Format;
    va_start (Marker, Format);
#if ENABLE_EXTERNAL_PRINTF == 1
    rcExternalPrintf (Format, Marker);
#elif defined MINIBIOS || defined SIM || defined JTAG
    rcVprintf (Format, Marker);
    return;
#else
    for (Index = 0; Format[Index] != 0; Index++) {
      CopyStr[Index] = Format[Index];
      switch (State) {
      case MarkerInit:
        if (Format[Index] == '%') {
          State = MarkerFound25h;
        }
        break;
      case MarkerFound25h:
        if (Format[Index] == '%') {
          State = MarkerInit;
        } else if (Format[Index] == '-') {
          State = MarkerFoundMinus;
        } else if (Format[Index] == 's') {
          CopyStr[Index] = 'a';
          State = MarkerInit;
        } else {
          State = MarkerInit;
        }
        break;
      case MarkerFoundMinus:
        if (SearchForDigit (Format[Index])) {
          State = MarkerFoundMinus;
        } else if (Format[Index] == 's') {
          CopyStr[Index] = 'a';
          State = MarkerInit;
        } else {
          State = MarkerInit;
        }
        break;
      default:
        State = MarkerInit;
        break;
      }
    }
    CopyStr[Index] = '\0';
    //
    // Convert the DEBUG() message to an ASCII String
    //
    AsciiVSPrint (Buffer, sizeof (Buffer), &CopyStr[0], Marker);
    va_end (Marker);
    //
    // Send the print string to a Serial Port using the EFI DebugLib
    //
    DEBUG((EFI_D_INFO, Buffer));

#endif
  }
#endif
//[-end-160329-IB07400711-modify]//
}

