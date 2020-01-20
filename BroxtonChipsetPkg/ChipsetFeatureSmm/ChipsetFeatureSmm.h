/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  ChipsetFeatureSmm.h

Abstract:

  Header file for
 
++*/

#ifndef _CHIPSET_FEATURE_SMM_H
#define _CHIPSET_FEATURE_SMM_H

#include <PiSmm.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmIchnDispatch.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/LoadedImage.h>
#include "Protocol/GlobalNvsArea.h"
#include <Guid/AcpiVariableCompatibility.h>
#include <ChipsetSetupConfig.h>
#include <IndustryStandard/Pci22.h>
#include "ScAccess.h"
#include "CpuRegs.h"
#include "ChipsetCmos.h"
//[-start-160923-IB07400789-add]//
#include <Library/CmosLib.h>
//[-end-160923-IB07400789-add]//
#include "PlatformBaseAddresses.h"
#include "SaRegs.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/SmmOemSvcKernelLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SmmServicesTableLib.h>
#include <ChipsetSmiTable.h>
#include <Protocol/SmmCpu.h>
#include <Library/HeciMsgLib.h>
#include <SeCAccess.h>
#include <ChipsetPostCode.h>
#include <Protocol/CpuIo2.h>
#include <Library/PostCodeLib.h>
#include <SmiTable.h> 
#include <PostCode.h>
//[-start-160923-IB07400789-add]//
#include <Protocol/AcpiRestoreCallbackDone.h> 
#include <Protocol/OverrideAspm.h>
#include <Protocol/AcpiEnableCallbackDone.h>
#include <Protocol/AcpiDisableCallbackDone.h>
#include <Library/PciLib.h>
//[-end-160923-IB07400789-add]//
//[-start-170315-IB07400848-add]//
#include <Protocol/SmmVariable.h>
//[-end-170315-IB07400848-add]//

EFI_STATUS
ChipsetFeatureSwSmiCallBack (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  );

#endif

