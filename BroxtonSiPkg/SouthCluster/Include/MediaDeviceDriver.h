/** @file
  Media Device Driver header

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
#include <PiDxe.h>
#include <Library/HobLib.h>
#include <Guid/HobList.h>
#include <Library/PciLib.h>

#include <IndustryStandard/Mmc.h>
#include <IndustryStandard/CeAta.h>
#include <IndustryStandard/SdCard.h>
#include <Library/PreSiliconLib.h>
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

extern EFI_COMPONENT_NAME_PROTOCOL  gMediaDeviceComponentName;
extern EFI_DRIVER_BINDING_PROTOCOL  gMediaDeviceDriverBinding;
extern EFI_COMPONENT_NAME2_PROTOCOL  gSdMediaDeviceName2;

extern UINT32  gMediaDeviceDebugLevel;

extern EFI_GUID gEfiEmmcBootPartitionProtocolGuid;

#define CARD_DATA_SIGNATURE  SIGNATURE_32 ('c', 'a', 'r', 'd')
#define CARD_PARTITION_SIGNATURE  SIGNATURE_32 ('c', 'p', 'a', 'r')

#define CARD_PARTITION_DATA_FROM_THIS(a) \
    CR(a, MMC_PARTITION_DATA, BlockIo, CARD_PARTITION_SIGNATURE)

#define CARD_DATA_FROM_THIS(a) \
    ((CARD_PARTITION_DATA_FROM_THIS(a))->CardData)

#define CARD_DATA_PARTITION_NUM(p) \
    ((((UINTN) p) - ((UINTN) &(p->CardData->Partitions))) / sizeof (*p))

//

//
// Partition Number
//
#define NO_ACCESS_TO_BOOT_PARTITION                0x00
#define BOOT_PARTITION_1                           0x01
#define BOOT_PARTITION_2                           0x02
#define REPLAY_PROTECTED_MEMORY_BLOCK              0x03
#define GENERAL_PURPOSE_PARTITION_1                0x04
#define GENERAL_PURPOSE_PARTITION_2                0x05
#define GENERAL_PURPOSE_PARTITION_3                0x06
#define GENERAL_PURPOSE_PARTITION_4                0x07
// Command timeout will be max 100 ms
//
#define  TIMEOUT_COMMAND     100
#define  TIMEOUT_DATA        5000

typedef enum {
  UnknownCard = 0,
  MMCCard,                ///< MMC card
  CEATACard,              ///< CE-ATA device
  SDMemoryCard,           ///< SD 1.1 card
  SDMemoryCard2,          ///< SD 2.0 or above standard card
  SDMemoryCard2High       ///< SD 2.0 or above high capacity card
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


///
///
///
/**
  MediaDeviceDriverEntryPoint

  @param[in]  ImageHandle
  @param[in]  SystemTable

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

/**
  MediaDeviceDriverInstallBlockIo

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MediaDeviceDriverInstallBlockIo (
  IN  CARD_DATA    *CardData
  );

/**
  MediaDeviceDriverUninstallBlockIo

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MediaDeviceDriverUninstallBlockIo (
  IN  CARD_DATA    *CardData
  );

/**
  MediaDeviceComponentNameGetDriverName

  @param[in]  This
  @param[in]  Language
  @param[out]  DriverName

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL     *This,
  IN  CHAR8                           *Language,
  OUT CHAR16                          **DriverName
  );

/**
  MediaDeviceComponentNameGetControllerName

  @param[in]  This
  @param[in]  ControllerHandle
  @param[in]  ChildHandle
  @param[in]  Language
  @param[out]  ControllerName

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ChildHandle, OPTIONAL
  IN  CHAR8                           *Language,
  OUT CHAR16                          **ControllerName
  );

/**
  MediaDeviceDriverBindingSupported

  @param[in]  This
  @param[in]  Controller
  @param[in]  RemainingDevicePath

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                  Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

/**
  MediaDeviceDriverBindingStart

  @param[in]  This
  @param[in]  Controller
  @param[in]  RemainingDevicePath

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      Controller,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  );

/**
  MediaDeviceDriverBindingStop

  @param[in]  This
  @param[in]  Controller
  @param[in]  NumberOfChildren
  @param[in]  ChildHandleBuffer

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );


/**
  MMCSDCardInit

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MMCSDCardInit (
  IN  CARD_DATA    *CardData
  );

/**
  MMCSDBlockIoInit

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MMCSDBlockIoInit (
  IN  CARD_DATA    *CardData
  );

/**
  SendCommand

  @param[in]  This
  @param[in]  CommandIndex
  @param[in]  Argument
  @param[in]  DataType
  @param[in]  Buffer
  @param[in]  BufferSize
  @param[in]  ResponseType
  @param[in]  TimeOut
  @param[out]  ResponseData

  @retval  EFI_STATUS
**/
EFI_STATUS
SendCommand (
  IN   EFI_SD_HOST_IO_PROTOCOL    *This,
  IN   UINT16                     CommandIndex,
  IN   UINT32                     Argument,
  IN   TRANSFER_TYPE              DataType,
  IN   UINT8                      *Buffer, OPTIONAL
  IN   UINT32                     BufferSize,
  IN   RESPONSE_TYPE              ResponseType,
  IN   UINT32                     TimeOut,
  OUT  UINT32                     *ResponseData
  );

/**
  FastIO

  @param[in]  CardData
  @param[in]  RegisterAddress
  @param[in,out]  RegisterData
  @param[in]  Write

  @retval  EFI_STATUS
**/
EFI_STATUS
FastIO (
  IN      CARD_DATA   *CardData,
  IN      UINT8       RegisterAddress,
  IN  OUT UINT8       *RegisterData,
  IN      BOOLEAN     Write
  );

/**
  IsCEATADevice

  @param[in]  CardData

  @retval  EFI_STATUS
**/
BOOLEAN
IsCEATADevice (
  IN  CARD_DATA    *CardData
  );

/**
  CEATABlockIoInit

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
CEATABlockIoInit (
  IN  CARD_DATA    *CardData
  );

/**
  IndentifyDevice

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
IndentifyDevice (
  IN  CARD_DATA    *CardData
  );

/**
  FlushCache

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
FlushCache (
  IN  CARD_DATA    *CardData
  );

/**
  StandByImmediate

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
StandByImmediate (
  IN  CARD_DATA    *CardData
  );

/**
  ReadDMAExt

  @param[in]  CardData
  @param[in]  LBA
  @param[in]  Buffer
  @param[in]  SectorCount

  @retval  EFI_STATUS
**/
EFI_STATUS
ReadDMAExt (
  IN  CARD_DATA   *CardData,
  IN  EFI_LBA     LBA,
  IN  UINT8       *Buffer,
  IN  UINT16      SectorCount
  );

/**
  WriteDMAExt

  @param[in]  CardData
  @param[in]  LBA
  @param[in]  Buffer
  @param[in]  SectorCount

  @retval  EFI_STATUS
**/
EFI_STATUS
WriteDMAExt (
  IN  CARD_DATA   *CardData,
  IN  EFI_LBA     LBA,
  IN  UINT8       *Buffer,
  IN  UINT16      SectorCount
  );

/**
  SoftwareReset

  @param[in]  CardData

  @retval  EFI_STATUS
**/
EFI_STATUS
SoftwareReset (
  IN  CARD_DATA    *CardData
  );

/**
  SendAppCommand

  @param[in]  CardData
  @param[in]  CommandIndex
  @param[in]  Argument
  @param[in]  DataType
  @param[in]  Buffer
  @param[in]  BufferSize
  @param[in]  ResponseType
  @param[in]  TimeOut
  @param[out]  ResponseData

  @retval  EFI_STATUS
**/
EFI_STATUS
SendAppCommand (
  IN   CARD_DATA                  *CardData,
  IN   UINT16                     CommandIndex,
  IN   UINT32                     Argument,
  IN   TRANSFER_TYPE              DataType,
  IN   UINT8                      *Buffer, OPTIONAL
  IN   UINT32                     BufferSize,
  IN   RESPONSE_TYPE              ResponseType,
  IN   UINT32                     TimeOut,
  OUT  UINT32                     *ResponseData
  );

/**
  MmcGetExtCsd8

  @param[in]  CardData
  @param[in]  Offset

  @retval  UINT32
**/
UINT32
MmcGetExtCsd8 (
  IN CARD_DATA                        *CardData,
  IN UINTN                            Offset
  );

/**
  MmcGetExtCsd24

  @param[in]  CardData
  @param[in]  Offset

  @retval  UINT32
**/
UINT32
MmcGetExtCsd24 (
  IN CARD_DATA                        *CardData,
  IN UINTN                            Offset
  );

/**
  MmcGetExtCsd32

  @param[in]  CardData
  @param[in]  Offset

  @retval  UINT32
**/
UINT32
MmcGetExtCsd32 (
  IN CARD_DATA                        *CardData,
  IN UINTN                            Offset
  );

/**
  MmcGetCurrentPartitionNum

  @param[in]  CardData

  @retval  UINTN
**/
UINTN
MmcGetCurrentPartitionNum (
  IN  CARD_DATA              *CardData
  );

/**
  SetEmmcWpOnEvent

  @param[in]  Event
  @param[in]  Context

  @retval  None
**/
VOID
EFIAPI
SetEmmcWpOnEvent(
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );


/**
  MmcSelectPartitionNum

  @param[in]  CardData
  @param[in]  Partition

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSelectPartitionNum (
  IN  CARD_DATA              *CardData,
  IN  UINT8                  Partition
  );

/**
  MmcSelectPartition

  @param[in]  Partition

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSelectPartition (
  IN  MMC_PARTITION_DATA     *Partition
  );

/**
  MmcSetPartition

  @param[in]  CardData
  @param[in]  Value

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSetPartition (
  IN  CARD_DATA              *CardData,
  IN  UINT8                  Value
  );


#endif
