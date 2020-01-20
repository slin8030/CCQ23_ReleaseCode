/** @file
  GenericUtilityLib

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _ENROLL_HASH_H_
#define _ENROLL_HASH_H_

#include "InternalUtilityLib.h"
#include "SecureBootMgr.h"
#include "SecureBootVar.h"
#include "SecureBootMisc.h"
#include "BootMaint.h"
#include "String.h"

#include <Protocol/FrameworkHii.h>
#include <Protocol/LoadFile.h>

#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Guid/WinCertificate.h>
#include <Guid/ImageAuthentication.h>


#define EM_MENU_ENTRY_SIGNATURE        SIGNATURE_32 ('E', 'M', 'E', 'T')
#define HASH_LINK_LIST_SIGNATURE       SIGNATURE_32 ('H', 'A', 'L', 'L')
#define EM_FILE_CONTEXT_SELECT         0x1
#define ENROLL_FILE_OPTION_OFFSET      0x8000
#define ENROLL_FILE_OPTION_MASK        0x7FFF

#define EM_UPDATE_DATA_SIZE            0x100000

#define EM_FILE_SYSTEM                 0x0001
#define EM_DIRECTORY                   0x0002
#define EM_UNKNOWN_CONTEXT             0x0003

#define EM_FILE_FORMAT_EFI             0x0001
#define EM_FILE_FORMAT_CER             0x0002
#define EM_FILE_FORMAT_SHA256          0x0003
#define EM_FILE_FORMAT_SHA512          0x0004

#define HASHALG_MD5                    0x00000000
#define HASHALG_SHA1                   0x00000001
#define HASHALG_SHA224                 0x00000002
#define HASHALG_SHA256                 0x00000003
#define HASHALG_SHA384                 0x00000004
#define HASHALG_SHA512                 0x00000005
#define HASHALG_MAX                    0x00000006

typedef struct {
  UINTN           Signature;
  LIST_ENTRY      Head;
  UINTN           MenuNumber;
} ENROLL_MENU_OPTION;

typedef struct {
  UINTN           Signature;
  LIST_ENTRY      Link;
  UINTN           OptionNumber;
  UINT16          *DisplayString;
  UINT16          *HelpString;
  EFI_STRING_ID   DisplayStringToken;
  EFI_STRING_ID   HelpStringToken;
  UINTN           ContextSelection;
  VOID            *VariableContext;
} ENROLL_MENU_ENTRY;

typedef struct {
  EFI_HANDLE                        Handle;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_FILE_HANDLE                   FHandle;
  UINT16                            *FileName;
  EFI_FILE_SYSTEM_VOLUME_LABEL      *Info;
  BOOLEAN                           IsRoot;
  BOOLEAN                           IsDir;
  BOOLEAN                           IsRemovableMedia;
  BOOLEAN                           IsLoadFile;
  BOOLEAN                           IsBootLegacy;
} EM_FILE_CONTEXT;

typedef struct {
  WIN_CERTIFICATE Hdr;
  UINT8           CertData[1];
} WIN_CERTIFICATE_EFI_PKCS;

typedef struct {
  EFI_GUID                 *Index;
  CHAR16                   *Name;           ///< Name for Hash Algorithm
  UINTN                    DigestLength;    ///< Digest Length
  UINT8                    *OidValue;       ///< Hash Algorithm OID ASN.1 Value
  UINTN                    OidLength;       ///< Length of Hash OID Value
} HASH_TABLE;

typedef struct {
  UINTN           Signature;
  LIST_ENTRY      Link;
  UINTN           ShaType;
  UINTN           HashSize;
  UINT8           Hash[32];
} HASH_LINK_LIST;

EFI_STATUS
InitializeEnrollMenu (
  IN SECURE_BOOT_MANAGER_CALLBACK_DATA      *CallbackData
  );

BOOLEAN
UpdateEmFileExplorer (
  IN SECURE_BOOT_MANAGER_CALLBACK_DATA      *CallbackData,
  IN UINT16                                 KeyValue
  );

extern ENROLL_MENU_OPTION         mSecureBootFsOptionMenu;
extern ENROLL_MENU_OPTION         mSecureBootDirectoryMenu;
extern STRING_DEPOSITORY          *mSecureBootFileOptionStrDepository;
#endif
