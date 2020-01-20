/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/FlashRegionLib.h>
#include "PnpRuntimeDxe.h"

#define FixedPcdGetPtrSize(TokenName)      (UINTN)_PCD_PATCHABLE_##TokenName##_SIZE

//
// global varibles
//
PNP_RUNTIME_PRIVATE_DATA              *mPrivateData;
EFI_EVENT                             mSmbiosGetTableEvent;
EFI_EVENT                             mVirtualAddressChangeEvent;
BOOLEAN                               mIsOemGPNVMap = FALSE;

//
// PnpRuntime services
//
PNP_REDIRECTED_ENTRY_POINT mPnpDmiFunctions[] = {
  Pnp0x50,
  Pnp0x51,
  Pnp0x52
  //
  // new services to be added
  //
};


/**
 ExitBootService callback function

 @param [in]   Event            Registered callback event
 @param [in]   Context

 @retval None.

**/
VOID
EFIAPI
SmbiosGetTableEntryPointCallback (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  VOID        *TempAddress;
  VOID        *TempAddress64Bit;
  UINTN       EntryLength;
  UINTN       EntryLength64Bit;
  UINTN       VarSize;
  EFI_STATUS  Status;
  BOOLEAN     IsSmbios32BitTable;
  BOOLEAN     IsSmbios64BitTable;

  IsSmbios32BitTable = FALSE;
  IsSmbios64BitTable = FALSE;

  if (mPrivateData == NULL) {
    return;
  }
  gBS->CloseEvent (Event);

  //
  //  Check 32Bit, 64 Bit Smbios Table support 
  //
  if (((PcdGet16 (PcdSmbiosVersion) >> 8) < 0x3) || 
    (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT0) == BIT0))) {
    IsSmbios32BitTable = TRUE;
  }

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT1) == BIT1)) {
    IsSmbios64BitTable = TRUE;
  }

  //
  // For 32 Bit Smbios Table
  //
  if (IsSmbios32BitTable && mPrivateData->SMBIOSTableEntryAddress == 0){
    //
    // find physical address of Smbios Table
    //
    EfiGetSystemConfigurationTable (&gEfiSmbiosTableGuid, (VOID **)&TempAddress);
    mPrivateData->SMBIOSTableEntryAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)TempAddress;
    mPrivateData->SmbiosTableEntryPoint = (SMBIOS_TABLE_ENTRY_POINT *)(UINTN)mPrivateData->SMBIOSTableEntryAddress;
    if (mPrivateData->SmbiosTableEntryPoint == NULL) {
      return;
    }
    mPrivateData->SmbiosTable = (SMBIOS_STRUCTURE *)(UINTN)(mPrivateData->SmbiosTableEntryPoint->TableAddress);

    EntryLength = mPrivateData->SmbiosTableEntryPoint->EntryPointLength;
    //
    // temp buffer for Pnp0x52ChangeString()
    //
    VarSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (mPrivateData->SmbiosTableEntryPoint->TableLength));
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    VarSize,
                    (VOID **)&mPrivateData->TempStoreArea
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
    gBS->SetMem (mPrivateData->TempStoreArea, VarSize, 0x00);

    //
    // for Runtime Access Smbios Table
    //
    VarSize += EntryLength;
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    VarSize,
                    (VOID **)&mPrivateData->SmbiosStoreArea
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
    gBS->SetMem (mPrivateData->SmbiosStoreArea, VarSize, 0x00);

    //
    // copy to runtime memory
    //
    CopyMem (
      mPrivateData->SmbiosStoreArea,
      mPrivateData->SmbiosTableEntryPoint,
      EntryLength
      );

    CopyMem (
      (UINT8 *)((UINTN)mPrivateData->SmbiosStoreArea + EntryLength),
      mPrivateData->SmbiosTable,
      mPrivateData->SmbiosTableEntryPoint->TableLength
      );

    //
    // change pointer
    //
    mPrivateData->SmbiosTableEntryPoint = (SMBIOS_TABLE_ENTRY_POINT *)mPrivateData->SmbiosStoreArea;
    mPrivateData->SmbiosTable = (SMBIOS_STRUCTURE *)((UINTN)mPrivateData->SmbiosStoreArea + EntryLength);

  }
  
  //
  // For 64 Bit Smbios Table
  //
  if ( IsSmbios64BitTable || mPrivateData->SMBIOSTableEntryAddress64Bit == 0){

    EfiGetSystemConfigurationTable (&gEfiSmbios3TableGuid, (VOID **)&TempAddress64Bit);
    mPrivateData->SMBIOSTableEntryAddress64Bit = (EFI_PHYSICAL_ADDRESS)(UINTN)TempAddress64Bit;
    mPrivateData->SmbiosTableEntryPoint64Bit = (SMBIOS_TABLE_3_0_ENTRY_POINT *)(UINTN)mPrivateData->SMBIOSTableEntryAddress64Bit;

    if (mPrivateData->SmbiosTableEntryPoint64Bit == NULL) {
      return;
    }
    mPrivateData->SmbiosTable64Bit = (SMBIOS_STRUCTURE *)(UINTN)(mPrivateData->SmbiosTableEntryPoint64Bit->TableAddress);
    EntryLength64Bit = sizeof (SMBIOS_TABLE_3_0_ENTRY_POINT);

    //
    // for Runtime Access 64 Bit Smbios Table
    //
    VarSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (mPrivateData->SmbiosTableEntryPoint64Bit->TableMaximumSize));
    VarSize += EntryLength64Bit;
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    VarSize,
                    (VOID **)&mPrivateData->SmbiosStoreArea64Bit
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
    gBS->SetMem (mPrivateData->SmbiosStoreArea64Bit, VarSize, 0x00);

    //
    // copy to runtime memory
    //
    CopyMem (
      mPrivateData->SmbiosStoreArea64Bit,
      mPrivateData->SmbiosTableEntryPoint64Bit,
      EntryLength64Bit
      );

    CopyMem (
      (UINT8 *)((UINTN)mPrivateData->SmbiosStoreArea64Bit + EntryLength64Bit),
      mPrivateData->SmbiosTable64Bit,
      mPrivateData->SmbiosTableEntryPoint64Bit->TableMaximumSize
      );
    
    //
    // change pointer
    //
    mPrivateData->SmbiosTableEntryPoint64Bit = (SMBIOS_TABLE_3_0_ENTRY_POINT *)mPrivateData->SmbiosStoreArea64Bit;
    mPrivateData->SmbiosTable64Bit = (SMBIOS_STRUCTURE *)((UINTN)mPrivateData->SmbiosStoreArea64Bit + EntryLength64Bit);
  }

}


/**
 VirtualAddressChange callback function

 @param [in]   Event            Registered callback event
 @param [in]   Context

 @retval None.

**/
VOID
EFIAPI
VirtualAddressChangeCallBack (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS    Status;

  if (mPrivateData == NULL) {
    return;
  }

  if (mPrivateData->SmbiosTableEntryPoint != NULL) {
    Status = EfiConvertPointer (0, (VOID **)&mPrivateData->SmbiosTableEntryPoint);
    ASSERT_EFI_ERROR (Status);
  }

  if (mPrivateData->SmbiosTableEntryPoint64Bit != NULL) {
    Status = EfiConvertPointer (0, (VOID **)&mPrivateData->SmbiosTableEntryPoint64Bit);
    ASSERT_EFI_ERROR (Status);
  }

  if (mPrivateData->SmbiosTable != NULL) {
    Status = EfiConvertPointer (0, (VOID **)&mPrivateData->SmbiosTable);
    ASSERT_EFI_ERROR (Status);
  }

  if (mPrivateData->SmbiosTable64Bit != NULL) {
    Status = EfiConvertPointer (0, (VOID **)&mPrivateData->SmbiosTable64Bit);
    ASSERT_EFI_ERROR (Status);
  }

  if (mPrivateData->GPNVHeader != NULL) {
    EfiConvertPointer (0, (VOID **)&mPrivateData->GPNVHeader);
  }

  if (mPrivateData->UpdatableStrings != NULL) {
    EfiConvertPointer (0, (VOID **)&mPrivateData->UpdatableStrings);
  }

  if (mPrivateData->DmiVariableBuf != NULL) {
    EfiConvertPointer (0, (VOID **)&mPrivateData->DmiVariableBuf);
  }

  if (mPrivateData->SmbiosStoreArea != NULL) {
    Status = EfiConvertPointer (0, (VOID **)&mPrivateData->SmbiosStoreArea);
    ASSERT_EFI_ERROR (Status);
  }

  if (mPrivateData->SmbiosStoreArea64Bit != NULL) {
    Status = EfiConvertPointer (0, (VOID **)&mPrivateData->SmbiosStoreArea64Bit);
    ASSERT_EFI_ERROR (Status);
  }

  if (mPrivateData->TempStoreArea != NULL) {
    EfiConvertPointer (0, (VOID **)&mPrivateData->TempStoreArea);
  }

  Status = EfiConvertPointer (0, (VOID **)&mPrivateData);
  ASSERT_EFI_ERROR (Status);

}


/**
 Driver entry point

 @param [in]   ImageHandle      The image handle
 @param [in]   SystemTable

 @retval EFI_SUCCESS            Command completed successfully
 @return Other value               Runtime memory allocation failed
                                or IrsiRegisterFunction failed

**/
EFI_STATUS
EFIAPI
PnpRuntimeDxeInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  DMI_UPDATABLE_STRING                  *TempStringTable;
  UINTN                                 VarSize;
  UINT32                                Index;
  UINT32                                Command;
  BOOLEAN                               NeedFreePnpTable;
  OEM_GPNV_MAP                         *OemGPNVMap = NULL;

  //
  // initialize global variable
  //
  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  sizeof (PNP_RUNTIME_PRIVATE_DATA),
                  (VOID **)&mPrivateData
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TempStringTable = NULL;

  mPrivateData->UpdatableStringCount = 0;
  mPrivateData->UpdatableStrings = NULL;
  mPrivateData->OemGPNVHandleCount = 0;
  mPrivateData->SMBIOSTableEntryAddress = 0;
  mPrivateData->SMBIOSTableEntryAddress64Bit = 0;
  mPrivateData->SmbiosTableEntryPoint = NULL;
  mPrivateData->SmbiosTableEntryPoint64Bit = NULL;
  mPrivateData->SmbiosTable = NULL;
  mPrivateData->SmbiosTable64Bit = NULL;

  
  VarSize = (UINTN) FdmGetNAtSize(&gH2OFlashMapRegionSmbiosUpdateGuid, 1);
  Status = gBS->AllocatePool (
                  EfiRuntimeServicesData,
                  VarSize,
                  (VOID **)&mPrivateData->DmiVariableBuf
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  gBS->SetMem (mPrivateData->DmiVariableBuf, VarSize, 0xFF);


  //
  // OemServices (Dxe)

  NeedFreePnpTable = TRUE;
  Status = OemSvcDxeInstallPnpStringTable (
             &mPrivateData->UpdatableStringCount,
             &TempStringTable
             );
  if (Status == EFI_UNSUPPORTED) {
    NeedFreePnpTable = FALSE;
    TempStringTable = PcdGetPtr(PcdSmbiosUpdatableStringTable);
    mPrivateData->UpdatableStringCount = FixedPcdGetPtrSize(PcdSmbiosUpdatableStringTable) / sizeof(DMI_UPDATABLE_STRING);
  }

  if ((TempStringTable != NULL) && (mPrivateData->UpdatableStringCount != 0)) {
    Status = gBS->AllocatePool (
                    EfiRuntimeServicesData,
                    ((mPrivateData->UpdatableStringCount) * sizeof (DMI_UPDATABLE_STRING)),
                    (VOID **)&mPrivateData->UpdatableStrings
                    );
    if (!EFI_ERROR (Status)) {
      gBS->CopyMem (
             mPrivateData->UpdatableStrings,
             TempStringTable,
             ((mPrivateData->UpdatableStringCount) * sizeof (DMI_UPDATABLE_STRING))
             );
    }

    if (NeedFreePnpTable) {
      gBS->FreePool(TempStringTable);
    }
  }

  //
  // OemServices (Dxe)
  //
  Status = OemSvcDxeInstallPnpGpnvTable (&mPrivateData->OemGPNVHandleCount, &OemGPNVMap);
  if (Status == EFI_MEDIA_CHANGED && OemGPNVMap != NULL) {
    mIsOemGPNVMap = TRUE;
  } else {
    mIsOemGPNVMap = FALSE;
  }

  //
  // This event will be triggered after SMBIOS table entry point is ready.
  // So, we can get the address of entry point and save it.
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK - 1,
             SmbiosGetTableEntryPointCallback,
             NULL,
             &mSmbiosGetTableEvent
             );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  VirtualAddressChangeCallBack,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mVirtualAddressChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register PnpRuntime services using IrsiLib
  //
  Command = PNP_0X50_SERVICES;
  for (Index = 0; Index < (sizeof (mPnpDmiFunctions) / sizeof (PNP_REDIRECTED_ENTRY_POINT)); Index++) {
    Status = IrsiRegisterFunction (
               &gIsbServicesGuid,
               Command,
               (IRSI_FUNCTION)mPnpDmiFunctions[Index]
               );

    if (EFI_ERROR (Status)) {
      break;
    }

    Command++;
  }

  return Status;
}

