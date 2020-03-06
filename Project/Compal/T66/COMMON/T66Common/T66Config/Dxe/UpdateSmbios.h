/** @file
  Private function to Update Smbios

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __UPDATE_SMBIOS_DXE_H__
#define __UPDATE_SMBIOS_DXE_H__

#include "T66ConfigDxe.h"
#include "Library/BaseLib.h"

/**
 Update Smbios data

 @param[in] ImageHandle
 @param[in] SystemTable

 @retval EFI_SUCCESS            The function is executed successfully.
**/
EFI_STATUS
EFIAPI
UpdateSmbios (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
);

#endif
