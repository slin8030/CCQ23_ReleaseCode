/** @file

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


#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include <PiSmm.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/Variable.h>
#include <Protocol/SmmRuntime.h>
#include <Protocol/SmmVariable.h>

#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ImageRelocationLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/PlatformConfigDataLib.h>
#include <Library/PostCodeLib.h>

#include <Guid/EventGroup.h>
#include <Guid/TreePhysicalPresenceData.h>
#include <Guid/Tcg2PhysicalPresenceData.h>
#include <Guid/MemoryConfigData.h>
#include <Guid/MemoryOverwriteControl.h>

#include <ChipsetSetupConfig.h>
#include <PostCode.h>

typedef struct {
  BOOLEAN                            AtRuntime;
  EFI_PHYSICAL_ADDRESS               PlatformHookVariableStore;
} ESAL_VARIABLE_GLOBAL;

//
// Definitions of Platform variable hook table
//
typedef struct {
  EFI_GUID          *VendorGuid;
  CHAR16            *VariableName;
  CHAR8             *NvmName;
  UINTN             Offset;
  UINTN             Size;
  UINTN             MemoryBufferOffset;
}PLATFORM_VARIABLE_HOOK_TABLE;


#define  PLATFORM_TCG_DATA_FILE_NAME    "NVS/TCGDATA"
#define  PLATFORM_TREE_DATA_FILE_NAME   "NVS/TREEDATA"

#define OFFSET_PHYSICAL_PRESENCE              0x00
#define OFFSET_PHYSICAL_PRESENCE_FLAG         0x10
#define OFFSET_MEMORY_OVERWRITE_CONTROL_DATA  0x20

/**
  Initializes variable store area for non-volatile and volatile variable.

  @retval EFI_SUCCESS           Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES  Fail to allocate enough memory resource.

**/
EFI_STATUS
VariableCommonInitialize (
  VOID
  );

/**
  Initialize SMM mode NV data

  @param  SmmBase        Pointer to EFI_SMM_BASE_PROTOCOL

  @retval EFI_SUCCESS    Initialize SMM mode NV data successful.
  @retval Other          Any error occured during initialize SMM NV data.

**/
EFI_STATUS
EFIAPI
SmmNvsInitialize (
  IN  EFI_SMM_BASE2_PROTOCOL               *SmmBase
  );


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
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );


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
  );



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
  );


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
  );

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
  );

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
);

extern ESAL_VARIABLE_GLOBAL         *mVariableModuleGlobal;
extern EFI_SMM_SYSTEM_TABLE2        *mSmst;

#endif
