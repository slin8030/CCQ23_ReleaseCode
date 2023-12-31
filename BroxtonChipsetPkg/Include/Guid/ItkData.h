/*++

Copyright (c)  1999 - 2005 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  ItkData.h

Abstract:

--*/

#ifndef _ITKDATAHUB_GUID_H_
#define _ITKDATAHUB_GUID_H_

// This GUID is for the ITK related data found in the Data Hub {E7060843-A336-4d5b-9598-13402F5D7375}

#define ITK_DATA_HUB_GUID \
  { 0xe7060843, 0xa336, 0x4d5b, 0x95, 0x98, 0x13, 0x40, 0x2f, 0x5d, 0x73, 0x75 }

extern EFI_GUID gItkDataHubGuid;

// This GUID is for the ITK related data found in a Variable  {3812723D-7E48-4e29-BC27-F5A39AC94EF1}

#define ITK_DATA_VAR_GUID \
  { 0x3812723d, 0x7e48, 0x4e29, 0xbc, 0x27, 0xf5, 0xa3, 0x9a, 0xc9, 0x4e, 0xf1 }

extern EFI_GUID gItkDataVarGuid;

#define ITK_DATA_VAR_NAME L"ItkDataVar"

extern CHAR16 gItkDataVarName[];

#define ITK_BIOS_MOD_VAR_NAME L"ItkBiosModVar"

extern CHAR16 gItkBiosModVarName[];

#pragma pack(1)
typedef struct {
  UINT32    Type;
  UINT32    RecordLength;
} EFI_ITK_DATA_HEADER;

typedef struct {
  EFI_ITK_DATA_HEADER   ItkHeader;
  UINT32                HecetaAddress;
} EFI_ITK_HECETA_ADDRESS;

typedef struct {
  UINT16    VarEqName;
  UINT16    VarEqValue;
} EFI_ITK_VAR_EQ_RECORD;

typedef struct {
  EFI_ITK_DATA_HEADER   ItkHeader;
  EFI_ITK_VAR_EQ_RECORD VarEqRecord[0x10000];
} EFI_ITK_VAR_EQ;
#pragma pack()

#define EFI_ITK_HECETA_ADDRESS_TYPE    1
#define EFI_ITK_MOBILE_BIOS_TYPE       2
#define EFI_ITK_VAR_EQ_TYPE            3

#endif
