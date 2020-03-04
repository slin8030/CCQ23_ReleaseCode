/** @file
  FbtsLib Library Instance implementation

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
#include "IhisiOemSmm.h"
#include "CompalEcLib.h"
#include "IhisiOemFbts.h"
#include "CompalFlash.h"

UINT8                         mActionAfterFlashing = SYSTEM_DIRECTLY_REBOOT;


/**
  AH=16h, FbtsComplete.

  @retval EFI_SUCCESS        Function succeeded.
**/

EFI_STATUS
EFIAPI
OemFbtsComplete (
  VOID
  )
{
  DEBUG ((EFI_D_ERROR, "OemFbtsComplete mActionAfterFlashing = %x\n", mActionAfterFlashing));
  OemFlashCompletely();
  switch (mActionAfterFlashing) {
  case DoNothing:
      CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_SYSTEM_NOTIFI_CMD, EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_SHUTDOWN);
      break;
  case SYSTEM_DIRECTLY_SHUTDOWN:
  case SYSTEM_DIRECTLY_REBOOT:
      CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_SYSTEM_NOTIFI_CMD, EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_REBOOT);
      break;
  }

  return IHISI_SUCCESS;
}

EFI_STATUS
EFIAPI
PrjFetsAfterWrite (
  VOID
  )
{
  UINT8               DestBlockNo;

  //
  // Get platform.ini flashcomplete setting and pass to EC in OemFbtsComplete() before shutdown units. 
  // Tool bug: Winflash will flash over 128K size and only the last flash will pass the flashcomplete setting in CL. 
  //
  DestBlockNo = (UINT8)  (mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) >> 8);
  if (DestBlockNo >= 2) {
    mActionAfterFlashing = (UINT8)  mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  }
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "DestBlockNo : %x\n", DestBlockNo));

  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PrjFetsDoBeforeWrite ActionAfterFlashing : %x\n", mActionAfterFlashing));
  return IHISI_SUCCESS;
}

//
// FBTS Support Functions
//
STATIC
IHISI_REGISTER_TABLE
OEM_FBTS_REGISTER_TABLE[] = {
  //
  // AH=16h
  //
  { FBTSComplete,         "S16PRJFbtsComplete0", OemFbtsComplete      },
  //
  // AH=20h
  //
  { FETSWrite,            "S20PRJEcFlashAfter0", PrjFetsAfterWrite }
};

EFI_STATUS
InitOemFbts (
  VOID
  )
{
  EFI_STATUS              Status;
  IHISI_REGISTER_TABLE   *SubFuncTable;
  UINT16                  TableCount;

  SubFuncTable = OEM_FBTS_REGISTER_TABLE;
  TableCount = sizeof(OEM_FBTS_REGISTER_TABLE)/sizeof(OEM_FBTS_REGISTER_TABLE[0]);
  Status = RegisterIhisiSubFunction (SubFuncTable, TableCount);
  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

