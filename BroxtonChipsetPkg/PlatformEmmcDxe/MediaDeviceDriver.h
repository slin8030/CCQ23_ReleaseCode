/** @file
  Header file for Media Device Driver

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _MEDIA_DEVICE_DRIVER_H
#define _MEDIA_DEVICE_DRIVER_H


#include <Uefi.h>

#include <Guid/EventGroup.h>


#include <Library/UefiDriverEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <IndustryStandard/Mmc.h>
#include <IndustryStandard/CeAta.h>
#include <IndustryStandard/SdCard.h>

//
// Driver Consumed Protocol Prototypes
//
#include <Protocol/DevicePath.h>
#include <Protocol/SdHostIo.h>

//
// Driver Produced Protocol Prototypes
//
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/BlockIo.h>


typedef enum{
  UnknownCard = 0,
  MMCCard,                // MMC card
  CEATACard,              // CE-ATA device
  SDMemoryCard,           // SD 1.1 card
  SDMemoryCard2,          // SD 2.0 or above standard card
  SDMemoryCard2High       // SD 2.0 or above high capacity card
} CARD_TYPE;


typedef struct _CARD_DATA CARD_DATA;

typedef struct {
  //
  //BlockIO
  //
  UINT32                    Signature;

  EFI_HANDLE                Handle;

  BOOLEAN                   Present;

  EFI_DEVICE_PATH_PROTOCOL  *DevPath;

  EFI_BLOCK_IO_PROTOCOL     BlockIo;

  EFI_BLOCK_IO_MEDIA        BlockIoMedia;

  CARD_DATA                 *CardData;

} MMC_PARTITION_DATA;


#define MAX_NUMBER_OF_PARTITIONS 8


struct _CARD_DATA {
  //
  //BlockIO
  //
  UINT32                    Signature;

  EFI_HANDLE                Handle;

  MMC_PARTITION_DATA        Partitions[MAX_NUMBER_OF_PARTITIONS];

  EFI_SD_HOST_IO_PROTOCOL   *SdHostIo;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
  CARD_TYPE                 CardType;

  UINT8                     CurrentBusWidth;
  BOOLEAN                   DualVoltage;
  BOOLEAN                   NeedFlush;
  UINT8                     Reserved[3];

  UINT16                    Address;
  UINT32                    BlockLen;
  UINT32                    MaxFrequency;
  UINT64                    BlockNumber;
  //
  //Common used
  //
  CARD_STATUS               CardStatus;
  OCR                       OCRRegister;
  CID                       CIDRegister;
  CSD                       CSDRegister;
  EXT_CSD                   ExtCSDRegister;
  UINT8                     *RawBufferPointer;
  UINT8                     *AlignedBuffer;
  //
  //CE-ATA specific
  //
  TASK_FILE                 TaskFile;
  IDENTIFY_DEVICE_DATA      IndentifyDeviceData;
  //
  //SD specific
  //
  SCR                       SCRRegister;
  SD_STATUS_REG             SDSattus;
  SWITCH_STATUS             SwitchStatus;
};

#endif


