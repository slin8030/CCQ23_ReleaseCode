/** @file

@copyright
 Copyright (c) 2009 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _PEI_SC_POLICY_UPDATE_H_
#define _PEI_SC_POLICY_UPDATE_H_

//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//
#include <PiPei.h>

#include <Guid/PlatformInfo.h>
#include <ChipsetSetupConfig.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/PeiServicesLib.h>
#include <Library/SteppingLib.h>
#include <ScAccess.h>
//[-start-160216-IB03090424-add]//
#include <Library/ScPlatformLib.h>
//
// Generic definitions for device Auto/enabling/disabling used by platform 
//
#define DEVICE_AUTO     2
#define DEVICE_ENABLE   1
#define DEVICE_DISABLE  0
//[-end-160216-IB03090424-add]//

#if 0
/**
  This function performs PCH USB Platform Policy initialzation

  @param[in]  PchUsbConfig     Pointer to PCH_USB_CONFIG data buffer
  @param[in]  SetupVariables   Pointer to Setup variable
  @param[in]  PlatformType     PlatformType specified
  @param[in]  PlatformFlavor   PlatformFlavor specified
  @param[in]  BoardType        BoardType specified
  @param[in]  BoardId          BoardId specified
**/
VOID
UpdatePchUsbConfig (
  IN PCH_USB_CONFIG            *PchUsbConfig,
  IN SYSTEM_CONFIGURATION      *SetupVariables,
  IN UINT8                     PlatformType,
  IN UINT8                     PlatformFlavor,
  IN UINT8                     BoardType,
  IN UINT16                    BoardId
  );
#endif

#endif
