/** @file
 DXE Chipset Services Library.
 	
 This file contains only one function that is DxeCsSvcSetIsaInfo().
 The function DxeCsSvcSetIsaInfo() use chipset services to set ISA
 information for common ISA driver utilization.
	
***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

//[-start-160818-IB03090432-modify]//
#include "ScRegs/RegsLpc.h"
//[-end-160818-IB03090432-modify]//

/**
 To set ISA information for common ISA driver utilization.

 @param[in, out]    Function            ISA function value
 @param[in, out]    Device              ISA device value

 @retval            EFI_SUCCESS         This function always return successfully.
*/
EFI_STATUS
SetIsaInfo (
  IN OUT UINT8    *Function,
  IN OUT UINT8    *Device
  )
{
//[-start-160818-IB03090432-modify]//
  *Device   = (UINT8)PCI_DEVICE_NUMBER_PCH_LPC;
  *Function = (UINT8)PCI_FUNCTION_NUMBER_PCH_LPC;
//[-end-160818-IB03090432-modify]//

  return EFI_SUCCESS;
}