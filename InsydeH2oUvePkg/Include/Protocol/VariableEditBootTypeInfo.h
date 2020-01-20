/** @file

  Provide the interface for Variable Editor Boot Type Infomation

;******************************************************************************
;* Copyright (c) 1983 - 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_H_
#define _VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_H_

#include <Protocol/LegacyBios.h>
#include <Library/SetupUtilityLib.h>

#define VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_GUID \
  { \
    0x83d72f64, 0xacda, 0x4502, { 0x8d, 0x62, 0x18, 0xe3, 0x4f, 0x36, 0xb2, 0xf6 } \
  }

typedef struct _VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL;

//
// Member functions
//
#pragma pack(1)

typedef struct _VARIABLE_EDIT_BOOT_TYPE_NAME_MAPPING {
  UINT16                                BootTypeID;
  CHAR16                                BootTypeName[31];
} VARIABLE_EDIT_BOOT_TYPE_NAME_MAPPING;

typedef struct _VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL {
  // offset 0x000
  UINT64                                Signature;
  UINT32                                Version;
  UINT32                                Length;
  UINT8                                 Reserved0[16];
  // offset 0x020
  BOOLEAN                               IsUefiOrLegacy; // 0: Legacy, 1: UEFI
  BOOLEAN                               IsValid;        // 1: Valid
  UINT8                                 Reserved1[14];
  // offset 0x030
  UINT16                                BootTypeStringCount;
  UINT16                                NameMappingCount;
  UINT16                                NameMappingNameCount;
  UINT16                                BootTypeOrderDefaultSequenceCount;
  UINT16                                BootTypeOrderNewSequenceCount;
  UINT8                                 Reserved2[6];
  // offset 0x040
  CHAR16                                BootTypeString[32];  // same as STRING_TOKEN(STR_BOOT_TYPE_STRING)
  // offset 0x080
  VARIABLE_EDIT_BOOT_TYPE_NAME_MAPPING  NameMapping[32];
  // offset 0x880
  UINT16                                BootTypeOrderDefaultSequence[32];
  // offset 0x8C0
  UINT16                                BootTypeOrderNewDefaultSequence[32];  // must same as BootTypeOrderDefaultSequence
  // offset 0x900
} VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL;

#pragma pack()

#define VARIABLE_EDIT_BOOT_TYPE_INFO_SIGNATURE      (SIGNATURE_64 ('$', 'U', 'V', 'E', 'T', 'I', 'S', 0))
#define VARIABLE_EDIT_BOOT_TYPE_INFO_VERSION        (0)
#define ARRAY_SIZE_OF_FIELD(s,m)                    (sizeof(((s *)0)->m) / sizeof((((s *)0)->m)[0]))

#define VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_CONTENT     \
  /* offset 0x000 */                                      \
  VARIABLE_EDIT_BOOT_TYPE_INFO_SIGNATURE,                 \
  VARIABLE_EDIT_BOOT_TYPE_INFO_VERSION,                   \
  sizeof (VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL),         \
  {0},                                                    \
  /* offset 0x020 */                                      \
  FALSE,                                                  \
  TRUE,                                                   \
  {0},                                                    \
  /* offset 0x030 */                                      \
  (UINT16) ARRAY_SIZE_OF_FIELD (VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL, BootTypeString),                  \
  (UINT16) ARRAY_SIZE_OF_FIELD (VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL, NameMapping),                     \
  (UINT16) ARRAY_SIZE_OF_FIELD (VARIABLE_EDIT_BOOT_TYPE_NAME_MAPPING,  BootTypeName),                    \
  (UINT16) ARRAY_SIZE_OF_FIELD (VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL, BootTypeOrderDefaultSequence),    \
  (UINT16) ARRAY_SIZE_OF_FIELD (VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL, BootTypeOrderNewDefaultSequence), \
  {0},                                                    \
  /* offset 0x040 */                                      \
  L"H2OUVE Boot Type",                                    \
  /* offset 0x080 */                                      \
  {                                                       \
    {BBS_FLOPPY,        L"H2OUVE FLOPPY"},                \
    {BBS_HARDDISK,      L"H2OUVE HARDDISK"},              \
    {BBS_CDROM,         L"H2OUVE CDROM"},                 \
    {BBS_PCMCIA,        L"H2OUVE PCMCIA"},                \
    {BBS_USB,           L"H2OUVE USB"},                   \
    {BBS_EMBED_NETWORK, L"H2OUVE EMBED_NETWORK"},         \
    {BBS_BEV_DEVICE,    L"H2OUVE BEV_DEVICE"},            \
    {OTHER_DRIVER,      L"H2OUVE OTHER_DRIVER"},          \
  },                                                      \
  /* offset 0x880 */                                      \
  {0},                                                    \
  /* offset 0x8C0 */                                      \
  {0},                                                    \
  /* offset 0x900 */

extern EFI_GUID gVariableEditBootTypeInfoProtocolGuid;

#endif
