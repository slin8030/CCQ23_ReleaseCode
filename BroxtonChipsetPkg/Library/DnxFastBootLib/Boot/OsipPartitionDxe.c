/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/** @file
**/

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
//#include <Library/UefiBootServicesTableLib.h>
//#include <Protocol/BlockIo.h>
//#include <Protocol/PciIo.h>
#include "OsipPrivate.h"

//#include <EfiFastBoot.h>

OSIP_HEADER             *mOsip               = NULL;
BOOLEAN                  mOsipFound          = FALSE;
EFI_BLOCK_IO_PROTOCOL   *mEmmcBlockIo        = NULL;
EFI_BLOCK_IO_PROTOCOL   *mSdBlockIo          = NULL;
EFI_BLOCK_IO_PROTOCOL   *mBootableBlockIo    = NULL;
UINT8 CommandLinePatch[48] = {0x20, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x62, 0x6f, 0x6f, 0x74, 0x2e, 0x77, 0x61, 0x6b,\
  	                          0x65, 0x73, 0x72, 0x63, 0x3d, 0x30, 0x30, 0x20, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x62,\
  	                          0x6f, 0x6f, 0x74, 0x2e, 0x6d, 0x6f, 0x64, 0x65, 0x3d, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x0a, 0x00};
  	                          
EFI_STATUS
LoadFileFromFileSystem (
  IN CONST CHAR16    *FileName,
  OUT VOID               **FileBuffer,
  OUT UINTN                *FileSize
);


EFI_STATUS
InitOsip( 
) 
{
  EFI_STATUS                  Status;
  EFI_HANDLE                  *HandleBuffer;
  UINTN                       Index;
  UINTN                       NumberOfHandles;
  //EFI_DEVICE_PATH_PROTOCOL   *DevicePath;
  EFI_PHYSICAL_ADDRESS       physAddr;
  
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_ERROR, " BLOCK IO protocol numbers = %d \n",NumberOfHandles));		
  
  for( Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol(
                          HandleBuffer[Index],
                          &gEfiBlockIoProtocolGuid,
                          (VOID **) &mEmmcBlockIo
                          );
    
    //
    // Allocate a buffer for the OSIP Header
    //
    Status = gBS->AllocatePages(
                            AllocateAnyPages,
                            EfiBootServicesData,
                            OSP_BLOCKS_TO_PAGES(OSIP_SIZE_IN_BLOCKS),
                            &physAddr);
    ASSERT_EFI_ERROR(Status);
  
    //
    // Copy OSIP from the storage device
    //
    Status = mEmmcBlockIo->ReadBlocks(
                                mEmmcBlockIo,
                                mEmmcBlockIo->Media->MediaId,
  
                                (EFI_LBA) 0,  // OSIP hard coded to start at LBA 0
                                OSIP_SIZE_IN_BLOCKS * OSP_BLOCK_SIZE,
                                (VOID *) (UINTN) physAddr);
    ASSERT_EFI_ERROR(Status);
  
    mOsip = (OSIP_HEADER *) (UINTN) physAddr;
    
    if (mOsip->Signature != OSIP_HEADER_SIGNATURE) {
    	mOsip = NULL;
      continue;
    }
    
    DEBUG ((EFI_D_ERROR, " Found a EMMC device.\n"));	
    
    ValidateOsip(mOsip, 1);
    
    mBootableBlockIo = mEmmcBlockIo;
    break;
  }  
    
  if (mBootableBlockIo == NULL) {
  	ASSERT_EFI_ERROR(Status);
  }
  
  return Status;
}

UINTN
GetVrlSize (
  IN    VOID  *Vrl
  )
{
  //
  // BUGBUG: This code is not based on VRL documentation
  //
  return (UINTN) *((UINT32*) (((UINTN) Vrl) + VRL_SIZE_LOCATION));
}

EFI_STATUS
EFIAPI
OsipLoadOsImageFromEmmc (
  IN    UINTN                           ImageNumber,
  OUT   EFI_PHYSICAL_ADDRESS            *ImageBase,
  OUT   UINTN                           *ImageSize,
  OUT   EFI_PHYSICAL_ADDRESS            *EntryPoint
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    osBase;
  UINTN                   osPages;
  //EFI_PHYSICAL_ADDRESS    loadPage;
  UINTN                   VrlSize;
  BOOLEAN                 SignedOs;

  UINTN CmdIndex;
  
  InitOsip();
  
  //
  // TODO: At this time, the SS FV is hardcoded to OSII[0]
  //
  //if (ImageNumber == 0) return EFI_INVALID_PARAMETER;

  //
  // Bounds check ImageNumber against info in OSIP
  //
  if (ImageNumber >= mOsip->NumberOfPointers) return EFI_INVALID_PARAMETER;

  //
  // Load the OS image into a scratch buffer
  //
  // TODO: Make sure that attributes indicate that it is a
  //       OS image.
  //
  Status = LoadImageByNumber(ImageNumber, &osBase, &osPages);
  DEBUG ((EFI_D_ERROR, "Load from EMMC succeed.\n"));	
  if (EFI_ERROR(Status)) 
  	return Status;

  //VrlSize = GetVrlSize ((VOID*)(UINTN) osBase);
  
  SignedOs = ((mOsip->Osii[ImageNumber].Attributes & 1) == 0) ? TRUE : FALSE;
  
  if (SignedOs) {
  	VrlSize = 0x1e0;
  } else {
  	VrlSize = 0;
  }	
  //
  // Allocate pages at load address
  //
  //loadPage  = (EFI_PHYSICAL_ADDRESS) (mOsip->Osii[ImageNumber].LoadAddress + VrlSize) & ~EFI_PAGE_MASK;
//  Status    = gBS->AllocatePages(
//                        AllocateAddress,
//                        EfiBootServicesCode,
//                        osPages + 1,
//                        &loadPage);
//  ASSERT_EFI_ERROR (Status);
//  if (EFI_ERROR(Status)) {
//    Status = EFI_OUT_OF_RESOURCES;
//    goto LOICleanup;
//  }

  //
  // Move the image to its preferred address
  //
  *ImageBase  = (EFI_PHYSICAL_ADDRESS) (mOsip->Osii[ImageNumber].LoadAddress);
  *ImageSize  = GetImageSizeByNumber(ImageNumber) - VrlSize;
  *EntryPoint = (EFI_PHYSICAL_ADDRESS) mOsip->Osii[ImageNumber].EntryPoint;
  RebaseImage(*ImageBase, osBase + VrlSize, *ImageSize);
  
  //
  // Update console.
  //
  // Log level
  *(UINT8 *)((UINTN)*ImageBase + 0x41) = 0x38;
  
  // "androidboot.wakesrc=00 androidboot.mode=main" from offset 0xf0
//  UINT8 CommandLinePatch[] = {0x20, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x62, 0x6f, 0x6f, 0x74, 0x2e, 0x77, 0x61, 0x6b,\
//  	                          0x65, 0x73, 0x72, 0x63, 0x3d, 0x30, 0x30, 0x20, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x62,\
//  	                          0x6f, 0x6f, 0x74, 0x2e, 0x6d, 0x6f, 0x64, 0x65, 0x3d, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x0a, 0x00};
//  UINTN CmdIndex;
  for(CmdIndex = 0; CmdIndex<48;CmdIndex++) {
    *((UINT8 *)((UINTN)*ImageBase + 0xf0 + CmdIndex )) = CommandLinePatch[CmdIndex]; 
  }
  //UpdateOsCommandLine();

//LOICleanup:
  //
  // Done with temporary buffer
  //
  gBS->FreePages(osBase, osPages);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
OsipLoadOsImage (
  IN    UINTN                           ImageNumber,
  OUT   EFI_PHYSICAL_ADDRESS            *ImageBase,
  OUT   UINTN                           *ImageSize,
  OUT   EFI_PHYSICAL_ADDRESS            *EntryPoint
  )
{
	EFI_STATUS                       Status;
	VOID                                 *FileBuffer;
	UINTN                                FileSize;
	UINTN                   VrlSize;
 	BOOLEAN                 SignedOs;
	UINTN CmdIndex;

       //
       // Find boot.bin
       //
       FileBuffer = NULL;
	FileSize = 0;
	Status = LoadFileFromFileSystem (L"\\boot.bin", &FileBuffer, &FileSize);
	if (EFI_ERROR (Status)) 
		return  EFI_NOT_FOUND;

	mOsip = (OSIP_HEADER *) (UINTN) FileBuffer;
	
	if (mOsip == NULL) 
		return  EFI_NOT_FOUND;
		
	ValidateOsip(mOsip, 1);

	SignedOs = ((mOsip->Osii[ImageNumber].Attributes & 1) == 0) ? TRUE : FALSE;  
	if (SignedOs) {
		VrlSize = 0x1e0;
	} else {
		VrlSize = 0;
	}	

	*ImageBase  = (EFI_PHYSICAL_ADDRESS) (mOsip->Osii[ImageNumber].LoadAddress);
  	*ImageSize  = GetImageSizeByNumber(ImageNumber) - VrlSize;
  	*EntryPoint = (EFI_PHYSICAL_ADDRESS) mOsip->Osii[ImageNumber].EntryPoint;
  	RebaseImage(*ImageBase,  (EFI_PHYSICAL_ADDRESS)(UINTN) ((UINT8 *)FileBuffer + 512 + VrlSize), *ImageSize);

	//
	// Update console.
	//
	// Log level
	*(UINT8 *)((UINTN)*ImageBase + 0x41) = 0x37;

	// "androidboot.wakesrc=00 androidboot.mode=main" from offset 0xf0
	//  UINT8 CommandLinePatch[] = {0x20, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x62, 0x6f, 0x6f, 0x74, 0x2e, 0x77, 0x61, 0x6b,\
	//  	                          0x65, 0x73, 0x72, 0x63, 0x3d, 0x30, 0x30, 0x20, 0x61, 0x6e, 0x64, 0x72, 0x6f, 0x69, 0x64, 0x62,\
	//  	                          0x6f, 0x6f, 0x74, 0x2e, 0x6d, 0x6f, 0x64, 0x65, 0x3d, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x0a, 0x00};
       
	for(CmdIndex = 0; CmdIndex<48;CmdIndex++) {
	*((UINT8 *)((UINTN)*ImageBase + 0xf0 + CmdIndex )) = CommandLinePatch[CmdIndex]; 
	}

	FreePool (FileBuffer);

	return EFI_SUCCESS;
}

