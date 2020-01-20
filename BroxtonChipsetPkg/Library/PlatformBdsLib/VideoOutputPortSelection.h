/** @file
  Add descriptiion

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

#ifndef _VIDEO_OUTPUT_PORT_SELECTION_H_
#define _VIDEO_OUTPUT_PORT_SELECTION_H_


#include "BdsPlatform.h"

//
// the VOLATILE variable "ActiveDisplayInfo" stores all of the active video controller info to 
// provide the using of SCU. 
//
#define PLUG_IN_VIDEO_DISPLAY_INFO_VAR_NAME  L"PlugInVgaHandles"
//
// For the Igd device.
// Following is the project-specific definition. 
// 
//=======================Start============================//
//
// ADR definition which collect from the GOP device path which by passing RemainingDevicePath == NULL
// with the connected display device. 
//
#define IGD_NULL_ADR    0x00000000

#define IGD_CRT_ADR     0x80010160  
#define IGD_LVDS1_ADR   0x80010440  
#define IGD_HDMI1_ADR   IGD_NULL_ADR   // HDMI-B
#define IGD_HDMI3_ADR   IGD_NULL_ADR   // HDMI-D
#define IGD_HDMI2_ADR   0x80010321     // HDMI-C
#define IGD_DP1_ADR     IGD_NULL_ADR     // DisplayPort-B
#define IGD_DP2_ADR     0x80030180   // DisplayPort-C
#define IGD_DP3_ADR     0x80010392     // DisplayPort-D

//========================End==============================//

EFI_STATUS 
PlatformBdsDisplayPortSelection (
  IN EFI_HANDLE                                VgaControllerHandle,
  IN OUT EFI_DEVICE_PATH_PROTOCOL              **RemainingDevicePath
  );

EFI_STATUS
PlatformBdsVgaConnectedFailCallback (  
  IN  EFI_HANDLE               VgaControllerHandle, 
  IN  EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  );

BOOLEAN
IsIGDController (
  IN EFI_HANDLE  VgaControllerHandle
  );

EFI_DEVICE_PATH_PROTOCOL *
SetAcpiAdr (
  IN UINT32  ADR1, 
  IN UINT32  ADR2
  );

#endif 
