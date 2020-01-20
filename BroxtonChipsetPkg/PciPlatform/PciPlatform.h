/*++

Copyright (c)  2003 - 2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    PciPlatform.h

Abstract:

--*/
#ifndef PCI_PLATFORM_H_
#define PCI_PLATFORM_H_


#include <PiDxe.h>
//#include "EfiDriverLib.h"
//#include "CMOSMap.h"
#include "ChipsetCmos.h"
#include "Platform.h"

//
// Consumed Protocols
//
//#include EFI_PROTOCOL_DEFINITION     (FirmwareVolume)
//#include EFI_PROTOCOL_DEFINITION     (PciIo)
//#include "FirmwareVolume"
//#include "PciIo"
//
// Produced Protocols
//
#include <Protocol/PciPlatform.h>

//#if defined SUPPORT_LVDS_DISPLAY && SUPPORT_LVDS_DISPLAY
////
//// Cedarview Video BIOS specific offset
////
//#define ACTIVE_LFP_OFFSET              0x1136
//#define ACTIVE_LFP_ENABLE                0x08
//#define ACTIVE_LFP_MASK                  0x18
//#define ACTIVE_EDP_ENABLE                0x18
//#define PANEL_TYPE_OFFSET              0x14D2
//#define EDP_LINK_PARAMETERS_OFFSET     0x14AF
//#define EDP_INTERFACE_TYPE_MASK          0x0F0
//#define LVDS_INTERFACE_TYPE_OFFSET     0x14D6
//#define LVDS_COLOR_DEPTH_OFFSET        0x14E0
//#define LFP_EDID_OFFSET                0x14D4
//#define LFP_EDID_EN                      0x40
//#define BACKLIGHT_CONTROL_OFFSET       0x1A25
//#define INV_POLARITY_INVERTED            0x04
//#define EDP_DATA_RATE_MASK               0x0F
//#define PANEL_POWER_SEQUENCING_OFFSET  0x1581
//#define POWERON_TO_BACKLIGHT_ENABLE_DELAY_TIME_MASK  0x1FFF
//#define POWERON_DELAY_TIME_MASK                      0x1FFF
//#define BACKLIGHT_OFF_TO_POWER_DOWN_DELAY_TIME_MASK  0x1FFF
//#define POWERON_DOWN_DELAY_TIME_MASK                 0x1FFF
//#define POWER_CYCLE_DELAY_TIME_MASK                  0x1F
//#define SYSTEM_BIOS_OFFSET             0x0D65
//#define INT15_EDID_HOOK_OFFSET           0x0E
#define IGD_DID_II                     0x0BE1
#define IGD_DID_0BE4                   0x0BE4
#define IGD_DID_BXT_A0                 0x0A84
//[-start-151228-IB03090424-add]//
#define IGD_DID_BXTP                   0x5A84
//[-end-151228-IB03090424-add]//
//[-start-160308-IB03090425-add]//
#define IGD_DID_BXTP_1                 0x5A85  // 12EU SKU
//[-end-160308-IB03090425-add]//
#define OPROM_DID_OFFSET               0x46
//
//
//typedef struct {
//  UINT16    Width;
//  UINT16    Height;
//} RESOLUTION;
//#endif

typedef struct {
  EFI_GUID  FileName;
  UINTN     Segment;
  UINTN     Bus;
  UINTN     Device;
  UINTN     Function;
  UINT16    VendorId;
  UINT16    DeviceId;
  UINT8     Flag;
} PCI_OPTION_ROM_TABLE;

EFI_STATUS
EFIAPI
PhaseNotify (
  IN  EFI_PCI_PLATFORM_PROTOCOL                     *This,
  IN  EFI_HANDLE                                    HostBridge,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE               ChipsetPhase
  ) ;


EFI_STATUS
EFIAPI
PlatformPrepController (
  IN EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN   EFI_HANDLE                                   HostBridge,
  IN  EFI_HANDLE                                    RootBridge,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS   PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE  Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE               ChipsetPhase
  );

EFI_STATUS
EFIAPI
GetPlatformPolicy (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL                      *This,
  OUT EFI_PCI_PLATFORM_POLICY                       *PciPolicy
  );

EFI_STATUS
EFIAPI
GetPciRom (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN EFI_HANDLE                                     PciHandle,
  OUT  VOID                                         **RomImage,
  OUT  UINTN                                        *RomSize
  );

//#if defined SUPPORT_LVDS_DISPLAY && SUPPORT_LVDS_DISPLAY
//VOID
//PatchVideoRom (
//  IN OUT VOID                                       **Buffer
//  );
//#endif

#endif


