//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/** @file

  The header file includes the definition of EFI_SPI_PROTOCOL.

Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#ifndef __EFI_FLASH_DEVICE_INFO_H__
#define __EFI_FLASH_DEVICE_INFO_H__

#define EFI_FLASH_DEVICE_INFO_PROTOCOL_GUID \
  { 0xcdd3ef06, 0xe873, 0x46ee, { 0xa0, 0x64, 0x7e, 0xc0, 0xb2, 0xc5, 0x6c, 0x11 } }

//
// Forward reference for ANSI C compatibility
//
typedef struct _EFI_FLASH_DEVICE_INFO_PROTOCOL  EFI_FLASH_DEVICE_INFO_PROTOCOL;

//
// Protocol definition
//
struct _EFI_FLASH_DEVICE_INFO_PROTOCOL {
  //
  // The base address pointed to SPI device info data array.
  //
  VOID               *Base;
  //
  // The length of SPI device info data array.
  //
  UINT32             Size;
};

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                gEfiFlashDeviceInfoProtocolGuid;

#endif
