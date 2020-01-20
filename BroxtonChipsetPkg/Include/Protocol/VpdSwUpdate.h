/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    VpdSwUpdate.h

Abstract:

    EFI Smm VPD update protocol

 

--*/

#ifndef _EFI_VPD_SW_UPDATE_H_
#define _EFI_VPD_SW_UPDATE_H_


//
// Global ID for the SW SMI Protocol
//

#define EFI_VPD_SW_UPDATE_PROTOCOL_GUID    \
  { 0xe541b773, 0xdd11, 0x420c, 0xb0, 0x26, 0xdf, 0x99, 0x36, 0x53, 0xf8, 0x22 }

//EFI_INTERFACE_DECL(_EFI_VPD_SW_UPDATE_PROTOCOL);


//
// Related Definitions
//
#define SW_SMI_PORT     0xb2

#define FW_WRITE        0x42
#define FW_UPDATE       0x43
#define INT15_0D042H    0x44
#define BIS_DATA_READ   0x45
#define BIS_DATA_WRITE  0x46
#define SMM_PnP_BIOS_CALL   0x47
#define INT15_VARS      0xEF // don't change this or you'll break utilities

//
// Member functions
//



//
// Interface structure for the SMM Software SMI Dispatch Protocol
//

//typedef struct _EFI_VPD_SW_UPDATE_PROTOCOL {
//  
//} EFI_VPD_SW_UPDATE_PROTOCOL;

extern EFI_GUID gEfiVpdSwUpdateProtocolGuid;

#endif
