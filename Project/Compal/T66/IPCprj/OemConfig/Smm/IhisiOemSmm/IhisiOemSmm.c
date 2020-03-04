/** @file
  SMM driver implementation for the IHISI Services

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Protocol/H2OIhisi.h"
#include <Library/SmmServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include "IhisiOemSmm.h"
#include "IhisiOemFbts.h"

H2O_IHISI_PROTOCOL                    *mH2OIhisi = NULL;

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
  )
{
  EFI_STATUS                    Status;
  PCD_IHISI_REGISTER_TABLE     *PcdRegisterTable;
  UINT16                        PcdRegisterTableCount;
  UINT16                        PcdRegisterTableIndex;
  UINT16                        SubFuncTableIndex;
  UINT8                         PcdCommand;
  CHAR8                         String[20];
  Status = EFI_SUCCESS;
  PcdRegisterTable = (PCD_IHISI_REGISTER_TABLE *)PcdGetPtr (PcdIhisiRegisterTable);
  PcdRegisterTableCount = FixedPcdGetPtrSize (PcdIhisiRegisterTable) / sizeof (PcdRegisterTable[0]);
  //
  //  Register IHISI sub function if subfunction CmdNumber/AsciiFuncGuid define in PcdIhisiRegisterTable list.
  //
  for (SubFuncTableIndex = 0; SubFuncTableIndex < TableCount; SubFuncTableIndex++) {
    for (PcdRegisterTableIndex = 0; PcdRegisterTableIndex < PcdRegisterTableCount; PcdRegisterTableIndex++) {
      PcdCommand = PcdRegisterTable[PcdRegisterTableIndex].CmdNumber;
      CopyMem (String, PcdRegisterTable[PcdRegisterTableIndex].FuncSignature, 20);
      if (SubFuncTable[SubFuncTableIndex].CmdNumber == PcdRegisterTable[PcdRegisterTableIndex].CmdNumber) {
        if (AsciiStrCmp (SubFuncTable[SubFuncTableIndex].FuncSignature,PcdRegisterTable[PcdRegisterTableIndex].FuncSignature) == 0) {
          Status = mH2OIhisi->RegisterCommand (SubFuncTable[SubFuncTableIndex].CmdNumber ,
                                               SubFuncTable[SubFuncTableIndex].IhisiFunction,
                                               PcdRegisterTable[PcdRegisterTableIndex].Priority);
          if (EFI_ERROR(Status)) {
            DEBUG ((EFI_D_ERROR, "IHISI command :0x%X, priority : 0x%X, that already has a registered function\n", SubFuncTable[SubFuncTableIndex].CmdNumber,PcdRegisterTable[PcdRegisterTableIndex].Priority));
            ASSERT (FALSE);
          }
          break;
        }
      }
    }
  }
  return Status;
}


/**
  IhisiSmm module entry point

  @param ImageHandle            A handle for the image that is initializing this driver
  @param SystemTable            A pointer to the EFI system table

  @retval EFI_SUCCESS:          Module initialized successfully
  @retval Others                Module initialized unsuccessfully
**/
EFI_STATUS
EFIAPI
IhisiOemSmmEntry(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS    Status;

  if (!InSmm ()) {
    return EFI_SUCCESS;
  }

  Status = gSmst->SmmLocateProtocol (
                  &gH2OIhisiProtocolGuid,
                  NULL,
                  (VOID **)&mH2OIhisi
                  );

  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  Status = InitOemFbts();
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
