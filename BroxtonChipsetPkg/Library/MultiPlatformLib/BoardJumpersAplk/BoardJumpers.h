/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/**@file
  Jumper setting for multiplatform.

  This file includes package header files, library classes.

  Copyright (c) 2010 - 2016 Intel Corporation. All rights reserved.<BR>
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.
**/

#ifndef _BOARDJUMPERS_H_
#define _BOARDJUMPERS_H_

//////////////////////////////////////////////////////////////////////
#include <PiPei.h>
#include "ScAccess.h"
#include "PlatformBaseAddresses.h"

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Guid/PlatformInfo_Apl.h>

BOOLEAN
IsRecoveryJumper (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB *PlatformInfoHob
);

#endif
