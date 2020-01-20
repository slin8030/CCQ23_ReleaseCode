/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/

/*++

Copyright (c)  2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  IsctNvsArea.h

Abstract:

  Note:  Data structures defined in this protocol are not naturally aligned.

--*/
#ifndef _RESERVED_NVS_AREA_H_
#define _RESERVED_NVS_AREA_H_

//
// Reserved NVS Area Protocol GUID
//
#define EFI_RESERVED_NVS_AREA_PROTOCOL_GUID \
  { \
    0x6614a586, 0x788c, 0x47e2, 0x89, 0x2d, 0x72, 0xe2, 0xc, 0x34, 0x48, 0x90 \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gEfiReservedNvsAreaProtocolGuid;


#pragma pack(1)
typedef struct {
//
// Isct NVS Area definition
//
  UINT8       IsctWakeReason;      //(0):  Wake Reason
                                   //      Bits   Description 
                                   //      0      User pressed power button or HID event: 0 = False, 1 = True
                                   //      1      Periodic Wake BIOS timer (EC or RTC) (requested by iSCT agent): 0 = False, 1 = True
                                   //      2      Non-Periodic Wake RTC timer: 0 = False, 1 = True
                                   //      3      Wake due to PME (Unkown Device Type): 0 = False, 1 = True
                                   //      4      Internal BIOS bit PME (Known Network Wake): 0 = False, 1 = True (Reserved for Agent- Must be 0)
                                   //      5 - 7  Reserved
  UINT8       IsctEnabled;         //(1):  1 - Enabled, 0 - Disabled 
  UINT32      IsctNvsPtr;          //(2-5): Ptr of Isct GlobalNvs
  UINT8       IsctOverWrite;       //(6): 1 - Isct , 0 - OS RTC
  UINT8       Reserved[33];        //(7 - 40) : Reserved for ISCT  

//
// DPTF NVS Area definition
//

} EFI_RESERVED_NVS_AREA;
#pragma pack()

//
// Isct NVS Area Protocol
//
typedef struct _ISCT_NVS_AREA_PROTOCOL {
  EFI_RESERVED_NVS_AREA     *Area;
  VOID                      *IsctData;  
} ISCT_NVS_AREA_PROTOCOL;

#endif
