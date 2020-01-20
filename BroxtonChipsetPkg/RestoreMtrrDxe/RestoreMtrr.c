/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2014 , Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/*++


Module Name:

 RestoreMtrr.c


--*/


#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Guid/MtrrDataInfo.h>
#define CACHE_VARIABLE_MTRR_BASE                                      0x00000200
#define IA32_MTRR_CAP                                                 0xFE

/**

  If plug memory > 16GB, Bios will calculate the MTRR setting value and push this value in Hob at PEI phase, 
  Here the Bios will get the MTRR setting value from Hob to replace present MTRR value to the > 16GB setting.  

  @param  ImageHandle           EFI_HANDLE
  @param  SystemTable           EFI_SYSTEM_TABLE pointer

  @retval 

**/
EFI_STATUS
EFIAPI
RestoreMtrrEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  UINT32                  NumberOfMtrr;
  UINT64                  TotalMemorySize = 0;
  UINT32                  Index;
  EFI_PEI_HOB_POINTERS    GuidHob;
  UINT8                   *MtrrRestoreData;


  GuidHob.Raw = GetHobList ();
  GuidHob.Raw = GetNextGuidHob ((CONST EFI_GUID*)&gMtrrDataInfoGuid, GuidHob.Raw);
  if (GuidHob.Raw != NULL) {
    MtrrRestoreData = (UINT8 *)GET_GUID_HOB_DATA (GuidHob.Guid);

    TotalMemorySize = ((HOB_MTRR_RESTORE_DATA*)MtrrRestoreData)->TotalMemorySize;
   	

      AsmWbinvd ();
      NumberOfMtrr = (UINT32)((AsmReadMsr64 (IA32_MTRR_CAP) & 0xFF) - 1);

      for (Index = 0; (Index <= NumberOfMtrr) && (Index < MTRR_MAX); Index++) {
        AsmWriteMsr64((UINT32)(CACHE_VARIABLE_MTRR_BASE+ 1 + (Index * 2)), 0);
        AsmWriteMsr64((UINT32)(CACHE_VARIABLE_MTRR_BASE + (Index * 2)), 0);
        //
        // Restore MTRR.
        //    
        AsmWriteMsr64((UINT32)(CACHE_VARIABLE_MTRR_BASE + (Index * 2)), ((HOB_MTRR_RESTORE_DATA*)MtrrRestoreData)->MtrrBase[Index]);
        AsmWriteMsr64((UINT32)(CACHE_VARIABLE_MTRR_BASE+ 1 + (Index * 2)), ((HOB_MTRR_RESTORE_DATA*)MtrrRestoreData)->MtrrSize[Index]);
      }

  }
  return EFI_SUCCESS;
}
