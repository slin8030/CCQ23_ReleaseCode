/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
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

  IsctPersistentData.h
    
Abstract:

  GUID definition for the Isct Persistent Data

--*/
#ifndef _ISCT_PERSISTENT_DATA_H_
#define _ISCT_PERSISTENT_DATA_H_

// GUID {69A20012-B167-4e35-A999-98EE0835F02E}
#define ISCT_PERSISTENT_DATA_GUID \
  { \
    0x69a20012, 0xb167, 0x4e35, 0xa9, 0x99, 0x98, 0xee, 0x8, 0x35, 0xf0, 0x2e \
  }

#define ISCT_PERSISTENT_DATA_NAME L"IsctData"

extern EFI_GUID gIsctPersistentDataGuid;

#pragma pack(1)
typedef struct {
  UINT32      IsctNvsPtr;
} ISCT_PERSISTENT_DATA;
#pragma pack()

#endif
