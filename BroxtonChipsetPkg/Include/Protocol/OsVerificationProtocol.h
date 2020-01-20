/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  OsVerificationProtocol.h
  
Abstract:

 
--*/

#ifndef _OS_VERIFICATION_PROTOCOL_H_
#define _OS_VERIFICATION_PROTOCOL_H_

#include <PiDxe.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>

#define TDTHI_MANIFEST_SIZE 1024
#define DEFAULT_HASH_SHA256_LEN 32
#define DEFAULT_MAX_NUM_IMAGES  12
#pragma pack(1)
typedef struct _ImageHash {
  UINT8 Hash[DEFAULT_HASH_SHA256_LEN];
} ImageHash;

typedef struct _OEM_DATA {
  UINT8 Reserved[4];
  ImageHash Image[DEFAULT_MAX_NUM_IMAGES];
  UINT8 Reserved2[12];
} OEM_DATA;

typedef struct _MANIFEST_HEADER {
  UINT32 ManifestIdentifier;
  UINT32 ManifestVersion;
  UINT32 PayloadSize;
  UINT32 SecureVersionNumber;
  UINT8 KeyManifestId;
  UINT8 Reserved1;
  UINT16 Reserved2;
  UINT8 PayloadHash[DEFAULT_HASH_SHA256_LEN];
  UINT32 ARBImagetype;
  UINT32 KeyIndex;
  UINT8  Reserved3[28];
  OEM_DATA OemData;
  UINT32 DebugActivate;
  UINT32 DebugSessionId[4];
} MANIFEST_HEADER; /* size(MANIFEST_HEADER) = 508 bytes */

#pragma pack()
typedef struct _OS_VERIFICATION_PROTOCOL OS_VERIFICATION_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_OS_VERIFY) (
  IN OS_VERIFICATION_PROTOCOL *This,
  IN VOID                     *OsImagePtr,
  IN UINTN                    OsImageSize,
  IN VOID                     *ManifestPtr,
  IN UINTN                    ManifestSize
  );
 
typedef
EFI_STATUS
(EFIAPI *GET_SECURITY_POLICY) (
  IN     OS_VERIFICATION_PROTOCOL *This,
  IN OUT BOOLEAN                  *AllowUnsignedOS
);
 
struct _OS_VERIFICATION_PROTOCOL {
  EFI_OS_VERIFY           VerifyOsImage;
  GET_SECURITY_POLICY     GetSecurityPolicy;
};

extern EFI_GUID gOsVerificationProtocolGuid;

#endif

