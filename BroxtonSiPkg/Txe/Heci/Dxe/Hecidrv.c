/** @file
  HECI driver

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2007 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#include <Guid/EventGroup.h>
//[-start-160907-IB07400781-add]//
#include <Guid/EventLegacyBios.h>
//[-end-160907-IB07400781-add]//
#include "Hecidrv.h"
#include <Private/Library/HeciInitLib.h>
#include <ScRegs/RegsPcu.h>
#include <Library/S3BootScriptLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/SideBandLib.h>
//[-start-160904-IB07400778-add]//
#include <Library/HobLib.h>
//[-end-160904-IB07400778-add]//

extern DXE_SEC_POLICY_PROTOCOL *mDxePlatformSeCPolicy;
//[-start-160810-IB03090430-add]//
extern EFI_GUID                gEfiBootMediaHobGuid;
extern EFI_GUID                gFdoModeEnabledHobGuid;
//[-end-160810-IB03090430-add]//

EFI_HECI_PROTOCOL *mHeci2Protocol = NULL;

HECI_INSTANCE     *mHeciContext;
EFI_HANDLE        mHeciDrv;

#define S_SEC_DevID_RANGE_LO_Test 0xA9A
#define S_SEC_DevID_RANGE_HI_Test 0xA9E

VOID
EFIAPI
FlashDeviceEndOfServicesEvent (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS     HobPtr;
  MBP_CURRENT_BOOT_MEDIA   *BootMediaData = NULL;

  DEBUG ((EFI_D_INFO, "Begin - End of Services HECI Event\n"));

  ASSERT_EFI_ERROR (mHeci2Protocol != NULL);

  HobPtr.Guid = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  ASSERT (HobPtr.Guid != NULL);

  BootMediaData = (MBP_CURRENT_BOOT_MEDIA*) GET_GUID_HOB_DATA (HobPtr.Guid);

  //
  // The Send Proxy State Message should not be sent on SPI boot
  //
  if (BootMediaData->PhysicalData != BOOT_FROM_SPI) {
    Heci2GetProxyStateNoResp (mHeci2Protocol);
  }

  DEBUG ((EFI_D_INFO, "End - End of Services HECI Event\n"));
}

VOID
EFIAPI
FlashDeviceEndOfPostEvent (
  VOID
  )
{
  EFI_STATUS  Status;

  DEBUG((EFI_D_INFO, "Begin - End of Post HECI Event\n"));

  if (mHeci2Protocol == NULL) {
    DEBUG ((EFI_D_INFO, "HECI Protocol %x\n", mHeci2Protocol));

    Status = gBS->LocateProtocol (
                    &gEfiHeciSmmRuntimeProtocolGuid,
                    NULL,
                    &mHeci2Protocol
                    );
    if (GetFirstGuidHob (&gFdoModeEnabledHobGuid) == NULL) {
      ASSERT_EFI_ERROR (Status);
    }
  }

  DEBUG ((EFI_D_INFO, "End - End of Post HECI Event\n"));
}

/*
  Notify the system of the End of Post message.
*/
VOID NotifyEOP (
  VOID
  )
{
  EFI_HANDLE    Handle;
  EFI_STATUS    Status;

  Handle = NULL;

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiCseEndofPostGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );

  FlashDeviceEndOfPostEvent ();
}

/*
  Notify the system send End of Services message.

*/
VOID NotifyEOS (
  VOID
  )
{
  EFI_HANDLE    Handle;
  EFI_STATUS    Status;

  Handle = NULL;

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiCseEndofServicesGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );

  FlashDeviceEndOfServicesEvent ();
}


/**
  CF9GR Lock Config.

  @param[in] VOID
**/
VOID
LockConfig (
  VOID
  )
{
  UINT32                          PmcBase;
  UINT32                          SecMode;
  HECI_FWS_REGISTER               SecFirmwareStatus;
  UINT32                          Data;

  DEBUG ((DEBUG_INFO, "LockConfig () - Start\n"));

  PmcBase        = PMC_BASE_ADDRESS;

  HeciGetSeCStatus (&SecMode);
  SecFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);
  DEBUG ((DEBUG_INFO, "SecFirmwareStatus.ul: 0x%X\n", SecFirmwareStatus.ul));

  ///
  /// APL/BXT IAFW Spec: Additional Power Management Programming
  /// Step 2
  /// Lock the "CF9h Global reset" field by setting CF9LOCK bit (PBASE + 0x48[31]) for production build,
  /// but this field should not be locked for manufacturing mode. When the Manufacturing Mode is closed,
  /// CF9h Global Reset should be cleared (step#1) and CF9LOCK bit should be set (step#2).
  ///
  Data = 0;
  if (((SecMode == SEC_MODE_NORMAL) || (SecMode == SEC_MODE_TEMP_DISABLED)) && !(SecFirmwareStatus.r.ManufacturingMode)) {
    Data |= B_PMC_ETR_CF9LOCK;
  }

  MmioAndThenOr32 (
    (UINTN) (PmcBase + R_PMC_ETR),
    (UINT32) (~(B_PMC_ETR_CF9LOCK | B_PMC_ETR_CF9GR)),
    (UINT32) Data
    );

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PmcBase + R_PMC_ETR),
    1,
    (VOID *) (UINTN) (PmcBase + R_PMC_ETR)
    );
}

/*
  Event handle for Exit boot services

  @param[in] Event
  @param[in] ParentImageHandle
*/
VOID
EFIAPI
SeCExitBootServicesEvent (
  IN EFI_EVENT           Event,
  IN VOID                *ParentImageHandle
  )
{
  DEBUG ((EFI_D_INFO, "SeCExitBootServicesEvent ++\n"));


  DEBUG((EFI_D_INFO, "Start Send EOS Heci Message\n"));
  HeciEndOfServices();
  DEBUG((EFI_D_INFO, "End Of Send EOS Heci Message\n"));
  NotifyEOS();
  gBS->CloseEvent (Event);

  DEBUG ((EFI_D_INFO, "SeCExitBootServicesEvent --\n"));
}

//[-start-170331-IB07400855-add]//
VOID
EFIAPI
HeciEndOfDXECallback (
  IN EFI_EVENT     Event,
  IN VOID          *ParentImageHandle
  )
{
  EFI_STATUS Status;
  BOOLEAN    SendEOP;
  
  SendEOP = TRUE;
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (!EFI_ERROR(Status)) {
    SendEOP = (mDxePlatformSeCPolicy->SeCConfig.EndOfPostEnabled == 1);
  }

  if (SendEOP) {
    HeciDXECallback (Event, ParentImageHandle);
  }
}
//[-end-170331-IB07400855-add]//

/*
  Event handle for ready to boot

  @param[in] Event
  @param[in] ParentImageHandle
*/
VOID
EFIAPI
SeCReadyToBootEvent (
  IN EFI_EVENT           Event,
  IN VOID                *ParentImageHandle
  )
{
  EFI_STATUS Status;
  BOOLEAN    SendEOP;
  DEBUG ((EFI_D_INFO, "SeCReadyToBootEvent ++\n"));
  SendEOP = TRUE;
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (!EFI_ERROR(Status)) {
    SendEOP = (mDxePlatformSeCPolicy->SeCConfig.EndOfPostEnabled == 1);
  }
  if (SendEOP) {
    HeciEndOfPost();
  }
  NotifyEOP();
  LockConfig();

  gBS->CloseEvent (Event);

  DEBUG ((EFI_D_INFO, "SeCReadyToBootEvent --\n"));
}


/**
  Entry point for HECI Driver

  @param[in]    ImageHandle - Standard entry point parameter.
  @param[in]    SystemTable - Standard entry point parameter.

  @retval  EFI_SUCCESS       Init success.
  @retval  Others            Error occurs during HECI driver init.
**/
EFI_STATUS
EFIAPI
InitializeHECI (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   ReadyToBootEvent = NULL;
  EFI_EVENT   ExitBootServicesEvent = NULL;
  EFI_EVENT   EndOfDxeEvent=NULL;
//[-start-160904-IB07400778-add]//
  EFI_BOOT_MODE  BootMode;
//[-end-160904-IB07400778-add]//

  DEBUG ((EFI_D_INFO, "InitializeHECI Driver Entry Point\n"));

//[-start-160904-IB07400778-add]//
  BootMode = GetBootModeHob ();
  //
  // In BOOT_IN_RECOVERY_MODE, TXE is not exist.
  //
  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return EFI_SUCCESS;
  }
//[-end-160904-IB07400778-add]//


  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             SeCReadyToBootEvent,
             (VOID *) &ImageHandle,
             &ReadyToBootEvent
             );
  ASSERT_EFI_ERROR (Status);
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SeCExitBootServicesEvent,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);
//[-start-160907-IB07400781-add]//
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SeCExitBootServicesEvent,
                  NULL,
                  &gEfiEventLegacyBootGuid,
                  &ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);
//[-end-160907-IB07400781-add]//
  DEBUG ((EFI_D_INFO, "Registering the End of DXE done Call back in HECI  ++\n"));
//[-start-160808-IB07220123-modify]//
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
//[-start-170331-IB07400855-modify]//
                  HeciEndOfDXECallback,
//[-end-170331-IB07400855-modify]//
                  NULL,
//                   &gEfiEndOfDxeEventGroupGuid,
                  &gEfiEventReadyToBootGuid,
                  &EndOfDxeEvent
                  );
//[-end-160808-IB07220123-modify]//
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "InitializeHECI ++\n"));

  mHeciDrv	  = ImageHandle;
  mHeciContext  = AllocateZeroPool (sizeof (HECI_INSTANCE));
  //
  // Initialize HECI protocol pointers
  //
  if (mHeciContext != NULL) {
    mHeciContext->HeciCtlr.ResetHeci	  = ResetHeciInterface;
    mHeciContext->HeciCtlr.SendwACK	  = HeciSendwACK;
    mHeciContext->HeciCtlr.ReadMsg	  = HeciReceive;
    mHeciContext->HeciCtlr.SendMsg	  = HeciSend;
    mHeciContext->HeciCtlr.InitHeci	  = HeciInitialize;
    mHeciContext->HeciCtlr.ReInitHeci   = HeciReInitialize;
    mHeciContext->HeciCtlr.SeCResetWait  = SeCResetWait;
    mHeciContext->HeciCtlr.GetSeCStatus  = HeciGetSeCStatus;
    mHeciContext->HeciCtlr.GetSeCMode    = HeciGetSeCMode;
  }


  //
  // Install the HECI interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
				&mHeciContext->Handle,
				&gEfiHeciProtocolGuid,
				&mHeciContext->HeciCtlr,
				NULL
				);

  DEBUG ((EFI_D_INFO, "InitializeHECI --\n"));


  //
  // Disable the HECI3
  //

  SeCDeviceControl (HECI3_DEVICE,Disabled);
  return EFI_SUCCESS;

}
