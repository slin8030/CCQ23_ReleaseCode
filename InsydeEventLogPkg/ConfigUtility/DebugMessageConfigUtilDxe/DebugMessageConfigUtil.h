/** @file
  Header file for DEBUG Message Config utility DXE

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

#ifndef _DEBUG_MESSAGE_CONFIG_UTIL_H_
#define _DEBUG_MESSAGE_CONFIG_UTIL_H_

#include <DebugMessageConfigUtilSetupConfig.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
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
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/PcdLib.h>

#include <Protocol/HiiConfigRouting.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Protocol/SetupUtilityApplication.h>
//[-start-180518-IB08400594-modify]//
#include <Protocol/H2ODialog.h>
//[-end-180518-IB08400594-modify]//
#include <Protocol/EventLog.h>
#include <Protocol/EventLogString.h>
#include <Protocol/EventLogStringOverwrite.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>

#include <Guid/DebugMessageVariable.h>

//
// This is the generated IFR binary data for each formset defined in VFR.
// This data array is ready to be used as input of HiiAddPackages() to
// create a packagelist (which contains Form packages, String packages, etc).
//
extern UINT8 DebugMessageConfigUtilConfigPageVfrBin[];
extern UINT8 DebugMessageConfigUtilEventAndMessagePageVfrBin[];

#define DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_SIGNATURE SIGNATURE_32 ('D', 'M', 'C', 'U')
#define MAX_STORAGE_NAME_NUM                    10
//[-start-180724-IB08400617-remove]//
//#define MAX_ELV_STR_NUM                         200
//[-end-180724-IB08400617-remove]//

#define ELV_LIST_BASE                           0x1200
#define MAX_SHOW_ELV_LIST_NUM                   100
#define VTS_LEFT_ALIGN                          0
#define VTS_RIGHT_ALIGN                         1
#define VTS_LEAD_0                              2

typedef enum {
  SHOW_FIRST_PAGE = 0,
  SHOW_PREVIOUS_PAGE = 1,
  SHOW_NEXT_PAGE = 2
} SHOW_EVENT_LOG_ACTION;

typedef struct {
  CHAR8                           *DebugMessage;
  UINTN                           DebugMsgLen;
} DEBUG_MESSAGE_STRING;

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

  EFI_HANDLE                         DriverHandle[2];
  EFI_HII_HANDLE                     HiiHandle[2];
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION Configuration;

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
//[-start-180507-IB08400587-add]//
  EFI_HII_CONFIG_ACCESS_PROTOCOL     ConfigAccess2;
//[-end-180507-IB08400587-add]//
} DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_DATA;

#pragma pack()

#define DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_FROM_THIS(a)  CR (a, DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_DATA, ConfigAccess, DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_SIGNATURE)

#endif
