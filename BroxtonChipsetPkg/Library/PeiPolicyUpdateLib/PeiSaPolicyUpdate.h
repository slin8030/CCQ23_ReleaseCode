/** @file

@copyright
  Copyright (c) 2013 - 2014 Intel Corporation. All rights reserved
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
#ifndef _PEI_SA_POLICY_UPDATE_H_
#define _PEI_SA_POLICY_UPDATE_H_

//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//
//#include <PlatformBoardId.h>
//#include <PlatformDefinitions.h>
#include <ChipsetSetupConfig.h>
#include <SaAccess.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Ppi/ReadOnlyVariable2.h>
//#include <Setup/SaSetup.h>
#include <Ppi/SaPolicy.h>
#include <Library/PeiServicesLib.h>

extern EFI_GUID gVbtInfoGuid;

typedef struct {
  EFI_PHYSICAL_ADDRESS    VbtAddress;
  UINT32                  VbtSize;
} VBT_INFO;

EFI_STATUS
EFIAPI 
PeiGetSectionFromFv (
  IN CONST  EFI_GUID        NameGuid,
  OUT VOID                  **Address,
  OUT UINT32               *Size
  );
#endif

