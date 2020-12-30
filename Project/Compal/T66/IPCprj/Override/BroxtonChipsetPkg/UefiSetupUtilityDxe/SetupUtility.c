/** @file

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

#include "SetupUtility.h"

//[-start-161215-IB08450359-remove]//
//#include <Protocol/VariableEdit.h>
//[-end-161215-IB08450359-remove]//
#include <Library/CmosLib.h>

#include "Main/PlatformInfo.h"
//[-start-150506-IB10860198-add]//
//[-start-161215-IB08450359-remove]//
//#include <Protocol/VariableEditBootTypeInfo.h>
//VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL mVariableEditBootTypeInfoData = { VARIABLE_EDIT_BOOT_TYPE_INFO_PROTOCOL_CONTENT };
//[-end-161215-IB08450359-remove]//
//[-end-150506-IB10860198-add]//
#include <Library/DxeInsydeChipsetLib.h>

//[-start-170721-IB07400892-add]//
#define H2O_UVE_INITIALIZE_SETUP_UTILITY  0x02
#define H2O_UVE_SHUTDOWN_SETUP_UTILITY    0x03
//[-end-170721-IB07400892-add]//

#include <SetupConfig.h>
//[PRJ]+ >>>> Hidden T66 unsupported items in SCU and add T66 showAllPage function
#include "Library/DxeProjectSvcLib.h"
//[PRJ]+ <<<< Hidden T66 unsupported items in SCU and add T66 showAllPage function

UINT8 mSetupOptionStatusForFullReset = NoChanged;

EFI_HII_HANDLE                   mDriverHiiHandle;
extern UINT32 mScuRecord;

//[PRJ]++ Fix Set PXE boot first function by CMFC fail
VOID
SetBDPVFlag (
  IN OUT CHIPSET_CONFIGURATION    *SetupNvData
  );

#define LEGACY_SUPPORT_ENABLED    0
#define LEGACY_SUPPORT_DISABLED   2
#define PXE_DISABLE               0
#define PXE_ENABLE                1
#define PXE_PROTOCOL_IPV4         0
#define PXE_PROTOCOL_IPV6         1
#define PXE_PROTOCOL_IPV4_IPV6    2
#define PXE_PROTOCOL_LEGACY       3
#define ADDPOSITION_FIRST         0
//[PRJ]-- Fix Set PXE boot first function by CMFC fail

//[-start-151123-IB11270134-remove]//
// //
// // Declare informaiton for EZH2O tool use
// //
// #define MAX_STRING_PACKAGE_NUMBER 2
// #define DRIVER_VFR_NUMBER         2
// #define LIBRARY_FVR_NUMBER        4
// #define TOTAL_VFR_NUMBER          (DRIVER_VFR_NUMBER + LIBRARY_FVR_NUMBER)
// 
// #if defined(_M_IX86) || defined(__i386__)
// #define IFR_PACKAGE_FIELD(Name, Size) UINT32 Name[Size * 2]
// #define IFR_PACKAGE_VALUE(Name)       (UINT32)(UINTN) Name, 0
// #else
// #define IFR_PACKAGE_FIELD(Name, Size) UINT64 Name[Size]
// #define IFR_PACKAGE_VALUE(Name)       (UINT64)(UINTN) Name
// #endif
// 
// #pragma pack(push, 1)
// typedef struct {
//   UINT8  Id[7];
//   UINT8  HIIVersion;
//   UINT32 EZH2OVersion;
//   UINT32 StringPackageNumber;
//   IFR_PACKAGE_FIELD(VfrNumber,                    MAX_STRING_PACKAGE_NUMBER);
//   IFR_PACKAGE_FIELD(StringPackageoffset,          MAX_STRING_PACKAGE_NUMBER);
//   IFR_PACKAGE_FIELD(VfrBinOffset,                 TOTAL_VFR_NUMBER);
//   IFR_PACKAGE_FIELD(VfrDefaultOffset,             TOTAL_VFR_NUMBER);
//   IFR_PACKAGE_FIELD(StringPackageTotalSizeOffset, MAX_STRING_PACKAGE_NUMBER);
// } EFI_IFR_PACKAGE_HEAD;
// #pragma pack(pop)
// 
// 
// EFI_IFR_PACKAGE_HEAD     IFR_PACKAGE_HEADER =
//                            { {'$', 'I', 'F', 'R', 'P', 'K', 'G'},
//                              0x21,
//                              0x02,
//                              MAX_STRING_PACKAGE_NUMBER,
//                              {IFR_PACKAGE_VALUE(DRIVER_VFR_NUMBER), IFR_PACKAGE_VALUE(LIBRARY_FVR_NUMBER)},
//                              {IFR_PACKAGE_VALUE(SetupUtilityStrings),               IFR_PACKAGE_VALUE(SetupUtilityLibStrings)},
//                              {IFR_PACKAGE_VALUE(AdvanceVfrBin),                     IFR_PACKAGE_VALUE(PowerVfrBin),
//                               IFR_PACKAGE_VALUE(MainVfrBin),                        IFR_PACKAGE_VALUE(SecurityVfrBin),
//                               IFR_PACKAGE_VALUE(BootVfrBin),                        IFR_PACKAGE_VALUE(ExitVfrBin)},
//                              {IFR_PACKAGE_VALUE(AdvanceVfrSystemConfigDefault0000), IFR_PACKAGE_VALUE(PowerVfrSystemConfigDefault0000),
//                               IFR_PACKAGE_VALUE(MainVfrSystemConfigDefault0000),    IFR_PACKAGE_VALUE(SecurityVfrSystemConfigDefault0000),
//                               IFR_PACKAGE_VALUE(BootVfrSystemConfigDefault0000),    IFR_PACKAGE_VALUE(0)},
//                              {IFR_PACKAGE_VALUE(&SetupUtilityStringsTotalSize),     IFR_PACKAGE_VALUE(&SetupUtilityLibStringsTotalSize)}
//                            };
//[-end-151123-IB11270134-remove]//


USER_UNINSTALL_CALLBACK_ROUTINE mUninstallCallbackRoutine[] ={
  UninstallExitCallbackRoutine, 
  UninstallBootCallbackRoutine, 
  UninstallPowerCallbackRoutine, 
  UninstallSecurityCallbackRoutine, 
  UninstallAdvanceCallbackRoutine, 
  UninstallMainCallbackRoutine
  };

EFI_GUID  mFormSetGuid      = SYSTEM_CONFIGURATION_GUID;
EFI_GUID  mFormSetClassGuid = SETUP_UTILITY_FORMSET_CLASS_GUID;
CHAR16    mVariableName[]   = L"SystemConfig";


EFI_STATUS
InitSetupUtilityBrowser(
  IN  EFI_SETUP_UTILITY_PROTOCOL        *This
  );

SETUP_UTILITY_BROWSER_DATA              *gSUBrowser;
UINT16                                  gCallbackKey;

//
// Hii vendor device path template
//
HII_TEMP_DEVICE_PATH  mHiiVendorDevicePathTemplate = {
  {
    {
      {
        HARDWARE_DEVICE_PATH,
        HW_VENDOR_DP,
        (UINT8) (sizeof (HII_VENDOR_DEVICE_PATH_NODE)),
        (UINT8) ((sizeof (HII_VENDOR_DEVICE_PATH_NODE)) >> 8)
      },
      EFI_IFR_TIANO_GUID,
    },
    0,
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH,
    0
  }
};

//[-start-150506-IB10860198-add]//
//[-start-161215-IB08450359-remove]//
//EFI_STATUS
//EFIAPI
//InstallVariableEditBootTypeInfo  (
//  IN UINT8                          *SetupNvData
//  )
//{
//  EFI_STATUS                        Status = EFI_SUCCESS;
//  EFI_HII_HANDLE                    Handle = NULL;
//  UINT8                            *DefaultLegacyBootTypeOrder;
//  UINTN                             Index = 0;
//  KERNEL_CONFIGURATION             *KernelConfig = (VOID *)SetupNvData;
//
//  DefaultLegacyBootTypeOrder = (UINT8 *) PcdGetPtr (PcdLegacyBootTypeOrder);
//  if (DefaultLegacyBootTypeOrder != NULL) {
//    for (Index = 0; DefaultLegacyBootTypeOrder[Index] != 0 && Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
//      mVariableEditBootTypeInfoData.BootTypeOrderDefaultSequence[Index] = DefaultLegacyBootTypeOrder[Index];
//    }
//  }
//  if (KernelConfig != NULL) {
//    for (Index = 0; Index < MAX_BOOT_ORDER_NUMBER; ++Index) {
//      mVariableEditBootTypeInfoData.BootTypeOrderNewDefaultSequence[Index] = KernelConfig->BootTypeOrder[Index];
//    }
//  }
//  Status = gBS->InstallProtocolInterface (
//                 &Handle,
//                 &gVariableEditBootTypeInfoProtocolGuid,
//                 EFI_NATIVE_INTERFACE,
//                 &mVariableEditBootTypeInfoData
//                 );
//  return Status;
//}
//[-end-161215-IB08450359-remove]//
//[-end-150506-IB10860198-add]//

/**
 The HII driver handle passed in for HiiDatabase.NewPackageList() requires
 that there should be DevicePath Protocol installed on it.
 This routine create a virtual Driver Handle by installing a vendor device
 path on it, so as to use it to invoke HiiDatabase.NewPackageList().

 @param [in]     DriverHandle         Handle to be returned

 @retval EFI_SUCCESS            Handle destroy success.
 @retval EFI_OUT_OF_RESOURCES   Not enough memory.

**/
EFI_STATUS
CreateHiiDriverHandle (
  OUT EFI_HANDLE               *DriverHandle
  )
{
  EFI_STATUS                   Status;
  HII_VENDOR_DEVICE_PATH_NODE  *VendorDevicePath;

  VendorDevicePath = AllocateCopyPool (sizeof (HII_TEMP_DEVICE_PATH), &mHiiVendorDevicePathTemplate);
  if (VendorDevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Use memory address as unique ID to distinguish from different device paths
  //
  VendorDevicePath->UniqueId = (UINT64) ((UINTN) VendorDevicePath);

  *DriverHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  VendorDevicePath,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DestroyHiiDriverHandle (
  IN EFI_HANDLE                 DriverHandle
  )
/*++

Routine Description:
  Destroy the Driver Handle created by CreateHiiDriverHandle().

Arguments:
  DriverHandle - Handle returned by CreateHiiDriverHandle()

Returns:
  EFI_SUCCESS - Handle destroy success.
  other       - Handle destroy fail.

--*/
{
  EFI_STATUS                   Status;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;

  Status = gBS->HandleProtocol (
                  DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->UninstallProtocolInterface (
                  DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  DevicePath
                  );
  gBS->FreePool (DevicePath);
  return Status;
}

EFI_STATUS
SetupUtilityInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  SETUP_UTILITY_DATA                    *SetupData;
  EFI_EVENT                             Event;
  VOID                                  *Registration;
  CHIPSET_CONFIGURATION                 *SetupNvData;

//[-start-161215-IB08450359-remove]//
//  VARIABLE_EDIT_PROTOCOL               *VariableEditor;
//  EFI_HANDLE                            Handle;
//[-end-161215-IB08450359-remove]//

//[-start-161215-IB08450359-remove]//
//  VariableEditor   =  NULL;
//  Handle           =  NULL;
//[-end-161215-IB08450359-remove]//

//[-start-151123-IB11270134-remove]//
//   //
//   // Do not remove, it is for compiler optimization
//   //
//   IFR_PACKAGE_HEADER.Id[0] = '$';
//[-end-151123-IB11270134-remove]//
  
  CheckLanguage ();
  
  SetupData = AllocatePool (sizeof(SETUP_UTILITY_DATA));
  if (SetupData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  SetupData->Signature                    = EFI_SETUP_UTILITY_SIGNATURE;
  SetupData->SetupUtility.StartEntry      = NULL;
  SetupData->SetupUtility.PowerOnSecurity = PowerOnSecurity;
  mSetupOptionStatusForFullReset = NoChanged;

  //
  // The FirstIn flag is for that if there is the first time entering SCU, we should install
  // Vfr of Menu to Hii database.
  // After that, we shouldn't install the Vfr to Hii database again.
  //
  SetupData->SetupUtility.FirstIn = TRUE;
  Status = GetSystemConfigurationVar (SetupData);
  SetupNvData = (CHIPSET_CONFIGURATION *) SetupData->SetupUtility.SetupNvData;

  //
  // Install Setup Utility
  //
  SetupData->Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                 &SetupData->Handle,
                 &gEfiSetupUtilityProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &SetupData->SetupUtility
                 );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (SetupData);
    return Status;
  }
//[-start-161215-IB08450359-remove]//
//  if (SetupNvData->H2OUVESupport) {
//    VariableEditor = AllocatePool (sizeof (VARIABLE_EDIT_PROTOCOL));
//    if (VariableEditor == NULL) {
//      return EFI_OUT_OF_RESOURCES;
//    }
//    VariableEditor->InstallSetupHii = InstallSetupHii;
//    Status = gBS->InstallProtocolInterface (
//                   &Handle,
//                   &gVariableEditProtocolGuid,
//                   EFI_NATIVE_INTERFACE,
//                   VariableEditor
//                   );
//    if (EFI_ERROR (Status)) {
//      gBS->FreePool (VariableEditor);
//      return Status;
//    }
//  }
////[-start-150506-IB10860198-add]//
//  InstallVariableEditBootTypeInfo (SetupData->SetupUtility.SetupNvData);
////[-end-150506-IB10860198-add]//
//[-end-161215-IB08450359-remove]//
  //
  // When execute Setup Utility application, install HII data
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK - 1,
                  SetupUtilityNotifyFn,
                  NULL,
                  &Event
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->RegisterProtocolNotify (
                    &gEfiSetupUtilityApplicationProtocolGuid,
                    Event,
                    &Registration
                    );
  }
  
  return Status;
}

VOID
CheckSystemConfigLoad(
  IN OUT  CHIPSET_CONFIGURATION *SystemConfigPtr
)
{
  EFI_STATUS                  Status;
  SEC_OPERATION_PROTOCOL      *SeCOp;
  SEC_INFOMATION              SeCInfo;
  Status = gBS->LocateProtocol (
                &gEfiSeCOperationProtocolGuid,
                NULL,
                (VOID **)&SeCOp
                );
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = SeCOp->GetPlatformSeCInfo(
                    &SeCInfo
                    );
  SystemConfigPtr->SecFirmwareUpdate = (UINT8)SeCInfo.FwUpdate;
  SystemConfigPtr->SecFlashUpdate = (UINT8)SeCInfo.HmrfpoEnable;

}

EFI_STATUS
CreateScuData (
  VOID
  )
{
  EFI_STATUS                                Status;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL        *Interface;
  UINTN                                     BufferSize;
  UINT8                                     *Lang;
  EFI_SETUP_UTILITY_PROTOCOL                *This;

  POST_CODE (BDS_ENTER_SETUP); //PostCode = 0x29, Enter Setup Menu
  //
  // There will be only one DeviceManagerSetup in the system.
  // If there is another out there, someone is trying to install us
  // again.  We fail in that scenario.
  //
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityBrowserProtocolGuid,
                  NULL,
                  (VOID **)&Interface
                  );
  //
  // If there was no error, assume there is an installation and fail to load
  //
  if (EFI_ERROR(Status) ) {
    Status = gBS->LocateProtocol (&gEfiSetupUtilityProtocolGuid, NULL, (VOID **) &This);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    Status = InitSetupUtilityBrowser(This);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    Status = InstallSetupUtilityBrowserProtocol (This);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    ZeroMem (gSUBrowser->SUCInfo->MapTable, sizeof (HII_HANDLE_VARIABLE_MAP_TABLE) * MAX_HII_HANDLES);
    if (!FeaturePcdGet (PcdH2OFormBrowserSupported)) {
      DisableQuietBoot ();
    }
    Status = InstallHiiData ();
    This->FirstIn = FALSE;
    if (EFI_ERROR(Status)) {
      return Status;
    }

    //
    // Load the variable data records
    //
    gSUBrowser->Interface.SCBuffer = (UINT8 *) gSUBrowser->SCBuffer;
  }
  Lang = GetVariableAndSize (
           L"PlatformLang",
           &gEfiGlobalVariableGuid,
           &BufferSize
           );
  if (Lang != NULL) {
    Status = gRT->SetVariable (
                    L"BackupPlatformLang",
                    &gEfiGenericVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    BufferSize,
                    Lang
                    );
    gBS->FreePool (Lang);
  }


  return Status;
}

EFI_STATUS
DestroyScuData (
  VOID
  )
{
  EFI_STATUS                                Status;
  UINTN                                     BufferSize;
  UINT8                                     *Lang;
  EFI_SETUP_UTILITY_PROTOCOL                *SetupUtility;
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL         *SysPasswordService;
  EFI_SETUP_UTILITY_PROTOCOL                *This;

  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&SysPasswordService
                  );
  ASSERT_EFI_ERROR (Status);
  SysPasswordService->LockPassword (SysPasswordService);

  Status = RemoveHiiData (
             SetupUtilityStrings,
             (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[ExitHiiHandle]),
             (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[BootHiiHandle]),
             (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[PowerHiiHandle]),
             (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[SecurityHiiHandle]),
             (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[AdvanceHiiHandle]),
             (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[MainHiiHandle]),
             NULL
             );

  Status = UninstallSetupUtilityBrowserProtocol ();
  Status = gBS->LocateProtocol (&gEfiSetupUtilityProtocolGuid, NULL, (VOID **) &This);
  if (!EFI_ERROR(Status)) {
    This->FirstIn = TRUE;
  }
  gSUBrowser = NULL;
  if (gST->ConOut != NULL) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    gST->ConOut->ClearScreen (gST->ConOut);
  }

  Lang = GetVariableAndSize (
           L"BackupPlatformLang",
           &gEfiGenericVariableGuid,
           &BufferSize
           );
  if (Lang != NULL) {
    Status = gRT->SetVariable (
                    L"PlatformLang",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    BufferSize,
                    Lang
                    );
    gBS->FreePool (Lang);

    gRT->SetVariable (
           L"BackupPlatformLang",
           &gEfiGenericVariableGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           0,
           NULL
           );
  }

  //
  // restore NVvMemory map of SetupData from rom
  //
  Status = gBS->LocateProtocol (&gEfiSetupUtilityProtocolGuid, NULL, (VOID **)&SetupUtility);
  if (!EFI_ERROR(Status)) {
    BufferSize = PcdGet32 (PcdSetupConfigSize);
    Status = gRT->GetVariable (
                    L"Setup",
                    &mFormSetGuid,
                    NULL,
                    &BufferSize,
                    SetupUtility->SetupNvData
                    );
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe ((CHIPSET_CONFIGURATION *)SetupUtility->SetupNvData, sizeof (CHIPSET_CONFIGURATION));  
    }
  }

  return Status;
}

//[-start-161215-IB08450359-remove]//
//EFI_STATUS
//EFIAPI
//InstallSetupHii (
//  IN  EFI_SETUP_UTILITY_PROTOCOL            *This,
//  IN  BOOLEAN                               InstallHii
//  )
//{
//  EFI_STATUS                                Status;
//  EFI_SETUP_UTILITY_BROWSER_PROTOCOL        *Interface;
//
//  if (InstallHii) {
//    Status = gBS->LocateProtocol (
//                    &gEfiSetupUtilityBrowserProtocolGuid,
//                    NULL,
//                    (VOID **)&Interface
//                    );
//    //
//    // If there was no error, assume there is an installation and fail to load
//    //
//    if (EFI_ERROR(Status) ) {
//      Status = InitSetupUtilityBrowser(This);
//      if (EFI_ERROR(Status)) {
//        return Status;
//      }
//
//      Status = InstallSetupUtilityBrowserProtocol (This);
//      if (EFI_ERROR(Status)) {
//        return Status;
//      }
//
//      ZeroMem (gSUBrowser->SUCInfo->MapTable, sizeof (HII_HANDLE_VARIABLE_MAP_TABLE) * MAX_HII_HANDLES);
//
//
//
//      Status = InstallHiiData ();
//
//      This->FirstIn = FALSE;
//
//      if (EFI_ERROR(Status)) {
//        return Status;
//      }
//
//      //
//      // Load the variable data records
//      //
//      gSUBrowser->Interface.SCBuffer = (UINT8 *) gSUBrowser->SCBuffer;
//    }
//  } else {
//    Status = RemoveHiiData (
//               SetupUtilityStrings,
//               (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[ExitHiiHandle]),
//               (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[BootHiiHandle]),
//               (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[PowerHiiHandle]),
//               (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[SecurityHiiHandle]),
//               (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[AdvanceHiiHandle]),
//               (HII_HANDLE_VARIABLE_MAP_TABLE *) &(gSUBrowser->SUCInfo->MapTable[MainHiiHandle]),
//               NULL
//               );
//
//    Status = UninstallSetupUtilityBrowserProtocol ();
//  }
//
//  return Status;
//}
//[-end-161215-IB08450359-remove]//

EFI_STATUS
PowerOnSecurity (
  IN  EFI_SETUP_UTILITY_PROTOCOL        *SetupUtility
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_GUID                              StringPackGuid = STRING_PACK_GUID;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo = NULL;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL    *Interface;
  EFI_HANDLE                            DriverHandle;
  VOID                                  *Table;
  UINT8                                 *HobSetupData;
  VOID                                  *HobList = NULL;
  EFI_BOOT_MODE                         BootMode;


  HiiHandle = 0;

  //
  // There will be only one DeviceManagerSetup in the system.
  // If there is another out there, someone is trying to install us
  // again.  We fail in that scenario.
  //
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityBrowserProtocolGuid,
                  NULL,
                  (VOID **)&Interface
                  );

  //
  // If there was no error, assume there is an installation and fail to load
  //
  if (!EFI_ERROR(Status) ) {
    Status = UninstallSetupUtilityBrowserProtocol ();
  }

  Status = InitSetupUtilityBrowser(SetupUtility);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = InstallSetupUtilityBrowserProtocol (SetupUtility);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  SUCInfo = gSUBrowser->SUCInfo;
  Status = CreateHiiDriverHandle (&DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  HiiHandle = HiiAddPackages (&StringPackGuid, DriverHandle, SecurityVfrBin ,SetupUtilityLibStrings, NULL);
  ASSERT(HiiHandle != NULL);

  gSUBrowser->SUCInfo->MapTable[SecurityHiiHandle].HiiHandle = HiiHandle;
  gSUBrowser->Interface.SCBuffer = (UINT8 *) gSUBrowser->SCBuffer;

  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&SUCInfo->SysPasswordService
                  );
  SUCInfo->SupervisorPassword = NULL;
  SUCInfo->UserPassword       = NULL;
  if (!EFI_ERROR(Status)) {
    //
    // Check password
    //
    BootMode = GetBootModeHob();
    if (BootMode == BOOT_IN_RECOVERY_MODE) {
      HobList = GetHobList ();
      Table= GetNextGuidHob (&gEfiPowerOnPwSCUHobGuid, HobList);
      if (Table != NULL) {
        HobSetupData = ((UINT8 *) Table) + sizeof (EFI_HOB_GUID_TYPE);
        ((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->PowerOnPassword = ((CHIPSET_CONFIGURATION *) HobSetupData)->PowerOnPassword;
      }
    }
    if (((CHIPSET_CONFIGURATION *) gSUBrowser->SCBuffer)->PowerOnPassword == POWER_ON_PASSWORD) {
      Status = PasswordCheck (
                 SUCInfo,
                 (KERNEL_CONFIGURATION *)gSUBrowser->SCBuffer
                 );
      ASSERT_EFI_ERROR (Status);
    }
  }

  if (SUCInfo->SupervisorPassword  != NULL) {
    if (SUCInfo->SupervisorPassword->NumOfEntry != 0) {
      gBS->FreePool (SUCInfo->SupervisorPassword->InputString);
      gBS->FreePool (SUCInfo->SupervisorPassword);
      SUCInfo->SupervisorPassword = NULL;
     }
  }

  if (SUCInfo->UserPassword  != NULL) {
    if (SUCInfo->UserPassword->NumOfEntry != 0) {
      gBS->FreePool (SUCInfo->UserPassword->InputString);
      gBS->FreePool (SUCInfo->UserPassword);
      SUCInfo->UserPassword = NULL;
     }
  }

  gSUBrowser->HiiDatabase->RemovePackageList (gSUBrowser->HiiDatabase, HiiHandle);
  DestroyHiiDriverHandle (DriverHandle);
  Status = UninstallSetupUtilityBrowserProtocol ();

  return Status;
}

/**
 Installs the SetupUtilityBrowser protocol including allocating
 storage for variable record data.

 @param [in]   This

 @retval EFI_SUCEESS            Protocol was successfully installed
 @retval EFI_OUT_OF_RESOURCES   Not enough resource to allocate data structures
 @return Other                  Some other error occured

**/
EFI_STATUS
InstallSetupUtilityBrowserProtocol (
  IN  EFI_SETUP_UTILITY_PROTOCOL            *This
  )
{
  EFI_STATUS                                Status;

  gSUBrowser->Interface.AtRoot        = TRUE;
  gSUBrowser->Interface.Finished      = FALSE;
  gSUBrowser->Interface.Guid          = &mFormSetGuid;
  gSUBrowser->Interface.UseMenus      = FALSE;
  gSUBrowser->Interface.Direction     = NoChange;
  gSUBrowser->Interface.CurRoot       = FALSE;
  gSUBrowser->Interface.MenuItemCount = FALSE;
  gSUBrowser->Interface.Size          = PcdGet32 (PcdSetupConfigSize);
  gSUBrowser->Interface.Firstin       = TRUE;
  gSUBrowser->Interface.Changed       = FALSE;
  gSUBrowser->Interface.JumpToFirstOption = TRUE;

  gSUBrowser->Interface.SCBuffer      = NULL;
  gSUBrowser->Interface.MyIfrNVData = NULL;
  gSUBrowser->Interface.PreviousMenuEntry = 0;
  //
  // Install the Protocol
  //
  gSUBrowser->Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &gSUBrowser->Handle,
                  &gEfiSetupUtilityBrowserProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gSUBrowser->Interface
                  );

  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
 Uninstalls the DeviceManagerSetup protocol and frees memory
 used for storing variable record data.

 @param None

 @retval EFI_SUCEESS            Protocol was successfully installed
 @retval EFI_ALREADY_STARTED    Protocol was already installed
 @retval EFI_OUT_OF_RESOURCES   Not enough resource to allocate data structures
 @return Other                  Some other error occured

**/
EFI_STATUS
UninstallSetupUtilityBrowserProtocol (
  VOID
  )
{
  EFI_STATUS                                Status;

  Status = gBS->UninstallProtocolInterface (
                  gSUBrowser->Handle,
                  &gEfiSetupUtilityBrowserProtocolGuid,
                  &gSUBrowser->Interface
                  );
  if ( EFI_ERROR(Status) ) {
    return Status;
  }

  if (gSUBrowser->SUCInfo->HddPasswordScuData  != NULL) {
    if (gSUBrowser->SUCInfo->HddPasswordScuData->NumOfEntry != 0) {
      if (gSUBrowser->SUCInfo->HddPasswordScuData[0].HddInfo != NULL) {
        gBS->FreePool (gSUBrowser->SUCInfo->HddPasswordScuData[0].HddInfo);
      }
      gBS->FreePool (gSUBrowser->SUCInfo->HddPasswordScuData);
    }
  }
  if (gSUBrowser->SUCInfo->SupervisorPassword  != NULL) {
    if (gSUBrowser->SUCInfo->SupervisorPassword->NumOfEntry != 0) {
      gBS->FreePool (gSUBrowser->SUCInfo->SupervisorPassword->InputString);
      gBS->FreePool (gSUBrowser->SUCInfo->SupervisorPassword);
    }
  }

  if (gSUBrowser->SUCInfo->UserPassword  != NULL) {
    if (gSUBrowser->SUCInfo->UserPassword->NumOfEntry != 0) {
      gBS->FreePool (gSUBrowser->SUCInfo->UserPassword->InputString);
      gBS->FreePool (gSUBrowser->SUCInfo->UserPassword);
    }
  }

  gBS->FreePool (gSUBrowser->SUCInfo);
  gBS->FreePool (gSUBrowser);

  return EFI_SUCCESS;
}

/**
 Call the browser and display the SetupUtility

 @param None

**/
EFI_STATUS
CallSetupUtilityBrowser (
  VOID
  )
{
  EFI_STATUS                                Status;
  UINTN                                     BufferSize;
  EFI_HANDLE                                DispPage;
  BOOLEAN                                   SetupUtilityBrowserEmpty;
  BOOLEAN                                   Continue;
  EFI_BROWSER_ACTION_REQUEST                ResetRequired;
  STRING_REF                                TempToken;
  UINTN                                     Index;
  EFI_HII_PACKAGE_LIST_HEADER               *Buffer;
  EFI_HII_DATABASE_PROTOCOL                 *HiiDatabase;
  EFI_FORM_BROWSER2_PROTOCOL                *Browser2;
  EFI_SETUP_MOUSE_PROTOCOL                  *SetupMouse;
  EFI_IFR_FORM_SET                          *FormSetPtr;
  UINT16                                    Class;
  UINT16                                    SubClass;
  EFI_GUID                                  FormSetGuid = EFI_HII_PLATFORM_SETUP_FORMSET_GUID;
  UINT8                                     *TempPtr;
//[PRJ]+ >>>> Hidden T66 unsupported items in SCU and add T66 showAllPage function
  BIOS_SETTING_ALL_PAGE                     *SCU_AllPage;
  BOOLEAN                                   ShowMoreInfo;
//[PRJ]+ <<<< Hidden T66 unsupported items in SCU and add T66 showAllPage function
  Status = EFI_SUCCESS;
  SetupUtilityBrowserEmpty = TRUE;
  Buffer = NULL;
  gCallbackKey = 0;
  Continue = TRUE;
  HiiDatabase = gSUBrowser->HiiDatabase;
  Browser2    = gSUBrowser->Browser2;
  SetupMouse  = NULL;
//[PRJ]+ >>>> Hidden T66 unsupported items in SCU and add T66 showAllPage function
  ShowMoreInfo = FALSE;
  Status = GetBiosSettingData((BIOS_SETTING_STRUCT**)&SCU_AllPage, BIOS_SETTING_ALPG);
  if (!EFI_ERROR(Status)) {
    ShowMoreInfo = SCU_AllPage->ShollAllPage;
    ((SYSTEM_CONFIGURATION *)gSUBrowser->SCBuffer)->T66ShowSCU = ShowMoreInfo;
    FreePool(SCU_AllPage);
  }
//[PRJ]+ <<<< Hidden T66 unsupported items in SCU and add T66 showAllPage function 
  for (Index = 0, BufferSize = 0; Index < MAX_HII_HANDLES && gSUBrowser->SUCInfo->MapTable[Index].HiiHandle != NULL; Index++) {
    //
    // Am not initializing Buffer since the first thing checked is the size
    // this way I can get the real buffersize in the smallest code size
    //
    Status = HiiDatabase->ExportPackageLists (
                            HiiDatabase,
                            gSUBrowser->SUCInfo->MapTable[Index].HiiHandle,
                            &BufferSize,
                            Buffer
                            );
    if (Status == EFI_NOT_FOUND) {
      break;
    }

    //
    // BufferSize should have the real size of the forms now
    //
    Buffer = AllocateZeroPool (BufferSize);
    ASSERT (Buffer != NULL);

    //
    // Am not initializing Buffer since the first thing checked is the size
    // this way I can get the real buffersize in the smallest code size
    //
    Status = HiiDatabase->ExportPackageLists (
                            HiiDatabase,
                            gSUBrowser->SUCInfo->MapTable[Index].HiiHandle,
                            &BufferSize,
                            Buffer
                            );


    //
    // Skips the header, now points to data
    //

    TempPtr = (UINT8 *) (Buffer + 1);
    TempPtr = (UINT8 *) ((EFI_HII_FORM_PACKAGE_HDR *) TempPtr + 1);
    FormSetPtr = (EFI_IFR_FORM_SET *) TempPtr;
    //
    // If this formset belongs in the device manager, add it to the menu
    //
    //
    // check Platform form set guid
    //
    if (CompareGuid ((EFI_GUID *) (FormSetPtr + sizeof (EFI_IFR_FORM_SET)), &FormSetGuid)) {
      TempPtr = (UINT8 *) ((EFI_IFR_FORM_SET *) TempPtr + 1);
      Class = ((EFI_IFR_GUID_CLASS *) TempPtr)->Class;
      TempPtr += sizeof (EFI_IFR_GUID_CLASS);
      SubClass = ((EFI_IFR_GUID_SUBCLASS *) TempPtr)->SubClass;
      if (Class == EFI_NON_DEVICE_CLASS) {
        continue;
      } else {
        if (SubClass == EFI_USER_ACCESS_THREE) {
          if ((((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->SetUserPass == 1) && (((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->UserAccessLevel == 3)) {
            BufferSize = 0;
            gBS->FreePool (Buffer);
            continue;
          };
        }

        if (SubClass == EFI_USER_ACCESS_TWO) {
          if ((((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->SetUserPass == 1) && (((CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer)->UserAccessLevel == 2)) {
            BufferSize = 0;
            gBS->FreePool(Buffer);
            continue;
          }
        }
      }
    }

    SetupUtilityBrowserEmpty = FALSE;


    if (gSUBrowser->Interface.MenuItemCount < MAX_ITEMS) {
      TempToken = FormSetPtr->FormSetTitle;
      gSUBrowser->Interface.MenuList[gSUBrowser->Interface.MenuItemCount].MenuTitle = TempToken;
      //
      // Set the display page.  Last page found is the first to be displayed.
      //
      gSUBrowser->Interface.MenuList[gSUBrowser->Interface.MenuItemCount].Page = gSUBrowser->SUCInfo->MapTable[Index].HiiHandle;
      //
      // NULL out the string pointer
      //
      gSUBrowser->Interface.MenuList[gSUBrowser->Interface.MenuItemCount].String = NULL;
      //
      // Entry is filled out so update count
      //
      gSUBrowser->Interface.MenuItemCount++;
    }
    BufferSize = 0;
    gBS->FreePool(Buffer);
  }

  //
  // Drop the TPL level from TPL_APPLICATION+2 to TPL_APPLICATION
  //
  gBS->RestoreTPL (TPL_APPLICATION);

  //
  // If we didn't add anything - don't bother going to device manager
  //
  if (!SetupUtilityBrowserEmpty) {
    Status = gBS->LocateProtocol (
                    &gSetupMouseProtocolGuid,
                    NULL,
                    (VOID **)&(gSUBrowser->Interface.SetupMouse)
                    );
    if (!EFI_ERROR (Status)) {
      gSUBrowser->Interface.SetupMouseFlag = TRUE;
      SetupMouse = (EFI_SETUP_MOUSE_PROTOCOL *) gSUBrowser->Interface.SetupMouse;
      Status = SetupMouse->Start (SetupMouse);
      if (EFI_ERROR (Status)) {
        SetupMouse = NULL;
        gSUBrowser->Interface.SetupMouseFlag = FALSE;
      }
    }
    //
    // Init before root page loop
    //
    gSUBrowser->Interface.AtRoot    = TRUE;
    gSUBrowser->Interface.CurRoot   = gSUBrowser->Interface.MenuItemCount - 1;
    gSUBrowser->Interface.UseMenus  = TRUE;
    gSUBrowser->Interface.Direction = NoChange;
    DispPage                       = gSUBrowser->Interface.MenuList[gSUBrowser->Interface.MenuItemCount - 1].Page;

    //
    // Loop until exit condition is found.  Use direction indicators and
    // the menu list to determine what root page needs to be displayed.
    //
    while (Continue) {
      Status = Browser2->SendForm (
                           Browser2,
                           (EFI_HII_HANDLE *) &DispPage,
                           1,
                           (FeaturePcdGet (PcdH2OFormBrowserSupported)) ? &mFormSetClassGuid : &mFormSetGuid,
                           1,
                           NULL,
                           &ResetRequired
                           );

      if (ResetRequired == EFI_BROWSER_ACTION_REQUEST_RESET) {
        gBS->RaiseTPL (TPL_NOTIFY);
        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
      } else if (ResetRequired == EFI_BROWSER_ACTION_REQUEST_EXIT) {
        gSUBrowser->Interface.UseMenus = FALSE;
        break;
      }

      //
      // Force return to Device Manager or Exit if finished
      //
      gSUBrowser->Interface.AtRoot  = TRUE;
      gSUBrowser->Interface.Firstin = FALSE;
      if (gSUBrowser->Interface.Finished) {
        // Need to set an exit at this point
        gSUBrowser->Interface.UseMenus = FALSE;
        Continue = FALSE;
        break;
      }

      //
      // Check for next page or exit states
      //
      switch (gSUBrowser->Interface.Direction) {

     case Right:
        //
        // Update Current Root Index
        //
        if (gSUBrowser->Interface.CurRoot == 0) {
          gSUBrowser->Interface.CurRoot = gSUBrowser->Interface.MenuItemCount - 1;
        } else {
          gSUBrowser->Interface.CurRoot--;
        }
        //
        // Set page to display
        //
        DispPage = gSUBrowser->Interface.MenuList[gSUBrowser->Interface.CurRoot].Page;
        //
        // Update Direction Flag
        //
        gSUBrowser->Interface.Direction = NoChange;
        break;

      case Left:
        //
        // Update Current Root Index
        //
        if (gSUBrowser->Interface.CurRoot == gSUBrowser->Interface.MenuItemCount - 1) {
          gSUBrowser->Interface.CurRoot = 0;
        } else {
          gSUBrowser->Interface.CurRoot++;
        }
        //
        // Set page to display
        //
        DispPage = gSUBrowser->Interface.MenuList[gSUBrowser->Interface.CurRoot].Page;
        //
        // Update Direction Flag
        //
        gSUBrowser->Interface.Direction = NoChange;
        break;

      case Jump:
        //
        // Update Current Root Index
        //
        if ((gSUBrowser->Interface.CurRoot >= 0) &&
            (gSUBrowser->Interface.CurRoot <= gSUBrowser->Interface.MenuItemCount - 1)) {
          //
          // Set page to display
          //
          DispPage = gSUBrowser->Interface.MenuList[gSUBrowser->Interface.CurRoot].Page;
        }
        gSUBrowser->Interface.Direction = NoChange;
        break;

      default:
        break;
      }

    }
  }
  if ((SetupMouse != NULL) && (gSUBrowser->Interface.SetupMouseFlag == TRUE)) {
    Status = SetupMouse->Close (SetupMouse);
  }
  //
  // We are exiting so clear the screen
  //
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_BLACK);
  gST->ConOut->ClearScreen (gST->ConOut);

  gBS->RaiseTPL (TPL_APPLICATION + 2);   // TPL_APPLICATION+2 = EFI_TPL_DRIVER
  return Status;
}

EFI_STATUS
GetSystemConfigurationVar (
  IN SETUP_UTILITY_DATA                     *SetupData
  )
{
  EFI_STATUS                                Status;
  UINTN                                     BufferSize;
  CHIPSET_CONFIGURATION                     *SetupNvData;
  UINT16                                    Timeout;

  //
  // Check the setup variable was create or not, if not then create default setup variable.
  //
  BufferSize = PcdGet32 (PcdSetupConfigSize);

  Status = gBS->AllocatePool(
                  EfiACPIMemoryNVS,
                  BufferSize,
                  (VOID **)&SetupNvData
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  ZeroMem (SetupNvData, BufferSize);

  //
  // Check "Setup" variable is exist or not...
  //
  BufferSize = PcdGet32 (PcdSetupConfigSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &mFormSetGuid,
                  NULL,
                  &BufferSize,
                  (VOID *) SetupNvData
                  );
//[-start-160803-IB07220122-remove]//
//   GetSystemConfigData(SetupNvData, &BufferSize);
//[-end-160803-IB07220122-remove]//
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (SetupNvData, sizeof (CHIPSET_CONFIGURATION));  
  } 

  if (EFI_ERROR (Status)) {
    //
    //"Setup" Variable doesn't exist,so get a buffer with default variable
    //
    DefaultSetup (SetupNvData);
    gRT->SetVariable (
           L"PlatformLang",
           &gEfiGlobalVariableGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           AsciiStrSize ((CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang)),
           (VOID *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang)
           );

    Timeout = (UINT16) PcdGet16 (PcdPlatformBootTimeOut);
    gRT->SetVariable (
           L"Timeout",
           &gEfiGlobalVariableGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           sizeof (UINT16),
           (VOID *) &Timeout
           );
  }

  CheckSystemConfigLoad(SetupNvData);

  SetupNvData->DefaultBootType = DEFAULT_BOOT_FLAG;

  //[PRJ]++ Fix Set PXE boot first function by CMFC fail
    SetBDPVFlag (SetupNvData);
  //[PRJ]-- Fix Set PXE boot first function by CMFC fail

  BufferSize = PcdGet32 (PcdSetupConfigSize);

  Status = SaveSetupConfig (
             SETUP_VARIABLE_NAME,
             &mFormSetGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             BufferSize,
             (VOID *) SetupNvData
             );
//[-start-160803-IB07220122-remove]//
//   SetSystemConfigData(SetupNvData, BufferSize);     
//[-end-160803-IB07220122-remove]//

  //
  // Set Quick boot setting to variable, common code can use this setting to
  // do quick boot or not.
  //
  gRT->SetVariable (
         L"QuickBoot",
         &gEfiGenericVariableGuid,
         EFI_VARIABLE_BOOTSERVICE_ACCESS,
         sizeof (SetupNvData->QuickBoot),
         (VOID *) &SetupNvData->QuickBoot
         );
  SetupData->SetupUtility.SetupNvData = (UINT8 *) SetupNvData;

  //
  // Check "Custom" variable is exist or not...
  //
  SetupNvData = AllocateZeroPool (BufferSize);
  Status = gRT->GetVariable (
                  L"Custom",
                   &mFormSetGuid,
                  NULL,
                  &BufferSize,
                  (VOID *)SetupNvData
                  );

  if (EFI_ERROR (Status)) {
    //
    // "Custom" Variable doesn't exist,so get a buffer with default variable
    //
    DefaultSetup (SetupNvData);
    gRT->SetVariable (
           L"CustomPlatformLang",
           &gEfiGenericVariableGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           AsciiStrSize ((CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang)),
           (VOID *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang)
           );
  }
/*
  if ( SetupNvData->EnableTurboMode && IPSSupport == 0) {
    SetupNvData->EnableIPS = 0;
  }
  SetupNvData->GbePeiEPortSelect = (UINT8)RegData32;
  Status = PEGDetect(SetupNvData);
*/

  //
  // Save Custom variable.
  //
  BufferSize = PcdGet32 (PcdSetupConfigSize);
  Status = gRT->SetVariable (
                  L"Custom",
                  &mFormSetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  BufferSize,
                  (VOID *)SetupNvData
                  );

  gBS->FreePool (SetupNvData);

  return EFI_SUCCESS;
}

/**
 Installs a string and ifr pack set

 @param None

 @retval VOID

**/
EFI_STATUS
InstallHiiData (
  VOID
  )
{
  EFI_HII_HANDLE                 HiiHandle;

  EFI_HANDLE                     DriverHandle;
  UINTN                          Index;
  EFI_STATUS                     Status;
  UINTN                          HandleCnt;
  NEW_PACKAGE_INFO               NewPackageInfo [] =
                                 {{InstallExitCallbackRoutine,     ExitVfrBin,     SetupUtilityLibStrings, SetupUtilityLibImages},
                                  {InstallBootCallbackRoutine,     BootVfrBin,     SetupUtilityLibStrings, SetupUtilityLibImages},
                                  {InstallPowerCallbackRoutine,    PowerVfrBin,    SetupUtilityStrings,    SetupUtilityImages   },
                                  {InstallSecurityCallbackRoutine, SecurityVfrBin, SetupUtilityLibStrings, SetupUtilityLibImages},
                                  {InstallAdvanceCallbackRoutine,  AdvanceVfrBin,  SetupUtilityStrings,    SetupUtilityImages   },
                                  {InstallMainCallbackRoutine,     MainVfrBin,     SetupUtilityLibStrings, SetupUtilityLibImages}
                                 };


  HandleCnt = sizeof (NewPackageInfo) / sizeof (NEW_PACKAGE_INFO);
  if (HandleCnt > MAX_HII_HANDLES) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = EFI_SUCCESS;
  for (Index = 0; Index < HandleCnt; Index++) {
    Status = CreateHiiDriverHandle (&DriverHandle);
    if (EFI_ERROR (Status)) {
      break;
    }

    HiiHandle = HiiAddPackages (
                  &mFormSetGuid,
                  DriverHandle,
                  NewPackageInfo[Index].IfrPack,
                  NewPackageInfo[Index].StringPack,
                  NewPackageInfo[Index].ImagePack,
                  NULL
                  );
    ASSERT(HiiHandle != NULL);

    gSUBrowser->SUCInfo->MapTable[Index].HiiHandle = HiiHandle;
    gSUBrowser->SUCInfo->MapTable[Index].DriverHandle = DriverHandle;
    NewPackageInfo[Index].CallbackRoutine (DriverHandle, HiiHandle);

  }
  mDriverHiiHandle = gSUBrowser->SUCInfo->MapTable[AdvanceHiiHandle].HiiHandle;
//[-start-160704-IB07220103-modify]//
  DisplayPlatformInfo (gSUBrowser);
//[-end-160704-IB07220103-modify]//
  return Status;

}

EFI_STATUS
InitSetupUtilityBrowser(
  IN  EFI_SETUP_UTILITY_PROTOCOL            *This
  )
{
  EFI_STATUS                                Status;
  UINTN                                     BufferSize;
  UINT8                                     *Setup = NULL;
  gSUBrowser = AllocateZeroPool (sizeof(SETUP_UTILITY_BROWSER_DATA));
  if (gSUBrowser == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  gSUBrowser->SUCInfo = AllocateZeroPool (sizeof(SETUP_UTILITY_CONFIGURATION));
  if (gSUBrowser->SUCInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Setup = GetVariableAndSize (
            SETUP_VARIABLE_NAME,
            &mFormSetGuid,
            &BufferSize
            );

  CopyMem ((UINT8 *)This->SetupNvData, Setup, BufferSize);
  gSUBrowser->SCBuffer          = This->SetupNvData;
  gSUBrowser->SUCInfo->SCBuffer = gSUBrowser->SCBuffer;
  gSUBrowser->Signature         = (UINTN)EFI_SETUP_UTILITY_BROWSER_SIGNATURE;
  gSUBrowser->ExtractConfig     = GenericExtractConfig;
  gSUBrowser->RouteConfig       = GenericRouteConfig;
  gSUBrowser->HotKeyCallback    = HotKeyCallBack;

  if (Setup != NULL) {
    gBS->FreePool (Setup);
  }
  
  //
  // There should only be one HII protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID**)&gSUBrowser->HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // There should only be one HII protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiStringProtocolGuid,
                  NULL,
                  (VOID**)&gSUBrowser->HiiString
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // There should only be one HII protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiHiiConfigRoutingProtocolGuid,
                  NULL,
                  (VOID**)&gSUBrowser->HiiConfigRouting
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // There will be only one FormConfig in the system
  // If there is another out there, someone is trying to install us
  // again.  Fail that scenario.
  //
  Status = gBS->LocateProtocol (
                  &gEfiFormBrowser2ProtocolGuid,
                  NULL,
                  (VOID**)&gSUBrowser->Browser2
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->LocateProtocol (
                 &gH2ODialogProtocolGuid,
                 NULL,
                 (VOID **)&gSUBrowser->H2ODialog
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
};

/**
 Installs a string and ifr pack set

 @param [in]   StringPack       string pack to store in database and associate with IfrPack
                                IrfPack - ifr pack to store in database (it will use StringPack data)
 @param        ...

 @retval VOID

**/
EFI_STATUS
RemoveHiiData (
  IN VOID     *StringPack,
  ...
  )
{
  VA_LIST                                   args;
  HII_HANDLE_VARIABLE_MAP_TABLE             *MapTable;
  EFI_HII_DATABASE_PROTOCOL                 *HiiDatabase;
  UINTN                                     Index;

  HiiDatabase = gSUBrowser->HiiDatabase;
  VA_START (args, StringPack );

  Index = 0;
  while (TRUE) {
    MapTable = VA_ARG( args, HII_HANDLE_VARIABLE_MAP_TABLE *);
    if (MapTable == NULL) {
      break;
    }
    HiiDatabase->RemovePackageList (HiiDatabase, MapTable->HiiHandle);
    mUninstallCallbackRoutine[Index++] (MapTable->DriverHandle);
    DestroyHiiDriverHandle (MapTable->DriverHandle);
  }

  if (gSUBrowser->IdeConfig != NULL) {
    gBS->FreePool (gSUBrowser->IdeConfig);
    gSUBrowser->IdeConfig = NULL;    
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
SetupUtilityNotifyFn (
  IN EFI_EVENT                             Event,
  IN VOID                                  *Context
  )
{
  EFI_STATUS                               Status;
  EFI_SETUP_UTILITY_APPLICATION_PROTOCOL   *SetupUtilityApp;
//[-start-170721-IB07400892-add]//
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL          *CurrentConOut;
//[-end-170721-IB07400892-add]//

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  NULL,
                  (VOID **) &SetupUtilityApp
                  );
  if (EFI_ERROR(Status)) {
    return;
  }

  mScuRecord = 0;

  if (!FeaturePcdGet (PcdH2OFormBrowserSupported)) {
    if (SetupUtilityApp->VfrDriverState == InitializeSetupUtility) {
      CreateScuData ();
      CallSetupUtilityBrowser ();
      DestroyScuData ();
    }
    return;
  }

//[-start-170721-IB07400892-modify]//
  switch (SetupUtilityApp->VfrDriverState) {
  case InitializeSetupUtility:
  case H2O_UVE_INITIALIZE_SETUP_UTILITY:
    CreateScuData ();
    break;

  case ShutdownSetupUtility:
    DestroyScuData ();
    break;
    
  case H2O_UVE_SHUTDOWN_SETUP_UTILITY:
    CurrentConOut = gST->ConOut;
    gST->ConOut = NULL;
    DestroyScuData ();
    gST->ConOut = CurrentConOut;
    break;
    
  default:
    break;
  }
//[-end-170721-IB07400892-modify]//
}

/**
 This function use to verify Setup Configuration structure size as a build-time.
 It is not be call in BIOS during POST.

**/
VOID
SetupConfigurationSizeVerify
(
  VOID
)
{
  {C_ASSERT((sizeof(SYSTEM_CONFIGURATION)) == FixedPcdGet32(PcdSetupConfigSize));}
}

//[PRJ]++ Fix Set PXE boot first function by CMFC fail
VOID
SetBDPVFlag (
  IN OUT CHIPSET_CONFIGURATION    *SetupNvData
  )
{
  EFI_STATUS                      Status;
  UINTN                           VariableSize = 0;
  UINT8                           *SetBDPVData = NULL;
  EFI_GUID mEfiSetBDPVGuid = { \
  0x20130519, 0xABCD, 0x1234, {0x00, 0x00, 0x20, 0x13, 0x05, 0x19, 0x03, 0x17} \
  };

  VariableSize = 0;
  do {
    Status = gRT->GetVariable (
                    L"SETBDPV",
                    &mEfiSetBDPVGuid,
                    NULL,
                    &VariableSize,
                    SetBDPVData
                    );
    if (Status == EFI_BUFFER_TOO_SMALL) {
      SetBDPVData = AllocateZeroPool(VariableSize);
    }
  } while (Status == EFI_BUFFER_TOO_SMALL);

  if (!EFI_ERROR(Status))
  {
    switch (*SetBDPVData)
    {
      case 1: //USB
        break;

      case 2: //CD
        break;

      case 3: //Network
//        if (SetupNvData->PxeBootToLan == PXE_DISABLE)
//        {
          SetupNvData->PxeBootToLan = PXE_ENABLE;
          if (SetupNvData->BootType == LEGACY_SUPPORT_ENABLED) {
             SetupNvData->NetworkProtocol = PXE_PROTOCOL_LEGACY;
          } else {
             SetupNvData->NetworkProtocol = PXE_PROTOCOL_IPV4;
          }
          SetupNvData->NewPositionPolicy = ADDPOSITION_FIRST;
//        }
        break;

      case 0xFE:
        Status = gRT->SetVariable (
                    L"BootOrder",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    0,
                    NULL
                    );

        Status = gRT->SetVariable (
                    L"PhysicalBootOrder",
                    &gEfiGenericVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    0,
                    NULL
                    );

        Status = gRT->SetVariable (
                    L"SETBDPV",
                    &mEfiSetBDPVGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    0,
                    NULL
                    );

        gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
        break;

      default:
        break;
    }

  }
}
//[PRJ]-- Fix Set PXE boot first function by CMFC fail

