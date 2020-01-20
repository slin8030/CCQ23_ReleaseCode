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

#define PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_GUID      {0xffffffff, 0xffff, 0xffff, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,0xff, 0xff}}
#define PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_NAME      L"PreservedTable2OtherAllName"

//
// Old funcion OemSvcVariablePreservedTable
// The followings are the sample codes for old function OemSvcVariablePreservedTable. Please customize here.
//
//#define SampleGuid { 0x11111111, 0x1111, 0x1111, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }
//PRESERVED_VARIABLE_TABLE mPreservedVariableTable[] = {
//  { SampleGuid, L"SampleVariable" },
//  { {0},   NULL}
//};

//
// New funcion OemSvcVariablePreservedTable2
// The followings are the sample codes for new function OemSvcVariablePreservedTable2 . Please customize here.
//
// PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_GUID : Other all variable GUID
// PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_NAME : Other all variable name
// If DeleteVariable is FALSE : Variable reserve.
// If DeleteVariable is TRUE  : Variable remove.
// The order of priority within the table is from top to bottom.
// The special item "PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_GUID, FALSE or TRUE, PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_NAME" needs to be placed at the last
// and before the end of table line.
// #define SampleGuid { 0x11111111, 0x1111, 0x1111, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }
//
//
//PRESERVED_VARIABLE_TABLE_2 mPreservedVariableTable2[] = {
//  VariableGuid    DeleteVariable    VariableName
//  { SampleGuid,            FALSE,   L"SampleVariable" },
//  { PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_GUID, FALSE, PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_NAME},   // put at here, other all variable GUID and Other all variable name
//  { {0}, FALSE, NULL}   // End of table
//};

PRESERVED_VARIABLE_TABLE_2 mPreservedVariableTable2[] = {
//  VariableGuid    DeleteVariable    VariableName
  { {0x7f9102df, 0xe999, 0x4740, {0x80, 0xa6, 0xb2, 0x03, 0x85, 0x12, 0x21, 0x7b}}, TRUE, L"SystemSupervisorPw"}, // gEfiSupervisorPwGuid, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean password when password in stored in variable.
  { {0x8cf3cfd3, 0xd8e2, 0x4e30, {0x83, 0xff, 0xb8, 0x6f, 0x0c, 0x52, 0x2a, 0x5e}}, TRUE, L"SystemUserPw"}, // gEfiUserPwGuid, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean password when password in stored in variable.
  { PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_GUID, FALSE, PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_NAME},   // Put at here, Other all variable GUID and Other all variable name
  { {0}, FALSE, NULL}   // End of table
};

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
  *VariablePreservedTable2 = mPreservedVariableTable2;
  return EFI_MEDIA_CHANGED;

//  return EFI_UNSUPPORTED;
}

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
  //*IsKeepVariableInList = TRUE;
  //*VariablePreservedTable = mPreservedVariableTable;
  //return EFI_MEDIA_CHANGED;

  return EFI_UNSUPPORTED;
}