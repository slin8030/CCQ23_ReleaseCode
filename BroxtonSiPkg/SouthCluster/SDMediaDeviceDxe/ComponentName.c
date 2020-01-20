/** @file
  Header file for ComponentName.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#include <MediaDeviceDriver.h>

//#pragma optimize ("", off)
//
// EFI Component Name Protocol
//
EFI_COMPONENT_NAME_PROTOCOL     gMediaDeviceComponentName = {
  MediaDeviceComponentNameGetDriverName,
  MediaDeviceComponentNameGetControllerName,
  "eng"
};

static EFI_UNICODE_STRING_TABLE mMediaDeviceDriverNameTable[] = {
  { "eng", L"UEFI MMC/SD Media Device Driver" },
  { NULL , NULL }
};

/**
  Retrieves a Unicode string that is the user readable name of the EFI Driver.

  @param[in]  This       - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  Language   - A pointer to a three character ISO 639-2 language identifier.
                           This is the language of the driver name that that the caller
                           is requesting, and it must match one of the languages specified
                           in SupportedLanguages.  The number of languages supported by a
                           driver is up to the driver writer.
  @param[out]  DriverName - A pointer to the Unicode string to return.  This Unicode string
                           is the name of the driver specified by This in the language
                           specified by Language.

  @retval   EFI_SUCCESS           - The Unicode string for the Driver specified by This
                                    and the language specified by Language was returned
                                    in DriverName.
  @retval   EFI_INVALID_PARAMETER - Language is NULL.
  @retval   EFI_INVALID_PARAMETER - DriverName is NULL.
  @retval   EFI_UNSUPPORTED       - The driver specified by This does not support the
                                    language specified by Language.
**/
EFI_STATUS
EFIAPI
MediaDeviceComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL     *This,
  IN  CHAR8                           *Language,
  OUT CHAR16                          **DriverName
  )
{
  return LookupUnicodeString (
          Language,
          gMediaDeviceComponentName.SupportedLanguages,
          mMediaDeviceDriverNameTable,
          DriverName
          );
}
/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by an EFI Driver.

  @param[in]  This             - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  ControllerHandle - The handle of a controller that the driver specified by
                                 This is managing.  This handle specifies the controller
                                 whose name is to be returned.
  @param[in]  ChildHandle      - The handle of the child controller to retrieve the name
                                 of.  This is an optional parameter that may be NULL.  It
                                 will be NULL for device drivers.  It will also be NULL
                                 for a bus drivers that wish to retrieve the name of the
                                 bus controller.  It will not be NULL for a bus driver
                                 that wishes to retrieve the name of a child controller.
   @param[in] Language         - A pointer to a three character ISO 639-2 language
                                 identifier.  This is the language of the controller name
                                 that that the caller is requesting, and it must match one
                                 of the languages specified in SupportedLanguages.  The
                                 number of languages supported by a driver is up to the
                                 driver writer.
   @param[out] ControllerName   - A pointer to the Unicode string to return.  This Unicode
                                 string is the name of the controller specified by
                                 ControllerHandle and ChildHandle in the language
                                 specified by Language from the point of view of the
                                 driver specified by This.

  @retval   EFI_SUCCESS          - The Unicode string for the user readable name in the
                                    language specified by Language for the driver
                                    specified by This was returned in DriverName.
  @retval  EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
  @retval  EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a valid
                                   EFI_HANDLE.
  @retval  EFI_INVALID_PARAMETER - Language is NULL.
  @retval  EFI_INVALID_PARAMETER - ControllerName is NULL.
  @retval  EFI_UNSUPPORTED       - The driver specified by This is not currently
                                   managing the controller specified by
                                   ControllerHandle and ChildHandle.
  @retval  EFI_UNSUPPORTED       - The driver specified by This does not support the
                                   language specified by Language.
**/
EFI_STATUS
EFIAPI
MediaDeviceComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ChildHandle, OPTIONAL
  IN  CHAR8                           *Language,
  OUT CHAR16                          **ControllerName
  )
{
  EFI_STATUS               Status;
  EFI_BLOCK_IO_PROTOCOL    *BlockIo;
  CARD_DATA                *CardData;

  //
  // This is a device driver, so ChildHandle must be NULL.
  //
  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get the device context
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  gMediaDeviceDriverBinding.DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  CardData  = CARD_DATA_FROM_THIS (BlockIo);

  return LookupUnicodeString (
           Language,
           gMediaDeviceComponentName.SupportedLanguages,
           CardData->ControllerNameTable,
           ControllerName
           );
}
