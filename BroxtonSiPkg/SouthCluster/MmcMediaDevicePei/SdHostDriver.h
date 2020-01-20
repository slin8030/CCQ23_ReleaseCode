/** @file
  Header file for driver.

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

#ifndef _SD_HOST_DRIVER_H
#define _SD_HOST_DRIVER_H

#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SdCard.h>
#include <SaCommonDefinitions.h>
#include <Ppi/Sdhc.h>

#ifdef FSP_FLAG
#include <Library/FspCommonLib.h>
#include <FspmUpd.h>
#endif

#define PEI_SDHOST_DATA_SIGNATURE  SIGNATURE_32 ('p', 's', 'd', 'h')

#define PEI_SDHOST_DATA_FROM_THIS(a)  \
    CR(a, PEI_SDHOST_DATA, SdControllerPpi, PEI_SDHOST_DATA_SIGNATURE)

#define BLOCK_SIZE   0x200
#define TIME_OUT_1S  1000


#define EMMC_DEV_NUMBER                                28
#define INTEL_VENDOR_ID                                0x8086
#define POULSBO_FUNC_SDIO1                             0x811C
#define POULSBO_FUNC_SDIO2                             0x811D
#define POULSBO_FUNC_SDIO3                             0x811E
#define R_SCH_LPC_REV_ID                               0x08
#define   V_SCH_LPC_REV_ID_A0                          0x00
#define   V_SCH_LPC_REV_ID_A1                          0x00
#define   V_SCH_LPC_REV_ID_A2                          0x01
#define   V_SCH_LPC_REV_ID_B0                          0x02
#define   V_SCH_LPC_REV_ID_B1                          0x03
#define   V_SCH_LPC_REV_ID_C0                          0x04
#define   V_SCH_LPC_REV_ID_D0                          0x05
#define   V_SCH_LPC_REV_ID_D1                          0x06
#define   V_SCH_LPC_REV_ID_D2                          0x07


#define BUFFER_CTL_REGISTER 0x84


#pragma pack(1)

extern UINT32  mSdBaseAddress;

//
// PCI Class Code structure
//
typedef struct {
  UINT8 PI;
  UINT8 SubClassCode;
  UINT8 BaseCode;
} PCI_CLASSC;

#pragma pack()

typedef struct {
  UINTN                      Signature;
  PEI_SD_CONTROLLER_PPI      SdControllerPpi;
  EFI_PEI_PPI_DESCRIPTOR     PpiList;
  EFI_PEI_PCI_CFG2_PPI       *PciCfgPpi;
  UINT8                      SdControllerId;
  UINTN                      BaseAddress;
  UINT16                     PciVid;
  UINT16                     PciDid;
  BOOLEAN                    IsAutoStopCmd;
  BOOLEAN                    IsPenwell;
  BOOLEAN                    IsEmmc;
  BOOLEAN                    EnableVerboseDebug;
  UINT32                     BaseClockInMHz;
  UINT32                     CurrentClockInKHz;
  UINT32                     BlockLength;
  EFI_UNICODE_STRING_TABLE   *ControllerNameTable;
  UINT32                     ControllerVersion;
} PEI_SDHOST_DATA;


/**
  Entry point for EFI drivers.

  @param[in]  FileHandle  - EFI_PEI_FILE_HANDLE
  @param[in]  PeiServices - EFI_PEI_SERVICES

  @retval  EFI_SUCCESS       - Success
  @retval  EFI_DEVICE_ERROR  - Fail
**/
EFI_STATUS
EFIAPI
SdHostDriverEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  );



/**
  The main function used to send the command to the card inserted into the SD host
  slot.
  It will assemble the arguments to set the command register and wait for the command
  and transfer completed until timeout. Then it will read the response register to fill
  the ResponseData

  @param[in]  This           - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  CommandIndex   - The command index to set the command index field of command register
  @param[in]  Argument       - Command argument to set the argument field of command register
  @param[in]  DataType       - TRANSFER_TYPE, indicates no data, data in or data out
  @param[in]  Buffer         - Contains the data read from / write to the device
  @param[in]  BufferSize     - The size of the buffer
  @param[in]  ResponseType   - RESPONSE_TYPE
  @param[in]  TimeOut        - Time out value in 1 ms unit
  @param[out] ResponseData   - Depending on the ResponseType, such as CSD or card status

  @retval  EFI_SUCCESS
  @retval  EFI_INVALID_PARAMETER
  @retval  EFI_OUT_OF_RESOURCES
  @retval  EFI_TIMEOUT
  @retval  EFI_DEVICE_ERROR
**/
EFI_STATUS
EFIAPI
HostSendCommand (
  IN   PEI_SD_CONTROLLER_PPI      *This,
  IN   UINT16                     CommandIndex,
  IN   UINT32                     Argument,
  IN   TRANSFER_TYPE              DataType,
  IN   UINT8                      *Buffer, OPTIONAL
  IN   UINT32                     BufferSize,
  IN   RESPONSE_TYPE              ResponseType,
  IN   UINT32                     TimeOut,
  OUT  UINT32                     *ResponseData OPTIONAL
  );
/**
  Set max clock frequency of the host, the actual frequency
  may not be the same as MaxFrequencyInKHz. It depends on
  the max frequency the host can support, divider, and host
  speed mode.

  @param[in]  This            - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  MaxFrequency    - Max frequency in KHZ

  @retval  EFI_SUCCESS
  @retval  EFI_TIMEOUT
**/
EFI_STATUS
EFIAPI
SetClockFrequency (
  IN PEI_SD_CONTROLLER_PPI       *This,
  IN UINT32                      MaxFrequencyInKHz
  );
/**
  Set bus width of the host

  @param[in]  This       - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  BusWidth   - Bus width in 1, 4, 8 bits

  @retval  EFI_STATUS
  @retval  EFI_INVALID_PARAMETER
**/
EFI_STATUS
EFIAPI
SetBusWidth (
  IN PEI_SD_CONTROLLER_PPI       *This,
  IN UINT32                      BusWidth
  );
/**
  Set voltage which could supported by the host.
  Support 0(Power off the host), 1.8V, 3.0V, 3.3V

  @param[in]  This      - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Voltage   - Units in 0.1 V

  @retval  EFI_SUCCESS
  @retval  EFI_INVALID_PARAMETER
**/
EFI_STATUS
EFIAPI
SetHostVoltage (
  IN PEI_SD_CONTROLLER_PPI       *This,
  IN UINT32                      Voltage
  );
/**
  Set Host High Speed

  @param[in]  This         - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  HighSpeed    - True for High Speed Mode set, false for normal mode

  @retval  EFI_SUCCESS
  @retval  EFI_INVALID_PARAMETER
**/
EFI_STATUS
EFIAPI
SetHostSpeedMode(
  IN  PEI_SD_CONTROLLER_PPI    *This,
  IN  UINT32                   DdrMode
  );
/**
  Set Host DDR mode

  @param[in]  This             - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  SetHostDdrMode   - True for DDR Mode set, false returns EFI_SUCCESS

  @retval  EFI_SUCCESS         - Success
  @retval  EFI_DEVICE_ERROR    - Fail
**/
EFI_STATUS
EFIAPI
SetHostDdrMode(
  IN  PEI_SD_CONTROLLER_PPI    *This,
  IN  UINT32                   DdrMode
  );
/**
  Reset the Sd host

  @param[in]  This       - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  ResetType  - Reset type(Data/CMD)

  @retval  EFI_SUCCESS        - Success
  @retval  EFI_DEVICE_ERROR   - Fail
**/
EFI_STATUS
EFIAPI
ResetSdHost (
  IN PEI_SD_CONTROLLER_PPI       *This,
  IN RESET_TYPE                  ResetType
  );
/**
  Reset the host

  @param[in]  This    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Enable  - TRUE to enable, FALSE to disable

  @retval  EFI_SUCCESS         - Success
  @retval  EFI_DEVICE_ERROR    - Fail
**/
EFI_STATUS
EFIAPI
EnableAutoStopCmd (
  IN PEI_SD_CONTROLLER_PPI       *This,
  IN BOOLEAN                     Enable
  );

/**
  Detect MMC

  @param[in]  This    - Pointer to PEI_SD_CONTROLLER_PPI

  @retval  EFI_SUCCESS         - Success
  @retval  EFI_DEVICE_ERROR    - Fail
**/
EFI_STATUS
EFIAPI
MMC_Detected (
  IN PEI_SD_CONTROLLER_PPI       *This
  );
/**
  Find whether these is a card inserted into the slot. If so
  init the host. If not, return EFI_NOT_FOUND.

  @param[in]  This      - Pointer to PEI_SD_CONTROLLER_PPI
  @retval  EFI_INVALID_PARAMETER
  @retval  EFI_NOT_FOUND
  @retval  EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
DetectCardAndInitHost (
  IN PEI_SD_CONTROLLER_PPI       *This
  );
/**
  Set the Block length

  @param[in]  This        - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  BlockLength - Card supportes block length

  @retval  EFI_SUCCESS       - Success
  @retval  EFI_DEVICE_ERROR  - Fail
**/
EFI_STATUS
EFIAPI
SetBlockLength (
  IN PEI_SD_CONTROLLER_PPI       *This,
  IN UINT32                      BlockLength
  );
/**
  Setup Device

  @param[in]  This    - Pointer to PEI_SD_CONTROLLER_PPI

  @retval  None
**/
EFI_STATUS
EFIAPI
SetupDevice(
  IN  PEI_SD_CONTROLLER_PPI    *This
  );
/**
  Read Sd host register

  @param[in]  SdHost    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Offset    - Offset of the SD Card

  @retval     Data      - Data read from SD Card
**/
UINT8
SdHostRead8 (
  IN PEI_SDHOST_DATA             *SdHost,
  IN UINTN                       Offset
  );
/**
  Read Sd host register

  @param[in]  SdHost    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Offset    - Offset of the SD Card

  @retval  Data         - Data read from SD Card
**/
UINT16
SdHostRead16 (
  IN PEI_SDHOST_DATA             *SdHost,
  IN UINTN                       Offset
  );
/**
  Read Sd host register

  @param[in]  SdHost    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Offset    - Offset of the SD Card

  @retval  Data         - Data read from SD Card
**/
UINT32
SdHostRead32 (
  IN PEI_SDHOST_DATA             *SdHost,
  IN UINTN                       Offset
  );
/**
  Write Sd host register

  @param[in]  SdHost    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Offset    - Offset of the SD Card
  @param[in]  Data      - Data write to SD Card
  @retval  Data  - Writen to SdHost
**/
UINT8
SdHostWrite8 (
  IN PEI_SDHOST_DATA             *SdHost,
  IN UINTN                       Offset,
  IN UINT8                       Data
  );
/**
  Write Sd host register

  @param[in]  SdHost    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Offset    - Offset of the SD Card
  @param[in]  Data      - Data write to SD Card

  @retval  Data         - Data written to SD Card
**/
UINT16
SdHostWrite16 (
  IN PEI_SDHOST_DATA             *SdHost,
  IN UINTN                       Offset,
  IN UINT16                      Data
  );
/**
  Write Sd host register

  @param[in]  SdHost    - Pointer to PEI_SD_CONTROLLER_PPI
  @param[in]  Offset    - Offset of the SD Card
  @param[in]  Data      - Data write to SD Card

  @retval  Data         - Data written to SD Card
**/
UINT32
SdHostWrite32 (
  IN PEI_SDHOST_DATA             *SdHost,
  IN UINTN                       Offset,
  IN UINT32                      Data
  );


#endif
