/** @file
  This header file is for Switchable Graphics Feature DXE driver.

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

#ifndef _SWITCHABLE_GRAPHICS_DXE_H_
#define _SWITCHABLE_GRAPHICS_DXE_H_

#include <Guid/HobList.h>
#include <Guid/SgInfoHob.h>
#include <IndustryStandard/Acpi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CommonPciLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciExpressLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <MchRegs.h>
#include <ScAccess.h>
#include <PlatformBaseAddresses.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/LegacyRegion.h>
#include <Protocol/PciIo.h>
#include <Protocol/SwitchableGraphicsEvent.h>
#include <SwitchableGraphicsDefine.h>
#include <SaCommonDefinitions.h>

#define MemoryRead16(Address)                 *(UINT16 *)(Address)
#define MemoryRead8(Address)                  *(UINT8 *)(Address)
#define AML_OPREGION_OP                       0x805b

#pragma pack (push, 1)
typedef struct {
  UINTN                                       Address;
  UINTN                                       Size;
} BINARY_INFORMATION;
#pragma pack (pop)

#pragma pack (push, 1)
typedef struct _SG_DXE_INFORMATION_DATA {
  BINARY_INFORMATION                          Vbios;
  BINARY_INFORMATION                          MxmBinFile;
  UINT32                                      GpioBaseAddress;
  UINT8                                       DgpuBus;
  UINT16                                      DgpuVendorId;
  UINT8                                       AmdSecondaryGrcphicsCommandRegister;
  UINT8                                       NvidiaSecondaryGrcphicsCommandRegister;
  BOOLEAN                                     SlaveMxmGpuSupport;
  VOID                                        *SgOpRegionAddress;
} SG_DXE_INFORMATION_DATA;
#pragma pack (pop)

#pragma pack (push, 1)
//
// This structure must match with the structure in AmdSsdt.asl and NvidiaSsdt.asl SG OpRegion "SGOP"
//
typedef struct {
  //
  // PEG Endpoint Info
  //
  UINT32      PcieBaseAddress;              // Any Device's PCIe Config Space Base Address
  UINT32      DgpuBridgePcieBaseAddress;    // dGPU bridge's PCIe Base Address
  UINT32      DgpuDevicePcieBaseAddress;    // dGPU device's PCIe Base Address
  UINT32      EndpointPcieCapBaseAddress;   // PEG Endpoint PCIe Capability Structure Base Address
  //
  // HG Board Info
  //
  UINT8       SgGPIOSupport;                // SG GPIO Support
  UINT32      GpioBaseAddress;              // GPIO Base Address
  //
  // GPIO Delay Time
  //
  UINT32      ActiveDgpuPwrEnableDelayTime; // Active dGPU_PWR_EN Delay time
  UINT32      ActiveDgpuHoldRstDelayTime;   // Active dGPU_HOLD_RST_PIN Delay time
  UINT32      InctiveDgpuHoldRstDelayTime;  // Inactive dGPU_HOLD_RST_PIN Delay time
  UINT32      DgpuSsidSvid;                 // dGPU SSID/SVID for Hybrid Graphics runtime restore
  //
  // GPIO Pin Setting
  //
  UINT32     HoldRstGPIOCommunityOffset;    // dGPU HLD RST GPIO Community Offset
  UINT32     HoldRstGPIOPinOffset;          // dGPU HLD RST GPIO Pin Offset
  UINT8      HoldRstGPIOActiveInfo;         // dGPU HLD RST GPIO Active Information
  UINT32     PowerEnableGPIOCommunityOffset;// dGPU Pwr Enable GPIO Community Offset
  UINT32     PowerEnableGPIOPinOffset;      // dGPU Pwr Enable GPIO Pin Offset
  UINT8      PowerEnableGPIOActiveInfo;     // dGPU Pwr Enable GPIO Active Information
  UINT8      SgDgpuEnable;                  // Sg Dgpu Enable Bit
  UINT8      SgDgpuDisable;                 // Sg Dgpu Disable Bit
  UINT8      SgDgpuPEnable;                 // Sg Dgpu Enable Bit
  UINT8      SgDgpuPDisable;                // Sg Dgpu Disable Bit
} OPERATION_REGION_SG;
#pragma pack (pop)

#pragma pack (push, 1)
//
// This structure must match with the structure in AmdSsdt.asl AMD OpRegion "AOPR"
//
typedef struct {
  UINT8       Reserved;                     // Reserved
} OPERATION_REGION_AMD;
#pragma pack (pop)

#pragma pack (push, 1)
//
// This structure must match with the structure in NvidiaSsdt.asl NVIDIA OpRegion "NOPR"
//
typedef struct {
  UINT8       DgpuHotPlugSupport;           // Optimus dGPU HotPlug Support or not
  UINT8       DgpuPowerControlSupport;      // Optimus dGPU Power Control Support or not
  UINT8       GpsFeatureSupport;            // GPS Feature Support or not
  UINT8       OptimusGc6Support;            // Optimus GC6 Feature Support or not
  UINT32      MxmBinarySize;                // MXM bin file Size (bits)
  CHAR8       MxmBinaryBuffer[0x1000];      // MXM Bin file 4K
} OPERATION_REGION_NVIDIA;
#pragma pack ()

#pragma pack (push, 1)
//
// This structure must match with the structure in NvidiaSsdt.asl NVIDIA OpRegion "NOPR"
//
typedef struct {
  UINT32      RVBS;                         // Runtime VBIOS Image Size
  CHAR8       VBOIS[0x40000];               // VBIOS 128KB
} OPERATION_REGION_VBIOS;
#pragma pack ()

//
// Function Prototypes
//
VOID
EFIAPI
SwitchableGraphicsBdsCallback (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  );

STATIC
EFI_STATUS
SwitchableGraphicsDxeInitialize (
  IN OUT SG_DXE_INFORMATION_DATA              **SgDxeInfoData,
  IN OUT SG_INFORMATION_DATA_HOB              **SgInfoDataHob
  );

STATIC
EFI_STATUS
DgpuBdsInitialize (
  IN OUT SG_DXE_INFORMATION_DATA              *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  );

STATIC
EFI_STATUS
SearchDiscreteGraphicsVbios (
  IN OUT SG_DXE_INFORMATION_DATA              *SgDxeInfoData
  );

STATIC
EFI_STATUS
SetSwitchableGraphicsSsdt (
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  );

STATIC
EFI_STATUS
InitializeOpRegion (
  IN EFI_ACPI_DESCRIPTION_HEADER              *NvStoreTable,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  );

STATIC
VOID
UpdateSgOpRegion (
  IN OUT OPERATION_REGION_SG                  *SgOpRegion,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  );

STATIC
VOID
UpdateAmdOpRegion (
  IN OUT OPERATION_REGION_AMD                 *AmdOpRegion,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  );

STATIC
VOID
UpdateNvidiaOpRegion (
  IN OUT OPERATION_REGION_NVIDIA              *NvidiaOpRegion,
  IN SG_DXE_INFORMATION_DATA                  *SgDxeInfoData,
  IN SG_INFORMATION_DATA_HOB                  *SgInfoDataHob
  );

VOID
EFIAPI
SetSecondaryGrcphicsCommandRegister (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  );

VOID
EFIAPI
CloseDiscreteHdAudio (
  IN EFI_EVENT                                Event,
  IN VOID                                     *Context
  );

STATIC
VOID
DisableDgpuBridgeHotplugSmi (
  IN  SG_INFORMATION_DATA_HOB                 *SgInfoDataHob
  );

STATIC
VOID
SetDgpuSsidSvid (
  IN  SG_DXE_INFORMATION_DATA                 *SgDxeInfoData
  );

BOOLEAN
IsAmlOpRegionObject (
  IN UINT8                                    *DsdtPointer
  );

EFI_STATUS
SetOpRegion (
  IN UINT8                                    *DsdtPointer,
  IN VOID*                                    RegionStartAddress,
  IN UINT32                                   RegionSize
  );

EFI_STATUS
AcpiChecksum (
  IN VOID                                     *Buffer,
  IN UINTN                                    Size,
  IN UINTN                                    ChecksumOffset
  );

VOID
EFIAPI
FreeSgDxeInfoData (
  IN OUT SG_DXE_INFORMATION_DATA              **SgDxeInfoData
  );
#endif
