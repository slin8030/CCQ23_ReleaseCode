/** @file
  Intrinsic Memory Routines Wrapper Implementation for OpenSSL-based
  Cryptographic Library.

Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "usb.h"
#include <Library/UefiBootServicesTableLib.h>
//#include <Library/UefiRuntimeLib.h>


typedef UINT32 uint32_t;
//
//Adopt EFI delay 
//
void delayMicro(uint32_t MicroSecond)
{
	gBS->Stall(MicroSecond);
  return;		
}

void delay(uint32_t MicroSecond)
{
	gBS->Stall(MicroSecond);
  return;		
}

void UdmWrapperInit(
  VOID
)
{

  return;		
}



