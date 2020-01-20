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

#include "IhisiSmm.h"
#include <Library/BaseMemoryLib.h>

H2O_IHISI_PROTOCOL                   *mH2OIhisi;
EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL    *mSmmFwBlockService;

/**
  Ihisi driver entry point to initialize all of IHISI relative services

  @param[in] ImageHandle        The firmware allocated handle for the UEFI image.
  @param[in] SystemTable        A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
*/
EFI_STATUS
EFIAPI
IhisiEntryPoint (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;

  Status = gSmst->SmmLocateProtocol (
                  &gH2OIhisiProtocolGuid,
                  NULL,
                  (VOID **)&mH2OIhisi
                  );

  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  Status = EFI_SUCCESS;

  if (mSmmFwBlockService == NULL) {
    Status = gSmst->SmmLocateProtocol (
                     &gEfiSmmFwBlockServiceProtocolGuid,
                     NULL,
                     (VOID **)&mSmmFwBlockService
                     );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = FbtsInit ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

//[-start-150506-IB10860198-add]//
//[-start-161215-IB08450359-remove]//
//  Status = H2oUveSmiServiceInit ();
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//[-end-161215-IB08450359-remove]//
//[-end-150506-IB10860198-add]//
  return EFI_SUCCESS;
}

/**
  Register IHISI sub function if SubFuncTable CmdNumber/AsciiFuncGuid define in PcdIhisiRegisterTable list.

  @param[out] SubFuncTable        Pointer to ihisi register table.
  @param[out] TableCount          SubFuncTable count

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
RegisterIhisiSubFunction (
  IHISI_REGISTER_TABLE          *SubFuncTable,
  UINT16                        TableCount
  )
{
//[-start-161213-IB-8450358-modify]//
  EFI_STATUS                    Status;
  UINT8                         *PcdTable;
  UINT8                         PcdPriority;
  UINT8                         EndChar;
  UINTN                         Index;
  UINTN                         PcdCount;
  UINTN                         PcdMaxCount;
  UINTN                         SignatureSize;
  BOOLEAN                       PcdFoundRegistered;

  Status             = EFI_SUCCESS;
  EndChar            = 0;
  PcdFoundRegistered = FALSE;

  PcdTable    = (UINT8 *)PcdGetPtr (PcdIhisiRegisterTable);
  PcdMaxCount = FixedPcdGetPtrSize (PcdIhisiRegisterTable) / sizeof (UINT8);
  if ((FixedPcdGetPtrSize (PcdIhisiRegisterTable) % sizeof (PCD_IHISI_REGISTER_TABLE)) != 0) {
    DEBUG ((EFI_D_ERROR, "PcdIhisiRegisterTable of description not follow PCD_IHISI_REGISTER_TABLE definition, \
                          it may cause some of IHISI function register fail \n"));
  }

  for (Index = 0; Index < TableCount; Index ++) {
    PcdCount   = 0;
    PcdPriority = 0x80;
    PcdFoundRegistered = FALSE;
    SignatureSize = AsciiStrLen (SubFuncTable[Index].FuncSignature);

    //
    // Caculate PCD of address to find 1. CmdNumber 2. FuncSignature 3. Priority
    //
    do {
      if (SubFuncTable[Index].CmdNumber == *(PcdTable + PcdCount)) {
        PcdCount++;
        if (AsciiStrnCmp (SubFuncTable[Index].FuncSignature, (CHAR8 *) (PcdTable + PcdCount), SignatureSize) == 0) {
          if (EndChar == *(PcdTable + PcdCount + SignatureSize)) {
            PcdPriority = *(PcdTable + PcdCount + SignatureSize + 1);
            PcdFoundRegistered = TRUE;
            break;
          }
        }
      }
      PcdCount++;
    } while (PcdCount < PcdMaxCount);

    if (PcdFoundRegistered) {
      Status = mH2OIhisi->RegisterCommand (SubFuncTable[Index].CmdNumber, SubFuncTable[Index].IhisiFunction, PcdPriority);
      if (EFI_ERROR(Status)) {
        DEBUG ((EFI_D_ERROR, "IHISI command :0x%X, priority : 0x%X, that already has a registered function\n", SubFuncTable[Index].CmdNumber, PcdPriority));
        ASSERT (FALSE);
      }
    }
  }

//[-end-161213-IB-8450358-modify]//
  return Status;
}


