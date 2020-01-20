/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/**@file
  Subsystem IDs setting for multiplatform.

  This file includes package header files, library classes.

  Copyright (c) 2010-2016, Intel Corporation. All rights reserved.<BR>
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.
**/

#include <Guid/PlatformInfo_Apl.h>
#include <Library/BaseMemoryLib.h>

//
// Default Vendor ID and Subsystem ID
//
#define SUBSYSTEM_VENDOR_ID1   0x8086
#define SUBSYSTEM_DEVICE_ID1   0x1999
#define SUBSYSTEM_SVID_SSID1   (SUBSYSTEM_VENDOR_ID1 + (SUBSYSTEM_DEVICE_ID1 << 16))

#define SUBSYSTEM_VENDOR_ID2   0x8086
#define SUBSYSTEM_DEVICE_ID2   0x1888
#define SUBSYSTEM_SVID_SSID2   (SUBSYSTEM_VENDOR_ID2 + (SUBSYSTEM_DEVICE_ID2 << 16))

#define SUBSYSTEM_VENDOR_ID   0x8086
#define SUBSYSTEM_DEVICE_ID   0x1234
#define SUBSYSTEM_SVID_SSID   (SUBSYSTEM_VENDOR_ID + (SUBSYSTEM_DEVICE_ID << 16))

EFI_STATUS
InitializeBoardSsidSvid (
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN EFI_PLATFORM_INFO_HOB        *PlatformInfoHob
  );
