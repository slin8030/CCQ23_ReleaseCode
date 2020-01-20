/** @file

  Wrap EFI_SPI_PROTOCOL to provide some library level interfaces
  for module use.


 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/

#include <Library/SpiFlashCommon.h>

#include <Protocol/FlashDeviceInfo.h>

//
// Serial Flash device initialization data table provided to the
// Intel(R) SPI Host Controller Compatibility Interface.
//
SPI_INIT_DATA    *mSpiTable     = NULL;
UINT32            mSpiTableSize = 0;

EFI_FLASH_DEVICE_INFO_PROTOCOL  Interface;

EFI_SPI_PROTOCOL  *mSpiProtocol;

/**
  Mapping SPI_CHIP_INIT_DATA private data to SPI_INIT_DATA.
  Note that why there is the tricky code is due to SPI_PROTOCOL interface is changed.
  To make the library work cross platform, mapping private data structure is necessary.

  @param[in]  SpiFlashDevice    A point to those supported spi flash devices.

  @retval     EFI_SUCCESS       Opertion is successful.
  @retval     EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
MapSpiChipInitDataToSpiInitTable (
  IN  SPI_CHIP_INIT_DATA    *SpiFlashDevice
  )
{
  SPI_INIT_DATA    *SpiTableEntry = NULL;

  if (SpiFlashDevice->TypeDataNum == 0) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Reallocate memory for security info structure.
  //
  mSpiTable = ReallocatePool (
                mSpiTableSize,
                mSpiTableSize + sizeof (SPI_INIT_DATA),
                mSpiTable
                );

  if (mSpiTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SpiTableEntry = (SPI_INIT_DATA *)((UINTN)mSpiTable + mSpiTableSize);

  CopyMem (SpiTableEntry, SpiFlashDevice, sizeof(SPI_CHIP_INIT_DATA));

  SpiTableEntry->BiosSize = (UINTN)PcdGet32 (PcdFlashAreaSize);
  mSpiTableSize += sizeof (SPI_INIT_DATA);
  return EFI_SUCCESS;
}

/**
  Register supported spi flash device info.

  @param[in]  SpiFlashDevice       A point to those supported spi flash devices.

  @retval EFI_SUCCESS              The handlers were registered successfully.
**/
EFI_STATUS
EFIAPI
RegisterSupportedFlashDevice (
  IN  SPI_CHIP_INIT_DATA    *SpiFlashDevice
  )
{
  EFI_STATUS Status;

  if (SpiFlashDevice == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = MapSpiChipInitDataToSpiInitTable (SpiFlashDevice);

  return Status;
}

/**
  Test if it's supported spi flash device.

  @retval EFI_SUCCESS              The tested spi flash device is supported.
  @retval EFI_UNSUPPORTED          The tested spi flash device is not supported.

**/
EFI_STATUS
EFIAPI
SpiFlashInit (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT8                           SfId[3];
  UINT8                           FlashIndex;
  EFI_HANDLE                      Handle = NULL;
  UINT8                           Number;
  UINT8                           Index;

  //
  // Auto-detect the flash part by stepping through the table of supported flash
  // parts, running ICH init and issuing ReadId command. If no matching flash
  // part is found return failure (though system is unlikely to boot).
  //
  for (FlashIndex = 0; FlashIndex < mSpiTableSize / sizeof (SPI_INIT_DATA); FlashIndex++) {
    Status = mSpiProtocol->Init (mSpiProtocol, &(mSpiTable[FlashIndex]));
    if (!EFI_ERROR (Status)) {
      //
      // Read Vendor/Device IDs to check if the driver supports the Serial Flash device
      //
      Status = mSpiProtocol->Execute (
                               mSpiProtocol,
                               SPI_READ_ID,
                               SPI_WREN,
                               TRUE,
                               FALSE,
                               FALSE,
                               0,
                               3,
                               SfId,
                               EnumSpiRegionAll
                               );
      if (!EFI_ERROR (Status)) {
        if (SfId[0] == mSpiTable[FlashIndex].VendorId) {
          Number = mSpiTable[FlashIndex].TypeDataNum;
            for (Index = 0; Index < Number; Index++ ) {
              if ((SfId[1] == mSpiTable[FlashIndex].TypeData[Index].DeviceId0) &&
                    (SfId[2] == mSpiTable[FlashIndex].TypeData[Index].DeviceId1)) {
                //
                // Found a matching SPI device, FlashIndex now contains flash device
                //
                DEBUG ((EFI_D_INFO, "Found SPI Flash Type in SPI Flash Driver\n"));
                DEBUG ((EFI_D_INFO, "  Device Vendor ID = 0x%02x!\n", SfId[0]));
                DEBUG ((EFI_D_INFO, "  Device Type ID 0 = 0x%02x!\n", SfId[1]));
                DEBUG ((EFI_D_INFO, "  Device Type ID 1 = 0x%02x!\n", SfId[2]));
                break;
            }
          }
        }
      }
    }
  }

  if (FlashIndex < mSpiTableSize / sizeof (SPI_INIT_DATA))  {
    //
    // BUGBUG: add a workaround to produce a private protocol for Sv.
    //
    Interface.Size = mSpiTableSize;
    Interface.Base = mSpiTable;
    Status =  gBS->InstallProtocolInterface (
                 &Handle,
                 &gEfiFlashDeviceInfoProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &Interface
                 );
    ASSERT_EFI_ERROR (Status);
    return EFI_SUCCESS;
  } else {
//    return EFI_UNSUPPORTED;
    return EFI_SUCCESS;

  }
}

/**
  Read the Serial Flash Status Registers.

  @param[in]      SpiStatus         Pointer to a caller-allocated UINT8. On successful return,
                                    it contains the status data read from the Serial Flash Status Register.

  @retval         EFI_SUCCESS       Operation success, status is returned in SpiStatus.
  @retval         EFI_DEVICE_ERROR  The block device is not functioning correctly and the operation failed.

**/
EFI_STATUS
ReadStatusRegister (
  UINT8   *SpiStatus
  )
{
  EFI_STATUS          Status;

  Status = mSpiProtocol->Execute (
             mSpiProtocol,
             SPI_RDSR,
             SPI_WREN,
             TRUE,
             FALSE,
             FALSE,
             0,
             1,
             SpiStatus,
             EnumSpiRegionBios
           );

  return Status;
}

/**
  Enable or disable block protection on the Serial Flash device.

  @param[in]  Lock              TRUE to lock. FALSE to unlock.

  @retval     EFI_SUCCESS       Opertion is successful.
  @retval     EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
SpiFlashLock (
  IN    BOOLEAN                   Lock
  )
{
  EFI_STATUS          Status;
  UINT8               SpiData;
  UINT8               SpiStatus;

  if (Lock) {
    SpiData = SF_SR_WPE;
  } else {
    SpiData = 0;
  }

  SpiData = 0;
  Status = mSpiProtocol->Execute (
                           mSpiProtocol,
                           SPI_WRSR,
                           SPI_EWSR,
                           TRUE,
                           TRUE,
                           TRUE,
                           0,
                           1,
                           &SpiData,
                           EnumSpiRegionBios
                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ReadStatusRegister (&SpiStatus);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((SpiStatus & SpiData) != SpiData) {
    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  Read NumBytes bytes of data from the address specified by
  PAddress into Buffer.

  @param[in]      Address       The starting physical address of the read.
  @param[in,out]  NumBytes      On input, the number of bytes to read. On output, the number
                                of bytes actually read.
  @param[out]     Buffer        The destination data buffer for the read.

  @retval         EFI_SUCCESS       Opertion is successful.
  @retval         EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
SpiFlashRead (
  IN     UINTN                        Address,
  IN OUT UINT32                       *NumBytes,
     OUT UINT8                        *Buffer
  )
{
  ASSERT ((NumBytes != NULL) && (Buffer != NULL));
  //
  // This function is implemented specifically for those platforms
  // at which the SPI device is memory mapped for read. So this
  // function just do a memory copy for Spi Flash Read.
  //
  CopyMem (Buffer, (VOID *) Address, *NumBytes);

  return EFI_SUCCESS;
}

/**
  Write NumBytes bytes of data from Buffer to the address specified by
  PAddresss.

  @param[in]      Address         The starting physical address of the write.
  @param[in,out]  NumBytes        On input, the number of bytes to write. On output,
                                  the actual number of bytes written.
  @param[in]      Buffer          The source data buffer for the write.

  @retval         EFI_SUCCESS       Opertion is successful.
  @retval         EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
SpiFlashWrite (
  IN     UINTN                      Address,
  IN OUT UINT32                     *NumBytes,
  IN     UINT8                      *Buffer
  )
{
  EFI_STATUS                Status;
  UINTN                     Offset;
  UINT32                    Length;
  UINT32                    RemainingBytes;

  ASSERT ((NumBytes != NULL) && (Buffer != NULL));
  ASSERT (Address >= (UINTN)PcdGet32 (PcdFlashAreaBaseAddress));

  Offset    = Address - (UINTN)PcdGet32 (PcdFlashAreaBaseAddress);

  ASSERT ((*NumBytes + Offset) <= (UINTN)PcdGet32 (PcdFlashAreaSize));

  Status = EFI_SUCCESS;
  RemainingBytes = *NumBytes;

  while (RemainingBytes > 0) {
    if (RemainingBytes > SECTOR_SIZE_4KB) {
      Length = SECTOR_SIZE_4KB;
    } else {
      Length = RemainingBytes;
    }
    Status = mSpiProtocol->Execute (
                             mSpiProtocol,
                             SPI_PROG,
                             SPI_WREN,
                             TRUE,
                             TRUE,
                             TRUE,
                             (UINT32) Offset,
                             Length,
                             Buffer,
                             EnumSpiRegionBios
                             );
    if (EFI_ERROR (Status)) {
      break;
    }
    RemainingBytes -= Length;
    Offset += Length;
    Buffer += Length;
  }

  //
  // Actual number of bytes written
  //
  *NumBytes -= RemainingBytes;

  return Status;
}

/**
  Erase the block starting at Address.

  @param[in]  Address         The starting physical address of the block to be erased.
                              This library assume that caller garantee that the PAddress
                              is at the starting address of this block.
  @param[in]  NumBytes        On input, the number of bytes of the logical block to be erased.
                              On output, the actual number of bytes erased.

  @retval     EFI_SUCCESS.      Opertion is successful.
  @retval     EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
SpiFlashBlockErase (
  IN    UINTN                     Address,
  IN    UINTN                     *NumBytes
  )
{
  EFI_STATUS          Status;
  UINTN               Offset;
  UINTN               RemainingBytes;

  ASSERT (NumBytes != NULL);
  ASSERT (Address >= (UINTN)PcdGet32 (PcdFlashAreaBaseAddress));

  Offset    = Address - (UINTN)PcdGet32 (PcdFlashAreaBaseAddress);

  ASSERT ((*NumBytes % SECTOR_SIZE_4KB) == 0);
  ASSERT ((*NumBytes + Offset) <= (UINTN)PcdGet32 (PcdFlashAreaSize));

  Status = EFI_SUCCESS;
  RemainingBytes = *NumBytes;

  while (RemainingBytes > 0) {
    Status = mSpiProtocol->Execute (
                             mSpiProtocol,
                             SPI_SERASE,
                             SPI_WREN,
                             FALSE,
                             TRUE,
                             FALSE,
                             (UINT32) Offset,
                             0,
                             NULL,
                             EnumSpiRegionBios
                             );
    if (EFI_ERROR (Status)) {
      break;
    }
    RemainingBytes -= SECTOR_SIZE_4KB;
    Offset         += SECTOR_SIZE_4KB;
  }

  //
  // Actual number of bytes erased
  //
  *NumBytes -= RemainingBytes;

  return Status;
}

