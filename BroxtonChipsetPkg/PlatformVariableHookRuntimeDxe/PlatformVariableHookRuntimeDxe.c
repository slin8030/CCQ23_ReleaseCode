/** @file
  If platform supports SMM mode, this dirver provides variable services in protected mode and SMM mode
  If platform doesn't support SMM mode, this driver provides variables services in protected mode.

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

#include "PlatformVariableHook.h"

//[-start-160308-IB10860193-add]//
#include <Guid/CustomVariable.h>
//[-end-160308-IB10860193-add]//
//[-start-160803-IB07220122-add]//
#include <Guid/MemoryConfigData.h>
#include <Guid/PlatformConfigDataGuid.h>
//[-end-160803-IB07220122-add]//

//[-start-161002-IB07400791-add]//
extern VOID  *mPlatformConfigDataLibHeci2Protocol;
//[-end-161002-IB07400791-add]//

#define PLATFORM_VARIABLE_HOOK_SHARE_DATA_PROTOCOL_GUID \
  { \
    0x6FAD2844, 0xE9B6, 0x46EF, 0x9B, 0xE6, 0xE9, 0x07, 0xB8, 0x94, 0x49, 0xEE \
  }
EFI_GUID                            mPlatformVariableHookShareDataProtocolGuid = PLATFORM_VARIABLE_HOOK_SHARE_DATA_PROTOCOL_GUID; 

//[-start-160803-IB07220122-add]//
extern EFI_GUID gEfiMemoryConfigVariableGuid;
//[-end-160803-IB07220122-add]//
EFI_GET_VARIABLE                    mOriginalRtGetVariable;
EFI_SET_VARIABLE                    mOriginalRtSetVariable;
EFI_GET_VARIABLE                    mOriginalSmmGetVariable;
EFI_SET_VARIABLE                    mOriginalSmmSetVariable;
ESAL_VARIABLE_GLOBAL                *mVariableModuleGlobal;
EFI_SMM_SYSTEM_TABLE2               *mSmst;
VOID                                *mHeci2Protocol = NULL;
PLATFORM_VARIABLE_HOOK_TABLE        VariableHookTable[] = {
//[-start-160530-IB08450346-remove]//
//--
//  //
//  // TPM 2.0
//  //
//  {
//    &gEfiTcg2PhysicalPresenceGuid,
//    TCG2_PHYSICAL_PRESENCE_VARIABLE,
//    PLATFORM_TREE_DATA_FILE_NAME,
//    OFFSET_PHYSICAL_PRESENCE,
//    sizeof (EFI_TCG2_PHYSICAL_PRESENCE),
//    0
//  },
//  {
//    &gEfiTcg2PhysicalPresenceGuid,
//    TCG2_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
//    PLATFORM_TREE_DATA_FILE_NAME,
//    OFFSET_PHYSICAL_PRESENCE_FLAG,
//    sizeof(EFI_TCG2_PHYSICAL_PRESENCE_FLAGS),
//    0
//  },
//  //
//  // TPM 1.2
//  //
//  {
//    &gEfiTrEEPhysicalPresenceGuid,
//    TREE_PHYSICAL_PRESENCE_VARIABLE,
//    PLATFORM_TCG_DATA_FILE_NAME,
//    OFFSET_PHYSICAL_PRESENCE,
//    sizeof(EFI_TREE_PHYSICAL_PRESENCE),
//    0    
//  },
//  {
//    &gEfiTrEEPhysicalPresenceGuid,
//    TREE_PHYSICAL_PRESENCE_FLAGS_VARIABLE,
//    PLATFORM_TCG_DATA_FILE_NAME,
//    OFFSET_PHYSICAL_PRESENCE_FLAG,
//    sizeof(EFI_TREE_PHYSICAL_PRESENCE_FLAGS),
//    0
//  },
//  //
//  // Memory overwrite
//  //
//  {
//    &gEfiMemoryOverwriteControlDataGuid, 
//    MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,  
//    PLATFORM_TREE_DATA_FILE_NAME,
//    OFFSET_MEMORY_OVERWRITE_CONTROL_DATA,
//    sizeof(UINT8),
//    0
//  },
//[-end-160530-IB08450346-remove]//
  //
  // Last entry
  //
  {
    NULL,
    NULL,
    NULL,
    0,
    0,
    0,
  }
};

/**
  Find corresponding hook table entry for specified variable.

  @param  VariableName   Variable name
  @param  VendorGuid     Vendor GUID

  @retval Not NULL       Pointer to hook table entry
  @retval NULL           It is not hook variable

**/
PLATFORM_VARIABLE_HOOK_TABLE *
CheckVariable(
  IN CONST   CHAR16        *VariableName,
  IN CONST   EFI_GUID      *VendorGuid
)
{
  UINTN Index;
  for (Index = 0; VariableHookTable[Index].VendorGuid != NULL; Index ++) {
    if (CompareGuid(VendorGuid,VariableHookTable[Index].VendorGuid) && StrCmp(VariableName,VariableHookTable[Index].VariableName) == 0) {
    return &VariableHookTable[Index];
    }
  }
  return NULL;
}



/**
  Do hooking process of GetVariable.

  @param[in]       VariableName  A Null-terminated string that is the name of the vendor's
                                 variable.
  @param[in]       VendorGuid    A unique identifier for the vendor.
  @param[out]      Attributes    If not NULL, a pointer to the memory location to return the
                                 attributes bitmask for the variable.
  @param[in, out]  DataSize      On input, the size in bytes of the return Data buffer.
                                 On output the size of data returned in Data.
  @param[out]      Data          The buffer to return the contents of the variable.
  @param[out]      RetStatus     Return the status of hook variable.

  @retval EFI_SUCCESS            Hook variable process done.
  @retval EFI_NOT_FOUND          It is not hook variable.

**/
EFI_STATUS
PlatformGetVariableHook (
  IN CONST CHAR16            *VariableName,
  IN CONST EFI_GUID          *VendorGuid,
  OUT      UINT32            *Attributes OPTIONAL,
  IN OUT   UINTN             *DataSize,
  OUT      VOID              *Data,
  OUT      EFI_STATUS        *RetStatus
)
{
  PLATFORM_VARIABLE_HOOK_TABLE *Hook;
  VOID *HookDataBuffer;

  Hook = CheckVariable(VariableName, VendorGuid);
  if (Hook == NULL) {
    return EFI_NOT_FOUND;
  }
  
  if (Attributes != NULL) {
    *Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  }
  if (*DataSize < Hook->Size) {
    *DataSize = Hook->Size;
    *RetStatus = EFI_BUFFER_TOO_SMALL;
    return EFI_SUCCESS;   
  }
  
  //
  // Read from buffer
  //
  HookDataBuffer = (VOID *)(UINTN)mVariableModuleGlobal->PlatformHookVariableStore;
  CopyMem (Data, (UINT8 *)HookDataBuffer + Hook->MemoryBufferOffset, Hook->Size);
  *DataSize = Hook->Size; 
  *RetStatus = EFI_SUCCESS;

  return EFI_SUCCESS;
}

/**
  Do hooking process of SetVariable.

  @param[in]  VariableName       A Null-terminated string that is the name of the vendor's variable.
                                 Each VariableName is unique for each VendorGuid. VariableName must
                                 contain 1 or more characters. If VariableName is an empty string,
                                 then EFI_INVALID_PARAMETER is returned.
  @param[in]  VendorGuid         A unique identifier for the vendor.
  @param[in]  Attributes         Attributes bitmask to set for the variable.
  @param[in]  DataSize           The size in bytes of the Data buffer. Unless the EFI_VARIABLE_APPEND_WRITE, 
                                 EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS, or 
                                 EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS attribute is set, a size of zero 
                                 causes the variable to be deleted. When the EFI_VARIABLE_APPEND_WRITE attribute is 
                                 set, then a SetVariable() call with a DataSize of zero will not cause any change to 
                                 the variable value (the timestamp associated with the variable may be updated however 
                                 even if no new data value is provided,see the description of the 
                                 EFI_VARIABLE_AUTHENTICATION_2 descriptor below. In this case the DataSize will not 
                                 be zero since the EFI_VARIABLE_AUTHENTICATION_2 descriptor will be populated). 
  @param[in]  Data               The contents for the variable.
  @param[out] RetStatus          Return the status of hook variable.

  @retval EFI_SUCCESS            Hook variable process done.
  @retval EFI_NOT_FOUND          It is not hook variable.
**/
EFI_STATUS
PlatformSetVariableHook (
  IN CONST CHAR16                  *VariableName,
  IN CONST EFI_GUID                *VendorGuid,
  IN       UINT32                  Attributes,
  IN       UINTN                   DataSize,
  IN       VOID                    *Data,
  OUT      EFI_STATUS              *RetStatus
)
{
  PLATFORM_VARIABLE_HOOK_TABLE *Hook;
  VOID *HookDataBuffer;

  Hook = CheckVariable(VariableName, VendorGuid);
  if (Hook == NULL) {
    return EFI_NOT_FOUND;
  }

  if (Attributes != (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS)) {
    *RetStatus = EFI_INVALID_PARAMETER;
    return EFI_SUCCESS;    
  }
  ASSERT (DataSize <= Hook->Size);
  if (DataSize > Hook->Size) {
    DataSize = Hook->Size;
  }

  if (mHeci2Protocol != NULL) {
//[-start-160819-IB07220131-modify]//
    *RetStatus = Heci2WriteNVMFile(
                    Hook->NvmName,
                    (UINT32)Hook->Offset,
                    Data,
                    DataSize,
                    FALSE
                    );
//[-end-160819-IB07220131-modify]//
  } else {
//[-start-160808-IB07220123-modify]//
    *RetStatus = HeciWriteNVMFile(Hook->NvmName,(UINT32)Hook->Offset,Data,DataSize,FALSE);
//[-end-160808-IB07220123-modify]//
  }

  HookDataBuffer = (VOID *)(UINTN)mVariableModuleGlobal->PlatformHookVariableStore;
  CopyMem ((UINT8 *)HookDataBuffer + Hook->MemoryBufferOffset, Data, Hook->Size);

  return EFI_SUCCESS;
}


/**
  This function allows the caller to determine if UEFI ExitBootServices() has been called.

  This function returns TRUE after all the EVT_SIGNAL_EXIT_BOOT_SERVICES functions have
  executed as a result of the OS calling ExitBootServices().  Prior to this time FALSE
  is returned. This function is used by runtime code to decide it is legal to access
  services that go away after ExitBootServices().

  @retval  TRUE  The system has finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.
  @retval  FALSE The system has not finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.

**/
BOOLEAN
EFIAPI
AtRuntime (
  VOID
  )
{
  return mVariableModuleGlobal->AtRuntime; 
}



/**
  Returns the value of a variable.

  @param[in]       VariableName  A Null-terminated string that is the name of the vendor's
                                 variable.
  @param[in]       VendorGuid    A unique identifier for the vendor.
  @param[out]      Attributes    If not NULL, a pointer to the memory location to return the
                                 attributes bitmask for the variable.
  @param[in, out]  DataSize      On input, the size in bytes of the return Data buffer.
                                 On output the size of data returned in Data.
  @param[out]      Data          The buffer to return the contents of the variable.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_NOT_FOUND          The variable was not found.
  @retval EFI_BUFFER_TOO_SMALL   The DataSize is too small for the result.
  @retval EFI_INVALID_PARAMETER  VariableName is NULL.
  @retval EFI_INVALID_PARAMETER  VendorGuid is NULL.
  @retval EFI_INVALID_PARAMETER  DataSize is NULL.
  @retval EFI_INVALID_PARAMETER  The DataSize is not too small and Data is NULL.
  @retval EFI_DEVICE_ERROR       The variable could not be retrieved due to a hardware error.
  @retval EFI_SECURITY_VIOLATION The variable could not be retrieved due to an authentication failure.

**/
EFI_STATUS
EFIAPI
RuntimeDxeGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      * VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  EFI_STATUS Status;

  if (PlatformGetVariableHook(VariableName, VendorGuid, Attributes, DataSize, Data, &Status) == EFI_SUCCESS) {
    return Status;
  }

//[-start-160803-IB07220122-add]//
  if (!AtRuntime() &&
      CompareGuid(VendorGuid, &gEfiMemoryConfigVariableGuid) && 
      StrCmp(VariableName, EFI_MEMORY_CONFIG_VARIABLE_NAME) == 0) {   

    if (Attributes != NULL) {
      *Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    }
    return GetMemoryConfigData(&gPlatformMemoryConfigDataGuid, &Data, DataSize);
  }

  if (!AtRuntime() &&
      CompareGuid(VendorGuid, &gEfiMemoryConfigVariableGuid) && 
      StrCmp(VariableName, L"MemoryBootData") == 0) {   

    if (Attributes != NULL) {
      *Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    }
    return GetMemoryConfigData(&gBootVariableNvDataGuid, &Data, DataSize);
  }
//[-end-160803-IB07220122-add]//

  return mOriginalRtGetVariable (
           VariableName,
           VendorGuid,
           Attributes,
           DataSize,
           Data
           );
}

/**
  Sets the value of a variable.

  @param[in]  VariableName       A Null-terminated string that is the name of the vendor's variable.
                                 Each VariableName is unique for each VendorGuid. VariableName must
                                 contain 1 or more characters. If VariableName is an empty string,
                                 then EFI_INVALID_PARAMETER is returned.
  @param[in]  VendorGuid         A unique identifier for the vendor.
  @param[in]  Attributes         Attributes bitmask to set for the variable.
  @param[in]  DataSize           The size in bytes of the Data buffer. Unless the EFI_VARIABLE_APPEND_WRITE, 
                                 EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS, or 
                                 EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS attribute is set, a size of zero 
                                 causes the variable to be deleted. When the EFI_VARIABLE_APPEND_WRITE attribute is 
                                 set, then a SetVariable() call with a DataSize of zero will not cause any change to 
                                 the variable value (the timestamp associated with the variable may be updated however 
                                 even if no new data value is provided,see the description of the 
                                 EFI_VARIABLE_AUTHENTICATION_2 descriptor below. In this case the DataSize will not 
                                 be zero since the EFI_VARIABLE_AUTHENTICATION_2 descriptor will be populated). 
  @param[in]  Data               The contents for the variable.

  @retval EFI_SUCCESS            The firmware has successfully stored the variable and its data as
                                 defined by the Attributes.
  @retval EFI_INVALID_PARAMETER  An invalid combination of attribute bits, name, and GUID was supplied, or the
                                 DataSize exceeds the maximum allowed.
  @retval EFI_INVALID_PARAMETER  VariableName is an empty string.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be retrieved due to a hardware error.
  @retval EFI_WRITE_PROTECTED    The variable in question is read-only.
  @retval EFI_WRITE_PROTECTED    The variable in question cannot be deleted.
  @retval EFI_SECURITY_VIOLATION The variable could not be written due to EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 
                                 or EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACESS being set, but the AuthInfo 
                                 does NOT pass the validation check carried out by the firmware.
  
  @retval EFI_NOT_FOUND          The variable trying to be updated or deleted was not found.

**/
EFI_STATUS
EFIAPI
RuntimeDxeSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  EFI_STATUS Status;
//[-start-160308-IB10860193-add]//
  UINTN      SetSetupAtRuntimeSize;
  UINT8      SetSetupAtRuntime;
//[-end-160308-IB10860193-add]//

  if (PlatformSetVariableHook(VariableName, VendorGuid, Attributes, DataSize, Data, &Status) == EFI_SUCCESS) {
    return Status;
  }
  Status = mOriginalRtSetVariable (
          VariableName,
          VendorGuid,
          Attributes,
          DataSize,
          Data
          );
  //
  // Workaround for PlatformConfigDataLib. If PlatformConfigDataLib's Setup data
  // does not synchorized with variable store. It may cause problems.
  //
  if (!AtRuntime() &&
      !EFI_ERROR(Status) && 
      CompareGuid(VendorGuid, &gSystemConfigurationGuid) && 
      StrCmp(VariableName, SETUP_VARIABLE_NAME) == 0) {   
    SetSystemConfigData(Data, DataSize);
  }

//[-start-160803-IB07220122-add]//
  if (!AtRuntime() &&
      !EFI_ERROR(Status) && 
      CompareGuid(VendorGuid, &gEfiMemoryConfigVariableGuid) && 
      StrCmp(VariableName, EFI_MEMORY_CONFIG_VARIABLE_NAME) == 0) {   
    SetMemoryConfigData(&gPlatformMemoryConfigDataGuid, Data, DataSize);
  }

  if (!AtRuntime() &&
      !EFI_ERROR(Status) && 
      CompareGuid(VendorGuid, &gEfiMemoryConfigVariableGuid) && 
      StrCmp(VariableName, L"MemoryBootData") == 0) {   
    SetMemoryConfigData(&gBootVariableNvDataGuid, Data, DataSize);
  }
//[-end-160803-IB07220122-add]//
//[-start-160308-IB10860193-add]//
  if (AtRuntime() &&
      !EFI_ERROR(Status) && 
      CompareGuid(VendorGuid, &gSystemConfigurationGuid) && 
      StrCmp(VariableName, SETUP_VARIABLE_NAME) == 0) {

    SetSetupAtRuntimeSize = sizeof (UINT8);
    SetSetupAtRuntime = 0x01;
    Status = RuntimeDxeSetVariable (
			   L"SetSetupAtRuntime",
			   &gCustomVariableGuid,
			   EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
			   SetSetupAtRuntimeSize,
			  (VOID*)&SetSetupAtRuntime
			  );
   }
//[-end-160308-IB10860193-add]//
  
  return Status;
}

/**
  Returns the value of a variable.

  @param[in]       VariableName  A Null-terminated string that is the name of the vendor's
                                 variable.
  @param[in]       VendorGuid    A unique identifier for the vendor.
  @param[out]      Attributes    If not NULL, a pointer to the memory location to return the
                                 attributes bitmask for the variable.
  @param[in, out]  DataSize      On input, the size in bytes of the return Data buffer.
                                 On output the size of data returned in Data.
  @param[out]      Data          The buffer to return the contents of the variable.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_NOT_FOUND          The variable was not found.
  @retval EFI_BUFFER_TOO_SMALL   The DataSize is too small for the result.
  @retval EFI_INVALID_PARAMETER  VariableName is NULL.
  @retval EFI_INVALID_PARAMETER  VendorGuid is NULL.
  @retval EFI_INVALID_PARAMETER  DataSize is NULL.
  @retval EFI_INVALID_PARAMETER  The DataSize is not too small and Data is NULL.
  @retval EFI_DEVICE_ERROR       The variable could not be retrieved due to a hardware error.
  @retval EFI_SECURITY_VIOLATION The variable could not be retrieved due to an authentication failure.

**/
EFI_STATUS
EFIAPI
SmmGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      * VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  EFI_STATUS Status;

  if (PlatformGetVariableHook(VariableName, VendorGuid, Attributes, DataSize, Data, &Status) == EFI_SUCCESS) {
    return Status;
  }
  return mOriginalSmmGetVariable (
           VariableName,
           VendorGuid,
           Attributes,
           DataSize,
           Data
           );
}


/**
  Sets the value of a variable.

  @param[in]  VariableName       A Null-terminated string that is the name of the vendor's variable.
                                 Each VariableName is unique for each VendorGuid. VariableName must
                                 contain 1 or more characters. If VariableName is an empty string,
                                 then EFI_INVALID_PARAMETER is returned.
  @param[in]  VendorGuid         A unique identifier for the vendor.
  @param[in]  Attributes         Attributes bitmask to set for the variable.
  @param[in]  DataSize           The size in bytes of the Data buffer. Unless the EFI_VARIABLE_APPEND_WRITE, 
                                 EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS, or 
                                 EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS attribute is set, a size of zero 
                                 causes the variable to be deleted. When the EFI_VARIABLE_APPEND_WRITE attribute is 
                                 set, then a SetVariable() call with a DataSize of zero will not cause any change to 
                                 the variable value (the timestamp associated with the variable may be updated however 
                                 even if no new data value is provided,see the description of the 
                                 EFI_VARIABLE_AUTHENTICATION_2 descriptor below. In this case the DataSize will not 
                                 be zero since the EFI_VARIABLE_AUTHENTICATION_2 descriptor will be populated). 
  @param[in]  Data               The contents for the variable.

  @retval EFI_SUCCESS            The firmware has successfully stored the variable and its data as
                                 defined by the Attributes.
  @retval EFI_INVALID_PARAMETER  An invalid combination of attribute bits, name, and GUID was supplied, or the
                                 DataSize exceeds the maximum allowed.
  @retval EFI_INVALID_PARAMETER  VariableName is an empty string.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be retrieved due to a hardware error.
  @retval EFI_WRITE_PROTECTED    The variable in question is read-only.
  @retval EFI_WRITE_PROTECTED    The variable in question cannot be deleted.
  @retval EFI_SECURITY_VIOLATION The variable could not be written due to EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS 
                                 or EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACESS being set, but the AuthInfo 
                                 does NOT pass the validation check carried out by the firmware.
  
  @retval EFI_NOT_FOUND          The variable trying to be updated or deleted was not found.

**/
EFI_STATUS
EFIAPI
SmmSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  EFI_STATUS       Status;
//[-start-160308-IB10860193-add]//
  UINTN            SetSetupInSmmSize;
  UINT8            SetSetupInSmm;
//[-end-160308-IB10860193-add]//
  if (PlatformSetVariableHook(VariableName, VendorGuid, Attributes, DataSize, Data, &Status) == EFI_SUCCESS) {
    return Status;
  }
  Status = mOriginalSmmSetVariable (
             VariableName,
             VendorGuid,
             Attributes,
             DataSize,
             Data
             );

//[-start-160308-IB10860193-add]//
  if (!EFI_ERROR(Status) && 
      CompareGuid(VendorGuid, &gSystemConfigurationGuid) && 
      StrCmp(VariableName, SETUP_VARIABLE_NAME) == 0) {

    SetSetupInSmmSize = sizeof (UINT8);
    SetSetupInSmm = 0x01;
    Status = SmmSetVariable (
		   L"SetSetupInSmm",
		   &gCustomVariableGuid,
		   EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
		   SetSetupInSmmSize,
		  (VOID*)&SetSetupInSmm
		  );
//[-start-161002-IB07400791-add]//
    if ((mSmst != NULL) && (mHeci2Protocol != NULL)) {
      mPlatformConfigDataLibHeci2Protocol = mHeci2Protocol;
      SetSystemConfigData(Data, DataSize);
    }
//[-end-161002-IB07400791-add]//
  }		  
//[-end-160308-IB10860193-add]//
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
VariableAddressChangeEvent (
  IN EFI_EVENT                            Event,
  IN VOID                                 *Context
  )
{
  //
  // convert relative base addresses or pointers
  //
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PlatformHookVariableStore);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal);

  gRT->ConvertPointer (0x0, (VOID **) &mOriginalRtGetVariable);
  gRT->ConvertPointer (0x0, (VOID **) &mOriginalRtSetVariable);

  gRT->ConvertPointer (0x0, (VOID **) &mHeci2Protocol);
}

/**
  Notification function of gEfiEventExitBootServicesGuid.

  This is a notification function registered on gEfiEventExitBootServicesGuid event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT                            Event,
  IN VOID                                 *Context
  )
{

  mVariableModuleGlobal->AtRuntime = TRUE;
  gBS->CloseEvent (Event);
  gBS->LocateProtocol (
          &gEfiHeciProtocolGuid,
          NULL,
          &mHeci2Protocol
          );
}

/**
  According to system mode to allocate pool.
  Allocate BootServicesData pool in protect mode if Runtime is FALSE.
  Allocate EfiRuntimeServicesData pool in protect mode if Runtime is TRUE.
  Allocate memory from SMM ram if system in SMM mode.

  @param[in] Size    The size of buffer to allocate
  @param[in] Runtime Runtime Data or not.

  @return NULL       Buffer unsuccessfully allocated.
  @return Other      Buffer successfully allocated.
**/
VOID *
VariableAllocateZeroBuffer (
  IN UINTN     Size,
  IN BOOLEAN   Runtime
  )
{
  VOID           *Buffer;
  EFI_STATUS     Status;

  if (mSmst == NULL) {
    Status = gBS->AllocatePool (
                    Runtime ? EfiRuntimeServicesData : EfiBootServicesData,
                    Size,
                    &Buffer
                    );
  } else {
    Status = mSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      Size,
                      &Buffer
                      );
  }

  if (!EFI_ERROR (Status)) {
    ZeroMem (Buffer, Size);
  } else {
    Buffer = NULL;
  }
  return Buffer;
}

/**
  Initializes variable store area for non-volatile and volatile variable.

  @retval EFI_SUCCESS           Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES  Fail to allocate enough memory resource.

**/
EFI_STATUS
VariableCommonInitialize (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            NewHandle;
  UINTN                                 Index;
  UINTN                                 HookDataBufferSize;
  UINTN                                 FileSize;
  VOID                                  *PlatformHookVariableStore;  

  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate memory for volatile variable store
  //
  POST_CODE (DXE_VARIABLE_INIT);

  mVariableModuleGlobal = VariableAllocateZeroBuffer (sizeof (ESAL_VARIABLE_GLOBAL), TRUE);
  if (mVariableModuleGlobal == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }

  //
  // Initialize platform hook variable data
  //
  Status = PlatformHookAssignPlacement (
                  VariableHookTable,
                  &HookDataBufferSize
                  );
  ASSERT_EFI_ERROR (Status);
  PlatformHookVariableStore = VariableAllocateZeroBuffer (HookDataBufferSize, TRUE);
  if (PlatformHookVariableStore == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Shutdown;
  }
  mVariableModuleGlobal->PlatformHookVariableStore = (EFI_PHYSICAL_ADDRESS)(UINTN)PlatformHookVariableStore;
  for (Index = 0; VariableHookTable[Index].VendorGuid != NULL; Index ++) {

    Status = HeciGetNVMFileSize(VariableHookTable[Index].NvmName, &FileSize);
    if (EFI_ERROR(Status) || (FileSize < (VariableHookTable[Index].Offset + VariableHookTable[Index].Size))) {
//[-start-160808-IB07220123-modify]//
      Status = HeciWriteNVMFile (
                      VariableHookTable[Index].NvmName,
                      (UINT32)VariableHookTable[Index].Offset,
                      (UINT8 *)PlatformHookVariableStore + VariableHookTable[Index].MemoryBufferOffset,
                      VariableHookTable[Index].Size,
                      FALSE
                      );
//[-end-160808-IB07220123-modify]//
      ASSERT_EFI_ERROR (Status);
    } else {
      FileSize = VariableHookTable[Index].Size;
      Status = HeciReadNVMFile (
                      VariableHookTable[Index].NvmName, 
                      (UINT32)VariableHookTable[Index].Offset, 
                      (UINT8 *)PlatformHookVariableStore + VariableHookTable[Index].MemoryBufferOffset, 
                      &FileSize
                      );
      ASSERT_EFI_ERROR (Status);
    }    
    
  }

  //
  // Install gEfiNonVolatileVariableProtocolGuid to let platform know support
  // non-volatile variable and also passes some information to SMM variable driver.
  //
  NewHandle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &NewHandle,
                  &mPlatformVariableHookShareDataProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  mVariableModuleGlobal
                  );
  if (EFI_ERROR (Status)) {
    goto Shutdown;
  }

Shutdown:
  return Status;
}

/**
  Assign hook variable elements offsetds.

  @param  HookTable           Pointer to hook variable table.
  @param  HookDataBufferSize  Pointer to a UINTN to receieve buffer size for
                              hook data buffer. 

  @retval EFI_SUCCESS         Operation successfully completed.

**/
EFI_STATUS
PlatformHookAssignPlacement (
  IN  PLATFORM_VARIABLE_HOOK_TABLE  *HookTable,
  OUT UINTN                         *HookDataBufferSize
)
{
  UINTN                                 Index;
  UINTN                                 CurrentOffset;
    
  CurrentOffset = 0;
  for (Index = 0; HookTable[Index].VendorGuid != NULL; Index ++) {
    HookTable[Index].MemoryBufferOffset = CurrentOffset;
    CurrentOffset += HookTable[Index].Size;
  }
  
  if (HookDataBufferSize != NULL) {
    *HookDataBufferSize = CurrentOffset;
  }
  return EFI_SUCCESS;
} 



/**
  Initialize SMM mode NV data

  @param  SmmBase        Pointer to EFI_SMM_BASE2_PROTOCOL

  @retval EFI_SUCCESS    Initialize SMM mode NV data successful.
  @retval Other          Any error occured during initialize SMM NV data.

**/
EFI_STATUS
EFIAPI
SmmNvsInitialize (
  IN  EFI_SMM_BASE2_PROTOCOL                 *SmmBase
  )
{
  EFI_STATUS                      Status;
  ESAL_VARIABLE_GLOBAL            *ProtectedModeVariableModuleGlobal;

  Status = gBS->LocateProtocol (
                  &mPlatformVariableHookShareDataProtocolGuid,
                  NULL,
                  (VOID **)&ProtectedModeVariableModuleGlobal
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (
    &mVariableModuleGlobal->PlatformHookVariableStore,
    &ProtectedModeVariableModuleGlobal->PlatformHookVariableStore,
    sizeof (mVariableModuleGlobal->PlatformHookVariableStore)
    );

  return Status;
}


/**
  The notification function when variable services is available.

  @param EFI_EVENT         Event of the notification
  @param Context           not used in this function

  @retval none

**/
VOID
EFIAPI
RtVariableServiceAvailavbleEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS      Status;
  VOID            *DummyProtocol;
  
  Status = gBS->LocateProtocol(
                      &gEfiVariableArchProtocolGuid,
                      NULL,
                      &DummyProtocol
                      );
  if (EFI_ERROR(Status)) {
    return;
  }

  gBS->CloseEvent(Event);                        

  mOriginalRtGetVariable = gRT->GetVariable;
  mOriginalRtSetVariable = gRT->SetVariable;

  gRT->GetVariable = RuntimeDxeGetVariable;
  gRT->SetVariable = RuntimeDxeSetVariable;
  return;
}


/**
  The notification function when SMM variable services is available.

  @param  Protocol              Points to the protocol's unique identifier.
  @param  Interface             Points to the interface instance.
  @param  Handle                The handle on which the interface was installed.

**/
EFI_STATUS
EFIAPI
SmmVariableAvailableNotify (
  IN     CONST EFI_GUID                *Protocol,
  IN     VOID                          *Interface,
  IN     EFI_HANDLE                    Handle
  )
{
  EFI_SMM_VARIABLE_PROTOCOL             *SmmVariable;
  EFI_STATUS                            Status;
  
  Status = mSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **)&SmmVariable
                    );
                    
  if (!EFI_ERROR(Status)) {
    mOriginalSmmGetVariable = SmmVariable->SmmGetVariable;
    mOriginalSmmSetVariable = SmmVariable->SmmSetVariable;
    
    SmmVariable->SmmGetVariable = SmmGetVariable;
    SmmVariable->SmmSetVariable = SmmSetVariable;  
  }

  return EFI_SUCCESS;
}


/**
  Variable Driver main entry point. The Variable driver places the 4 EFI
  runtime services in the EFI System Table and installs arch protocols
  for variable read and write services being available. It also registers
  a notification function for an EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
PlatformVariableHookInitialize (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_EVENT                             Event;
  VOID                                  *Registration;
  EFI_SMM_BASE2_PROTOCOL                *SmmBase;
  BOOLEAN                               InSmm;
  EFI_SMM_VARIABLE_PROTOCOL             *SmmVariable;
  
//[-start-160614-IB07400744-add]//
  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    return EFI_UNSUPPORTED;
  }
//[-end-160614-IB07400744-add]//

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&SmmBase
                  );
  InSmm = FALSE;
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  }

  if (!InSmm) {

    if (!IsRuntimeDriver (ImageHandle)) {
       Status = RelocateImageToRuntimeDriver (ImageHandle);
       ASSERT_EFI_ERROR (Status);
        //
        // We only want to load runtime services code to memory and don't load boot services code to memory,
        // so just return EFI_ALREADY_STARTED if it isn't a runtime driver.
        //
        return EFI_ALREADY_STARTED;
    }
    //
    // Great!!! Now, this is a runtime driver.
    //
    Status = VariableCommonInitialize ();
    ASSERT_EFI_ERROR (Status);
    if (Status == EFI_UNSUPPORTED ) {
      return Status;
    }

    EfiCreateProtocolNotifyEvent  (
                          &gEfiVariableArchProtocolGuid,
                          TPL_NOTIFY,
                          RtVariableServiceAvailavbleEvent,
                          NULL,
                          &Registration
                          );

    //
    // Register the event to convert the pointer for runtime.
    //
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    VariableAddressChangeEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &Event
                    );
    ASSERT_EFI_ERROR (Status);

    //
    // Note!!!
    // This driver is SMM and RuntimeDxe hybrid driver and UefiRuntimeLib creates ExitBootServices and
    // VariableAddressChange event in constructor, so we cannot use this library to prevent from system
    // calls function located in SMM RAM in protected mode when ExitBootServices event is signaled.
    //
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    ExitBootServicesEvent,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &Event
                    );
    ASSERT_EFI_ERROR (Status);

  } else {

    //
    // Get Smm Syatem Table
    //
    Status = SmmBase->GetSmstLocation(
                        SmmBase,
                        &mSmst
                        );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mVariableModuleGlobal = VariableAllocateZeroBuffer (sizeof (ESAL_VARIABLE_GLOBAL), TRUE);

    if (mVariableModuleGlobal == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = mSmst->SmmLocateProtocol (
                      &gEfiSmmVariableProtocolGuid,
                      NULL,
                      (VOID **)&SmmVariable
                      );
    if (!EFI_ERROR (Status)) {
      SmmVariableAvailableNotify (
                  &gEfiSmmVariableProtocolGuid,
                  (VOID *)SmmVariable,
                  (EFI_HANDLE)0
                  );
    } else {
      Status = mSmst->SmmRegisterProtocolNotify (
                        &gEfiSmmVariableProtocolGuid,
                        SmmVariableAvailableNotify,
                        &Registration
                        );
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
    
    //
    // Start of relocated code (to avoid uninitialized variable access
    // in the set and get variable functions in callbacks triggered by
    // the installation of the protocol)
    //
    Status = SmmNvsInitialize (SmmBase);
    ASSERT_EFI_ERROR(Status);

    PlatformHookAssignPlacement (
                VariableHookTable,
                NULL
                );
    //
    // Locate SMM Heci protocol
    //                    
    Status = mSmst->SmmLocateProtocol (
                      &gEfiHeciSmmProtocolGuid,
                      NULL,
                      (VOID **)&mHeci2Protocol
                      );
    ASSERT_EFI_ERROR(Status);
    
  }

  return EFI_SUCCESS;
}
