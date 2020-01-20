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

#include <Library/UefiDriverEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/TimerLib.h>

#include <Ppi/Sdhc.h>
#include <Ppi/PeiBlockIo.h>

#include <IndustryStandard/Mmc.h>
#include <IndustryStandard/SdCard.h>

//
// Driver Consumed Protocol Prototypes
//
#include <Protocol/DevicePath.h>

//
// Driver Produced Protocol Prototypes
//
//#include <Ppi/BlockIo.h>
#include <Ppi/BlockIoPei.h>  //Override



extern UINT32  gMediaDeviceDebugLevel;


#define PEI_CARD_DATA_SIGNATURE  SIGNATURE_32 ('p', 'c', 'r', 'd')
#define PEI_CARD_PARTITION_SIGNATURE  SIGNATURE_32 ('p', 'c', 'a', 'r')

#define MAX_NUMBER_OF_PARTITIONS 8

#define PEI_CARD_PARTITION_DATA_FROM_THIS(a) \
    CR(a, PEI_MMC_PARTITION_DATA, BlockIo, PEI_CARD_PARTITION_SIGNATURE)

#define PEI_CARD_DATA_FROM_THIS(a) \
    ((PEI_CARD_PARTITION_DATA_FROM_THIS(a))->PeiCardData)

#define PEI_CARD_DATA_PARTITION_NUM(p) \
    ((((UINTN) p) - ((UINTN) &(p->PeiCardData->Partitions))) / sizeof (*p))

    ///
    /// Used to check-call local functions inside this eMMC module
    ///
    /// Func - The function name which needs to be checked
    /// Str   - Optional, it is the description of the output infomation
    /// FailOrNot - When functional call is failed, it should stop or continue
    ///
#define CHK_FUNC_CALL(Func, Str, FailOrNot) \
        do { \
            EFI_STATUS ret = Func; \
            if (EFI_SUCCESS!= ret && TRUE == FailOrNot) { \
                DEBUG((EFI_D_ERROR, "ERROR: %a,%d:", __FUNCTION__,__LINE__)); \
                if ( NULL != (void *)Str  ) {\
                    DEBUG((EFI_D_ERROR, "%a:", Str)); \
                } \
                DEBUG((EFI_D_ERROR, " Status = 0x%x\n", ret)); \
                return ret; \
            } \
        } while (0)


///
/// Command timeout will be max 100 ms
///
#define  TIMEOUT_COMMAND     500
#define  TIMEOUT_DATA        5000

typedef enum {
  UnknownCard = 0,
  MMCCard,                ///< MMC card
  CEATACard,              ///< CE-ATA device
  SDMemoryCard,           ///< SD 1.1 card
  SDMemoryCard2,          ///< SD 2.0 or above standard card
  SDMemoryCard2High       ///< SD 2.0 or above high capacity card
} CARD_TYPE;


//typedef struct _CARD_DATA CARD_DATA;
typedef struct _PEI_CARD_DATA PEI_CARD_DATA;

typedef struct {
  //
  //BlockIO
  //
  UINT32                    Signature;
  EFI_HANDLE                Handle;
  BOOLEAN                   Present;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI   BlockIo;
  EFI_PEI_PPI_DESCRIPTOR          BlkIoPpiList;
  PEI_BLOCK_IO_MEDIA        BlockIoMedia;
  PEI_CARD_DATA             *PeiCardData;
} PEI_MMC_PARTITION_DATA;

struct _PEI_CARD_DATA {
  //
  //BlockIO
  //
  UINT32                    Signature;
  EFI_HANDLE                Handle;
  PEI_MMC_PARTITION_DATA    Partitions[MAX_NUMBER_OF_PARTITIONS];
  EFI_PEI_PPI_DESCRIPTOR    PpiList;
  PEI_SD_CONTROLLER_PPI     *SdControllerPpi;
  EFI_PEI_PCI_CFG2_PPI      *PciCfgPpi;
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
  //SD specific
  //
  SCR                       SCRRegister;
  SD_STATUS_REG             SDSattus;
  SWITCH_STATUS             SwitchStatus;
};


//
//
//
/**
  MediaDeviceDriverEntryPoint

  @param[in]  FileHandle
  @param[in]  PeiServices

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  );

/**
  BotGetNumberOfBlockDevices

  @param[in]  PeiServices
  @param[in]  This
  @param[in]  NumberBlockDevices

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
BotGetNumberOfBlockDevices (
  IN  EFI_PEI_SERVICES                         **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO_PPI            *This,
  OUT UINTN                                    *NumberBlockDevices
  );

/**
  BotGetMediaInfo

  @param[in]  PeiServices
  @param[in]  This
  @param[in]  DeviceIndex
  @param[out]  MediaInfo

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
BotGetMediaInfo (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO_PPI             *This,
  IN  UINTN                                     DeviceIndex,
  OUT EFI_PEI_BLOCK_IO_MEDIA                    *MediaInfo
  );

/**
  MMCSDBlockReadBlocks

  @param[in]  PeiServices
  @param[in]  This
  @param[in]  MediaId
  @param[in]  LBA
  @param[in]  BufferSize
  @param[out]  Buffer

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MMCSDBlockReadBlocks (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN   EFI_PEI_RECOVERY_BLOCK_IO_PPI        *This,
  IN   UINT32                  MediaId,
  IN   EFI_LBA                 LBA,
  IN   UINTN                   BufferSize,
  OUT  VOID                    *Buffer
  );

/**
  MMCSDBlockWriteBlocks

  @param[in]  PeiServices
  @param[in]  This
  @param[in]  MediaId
  @param[in]  LBA
  @param[in]  BufferSize
  @param[in]  Buffer

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
MMCSDBlockWriteBlocks (
  IN  EFI_PEI_SERVICES                                **PeiServices,
  IN   EFI_PEI_RECOVERY_BLOCK_IO_PPI                  *This,
  IN   UINTN                                          MediaId,
  IN   EFI_PEI_LBA                                    LBA,
  IN   UINTN                                          BufferSize,
  IN   VOID                                           *Buffer
  );

/**
  MMCSDCardInit

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MMCSDCardInit (
  IN  PEI_CARD_DATA              *PeiCardData
  );

/**
  MMCSDBlockIoInit

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MMCSDBlockIoInit (
  IN  PEI_CARD_DATA    *PeiCardData
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
  IN  PEI_SD_CONTROLLER_PPI       *This,
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

  @param[in]  PeiCardData
  @param[in]  RegisterAddress
  @param[in,out]  RegisterData
  @param[in]  Write

  @retval  EFI_STATUS
**/
EFI_STATUS
FastIO (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN      UINT8       RegisterAddress,
  IN  OUT UINT8       *RegisterData,
  IN      BOOLEAN     Write
  );


/**
  IndentifyDevice

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
IndentifyDevice (
  IN  PEI_CARD_DATA              *PeiCardData
  );

/**
  FlushCache

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
FlushCache (
  IN  PEI_CARD_DATA    *PeiCardData
  );

/**
  StandByImmediate

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
StandByImmediate (
  IN  PEI_CARD_DATA           *PeiCardData
  );

/**
  ReadDMAExt

  @param[in]  PeiCardData
  @param[in]  LBA
  @param[in]  Buffer
  @param[in]  SectorCount

  @retval  EFI_STATUS
**/
EFI_STATUS
ReadDMAExt (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  EFI_LBA     LBA,
  IN  UINT8       *Buffer,
  IN  UINT16      SectorCount
  );

/**
  WriteDMAExt

  @param[in]  PeiCardData
  @param[in]  LBA
  @param[in]  Buffer
  @param[in]  SectorCount

  @retval  EFI_STATUS
**/
EFI_STATUS
WriteDMAExt (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  EFI_LBA     LBA,
  IN  UINT8       *Buffer,
  IN  UINT16      SectorCount
  );

/**
  SoftwareReset

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
SoftwareReset (
  IN  PEI_CARD_DATA              *PeiCardData
  );

/**
  SendAppCommand

  @param[in]  PeiCardData
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
  IN  PEI_CARD_DATA              *PeiCardData,
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

  @param[in]  PeiCardData
  @param[in]  Offset

  @retval  UINT32
**/
UINT32
MmcGetExtCsd8 (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  UINTN            Offset
  );

/**
  MmcGetExtCsd24

  @param[in]  PeiCardData
  @param[in]  Offset

  @retval  UINT32
**/
UINT32
MmcGetExtCsd24 (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  UINTN            Offset
  );

/**
  MmcGetExtCsd32

  @param[in]  PeiCardData
  @param[in]  Offset

  @retval  UINT32
**/
UINT32
MmcGetExtCsd32 (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  UINTN            Offset
  );

/**
  MmcGetCurrentPartitionNum

  @param[in]  PeiCardData

  @retval  UINTN
**/
UINTN
MmcGetCurrentPartitionNum (
  IN  PEI_CARD_DATA    *PeiCardData
  );

/**
  MmcSelectPartitionNum

  @param[in]  PeiCardData
  @param[in]  Partition

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSelectPartitionNum (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  UINT8            Partition
  );

/**
  MmcSelectPartition

  @param[in]  Partition

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSelectPartition (
  IN  PEI_MMC_PARTITION_DATA     *Partition
  );

/**
  MmcReadExtCsd

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcReadExtCsd (
  IN  PEI_CARD_DATA    *PeiCardData
  );

/**
  MmcMoveToTranState

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcMoveToTranState (
  IN  PEI_CARD_DATA    *PeiCardData
  );

/**
  MmcSelect

  @param[in]  PeiCardData
  @param[in]  Select

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSelect (
  IN  PEI_CARD_DATA    *PeiCardData,
  IN  BOOLEAN          Select
  );

/**
  MmcSendSwitch

  @param[in]  PeiCardData
  @param[in]  SwitchArgument

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSendSwitch (
  IN  PEI_CARD_DATA        *PeiCardData,
  IN  SWITCH_ARGUMENT      *SwitchArgument
  );

/**
  MmcGoIdleState

  @param[in]  PeiCardData

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcGoIdleState (
  IN  PEI_CARD_DATA    *PeiCardData
  );

/**
  MmcSetPartition

  @param[in]  CardData
  @param[in]  Value

  @retval  EFI_STATUS
**/
EFI_STATUS
MmcSetPartition (
  IN  PEI_CARD_DATA          *CardData,
  IN  UINT8                  Value
  );

#endif


