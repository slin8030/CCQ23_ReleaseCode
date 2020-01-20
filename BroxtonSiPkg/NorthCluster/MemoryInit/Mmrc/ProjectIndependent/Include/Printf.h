/** @file
  Memory Reference Code
  ESS - Enterprise Silicon Software

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

#ifndef _printf_h
#define _printf_h
#include "MmrcProjectDefinitions.h"
#if defined SIM || defined JTAG
#include <stdarg.h>
#endif
#if !defined SIM && !defined JTAG
#if defined(EFI64) || defined(EFIx64)
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/SerialPortLib.h>
#include <Library/PrintLib.h>
#endif
#endif


#if (MRC_DEBUG_DISABLE==1)
#define MmrcDebugPrint(dbgInfo)
#else
#define MmrcDebugPrint(dbgInfo) rcPrintf dbgInfo
#endif

#if ERROR_MESSAGES
#define MmrcErrorPrint(dbgInfo) rcPrintf dbgInfo
#else
#define MmrcErrorPrint(dbgInfo)
#endif
//Please DO NOT ABUSE the following print in view of saving crucial bytes
#define MmrcVitalPrint(dbgInfo) rcPrintf dbgInfo

#if MAXPIDLL_DEBUG_PRINT
#define MmrcMaxPiDllDebugPrint(dbgInfo) rcPrintf dbgInfo
#else
#define MmrcMaxPiDllDebugPrint(dbgInfo)
#endif

#ifdef PERIODIC_TRAINING_DEBUG_PRINT
#define MmrcPertrainDebugPrint(dbgInfo) rcPrintf dbgInfo
#else
#define MmrcPertrainDebugPrint(dbgInfo)
#endif

#define BDAT_DEBUG_PRINT 1

#ifdef BDAT_DEBUG_PRINT
#define MmrcBdatDebugPrint(dbgInfo) rcPrintf dbgInfo
#else
#define MmrcBdatDebugPrint(dbgInfo)
#endif



#define MMRC_DBG_NONE           0
#define MMRC_DBG_MIN            BIT0                                  // Show only basic flow through MRC and training results.
#define MMRC_DBG_MED            BIT1                                  // Show eye diagrams and other training info useful for EV.
#define MMRC_DBG_MAX            BIT2                                  // Show intermediate steps in training. Useful for training algo debug.
#define MMRC_DBG_TRACE          BIT3                                  // Show all register accesses.
#define MMRC_DBG_TST            BIT4                                  // Show any test messages - these are ones MRC developers can use for
                                                                      // debug but don't typically want displayed in the release BIOS.
#define MMRC_DBG_DEBUG          BIT5                                  // Use for temporary debug of issues. Do not release any MRC code with this
                                                                      // print level in use.
#define MMRC_DBG_REG_ACCESS     BIT6                                  // Displays all register accesses.
#define MMRC_DBG_TYPICAL        MMRC_DBG_MIN + MMRC_DBG_MED           // Normal messages are MRC flow plus training results and eye diagrams.
#define MMRC_DBG_VERBOSE        MMRC_DBG_MIN + MMRC_DBG_MAX + MMRC_DBG_MED + MMRC_DBG_TRACE // This should be the log level for most debug. Ask customers to turn
                                                                      // this level on before submitting serial debug logs to sightings.
#ifndef ASM_INC
#define TAB_STOP            4
#define LEFT_JUSTIFY        0x01
#define PREFIX_SIGN         0x02
#define PREFIX_BLANK        0x04
#define COMMON_PREFIX_ZERO  0x08
#define LONG_TYPE           0x10

#define INT_SIGNED          0x20
#define COMA_TYPE           0x40
#define LONG_LONG_TYPE      0x80

#define CHAR_CR             0x0d
#define CHAR_LF             0x0a

//
// ANSI Escape sequences for color
//
#ifndef _WINCON_
#define FOREGROUND_BLACK    30
#define FOREGROUND_RED      31
#define FOREGROUND_GREEN    32
#define FOREGROUND_YELLOW   33
#define FOREGROUND_BLUE     34
#define FOREGROUND_MAGENTA  35
#define FOREGROUND_CYAN     36
#define FOREGROUND_WHITE    37

#define BACKGROUND_BLACK    40
#define BACKGROUND_RED      41
#define BACKGROUND_GREEN    42
#define BACKGROUND_YELLOW   43
#define BACKGROUND_BLUE     44
#define BACKGROUND_MAGENTA  45
#define BACKGROUND_CYAN     46
#define BACKGROUND_WHITE    47

#define ATTRIBUTE_OFF       0
#define ATTRIBUTE_BOLD      1
#define ATTRIBUTE_UNDERSCORE  4
#define ATTRIBUTE_BLINK     5
#define ATTRIBUTE_REVERSE   7
#define ATTRIBUTE_CONCEAL   8
#endif
#ifndef INT32_MAX
#define INT32_MAX           0x7fffffffU
#endif

#ifndef va_start
typedef INT8   *va_list;
#ifndef _INTSIZEOF
#define _INTSIZEOF(n)       ((sizeof (n) + sizeof (UINT32) - 1) &~(sizeof (UINT32) - 1))
#endif
#define va_start(ap, v)     (ap = (va_list) & v + _INTSIZEOF (v))
#define va_arg(ap, t)       (*(t *) ((ap += _INTSIZEOF (t)) - _INTSIZEOF (t)))
#define va_end(ap)          (ap = (va_list) 0)
#endif

#define isdigit(_c)         (((_c) >= '0') && ((_c) <= '9'))
#define ishexdigit(_c)      (((_c) >= 'a') && ((_c) <= 'f'))
#endif // ASM_INC

typedef enum {
  MarkerInit = 0,
  MarkerFound25h,
  MarkerFoundMinus,
} MARKER_STATES;

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
;

#endif // _printf_h
