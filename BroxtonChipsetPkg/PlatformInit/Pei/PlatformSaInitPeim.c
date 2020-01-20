/*++

Copyright (c) 2012 - 2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformSaInitPeim.c

Abstract:

  Do Early System Agent platform initialization.

--*/

#include "PlatformInit.h"

VOID
PlatformSaInit (
  IN CHIPSET_CONFIGURATION        *SystemConfiguration,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Perform SSA related platform initialization.

--*/
{

//[-start-151229-IB03090424-modify]//
  DEBUG ((EFI_D_INFO, "PlatformSaInit() - Start\n"));
  DEBUG ((EFI_D_INFO, "PlatformSaInit() - SystemConfiguration->IpuEn 0x%x\n",SystemConfiguration->IpuEn));
  if(SystemConfiguration->IpuEn == DEVICE_ENABLE) {

  }
//[-end-151229-IB03090424-modify]//


  DEBUG ((EFI_D_INFO, "PlatformSaInit() - End\n"));

  return;
}
