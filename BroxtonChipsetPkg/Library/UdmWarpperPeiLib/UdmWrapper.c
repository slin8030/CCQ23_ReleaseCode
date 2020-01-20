/** @file

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


//#include "usb.h"
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/DebugLib.h>
#include <Ppi/Stall.h>

typedef UINT32 uint32_t;
//
//Adopt EFI delay 
//
void delayMicro(uint32_t MicroSecond)
{
  EFI_STATUS                    Status;
  CONST EFI_PEI_SERVICES        **PeiServices;
  EFI_PEI_STALL_PPI             *StallPpi;

  PeiServices = GetPeiServicesTablePointer ();

  Status = (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **)&StallPpi);
  ASSERT(!EFI_ERROR(Status));

  StallPpi->Stall (PeiServices, StallPpi, MicroSecond);
  
  return;
}

void delay(uint32_t MicroSecond)
{
  EFI_STATUS                    Status;
  CONST EFI_PEI_SERVICES        **PeiServices;
  EFI_PEI_STALL_PPI             *StallPpi;

  PeiServices = GetPeiServicesTablePointer ();

  Status = (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **)&StallPpi);
  ASSERT(!EFI_ERROR(Status));

  StallPpi->Stall (PeiServices, StallPpi, MicroSecond);
  
  return;
}

void UdmWrapperInit(
  VOID
)
{
  return;
}



