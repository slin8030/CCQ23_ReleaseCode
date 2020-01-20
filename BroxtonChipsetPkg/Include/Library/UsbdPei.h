/**

Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/          

#ifndef _EFI_USBD_PEI_
#define _EFI_USBD_PEI_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UsbDeviceModeLib.h>
#include <Ppi/UsbDeviceMode.h>
#include <ScAccess.h>

typedef struct  {
    UINT16 DevProductId;          // Product ID value for standard USB Device Descriptor
    UINT16 DevBcd;                // BCD value for standard USB Device Descriptor 
//    CHAR8  *pDevProductStr;       // Pointer to NULL terminated wide char string 
//    CHAR8  *pDevSerialNumStr;     // Pointer to NULL terminated wide char string 
    CHAR16  *pDevProductStr;       // Pointer to NULL terminated wide char string 
    CHAR16  *pDevSerialNumStr;     // Pointer to NULL terminated wide char string 
    UINT32 KernelLoadAddr;        // kernel base load address, not including any fastboot header 
    UINT32 KernelEntryOffset;     // offset from load address 
    UINT32 TimeoutMs;             // Timeout in ms for connecting to host. If 0, waits forever 
} FASTBOOTCMDPARAM;

FASTBOOTCMDPARAM FastbootParams;

#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )


EFI_STATUS
EFIAPI
FastBootDataInit (
  IN CONST EFI_PEI_SERVICES    **PeiServices
  );

EFI_STATUS
EFIAPI
FastBootStart (
  IN CONST EFI_PEI_SERVICES    **PeiServices
  );

#endif