/** @file

  This file defines the EFI SPI PPI which implements the
  Intel(R) SC SPI Host Controller Compatibility Interface.

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

@par Specification
**/
#ifndef _PEI_SPI_H_
#define _PEI_SPI_H_

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                     gScSpiPpiGuid;
//extern EFI_GUID                   gScSpiProtocolGuid;
//extern EFI_GUID                   gScSmmSpiProtocolGuid;

//
// Forward reference for ANSI C compatibility
//
typedef struct _SC_SPI_PROTOCOL  SC_SPI_PROTOCOL;

//
// SPI protocol data structures and definitions
//

///
/// Flash Region Type
///
typedef enum {
  FlashRegionDescriptor,
  FlashRegionBios,
  FlashRegionMe,
  FlashRegionGbE,
  FlashRegionPlatformData,
  FlashRegionAll,
  FlashRegionMax
} FLASH_REGION_TYPE;

//
// Protocol member functions
//

/**
  Read data from the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] FlashRegionType      The Flash Region type for flash cycle which is listed in the Descriptor.
  @param[in] Address              The Flash Linear Address must fall within a region for which BIOS has access permissions.
  @param[in] ByteCount            Number of bytes in the data portion of the SPI cycle.
  @param[out] Buffer              The Pointer to caller-allocated buffer containing the dada received.
                                  It is the caller's responsibility to make sure Buffer is large enough for the total number of bytes read.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_READ) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     FLASH_REGION_TYPE  FlashRegionType,
  IN     UINT32             Address,
  IN     UINT32             ByteCount,
  OUT    UINT8              *Buffer
  );

/**
  Write data to the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] FlashRegionType      The Flash Region type for flash cycle which is listed in the Descriptor.
  @param[in] Address              The Flash Linear Address must fall within a region for which BIOS has access permissions.
  @param[in] ByteCount            Number of bytes in the data portion of the SPI cycle.
  @param[in] Buffer               Pointer to caller-allocated buffer containing the data sent during the SPI cycle.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_WRITE) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     FLASH_REGION_TYPE  FlashRegionType,
  IN     UINT32             Address,
  IN     UINT32             ByteCount,
  IN     UINT8              *Buffer
  );

/**
  Erase some area on the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] FlashRegionType      The Flash Region type for flash cycle which is listed in the Descriptor.
  @param[in] Address              The Flash Linear Address must fall within a region for which BIOS has access permissions.
  @param[in] ByteCount            Number of bytes in the data portion of the SPI cycle.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_ERASE) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     FLASH_REGION_TYPE  FlashRegionType,
  IN     UINT32             Address,
  IN     UINT32             ByteCount
  );

/**
  Read SFDP data from the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] ComponentNumber      The Componen Number for chip select
  @param[in] ByteCount            Number of bytes in SFDP data portion of the SPI cycle, the max number is 64
  @param[out] SfdpData            The Pointer to caller-allocated buffer containing the SFDP data received
                                  It is the caller's responsibility to make sure Buffer is large enough for the total number of bytes read.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_READ_SFDP) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     UINT8              ComponentNumber,
  IN     UINT32             ByteCount,
  OUT    UINT8              *SfdpData
  );

/**
  Read Jedec Id from the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] ComponentNumber      The Componen Number for chip select
  @param[in] ByteCount            Number of bytes in JedecId data portion of the SPI cycle, the data size is 3 typically
  @param[out] JedecId             The Pointer to caller-allocated buffer containing JEDEC ID received
                                  It is the caller's responsibility to make sure Buffer is large enough for the total number of bytes read.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_READ_JEDEC_ID) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     UINT8              ComponentNumber,
  IN     UINT32             ByteCount,
  OUT    UINT8              *JedecId
  );

/**
  Write the status register in the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] ByteCount            Number of bytes in Status data portion of the SPI cycle, the data size is 1 typically
  @param[in] StatusValue          The Pointer to caller-allocated buffer containing the value of Status register writing

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_WRITE_STATUS) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     UINT32             ByteCount,
  IN     UINT8              *StatusValue
  );

/**
  Read status register in the flash part.

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] ByteCount            Number of bytes in Status data portion of the SPI cycle, the data size is 1 typically
  @param[out] StatusValue         The Pointer to caller-allocated buffer containing the value of Status register received.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_FLASH_READ_STATUS) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     UINT32             ByteCount,
  OUT    UINT8              *StatusValue
  );

/**
  Read PCH Soft Strap Values

  @param[in] This                 Pointer to the SC_SPI_PROTOCOL instance.
  @param[in] SoftStrapAddr        PCH Soft Strap address offset from FPSBA.
  @param[in] ByteCount            Number of bytes in SoftStrap data portion of the SPI cycle
  @param[out] SoftStrapValue      The Pointer to caller-allocated buffer containing PCH Soft Strap Value.
                                  It is the caller's responsibility to make sure Buffer is large enough for the total number of bytes read.

  @retval EFI_SUCCESS             Command succeed.
  @retval EFI_INVALID_PARAMETER   The parameters specified are not valid.
  @retval EFI_DEVICE_ERROR        Device error, command aborts abnormally.
**/
typedef
EFI_STATUS
(EFIAPI *SC_SPI_READ_PCH_SOFTSTRAP) (
  IN     SC_SPI_PROTOCOL   *This,
  IN     UINT32             SoftStrapAddr,
  IN     UINT32             ByteCount,
  OUT    UINT8              *SoftStrapValue
  );

/**
  These protocols/PPI allows a platform module to perform SPI operations through the
  Intel PCH SPI Host Controller Interface.
**/
struct _SC_SPI_PROTOCOL {
  SC_SPI_FLASH_READ                FlashRead;          ///< Read data from the flash part.
  SC_SPI_FLASH_WRITE               FlashWrite;         ///< Write data to the flash part.
  SC_SPI_FLASH_ERASE               FlashErase;         ///< Erase some area on the flash part.
  SC_SPI_FLASH_READ_SFDP           FlashReadSfdp;      ///< Read SFDP data from the flash part.
  SC_SPI_FLASH_READ_JEDEC_ID       FlashReadJedecId;   ///< Read Jedec Id from the flash part.
  SC_SPI_FLASH_WRITE_STATUS        FlashWriteStatus;   ///< Write the status register in the flash part.
  SC_SPI_FLASH_READ_STATUS         FlashReadStatus;    ///< Read status register in the flash part.
  SC_SPI_READ_PCH_SOFTSTRAP        ReadPchSoftStrap;   ///< Read PCH Soft Strap Values
};

#endif
