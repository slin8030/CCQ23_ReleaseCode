/** @file
 SMM Chipset Services driver. 
 
 It produces an instance of the SMM Chipset Services protocol to provide the chipset related functions 
 which will be used by Kernel or Project code. These protocol functions should be used by calling the 
 corresponding functions in SmmChipsetSvcLib to have the protocol size checking

***************************************************************************
* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/
#include <PiSmm.h>
#include <CsSvcIhisiFbts.h>
#include <CsSvcIhisiFets.h>
#include <CsSvcIhisiOemExtraDataCommunication.h>

//
// Libraries
//
#include <Library/SmmServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

//
// Consumer Protocols
//
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/ComponentName.h>

//
// Produced Protocols
//
#include <Protocol/H2OSmmChipsetServices.h>
//#include <Protocol/SmmFwBlockService.h>
//#include <Protocol/GlobalNvsArea.h>
#include <Protocol/H2OIhisi.h>

//[-start-161026-IB07400805-add]//
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GlobalNvsArea.h>
//[-end-161026-IB07400805-add]//

//
// Global variable
//
extern EFI_COMPONENT_NAME2_PROTOCOL  gChipsetSvcSmmComponentName2;
extern EFI_COMPONENT_NAME_PROTOCOL   gChipsetSvcSmmComponentName;

//
// Driver Support EFI Version Protocol instance
//
GLOBAL_REMOVE_IF_UNREFERENCED 
EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL gSmmChipsetSvcDriverSupportedEfiVersion = {
  sizeof (EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL),
  0x0002001E
};

H2O_SMM_CHIPSET_SERVICES_PROTOCOL *mSmmChipsetSvc = NULL;
H2O_IHISI_PROTOCOL                    *mH2OIhisi = NULL;
EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL     *mSmmFwBlockService = NULL;
//[-start-161026-IB07400805-add]//
EFI_GLOBAL_NVS_AREA                   *mGlobalNvsArea = NULL;
//[-end-161026-IB07400805-add]//

#define ASSIGN_FUNCTION_ENTRY(ProtocolMember, FunctionEntry)                 \
          if (mSmmChipsetSvc->Size >= (OFFSET_OF (H2O_SMM_CHIPSET_SERVICES_PROTOCOL, ProtocolMember) + sizeof (VOID*))) { \
            mSmmChipsetSvc->ProtocolMember = FunctionEntry; \
          }

EFI_STATUS
EFIAPI
ResetSystem (
  IN EFI_RESET_TYPE   ResetType
  );

//[-start-161026-IB07400805-add]//
STATIC
EFI_STATUS
EFIAPI
GlobalNvsNotification (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  EFI_STATUS                        Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL      *GlobalNvsAreaPtr = NULL;
    
  Status = gSmst->SmmLocateProtocol (
             &gEfiGlobalNvsAreaProtocolGuid,
             NULL,
             (VOID **)&GlobalNvsAreaPtr
             );
  mGlobalNvsArea = GlobalNvsAreaPtr->Area;
  DEBUG ((EFI_D_ERROR, "GlobalNvsNotification: GlobalNvsAreaPtr = %x\n", GlobalNvsAreaPtr));
  DEBUG ((EFI_D_ERROR, "GlobalNvsNotification: mGlobalNvsArea = %x\n", mGlobalNvsArea));
  ASSERT_EFI_ERROR (Status);
  return Status;
}
//[-end-161026-IB07400805-add]//

/**
  H2O IHISI Protocol notification event handler.

  @param[in]  Protocol      Points to the protocol's unique identifier
  @param[in]  Interface     Points to the interface instance
  @param[in]  Handle        The handle on which the interface was installed

  @retval EFI_SUCCESS       SmmEventCallback runs successfully

 **/
STATIC
EFI_STATUS
EFIAPI
H2OIHISINotification (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  EFI_STATUS       Status;
  Status = gSmst->SmmLocateProtocol (
             &gH2OIhisiProtocolGuid,
             NULL,
             (VOID **)&mH2OIhisi
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  The notification of gEfiSmmFwBlockServiceProtocolGuid protocol is installed

  @param[in] Protocol      Points to the protocol's unique identifier.
  @param[in] Interface     Points to the interface instance.
  @param[in] Handle        The handle on which the interface was installed.

  @retval EFI_SUCCESS      Locate gEfiSmmVariableProtocolGuid protocol successful.
  @retval EFI_NOT_FOUND    Cannot find gEfiSmmVariableProtocolGuid instance.
**/
STATIC
EFI_STATUS
SmmFwBlockNotify (
  IN     CONST EFI_GUID                *Protocol,
  IN     VOID                          *Interface,
  IN     EFI_HANDLE                    Handle
  )
{
  EFI_STATUS            Status;

  Status = gSmst->SmmLocateProtocol (
                  &gEfiSmmFwBlockServiceProtocolGuid,
                  NULL,
                  &mSmmFwBlockService
                  );

  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }
  return Status;
}

/**
 This is the declaration of SMM Chipset Services driver entry point. 

 @param[in]         ImageHandle         The firmware allocated handle for the UEFI image.
 @param[in]         SystemTable         A pointer to the EFI System Table.

 @retval            EFI_SUCCESS         The operation completed successfully.
 @retval            Others              An unexpected error occurred.
*/
EFI_STATUS
EFIAPI
ChipsetSvcSmmEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT32      Size;
  EFI_HANDLE  Handle;
  VOID                               *Registration;
//[-start-161026-IB07400805-add]//
  EFI_GLOBAL_NVS_AREA_PROTOCOL       *GlobalNvsAreaPtr = NULL;
//[-end-161026-IB07400805-add]//

  Status = EFI_SUCCESS;
  Handle = NULL;
  
  //
  // Create an instance of the H2O SMM Chipset Services protocol.
  // Then install it on the image handle.
  //
  Status = gSmst->SmmLocateProtocol (
             &gH2OIhisiProtocolGuid,
             NULL,
             (VOID **)&mH2OIhisi
             );
  if (EFI_ERROR (Status)) {
    //
    // If H2O Chipset Services is not installed, register Protocol notification for H2O Chipset Services
    //
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gH2OIhisiProtocolGuid,
                      H2OIHISINotification,
                      &Registration
                      );
    ASSERT_EFI_ERROR (Status);
  }
  
//[-start-161026-IB07400805-add]//
  //
  //  Locate the Global NVS Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsAreaPtr
                  );
  if (EFI_ERROR (Status)) {
    //
    // If gEfiGlobalNvsAreaProtocolGuid is not installed, register Protocol notification
    //
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gEfiGlobalNvsAreaProtocolGuid,
                      GlobalNvsNotification,
                      &Registration
                      );
    DEBUG ((EFI_D_ERROR, "ChipsetSvcSmmEntryPoint:SmmRegisterProtocolNotify gEfiGlobalNvsAreaProtocolGuid = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
  } else {
    mGlobalNvsArea = GlobalNvsAreaPtr->Area;
    DEBUG ((EFI_D_ERROR, "ChipsetSvcSmmEntryPoint:mGlobalNvsArea = %x\n", mGlobalNvsArea));
  }
//[-end-161026-IB07400805-add]//

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmFwBlockServiceProtocolGuid,
                    NULL,
                    &mSmmFwBlockService
                    );

  if (EFI_ERROR (Status)) {
    Status = gSmst->SmmRegisterProtocolNotify (
                      &gEfiSmmFwBlockServiceProtocolGuid,
                      SmmFwBlockNotify,
                      &mSmmFwBlockService
                      );
    ASSERT_EFI_ERROR (Status);
  }

  
  Size = sizeof (H2O_SMM_CHIPSET_SERVICES_PROTOCOL);
  if (Size < sizeof (UINT32)) {         // must at least contain Size field.
    return EFI_INVALID_PARAMETER;
  }
  
  mSmmChipsetSvc = AllocateZeroPool (Size);
  if (mSmmChipsetSvc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  mSmmChipsetSvc->Size = Size;

  //
  // Now, populate functions based on Size.
  //
//  ASSIGN_FUNCTION_ENTRY (EnableFdWrites,                EnableFdWrites);
//  ASSIGN_FUNCTION_ENTRY (LegacyRegionAccessCtrl,        LegacyRegionAccessCtrl);
  ASSIGN_FUNCTION_ENTRY (ResetSystem,                   ResetSystem);
//  ASSIGN_FUNCTION_ENTRY (SataComReset,                  SataComReset);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsGetPermission,        IhisiFbtsGetPermission);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsGetOemFlashMap,       IhisiFbtsGetOemFlashMap);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsDoBeforeWriteProcess, IhisiFbtsDoBeforeWriteProcess);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsDoAfterWriteProcess,  IhisiFbtsDoAfterWriteProcess);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsDoBeforeReadProcess,  IhisiFbtsDoBeforeReadProcess);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsDoAfterReadProcess,   IhisiFbtsDoAfterReadProcess);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsApTerminated,         IhisiFbtsApTerminated);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsNormalFlash,          IhisiFbtsNormalFlash);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsPartialFlash,         IhisiFbtsPartialFlash);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsOemComplete,          IhisiFbtsOemComplete);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsShutdown,             IhisiFbtsShutDown);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsReboot,               IhisiFbtsReboot);
  ASSIGN_FUNCTION_ENTRY (IhisiFbtsApRequestDoNothing,   IhisiFbtsApRequestDoNothing);
  ASSIGN_FUNCTION_ENTRY (IhisiFetsReboot,               IhisiFetsReboot);
  ASSIGN_FUNCTION_ENTRY (IhisiFetsShutdown,             IhisiFetsShutdown);
  ASSIGN_FUNCTION_ENTRY (IhisiOemExtCommunication,      IhisiOemExtCommunication);
  ASSIGN_FUNCTION_ENTRY (IhisiOemExtDataWrite,          IhisiOemExtDataWrite);
  ASSIGN_FUNCTION_ENTRY (IhisiOemExtDataRead,           IhisiOemExtDataRead);
  
  Status = gSmst->SmmInstallProtocolInterface (
                    &Handle,
                    &gH2OSmmChipsetServicesProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    mSmmChipsetSvc
                    );
  
  return Status;
}
