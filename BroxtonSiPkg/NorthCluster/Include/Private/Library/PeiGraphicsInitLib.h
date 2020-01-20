/** @file
  Header file for Graphics initialization

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

@par Specification Reference:
**/

#ifndef _PEI_GRAPHICS_INIT_LIB_H_
#define _PEI_GRAPHICS_INIT_LIB_H_

#include <SaRegs.h>
#include <SaAccess.h>
#include <SaCommonDefinitions.h>
#include <Ppi/SaPolicy.h>
#include <IndustryStandard/Pci22.h>
#include <Library/MmPciLib.h>
#include <Library/SteppingLib.h>
#include <Library/PreSiliconLib.h>

extern EFI_GUID gSiSaPreMemPolicyPpiGuid;

#define IGD_ENABLE  1
#define IGD_DISABLE 0

typedef enum {
  IGD               = 0,
  PCI,
  DISPLAY_DEVICE_MAX
} DISPLAY_DEVICE;

typedef enum {
  VBIOS_DEFAULT     = 0,
  CRT,
  LFP,
  CRT_LFP,
  TV,
  LFPSDVO,
  EFP,
  TVSDVO,
  CRT_LFPSDVO,
  CRT_EFP,
  IGD_BOOT_TYPE_MAX
} IGD_BOOT_TYPE;

typedef enum {
  GMS_FIXED         = 0,
  GMS_DVMT,
  GMS_FIXED_DVMT,
  GMS_MAX
} GRAPHICS_MEMORY_SELECTION;

typedef enum {
  GM_32M            = 1,
  GM_64M            = 2,
  GM_128M           = 4,
  GM_MAX
} STOLEN_MEMORY;

typedef struct {
  UINT64  BaseAddr;
  UINT32  Offset;
  UINT32  AndMask;
  UINT32  OrMask;
} BOOT_SCRIPT_REGISTER_SETTING;

#define GT_WAIT_TIMEOUT     3000     ///< ~3 seconds

#define GTTMMADR_SIZE_4MB       0x400000
#define GTT_SIZE_2MB            1
#define GTT_SIZE_4MB            2
#define GTT_SIZE_8MB            3

#define APERTURE_SIZE_128MB     1
#define APERTURE_SIZE_256MB     2
#define APERTURE_SIZE_512MB     3
#define APERTURE_SIZE_1024MB    4
#define APERTURE_SIZE_2048MB    5
#define APERTURE_SIZE_4096MB    6

#define RC6CTXBASE_SIZE       24*1024    //Top of Stolen Memory - 24K

///
/// PAVP Modes
///
#define PAVP_PCM_SIZE_1_MB    1
#define PAVP_PCM_SIZE_2_MB    2
#define PAVP_PCM_SIZE_4_MB    4
#define PAVP_PCM_SIZE_8_MB    8

#define PAVP_LITE_MODE        1



/**
  "Poll Status" for GT Readiness

  @param[in] Base             Base address of MMIO
  @param[in] Offset           MMIO Offset
  @param[in] Mask             Mask
  @param[in] Result           Value to wait for
**/
VOID
PollGtReady (
  IN       UINT64                  Base,
  IN       UINT32                  Offset,
  IN       UINT32                  Mask,
  IN       UINT32                  Result
  );

/**
  Initialize PAVP feature

  @param[in] GtConfig    Instance of GRAPHICS_CONFIG

  @retval EFI_SUCCESS    PAVP initialization complete
**/
EFI_STATUS
PavpInit(
IN       GRAPHICS_CONFIG             *GtConfig
);

/**
  Initialize GT Power management

  @param[in] GtConfig   Instance of GRAPHICS_CONFIG

  @retval EFI_SUCCESS   GT Power management initialization complete
**/
EFI_STATUS
GraphicsPmInit(
IN       GRAPHICS_CONFIG             *GtConfig
);

/**
  Program the max Cd Clock supported by the platform

  @param[in] GtConfig            Instance of GRAPHICS_CONFIG
  @param[in] GttMmAdr            Base Address of IGFX MMIO BAR

  @retval EFI_SUCCESS            GT Power Management initialization complete
  @retval EFI_INVALID_PARAMETER  The input parameter is invalid
**/
EFI_STATUS
CdClkInit (
  IN GRAPHICS_CONFIG *GtConfig,
  IN UINTN           GttMmAdr
  );

/**
GraphicsPreMemInit: Initialize the IGD if no other external graphics is present

  @param[in] PeiServices          General purpose services available to every PEIM.
**/
VOID
GraphicsPreMemInit(
  IN CONST EFI_PEI_SERVICES  **PeiServices
);

#define PCI_BCC                 0x000B  // Base Class Code Registe
#define PCI_BAR0                0x0010  // Base Address Register 0
#define PCI_BAR5                0x0024  // Base Address Register 5
#define PCI_VID                 0x0000  // Vendor ID Register

//
//Device 28(PCIe) Reg offsets and bit definitions
//
#define R_PCI_PCI_VID           0x00
#define R_PCI_PCI_SCC           0x0A
#define R_PCI_PCI_HEADTYP       0x0E
#define B_PCI_PCI_HEADTYP_MFD   0x80
#define R_PCI_PCI_PBN           0x18
#define R_PCI_PCI_SCBN          0x19
#define R_PCI_PCI_SBBN          0x1A
//
//For SeC Engine check
//
// HECI PCI Access Macro
//
#define CSE_BUS                 0
#define CSE_DEVICE_NUMBER       15  // Refer to WLV_BXT_Addressmap_xxx.docx

#define HECI_FUNCTION_NUMBER    0x00
#define HECI2_FUNCTION_NUMBER   0x01

#define R_SEC_DevID_VID         0x0
#define S_SEC_DevID_MASK        0xFFFF0000
#define S_SEC_DevID_RANGE_LO    0x5A9A  // B.Michael need to update in BWG0.3
#define S_SEC_DevID_RANGE_HI    0x5A9E  // B.Michael need to update in BWG0.3

#define HeciPciRead32(Register) MmPci32(0, CSE_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register)

#endif
