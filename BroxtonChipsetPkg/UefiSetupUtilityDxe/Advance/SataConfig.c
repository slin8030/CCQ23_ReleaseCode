/** @file

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <SetupUtility.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/IdeControllerInit.h>
#include <Library/MmPciLib.h>
#include <Protocol/PciIo.h>
#include <ScAccess.h>

#define CHARACTER_NUMBER_FOR_VALUE              30
#define _48_BIT_ADDRESS_FEATURE_SET_SUPPORTED   0x0400
#define ATAPI_DEVICE                            0x8000


GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID gSATA[2] = {
  STRING_TOKEN(STR_SATA0_NAME),
  STRING_TOKEN(STR_SATA1_NAME),
};
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID gSOFTPRES[2] = {
  STRING_TOKEN(STR_SATA0_SOFT_PRESERVE_STATUS),
  STRING_TOKEN(STR_SATA1_SOFT_PRESERVE_STATUS),
};

//[-start-160906-IB10860209-remove]//
//GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN SataControllerConnected = FALSE;
//[-end-160906-IB10860209-remove]//



VOID 
SwapEntries (
  IN CHAR8    *Data,
  IN UINT16   Size
  )
{
  UINT16  Index;
  CHAR8   Temp8;

  for (Index = 0; (Index+1) < Size; Index+=2) {
    Temp8           = Data[Index];
    Data[Index]     = Data[Index + 1];
    Data[Index + 1] = Temp8;
  }
}


//[-start-160906-IB10860209-modify]//
UINT8
GetChipsetSataPortSpeed (
  UINTN PortNum
  )
{
  UINT32                      DeviceSpeed;
  UINT32                      IdeAhciBar;
  UINT8                       FunNum;

  DeviceSpeed = 0x01; // generation 1


  FunNum = PCI_FUNCTION_NUMBER_SATA;
  IdeAhciBar = MmioRead32 (
                 MmPciAddress (0,
                   0,
                   PCI_DEVICE_NUMBER_SATA,
                   FunNum,
                   R_SATA_AHCI_BAR
                   )
                 );
  
  IdeAhciBar &= 0xFFFFF800;

  switch (PortNum) {
    case 0:
      DeviceSpeed = *(volatile UINT32 *)(UINTN)(IdeAhciBar + R_SATA_AHCI_P0SSTS);
      break;
    case 1:
      DeviceSpeed = *(volatile UINT32 *)(UINTN)(IdeAhciBar + R_SATA_AHCI_P1SSTS);
      break;
  }

  DeviceSpeed = (UINT8)((DeviceSpeed >> 4) & 0x0F);

  return (UINT8)DeviceSpeed;
}
//[-end-160906-IB10860209-modify]//

VOID
SataDeviceCallBack (
  IN EFI_HII_HANDLE HiiHandle
  )
{
  EFI_STATUS                      Status;
  PCI_DEVICE_PATH                 *PciDevicePath;
  CHAR16                          NewString[100];   // Assume no line strings is longer than 256 bytes.
  CHAR16                          SoftPres[40];
  UINT8                           Index;
  UINTN                           HandleCount;
  EFI_HANDLE                      *HandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePathNode;
  EFI_DISK_INFO_PROTOCOL          *DiskInfo;
  UINT32                          SataPortIndex, IdeChannel;
  EFI_ATA_IDENTIFY_DATA           *IdentifyDriveInfo = NULL;
  UINT32                          BufferSize = 0;
  EFI_STRING_ID                   Token;
  EFI_STRING_ID                   SoftPresStatus;
  CHAR8                           ModelNumber[42];
  UINT64                          NumSectors = 0;
  UINT64                          DriveSizeInBytes = 0;
  UINT64                          RemainderInBytes = 0;
  UINT32                          DriveSizeInGB = 0;
  UINT32                          NumTenthsOfGB = 0;
//[-start-160906-IB10860209-remove]//
//  EFI_PCI_IO_PROTOCOL             *PciIo;
//  UINTN                           Segment;
//  UINTN                           Bus;
//  UINTN                           Device;
//  UINTN                           Function;
//[-end-160906-IB10860209-remove]//
  UINT32                          DeviceSpeed;
  UINTN                           PortNumber;

//[-start-160906-IB10860209-remove]//
//  //
//  // If SATA controller has been connected, just return
//  //
//  if (SataControllerConnected) {
//    return;
//  }
//
//  DEBUG ((DEBUG_INFO, "Update SATA device info\n"));
//
//  Status = gBS->LocateHandleBuffer (
//                  ByProtocol,
//                  &gEfiPciIoProtocolGuid,
//                  NULL,
//                  &HandleCount,
//                  &HandleBuffer
//                  );
//  if (EFI_ERROR(Status)) {
//    HandleCount = 0;
//  }
//
//  for (Index = 0; Index < HandleCount; Index++) {
//    Status = gBS->HandleProtocol (
//                    HandleBuffer[Index],
//                    &gEfiPciIoProtocolGuid,
//                    (VOID *) &PciIo
//                     );
//    ASSERT_EFI_ERROR(Status);
//
//    PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
//    if ((Bus == DEFAULT_PCI_BUS_NUMBER_SC) &&
//        (Device == PCI_DEVICE_NUMBER_SATA) &&
//        (Function == PCI_FUNCTION_NUMBER_SATA)) {
//      gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
//    }
//  }
//
//  if (HandleBuffer) {
//    FreePool (HandleBuffer);
//  }
//
//  //
//  // Indicate SATA controller has been connected
//  //
//  SataControllerConnected = TRUE;
//[-end-160906-IB10860209-remove]//
  
  PciDevicePath = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDiskInfoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) HandleCount = 0;

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID *) &DevicePath
                    );
    ASSERT_EFI_ERROR(Status);

    DevicePathNode = DevicePath;
    while (!IsDevicePathEndType (DevicePathNode)) {
      if ((DevicePathNode->Type == HARDWARE_DEVICE_PATH) &&
          (DevicePathNode->SubType == HW_PCI_DP))
      {
        PciDevicePath = (PCI_DEVICE_PATH *) DevicePathNode;
        break;
      }

      DevicePathNode = NextDevicePathNode (DevicePathNode);
    }

    if (PciDevicePath == NULL) continue;

    if ((PciDevicePath->Device == PCI_DEVICE_NUMBER_SATA) &&
        (PciDevicePath->Function == PCI_FUNCTION_NUMBER_SATA)) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiDiskInfoProtocolGuid,
                      &DiskInfo
                      );
      ASSERT_EFI_ERROR (Status);

      Status = DiskInfo->WhichIde (
                           DiskInfo,
                           &IdeChannel,
                           &SataPortIndex
                           );
      Token = gSATA[IdeChannel];
      SoftPresStatus = gSOFTPRES[IdeChannel];

      IdentifyDriveInfo = AllocatePool(sizeof (EFI_ATAPI_IDENTIFY_DATA));
      ASSERT (IdentifyDriveInfo != NULL);
      if (IdentifyDriveInfo == NULL) {
        return;
      }
      ZeroMem(IdentifyDriveInfo, sizeof (EFI_ATAPI_IDENTIFY_DATA));

      BufferSize = sizeof (EFI_ATAPI_IDENTIFY_DATA);
      Status = DiskInfo->Identify (
                                  DiskInfo,
                                  IdentifyDriveInfo,
                                  &BufferSize
                                  );
      ASSERT_EFI_ERROR (Status);

    } else {
      continue;
    }

    ZeroMem(ModelNumber, 42);
    CopyMem (ModelNumber, IdentifyDriveInfo->ModelName, 40);
    SwapEntries (ModelNumber, 40);
    ModelNumber[14] = '\0';           // Truncate it at 14 characters

    //
    // For HardDisk append the size. Otherwise display atapi
    //
    if (!(IdentifyDriveInfo->config & ATAPI_DEVICE)) {
      if (IdentifyDriveInfo->command_set_supported_83 & _48_BIT_ADDRESS_FEATURE_SET_SUPPORTED) {
        NumSectors = *(UINT64 *) &IdentifyDriveInfo->maximum_lba_for_48bit_addressing;
      } else {
        NumSectors = (UINT64) *(UINT32 *) &IdentifyDriveInfo->user_addressable_sectors_lo;
      }
      DriveSizeInBytes = MultU64x32 (NumSectors, 512);

      //DriveSizeInGB is DriveSizeInBytes / 1 GB (1 Binary GB = 2^30 bytes)
      //DriveSizeInGB = (UINT32) Div64(DriveSizeInBytes, (1 << 30), &RemainderInBytes);
      //Convert the Remainder, which is in bytes, to number of tenths of a Binary GB.
      //NumTenthsOfGB = GetNumTenthsOfGB(RemainderInBytes);

      //DriveSizeInGB is DriveSizeInBytes / 1 GB (1 Decimal GB = 10^9 bytes)
      DriveSizeInGB = (UINT32) DivU64x64Remainder (DriveSizeInBytes, 1000000000, &RemainderInBytes);
      //Convert the Remainder, which is in bytes, to number of tenths of a Decimal GB.
      NumTenthsOfGB = (UINT32) DivU64x64Remainder (RemainderInBytes, 100000000, NULL);

      //
      // Update SPEED.
      //
      PortNumber = (SataPortIndex << 1) + IdeChannel;
      DeviceSpeed = GetChipsetSataPortSpeed(PortNumber);

      switch (DeviceSpeed) {
        case 1:
          UnicodeSPrint (NewString, sizeof(NewString), L"%a (%d.%dGB - 1.5GB/s)", ModelNumber, DriveSizeInGB, NumTenthsOfGB);
          break;
        case 2:
          UnicodeSPrint (NewString, sizeof(NewString), L"%a (%d.%dGB - 3.0GB/s)", ModelNumber, DriveSizeInGB, NumTenthsOfGB);
          break;
        case 3:
          UnicodeSPrint (NewString, sizeof(NewString), L"%a (%d.%dGB - 6.0GB/s)", ModelNumber, DriveSizeInGB, NumTenthsOfGB);
          break;
        default: 
          UnicodeSPrint (NewString, sizeof(NewString), L"%a (%d.%dGB)", ModelNumber, DriveSizeInGB, NumTenthsOfGB);
          break;   
      }
        

      if ((IdentifyDriveInfo->serial_ata_capabilities != 0xFFFF) && (IdentifyDriveInfo->serial_ata_features_supported & 0x0040)) {
        UnicodeSPrint (SoftPres, sizeof(SoftPres), L"SUPPORTED");
	  } else {
        UnicodeSPrint (SoftPres, sizeof(SoftPres), L"NOT SUPPORTED");
	  }	
      
    } else {
      UnicodeSPrint (NewString, sizeof(NewString), L"%a ATAPI", ModelNumber);
      UnicodeSPrint (SoftPres, sizeof(SoftPres), L"  N/A  ");
    }

    HiiSetString(HiiHandle, (STRING_REF)Token, NewString, NULL);
    HiiSetString(HiiHandle, (STRING_REF)SoftPresStatus, SoftPres, NULL);

    if (IdentifyDriveInfo) {
      FreePool (IdentifyDriveInfo);
      IdentifyDriveInfo = NULL;
    }
  }

  if (HandleBuffer) {
    FreePool (HandleBuffer);
  }

}

