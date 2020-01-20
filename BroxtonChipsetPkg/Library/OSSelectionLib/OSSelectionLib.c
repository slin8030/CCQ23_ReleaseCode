/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/
/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  OSSelectionLib.c
    
Abstract:


--*/

//
// Include files
//


#include "PiDxe.h"
#include "Pi/PiSmmCis.h"
#include <Guid/SetupVariable.h>
#include <Guid/MemoryTypeInformation.h>
#include <Library/OSSelectionLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#define   OS_SELECTION_ITEM_END                 ((UINTN)-1)
#define   OS_SELECTION_ANDROID                  0
#define   OS_SELECTION_WINDOWS                  1
#define   OS_SELECTION_UNKNOWN                  2
#define   SYSTEM_CONFIGURATION_OFFSET(Field)    (UINTN)((CHAR8*)&((SYSTEM_CONFIGURATION*)0)->Field - (CHAR8*)0)

typedef struct {
  UINTN          Offset;
  UINT8          Data;
}OS_SELECTION_ITEM;

OS_SELECTION_ITEM mOSSelectionTable_Windows[] = {
  {SYSTEM_CONFIGURATION_OFFSET(ISPDevSel),       1},  //ACPI mode, sub-device of Device 2
  {SYSTEM_CONFIGURATION_OFFSET(PchUsbOtg),       0},  //Disable
  {SYSTEM_CONFIGURATION_OFFSET(PchUsbVbusOn),    1},  //Enable
  {SYSTEM_CONFIGURATION_OFFSET(EnableDptf),      1},  //Enable
  {SYSTEM_CONFIGURATION_OFFSET(fTPM),            1},  //Enable
  {SYSTEM_CONFIGURATION_OFFSET(PavpMode),        1},  //PAVP is set to lite mode
  {SYSTEM_CONFIGURATION_OFFSET(LpssPwm1Enabled), 0},  //Disable
  {SYSTEM_CONFIGURATION_OFFSET(PreOsSelection),  OS_SELECTION_WINDOWS},
  {OS_SELECTION_ITEM_END, 0}
};
OS_SELECTION_ITEM mOSSelectionTable_Android[] = {
  {SYSTEM_CONFIGURATION_OFFSET(ISPDevSel),       2},  //PCI mode, B0:D3:F0
  {SYSTEM_CONFIGURATION_OFFSET(PchUsbOtg),       1},  //Enable and in PCI mode
  {SYSTEM_CONFIGURATION_OFFSET(PchUsbVbusOn),    0},  //Disable
  {SYSTEM_CONFIGURATION_OFFSET(EnableDptf),      0},  //Disable
  {SYSTEM_CONFIGURATION_OFFSET(fTPM),            0},  //Disable
  {SYSTEM_CONFIGURATION_OFFSET(PavpMode),        2},  //PAVP is set to heavy mode
  {SYSTEM_CONFIGURATION_OFFSET(LpssPwm1Enabled), 1},  //Enable
  {SYSTEM_CONFIGURATION_OFFSET(PreOsSelection),  OS_SELECTION_ANDROID},
  {OS_SELECTION_ITEM_END, 0}
};

UINTN 
OSSelectByPlatform(
  )
{
  return OS_SELECTION_UNKNOWN;
}

UINTN
OSSelectByConfig(
  SYSTEM_CONFIGURATION *Config
  )
{
  UINTN OSSelection;
  OSSelection = OSSelectByPlatform();
  switch(Config->OsSelection) {
    case 0:
    OSSelection = OS_SELECTION_WINDOWS;
    break;
  case 1:
    OSSelection = OS_SELECTION_ANDROID;
    break;
  default:
    OSSelection = OS_SELECTION_UNKNOWN;
	break;
  }
  return OSSelection;
}

UINTN 
OSCheck(
  SYSTEM_CONFIGURATION *Config
  )
{
  UINTN OSSelection;
  OSSelection = OSSelectByPlatform();
  if (OSSelection != OS_SELECTION_UNKNOWN) {
  	return OSSelection;
  }
  return OSSelectByConfig(Config);
}

BOOLEAN
OSConfigNeedUpdate(
  VOID * PlatformConfig
  )
{
  SYSTEM_CONFIGURATION *Config;
  UINTN				  OsSelection;
  Config = (SYSTEM_CONFIGURATION*)PlatformConfig;
  OsSelection = OSCheck(Config);
  return (OsSelection != Config->PreOsSelection);
}

VOID UpdateOSConfig(
  VOID *PlatformConfig
  ) 
{
  SYSTEM_CONFIGURATION *Config;
  UINTN                 OsSelection;
  OS_SELECTION_ITEM    *OsSelectionTable;
  UINTN                 Index;
  EFI_STATUS            Status;

  Config = (SYSTEM_CONFIGURATION*)PlatformConfig;
  OsSelection = OSCheck(Config);
  switch(OsSelection) {
  	case OS_SELECTION_ANDROID:
		OsSelectionTable = mOSSelectionTable_Android;
		break;
	case OS_SELECTION_WINDOWS:
		OsSelectionTable = mOSSelectionTable_Windows;
		break;
	default:
		OsSelectionTable = NULL;
		break;
  }

  if (OsSelectionTable == NULL) {
  	return;
  }
  for (Index = 0; OsSelectionTable[Index].Offset != OS_SELECTION_ITEM_END; Index ++) {
  	*((UINT8*)Config + OsSelectionTable[Index].Offset) = OsSelectionTable[Index].Data;
  }
  //
  // Clear the MemoryTypeInformation variable
  //
  Status = gRT->SetVariable(
                  EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME,
                  &gEfiMemoryTypeInformationGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  0,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
}
