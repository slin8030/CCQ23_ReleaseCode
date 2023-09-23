/*
 * (C) Copyright 2016 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2016 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal��s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "OemConfig.h"
#include "ModifyPlatformSetting.h"
#include "ProjectDefinition.h"
#include <SetupConfig.h>
#include <CompalEcLib.h>
#include <Library/SideBandLib.h>
#include <Library/ScPlatformLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/Variable.h>

typedef enum {
  USB_PORT0 = 1,
  USB_PORT1,
  USB_PORT2,
  USB_PORT3,
  USB_PORT4,
  USB_PORT5,
  USB_PORT6,
  USB_PORT7
} USB_PORT_INDEX;

typedef struct {
  UINT16 Address;
  UINT32 Data;
} PROJECT_SIDEBAND_TABLE_STRUCT;

#define USB2_PER_PORT_PPX(USB_PORTx)    R_PCR_USB2_PHY_LANE_BASE_ADDR|R_PCR_USB2_PPORT |(USB_PORTx << 8)
#define USB2_PER_PORT2_PPX(USB_PORTx)   R_PCR_USB2_PHY_LANE_BASE_ADDR|R_PCR_USB2_PPORT2|(USB_PORTx << 8)

SYSTEM_CONFIGURATION  *mSystemConfiguration;

STATIC
EFI_STATUS
ConfigEC_NameSpace (
  );

/**
  Modify USB config.

  @retval EFI_SUCCESS             The function is executed successfully.
  @retval EFI_INVALID_PARAMETER   mPchPlatformPolicy point to NULL.

**/
STATIC
EFI_STATUS
ModifyUSBConfig (
  )
{
//[PRJ]+ >>>> Modify USB2 eye diagram 
  UINT8             Index;

  PROJECT_SIDEBAND_TABLE_STRUCT ProjectUsb2PhyPerPortTable[] = {
    {USB2_PER_PORT_PPX(USB_PORT0), 0x00059781},
    {USB2_PER_PORT_PPX(USB_PORT1), 0x0005AE81},
    {USB2_PER_PORT_PPX(USB_PORT2), 0x0005AE81},
    {USB2_PER_PORT_PPX(USB_PORT3), 0x00059781},
    {USB2_PER_PORT_PPX(USB_PORT4), 0x00059781},
    {USB2_PER_PORT_PPX(USB_PORT5), 0x00059781},
    {USB2_PER_PORT_PPX(USB_PORT6), 0x00059781},
    {USB2_PER_PORT_PPX(USB_PORT7), 0x00059781},

    {USB2_PER_PORT2_PPX(USB_PORT0), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT1), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT2), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT3), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT4), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT5), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT6), 0x00829248},
    {USB2_PER_PORT2_PPX(USB_PORT7), 0x00829248},
  };

  for (Index = 0; Index < sizeof(ProjectUsb2PhyPerPortTable)/sizeof(ProjectUsb2PhyPerPortTable[0]); Index++) {
    SideBandWrite32((UINT8)(USB2_PHY_PORT_ID & 0xff),
                     ProjectUsb2PhyPerPortTable[Index].Address,
                     ProjectUsb2PhyPerPortTable[Index].Data
                   );
  }
//[PRJ]+ <<<< Modify USB2 eye diagram   
  
  return EFI_SUCCESS;
}

/**
  Getvariable ""Setup" and set to mSystemConfiguration ptr.

@retval EFI_SUCCESS     mSystemConfiguration correct.
@retval others          get Setup variable fail.
**/
EFI_STATUS
GetSystemConfig (
  )
{
  EFI_STATUS              Status;
  UINTN                   VarSize;

  if (mSystemConfiguration != NULL) {
    Status = EFI_SUCCESS;
    goto Exit;
  }

  VarSize = 0;
  Status = gRT->GetVariable(
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &VarSize,
                  NULL
                  );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    if (sizeof (SYSTEM_CONFIGURATION) != VarSize) {
      ASSERT (sizeof (SYSTEM_CONFIGURATION) == VarSize);
      Status = EFI_UNSUPPORTED;
      goto Exit;
    }

    mSystemConfiguration = AllocateZeroPool(VarSize);
    if (mSystemConfiguration == NULL) {
      ASSERT (mSystemConfiguration);
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }

    Status = gRT->GetVariable(
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    NULL,
                    &VarSize,
                    mSystemConfiguration
                    );
    if (EFI_ERROR (Status)) {
      FreePool (mSystemConfiguration);
      mSystemConfiguration = NULL;
    }
  }
  if (mSystemConfiguration == NULL) {
    DEBUG((DEBUG_ERROR, "mSystemConfiguration doesn't ready\n"));
  }
Exit:
  return Status;
}

/**
 Notify function to locate gEfiVariableArchProtocolGuid.

 @param[in] Event     The Event this notify function registered to.
 @param[in] Context   Pointer to the context data registered to the Event.
**/
STATIC
VOID
VariableArchProtocolNotify (
    IN EFI_EVENT          Event,
    IN VOID              *Context
  )
{
  EFI_STATUS              Status;
  VOID                   *Interface;

  gBS->CloseEvent (Event);
  Status = gBS->LocateProtocol (
                  &gEfiVariableArchProtocolGuid,
                  NULL,
                  &Interface
                  );

  if (EFI_ERROR (Status)) {
    return;
  }

  Status = GetSystemConfig ();

  Status = ConfigEC_NameSpace();
  return;
}

/**
  Update EC name space setting

  @retval EFI_SUCCESS             The function is executed successfully.
  @retval EFI_NOT_READY           Setup variable is null.

**/
STATIC
EFI_STATUS
ConfigEC_NameSpace (
  )
{
  ECNAME_F4A4_STRU       EC_Name_F4A4;
  EC_Name_F4A4.Data=CompalECMemoryRead (ECNAME_F4A4);
  EC_Name_F4A4.Bits.USB_Wake = 0;
  EC_Name_F4A4.Bits.LAN_Wake = 0;
  EC_Name_F4A4.Bits.WLAN_Wake = 0;
//[S4S5RTCWake]LouisYT[-start-200701-001-add]//
  EC_Name_F4A4.Bits.RTC_Wake = 0;
  
  if (mSystemConfiguration->WakeOnS5) {
    EC_Name_F4A4.Bits.RTC_Wake = 1;
  }
//[S4S5RTCWake]LouisYT[-end-200701-001-add]//
  
  if (mSystemConfiguration->WakeOnPME) {
    EC_Name_F4A4.Bits.LAN_Wake = 1;
//[PRJ]+ >>>>Fix system will auto power on when first time boot to os and let system shutdown    EC_Name_F4A4.Bits.WLAN_Wake = 1;
  }
  
  if (mSystemConfiguration->S3WakeOnUsb) {
    EC_Name_F4A4.Bits.USB_Wake = 1;
  }
  CompalECMemoryWrite(ECNAME_F4A4, EC_Name_F4A4.Data);
  CompalECWriteCmdTwoByte(EC_MEMORY_MAPPING_PORT, 0x59, 0xFA, mSystemConfiguration->SlideShutDown);
  return EFI_SUCCESS;
}

/**
  Modify platform setting

  @retval EFI_SUCCESS            The function is executed successfully.
**/
EFI_STATUS
EFIAPI
ModifyPlatformSetting (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  EFI_EVENT               Event;
  VOID                   *Registration;
  VOID                   *Interface;

  mSystemConfiguration = NULL;
  Status = gBS->LocateProtocol (&gEfiVariableArchProtocolGuid, NULL, &Interface);
  if (EFI_ERROR (Status)) {
    //
    // Read/Write variable doesn't ready , notify protocol
    //
    Status = OemCreateProtocolNotifyEvent(
              &gEfiVariableArchProtocolGuid,
              TPL_NOTIFY,
              VariableArchProtocolNotify,
              NULL,
              &Registration,
              &Event
              );
  } else {
    Status = GetSystemConfig ();
  }

  //
  // Modify USB2 Tuning register.
  //
  Status = ModifyUSBConfig();
  Status = ConfigEC_NameSpace();

  FreePool(mSystemConfiguration);
  return EFI_SUCCESS;
}


