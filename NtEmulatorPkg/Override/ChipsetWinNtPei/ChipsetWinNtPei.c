/** @file
  WinNt driver

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <PiPei.h>
#include <Library/PeimEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#define __HOB__H__
#include <Guid/XTUInfoHob.h>

/**
  Initialize the state information for ChipsetWinNtDriver

  @param[in] ImageHandle          Image handle of the loaded driver
  @param[in] SystemTable          Pointer to the System Table

  @retval EFI_SUCCESS             Thread can be successfully created
  @retval EFI_OUT_OF_RESOURCES    Cannot allocate protocol data structure
  @retval EFI_DEVICE_ERROR        Cannot create the timer service
**/
EFI_STATUS
EFIAPI
ChipsetWinNtPeiInit (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  XTU_INFO_DATA                 *XtuInfo;

  XtuInfo = (XTU_INFO_DATA *) BuildGuidHob (&gXTUInfoHobGuid, sizeof (XTU_INFO_DATA));
  ZeroMem (XtuInfo, sizeof (XTU_INFO_DATA));

  return EFI_SUCCESS;
}
