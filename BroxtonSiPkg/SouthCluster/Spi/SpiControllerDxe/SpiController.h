/** @file
  Function Header file for SPI command transactions

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

@par Specification Reference:
**/
#ifndef _SPI_CONTROLLER_H_
#define _SPI_CONTROLLER_H_
#include <Uefi.h>
#include <Library/SpiTargetSettings.h>
#include <Protocol/SpiHost.h>


extern UINT32 SpiMaxReadBytes;
extern UINT32 SpiMaxWriteBytes;
extern UINT32 SpiMaxTransmitBytes;

/**
  Execute Spi Write

  @param [in] pRegisters                A pointer to SPI_PLATFORM_CONTEXT instance
  @param [in] pTargetSettings           A pointer to SPI_TARGET_SETTINGS instance.
  @param [in] writeBuffer               Buffer contains data to write
  @param [in] writeBytes                Specifies size to write
  @param [in, out] pBytesWritten        Actual bytes written
  @param [in] readBuffer                Buffer to store read data
  @param [in] readBytes                 Specifies size to read
  @param [in, out] pBytesRead           Actual bytes read

  @retval EFI_STATUS                    Spi Write Successfully
  @retval Others                        Spi Write failure
**/
EFI_STATUS
SpiDoWrite (
  IN SPI_REGISTERS                       *pRegisters,
  IN SPI_TARGET_SETTINGS                 *pTargetSettings,
  IN VOID                                *writeBuffer,
  IN UINT32                              writeBytes,
  IN OUT UINT32                          *pBytesWritten,
  IN VOID                                *readBuffer,
  IN UINT32                              readBytes,
  IN OUT UINT32                          *pBytesRead
);

/**
  Execute Spi Read

  @param [in] pRegisters                 A pointer to SPI_PLATFORM_CONTEXT instance
  @param [in] pTargetSettings            A pointer to SPI_TARGET_SETTINGS instance.
  @param [in] readBuffer                 Buffer to store read data
  @param [in] readBytes                  Specifies size to read
  @param [in, out] pBytesRead            Actual bytes read

  @retval EFI_STATUS                     Spi Read Successfully
  @retval Others                         Spi Read failure
**/
EFI_STATUS
SpiDoRead (
  IN SPI_REGISTERS                       *pRegisters,
  IN SPI_TARGET_SETTINGS                 *pTargetSettings,
  IN VOID                                *readBuffer,
  IN UINT32                              readBytes,
  IN OUT UINT32                          *pBytesRead
);

/**
  Execute SPI Reset

  @param [in] platformData    A pointer to SPI_PLATFORM_CONTEXT instance.
**/
VOID SpiReset (
  IN VOID                     *platformData
);

/**
  Execute SPI command Transfer

  @param [in] platformData              A pointer to SPI_PLATFORM_CONTEXT instance.
  @param [in] pTargetSettings           A pointer to SPI_TARGET_SETTINGS instance.
  @param [in, out] RequestPacket        A pointer to EFI_SPI_REQUEST_PACKET instance.
  @param [out] SpiStatus                Status returned by spi write command

  @retval EFI_STATUS                    Spi command transfer suscessfully
  @retval Others                        Spi command transfer failure
**/
EFI_STATUS
SpiTransfer (
  IN VOID                               *platformData,
  IN SPI_TARGET_SETTINGS                *pTargetSettings,
  IN OUT EFI_SPI_REQUEST_PACKET         *RequestPacket,
  OUT EFI_STATUS                        *SpiStatus
);

#endif
