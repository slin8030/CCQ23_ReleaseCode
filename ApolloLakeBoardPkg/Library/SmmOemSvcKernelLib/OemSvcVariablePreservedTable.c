/** @file
  To return the preserved list.

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmOemSvcKernelLib.h>

//
// The followings are the sample codes. Please customize here.
//
PRESERVED_VARIABLE_TABLE mPreservedVariableTable[] = {
  { {0},   NULL}
};
//[-start-151123-IB11270134-add]//
/**
  New function OemSvcVariablePreservedTable2 to return new structure (PRESERVED_VARIABLE_TABLE_2) preserved list.
  The process will launch new function OemSvcVariablePreservedTable2 first,
  and then launch old function OemSvcVariablePreservedTable only when new function OemSvcVariablePreservedTable2
  return EFI_UNSUPPORTED.
  According to logo requirement, should preserve all UEFI variables with VendorGuid
  {77fa9abd-0359-4d32-bd60-28f4e78f784b}, so the those UEFI variables with this VendorGuidVendorGuid
  will not be limited in this table when if table is filled with those special variables to remove in table, 
  this table parse process will skip those.

  @param[in, out]  VariablePreservedTable2    Pointer to the table of preserved variables.
                                              (new structure PRESERVED_VARIABLE_TABLE_2)
  @retval          EFI_UNSUPPORTED           Returns unsupported by default.
  @retval          EFI_SUCCESS               The work to delete the variables is completed.
  @retval          EFI_MEDIA_CHANGED         The table of preserved variables is updated.
**/
EFI_STATUS
OemSvcVariablePreservedTable2 (
  IN OUT PRESERVED_VARIABLE_TABLE_2            **VariablePreservedTable2
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  //
  // The followings are the sample codes. Please customize here.
  //
  //*VariablePreservedTable2 = mPreservedVariableTable2;
  //return EFI_MEDIA_CHANGED;

  return EFI_UNSUPPORTED;
}

//[-end-151123-IB11270134-add]//

/**
  Old function OemSvcVariablePreservedTable to return the old sturcture (PRESERVED_VARIABLE_TABLE) preserved list.
  The process will launch new function OemSvcVariablePreservedTable2 first,
  and then launch old function OemSvcVariablePreservedTable only when new function OemSvcVariablePreservedTable2
  return EFI_UNSUPPORTED.
  According to logo requirement, should preserve all UEFI variables with VendorGuid
  {77fa9abd-0359-4d32-bd60-28f4e78f784b}, so the those UEFI variables with this VendorGuidVendorGuid
  will not be limited in this table when if table is filled with those special variables to remove in table, 
  this table parse process will skip those.

  @param[in, out]  VariablePreservedTable    Pointer to the table of preserved variables.
                                             (old structure PRESERVED_VARIABLE_TABLE)
  @param[in, out]  IsKeepVariableInList      True: The variables in the table should be preserved.
                                             FALSE: The variables in the table should be deleted.

  @retval          EFI_UNSUPPORTED           Returns unsupported by default.
  @retval          EFI_SUCCESS               The work to delete the variables is completed.
  @retval          EFI_MEDIA_CHANGED         The table of preserved variables is updated.
**/
EFI_STATUS
OemSvcVariablePreservedTable (
  IN OUT PRESERVED_VARIABLE_TABLE              **VariablePreservedTable,
  IN OUT BOOLEAN                               *IsKeepVariableInList
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  //
  // The followings are the sample codes. Please customize here.
  //
  *IsKeepVariableInList = TRUE;

  *VariablePreservedTable = mPreservedVariableTable;

  return EFI_MEDIA_CHANGED;
}

