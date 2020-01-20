/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
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

#ifndef _OS_HOB_HEADER_
#define _OS_HOB_HEADER_

#include <Uefi.h>
#include <Library/BaseLib.h>
//#include <CmosMap.h>

#define OSNIB_LENGTH         0x40
#define OSNIB_SIGNATURE      0x0042494E //'N''I''B''\0'

#pragma pack(1)

typedef struct {
  UINT32         Signature;
  UINT8          MajorVer;
  UINT8          MinorVer;
  UINT16         Rsvd0;  
} OSNIB_HEADER;

typedef enum{
  ModeMain      =   0x00,
  ModeCharging  =   0x0A,
  ModeRecovery  =   0x0C,  
  ModeFastBoot  =   0x0E,  
  ModeFactory   =   0x12,  
  ModeNotUse    =   0xF0,
} TARGET_MODE_ATTR;


typedef struct {
  OSNIB_HEADER      OsNibHeader;          // byte0
  UINT8             WdtResetCount   :4 ;  // byte8
  UINT8             Rsvd1           :4 ;  
  UINT8             Rsvd2[20];            // byte9~28
  TARGET_MODE_ATTR  TargetMode;           // byte29
  UINT8             Rsvd3           :6 ;  // byte30
  UINT8             FwUpdate        :1 ;  
  UINT8             RtcAlarmCharger :1 ;  
  UINT8             CheckSum;             // byte31
  UINT8             OemReserved[32];
} OSNIB_64B;

#pragma pack()


#endif

