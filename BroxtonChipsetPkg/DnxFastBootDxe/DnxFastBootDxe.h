//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
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

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include "Library/PmicLib.h"
#include "PmicReg.h"
#include "PlatformBaseAddresses.h"
#include <ScAccess.h>
#include <Library/IoLib.h>
#include <Library/Usbd.h>

typedef struct _DNX_FB_DEVICE_CONTEXT{
	EFI_EVENT	TimerEvent;
} DNX_FB_DEVICE_CONTEXT;

EFI_STATUS
EFIAPI
DnXFastBoot (
  VOID
  );

BOOLEAN
EFIAPI
DnXFastBootPwrBtnGetLastWakeSource (
	VOID
	);


