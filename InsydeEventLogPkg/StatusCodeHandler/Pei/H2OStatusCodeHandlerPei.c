/** @file
  Implementation of H2OStatusCodeHandlerPei module.

;******************************************************************************
;* Copyright (c) 2016-2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/** @file
  Report Status Code Handler PEIM which produces general handlers and hook them
  onto the PEI status code router.

  Copyright (c) 2009, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "H2OStatusCodeHandlerPei.h"

EFI_PEI_NOTIFY_DESCRIPTOR mReadOnlyVariable2PpiNotify = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  ReadOnlyVariable2Callback
};

/**
  Entry point of Status Code PEIM.
  
  This function is the entry point of this Status Code PEIM.
  It initializes supported status code devices according to PCD settings,
  and installs Status Code PPI.

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCESS  The entry point of DXE IPL PEIM executes successfully.

**/
EFI_STATUS
EFIAPI
StatusCodeHandlerPeiEntry (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                  Status;
  EFI_PEI_RSC_HANDLER_PPI     *RscHandlerPpi;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariablePpi;

  Status = PeiServicesLocatePpi (
             &gEfiPeiRscHandlerPpiGuid,
             0,
             NULL,
             (VOID **) &RscHandlerPpi
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Dispatch initialization request to sub-statuscode-devices.
  // If enable UseSerial, then initialize serial port.
  // if enable UseMemory, then initialize memory status code worker.
  //
  if (FeaturePcdGet (PcdStatusCodeUseSerial)) {
    Status = SerialPortInitialize();
    ASSERT_EFI_ERROR (Status);
    Status = RscHandlerPpi->Register (SerialStatusCodeReportWorker);                     
    ASSERT_EFI_ERROR (Status);

    Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariablePpi);
    if (EFI_ERROR (Status)) {
      Status = PeiServicesNotifyPpi (&mReadOnlyVariable2PpiNotify); 
      ASSERT_EFI_ERROR (Status);
    } else {
//[-start-161109-IB04560702-modify]//
      UpdateStatusCodePolicy (VariablePpi);
//[-end-161109-IB04560702-modify]//
    }
  }
  if (FeaturePcdGet (PcdStatusCodeUseMemory)) {
    Status = MemoryStatusCodeInitializeWorker ();
    ASSERT_EFI_ERROR (Status);
    Status = RscHandlerPpi->Register (MemoryStatusCodeReportWorker);                     
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

