/** @file

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
/*++
/** @file

  Implement all four UEFI Runtime Variable services for the nonvolatile
  and volatile storage space and install variable architecture protocol.
  
Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include "VariableRuntimeCmos.h"

EFI_HANDLE                     mHandle                    = NULL;
EFI_EVENT                      mVirtualAddressChangeEvent = NULL;
EFI_EVENT                      mRegistration           = NULL;

//
// VariableName for Osnib variable
//
#define OSNIB_VARARIABLE_NAME  L"OsnibVar"

VARIABLE_CMOS_MODULE_GLOBAL  *mVariableCmosModuleGlobal;

/**
  Return TRUE if ExitBootServices () has been called.
  
  @retval TRUE If ExitBootServices () has been called.
**/
BOOLEAN
AtRuntime (
  VOID
  )
{
  return EfiAtRuntime ();
}

/**
  Acquires lock only at boot time. Simply returns at runtime.

  This is a temperary function that will be removed when
  EfiAcquireLock() in UefiLib can handle the call in UEFI
  Runtimer driver in RT phase.
  It calls EfiAcquireLock() at boot time, and simply returns
  at runtime.

  @param  Lock         A pointer to the lock to acquire.

**/
VOID
AcquireLockOnlyAtBootTime (
  IN EFI_LOCK                             *Lock
  )
{
  if (!AtRuntime ()) {
    EfiAcquireLock (Lock);
  }
}


/**
  Releases lock only at boot time. Simply returns at runtime.

  This is a temperary function which will be removed when
  EfiReleaseLock() in UefiLib can handle the call in UEFI
  Runtimer driver in RT phase.
  It calls EfiReleaseLock() at boot time and simply returns
  at runtime.

  @param  Lock         A pointer to the lock to release.

**/
VOID
ReleaseLockOnlyAtBootTime (
  IN EFI_LOCK                             *Lock
  )
{
  if (!AtRuntime ()) {
    EfiReleaseLock (Lock);
  }
}

UINT8 
CmosGetOsNibBase (
 VOID
    )
{
  UINT8         Index;
  UINT32  Data32;
  UINT8   OsNibOffset;

  Data32 = 0;
  OsNibOffset = EFI_OSNIB_INVALID_OFFSET;
  
  for(Index =0; Index < OSNIB_LENGTH; Index ++){
    Data32= ReadCmos32((UINT8)(EFI_OSNIB_INITIAL_OFFSET + Index));
    if (Data32 == OSNIB_SIGNATURE){
      OsNibOffset = Index + EFI_OSNIB_INITIAL_OFFSET;
      break;
    }
  }

  return OsNibOffset;
}

EFI_STATUS
CmosGetOsNib (
  IN OUT VOID * OsNib
    )
{
  UINTN         Index;
  UINT8         OsNibCmosOffset;
  EFI_STATUS    Status;
  OSNIB_64B     *OsNibBuffer;

  Status = EFI_SUCCESS;

  OsNibBuffer = (OSNIB_64B * )OsNib;
  if (!OsNibBuffer){return EFI_INVALID_PARAMETER;}
  
  OsNibCmosOffset = CmosGetOsNibBase();
  if (OsNibCmosOffset == EFI_OSNIB_INVALID_OFFSET){
    Status = EFI_NOT_FOUND;
  } else {
    for(Index =0; Index < OSNIB_LENGTH; Index +=4){
     *(UINT32 *) ((UINTN)OsNibBuffer + Index) = ReadCmos32((UINT8)(OsNibCmosOffset + Index));
     //DEBUG((EFI_D_INFO, "ReadCmos32\n"));
    }
  }
  return Status;
}

EFI_STATUS
CmosSetOsNib (
 IN OUT VOID * OsNib
    )
{
  UINTN         Index;
  UINT8         OsNibCmosOffset;
  EFI_STATUS    Status;
  OSNIB_64B     *OsNibBuffer;

  Status = EFI_SUCCESS;

  OsNibBuffer = (OSNIB_64B * )OsNib;
  if (!OsNibBuffer){return EFI_INVALID_PARAMETER;}
  
  OsNibCmosOffset = CmosGetOsNibBase();
  if (OsNibCmosOffset == EFI_OSNIB_INVALID_OFFSET){
    OsNibCmosOffset = EFI_OSNIB_INITIAL_OFFSET;
  } 

  if ((OsNibCmosOffset + OSNIB_LENGTH) > (EFI_OSNIB_INVALID_OFFSET +1)){
    Status = EFI_BUFFER_TOO_SMALL;
  } else {
    //
    // first to calculate checksum
    //
    for(Index =0; Index < OSNIB_LENGTH; Index +=4){
      WriteCmos32((UINT8)(OsNibCmosOffset + Index), *(UINT32 *) ((UINTN)OsNibBuffer + Index));
      //DEBUG((EFI_D_INFO, "After Write, ReadCmos32[%02x]= %08x\n", OsNibCmosOffset+Index, ReadCmos32((UINT8)(OsNibCmosOffset + Index))));
    }
  }
 
  return Status;
}


EFI_STATUS
EFIAPI
VariableServiceCmosGetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data
  )
{
  EFI_STATUS Status;
  
  if (CompareGuid (VendorGuid, &gAndroidVariableGuid) 
    && (StrCmp (VariableName, OSNIB_VARARIABLE_NAME) == 0)){
    //DEBUG((EFI_D_INFO, "OSNIB GetVar...in\n"));
    //
    // if CMOS related, check DataSize/Data buffer first
    //
    if (!DataSize || !Data){
      Status = EFI_INVALID_PARAMETER;
      //DEBUG((EFI_D_INFO, "error...EFI_INVALID_PARAMETER\n"));
    } else if (*DataSize < OSNIB_LENGTH){
      //
      // tell caller the size of this var
      //
      *DataSize = OSNIB_LENGTH;
      Status = EFI_BUFFER_TOO_SMALL;
      //DEBUG((EFI_D_INFO, "error...EFI_BUFFER_TOO_SMALL\n"));
    } else {
      //
      // check OsNib valid or not
      //
      *DataSize = 0;
      Status = CmosGetOsNib(Data);
      //DEBUG((EFI_D_INFO, "CmosGetOsNib - Status = %08x\n", Status));
      if (Status == EFI_SUCCESS){
        *DataSize = OSNIB_LENGTH;
      }
    }    
  } else {
    ////DEBUG((EFI_D_INFO, "VariableServiceCmosGetVariable...in\n"));
    Status = mVariableCmosModuleGlobal->GetVariableOriginal(VariableName, VendorGuid, Attributes, DataSize, Data);
  }
  
  return Status;

}

EFI_STATUS
EFIAPI
VariableServiceCmosGetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  )
{
  EFI_STATUS Status;

  if (CompareGuid (VendorGuid, &gAndroidVariableGuid) 
    && (StrCmp (VariableName, OSNIB_VARARIABLE_NAME) == 0)){
    //
    // if CMOS related, only one set of var which is Osnib exists, no next var.
    //
    Status = EFI_NOT_FOUND;
  } else {  
    ////DEBUG((EFI_D_INFO, "VariableServiceCmosGetNextVariableName...in\n"));
    Status = mVariableCmosModuleGlobal->GetNextVariableNameOriginal (VariableNameSize, VariableName, VendorGuid);
  }
  
  return Status;
}

EFI_STATUS
EFIAPI
VariableServiceCmosSetVariable (
  IN  CHAR16                       *VariableName,
  IN  EFI_GUID                     *VendorGuid,
  IN  UINT32                       Attributes,
  IN  UINTN                        DataSize,
  IN  VOID                         *Data
  )
{
  EFI_STATUS Status;

  if (CompareGuid (VendorGuid, &gAndroidVariableGuid) 
    && (StrCmp (VariableName, OSNIB_VARARIABLE_NAME) == 0)){
    //
    // if CMOS related, check DataSize/Data buffer first
    //
    //DEBUG((EFI_D_INFO, "OSNIB SetVar...in\n"));
    if ((DataSize > OSNIB_LENGTH) || !Data){
      //DEBUG((EFI_D_INFO, "error...EFI_INVALID_PARAMETER\n"));
      Status = EFI_INVALID_PARAMETER;
    } else {
      //
      // set Osnib
      //      
      Status = CmosSetOsNib(Data);
      //DEBUG((EFI_D_INFO, "CmosSetOsNib - Status = %08x\n", Status));
    }    
  } else {
    ////DEBUG((EFI_D_INFO, "VariableServiceCmosSetVariable...in\n"));
    Status = mVariableCmosModuleGlobal->SetVariableOriginal(VariableName, VendorGuid, Attributes, DataSize, Data);
  }
  
  return Status;

}


/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
VariableCmosAddressChangeEvent (
  IN EFI_EVENT                            Event,
  IN VOID                                 *Context
  )
{
  EfiConvertPointer (0x0, (VOID **) &mVariableCmosModuleGlobal->SetVariableOriginal);
  EfiConvertPointer (0x0, (VOID **) &mVariableCmosModuleGlobal->GetNextVariableNameOriginal);
  EfiConvertPointer (0x0, (VOID **) &mVariableCmosModuleGlobal->GetVariableOriginal);
  EfiConvertPointer (0x0, (VOID **) &mVariableCmosModuleGlobal);
  
}


VOID
EFIAPI
VariableCmosFilterInstallEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  This function is invoked when LegacyBios protocol is installed, we must
  allocate reserved memory under 1M for AP.

Arguments:

  Event   - The triggered event.
  Context - Context for this event.

Returns:

  None

--*/
{

  EFI_SYSTEM_TABLE     *SystemTable;
  VOID                 *Protocol;
  EFI_LOCK             Lock;
  EFI_STATUS           Status;
  
  if (Context == NULL) {
    return;
  }

  SystemTable = (EFI_SYSTEM_TABLE *)Context;

  Status = gBS->LocateProtocol (
                  &gEfiVariableArchProtocolGuid,
                  NULL,
                  &Protocol
                  );
  
  if (Status != EFI_SUCCESS){return;}
  Status = gBS->LocateProtocol (
                  &gEfiVariableWriteArchProtocolGuid,
                  NULL,
                  &Protocol
                  );
  
  if (Status != EFI_SUCCESS){return;}

  
  //
  // Both Variable protocols are ready, now try to add filter layer
  //
  //DEBUG((EFI_D_INFO, "VariableCmosFilterInstallEvent...in\n"));
  EfiInitializeLock (&Lock, TPL_NOTIFY);
  AcquireLockOnlyAtBootTime(&Lock);

  mVariableCmosModuleGlobal->GetVariableOriginal         = SystemTable->RuntimeServices->GetVariable;
  mVariableCmosModuleGlobal->GetNextVariableNameOriginal = SystemTable->RuntimeServices->GetNextVariableName;
  mVariableCmosModuleGlobal->SetVariableOriginal         = SystemTable->RuntimeServices->SetVariable;

  SystemTable->RuntimeServices->GetVariable         = VariableServiceCmosGetVariable;
  SystemTable->RuntimeServices->GetNextVariableName = VariableServiceCmosGetNextVariableName;
  SystemTable->RuntimeServices->SetVariable         = VariableServiceCmosSetVariable;
  // 
  // We don't do filter for variable query, this request Android don't use this to get CMOS storage info for now
  //
  //SystemTable->RuntimeServices->QueryVariableInfo   = VariableServiceQueryVariableInfo;

  ReleaseLockOnlyAtBootTime(&Lock);
  
  //
  // Both Variable protocols are ready, now try to add filter layer
  //
  //DEBUG((EFI_D_INFO, "VariableCmosFilter installed OK\n"));
}

/**
  Variable Driver main entry point. The Variable driver places the 4 EFI
  runtime services in the EFI System Table and installs arch protocols 
  for variable read and write services being availible. It also registers
  a notification function for an EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
VariableCmosFilterInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;

  Status = EFI_NOT_STARTED;
  
  mVariableCmosModuleGlobal = AllocateRuntimeZeroPool(sizeof (VARIABLE_CMOS_MODULE_GLOBAL));

  if (mVariableCmosModuleGlobal){
    //
    // Register FtwNotificationEvent () notify function.
    // 
    EfiCreateProtocolNotifyEvent (
      &gEfiVariableWriteArchProtocolGuid,
      TPL_CALLBACK,
      VariableCmosFilterInstallEvent,
      (VOID *)SystemTable,
      &mRegistration
      );

    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    VariableCmosAddressChangeEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &mVirtualAddressChangeEvent
                    );
    ASSERT_EFI_ERROR (Status);

    Status = EFI_SUCCESS;
  } 

  return Status;
  
}

