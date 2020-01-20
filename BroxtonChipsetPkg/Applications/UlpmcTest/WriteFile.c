/** @file

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
/** @file
  Write File function.

  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#include <Uefi.h>

#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

extern EFI_BOOT_SERVICES     *gBS;

/**
  InitFileSystem

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] Name           
  @param[in] MODE           
  @param[OUT] FileHandle    
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
InitFileSystem (
  IN  EFI_HANDLE  ImageHandle,
  IN  CHAR16      *Name,
  IN  UINT64      Mode,
  OUT EFI_FILE    **FileHandle
)
{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *SimpleFileSystem;
  EFI_LOADED_IMAGE_PROTOCOL        *Image;
  EFI_DEVICE_PATH_PROTOCOL         *DevicePath;
  EFI_HANDLE                       DeviceHandle;
  EFI_STATUS                       Status;
  static EFI_FILE                  *mRoot = NULL;
 
  Status = gBS->HandleProtocol (
               		ImageHandle,
               		&gEfiLoadedImageProtocolGuid,
               		(VOID **)&Image
               		);
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }
	
  Status = gBS->HandleProtocol (
               		Image->DeviceHandle,
               		&gEfiDevicePathProtocolGuid,
               		(VOID **)&DevicePath
               		);
	
  if(EFI_ERROR(Status)) {   
    return EFI_INVALID_PARAMETER;
  }
		
  Status = gBS->LocateDevicePath ( 
                  &gEfiSimpleFileSystemProtocolGuid,
                  &DevicePath,
                  &DeviceHandle
                  );
  if(EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->HandleProtocol (
                  DeviceHandle, 
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&SimpleFileSystem
                  );
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }
		
  Status = SimpleFileSystem->OpenVolume (
                               SimpleFileSystem,
                               &mRoot
                               );
  if (EFI_ERROR (Status)) {    
    return EFI_INVALID_PARAMETER;
  }

  Status = mRoot->Open (
                    mRoot,
                    FileHandle,
                    Name,
                    Mode,
                    0
                    );
  if (EFI_ERROR (Status)) {  
    return EFI_INVALID_PARAMETER;
  }
  
  return EFI_SUCCESS;
}

/**
  WriteString2File

  @param[in] FilePtr        Pointer to file system.
  @param[in] String         The string will be wrote into file.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
WriteString2File(
  IN  EFI_FILE  *FilePtr,
  IN  CHAR8     *String
)
{
  UINTN  BufferSize = 0;
  
  BufferSize = strlen(String);
  FilePtr->Write(FilePtr, &BufferSize, String);
  FilePtr->Flush(FilePtr); 
  return EFI_SUCCESS;
}


/**
  MyUnicodeStrToAsciiStr

  @param[in]  Source         
  @param[out] Destination    
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
VOID
MyUnicodeStrToAsciiStr (
  IN  CONST CHAR16  *Source,
  OUT       CHAR8   *Destination
  )
{
  while (*Source != '\0') {
    *(Destination++) = (CHAR8) *(Source++);
  }

  *Destination = '\0';
}

/**
  MyAsciiToUnicodeStrStr

  @param[in]  Source         
  @param[out] Destination    
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
VOID
MyAsciiToUnicodeStrStr (
  IN  CONST CHAR8   *Source,
  OUT       CHAR16  *Destination
  )
{
  while (*Source != '\0') {
    *(Destination++) = (CHAR16) *(Source++);
  }
  *Destination = '\0';
}

/**
  ReadStringLine

  @param[in]  FilePtr    
  @param[in]  MaxNum           
  @param[out] String    
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
ReadStringLine(
  IN   EFI_FILE  *FilePtr,
  IN   UINT16    MaxNum,
  OUT  CHAR8     *String
)
{
  EFI_STATUS  Status;
  UINT16      Index = 0;

  for (Index = 0; Index < MaxNum; Index++) {
    
    UINTN   BufferSize = 1;
    CHAR8   AsciiStr[4];	
    CHAR16  UnicodeStr[4];
	
    Status = FilePtr->Read(FilePtr, &BufferSize, UnicodeStr);
    
    if (BufferSize == 0){
      String[Index] = 0;
      return EFI_NOT_READY;
    }
    
    UnicodeStr[BufferSize] = 0;	
    MyUnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
    
    if (AsciiStr[0] == 0x0D) {
      FilePtr->Read(FilePtr, &BufferSize, UnicodeStr);  
      break;
    }
    
    String[Index] = AsciiStr[0];
  }
  
  String[Index] = 0;
  
  return EFI_SUCCESS;
}


