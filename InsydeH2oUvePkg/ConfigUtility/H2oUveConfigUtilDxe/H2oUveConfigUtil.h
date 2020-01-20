/** @file
  Header file for H2oUve Config utility DXE

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _H2OUVE_CONFIG_UTIL_H_
#define _H2OUVE_CONFIG_UTIL_H_

#include <H2oUveConfigUtilSetupConfig.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/SetupUtilityLib.h>
#include <Library/DevicePathLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Protocol/SetupUtilityApplication.h>

//
// This is the generated IFR binary data for each formset defined in VFR.
// This data array is ready to be used as input of HiiAddPackages() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8 H2oUveConfigUtilConfigPageVfrBin[];

#define H2OUVE_CONFIG_UTIL_PRIVATE_SIGNATURE SIGNATURE_32 ('U', 'V', 'E', 'C')

#pragma pack(1)
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

typedef struct {
  UINTN                                    Signature;

  EFI_HANDLE                               DriverHandle;
  EFI_HII_HANDLE                           HiiHandle;
  VARIABLE_EDIT_FUNCTION_DISABLE_SETTING   Configuration;

  //
  // Consumed protocol
  //
  EFI_HII_DATABASE_PROTOCOL                *HiiDatabase;
  EFI_HII_STRING_PROTOCOL                  *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL          *HiiConfigRouting;
  EFI_FORM_BROWSER2_PROTOCOL               *FormBrowser2;

  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL           ConfigAccess;
} H2OUVE_CONFIG_UTIL_PRIVATE_DATA;

#pragma pack()

#endif
