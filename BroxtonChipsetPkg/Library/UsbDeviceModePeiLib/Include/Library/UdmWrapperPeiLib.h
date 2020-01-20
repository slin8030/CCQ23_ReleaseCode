/** @file
Copyright (c) 2009 - 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BASE_UDM_WRAPPER_PEI_LIB_H__
#define __BASE_UDM_WRAPPER_PEI_LIB_H__
#include <Library/DebugLib.h>

#define sprintf_s(...)
#define strncmp(string1,string2,count)    strcmp(string1,string2)
#define malloc(size)  AllocatePool (size)
#define strtoul(Str, endptr, Delim)  Strtok( Str, Delim )


void delayMicro(uint32_t MicroSecond);
void delay(uint32_t MicroSecond);

#endif // 
