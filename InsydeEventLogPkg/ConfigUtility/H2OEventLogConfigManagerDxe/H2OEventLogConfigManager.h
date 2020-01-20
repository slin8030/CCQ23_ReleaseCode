/** @file
  Header file for H2O Event Log Config Manager

;******************************************************************************
;* Copyright (c) 2016 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _H2O_EVENT_LOG_CONFIG_MANAGER_H_
#define _H2O_EVENT_LOG_CONFIG_MANAGER_H_

#include <H2OEventLogConfigManagerSetupConfig.h>
#include <Library/DebugLib.h>
//[-start-171208-IB08400539-modify]//
//#include <Library/SetupUtilityLib.h>
#include <Guid/MdeModuleHii.h>
//[-end-171208-IB08400539-modify]//
#include <Library/DevicePathLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HiiLib.h>
#include <Library/HiiExLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Protocol/SetupUtilityApplication.h>
//[-start-180518-IB08400594-modify]//
#include <Protocol/H2ODialog.h>
//[-end-180518-IB08400594-modify]//

//
// This is the generated IFR binary data for each formset defined in VFR.
// This data array is ready to be used as input of HiiAddPackages() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8  H2OEventLogConfigManagerVfrBin[];

#define H2O_EVENT_LOG_CONFIG_MAMAGER_PRIVATE_SIGNATURE SIGNATURE_32 ('H', 'E', 'L', 'C')

#pragma pack(1)
///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH             VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL       End;
} HII_VENDOR_DEVICE_PATH;

typedef struct {
  UINTN                              Signature;

  EFI_HANDLE                         DriverHandle;
  EFI_HII_HANDLE                     HiiHandle;

  //
  // Consumed protocol
  //
  EFI_HII_DATABASE_PROTOCOL          *HiiDatabase;
  EFI_HII_STRING_PROTOCOL            *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL    *HiiConfigRouting;
  EFI_FORM_BROWSER2_PROTOCOL         *FormBrowser2;

  //
  // Produced protocol
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL     ConfigAccess;
} H2O_EVENT_LOG_CONFIG_MANAGER_PRIVATE_DATA;

#pragma pack()

#define H2O_EVENT_LOG_CONFIG_MANAGER_PRIVATE_FROM_THIS(a)  CR (a, H2O_EVENT_LOG_CONFIG_MANAGER_PRIVATE_DATA, ConfigAccess, H2O_EVENT_LOG_CONFIG_MAMAGER_PRIVATE_SIGNATURE)

#endif
