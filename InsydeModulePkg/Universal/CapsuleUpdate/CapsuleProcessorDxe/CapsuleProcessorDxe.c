/** @file
  Firmware Management Protocol implementation for Capsule Processor

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "CapsuleProcessorDxe.h"

//
// The firmware regions share the same version number - the ESRT firmware version
//
CAPSULE_FMP_INSTANCE             mCapsuleFmp;

/**
  Find the Capsule Fmp Entry with specific ImageIndex from the CapsuleFmp instance

  @param[in]  ImageIndex        ImageIndex in EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  
  @returns                      The Capsule Fmp Entry pointer with the specified ImageIndex
                                NULL if Capsule Fmp Entry was not found

**/
STATIC
CAPSULE_FMP_ENTRY*
FindCapsuleFmpEntryByIndex (
  UINT8   ImageIndex
  )
{
  LIST_ENTRY            *Link;
  CAPSULE_FMP_ENTRY     *CapsuleFmpEntry;

  Link = GetFirstNode (&mCapsuleFmp.CapsuleProcessorListHead);
  while (!IsNull (&mCapsuleFmp.CapsuleProcessorListHead, Link)) {
    CapsuleFmpEntry = (CAPSULE_FMP_ENTRY*)Link;
    if (CapsuleFmpEntry->ImageIndex == ImageIndex) {
      return CapsuleFmpEntry;
    }
    Link = GetNextNode(&mCapsuleFmp.CapsuleProcessorListHead, Link);
  };
  return NULL;
}

/**
  Returns information about the current firmware image(s) of the device.

  This function allows a copy of the current firmware image to be created and saved.
  The saved copy could later been used, for example, in firmware image recovery or rollback.

  @param[in]      This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in, out] ImageInfoSize      A pointer to the size, in bytes, of the ImageInfo buffer. 
                                     On input, this is the size of the buffer allocated by the caller.
                                     On output, it is the size of the buffer returned by the firmware 
                                     if the buffer was large enough, or the size of the buffer needed
                                     to contain the image(s) information if the buffer was too small.
  @param[in, out] ImageInfo          A pointer to the buffer in which firmware places the current image(s)
                                     information. The information is an array of EFI_FIRMWARE_IMAGE_DESCRIPTORs.
  @param[out]     DescriptorVersion  A pointer to the location in which firmware returns the version number
                                     associated with the EFI_FIRMWARE_IMAGE_DESCRIPTOR. 
  @param[out]     DescriptorCount    A pointer to the location in which firmware returns the number of
                                     descriptors or firmware images within this device.
  @param[out]     DescriptorSize     A pointer to the location in which firmware returns the size, in bytes,
                                     of an individual EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  @param[out]     PackageVersion     A version number that represents all the firmware images in the device.
                                     The format is vendor specific and new version must have a greater value
                                     than the old version. If PackageVersion is not supported, the value is
                                     0xFFFFFFFF. A value of 0xFFFFFFFE indicates that package version comparison
                                     is to be performed using PackageVersionName. A value of 0xFFFFFFFD indicates
                                     that package version update is in progress.
  @param[out]     PackageVersionName A pointer to a pointer to a null-terminated string representing the
                                     package version name. The buffer is allocated by this function with 
                                     AllocatePool(), and it is the caller's responsibility to free it with a call
                                     to FreePool().

  @retval EFI_SUCCESS                The device was successfully updated with the new image.
  @retval EFI_BUFFER_TOO_SMALL       The ImageInfo buffer was too small. The current buffer size 
                                     needed to hold the image(s) information is returned in ImageInfoSize.                                               
  @retval EFI_INVALID_PARAMETER      ImageInfoSize is NULL.
  @retval EFI_DEVICE_ERROR           Valid information could not be returned. Possible corrupted image.

**/                         
EFI_STATUS
EFIAPI 
CapsuleProcessorGetImageInfo (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL       *This,
  IN OUT UINTN                                  *ImageInfoSize,
  IN OUT EFI_FIRMWARE_IMAGE_DESCRIPTOR          *ImageInfo,
  OUT    UINT32                                 *DescriptorVersion,
  OUT    UINT8                                  *DescriptorCount,
  OUT    UINTN                                  *DescriptorSize,
  OUT    UINT32                                 *PackageVersion,
  OUT    CHAR16                                 **PackageVersionName
  )
{
  EFI_STATUS            Status;
  UINTN                 Index;
  LIST_ENTRY            *Link;
  CAPSULE_FMP_ENTRY     *CapsuleFmpEntry;

  if (This == NULL || ImageInfoSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = EFI_SUCCESS;
  if (*ImageInfoSize < mCapsuleFmp.CapsuleProcessorCount * sizeof (EFI_FIRMWARE_IMAGE_DESCRIPTOR)) {
    Status = EFI_BUFFER_TOO_SMALL;
  }
  *ImageInfoSize = mCapsuleFmp.CapsuleProcessorCount * sizeof (EFI_FIRMWARE_IMAGE_DESCRIPTOR);
  if (DescriptorVersion != NULL) {     
    *DescriptorVersion = EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION;
  }
  if (DescriptorCount != NULL) {
    *DescriptorCount = (UINT8)mCapsuleFmp.CapsuleProcessorCount;
  }
  if (DescriptorSize != NULL) {
    *DescriptorSize = sizeof (EFI_FIRMWARE_IMAGE_DESCRIPTOR);
  }
  if (PackageVersion != NULL) {
    //
    // PackageVersion is not supported
    //
    *PackageVersion = 0xFFFFFFFF;
  }
  if (PackageVersionName != NULL) {
    //
    // PackageVersionName is not supported
    //
    *PackageVersionName = NULL;
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (ImageInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (ImageInfo, *ImageInfoSize);
  Link = GetFirstNode (&mCapsuleFmp.CapsuleProcessorListHead);
  Index = 0;
  while (!IsNull (&mCapsuleFmp.CapsuleProcessorListHead, Link)) {
    CapsuleFmpEntry = (CAPSULE_FMP_ENTRY*)Link;
    ImageInfo[Index].ImageIndex  = CapsuleFmpEntry->ImageIndex;
    if (CapsuleFmpEntry->Attributes != 0) {
      ImageInfo[Index].AttributesSupported = 1;
      ImageInfo[Index].AttributesSetting   = CapsuleFmpEntry->Attributes;
    }
    CopyGuid (&ImageInfo[Index].ImageTypeId, &CapsuleFmpEntry->ImageTypeGuid);
    Link = GetNextNode(&mCapsuleFmp.CapsuleProcessorListHead, Link);
    Index ++;
  }
  return Status;
}

/**
  Retrieves a copy of the current firmware image of the device. 

  This function allows a copy of the current firmware image to be created and saved.
  The saved copy could later been used, for example, in firmware image recovery or rollback.

  @param[in]  This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in]  ImageIndex         A unique number identifying the firmware image(s) within the device.
                                 The number is between 1 and DescriptorCount.
  @param[out] Image              Points to the buffer where the current image is copied to.
  @param[out] ImageSize          On entry, points to the size of the buffer pointed to by Image, in bytes.
                                 On return, points to the length of the image, in bytes.

  @retval EFI_SUCCESS            The device was successfully updated with the new image.
  @retval EFI_BUFFER_TOO_SMALL   The buffer specified by ImageSize is too small to hold the 
                                 image. The current buffer size needed to hold the image is returned 
                                 in ImageSize.
  @retval EFI_INVALID_PARAMETER  The Image was NULL.
  @retval EFI_NOT_FOUND          The current image is not copied to the buffer.
  @retval EFI_UNSUPPORTED        The operation is not supported.
  @retval EFI_SECURITY_VIOLATIO  The operation could not be performed due to an authentication failure.

**/                         
EFI_STATUS
EFIAPI 
CapsuleProcessorGetImage (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL       *This,
  IN     UINT8                                  ImageIndex,
  IN OUT VOID                                   *Image,
  IN OUT UINTN                                  *ImageSize
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Updates the firmware image of the device.

  This function updates the hardware with the new firmware image.
  This function returns EFI_UNSUPPORTED if the firmware image is not updatable.
  If the firmware image is updatable, the function should perform the following minimal validations
  before proceeding to do the firmware image update.
  - Validate the image authentication if image has attribute 
    IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED.  The function returns 
    EFI_SECURITY_VIOLATION if the validation fails.
  - Validate the image is a supported image for this device.  The function returns EFI_ABORTED if 
    the image is unsupported.  The function can optionally provide more detailed information on 
    why the image is not a supported image.
  - Validate the data from VendorCode if not null.  Image validation must be performed before 
    VendorCode data validation.  VendorCode data is ignored or considered invalid if image 
    validation failed.  The function returns EFI_ABORTED if the data is invalid.
  
  VendorCode enables vendor to implement vendor-specific firmware image update policy.  Null if 
  the caller did not specify the policy or use the default policy.  As an example, vendor can implement 
  a policy to allow an option to force a firmware image update when the abort reason is due to the new 
  firmware image version is older than the current firmware image version or bad image checksum.  
  Sensitive operations such as those wiping the entire firmware image and render the device to be
  non-functional should be encoded in the image itself rather than passed with the VendorCode.
  AbortReason enables vendor to have the option to provide a more detailed description of the abort
  reason to the caller.

  @param[in]  This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in]  ImageIndex         A unique number identifying the firmware image(s) within the device.
                                 The number is between 1 and DescriptorCount.
  @param[in]  Image              Points to the new image.
  @param[in]  ImageSize          Size of the new image in bytes.
  @param[in]  VendorCode         This enables vendor to implement vendor-specific firmware image update policy.  
                                 Null indicates the caller did not specify the policy or use the default policy.
  @param[in]  Progress           A function used by the driver to report the progress of the firmware update.
  @param[out] AbortReason        A pointer to a pointer to a null-terminated string providing more
                                 details for the aborted operation. The buffer is allocated by this function 
                                 with AllocatePool(), and it is the caller's responsibility to free it with a
                                 call to FreePool().

  @retval EFI_SUCCESS            The device was successfully updated with the new image.
  @retval EFI_ABORTED            The operation is aborted.
  @retval EFI_INVALID_PARAMETER  The Image was NULL.
  @retval EFI_UNSUPPORTED        The operation is not supported.
  @retval EFI_SECURITY_VIOLATION The operation could not be performed due to an authentication failure.

**/                         
EFI_STATUS
EFIAPI 
CapsuleProcessorSetImage (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL               *This,
  IN     UINT8                                          ImageIndex,
  IN     CONST VOID                                     *Image,
  IN     UINTN                                          ImageSize,
  IN     CONST VOID                                     *VendorCode,
  IN     EFI_FIRMWARE_MANAGEMENT_UPDATE_IMAGE_PROGRESS  Progress,
  OUT    CHAR16                                         **AbortReason
  )
{
  EFI_STATUS            Status;
  CAPSULE_FMP_ENTRY     *CapsuleFmpEntry;
  
  if (Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = EFI_UNSUPPORTED;
  CapsuleFmpEntry = FindCapsuleFmpEntryByIndex (ImageIndex);
  if(CapsuleFmpEntry == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (CapsuleFmpEntry->SetImage != NULL) {
    Status = CapsuleFmpEntry->SetImage (
                                This,
                                ImageIndex,
                                Image,
                                ImageSize,
                                VendorCode,
                                Progress,
                                AbortReason
                                );
  }
  return Status;
}

/**
  Checks if the firmware image is valid for the device.

  This function allows firmware update application to validate the firmware image without
  invoking the SetImage() first.

  @param[in]  This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in]  ImageIndex         A unique number identifying the firmware image(s) within the device.
                                 The number is between 1 and DescriptorCount.
  @param[in]  Image              Points to the new image.
  @param[in]  ImageSize          Size of the new image in bytes.
  @param[out] ImageUpdatable     Indicates if the new image is valid for update. It also provides,
                                 if available, additional information if the image is invalid.

  @retval EFI_SUCCESS            The image was successfully checked.
  @retval EFI_INVALID_PARAMETER  The Image was NULL.
  @retval EFI_UNSUPPORTED        The operation is not supported.
  @retval EFI_SECURITY_VIOLATION The operation could not be performed due to an authentication failure.

**/
EFI_STATUS
EFIAPI
CapsuleProcessorCheckImage (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL       *This,
  IN     UINT8                                  ImageIndex,
  IN     CONST VOID                             *Image,
  IN     UINTN                                  ImageSize,
  OUT    UINT32                                 *ImageUpdatable
  )
{
  EFI_STATUS            Status;
  CAPSULE_FMP_ENTRY     *CapsuleFmpEntry;

  if (Image == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = EFI_UNSUPPORTED;
  CapsuleFmpEntry = FindCapsuleFmpEntryByIndex (ImageIndex);
  if(CapsuleFmpEntry == NULL) {
    return Status;
  }
  if (CapsuleFmpEntry->CheckImage != NULL) {
    Status = CapsuleFmpEntry->CheckImage (
                                This,
                                ImageIndex,
                                Image,
                                ImageSize,
                                ImageUpdatable
                                );
  }
  return Status;
}

/**
  Returns information about the firmware package. 

  This function returns package information.

  @param[in]  This                     A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[out] PackageVersion           A version number that represents all the firmware images in the device.
                                       The format is vendor specific and new version must have a greater value
                                       than the old version. If PackageVersion is not supported, the value is
                                       0xFFFFFFFF. A value of 0xFFFFFFFE indicates that package version
                                       comparison is to be performed using PackageVersionName. A value of
                                       0xFFFFFFFD indicates that package version update is in progress.
  @param[out] PackageVersionName       A pointer to a pointer to a null-terminated string representing
                                       the package version name. The buffer is allocated by this function with
                                       AllocatePool(), and it is the caller's responsibility to free it with a
                                       call to FreePool().
  @param[out] PackageVersionNameMaxLen The maximum length of package version name if device supports update of
                                       package version name. A value of 0 indicates the device does not support
                                       update of package version name. Length is the number of Unicode characters,
                                       including the terminating null character.
  @param[out] AttributesSupported      Package attributes that are supported by this device. See 'Package Attribute 
                                       Definitions' for possible returned values of this parameter. A value of 1 
                                       indicates the attribute is supported and the current setting value is
                                       indicated in AttributesSetting. A value of 0 indicates the attribute is not
                                       supported and the current setting value in AttributesSetting is meaningless.
  @param[out] AttributesSetting        Package attributes. See 'Package Attribute Definitions' for possible returned
                                       values of this parameter

  @retval EFI_SUCCESS                  The package information was successfully returned.
  @retval EFI_UNSUPPORTED              The operation is not supported.

**/
EFI_STATUS
EFIAPI 
CapsuleProcessorGetPackageInfo (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL       *This,
  OUT    UINT32                                 *PackageVersion,
  OUT    CHAR16                                 **PackageVersionName,
  OUT    UINT32                                 *PackageVersionNameMaxLen,
  OUT    UINT64                                 *AttributesSupported,
  OUT    UINT64                                 *AttributesSetting
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Updates information about the firmware package.

  This function updates package information.
  This function returns EFI_UNSUPPORTED if the package information is not updatable.
  VendorCode enables vendor to implement vendor-specific package information update policy.
  Null if the caller did not specify this policy or use the default policy.

  @param[in]  This               A pointer to the EFI_FIRMWARE_MANAGEMENT_PROTOCOL instance.
  @param[in]  Image              Points to the authentication image.
                                 Null if authentication is not required.
  @param[in]  ImageSize          Size of the authentication image in bytes.
                                 0 if authentication is not required.
  @param[in]  VendorCode         This enables vendor to implement vendor-specific firmware
                                 image update policy.  
                                 Null indicates the caller did not specify this policy or use
                                 the default policy.
  @param[in]  PackageVersion     The new package version.
  @param[in]  PackageVersionName A pointer to the new null-terminated Unicode string representing
                                 the package version name. 
                                 The string length is equal to or less than the value returned in 
                                 PackageVersionNameMaxLen.

  @retval EFI_SUCCESS            The device was successfully updated with the new package
                                 information.
  @retval EFI_INVALID_PARAMETER  The PackageVersionName length is longer than the value 
                                 returned in PackageVersionNameMaxLen.
  @retval EFI_UNSUPPORTED        The operation is not supported.
  @retval EFI_SECURITY_VIOLATIO  The operation could not be performed due to an authentication failure.

**/
EFI_STATUS
EFIAPI 
CapsuleProcessorSetPackageInfo (
  IN     EFI_FIRMWARE_MANAGEMENT_PROTOCOL       *This,
  IN     CONST VOID                             *Image,
  IN     UINTN                                  ImageSize,
  IN     CONST VOID                             *VendorCode,
  IN     UINT32                                 PackageVersion,
  IN     CONST CHAR16                           *PackageVersionName
  )
{
  return EFI_UNSUPPORTED;
}

/**
  FirmwareManagement RuntimeDxe module entry point


  @param ImageHandle            A handle for the image that is initializing this driver
  @param SystemTable            A pointer to the EFI system table

  @retval EFI_SUCCESS:          Module initialized successfully
  @retval Others                Module initialized unsuccessfully
**/
EFI_STATUS
EFIAPI
CapsuleProcessorEntryPoint (
  IN     EFI_HANDLE             ImageHandle,
  IN     EFI_SYSTEM_TABLE       *SystemTable
  )
{
  EFI_STATUS    Status;
  //
  // Setup instance
  //
  ZeroMem (&mCapsuleFmp, sizeof (CAPSULE_FMP_INSTANCE));
  mCapsuleFmp.Signature          = CAPSULE_FMP_SIGNATURE;
  InitializeListHead (&mCapsuleFmp.CapsuleProcessorListHead);
  mCapsuleFmp.Fmp.GetImageInfo   = CapsuleProcessorGetImageInfo;
  mCapsuleFmp.Fmp.GetImage       = CapsuleProcessorGetImage;
  mCapsuleFmp.Fmp.SetImage       = CapsuleProcessorSetImage;
  mCapsuleFmp.Fmp.CheckImage     = CapsuleProcessorCheckImage;
  mCapsuleFmp.Fmp.GetPackageInfo = CapsuleProcessorGetPackageInfo;
  mCapsuleFmp.Fmp.SetPackageInfo = CapsuleProcessorSetPackageInfo;
  //
  // Install stock processors
  //
  InstallCapsuleInWindowsUx ();
  InstallCapsuleInUefiImage ();
  InstallCapsuleInUefiFmp ();
  //
  // Install FMP into image handle
  //
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gEfiFirmwareManagementProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mCapsuleFmp.Fmp
                  );
  return Status;
}