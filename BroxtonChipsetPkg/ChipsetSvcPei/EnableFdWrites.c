/** @file
 PEI Chipset Services Library.

 This file contains only one function that is PeiCsSvcEnableFdWrites().
 The function PeiCsSvcEnableFdWrites() use chipset services to enable/disable 
 flash device write access.

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

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PciCf8Lib.h>
#include <Protocol/SmmFwBlockService.h>
#include <ScAccess.h>
#include <PlatformBaseAddresses.h>


//
// function prototype defines
//
EFI_STATUS
EnableFvbWrites (
  BOOLEAN EnableWrites
  );

/**
 Platform specific function to enable/disable flash device write access.

 @param[in]         EnableWrites        TRUE  - Enable
                                        FALSE - Disable
                    
 @retval            EFI_SUCCESS         Function returns successfully
*/
EFI_STATUS
EFIAPI
EnableFdWrites (
  IN  BOOLEAN           EnableWrites
  )
{
  EFI_STATUS   Status;
  Status = EnableFvbWrites(TRUE);
  return Status;
}
