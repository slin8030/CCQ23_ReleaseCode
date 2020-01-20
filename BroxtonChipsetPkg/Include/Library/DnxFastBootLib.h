//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
--*/
#ifndef _FASTBOOT_H_
#define _FASTBOOT_H_


#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UsbDeviceModeLib.h>
#include <Protocol/RunTimeUsbDeviceMode.h>
#include <Library/UefiBootServicesTableLib.h>

//#include <Protocol/FirmwareVolume2.h>
#include <ScAccess.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/PciIo.h>

//#include <Library/DxeServicesLib.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SimpleFileSystem.h>

#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )

typedef struct  {
    UINT16 DevProductId;          // Product ID value for standard USB Device Descriptor
    UINT16 DevBcd;                // BCD value for standard USB Device Descriptor 
    CHAR8  *pDevProductStr;       // Pointer to NULL terminated wide char string 
    CHAR8  *pDevSerialNumStr;     // Pointer to NULL terminated wide char string 
    UINT32 KernelLoadAddr;        // kernel base load address, not including any fastboot header 
    UINT32 KernelEntryOffset;     // offset from load address 
    UINT32 TimeoutMs;             // Timeout in ms for connecting to host. If 0, waits forever 
} FASTBOOT_CMD_PARAM;

FASTBOOT_CMD_PARAM FastbootParams;

EFI_STATUS 
InstallLegacyAcpi();

EFI_STATUS 
BdsBootAndroidFromEmmc ();

EFI_STATUS
LoadFileFromFileSystem (
  IN CONST CHAR16    *FileName,
  OUT VOID               **FileBuffer,
  OUT UINTN                *FileSize
);

typedef struct {
UINT64 Size;
UINT64 FileSize;
UINT64 PhysicalSize;
EFI_TIME CreateTime;
EFI_TIME LastAccessTime;
EFI_TIME ModificationTime;
UINT64 Attribute;
CHAR16 *FileName;
} EFI_FILE_INFO;


#endif
