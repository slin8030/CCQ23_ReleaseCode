/** @file
  Data type definition for MRC.

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

#ifndef _BIND_H_
#define _BIND_H_
#pragma warning (disable : 4142 )

#pragma pack()

#if _MSC_EXTENSIONS

//
// Disable warning that make it impossible to compile at /W4
// This only works for Microsoft* tools
//

//
// Disabling bitfield type checking warnings.
//
#pragma warning (disable : 4214 )

//
// Disabling  warnings.
//
#pragma warning (disable : 4213)

//
// Disabling the unreferenced formal parameter warnings.
//
#pragma warning (disable : 4100 )

//
// Disable slightly different base types warning as CHAR8 * can not be set
// to a constant string.
//
#pragma warning (disable : 4057 )

//
// ASSERT(FALSE) or while (TRUE) are legal constructes so supress this warning
//
#pragma warning (disable : 4127 )

//
// Int64ShllMod32 unreferenced inline function
//
#pragma warning (disable : 4514 )

//
// Unreferenced formal parameter - We are object oriented, so we pass This even
//  if we  don't need them.
//
#pragma warning (disable : 4100 )


//#pragma warning (disable : 4133 )

#endif

#if (__STDC_VERSION__ < 199901L)
//
// No ANSI C 2000 stdint.h integer width declarations, so define equivalents
//
#if _MSC_EXTENSIONS
//
// use Microsoft* C complier dependent interger width types
//
#if defined(EFI64) || defined(EFIx64)
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
#endif

typedef unsigned long       uint32_t;
typedef long                int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                int8_t;
//#else  NEED SIM WITH _MSC_EXTENSIONS
#if SIM

typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                my_int8_t;
#else
//
// Assume standard IA-32 alignment.
// BugBug: Need to check portability of long long
//
//    typedef unsigned long long  uint64_t;
//    typedef long long           int64_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                int8_t;
#endif
#else
//
// Linux
//
#if !defined SIM
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                my_int8_t;
typedef char                int8_t;
#endif
#endif
#else
//
// Use ANSI C 2000 stdint.h integer width declarations
//
#include "stdint.h"
#endif

#if SIM

typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                my_int8_t;
#endif
//
// Native integer size in stdint.h
//
#if defined(EFI64) || defined(EFIx64)
typedef int64_t intn_t;
typedef uint64_t uintn_t;
#else
typedef uint32_t  uintn_t;
typedef int32_t   intn_t;
#endif

#endif

