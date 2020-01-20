/** @file
  To return the preserved list.

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

#include <ChipsetFota.h>

//
// The followings are the sample codes. Please customize here.
//
FOTA_PRESERVED_VARIABLE_TABLE mFotaPreservedVariableTable[] = {
  { {0},   NULL}
};

/**
  To return the preserved list.

  @param[in, out]  VariablePreservedTable    Pointer to the table of preserved variables.
  @param[in, out]  IsKeepVariableInList      True: The variables in the table should be preserved.
                                             FALSE: The variables in the table should be deleted.

  @retval          EFI_UNSUPPORTED           Returns unsupported by default.
  @retval          EFI_SUCCESS               The work to delete the variables is completed.
  @retval          EFI_MEDIA_CHANGED         The table of preserved variables is updated.
**/
EFI_STATUS
OemSvcFotaVariablePreservedTable (
  IN OUT FOTA_PRESERVED_VARIABLE_TABLE         **VariablePreservedTable,
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
  *VariablePreservedTable = mFotaPreservedVariableTable;

  return EFI_MEDIA_CHANGED;
}

