/** @file

  Set setup data to variable store for DXE drivers.

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HobLib.h>
#include <Guid/SetupDefaultHob.h>
#include <Protocol/Variable.h>
#include <ChipsetSetupConfig.h>
#include <Library/BaseMemoryLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-160628-IB07400749-add]//
#include <Guid/GlobalVariable.h>
#include <Library/PlatformCmosLib.h>
#include <Library/MemoryAllocationLib.h>
#include <ChipsetPostCode.h>
//[-end-160628-IB07400749-add]//
//[-start-170621-IB07400876-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-170621-IB07400876-add]//
//[-start-160629-IB07400749-remove]//
//STATIC EFI_EVENT     mSetSetupDataEvent;
//[-end-160629-IB07400749-remove]//

//[-start-160629-IB07400749-add]//
EFI_STATUS
RemoveVariableByGuid (
  IN  EFI_GUID    *RemoveVarGuid
  )
{ 
  EFI_STATUS           Status;
  UINTN                VariableNameSize;
  CHAR16               *VariableName;
  EFI_GUID             VendorGuid;
  UINTN                VariableNameBufferSize;

  //
  // Get all RemoveVarGuid Variable.
  //
  VariableNameSize = sizeof (CHAR16);
  VariableName = AllocateZeroPool (VariableNameSize);
  VariableNameBufferSize = VariableNameSize;

  while (TRUE) {
    Status = gRT->GetNextVariableName (
                    &VariableNameSize,
                    VariableName,
                    &VendorGuid
                    );

    if (Status == EFI_BUFFER_TOO_SMALL) {
      VariableName = ReallocatePool (VariableNameBufferSize, VariableNameSize, VariableName);
      VariableNameBufferSize = VariableNameSize;
      //
      // Try again using the new buffer.
      //
      Status = gRT->GetNextVariableName (
                      &VariableNameSize,
                      VariableName,
                      &VendorGuid
                      );
    }

    if (EFI_ERROR (Status)) {
      //
      // No more variable available, finish search.
      //
      break;
    }

    //
    // Check variable GUID.
    //
    if (!CompareGuid (&VendorGuid, RemoveVarGuid)) {
      continue;
    }

    DEBUG ((EFI_D_ERROR, "Delete VariableName = %s, Guid = %g\n", VariableName, *RemoveVarGuid));
    gRT->SetVariable (
           VariableName,
           RemoveVarGuid,
           (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE),
           0,
           NULL
           );
    //
    // Re-search Variable
    //
    ZeroMem (VariableName, VariableNameBufferSize);
  }

  FreePool (VariableName);

  return EFI_SUCCESS;
}

// extern EFI_GUID gCrConfigurationGuid;
#define CR_CONFIGURATION_GUID \
{ \
  0x7EC07B9F, 0x66E3, 0x43d4, {0x9B, 0x52, 0x38, 0xFB, 0xB4, 0x63, 0x90, 0xCC}  \
}
//[-start-170119-IB07400837-add]//
// extern gH2oUveVarstoreGuid    
#define H2OUVE_VARSTORE_GUID \
{ \
  0xef0849b6, 0xfad0, 0x40e9, {0x91, 0x07, 0x97, 0x4a, 0xeb, 0x87, 0x87, 0xa2}  \
}        
// SIO WPCD374L
#define SIO_WPCD374L_SETUP00_FORMSET_GUID \
{ \
  0x2c221d35, 0x982e, 0x4e78, { 0xab, 0x1, 0x4a, 0x1, 0xdd, 0x39, 0xd0, 0xf8 } \
}
//[-end-170119-IB07400837-add]//

EFI_STATUS
LinkSetupPackageRestoreDefault (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_GUID    CrVarGuid = CR_CONFIGURATION_GUID;
//[-start-170119-IB07400837-add]//
  EFI_GUID    UveVarGuid = H2OUVE_VARSTORE_GUID;
#ifdef W8374LF2_SIO_SUPPORT
  EFI_GUID    SioWpcd374lVarGuid = H2OUVE_VARSTORE_GUID;
#endif
//[-end-170119-IB07400837-add]//

  DEBUG ((EFI_D_ERROR, "LinkSetupPackageRestoreDefault Start!!\n"));
  Status = RemoveVariableByGuid (&CrVarGuid);  // Console Redirection Variable

//[-start-170119-IB07400837-add]//
  Status = RemoveVariableByGuid (&UveVarGuid); // H2O UVE Variable
//[-end-170119-IB07400837-add]//

  //
  // ProjectToDo: Delete other Setup Variable here (Link Setup Package). 
  //
//[-start-170119-IB07400837-add]//
#ifdef W8374LF2_SIO_SUPPORT
  Status = RemoveVariableByGuid (&SioWpcd374lVarGuid); // SIO W8374LF2 Variable
#endif
//[-end-170119-IB07400837-add]//
  
  DEBUG ((EFI_D_ERROR, "LinkSetupPackageRestoreDefault End, Status = %r!!\n", Status));
  return Status;
}
//[-end-160629-IB07400749-add]//

//[-start-180112-IB07400943-modify]//
EFI_STATUS
CmosDataLoadDefault (
  VOID
  )
{
  UINT8   CmosData;
  
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
  CmosData &= ~(B_CMOS_S5_WAKE_ON_USB);
  CmosData &= ~(B_CMOS_ACPI_CALLBACK_NOTIFY);
  CmosData &= ~(B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT);
  CmosData &= ~(B_CMOS_EFI_LEGACY_BOOT_ORDER);
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
  
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2); 
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND 
  CmosData &= ~(B_CMOS_VAR_DATA_CLEAR_WORKAROUND);
#endif
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData); 
  return EFI_SUCCESS;
}
//[-end-180112-IB07400943-modify]//

/**
  Callback function to set setup data.

  This routine is the notification function for SetupUtility Protocol

  @param EFI_EVENT         Event of the notification
  @param Context           not used in this function

  @retval none

**/
VOID
EFIAPI
SetSetupDataEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS      Status;
  UINTN           Size;
  VOID            *DummyProtocol;
  VOID            *SetupDefaultHob;
  VOID            *SetupData;
  UINT32          Data;
//[-start-160425-IB11270152-remove]//
// CHIPSET_CONFIGURATION               SetupConfig;
//[-end-160425-IB11270152-remove]//
//[-start-160628-IB07400749-add]//
  BOOLEAN         LoadSetupDefault;
//[-end-160628-IB07400749-add]//

  Status = gBS->LocateProtocol(
                      &gEfiVariableArchProtocolGuid,
                      NULL,
                      &DummyProtocol
                      );
  if (EFI_ERROR(Status)) {
    return;
  }

//[-start-160629-IB07400749-modify]//
  gBS->CloseEvent(Event);       
//[-end-160629-IB07400749-modify]//

//[-start-160628-IB07400749-add]//
  LoadSetupDefault = FALSE;
  if ((FeaturePcdGet(PcdCmosRestoreSetupDefault)) && !CheckCmosBatteryStatus()) {
    CHIPSET_POST_CODE (DXE_CMOS_RESTORE_SETUP_DEFAULT);
    DEBUG ((EFI_D_ERROR, "SetupDataProviderDxe.c: CMOS Data Missing, Restore Setup Default!!\n"));
    ClearCmosBatteryStatus ();
    LoadSetupDefault = TRUE;
  }
//[-start-180112-IB07400943-add]//
  else if (!CheckCmosBatteryStatus()){
    CmosDataLoadDefault();
    ClearCmosBatteryStatus ();
  }
//[-end-180112-IB07400943-add]//
//[-end-160628-IB07400749-add]//

  //
  // If setup data is available, skip to set setup data. 
  //
  Size = sizeof(UINT32);  
  Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    NULL,
                    &Size,
                    &Data
                    );
//[-start-160425-IB11270152-remove]//
//  Size = PcdGet32 (PcdSetupConfigSize);
//  ZeroMem (&SetupConfig, Size);
//  Status = GetSystemConfigData(&SetupConfig, &Size);
//
//  if (!EFI_ERROR(Status)) {
//    Status = gRT->SetVariable (
//                    SETUP_VARIABLE_NAME,
//                    &gSystemConfigurationGuid,
//                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
//                    Size,
//                    &SetupConfig
//                    );
//  }
//[-end-160425-IB11270152-remove]//
//[-start-160628-IB07400749-modify]//
  if (((Status == EFI_BUFFER_TOO_SMALL) || !EFI_ERROR(Status)) && (!LoadSetupDefault)) {
//[-end-160628-IB07400749-modify]//
    return;
  }
  
  //
  // Get SetupData from HOB
  //
//[-start-160628-IB07400749-modify]//
  SetupDefaultHob = GetFirstGuidHob (&gSetupDefaultHobGuid);
//  ASSERT (SetupDefaultHob != NULL);
  if (SetupDefaultHob != NULL) { 
  
    Size = PcdGet32 (PcdSetupConfigSize);
    SetupData = GET_GUID_HOB_DATA(SetupDefaultHob);
//    ASSERT (SetupData != NULL);
    if (SetupData != NULL) {
      //
      // Set Variable
      //
      Status = gRT->SetVariable (
                      SETUP_VARIABLE_NAME,
                      &gSystemConfigurationGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      Size,
                      SetupData
                      );

//[-start-160803-IB07220122-remove]//
//   SetSystemConfigData(SetupData, Size);     
//[-end-160803-IB07220122-remove]//
      //
      // Delete BootType to restore default BootType
      //
      Status = gRT->SetVariable (
        L"BootType",
        &gSystemConfigurationGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0,
        NULL
        );

      //
      // Delete PlatformLang to restore default Language
      //
      Status = gRT->SetVariable (
        L"PlatformLang",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        0,
        NULL
        );
//[-start-170119-IB07400837-add]//
      //
      // Delete Timeout variable to restore default timeout value
      //
      Status = gRT->SetVariable (
        L"Timeout",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
        0,
        NULL
        );
//[-end-170119-IB07400837-add]//
      Status = LinkSetupPackageRestoreDefault ();
//[-start-170621-IB07400876-add]//
//[-start-170626-IB07400880-remove]//
        //
        // Move to ExtractSetupDefault()
        //
//      Status = CmosDataLoadDefault ();
//[-end-170626-IB07400880-remove]//
//[-end-170621-IB07400876-add]//
    }  
  }
//[-end-160628-IB07400749-modify]//

  return;
}

/**

  Register a callback to set setup data when variable service is available.
  
  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
SetupDataProviderDxeEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{


  VOID                           *Registration;

  EfiCreateProtocolNotifyEvent  (
                        &gEfiVariableArchProtocolGuid,
                        TPL_NOTIFY,
                        SetSetupDataEvent,
                        NULL,
                        &Registration
                        );

  return EFI_SUCCESS;
}
