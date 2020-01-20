/** @file
  Content file contains function definitions for Variable Edit Driver.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#ifndef _VARIABLE_EDIT_H_
#define _VARIABLE_EDIT_H_

#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiInternalFormRepresentation.h>
#include <Csm/LegacyBiosDxe/LegacyBiosInterface.h>
#include <StdLib/StdCLibSupport.h>

#include <Library/SetupUtilityLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/VariableLib.h>
#include <Library/DxeOemSvcKernelLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/LegacyRegion2.h>
#include <Protocol/SetupUtilityBrowser.h>
#include <Protocol/SysPasswordService.h>
#include <Protocol/SetupUtilityApplication.h>
#include <Protocol/SetupUtility.h>

#include <Guid/H2oUveConfigUtilHii.h>

typedef enum {
  FormsetGuidFlagFormsetGuid      = 0,
  FormsetGuidFlagFormsetClassGuid = 1,
  FormsetGuidFlagAll              = 2
} FORMSET_GUID_FLAG;

#define VARIABLE_EDIT_IDENTIFY_GUID \
  { \
    0x7a633d68, 0x33f8, 0x428d, { 0xa8, 0xb9, 0x87, 0xf8, 0x37, 0x27, 0x06, 0x21 } \
  }

#define VARIABLE_EDIT_PTR_VER        4
// Version 1 for initial version
// Version 2 for Boot device / order by James
// Version 3 for Load Default Call Back ID By CH
// Version 4 for Support H2OFormBrowser

#define VARIABLE_EDIT_PTR_SIGNATURE     SIGNATURE_64 ('$', 'I', 'F', 'R', 'P', 'K', 'G', VARIABLE_EDIT_PTR_VER)
#define FILE_VERSION_SIGNATURE          SIGNATURE_64 ('$', 'I', 'F', 'R', 'P', 'K', 'G', 0x21)
#define UVE_DATA_INFO_SIGNATURE         SIGNATURE_64 ('$', 'U', 'V', 'E', 'I', 'N', 'F', 'O')

#define F0000Region                     0x01
#define E0000Region                     0x02
#define VARIABLE_EDIT_PTR_ALIGNMENT     0x10
#define VARIABLE_EDIT_TABLE_MAX_ADDRESS 0xFFFFFFFF  // 4G
#define VARIABLE_EDIT_TABLE_SIGNATURE   VARIABLE_EDIT_PTR_SIGNATURE
#define FORMSET_ORDER_MAX               0xFFFF
#define FLAG_VAR_EDIT_TBL_H2O_FORM_BROWSER_SUPPORTED    0x1

#define FIELD_SIZE(s,m)                 (sizeof ((((s *) 0)->m)))
#define EFI_FIELD_OFFSET(TYPE,Field)    ((UINTN)(&(((TYPE *) 0)->Field)))

#define IS_VARIABLE_EDIT_PTR(p)          ((p)->Signature == VARIABLE_EDIT_PTR_SIGNATURE)
#define IS_VARIABLE_EDIT_TABLE(p)        ((p)->Signature == VARIABLE_EDIT_TABLE_SIGNATURE)

#pragma pack(1)

typedef struct _FILE_VERSION {
  UINT64                Signature;
  UINT32                MajorVersion;
  UINT32                MinorVersion;
} FILE_VERSION;

typedef struct _VFR_STR_OFFSET_PAIR {
  UINT32                StrPkgOffset;
  UINT32                VfrBinOffset;
} VFR_STR_OFFSET_PAIR;

typedef struct _VARIABLE_EDIT_PTR {
  UINT64                Signature;
  UINT32                Address;
  UINT32                Size;
} VARIABLE_EDIT_PTR;

typedef struct _VARIABLE_EDIT_TABLE {
  UINT64                Signature;
  UINT8                 SmiNumber;
  UINT8                 BootTypeOrderSize;
  UINT16                BootTypeOrderOffset;
  UINT32                OemDevOffset;          //v2  // Use this offset to get OEM_VAREDIT_DATA (No matter it is ZERO or not, check the signature)
  UINT16                BootTypeOrderStringID; //v2
  UINT32                VfrStrPkgOffset;       //v2
  UINT16                LoadDefaultCallBackID; //v3
  UINT16                Flag;                  //v4
  UINT16                RootFormsetCount;      //v4
  UINT8                 Reserve1[48];
  UINT32                PairCount;
} VARIABLE_EDIT_TABLE;

typedef struct _PACKAGE_LIST_INFO {
  VOID                  *PackageList;
  UINTN                 Size;
  EFI_HII_HANDLE        HiiHandle;
  EFI_HII_PACKAGE_HEADER *pFormsetHdr;
  EFI_HII_PACKAGE_HEADER *pStringHdr;
  UINT16                Order;
} PACKAGE_LIST_INFO;

typedef struct _PACKAGE_LIST {
  PACKAGE_LIST_INFO     *ListInfo;
  UINTN                 RootCount;
  UINTN                 ListCount;
} PACKAGE_LIST;

typedef struct _UVE_DATA_INFO {
  UINT64                Signature;
  UINT32                Version;
  UINT8                 Reserve[4];
  UINT8                 H2OFormBrowserSupported;
  UINT8                 Reserve1[7];
  UINT64                ScuFormsetGuidListAddr;
  UINT64                ScuFormsetFlagListAddr;
} UVE_DATA_INFO;

#pragma pack()

#endif

