/** @file
   Provide OEM to set BIOS region protection for FOTA update process.
   
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

#include <Library/DxeOemSvcChipsetLib.h>

/**
  Customize BIOS protect region for FOTA update process. 

  @param[out]        *BiosRegionTable    Pointer to BiosRegion Table.
  @param[out]        ProtectRegionNum    The number of Bios protect region instances.
  
  @retval            EFI_UNSUPPORTED     Returns unsupported by default, OEM did not implement this function.
  @retval            EFI_MEDIA_CHANGED   Provide table for kernel to set protect region and lock flash program registers.
                                         This table will be freed by kernel.
**/
EFI_STATUS
OemSvcFotaBiosProtectTable (
  OUT BIOS_PROTECT_REGION           **BiosRegionTable,
  OUT UINT8                         *ProtectRegionNum
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/
#if 0 // Sample Implementation
  //
  // When OEM needs to add protect region, this map provides the protected BIOS region.
  // Other, OEM just redefine these PCDs in your projectpkg .dsc file.
  // These protected regions will be locked by BiosProtect Driver and only can be updated by Secure Flash mechanism 
  //
  UINT8         BiosProtectRegionNumber;
  
  FreePool (*BiosRegionTable);
  *BiosRegionTable = NULL;
  BiosProtectRegionNumber = 3;
  
  if (BiosRegionTable == NULL ||  ProtectRegionNum == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*BiosRegionTable != NULL) {
    //
    // We will allocate a new one, so free it.
    //
    FreePool (*BiosRegionTable);
    *BiosRegionTable = NULL;
  } 
  
  *BiosRegionTable = AllocateZeroPool (sizeof (BIOS_PROTECT_REGION) * (BiosProtectRegionNumber));

  if (*BiosRegionTable == NULL) {
    DEBUG ((EFI_D_INFO, "BiosRegionTable allocate memory resource failed.\n"));
    return EFI_OUT_OF_RESOURCES;
  }  

  (*BiosRegionTable)[0].Base = PcdGet32 (PcdFlashFvMainBase);
  (*BiosRegionTable)[0].Size = PcdGet32 (PcdFlashFvMainSize);
  (*BiosRegionTable)[1].Base = PcdGet32 (PcdFlashFvRecoveryBase);
  (*BiosRegionTable)[1].Size = PcdGet32 (PcdFlashFvRecoverySize);
  (*BiosRegionTable)[2].Base = PcdGet32 (PcdFlashNvStorageMicrocodeBase);
  (*BiosRegionTable)[2].Size = PcdGet32 (PcdFlashNvStorageMicrocodeSize);

  *ProtectRegionNum = BiosProtectRegionNumber;
  
  return EFI_MEDIA_CHANGED;
#endif

  return EFI_UNSUPPORTED;
}