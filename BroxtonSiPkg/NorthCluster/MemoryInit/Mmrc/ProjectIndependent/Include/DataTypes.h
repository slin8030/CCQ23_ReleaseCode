/** @file
  DataTypes.h
  This file include all the external data types.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#ifndef _DATATYPES_H_
#define _DATATYPES_H_

#ifndef __GNUC__
#pragma warning (disable : 4214 )
#endif

#ifndef EFI_MEMORY_INIT
#include "Bind.h"
typedef uint8_t     BOOLEAN;
#ifndef MRC_DATA_REQUIRED_FROM_OUTSIDE
#ifndef UINTN
typedef uintn_t     UINTN;
#endif
#endif

#ifndef size_t
#define size_t UINT32
#endif

#if SIM
typedef my_int8_t   INT8;
#else
typedef int8_t      INT8;
#endif
typedef uint8_t     UINT8;
typedef int16_t     INT16;
typedef uint16_t    UINT16;
#ifndef INT32
typedef int         INT32;
#endif
#ifndef UINT32
typedef uint32_t    UINT32;
#endif
typedef long long INT64;
typedef unsigned long long UINT64;
typedef uint8_t     CHAR8;
typedef uint16_t    CHAR16;
#ifndef EFI_PHYSICAL_ADDRESS
typedef UINTN       EFI_PHYSICAL_ADDRESS;
#endif
#ifndef GLOBAL_REMOVE_IF_UNREFERENCED
  //
  // The Microsoft* C compiler can removed references to unreferenced data items
  //  if the /OPT:REF linker option is used. We defined a macro as this is a
  //  a non standard extension
  //
  #if _MSC_EXTENSIONS
    #define GLOBAL_REMOVE_IF_UNREFERENCED __declspec(selectany)
  #else
    #define GLOBAL_REMOVE_IF_UNREFERENCED
  #endif
#endif
#endif

typedef struct {
  UINT32     Index;
  UINT32     DataHigh;
  UINT32     DataLow;
} MSR_REG;

typedef struct {
  INT8 x;
  INT8 y;
} TS_LOC;

//
// Modifiers to abstract standard types to aid in debug of problems
//
#ifndef CONST
#define CONST       const
#endif

#ifndef STATIC
#define STATIC      static
#endif

#ifndef VOID
#define VOID        void
#endif

#ifndef VOLATILE
#define VOLATILE    volatile
#endif
//
// Constants. They may exist in other build structures, so #ifndef them.
//
#ifndef IN
#define             IN
#endif

#ifndef OUT
#define             OUT
#endif

#ifndef TRUE
#define TRUE        ((BOOLEAN) 1 == 1)
#endif

#ifndef FALSE
#define FALSE       ((BOOLEAN) 0 == 1)
#endif

#ifndef NULL
#define NULL        ((VOID *) 0)
#endif

typedef UINT32           MMRC_STATUS;
#define MMRC_SUCCESS     0
#define MMRC_DATA_DIRTY  0xFFFFFFFE
#define MMRC_FAILURE     0xFFFFFFFF

#endif // DATATYPES_H

