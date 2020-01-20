/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _IHISI_SMM_H_
#define _IHISI_SMM_H_

#include "Fbts.h"
//[-start-150506-IB10860198-add]//
//[-start-161215-IB08450359-remove]//
//#include "H2oUveSmi.h"
//[-end-161215-IB08450359-remove]//
//[-end-150506-IB10860198-add]//
#include <H2OIhisi.h>
#include <Library/DebugLib.h>
#include <Library/SmmOemSvcChipsetLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/H2OIhisi.h>
#include <Protocol/SmmFwBlockService.h>
#include <Library/BaseLib.h>

extern H2O_IHISI_PROTOCOL                   *mH2OIhisi;
extern EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL    *mSmmFwBlockService;

/**
  Register IHISI sub function if SubFuncTable CmdNumber/AsciiFuncGuid define in PcdIhisiRegisterTable list.

  @param[out] SubFuncTable        Pointer to ihisi register table.
  @param[out] TableCount          SubFuncTable count

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
RegisterIhisiSubFunction (
  IHISI_REGISTER_TABLE         *SubFuncTable,
  UINT16                        TableCount
  );


#endif
