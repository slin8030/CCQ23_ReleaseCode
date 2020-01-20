/** @file
  PEI Graphics Display header file

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
#ifndef _GRAPHICS_PEIM_INIT_H_
#define _GRAPHICS_PEIM_INIT_H_

#include <Library/BaseLib.h>
#include <PiPei.h>

#include <IndustryStandard/Bmp.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MmPciLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/HobLib.h>
#include <Guid/GraphicsInfoHob.h>

#include <Ppi/SaPolicy.h>
#include <Ppi/Stall.h>
#include <SaRegs.h>

/**
  DisplayInit: Initialize the IGD using GraphicsInitPeim and display logo in PEI
  
  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] GtConfig                    GRAPHICS_CONFIG to access the GtConfig related information
             GtConfig.GraphicsConfigPtr  Address of the Graphics Configuration Table
             GtConfig.LogoPtr            Address of Logo to be displayed
             GtConfig.LogoSize           Logo Size

  @retval     EFI_STATUS
**/
EFI_STATUS
EFIAPI
PeiDisplayInit (
//[-start-191206-IB16530064-add]//
  IN CONST EFI_PEI_SERVICES        **PeiServices,
//[-end-191206-IB16530064-add]//
  IN   GRAPHICS_CONFIG             *GtConfig
  );

/**
  FillFrameBufferAndShowLogo: Fill frame buffer with the image

  @param[in] GtConfig                    GRAPHICS_CONFIG to access the GtConfig related information
             GtConfig.GraphicsConfigPtr  Address of the Graphics Configuration Table

  @retval    EFI_STATUS
**/
EFI_STATUS
FillFrameBufferAndShowLogo(
IN GRAPHICS_CONFIG                    *GtConfig
);

/**
  PeiGraphicsNotifyCallback     Run this function after PEI_GRAPHICS_PPI is installed to CallPpiAndFillFrameBuffer.

  @param[in]  PeiServices       General purpose services available to every PEIM.
  @param[in]  NotifyDescriptor  Notify that this module published.
  @param[in]  Ppi               PPI that was installed.

  @retval     EFI_SUCCESS       The function completed successfully.
**/
EFI_STATUS
EFIAPI
PeiGraphicsNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );

/**
  CallPpiAndFillFrameBuffer: Call GraphicsInitPeim PeiGraphicsPpi to initalize display and get Mode info.
  Publish GraphicsInfoHob and call FillFrameBufferAndShowLogo

  @param[in] GtConfig           GRAPHICS_CONFIG to access the GtConfig related information
             GtConfig.LogoPtr   Address of Logo to be displayed
             GtConfig.LogoSize  Logo Size

  @retval    EFI_STATUS
**/
EFI_STATUS
EFIAPI
CallPpiAndFillFrameBuffer (
  IN   GRAPHICS_CONFIG             *GtConfig
  );

/**
  Convert a *.BMP graphics image to a GOP blt buffer. If a NULL Blt buffer
  is passed in a GopBlt buffer will be allocated by this routine. If a GopBlt
  buffer is passed in it will be used if it is big enough.

  @param[in]      BmpImage      Pointer to BMP file
  @param[in]      BmpImageSize  Number of bytes in BmpImage
  @param[in,out]  GopBlt        Buffer containing GOP version of BmpImage.
  @param[in,out]  GopBltSize    Size of GopBlt in bytes.
  @param[out]     PixelHeight   Height of GopBlt/BmpImage in pixels
  @param[out]     PixelWidth    Width of GopBlt/BmpImage in pixels

  @retval EFI_SUCCESS           GopBlt and GopBltSize are returned.
  @retval EFI_UNSUPPORTED       BmpImage is not a valid *.BMP image
  @retval EFI_BUFFER_TOO_SMALL  The passed in GopBlt buffer is not big enough.
                                GopBltSize will contain the required size.
  @retval EFI_OUT_OF_RESOURCES  No enough buffer to allocate.

**/
EFI_STATUS
ConvertBmpToGopBlt (
  IN     VOID      *BmpImage,
  IN     UINTN     BmpImageSize,
  IN OUT VOID      **GopBlt,
  IN OUT UINTN     *GopBltSize,
     OUT UINTN     *PixelHeight,
     OUT UINTN     *PixelWidth
  );

/**
  InstallPeiGraphicsPlatformPolicyPpi: Install PeiGraphicsPlatformPolicy PPI -Consumed by GraphicsInitPeim

  @param[in] GtConfig                    GRAPHICS_CONFIG to access the GtConfig related information
             GtConfig.GraphicsConfigPtr  Address of the Graphics Configuration Table

  @retval EFI_STATUS
**/
EFI_STATUS
EFIAPI
InstallPeiGraphicsPlatformPolicyPpi (
  GRAPHICS_CONFIG       *GtConfig
 );

/**
  This function provides a blocking stall for reset at least the given number of microseconds
  stipulated in the final argument.

  @param[in]  PeiServices   General purpose services available to every PEIM.
  @param[in]  This          Pointer to the local data for the interface.
  @param[in]  Microseconds  number of microseconds for which to stall.

  @retval EFI_SUCCESS the function provided at least the required stall.
**/
EFI_STATUS
EFIAPI
Stall (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN CONST EFI_PEI_STALL_PPI  *This,
  IN UINTN                    Microseconds
  );

/**
  Install Stall PPI - Consumed by GraphicsInitPeim

  @param[in]  None

  @retval EFI_SUCCESS if Ppi is installed successfully.
**/
EFI_STATUS
EFIAPI
InstallStallPpi(
  VOID
  );


/**
  TODO- Remove from RC
  PeiGetSectionFromFv finds the file in FV and gets file Address and Size

  @param[in] NameGuid               File GUID
  @param[out] Address               Pointer to the File Address
  @param[out] Size                  Pointer to File Size

  @retval EFI_SUCCESS               Successfull in reading the section from FV
**/
EFI_STATUS
EFIAPI
PeiGetSectionFromFv (
  IN CONST  EFI_GUID        NameGuid,
  OUT VOID                  **Address,
  OUT UINT32               *Size
  );
#endif
