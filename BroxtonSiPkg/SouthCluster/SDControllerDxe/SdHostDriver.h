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

#include <Library/UefiDriverEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SdCard.h>
#include <ScAccess.h>
#include <Library/PreSiliconLib.h>

//
// Driver Consumed Protocol Prototypes
//
#include <Protocol/DevicePath.h>
#include <Protocol/PciIo.h>

//
// Driver Produced Protocol Prototypes
//
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/SdHostIo.h>

extern EFI_COMPONENT_NAME_PROTOCOL  gSdHostComponentName;
extern EFI_DRIVER_BINDING_PROTOCOL  gSdHostDriverBinding;

#define SDHOST_DATA_SIGNATURE  SIGNATURE_32 ('s', 'd', 'h', 's')

#define SDHOST_DATA_FROM_THIS(a) \
    CR(a, SDHOST_DATA, SdHostIo, SDHOST_DATA_SIGNATURE)

#define BLOCK_SIZE   0x200
#define TIME_OUT_1S  1000


#define INTEL_VENDOR_ID     0x8086
#define POULSBO_FUNC_SDIO1  0x811C
#define POULSBO_FUNC_SDIO2  0x811D
#define POULSBO_FUNC_SDIO3  0x811E

#define BUFFER_CTL_REGISTER 0x84


#pragma pack(1)
//
// PCI Class Code structure
//
typedef struct {
  UINT8 PI;
  UINT8 SubClassCode;
  UINT8 BaseCode;
} PCI_CLASSC;

#pragma pack()

//
// SDHOST_DATA structure
//
typedef struct {
  UINTN                      Signature;
  EFI_SD_HOST_IO_PROTOCOL    SdHostIo;
  EFI_PCI_IO_PROTOCOL        *PciIo;
  UINT16                     PciVid;
  UINT16                     PciDid;
  BOOLEAN                    IsAutoStopCmd;
  BOOLEAN                    IsEmmc;
  BOOLEAN                    EnableVerboseDebug;
  UINT32                     BaseClockInMHz;
  UINT32                     CurrentClockInKHz;
  UINT32                     BlockLength;
  EFI_UNICODE_STRING_TABLE   *ControllerNameTable;
  UINT32                     ControllerVersion;
} SDHOST_DATA;

///
/// Prototypes
/// Driver model protocol interface
///
/**
  Entry point for SD Host EFI drivers.

  @param[in] ImageHandle          - EFI_HANDLE
  @param[in] SystemTable          - EFI_SYSTEM_TABLE

  @retval EFI_SUCCESS             - The function completed successfully
  @retval EFI_DEVICE_ERROR        - The function failed to complete
**/
EFI_STATUS
EFIAPI
SdHostDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

/**
  Test to see if this SD Host driver supports ControllerHandle.
  Any ControllerHandle that has installed will be supported.

  @param[in] This                - Protocol instance pointer
  @param[in] Controller          - Handle of device to test
  @param[in] RemainingDevicePath - Not used

  @retval EFI_SUCCESS            - This driver supports this device
  @retval EFI_UNSUPPORTED        - This driver does not support this device
**/
EFI_STATUS
EFIAPI
SdHostDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                  Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  );

/**
  Starting the SD Host Driver

  @param[in] This                - Protocol instance pointer
  @param[in] Controller          - Handle of device to start
  @param[in] RemainingDevicePath - Not used

  @retval EFI_SUCCESS            - This driver start this device
  @retval EFI_UNSUPPORTED        - This driver does not support this device
  @retval EFI_DEVICE_ERROR       - This driver cannot be started due to device Error
  @retval EFI_OUT_OF_RESOURCES   - This driver cannot allocate resources
**/
EFI_STATUS
EFIAPI
SdHostDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      Controller,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  );
/**
  Stop SD Host driver on ControllerHandle. Support stoping any child handles
  created by this driver.

  @param[in] This                - Protocol instance pointer
  @param[in] Controller          - Handle of device to stop driver on
  @param[in] NumberOfChildren    - Number of Children in the ChildHandleBuffer
  @param[in] ChildHandleBuffer   - List of handles for the children we need to stop

  @retval EFI_SUCCESS            - This driver stop this device
  @retval EFI_DEVICE_ERROR       - This driver cannot be stop due to device Error
**/
EFI_STATUS
EFIAPI
SdHostDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  );

//
// EFI Component Name Functions
//
/**
  Retrieves a Unicode string that is the user readable name of the EFI Driver.

  @param[in]  This       - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  Language   - A pointer to a three character ISO 639-2 language identifier.
                           This is the language of the driver name that that the caller
                           is requesting, and it must match one of the languages specified
                           in SupportedLanguages.  The number of languages supported by a
                           driver is up to the driver writer.
  @param[out] DriverName - A pointer to the Unicode string to return.  This Unicode string
                           is the name of the driver specified by This in the language
                           specified by Language.


  @retval  EFI_SUCCESS           - The Unicode string for the Driver specified by This
                                   and the language specified by Language was returned
                                   in DriverName.
  @retval  EFI_INVALID_PARAMETER - Language is NULL.
  @retval  EFI_INVALID_PARAMETER - DriverName is NULL.
  @retval  EFI_UNSUPPORTED       - The driver specified by This does not support the
                                   language specified by Language.

**/
EFI_STATUS
EFIAPI
SdHostComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL     *This,
  IN  CHAR8                           *Language,
  OUT CHAR16                          **DriverName
  );
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
  @param[in]  Language         - A pointer to a three character ISO 639-2 language
                                 identifier.  This is the language of the controller name
                                 that that the caller is requesting, and it must match one
                                 of the languages specified in SupportedLanguages.  The
                                 number of languages supported by a driver is up to the
                                 driver writer.
  @param[out]  ControllerName   - A pointer to the Unicode string to return.  This Unicode
                                  string is the name of the controller specified by
                                  ControllerHandle and ChildHandle in the language
                                  specified by Language from the point of view of the
                                  driver specified by This.


  @retval  EFI_SUCCESS           - The Unicode string for the user readable name in the
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
SdHostComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ChildHandle, OPTIONAL
  IN  CHAR8                           *Language,
  OUT CHAR16                          **ControllerName
  );

/**
  The main function used to send the command to the card inserted into the SD host
  slot.
  It will assemble the arguments to set the command register and wait for the command
  and transfer completed until timeout. Then it will read the response register to fill
  the ResponseData

  @param[in]  This           - Pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
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
SendCommand (
  IN   EFI_SD_HOST_IO_PROTOCOL    *This,
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

  @param[in]  This            - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  MaxFrequency    - Max frequency in HZ

  @retval  EFI_SUCCESS
  @retval  EFI_TIMEOUT
**/
EFI_STATUS
EFIAPI
SetClockFrequency (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  UINT32                     MaxFrequencyInKHz
  );
/**
  Set bus width of the host

  @param[in]  This       - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  BusWidth   - Bus width in 1, 4, 8 bits


  @retval  EFI_STATUS
  @retval  EFI_INVALID_PARAMETER
**/
EFI_STATUS
EFIAPI
SetBusWidth (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  UINT32                     BusWidth
  );
/**
  Set voltage which could supported by the host.
  Support 0(Power off the host), 1.8V, 3.0V, 3.3V

  @param[in]  This      - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Voltage   - Units in 0.1 V

  @retval  EFI_SUCCESS
  @retval  EFI_INVALID_PARAMETER
**/
EFI_STATUS
EFIAPI
SetHostVoltage (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  UINT32                     Voltage
  );
/**
  Set Host mode in DDR

  @param[in]  This             - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  DdrMode          - True for DDR Mode set, false returns EFI_SUCCESS

  @retval  EFI_SUCCESS
  @retval  EFI_INVALID_PARAMETER
**/
EFI_STATUS
EFIAPI
SetHostDdrMode (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN UINT32                      DdrMode
);

/**
  Set Host SDR Mode

  @param[in] This                - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in] DdrMode             - True for SDR Mode set, false for normal mode

  @retval EFI_SUCCESS            - The function completed successfully
  @retval EFI_INVALID_PARAMETER  - A parameter was incorrect.
**/
EFI_STATUS
EFIAPI
SetHostSdrMode (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN UINT32                      SdrMode
);

/**
  Set Host High Speed

  @param[in] This                - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in] HighSpeed           - True for High Speed Mode set, false for normal mode

  @retval EFI_SUCCESS            - The function completed successfully
  @retval EFI_INVALID_PARAMETER  - A parameter was incorrect.
**/
EFI_STATUS
EFIAPI
SetHostSpeedMode(
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  UINT32                     HighSpeed
  );
/**
  Reset the host

  @param[in] This                - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in] ResetAll            - TRUE to reset all

  @retval EFI_SUCCESS            - The function completed successfully
  @retval EFI_TIMEOUT            - The timeout time expired.
**/
EFI_STATUS
EFIAPI
ResetSdHost (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  RESET_TYPE                 ResetType
  );
/**
  Reset the host

  @param[in] This                - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in] ResetAll            - TRUE to reset all

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             The timeout time expired.
**/
EFI_STATUS
EFIAPI
EnableAutoStopCmd (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  BOOLEAN                    Enable
  );
/**
  Find whether these is a card inserted into the slot. If so
  init the host. If not, return EFI_NOT_FOUND.

  @param[in]  This             - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @retval  EFI_SUCCESS         - Success
  @retval  EFI_DEVICE_ERROR    - Fail
**/
EFI_STATUS
EFIAPI
DetectCardAndInitHost (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This
  );
/**
  Set the Block length

  @param[in]  This            - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  BlockLength     - card supportes block length

  @retval  EFI_SUCCESS        - Success
  @retval  EFI_DEVICE_ERROR   - Fail
**/
EFI_STATUS
EFIAPI
SetBlockLength (
  IN  EFI_SD_HOST_IO_PROTOCOL    *This,
  IN  UINT32                     BlockLength
  );
/**
  Setup Device

  @param[in]  This             - Pointer to EFI_SD_HOST_IO_PROTOCOL

  @retval  EFI_SUCCESS         - Success
  @retval  EFI_DEVICE_ERROR    - Fail
**/
EFI_STATUS
EFIAPI
SetupDevice(
             IN  EFI_SD_HOST_IO_PROTOCOL    *This
            );
/**
  Read Sd host register

  @param[in]  SdHost           - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Offset           - Offset of the SD Card
  @retval  Data                - Data read from SD Card
**/
UINT8
SdHostRead8 (
  IN     SDHOST_DATA                  *SdHost,
  IN     UINTN                        Offset
  );
/**
  Read Sd host register

  @param[in]  SdHost    - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Offset    - Offset of the SD Card

  @retval  Data         - Data read from SD Card
**/
UINT16
SdHostRead16 (
  IN     SDHOST_DATA                  *SdHost,
  IN     UINTN                        Offset
  );
/**
  Read Sd host register

  @param[in]  SdHost    - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Offset    - Offset of the SD Card

  @retval  Data         - Data read from SD Card
**/
UINT32
SdHostRead32 (
  IN     SDHOST_DATA                  *SdHost,
  IN     UINTN                        Offset
  );
/**
  Write Sd host register

  @param[in]  SdHost    - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Offset    - Offset of the SD Card
  @param[in]  Data      - Data write to SD Card
  @retval  Data  - Writen to SdHost
**/
UINT8
SdHostWrite8 (
  IN     SDHOST_DATA                  *SdHost,
  IN     UINTN                        Offset,
  IN     UINT8                        Data
  );
/**
  Write Sd host register

  @param[in]  SdHost    - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Offset    - Offset of the SD Card
  @param[in]  Data      - Data write to SD Card

  @retval  Data         - Data written to SD Card
**/
UINT16
SdHostWrite16 (
  IN     SDHOST_DATA                  *SdHost,
  IN     UINTN                        Offset,
  IN     UINT16                       Data
  );
/**
  Write Sd host register

  @param[in]  SdHost    - Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in]  Offset    - Offset of the SD Card
  @param[in]  Data      - Data write to SD Card

  @retval  Data         - Data written to SD Card
**/
UINT32
SdHostWrite32 (
  IN     SDHOST_DATA                  *SdHost,
  IN     UINTN                        Offset,
  IN     UINT32                       Data
  );

#endif
