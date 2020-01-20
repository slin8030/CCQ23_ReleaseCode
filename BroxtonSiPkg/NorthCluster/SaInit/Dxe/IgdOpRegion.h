/** @file
This is part of the implementation of an Intel Graphics drivers OpRegion /
  Software SCI interface between system BIOS, ASL code, and Graphics drivers.

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
  - OpRegion / Software SCI SPEC
**/

#ifndef _IGD_OPREGION_H_
#define _IGD_OPREGION_H_

///
/// Statements that include other header files.
///
#include "SaInit.h"
#include "SaCommonDefinitions.h"

#include <Uefi/UefiInternalFormRepresentation.h>
#include <FrameworkDxe.h>

///
/// OpRegion (Miscellaneous) defines.
///
/// OpRegion Header defines.
///

#define HEADER_SIGNATURE    "IntelGraphicsMem"
#define HEADER_SIZE         0x2000
#define HEADER_OPREGION_VER 0x0200
#define HEADER_OPREGION_REV 0x00
///
/// For BXT Tablet, MailBOX2(SCI) and MailBOX5(Bios to Drv MailBox Extended), not supported.
///
#define HEADER_MBOX_SUPPORT (HD_MBOX4 + HD_MBOX3 + HD_MBOX1)
#define HD_MBOX1            BIT0
#define HD_MBOX2            BIT1
#define HD_MBOX3            BIT2
#define HD_MBOX4            BIT3
#define HD_MBOX5            BIT4
#define SVER_SIZE           32

///
/// Audio Type support for BXT
///
#define AUDIO_TYPE_SUPPORT_MASK    0xFFFFFFF3
#define NO_AUDIO_SUPPORT           (0<<2)
#define HD_AUDIO_SUPPORT           (1<<2)
#define LPE_AUDIO_SUPPORT          (2<<2)
#define AUDIO_TYPE_FIELD_MASK      0xFFFFFFEF
#define AUDIO_TYPE_FIELD_VALID     (1<<4)
#define AUDIO_TYPE_FIELD_INVALID   (0<<4)
///
/// OpRegion Mailbox 1 EQUates.
///
/// OpRegion Mailbox 3 EQUates.
///

#define ALS_ENABLE            BIT0
#define BLC_ENABLE            BIT1
#define BACKLIGHT_BRIGHTNESS  0xFF
#define FIELD_VALID_BIT       BIT31
#define WORD_FIELD_VALID_BIT  BIT15
#define PFIT_ENABLE           BIT2
#define PFIT_OPRN_AUTO        0x00000000
#define PFIT_OPRN_SCALING     0x00000007
#define PFIT_OPRN_OFF         0x00000000
#define PFIT_SETUP_AUTO       0
#define PFIT_SETUP_SCALING    1
#define PFIT_SETUP_OFF        2
#define INIT_BRIGHT_LEVEL     0x64
#define PFIT_STRETCH          6
#define PFIT_CENTER           1

///
/// SA PCI register access defines.
///

#define IgdMmPci32(Register)            MmPci32   (0, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, Register)
#define IgdMmPci16Or(Register, OrData)  MmPci16Or (0, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, Register, OrData)
#define IgdMmPci16AndThenOr(Register,AndData,OrData) MmPci16AndThenOr (0, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, Register, AndData, OrData)

///
/// Video BIOS / VBT defines
///

#define OPTION_ROM_SIGNATURE    0xAA55
#define VBIOS_LOCATION_PRIMARY  0xC0000

#define VBT_SIGNATURE           SIGNATURE_32 ('$', 'V', 'B', 'T')
///
/// Typedef stuctures
///

#pragma pack (1)
typedef struct {
  UINT16  Signature;    // 0xAA55
  UINT8   Size512;
  UINT8   Reserved[21];
  UINT16  PcirOffset;
  UINT16  VbtOffset;
} INTEL_VBIOS_OPTION_ROM_HEADER;
#pragma pack ()

#pragma pack (1)
typedef struct {
  UINT32  Signature;  // "PCIR"
  UINT16  VendorId;   // 0x8086
  UINT16  DeviceId;
  UINT16  Reserved0;
  UINT16  Length;
  UINT8   Revision;
  UINT8   ClassCode[3];
  UINT16  ImageLength;
  UINT16  CodeRevision;
  UINT8   CodeType;
  UINT8   Indicator;
  UINT16  Reserved1;
} INTEL_VBIOS_PCIR_STRUCTURE;
#pragma pack ()

#pragma pack (1)
typedef struct {
  UINT8   HeaderSignature[20];
  UINT16  HeaderVersion;
  UINT16  HeaderSize;
  UINT16  HeaderVbtSize;
  UINT8   HeaderVbtCheckSum;
  UINT8   HeaderReserved;
  UINT32  HeaderOffsetVbtDataBlock;
  UINT32  HeaderOffsetAim1;
  UINT32  HeaderOffsetAim2;
  UINT32  HeaderOffsetAim3;
  UINT32  HeaderOffsetAim4;
  UINT8   DataHeaderSignature[16];
  UINT16  DataHeaderVersion;
  UINT16  DataHeaderSize;
  UINT16  DataHeaderDataBlockSize;
  UINT8   CoreBlockId;
  UINT16  CoreBlockSize;
  UINT16  CoreBlockBiosSize;
  UINT8   CoreBlockBiosType;
  UINT8   CoreBlockReleaseStatus;
  UINT8   CoreBlockHWSupported;
  UINT8   CoreBlockIntegratedHW;
  UINT8   CoreBlockBiosBuild[4];
  UINT8   CoreBlockBiosSignOn[155];
} VBIOS_VBT_STRUCTURE;
#pragma pack ()

///
/// Driver Private Function definitions
///

/**
  GetSVER

  @param[out]  SVER

  @retval  EFI_STATUS
**/
EFI_STATUS
GetSVER (
  OUT UINT8 *SVER
  );

/**
  Acquire the string associated with the ProducerGuid and return it.

  @param[in]   ProducerGuid  The Guid to search the HII database for
  @param[in]   Token         The token value of the string to extract
  @param[out]  String        The string that is extracted

  @retval  EFI_SUCCESS       The function completed successfully
  @retval  EFI_NOT_FOUND     The requested string was not found
**/
EFI_STATUS
GetStringFromToken (
  IN      EFI_GUID                  *ProducerGuid,
  IN      STRING_REF                Token,
  OUT     CHAR16                    **String
  )
;

/**
  Graphics OpRegion / Software SCI driver installation function.


  @param[in]  None

  @retval  EFI_SUCCESS     The driver installed without error.
  @retval  EFI_ABORTED     The driver encountered an error and could not complete
                           installation of the ACPI tables.
**/
EFI_STATUS
IgdOpRegionInit (
  void
  )
;

/**
  Extract information pertaining to the HiiHandle


  @param[in]      HiiHandle        Hii handle
  @param[in,out]  ImageLength      For input, length of DefaultImage;
                                   For output, length of actually required

  @param[out]     DefaultImage     Image buffer prepared by caller
  @param[out]     Guid             Guid information about the form

  @retval  EFI_OUT_OF_RESOURCES    No enough buffer to allocate
  @retval  EFI_BUFFER_TOO_SMALL    DefualtImage has no enough ImageLength
  @retval  EFI_SUCCESS             Successfully extract data from Hii database.
**/
EFI_STATUS
ExtractDataFromHiiHandle (
  IN      EFI_HII_HANDLE      HiiHandle,
  IN OUT  UINT16              *ImageLength,
  OUT     UINT8               *DefaultImage,
  OUT     EFI_GUID            *Guid
  )
;
#endif
