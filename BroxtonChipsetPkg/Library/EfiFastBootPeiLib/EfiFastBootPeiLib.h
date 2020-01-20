/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _EFI_FASTBOOT_PEI_LIB_H_
#define _EFI_FASTBOOT_PEI_LIB_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UsbDeviceModeLib.h>

#include <ScAccess.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/PciIo.h>

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
