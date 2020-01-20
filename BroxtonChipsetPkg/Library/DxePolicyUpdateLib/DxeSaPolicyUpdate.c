/** @file
This file is the library for SA DXE Policy initialzation.

@copyright
Copyright (c) 2004 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by the
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor. This file may be modified by the user, subject to
the additional terms of the license agreement.

@par Specification Reference:
**/

#include <DxeSaPolicyUpdate.h>

//[-start-160701-IB07400750-add]//
EFI_STATUS 
UpdateVbtInfoHob (
  IN PLATFORM_GOP_POLICY_PROTOCOL  *GopPolicy
  )
{
  EFI_STATUS                    Status;
  EFI_PHYSICAL_ADDRESS          VbtAddress;
  UINT32                        Size;
  EFI_PEI_HOB_POINTERS          GuidHob;
  VBT_INFO                      *VbtInfo = NULL;
  
  ///
  /// Get VBT data
  ///
  VbtAddress      = 0;
  Size            = 0;
  DEBUG ((DEBUG_INFO, "GetVbtData\n"));
  Status = GopPolicy->GetVbtData (&VbtAddress, &Size);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GopPolicy->GetVbtData %r\n", Status));
    return Status;
  }

  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gVbtInfoGuid, GuidHob.Raw)) != NULL) {
      VbtInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
      VbtInfo->VbtAddress = VbtAddress;
      VbtInfo->VbtSize = Size;
      Status = EFI_SUCCESS;
    }
  } else {
    Status = EFI_NOT_FOUND;
    DEBUG ((EFI_D_ERROR, "Get gVbtInfoGuid Hob %r\n", Status));
  }
  
  return Status;
}

VOID
EFIAPI
PlatformGOPPolicyCallback (
  IN EFI_EVENT    Event,
  IN VOID*        Context
  )
{
  EFI_STATUS                    Status;
  PLATFORM_GOP_POLICY_PROTOCOL  *GopPolicy;

  GopPolicy = NULL;
  Status = gBS->LocateProtocol (
                  &gPlatformGOPPolicyGuid,
                  NULL,
                  &GopPolicy
                  );
  if (!EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
    Status = UpdateVbtInfoHob (GopPolicy);
  }
}
//[-end-160701-IB07400750-add]//

/**
Get data for platform policy from setup options.

@param[in] DxeSaPolicy              The pointer to get SA Policy protocol instance
@param[in] SystemConfiguration		The pointer to get System Setup

@retval EFI_SUCCESS                  Operation success.

**/
EFI_STATUS
EFIAPI
UpdateDxeSaPolicy(
  IN OUT SA_POLICY_PROTOCOL *SaPolicy,
    IN CHIPSET_CONFIGURATION* SystemConfiguration
)
{
  PLATFORM_GOP_POLICY_PROTOCOL *GopPolicy;
//[-start-160701-IB07400750-remove]//
//  VBT_INFO                     *VbtInfo = NULL;
//  EFI_PHYSICAL_ADDRESS         VbtAddress;
//  UINT32                       Size;
//  EFI_PEI_HOB_POINTERS         GuidHob;
//[-end-160701-IB07400750-remove]//
  EFI_STATUS                   Status;
  SA_DXE_MISC_CONFIG           *SaDxeMiscConfig = NULL;
  IGD_PANEL_CONFIG             *IgdPanelConfig = NULL;

//[-start-160429-IB03090426-add]//
  DEBUG ((DEBUG_INFO, "UpdateDxeSaPolicy\n"));
//[-end-160429-IB03090426-add]//

  Status = GetConfigBlock((CONFIG_BLOCK_TABLE_HEADER*)SaPolicy, &gIgdPanelConfigGuid, (VOID *)&IgdPanelConfig);
  ASSERT_EFI_ERROR(Status);

  Status = GetConfigBlock((CONFIG_BLOCK_TABLE_HEADER*)SaPolicy, &gSaDxeMiscConfigGuid, (VOID *)&SaDxeMiscConfig);
  ASSERT_EFI_ERROR(Status);

  DEBUG ((DEBUG_INFO, "Locate GopPolicy and GetVbtData\n"));

//[-start-160701-IB07400750-modify]//
  ///
  /// Locate the GOP Policy Protocol.
  ///
  GopPolicy = NULL;
  Status = gBS->LocateProtocol (
                  &gPlatformGOPPolicyGuid,
                  NULL,
                  &GopPolicy
                  );

  if (EFI_ERROR (Status) || (GopPolicy == NULL)) {
    Status = EfiNamedEventListen (
      &gPlatformGOPPolicyGuid,
      TPL_NOTIFY,
      PlatformGOPPolicyCallback,
      NULL,
      NULL
      );
  } else {
    Status = UpdateVbtInfoHob (GopPolicy);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
//[-end-160701-IB07400750-modify]//

  IgdPanelConfig->PFITStatus     = SystemConfiguration->PanelScaling;
//[-start-151228-IB03090424-modify]//
  SaDxeMiscConfig->S0ixSupported = SystemConfiguration->LowPowerS0Idle;
//[-end-151228-IB03090424-modify]//

#if defined (TABLET_PF_ENABLE) && (TABLET_PF_ENABLE == 1)
  if (SystemConfiguration->ScHdAudio == 0) {
    SaDxeMiscConfig->AudioTypeSupport = NO_AUDIO;
  }
  else {
    SaDxeMiscConfig->AudioTypeSupport = HD_AUDIO;
  }
//[-start-160216-IB03090424-add]//
#else
  IgdPanelConfig->PanelSelect    = SystemConfiguration->VbtSelect;
//[-start-181217-IB07401056-add]//
#ifdef MIPI_PANEL_EXAMPLE_SUPPORT
  IgdPanelConfig->PanelSelect    = 1; // MIPI Panel to using PANEL1_VDDEN, PANEL1_BKLTEN as GPIO
#endif
//[-end-181217-IB07401056-add]//
//[-end-160216-IB03090424-add]//
#endif

  return EFI_SUCCESS;
}

