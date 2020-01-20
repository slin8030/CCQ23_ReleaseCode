/*++

Copyright (c)  1999 - 2007 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    Manifest.h

Abstract:

--*/

#ifndef _MANIFEST_H_
#define _MANIFEST_H_

#include <Library/PcdLib.h>

#define SECTOR_SIZE_4KB             0x1000 // Common 4kBytes sector size
#define SECTOR_SIZE_5KB             0x1400 // Common 5kBytes sector size

//
//              0h            Fh    Address Mapping (rip/eip register)
//           0L .-------------.
//              |             |
//              |             |
//     SBM - 4K +-------------+ <--- 0xFFFD,F000
//              |   KM (4K)   |
//     IBB - 1K +-------------+ <--- 0xFFFE,0000 (PcdManifestBase)
//              |  SBM (1K)   |
//     H - 127K +-------------+ <--- 0xFFFE,0400
//              |  IBB (1K)   |
//              |             |
//              |             |
//              |             |
//            H '-------------' <--- 0xFFFF,FFFF
//     (2M/4M/8M)
//

///
/// The Global ID of a GUIDed HOB used to pass fastboot information.
///
#define EFI_MANIFEST_HOB_GUID \
  { \
    0xc88ca1fe, 0xd296, 0x4430, { 0x86, 0x7d, 0x9a, 0xc6, 0x90, 0x9c, 0xfd, 0x6b } \
  }

extern EFI_GUID gEfiManifestHobGuid;

typedef struct {
  UINT8               Data[SECTOR_SIZE_5KB];
} MANIFEST_HOB_DATA;

//
// Secure Boot Manifest for Android.
// (272 bytes = 17 x 16 bytes, with 16 bytes alignment).
//
#pragma pack(1)
typedef struct {
  UINT32              Signature;
  UINT8               SystemFirmwareGuid[16];
  UINT32              BIOSVersion;
  UINT32              BIOSCompatibleVersion;
  UINT8               Bios2ndStageHashLen;
  UINT8               Sha256Hash[32];
  UINT8               Reserved1[7];
  UINT8               OSPubKeyHash[32];
  UINT8               OSBLPubKeyHash[32];
  UINT8               Reserved2[32];
  UINT8               RecoveryFVHashLen;
  UINT8               RecoveryFVHash[32];
  UINT8               Reserved[32];
  UINT32              IFWIVersionLen;
  UINT8               IFWIVersion[32];
  UINT8               Reserved3[7];
} MANIFEST_OEM_BLOCK;
#pragma pack()

#endif

