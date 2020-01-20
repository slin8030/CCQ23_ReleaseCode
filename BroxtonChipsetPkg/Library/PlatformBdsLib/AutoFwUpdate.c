/** @file
;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/** @file
  Copyright(c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#include <Uefi.h>
#include <Protocol/Spi.h>
#include <Library/PcdLib.h>
#include <Library/ShellLib.h>
#include <LIbrary/DebugLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/FileHandleLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseCryptLib.h>

#include <Guid/Capsule.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SeCOperation.h>
#include <Guid/Gpt.h>
#include <Guid/FileSystemInfo.h>

#include <Protocol/GraphicsOutput.h>
#include <Library/GenericBdsLib.h>
#include "BdsPlatform.h"

#include <HeciRegs.h>
#include <SeCAccess.h>
#include <Library/HeciMsgLib.h>
#include <Library/PciLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PmicLib.h>
#include <Protocol/TcoReset.h>
#include <Guid/IntelFotaCapuleGuid.h>
#include <Guid/EfiSystemResourceTable.h>
#include <Library/PrintLib.h>
#include <ChipsetFota.h>
#include <Library/DfuCapsuleLib.h>

#define PMIC_REG_PBCONFIG      0x26
#define MASK_PBCONFIG_FCOT     0x0f  // last 4 bits corresponds to FCOT
#define MASK_PMIC_PB_DISABLE   0xf0  // disable power button

#define MAX_STRING_LENGTH      128

typedef struct {
  CHAR16    *FileName;
  CHAR16    *String;
} FILE_NAME_TO_STRING;


#define Capsule_Image  L"\\INTEL.bmp"

#pragma pack(1)

typedef struct {
  UINT8   Blue;
  UINT8   Green;
  UINT8   Red;
  UINT8   Reserved;
} BMP_COLOR_MAP;

typedef struct {
  CHAR8         CharB;
  CHAR8         CharM;
  UINT32        Size;
  UINT16        Reserved[2];
  UINT32        ImageOffset;
  UINT32        HeaderSize;
  UINT32        PixelWidth;
  UINT32        PixelHeight;
  UINT16        Planes;          ///< Must be 1
  UINT16        BitPerPixel;     ///< 1, 4, 8, or 24
  UINT32        CompressionType;
  UINT32        ImageSize;       ///< Compressed image size in bytes
  UINT32        XPixelsPerMeter;
  UINT32        YPixelsPerMeter;
  UINT32        NumberOfColors;
  UINT32        ImportantColors;
} BMP_IMAGE_HEADER;

typedef struct {
  UINT8    Version;
  UINT8    CheckSum;
  UINT8    ImageType;
  UINT8    Reserved;
  UINT32   VideoMode;
  UINT32   ImageOffsetX;
  UINT32   ImageOffsetY;
 } DISPLAY_CAPSULE_PAYLOAD;

#pragma pack()


FILE_NAME_TO_STRING mBitMapFileNames[] = {
  { Capsule_Image }, 
};




EFI_GUID SystemFirmwareGuid = { 0xB122A262, 0x3551, 0x4f48, { 0x88, 0x92, 0x55, 0xf6, 0xc0, 0x61, 0x42, 0x90 }};
EFI_GUID BiosCapsuleFromAfuGuid = { 0xCD193840, 0x2881, 0x9567, { 0x39, 0x28, 0x38, 0xc5, 0x97, 0x53, 0x49, 0x77 }};

#define SECTOR_SIZE_4KB      0x1000      // Common 4K Bytes sector size
//#define MAX_FWH_SIZE         0x00200000  // 8Mbit (Note that this can also be used for the 4Mbit )
#define IFWI_SIZE            (0x00800000)
#define IBB_SIZE             (0x20000)
#define BIOS_MANIFEST_SIZE   (0x400)
#define BIOS_MANIFEST_OFFSET (IFWI_SIZE - IBB_SIZE)
#define BIOS_STAGE_I_SIZE    (IBB_SIZE - 0x400)
#define BIOS_STAGE_I_OFFSET  (IFWI_SIZE - BIOS_STAGE_I_SIZE)

//
// Prefix Opcode Index on the host SPI controller
//
typedef enum {
  SPI_WREN,             // Prefix Opcode 0: Write Enable
  SPI_EWSR,             // Prefix Opcode 1: Enable Write Status Register
} PREFIX_OPCODE_INDEX;

//
// Opcode Menu Index on the host SPI controller
//
typedef enum {
  SPI_READ_ID,        // Opcode 0: READ ID, Read cycle with addressary
  SPI_READ,           // Opcode 1: READ, Read cycle with address
  SPI_RDSR,           // Opcode 2: Read Status Register, No address
  SPI_WRDI,           // Opcode 3: Write Disable, No address
  SPI_SERASE,         // Opcode 4: Sector Erase (4KB), Write cycle with address
  SPI_BERASE,         // Opcode 5: Block Erase (32KB), Write cycle with address
  SPI_PROG,           // Opcode 6: Byte Program, Write cycle with address
  SPI_WRSR,           // Opcode 7: Write Status Register, No address
} SPI_OPCODE_INDEX;

//
// Serial Flash Status Register definitions
//
#define SF_SR_BUSY        0x01      // Indicates if internal write operation is in progress
#define SF_SR_WEL         0x02      // Indicates if device is memory write enabled
#define SF_SR_BP0         0x04      // Block protection bit 0
#define SF_SR_BP1         0x08      // Block protection bit 1
#define SF_SR_BP2         0x10      // Block protection bit 2
#define SF_SR_BP3         0x20      // Block protection bit 3
#define SF_SR_WPE         0x3C      // Enable write protection on all blocks
#define SF_SR_AAI         0x40      // Auto Address Increment Programming status
#define SF_SR_BPL         0x80      // Block protection lock-down

typedef enum {
  IFWI_TYPE_CAPSULE_NO_HEADER,
  IFWI_TYPE_FULL_SPI_IMAGE,
  IFWI_TYPE_INVALID,
} IFWI_TYPE_INDEX;

EFI_SPI_PROTOCOL  *mSpiProtocol = NULL;

UINT8 mFotaProcessData;

//
// FotaFwUpdate.h
//
typedef struct _FPTHdrV2 {
  UINT32 ReservedQ[4];    //4 DWORD filled with all 0s
  UINT32 Signature;       //$FPT
  UINT32 NumFptEntries;   //Number of FPT entries
  UINT8  HdrVer;
  UINT8  EntryVer;
  UINT8  HdrLen;
  UINT8  CheckSum;
  UINT16 FlashLifeTime;
  UINT16 FlashCycleLmt;
  UINT32 DRAMSize;
  UINT32 FPTFlags;
  UINT32 ReservedS[2];    //2 DWORD reserved.
} FPTHdrV2;

typedef struct _FPTEntryHdrV1 {
  UINT32 PartitionName;   //Unique name of the partition
  UINT32 PartitionOwner;
  UINT32 Offset;          //Offset of the partition from beginning of FPT
  UINT32 Length;          //Length of the partition
  UINT32 TokensOnStart;
  UINT32 MaximumTokens;
  UINT32 ScratchSectors;
  UINT32 Attributes;
} FPTEntryHdrV1;

#define  TXE_UPD_SIZE       0x300000
#define  SECURE_FLASH_SIZE  0x0C10

UINT8   *mPartitionBuffer;
UINT8    mAfuCapsuleFlag;

extern EFI_GUID gWindowsUxCapsuleGuid;

extern BOOLEAN
DisplayWindowsUxCapsule (
  VOID
  );

EFI_STATUS
DisablePowerButton(
  OUT UINT32 *RcrbCfg,
  OUT UINT8  *ButtonHoldTime
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32     RcrbValue;
  EFI_TCO_RESET_PROTOCOL *TcoResetProtocol;

  RcrbValue = 0;
  if(RcrbCfg == NULL || ButtonHoldTime == NULL){
    return EFI_INVALID_PARAMETER;
  }
  Status = gBS->LocateProtocol(&gEfiTcoResetProtocolGuid, NULL, (VOID**)&TcoResetProtocol);

  if(EFI_ERROR(Status)){
     DEBUG((EFI_D_ERROR, "Failed to locate TCO_RESET protocol.\n"));
     return Status;
  }

  TcoResetProtocol->EnableTcoReset(&RcrbValue);

  *RcrbCfg = RcrbValue;
  
  //
  //Disable power button. 
  //
  *ButtonHoldTime = PmicRead8((UINT8)PMIC_REG_PBCONFIG);
  Status = PmicWrite8((UINT8)PMIC_REG_PBCONFIG, (*ButtonHoldTime)&MASK_PMIC_PB_DISABLE);
  
  if(EFI_ERROR(Status)){
    DEBUG((EFI_D_ERROR, "Failed to disable power button.\n"));
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EnablePowerButton(
  IN UINT32 RcrbCfg,
  IN UINT8  ButtonHoldTime
  )
{
  EFI_STATUS Status  = EFI_SUCCESS;
  UINT8      FcotVal = 0;
  EFI_TCO_RESET_PROTOCOL *TcoResetProtocol;

  //
  //Enable power button. Make sure the Button hold time passed in is not 0x20.
  //As it would keep the system at un-attended state.Force to 4 seconds if this happens
  //
  if( (ButtonHoldTime&MASK_PBCONFIG_FCOT) == 0){
    ButtonHoldTime &= 0xF3;
  }
  
  Status = PmicWrite8((UINT8)PMIC_REG_PBCONFIG, ButtonHoldTime);
  if(EFI_ERROR(Status)){
    //
    //In case we failed to enable power button, keep TCO timer enabled so we're safe not to leave system as dead fish.
    //
    return Status;
  }

  FcotVal = PmicRead8((UINT8)PMIC_REG_PBCONFIG);
  if( (FcotVal&0xf) !=0){
    Status = gBS->LocateProtocol(&gEfiTcoResetProtocolGuid, NULL, (VOID**)&TcoResetProtocol);

    if(EFI_ERROR(Status)){
      DEBUG((EFI_D_ERROR, "Failed to locate TCO_RESET protocol.\n"));
      return Status;
    }

    TcoResetProtocol->DisableTcoReset(RcrbCfg);
    return EFI_SUCCESS;
  }else{
    return EFI_NOT_READY;
  }

}
 
 
/**
  The firmware implements to process the capsule image for FOTA.
 
  @param  CapsuleHeader         Points to a capsule header.
 
  @retval EFI_SUCESS            Process Capsule Image successfully.
  @retval EFI_UNSUPPORTED       Capsule image is not supported by the firmware.
  @retval EFI_VOLUME_CORRUPTED  FV volume in the capsule is corrupted.
  @retval EFI_OUT_OF_RESOURCES  Not enough memory.
**/
EFI_STATUS
EFIAPI
IntelFotaProcessCapsuleImage (
  IN EFI_CAPSULE_HEADER *CapsuleHeader
  )
{
  EFI_STATUS                   Status;
  EFI_FIRMWARE_VOLUME_HEADER   *FvImage;
  UINT32                       Length;
  EFI_FIRMWARE_VOLUME_HEADER   *ProcessedFvImage;
  EFI_HANDLE                   FvProtocolHandle;
  UINT32                       FvAlignment;
  UINT32                       RcrbCfg;
  UINT8                        ButtonHoldTime;

  DFU_UPDATE_STATUS            *UpdateStatus = NULL;
 
  FvImage = NULL;
  ProcessedFvImage = NULL;
  Length  = 0;
  RcrbCfg = 0;
  ButtonHoldTime = 0x23;

  //
  //Disable power button now
  //
  Status = DisablePowerButton(&RcrbCfg, &ButtonHoldTime);
  if(EFI_ERROR(Status)){
    DEBUG((EFI_D_ERROR, "Failed to disable power button. aborting.\n"));    
    return Status;
  }
 
  FvImage = (EFI_FIRMWARE_VOLUME_HEADER *) ((UINT8 *) CapsuleHeader + CapsuleHeader->HeaderSize);
  Length  = CapsuleHeader->CapsuleImageSize - CapsuleHeader->HeaderSize;
 
  while (Length != 0) {
    //
    // Point to the next firmware volume header, and then
    // call the DXE service to process it.
    //
    if (FvImage->FvLength > (UINTN) Length) {
      //
      // Notes: need to stuff this status somewhere so that the
      // error can be detected at OS runtime
      //
      Status = EFI_VOLUME_CORRUPTED;
      break;
    }
    
    FvAlignment = 1 << ((FvImage->Attributes & EFI_FVB2_ALIGNMENT) >> 16);
    //
    // FvAlignment must be more than 8 bytes required by FvHeader structure.
    //
    if (FvAlignment < 8) {
      FvAlignment = 8;
    }
    //
    // Check FvImage Align is required.
    //
    if (((UINTN) FvImage % FvAlignment) == 0) {
      ProcessedFvImage = FvImage;
    } else {
      //
      // Allocate new aligned buffer to store FvImage.
      //
      ProcessedFvImage = (EFI_FIRMWARE_VOLUME_HEADER *) AllocateAlignedPages ((UINTN) EFI_SIZE_TO_PAGES ((UINTN) FvImage->FvLength), (UINTN) FvAlignment);
      if (ProcessedFvImage == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        break;
      }
      CopyMem (ProcessedFvImage, FvImage, (UINTN) FvImage->FvLength);
    }
    
    Status = gDS->ProcessFirmwareVolume (
                (VOID *) ProcessedFvImage,
                (UINTN) ProcessedFvImage->FvLength,
                &FvProtocolHandle
                );
    if (EFI_ERROR (Status)) {
      break;
    }

    //
    // Display Windows Update Logo
    //
    DisplayWindowsUxCapsule ();
    //
    // Call the dispatcher to dispatch any drivers from the produced firmware volume
    //
    gDS->Dispatch ();
    //
    // Get DFU ReportStatus
    //
    Status = gBS->LocateProtocol(
                    &gEfiDFUResultGuid,
                    NULL,
                    (VOID **)&UpdateStatus    
                    );
    if(EFI_ERROR(Status)) {
      return Status;
    } else {
      if (UpdateStatus->UpdateStatus != UPDATE_SUCCESS) {
        Status = RETURN_ABORTED;
      }
    }
    //
    // On to the next FV in the capsule
    //
    Length -= (UINT32) FvImage->FvLength;
    FvImage = (EFI_FIRMWARE_VOLUME_HEADER *) ((UINT8 *) FvImage + FvImage->FvLength);
 
  }
  
  //
  //Enable power button again, and disalbe TCO timer.
  //
  EnablePowerButton(RcrbCfg, ButtonHoldTime);
  return Status;
}

EFI_STATUS
AfuConvertBmpToGopBlt (
  IN     VOID      *BmpImage,
  IN     UINTN     BmpImageSize,
  IN OUT VOID      **GopBlt,
  IN OUT UINTN     *GopBltSize,
     OUT UINTN     *PixelHeight,
     OUT UINTN     *PixelWidth
  )
{
  UINT8                         *Image;
  UINT8                         *ImageHeader;
  BMP_IMAGE_HEADER              *BmpHeader;
  BMP_COLOR_MAP                 *BmpColorMap;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt;
  UINT64                        BltBufferSize;
  UINTN                         Index;
  UINTN                         Height;
  UINTN                         Width;
  UINTN                         ImageIndex;
  BOOLEAN                       IsAllocated;

  BmpHeader = (BMP_IMAGE_HEADER *) BmpImage;

  if (BmpHeader->CharB != 'B' || BmpHeader->CharM != 'M') {
    //Print(L"Invalid BMP header.\r\n");
    return EFI_UNSUPPORTED;
  }

  //
  // Doesn't support compress.
  //
  if (BmpHeader->CompressionType != 0) {
   // Print(L"Unsupported compress type.\r\n");
    return EFI_UNSUPPORTED;
  }

  //
  // Calculate Color Map offset in the image.
  //
  Image       = BmpImage;
  BmpColorMap = (BMP_COLOR_MAP *) (Image + sizeof (BMP_IMAGE_HEADER));

  //
  // Calculate graphics image data address in the image
  //
  Image         = ((UINT8 *) BmpImage) + BmpHeader->ImageOffset;
  ImageHeader   = Image;

  //
  // Calculate the BltBuffer needed size.
  //
  BltBufferSize = MultU64x32 ((UINT64) BmpHeader->PixelWidth, BmpHeader->PixelHeight);
  //
  // Ensure the BltBufferSize * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) doesn't overflow
  //
  if (BltBufferSize > DivU64x32 ((UINTN) ~0, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL))) {
      //Print(L"Needed bltbuffer size overflow.\r\n");
      return EFI_UNSUPPORTED;
   }
  BltBufferSize = MultU64x32 (BltBufferSize, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  IsAllocated   = FALSE;
  if (*GopBlt == NULL) {
    //
    // GopBlt is not allocated by caller.
    //
    *GopBltSize = (UINTN) BltBufferSize;
    *GopBlt     = AllocatePool (*GopBltSize);
    IsAllocated = TRUE;
    if (*GopBlt == NULL) {
      //Print(L"Failed to allocate GopBlt buffer when NULL.\r\n");
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    //
    // GopBlt has been allocated by caller.
    //
    if (*GopBltSize < (UINTN) BltBufferSize) {
      *GopBltSize = (UINTN) BltBufferSize;
      //Print(L"GopBlt buffer alloc by caller, but buffer too small.\r\n");
      return EFI_BUFFER_TOO_SMALL;
    }
  }

  *PixelWidth   = BmpHeader->PixelWidth;
  *PixelHeight  = BmpHeader->PixelHeight;

  //
  // Convert image from BMP to Blt buffer format
  //
  BltBuffer = *GopBlt;
  for (Height = 0; Height < BmpHeader->PixelHeight; Height++) {
    Blt = &BltBuffer[(BmpHeader->PixelHeight - Height - 1) * BmpHeader->PixelWidth];
    for (Width = 0; Width < BmpHeader->PixelWidth; Width++, Image++, Blt++) {
      switch (BmpHeader->BitPerPixel) {
      case 1:
        //
        // Convert 1-bit (2 colors) BMP to 24-bit color
        //
        for (Index = 0; Index < 8 && Width < BmpHeader->PixelWidth; Index++) {
          Blt->Red    = BmpColorMap[((*Image) >> (7 - Index)) & 0x1].Red;
          Blt->Green  = BmpColorMap[((*Image) >> (7 - Index)) & 0x1].Green;
          Blt->Blue   = BmpColorMap[((*Image) >> (7 - Index)) & 0x1].Blue;
          Blt++;
          Width++;
        }

        Blt--;
        Width--;
        break;

      case 4:
        //
        // Convert 4-bit (16 colors) BMP Palette to 24-bit color
        //
        Index       = (*Image) >> 4;
        Blt->Red    = BmpColorMap[Index].Red;
        Blt->Green  = BmpColorMap[Index].Green;
        Blt->Blue   = BmpColorMap[Index].Blue;
        if (Width < (BmpHeader->PixelWidth - 1)) {
          Blt++;
          Width++;
          Index       = (*Image) & 0x0f;
          Blt->Red    = BmpColorMap[Index].Red;
          Blt->Green  = BmpColorMap[Index].Green;
          Blt->Blue   = BmpColorMap[Index].Blue;
        }
        break;

      case 8:
        //
        // Convert 8-bit (256 colors) BMP Palette to 24-bit color
        //
        Blt->Red    = BmpColorMap[*Image].Red;
        Blt->Green  = BmpColorMap[*Image].Green;
        Blt->Blue   = BmpColorMap[*Image].Blue;
        break;

      case 24:
        //
        // It is 24-bit BMP.
        //
        Blt->Blue   = *Image++;
        Blt->Green  = *Image++;
        Blt->Red    = *Image;
        break;
      
      case 32:
        //
        // It is 24-bit BMP.
        //
        Blt->Blue   = *Image++;
        Blt->Green  = *Image++;
        Blt->Red    = *Image++;
        break;


      default:
        //
        // Other bit format BMP is not supported.
        //
        if (IsAllocated) {
          FreePool (*GopBlt);
          *GopBlt = NULL;
        }
        //Print(L"Default BMP format not supported.\r\n");
        return EFI_UNSUPPORTED;
        break;
      };

    }

    ImageIndex = (UINTN) (Image - ImageHeader);
    if ((ImageIndex % 4) != 0) {
      //
      // Bmp Image starts each row on a 32-bit boundary!
      //
      Image = Image + (4 - (ImageIndex % 4));
    }
  }
  
  //Print(L"Exiting AfuGopBmpConvert \r\n");
  return EFI_SUCCESS;
}

EFI_STATUS
DisplayCapsuleImage (
 VOID
 )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            *HandleArray;
  UINTN                                 HandleArrayCount,Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Fs;
  EFI_FILE                              *Root,*FileHandle = NULL;
  VOID                                  *FileBuffer = NULL,*ImageBuffer = NULL;
  UINTN                                FileSize =0,ImageSize =0;
  UINT64                                TempFileSize=0;
  INTN                                  DestX = 0,DestY = 0;
  UINTN                                 BltSize = 0,Height = 0,Width = 0;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL         *Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsOutput;
  DISPLAY_CAPSULE_PAYLOAD               *DisplayCapsuleAttr = NULL;
  

  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleArrayCount, &HandleArray);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // For each system partition...
  //
  for (Index = 0; Index < HandleArrayCount; Index++) {
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Fs);
    if (EFI_ERROR (Status)) {
      continue;
    }
    Status = Fs->OpenVolume (Fs, &Root);
    if (EFI_ERROR (Status)) {
      continue;
    }
    Status = Root->Open (Root, &FileHandle, mBitMapFileNames[0].FileName, EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR (Status)) {
      continue;
    }
    Status = FileHandleGetSize (FileHandle, (UINT64*)&TempFileSize);
    if(EFI_ERROR(Status)) {
      return Status ;
    }
     
    FileSize = (UINT32) TempFileSize;

    FileBuffer = AllocateZeroPool(FileSize);
    if(FileBuffer == NULL) {
      return Status ;
    }

    Status = FileHandleRead (FileHandle, &FileSize, FileBuffer);
    if(EFI_ERROR(Status)) {
      FreePool (FileBuffer);
      return Status ;
    }

    Status = FileHandleDelete(FileHandle);
    if(EFI_ERROR(Status)) {
      FreePool (FileBuffer);
      return Status ;
    }
  DisplayCapsuleAttr = ( DISPLAY_CAPSULE_PAYLOAD *)FileBuffer;
  ImageBuffer = (VOID *)((UINTN)DisplayCapsuleAttr + sizeof (DISPLAY_CAPSULE_PAYLOAD));// pointing to start of image
  ImageSize = FileSize - sizeof (DISPLAY_CAPSULE_PAYLOAD);// getting image size

    Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
    
    if (EFI_ERROR (Status)) {
      return EFI_UNSUPPORTED;
    }
    
    if (GraphicsOutput != NULL) {
      GraphicsOutput->Mode->Mode = DisplayCapsuleAttr->VideoMode;
      DestX = DisplayCapsuleAttr->ImageOffsetX;
      DestY = DisplayCapsuleAttr->ImageOffsetY;
    }
 
    Blt = NULL;
    Status = AfuConvertBmpToGopBlt (
              ImageBuffer,
              ImageSize,
              (VOID **) &Blt,
              &BltSize,
              &Height,
              &Width
              );
    if (EFI_ERROR (Status)) {
      FreePool (FileBuffer);
      FileBuffer = NULL;
      return Status;
    }

    if (GraphicsOutput != NULL) {
        Status = GraphicsOutput->Blt (
                            GraphicsOutput,
                            Blt,
                            EfiBltBufferToVideo,
                            0,
                            0,
                            DestX,
                            DestY,
                            Width,
                            Height,
                            0
                            );
        if (EFI_ERROR (Status)) {
          FreePool (FileBuffer);
          FileBuffer = NULL;
          return Status;
        }
   }else {
      Status = EFI_UNSUPPORTED;
   }
   FreePool (FileBuffer);
   FileBuffer = NULL;

    if (Blt != NULL) {
      FreePool (Blt);
    }
 
  }//End of For
  return EFI_SUCCESS;

}

BOOLEAN
FlashReadCompare (
  IN  UINT32 BaseAddress,
  IN  UINT8 *Byte,
  IN  UINTN Length,
  IN  SPI_REGION_TYPE  SpiRegionType
)
{
  EFI_STATUS          Status = EFI_SUCCESS;
  UINT32              SpiAddress;
  UINT8               Buffer[SECTOR_SIZE_4KB];

  SpiAddress = (UINTN)(BaseAddress);
  
  while ( Length > 0 ) {
    Status = mSpiProtocol->Execute (
                  mSpiProtocol,
                  SPI_READ,
                  SPI_WREN,
                  TRUE,
                  TRUE,
                  FALSE,
                  SpiAddress,
                  SECTOR_SIZE_4KB,
                  Buffer,
                  SpiRegionType
                );
    if (EFI_ERROR (Status)) {
      Print(L"Read SPI ROM Failed [0x%08x]\n", SpiAddress); 
      return FALSE;
    }
    SpiAddress += SECTOR_SIZE_4KB;
    Length     -= SECTOR_SIZE_4KB;
    if (CompareMem(Buffer, Byte, SECTOR_SIZE_4KB)) {
      return FALSE;
    }
  }
  return TRUE;
}


EFI_STATUS
AfuFlashErase (
  IN  UINT32  BaseAddress,
  IN  UINTN  NumBytes,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
  EFI_STATUS          Status = EFI_SUCCESS;
  UINT32              SpiAddress;

  SpiAddress = (UINTN)(BaseAddress);
  while ( NumBytes > 0 ) {
    Status = mSpiProtocol->Execute (
                             mSpiProtocol,
                             SPI_SERASE,
                             SPI_WREN,
                             FALSE,
                             TRUE,
                             FALSE,
                             SpiAddress,
                             0,
                             NULL,
                             SpiRegionType
                             );
    if (EFI_ERROR (Status)) {
      break;
    }
    SpiAddress += SECTOR_SIZE_4KB;
    NumBytes   -= SECTOR_SIZE_4KB;
  }

  return Status;
}


EFI_STATUS
FlashWriteData (
  IN  UINT32 DstBufferPtr,
  IN  UINT8 *Byte,
  IN  UINTN Length,
  IN  SPI_REGION_TYPE   SpiRegionType
  )
{
  EFI_STATUS                Status;
  UINT32                    NumBytes = (UINT32)Length;
  UINT8*                    pBuf8 = Byte;
  UINT32                    SpiAddress;

  SpiAddress = (UINTN)(DstBufferPtr);
  Status = mSpiProtocol->Execute (
                           mSpiProtocol,
                           SPI_PROG,
                           SPI_WREN,
                           TRUE,
                           TRUE,
                           TRUE,
                           SpiAddress,
                           NumBytes,
                           pBuf8,
                           SpiRegionType
                           );
  return Status;
}


EFI_STATUS BIOSVerify(
  IN  UINTN            FileSize,
  IN  UINT8            *FileBuffer,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
 UINT32           DataIndex;
 BOOLEAN         Flag = TRUE;   

  for (DataIndex = 0; DataIndex < FileSize; DataIndex += SECTOR_SIZE_4KB) { 
    if(FlashReadCompare(DataIndex, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType)) {
      Print(L"Verifying... %d%% Completed.  \r", (DataIndex * 100 / FileSize));  
      continue;      
    } 
    Print(L"Verifying... %d%% Completed   Error at [%08x].  \n", (DataIndex * 100 / FileSize), DataIndex);  
    Flag = FALSE;
  }
  Print(L"Flash Verify Complete. ");
  if(Flag) {
    Print(L"It's same...!!\n");
  } else {
    Print(L"It's different as show...!!\n");
  }
  return EFI_SUCCESS;
}


EFI_STATUS BIOSFlash(
  IN  UINTN              FileSize,
  IN  UINT8              *FileBuffer,
  IN  SPI_REGION_TYPE    SpiRegionType
  )
{
  UINT32  DataIndex;
  for (DataIndex = 0; DataIndex < FileSize; DataIndex += SECTOR_SIZE_4KB) { 
    if(FlashReadCompare(DataIndex, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType)) {
      Print(L"Updating firmware... %3d%% Completed.  \r", (DataIndex * 100 / FileSize));  
      continue;      
    } 
    AfuFlashErase(DataIndex, SECTOR_SIZE_4KB, SpiRegionType);
    Print(L"Updating firmware... %3d%% Completed.  \r", (DataIndex * 100 / FileSize));  
    FlashWriteData(DataIndex, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType);
  }
  
  return EFI_SUCCESS;  
}

EFI_STATUS Sha1_Hash(UINT8 *Digest, void * sha1buffer, UINT32 sha1Size){
  UINTN    CtxSize;
  VOID     *HashCtx;
  EFI_STATUS Status;
  ZeroMem (Digest, MAX_DIGEST_SIZE);
  CtxSize = Sha1GetContextSize ();
  HashCtx = AllocatePool (CtxSize);
  if(HashCtx == NULL) return EFI_OUT_OF_RESOURCES;
  DEBUG((EFI_D_INFO, "Sha1 Init...\r\n"));
  Status  = Sha1Init (HashCtx);   
  if (!Status) {
    Print (L"Sha1Init fail\r\n");
    goto _sha1_failed;
  }
  Status = Sha1Update (HashCtx, sha1buffer, sha1Size);
  if (!Status) {
    Print (L"Sha1Update fail\r\n");
    goto _sha1_failed;
  }
  DEBUG((EFI_D_INFO, "Sha1 Finalize... \r\n"));
  Status = Sha1Final (HashCtx, Digest);
  if (!Status) {
    Print(L"Sha1Final Fail\r\n");
    goto _sha1_failed;
  }
  FreePool(HashCtx);
  return EFI_SUCCESS; 
_sha1_failed:
  FreePool(HashCtx);
  return EFI_ABORTED;
   
}

EFI_STATUS Sha256_Hash(UINT8 *Digest, void * sha256buffer, UINT32 sha256Size){
  UINTN    CtxSize;
  VOID     *HashCtx;
  EFI_STATUS Status;
  ZeroMem (Digest, MAX_DIGEST_SIZE);
  CtxSize = Sha256GetContextSize ();
  HashCtx = AllocatePool (CtxSize);
  if(HashCtx == NULL) return EFI_OUT_OF_RESOURCES;
  DEBUG((EFI_D_INFO, "Sha256 Init...\r\n"));
  Status  = Sha256Init (HashCtx);   
  if (!Status) {
    Print (L"Sha256 Init fail\r\n");
    goto _sha256_failed;
  }
  DEBUG((EFI_D_INFO, "Sha256 Update...\r\n"));
  Status = Sha256Update (HashCtx, sha256buffer, sha256Size);
  if (!Status) {
    Print (L"Sha256 Update fail\r\n");
    goto _sha256_failed;
  }
  DEBUG((EFI_D_INFO, "Sha256 Finalize... \r\n"));
  Status = Sha256Final (HashCtx, Digest);
  if (!Status) {
    Print(L"Sha256 Final Fail\r\n");
    goto _sha256_failed;
  }
  FreePool(HashCtx);
  return EFI_SUCCESS;  
_sha256_failed:
  FreePool(HashCtx);
  return EFI_ABORTED;
}


EFI_STATUS ImageIntegrityVerify(void *FileBuffer, UINT32 FileSize) {  
  UINT8    Digest[MAX_DIGEST_SIZE];
  EFI_STATUS  Status;
  UINT32   i = 0,match = 0;
  UINT32   IFWISigBaseAddress = 0;
  UINT32   shaoffset = 0, shasize = 0;
  Status = EFI_SUCCESS;
//_verify_PLATFORM_ID:
//_verify_IFWI_: 
  IFWISigBaseAddress = PcdGet32(PcdIFWISigBaseAddress);
  if(!CompareMem((void *)((UINTN)FileBuffer + 0x100000), (void *)"IFWISHA1_01", AsciiStrLen("IFWISHA1_01"))) {
//for rollback compatibility    
    IFWISigBaseAddress = 0x100000;
  }
//
//At PcdIFWISigBaseAddress, there mayn't exist a valid signature.
//AsciiStrnCmp will always search the first null terminated character.
//
  if(!CompareMem((void *)((UINTN)FileBuffer + IFWISigBaseAddress), (void *)"IFWISHA1_01", AsciiStrLen("IFWISHA1_01"))) {
// 32 bytes of signature
// 32 bytes of sha1 hash of 1 st part(0x0~0x100000) + 32 bytes hash of 2nd part(0x100060 ~ 0x800000) 
    shaoffset = 0;
    shasize   = IFWISigBaseAddress;
    DEBUG((EFI_D_INFO, "IFWI Signature: IFWISHA1_01...\r\n"));
    DEBUG((EFI_D_INFO, "Verify the 1st part\r\n"));
    Sha1_Hash(Digest, (void *)((UINTN)FileBuffer + shaoffset), shasize);
    DEBUG((EFI_D_INFO, "Check Value...\r\n"));
    DEBUG((EFI_D_INFO, "Digest:\r\n"));
    for(i = 0; i < SHA1_DIGEST_SIZE ; i ++) {
      if( (i % 16) == 0 ) {
        DEBUG((EFI_D_INFO, "0x%02x: ", i));
      }
      DEBUG((EFI_D_INFO, "%02x ", Digest[i]));
      if( (i % 16) == 15) {
        DEBUG((EFI_D_INFO, "\r\n"));
      }
    }
    DEBUG((EFI_D_INFO, "\r\nHash in file:\r\n"));
    for(i = 0; i < SHA256_DIGEST_SIZE ; i ++) {
      if( (i % 16) == 0 ) {
        DEBUG((EFI_D_INFO, "0x%02x: ", i));
      }
      DEBUG((EFI_D_INFO, "%02x ", ((UINT8 *)((UINTN)FileBuffer + IFWISigBaseAddress + 0x20))[i]));
      if( (i % 16) == 15) {
        DEBUG((EFI_D_INFO, "\r\n"));
      }
    }
    match = 1;
    for(i = 0; i < SHA1_DIGEST_SIZE ; i++) {      
      if(((UINT8 *)((UINTN)FileBuffer + IFWISigBaseAddress + 0x20))[i] !=  Digest[SHA1_DIGEST_SIZE - i]) {
        match = 0;
        break;
      }
    }
    if( match == 0) {
      match = 1;
      for(i = 0; i < SHA1_DIGEST_SIZE ; i++) {      
        if(((UINT8 *)((UINTN)FileBuffer + IFWISigBaseAddress + 0x20))[i] !=  Digest[i]) {
          match = 0;
          break;
        }
      }
    } 
    if(match == 0) {
      Print(L"1st part verification failed\r\n");
      return EFI_ABORTED;
    }
    DEBUG((EFI_D_INFO, "1st part verify success\r\n"));

    shaoffset = IFWISigBaseAddress + 0x60;
    shasize   = IFWI_SIZE - shaoffset;
    DEBUG((EFI_D_INFO, "Verify the 2nd part\r\n"));
    Sha1_Hash(Digest, (void *)((UINTN)FileBuffer + shaoffset), shasize);
    DEBUG((EFI_D_INFO, "Check Value...\r\n"));
    DEBUG((EFI_D_INFO, "Digest:\r\n"));
    for(i = 0; i < SHA1_DIGEST_SIZE ; i ++) {
      if( (i % 16) == 0 ) {
        DEBUG((EFI_D_INFO, "0x%02x: ", i));
      }
      DEBUG((EFI_D_INFO, "%02x ", Digest[i]));
      if( (i % 16) == 15) {
        DEBUG((EFI_D_INFO, "\r\n"));
      }
    }
    DEBUG((EFI_D_INFO, "\r\nHash in file:\r\n"));
    for(i = 0; i < SHA256_DIGEST_SIZE ; i ++) {
      if( (i % 16) == 0 ) {
        DEBUG((EFI_D_INFO, "0x%02x: ", i));
      }
      DEBUG((EFI_D_INFO, "%02x ", ((UINT8 *)((UINTN)FileBuffer + IFWISigBaseAddress + 0x40))[i]));
      if( (i % 16) == 15) {
        DEBUG((EFI_D_INFO, "\r\n"));
      }
    }
    match = 1;
    for(i = 0; i < SHA1_DIGEST_SIZE ; i++) {      
      if(((UINT8 *)((UINTN)FileBuffer + IFWISigBaseAddress + 0x40))[i] !=  Digest[SHA1_DIGEST_SIZE - i]) {
        match = 0;
        break;
      }
    }
    if( match == 0) {
      match = 1;
      for(i = 0; i < SHA1_DIGEST_SIZE ; i++) {      
        if(((UINT8 *)((UINTN)FileBuffer + IFWISigBaseAddress + 0x40))[i] !=  Digest[i]) {
          match = 0;
          break;
        }
      }
    } 
    if(match == 0) {
      Print(L"2nd part verification failed\r\n");
      return EFI_ABORTED;
    }
    return EFI_SUCCESS;
  } else {
    Print(L"No valid IFWI signature found\r\n");
    return EFI_ABORTED;
  }
#if 0  
  DEBUG((EFI_D_INFO, "IFWI verify success\r\n"));  
  DEBUG((EFI_D_INFO, "Public key verify skipped, success\r\n"));
  DEBUG((EFI_D_INFO, "Manifest RSA Verify skipped, success\r\n"));
  DEBUG((EFI_D_INFO, "Verify BIOS Stage I\r\n"));   
  Status  = Sha256_Hash (Digest, (void *)((UINTN)FileBuffer + BIOS_STAGE_I_OFFSET), BIOS_STAGE_I_SIZE);
  if (EFI_ERROR(Status)) {
    Print (L"Sha256_Hash fail\r\n");
    return EFI_ABORTED;
  }

  DEBUG((EFI_D_INFO, "Check Value...\r\n"));
  DEBUG((EFI_D_INFO, "Digest:\r\n"));
  for(i = 0; i < SHA256_DIGEST_SIZE ; i ++) {
    if( (i % 16) == 0 ) {
      DEBUG((EFI_D_INFO, "0x%02x: ", i));
    }
    DEBUG((EFI_D_INFO, "%02x ", Digest[i]));
    if( (i % 16) == 15) {
      DEBUG((EFI_D_INFO, "\r\n"));
    }
  }
  DEBUG((EFI_D_INFO, "\r\nManifest hash:\r\n"));
  for(i = 0; i < SHA256_DIGEST_SIZE ; i ++) {
    if( (i % 16) == 0 ) {
      DEBUG((EFI_D_INFO, "0x%02x: ", i));
    }
    DEBUG((EFI_D_INFO, "%02x ", ((UINT8 *)((UINTN)FileBuffer + BIOS_MANIFEST_OFFSET + 20))[i]));
    if( (i % 16) == 15) {
      DEBUG((EFI_D_INFO, "\r\n"));
    }
  }
  match = 1;
  for(i = 0; i < SHA256_DIGEST_SIZE ; i ++) {
    if(((UINT8 *)((UINTN)FileBuffer + BIOS_MANIFEST_OFFSET + 20))[i] !=  Digest[SHA256_DIGEST_SIZE - i - 1]) {
      DEBUG((EFI_D_INFO, "Sha256 Digest compare fail1\r\n"));
      match = 0;
      break;;
    } 
  }
  if(match == 0) {
    match = 1;
    for(i = 0; i < SHA256_DIGEST_SIZE ; i ++) {
      if(((UINT8 *)((UINTN)FileBuffer + BIOS_MANIFEST_OFFSET + 20))[i] !=  Digest[i]) {
        DEBUG((EFI_D_INFO, "Sha256 Digest compare fail2\r\n"));
        match = 0;
        break;;
      } 
    }
  }
  return match == 0? EFI_ABORTED: EFI_SUCCESS;
#endif  
}

EFI_STATUS
ExtractCodePartitions(
  IN   UINT8     *FileBuffer,
  IN   UINT32     SecUpdSize,
  OUT  UINT32    *DestSize,
  OUT  UINT8     *ReuseSrcBuf,
  OUT  VERSION   *SecVersion
  )
{
  EFI_STATUS      Status;
  FPTHdrV2       *TxeHeader;
  UINT32          NumParts;
  UINT32          PartIdx;
  UINT8          *BufPtr;
  FPTEntryHdrV1  *FptEntry;
  UINT32          FtprEnd;
  UINT32          FtprStart;
  UINT32          NftpEnd;
  UINT32          NftpStart;
  UINT32          MdmvEnd;
  UINT32          MdmvStart;

  Status          = EFI_SUCCESS;
  NumParts        = 0;
  PartIdx         = 0;
  BufPtr          = FileBuffer;
  *DestSize       = 0;
  *ReuseSrcBuf    = 1;
  FtprEnd         = 0;
  NftpEnd         = 0;
  MdmvEnd         = 0;
  FtprStart       = 0;
  NftpStart       = 0;
  MdmvStart       = 0;

  //
  //TXE UPD size must not exceed 0x300000
  //
  if (SecUpdSize > TXE_UPD_SIZE) {
    Print (L"Invalid UPD size. Aborting.\r\n");
    return EFI_ABORTED;
  }

  TxeHeader = (FPTHdrV2*)FileBuffer;
  if (TxeHeader->Signature != SIGNATURE_32 ('$','F','P','T')) {
    Print (L"Invalid FPT header. Aborting.\r\n");
    return EFI_ABORTED;
  }
  NumParts = TxeHeader->NumFptEntries;
  if (NumParts < 2) {
    Print (L"Not enough partitions for the update.\r\n");
    return EFI_ABORTED;
  }
  //
  //Parse the FPT entries, and find FTPR, NFTP and MDMV partitions, and check if they're continuous blocks
  //
  BufPtr = FileBuffer + TxeHeader->HdrLen;

  for (PartIdx = 0; PartIdx < NumParts; PartIdx++) {
    FptEntry = (FPTEntryHdrV1*)BufPtr;
    if (FptEntry->PartitionName == SIGNATURE_32 ('F','T','P','R')) {
      FtprStart = FptEntry->Offset;
      if(FtprStart > SecUpdSize || FptEntry->Length > SecUpdSize) {
        return EFI_ABORTED;
      }
      mPartitionBuffer += FptEntry->Offset;
      *DestSize += FptEntry->Length;
      FtprEnd = FptEntry->Offset + FptEntry->Length -1;
      Print (L"FTPR Size:%x.\r\n", *DestSize);
    } else if (FptEntry->PartitionName == SIGNATURE_32 ('N','F','T','P')) {
      NftpStart = FptEntry->Offset;
      if (NftpStart > SecUpdSize || FptEntry->Length > SecUpdSize) {
        return EFI_ABORTED;
      }
      NftpEnd = FptEntry->Offset + FptEntry->Length -1;
      *DestSize += FptEntry->Length;
      Print (L"NFTP Size: %x\n", FptEntry->Length);
    } else if (FptEntry->PartitionName == SIGNATURE_32 ('M','D','M','V')) {
      MdmvStart = FptEntry->Offset;
      if (MdmvStart > SecUpdSize || FptEntry->Length > SecUpdSize) {
        return EFI_ABORTED;
      }
      MdmvEnd = FptEntry->Offset + FptEntry->Length -1;
      *DestSize += FptEntry->Length;
      Print (L"MDMV Size: %x\n", FptEntry->Length);
    }
    BufPtr += sizeof (FPTEntryHdrV1);
  }
  if ((FtprStart < NftpStart) && (NftpStart < MdmvStart) && (FtprEnd + 1 == NftpStart) && (NftpEnd + 1 == MdmvStart)) {
    //
    //No need to copy buffer. Just point to the start of FTPR and provide the length.
    //
    Print (L"Reuse buffer allocated previously. Buffer size:%x, offset:%x\r\n", *DestSize, FtprStart);
    SecVersion->Major = *(UINT16*)(mPartitionBuffer + 0x24);
    SecVersion->Minor = *(UINT16*)(mPartitionBuffer + 0x26);
    SecVersion->Hotfix= *(UINT16*)(mPartitionBuffer + 0x28);
    SecVersion->Build = *(UINT16*)(mPartitionBuffer + 0x2A);
    return EFI_SUCCESS;
  } else {
    return EFI_ABORTED;
  }
}

EFI_STATUS
HeciTxeUpdate (
  EFI_CAPSULE_HEADER  *CapsuleHeader
  )
{
  EFI_STATUS   Status;
  UINT8        ReuseSrcBuf;
  UINT8        SecAddress;  
  UINT8        InProgress;  
  UINT8        LoopCount;
  UINT32       PartitionsSize;
  UINT32       RuleData;
  UINT32       MaxBufferSize;   
  UINT32       ResetType;
  VERSION      NewSecVersion;
  OEM_UUID     SecOemId;
  FPTHdrV2    *TxeHeader;
  //
  //TXE firmware update progress variables
  //
  UINT32       Percentage;
  UINT32       CurrentStage;
  UINT32       TotalStages;
  UINT32       LastUpdateStatus;
  UINT32       LastResetType;

  UINT32       SpiBase;
  UINT32       SecRegionBase;

  mPartitionBuffer = NULL;
  
  Status           = EFI_NOT_FOUND;
  ReuseSrcBuf      = 0;
  SecAddress       = 0;  
  InProgress       = 0;  
  LoopCount        = 0;
  PartitionsSize   = 0;
  RuleData         = 0;
  MaxBufferSize    = 0;
  ResetType        = 4;    // FotaFwUpdateCapsule.c  
  TxeHeader        = NULL;

  Percentage       = 0;
  CurrentStage     = 0;
  TotalStages      = 0;
  LastUpdateStatus = 0;
  LastResetType    = 0;

  SpiBase          = MmioRead32 (
                       MmPciAddress (0,
                         DEFAULT_PCI_BUS_NUMBER_SC,
                         PCI_DEVICE_NUMBER_SPI,
                         PCI_FUNCTION_NUMBER_SPI,
                         R_SPI_BASE
                       )
                     ) & B_SPI_BASE_BAR;
  
  SecRegionBase    = (MmioRead32 (SpiBase + R_SPI_FREG2_SEC) & B_SPI_FREG2_BASE_MASK) << 12;

  //
  // Return if Capsule contains only BIOS region
  //
  if ((UINTN)CapsuleHeader->CapsuleImageSize <= PcdGet32 (PcdFlashAreaSize)) {
    return Status;
  }
  //
  // Check signature to find TXE region.
  //
  TxeHeader = (FPTHdrV2*)((UINT8*)CapsuleHeader + CapsuleHeader->HeaderSize + SECURE_FLASH_SIZE + SecRegionBase);
  if (TxeHeader->Signature == SIGNATURE_32 ('$','F','P','T')) {
    mPartitionBuffer = (UINT8*)TxeHeader;
  } else {
    Print (L"FPT not found\n");
    return Status;
  }    

  Status = ExtractCodePartitions (mPartitionBuffer, TXE_UPD_SIZE, &PartitionsSize, &ReuseSrcBuf, &NewSecVersion);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Print (L"\n!!!Updating system firmware. DO NOT UNPLUG POWER or RESET SYSTEM!!!\r\n\n");

  Status = HeciGetLocalFwUpdate (&RuleData);
  if ((EFI_ERROR (Status)) || (RuleData != 1)) {
    return Status;
  }

  Status = HeciConnectFwuInterface (&SecAddress, &MaxBufferSize);
  if (EFI_ERROR (Status)) {
    return Status;
  }
 
  Status = HeciSendFwuGetOemIdMsg (&SecOemId,SecAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HeciVerifyOemId (&SecOemId, SecAddress);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = HeciSendFwuStartMsg (PartitionsSize, &SecOemId,SecAddress, MaxBufferSize);
  if (EFI_ERROR (Status)) {
    Print (L"Failed to send FwuStart message to TXE.\n");
    Status = HeciCheckFwuInProgress (&InProgress);
    Print (L"FWU progress check after FWU start message. In progress: %d.\n", InProgress);
    return Status;
  }

  Status = HeciSendFwuDataMsg (mPartitionBuffer, PartitionsSize, SecAddress, MaxBufferSize);
  if (EFI_ERROR (Status)) {
    Print (L"Failed to send Fwu Data messag to TXE.\n");
    return Status;
  }

  Status = HeciSendFwuEndMsg (&ResetType,SecAddress);
  if (EFI_ERROR (Status)) {
    Print (L"Timeout waiting for FWU_END reply. Continue to query the status for 90 seconds\n");
  }

  Status = HeciDisconnectFwuInterface (SecAddress,MaxBufferSize);
  if (EFI_ERROR (Status)) {
    Print (L"Failed to disconnect from TXE.\r\n");
  }
  //
  //In theory, the update won't take longer than 90 seconds~
  //
  do {
    Status = HeciFwuQueryUpdateStatus (&Percentage, &CurrentStage, &TotalStages, &LastUpdateStatus, &LastResetType, &InProgress);
    if (EFI_ERROR (Status)) {
      Print (L"Query update status failed.\r\n");
      return Status;
    }

    Print (L"Fw update in progress- Percentage:%d%%, Stage:%d/%d, LastUpdateStatus:%d.\r", Percentage, CurrentStage, TotalStages, LastUpdateStatus);
    gBS->Stall (500 * 1000);
    LoopCount++;
  } while (Percentage < 100 && LoopCount < 180);
  
  return Status;
}

EFI_STATUS
GetValidIFWI(void **pBuffer, UINT64 *pSize)
{
  EFI_STATUS Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
  EFI_FILE                        *Root;
  EFI_HANDLE                      *HandleArray;
  UINTN                            HandleArrayCount;
  UINT32                           Index, i;
  EFI_FILE                        *FileHandle;
  UINT64                           FileSize = 0, TmpFileSize = 0;
  VOID                            *FileBuffer  = NULL;
  EFI_CAPSULE_HEADER        *CapsuleHeader = NULL;
  EFI_GUID                  CapsuleGuid;
  EFI_CAPSULE_HEADER  *ech[1]; 
  ESRT_STATUS                 EsrtStatus;
  EFI_GUID                    EsrtCapsuleGuid;
  UINTN                       Size;
  CHAR16                      FlashFileName[MAX_STRING_LENGTH];
  EFI_FILE                   *BackupFileHandle;
  VOID                       *BackupFileBuffer;
  CHAR16                     *BackupFileName;
  UINTN                       BackupFileSize;
  UINT8                       CapsuleFromAfu;

  BackupFileHandle = NULL;
  BackupFileBuffer = NULL;
  BackupFileName   = (CHAR16 *)PcdGetPtr (PcdFotaCapsuleUpdateBinary);
  BackupFileSize   = 0;
  CapsuleFromAfu   = CAPSULE_NOT_FOUND;

  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleArrayCount, &HandleArray);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "AFU: LocateHandleBuffer Error, no valid partition found.\r\n"));
    return Status;
  }

  if ( (FeaturePcdGet (PcdFotaManualSupport) && FeaturePcdGet (PcdFotaFeatureSupport) && (mAfuCapsuleFlag == CAPSULE_NOT_FOUND)) || 
       (mFotaProcessData != SPI_DEFAULT) ) {
    Size = 0;
    EsrtCapsuleGuid = SystemFirmwareGuid;
    StrCpy (FlashFileName, (CHAR16 *)PcdGetPtr (PcdFotaCapsuleUpdateBinary));
  } else {
    //
    // Get Capsule Guid and search by Guid name
    //
    Size = sizeof (EFI_GUID);
    Status = gRT->GetVariable (
                    L"CapsuleGuid",
                    &BiosCapsuleFromAfuGuid,
                    NULL,
                    &Size,
                    &EsrtCapsuleGuid
                  );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    UnicodeSPrint ( FlashFileName,
                    MAX_STRING_LENGTH,
                    L"%s\\%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x.bin",
                    PcdGetPtr (PcdCapsuleImageFolder),
                    (UINTN)EsrtCapsuleGuid.Data1,
                    (UINTN)EsrtCapsuleGuid.Data2,
                    (UINTN)EsrtCapsuleGuid.Data3,
                    (UINTN)EsrtCapsuleGuid.Data4[0],
                    (UINTN)EsrtCapsuleGuid.Data4[1],
                    (UINTN)EsrtCapsuleGuid.Data4[2],
                    (UINTN)EsrtCapsuleGuid.Data4[3],
                    (UINTN)EsrtCapsuleGuid.Data4[4],
                    (UINTN)EsrtCapsuleGuid.Data4[5],
                    (UINTN)EsrtCapsuleGuid.Data4[6],
                    (UINTN)EsrtCapsuleGuid.Data4[7]
                    );
  }
  
  //
  //For each system partition, search existence of IFWI.bin
  //
  for(Index = 0; Index < HandleArrayCount; Index++) {
    Status = gBS->HandleProtocol(HandleArray[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "AFU: Open Handle protocol failed\r\n"));
      continue;
    }
    Status = Fs->OpenVolume(Fs, &Root);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "AFU: Open Volume Error.\r\n"));
      continue;
    }    
    for(i = 0; i < sizeof(FlashFileName) / sizeof (FlashFileName); i++) {   
      DEBUG((EFI_D_INFO, "AFU: try file %s\r\n", FlashFileName));
      Status = Root->Open(Root, &FileHandle, FlashFileName, EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ, 0);
      if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_INFO, "AFU: Open IFWI %s failed, try next one\r\n", FlashFileName));
        continue;
      } else {
       // Print(L"AFU: Test IFWI %s \r\n", mFlashFileNames[i].FileName);
      }
      Status = FileHandleGetSize(FileHandle, &FileSize);
      if(EFI_ERROR(Status)) {
        Print(L"AFU: GetSize failed.\r\n");
        continue;
      }
     // Print(L"FileSize is 0x%08x\r\n", FileSize);
      FileBuffer = AllocateZeroPool((UINTN)FileSize);
      if(FileBuffer == NULL) {
        Print(L"AFU: Failed to allocate memory\r\n");
        continue;
      }
      TmpFileSize = FileSize;
      Status = FileHandleRead(FileHandle, (UINTN*)&TmpFileSize, FileBuffer);
      if(EFI_ERROR(Status) || (UINT32)FileSize != (UINT32)TmpFileSize) {
        Print(L"AFU: File read failed.\r\n");
        FreePool(FileBuffer);
        FileBuffer = NULL;
        continue;
      }
      //
      //If current image is BIOSUpdate.FV, then we're in UpdateCapsule path.
      //
      if(i == 0){
        BdsLibConnectAll ();    

        CapsuleGuid = EsrtCapsuleGuid;
        CapsuleHeader = (EFI_CAPSULE_HEADER *)AllocateZeroPool((UINTN)FileSize + sizeof(EFI_CAPSULE_HEADER));
        CapsuleHeader->HeaderSize = sizeof(EFI_CAPSULE_HEADER);
        CapsuleHeader->CapsuleImageSize =(UINT32)((UINTN)( FileSize + CapsuleHeader->HeaderSize));
        CapsuleHeader->Flags = CAPSULE_FLAGS_INITIATE_RESET | CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE;
        CapsuleHeader->CapsuleGuid = CapsuleGuid;
        CopyMem((void *)((UINTN)CapsuleHeader + CapsuleHeader->HeaderSize),FileBuffer, (UINTN)FileSize); 
        ech[0] = CapsuleHeader;

        //
        // If this is first time to execute FOTA, and process is not for updating TXE through Insyde Capsule while TXE lock
        //
        if ((mFotaProcessData == SPI_DEFAULT) && (mAfuCapsuleFlag == CAPSULE_UPDATE)) {
          //
          // Backup system firmware capsule for recovery usage.
          //          
          Status = Root->Open (Root, &BackupFileHandle, BackupFileName, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0);
          if (FileHandle == NULL) {
            DEBUG((EFI_D_INFO, "AFU: Open IFWI %s failed, try next one\r\n", BackupFileName));
            continue;
          }

          BackupFileBuffer = AllocateZeroPool ((UINTN)FileSize);
          if (BackupFileBuffer == NULL) {
            Print(L"AFU: Failed to allocate memory\r\n");
            continue;
          }

          BackupFileSize = (UINTN)FileSize;
          CopyMem (BackupFileBuffer, FileBuffer, BackupFileSize);
    
          Status = FileHandleWrite (BackupFileHandle, &BackupFileSize, BackupFileBuffer);
          if(EFI_ERROR (Status)) {
            if (BackupFileBuffer != NULL) {
              FreePool (BackupFileBuffer);
            }
            continue;
          }

          Status = FileHandleClose (BackupFileHandle);
          if(EFI_ERROR (Status)) {
            if (BackupFileBuffer != NULL) {
              FreePool (BackupFileBuffer);
            }
            continue;
          }

          if (BackupFileBuffer != NULL) {
            FreePool (BackupFileBuffer);
          }
      
        }

        FreePool (FileBuffer);
        FileHandleClose (FileHandle);

        DisplayCapsuleImage();

        if (mAfuCapsuleFlag == CAPSULE_TXE_CHECK) {
          //
          // Display Windows Update Logo
          //
          DisplayWindowsUxCapsule ();          
          Status = HeciTxeUpdate (CapsuleHeader);
          return Status;          
        }

        //
        // Process Capsule FirmwareVolume and Dispatch
        //
        Status = IntelFotaProcessCapsuleImage (ech[0]);        
        if (EFI_ERROR (Status)) {
          EsrtStatus = ESRT_ERROR_UNSUCCESSFUL;
        } else {
          EsrtStatus = ESRT_SUCCESS;
        }
        
        CapsuleFromAfu = CAPSULE_NOT_FOUND;
        gRT->SetVariable(
               L"CapsuleFromAFU",
               &BiosCapsuleFromAfuGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               sizeof (UINT8),
               &CapsuleFromAfu
               );  
        //
        // Set ESRT table
        //
        gRT->SetVariable (
               L"EsrtLastAttemptStatus",
               &EsrtCapsuleGuid,
               EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
               sizeof(UINT32),
               &EsrtStatus
               );
        gRT->SetVariable (
               L"WindowsUxCapsuleDevicePath",
               &gWindowsUxCapsuleGuid,
               EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
               0,
               NULL
               );

        FreePool (CapsuleHeader);
        gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
//        FreePool(CapsuleHeader);
        return Status;
      }

      if(ImageIntegrityVerify(FileBuffer, (UINT32)FileSize) == EFI_SUCCESS) {       
         break;        
      } else {
        FreePool(FileBuffer);
        continue;
      }        
    }
    if(i <  sizeof(FlashFileName) / sizeof (FlashFileName)) {
      Print(L"Find valid IFWI %s\r\n", FlashFileName);
      *pBuffer = FileBuffer;
      *pSize = FileSize;
      return EFI_SUCCESS;
    }
  }
  return EFI_ABORTED;
}

void CleanIFWI()
{
  EFI_STATUS Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
  EFI_FILE                        *Root;
  EFI_HANDLE                      *HandleArray;
  UINTN                            HandleArrayCount;
  UINT32                           Index, i;
  EFI_FILE                        *FileHandle;
  EFI_GUID                         EsrtCapsuleGuid;
  UINTN                            Size;
  CHAR16                           FlashFileName[MAX_STRING_LENGTH];
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleArrayCount, &HandleArray);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "AFU: LocateHandleBuffer Error, no valid partition found.\r\n"));
    return ;
  }

  if (FeaturePcdGet (PcdFotaManualSupport) && FeaturePcdGet (PcdFotaFeatureSupport)) {
    Size = 0;
    EsrtCapsuleGuid = SystemFirmwareGuid;
    StrCpy (FlashFileName, (CHAR16 *)PcdGetPtr (PcdFotaCapsuleUpdateBinary));
  } else {
    Size = sizeof (EFI_GUID);
    Status = gRT->GetVariable (
                    L"CapsuleGuid",
                    &BiosCapsuleFromAfuGuid,
                    NULL,
                    &Size,
                    &EsrtCapsuleGuid
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
    UnicodeSPrint ( FlashFileName,
                    MAX_STRING_LENGTH,
                    L"%s\\%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x.bin",
                    PcdGetPtr (PcdCapsuleImageFolder),
                    (UINTN)EsrtCapsuleGuid.Data1,
                    (UINTN)EsrtCapsuleGuid.Data2,
                    (UINTN)EsrtCapsuleGuid.Data3,
                    (UINTN)EsrtCapsuleGuid.Data4[0],
                    (UINTN)EsrtCapsuleGuid.Data4[1],
                    (UINTN)EsrtCapsuleGuid.Data4[2],
                    (UINTN)EsrtCapsuleGuid.Data4[3],
                    (UINTN)EsrtCapsuleGuid.Data4[4],
                    (UINTN)EsrtCapsuleGuid.Data4[5],
                    (UINTN)EsrtCapsuleGuid.Data4[6],
                    (UINTN)EsrtCapsuleGuid.Data4[7]
                   );
  } 

  //
  //For each system partition, search existence of IFWI.bin
  //
  for(Index = 0; Index < HandleArrayCount; Index++) {
    Status = gBS->HandleProtocol(HandleArray[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&Fs);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "AFU: Open Handle protocol failed\r\n"));
      continue;
    }
    Status = Fs->OpenVolume(Fs, &Root);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "AFU: Open Volume Error.\r\n"));
      continue;
    }    
    for(i = 0; i < sizeof(FlashFileName) / sizeof (FlashFileName); i++) {   
      DEBUG((EFI_D_INFO, "AFU: try to delete file %s\r\n", FlashFileName));
      Status = Root->Open(Root, &FileHandle, FlashFileName, EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ, 0);
      if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_INFO, "AFU: Open %s failed, try next one\r\n", FlashFileName));
        continue;
      }
      Status = FileHandleDelete(FileHandle);
    }
  }
  return;
}
  
EFI_STATUS
IFWIUpdateHack(
  IN UINT8 FotaProcess
  )
{
  EFI_STATUS                      Status;  
  
  VOID                            *FileBuffer  = NULL;
  UINT64                          FileSize;
  SPI_REGION_TYPE                 SpiRegionType;

  UINT32                          SecEnabled = 1;
  UINT32                          SecOvrEnabled = 0;
  UINT32                          HmrpfoEnableRequired = 0;

  SEC_OPERATION_PROTOCOL          *SecOp;
  SEC_INFOMATION                  SecInfo;

  UINTN                           Size;
  HECI_FWS_REGISTER               SecFirmwareStatus;

  FileSize = 0;
  FileBuffer = NULL;
  HmrpfoEnableRequired = 0;   //indicate the update process that no hmrpfo enable operation is required.

  mFotaProcessData = FotaProcess;
  Size = sizeof (mAfuCapsuleFlag);
  Status = gRT->GetVariable (
                  L"CapsuleFromAFU",
                  &BiosCapsuleFromAfuGuid,
                  NULL,
                  &Size,
                  &mAfuCapsuleFlag
                  );
  if (EFI_ERROR (Status)) {
    mAfuCapsuleFlag = CAPSULE_NOT_FOUND;
  }
  SecFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);
  if ((SecFirmwareStatus.r.ManufacturingMode != 0) && (mAfuCapsuleFlag == CAPSULE_TXE_CHECK)) {
    mAfuCapsuleFlag = CAPSULE_NOT_FOUND;
    gRT->SetVariable (
           L"CapsuleFromAFU",
           &BiosCapsuleFromAfuGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
           sizeof (UINT8),
           &mAfuCapsuleFlag
           );
    return EFI_UNSUPPORTED;          
  }

  Status = gBS->LocateProtocol(
          &gEfiSeCOperationProtocolGuid,
          NULL,
          &SecOp);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "AFU: SecOperationProtocol couldn't be located\r\n"));
    return Status;
  }
  Status = SecOp->GetPlatformSeCInfo(&SecInfo);
  if(EFI_ERROR(Status)) {
    Print(L"AFU: GetPlatformSecInfo failed\n");
    return Status;
  }
  if( (SecInfo.SeCExist == 0) || (SecInfo.SeCEnable == 0)) {
    SecEnabled = 0;    
  } else if(SecInfo.SeCOpMode == 5) { //hmrfpo enabled.
    SecOvrEnabled = 1;
  } else {
    HmrpfoEnableRequired = 1;
  }    

  Status = GetValidIFWI(&FileBuffer, &FileSize);
  if (mAfuCapsuleFlag == CAPSULE_TXE_CHECK) {
    mAfuCapsuleFlag = CAPSULE_NOT_FOUND;
    gRT->SetVariable(
           L"CapsuleFromAFU",
           &BiosCapsuleFromAfuGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
           sizeof (UINT8),
           &mAfuCapsuleFlag
           );
    goto ErrorExit;
  }
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_INFO, "No valid IFWI found, clean all the files and exit\r\n"));
    CleanIFWI();
    goto ErrorExit;
  }
  // Print(L"Find valid IFWI\r\n");
  
  if(FileSize == 0x800000) {
    SpiRegionType = EnumSpiRegionAll;
  } else {
    Print(L"AFU: image size is incorrect.\r\n");
    CleanIFWI();
    Status = EFI_ABORTED;
    goto ErrorExit;
  }
  
  //
  //If file read succeed and GRST is required, we cannot delete the file.
  //Enable hmrfpo. GRST will be issued and control never returns to IA
  //
  if(HmrpfoEnableRequired == 1) {
    Print(L"AFU: hmrfpo not enabled.. Enable hmrfpo and issue GRST\n");
    SecInfo.HmrfpoEnable = 1;
    //
    //GSRT will be issued once the setting to sec succeed.
    //
    Status = SecOp->SetPlatformSeCInfo(&SecInfo);
    if(EFI_ERROR(Status)) {
      Print(L"AFU: SetPlatformSecInfo failed\n");
      CleanIFWI();
      return Status;
    }
    gBS->Stall(10000000);
    //Shouldn't reach here.
    Print(L"\r\n\r\n\r\n!!!Shouldn't reach here\r\n\r\n\r\n");
    CleanIFWI();
    return Status;
  } 
  
  CleanIFWI();
  Status = gBS->LocateProtocol (
             &gScSpiProtocolGuid,
             NULL,
             (VOID **)&mSpiProtocol
           );
  if(EFI_ERROR(Status)) {
    Print(L"AFU: Failed to locate SPI protocol\r\n");
    goto ErrorExit;
  }  
  
  if(SecEnabled == 0 || SecOvrEnabled == 1) {
    Print(L"Start to update firmware\r\n");
    Status = BIOSFlash((UINTN)FileSize, (UINT8*)(FileBuffer), SpiRegionType);   
    Print(L"Flash Update Complete Status %r. Ready to reset...\n", Status);
    if(FileBuffer != NULL) {
      FreePool(FileBuffer);
    }
    if(EFI_ERROR(Status)) {
    //
    //TODO: At this stage the recovery flow is not yet defined for AFU. 
    //
      
    }    
    gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    CpuDeadLoop();
  } else {      
    Print(L"\r\n\r\nWe should never reach here\r\n");
    gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    CpuDeadLoop();
    return Status;  //We never return;
  }
ErrorExit:
  if (FileBuffer != NULL) {
    FreePool(FileBuffer);
  }
  return Status;
 }

