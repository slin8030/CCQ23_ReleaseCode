/*++

Copyright (c)  2003, 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  FirmwareId.h

--*/

#ifndef _FirmwareId_h_GUID_included
#define _FirmwareId_h_GUID_included

//#include "Tiano.h"

#pragma pack(1)

//
// Firmware ID
//

#define FIRMWARE_ID_MAX_LENGTH      35

typedef struct {
  CHAR8    BiosId[8];
  CHAR8    Separator1;
  CHAR8    OemId[3];
  CHAR8    Separator2;
  CHAR8    BuildId[4];
  CHAR8    Separator3;
  CHAR8    Century[2];
  CHAR8    Year[2];
  CHAR8    Separator4;
  CHAR8    Month[2];
  CHAR8    Date[2];
  CHAR8    Separator5;
  CHAR8    Hour[2];
  CHAR8    Minute[2];
  CHAR8    Dummy[3];
} FIRMWARE_ID_DATA;

#define  OLD_FIRMWARE_ID_GUID \
  {0xefc071ae, 0x41b8, 0x4018, 0xaf, 0xa7, 0x31, 0x4b, 0x18, 0x5e, 0x57, 0x8b}

#define  FIRMWARE_ID_GUID \
  {0x5e559c23, 0x1faa, 0x4ae1, 0x8d, 0x4a, 0xc6, 0xcf, 0x02, 0x6c, 0x76, 0x6f}

#define FIRMWARE_ID_NAME L"FirmwareId"
#define FIRMWARE_ID_NAME_WITH_PASSWORD FIRMWARE_ID_NAME L"H#8,^-!t"

extern EFI_GUID gFirmwareIdGuid;
extern CHAR16   gFirmwareIdName[];

#pragma pack()

#endif

