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
// The followings are the sample codes. Please customize here.
//
/*
PRESERVED_VARIABLE_TABLE mPreservedVariableTable[] = {
//  { {0x1DA748E5, 0x3C39, 0x43E6, {0xB7, 0xAB, 0x55, 0xBA, 0xC7, 0x1A, 0xC7, 0xD2}}, L"SetupChange"}, // gH2OSetupChangeVariableGuid
//  { {0xA04A27f4, 0xDF00, 0x4D42, {0xB5, 0x52, 0x39, 0x51, 0x13, 0x02, 0x11, 0x3D}}, L"Setup"},       // SYSTEM_CONFIGURATION_GUID
#if COMPAL_EEPROM_SUPPORT 
  { {0xa86e3b62, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData0"},
  { {0xa86e3b0a, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData1"},
  { {0xa86e3b7b, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData2"},
  { {0xa86e3b39, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData3"},
  { {0xa86e3b23, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData4"},
  { {0xa86e3b56, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData5"},
  { {0xa86e3b17, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, L"EepromData6"},
#endif 
  { {0},   NULL}
};
*/


PRESERVED_VARIABLE_TABLE_2 mPreservedVariableTable2[] = {
//  VariableGuid    DeleteVariable    VariableName
//  { {0x7f9102df, 0xe999, 0x4740, {0x80, 0xa6, 0xb2, 0x03, 0x85, 0x12, 0x21, 0x7b}}, FALSE, L"SystemSupervisorPw"}, //[PRJ] gEfiSupervisorPwGuid, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean password when password in stored in variable.
//  { {0x8cf3cfd3, 0xd8e2, 0x4e30, {0x83, 0xff, 0xb8, 0x6f, 0x0c, 0x52, 0x2a, 0x5e}}, FALSE, L"SystemUserPw"}, // [PRJ]gEfiUserPwGuid, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean password when password in stored in variable.
//  { {0x1DA748E5, 0x3C39, 0x43E6, {0xB7, 0xAB, 0x55, 0xBA, 0xC7, 0x1A, 0xC7, 0xD2}}, FALSE, L"SetupChange"},  // gH2OSetupChangeVariableGuid
//  { {0xA04A27f4, 0xDF00, 0x4D42, {0xB5, 0x52, 0x39, 0x51, 0x13, 0x02, 0x11, 0x3D}}, FALSE, L"Setup"},        //[PRJ]SYSTEM_CONFIGURATION_GUID, DeleteVariable = FALSE to keep setup variable value after flash BIOS.
#if COMPAL_EEPROM_SUPPORT 
  { {0xa86e3b62, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData0"}, // [PRJ] VEEPROM Bank0, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
  { {0xa86e3b0a, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData1"}, // [PRJ] VEEPROM Bank1, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
  { {0xa86e3b7b, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData2"}, // [PRJ] VEEPROM Bank2, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
  { {0xa86e3b39, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData3"}, // [PRJ] VEEPROM Bank3, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
  { {0xa86e3b23, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData4"}, // [PRJ] VEEPROM Bank4, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
  { {0xa86e3b56, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData5"}, // [PRJ] VEEPROM Bank5, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
  { {0xa86e3b17, 0x28f4, 0x11e4, {0xAB, 0x4c, 0xa6, 0xc5, 0xe4, 0xd2, 0x2f, 0xb7}}, FALSE, L"EepromData6"}, // [PRJ] VEEPROM Bank6, DeleteVariable = TRUE to keep kernel behavior that flash BIOS will clean variable.
#endif
  { PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_GUID, TRUE, PRESERVED_VARIABLE_TABLE_2_OTHER_ALL_NAME},   // Put at here, Other all variable GUID and Other all variable name
  { {0}, FALSE, NULL}   // End of table
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
  *VariablePreservedTable2 = mPreservedVariableTable2;
  return EFI_MEDIA_CHANGED;

 // return EFI_UNSUPPORTED;
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
//	  *IsKeepVariableInList = TRUE;
//	
//	  *VariablePreservedTable = mPreservedVariableTable;
//	
//	  return EFI_MEDIA_CHANGED;

  return EFI_UNSUPPORTED;
}

