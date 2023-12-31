/** @file
  Block IO protocol as defined in the UEFI 2.0 specification.

  The Block IO protocol is used to abstract block devices like hard drives,
  DVD-ROMs and floppy drives.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

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

#ifndef __PEI_BLOCK_IO_H__
#define __PEI_BLOCK_IO_H__
// {BC5FA650-EDBB-4d0d-B3A3-D98907F847DF}
#define PEI_BLOCK_IO_PPI_GUID \
  {  \
    0xbc5fa650, 0xedbb, 0x4d0d, { 0xb3, 0xa3, 0xd9, 0x89, 0x7, 0xf8, 0x47, 0xdf }  \
  }

typedef struct _PEI_BLOCK_IO_PPI  PEI_BLOCK_IO_PPI;


/**
  Reset the Block Device.

  @param[in]  This                 Indicates a pointer to the calling context.
  @param[in]  ExtendedVerification Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS          The device was reset.
  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could
                               not be reset.

**/
typedef
EFI_STATUS
(EFIAPI *PEI_BLOCK_RESET)(
  IN PEI_BLOCK_IO_PPI               *This,
  IN BOOLEAN                        ExtendedVerification
  );

/**
  Read BufferSize bytes from Lba into Buffer.

  @param[in]   PeiServices
  @param[in]   This       Indicates a pointer to the calling context.
  @param[in]   MediaId    Id of the media, changes every time the media is replaced.
  @param[in]   Lba        The starting Logical Block Address to read from
  @param[in]   BufferSize Size of Buffer, must be a multiple of device block size.
  @param[out]  Buffer     A pointer to the destination buffer for the data. The caller is
                          responsible for either having implicit or explicit ownership of the buffer.

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the read.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.

**/
typedef
EFI_STATUS
(EFIAPI *PEI_BLOCK_READ)(
  IN  EFI_PEI_SERVICES              **PeiServices,
  IN PEI_BLOCK_IO_PPI               *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  );

/**
  Write BufferSize bytes from Lba into Buffer.

  @param[in]  PeiServices
  @param[in]  This        Indicates a pointer to the calling context.
  @param[in]  MediaId     The media ID that the write request is for.
  @param[in]  Lba         The starting logical block address to be written. The caller is
                          responsible for writing to only legitimate locations.
  @param[in]  BufferSize  Size of Buffer, must be a multiple of device block size.
  @param[in]  Buffer      A pointer to the source buffer for the data.

  @retval EFI_SUCCESS           The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED   The device can not be written to.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the write.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.

**/
typedef
EFI_STATUS
(EFIAPI *PEI_BLOCK_WRITE)(
  IN  EFI_PEI_SERVICES              **PeiServices,
  IN PEI_BLOCK_IO_PPI               *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  );

/**
  Flush the Block Device.

  @param[in]  This          Indicates a pointer to the calling context.

  @retval EFI_SUCCESS       All outstanding data was written to the device
  @retval EFI_DEVICE_ERROR  The device reported an error while writting back the data
  @retval EFI_NO_MEDIA      There is no media in the device.

**/
typedef
EFI_STATUS
(EFIAPI *PEI_BLOCK_FLUSH)(
  IN  PEI_BLOCK_IO_PPI           *This
  );

/**
  Block IO read only mode data and updated only via members of BlockIO
**/
typedef struct {
  ///
  /// The curent media Id. If the media changes, this value is changed.
  ///
  UINT32  MediaId;

  ///
  /// TRUE if the media is removable; otherwise, FALSE.
  ///
  BOOLEAN RemovableMedia;

  ///
  /// TRUE if there is a media currently present in the device;
  /// othersise, FALSE. THis field shows the media present status
  /// as of the most recent ReadBlocks() or WriteBlocks() call.
  ///
  BOOLEAN MediaPresent;

  ///
  /// TRUE if LBA 0 is the first block of a partition; otherwise
  /// FALSE. For media with only one partition this would be TRUE.
  ///
  BOOLEAN LogicalPartition;

  ///
  /// TRUE if the media is marked read-only otherwise, FALSE.
  /// This field shows the read-only status as of the most recent WriteBlocks () call.
  ///
  BOOLEAN ReadOnly;

  ///
  /// TRUE if the WriteBlock () function caches write data.
  ///
  BOOLEAN WriteCaching;

  ///
  /// The intrinsic block size of the device. If the media changes, then
  /// this field is updated.
  ///
  UINT32  BlockSize;

  ///
  /// Supplies the alignment requirement for any buffer to read or write block(s).
  ///
  UINT32  IoAlign;

  ///
  /// The last logical block address on the device.
  /// If the media changes, then this field is updated.
  ///
  EFI_LBA LastBlock;

  ///
  /// Only present if EFI_BLOCK_IO_PROTOCOL.Revision is greater than or equal to
  /// EFI_BLOCK_IO_PROTOCOL_REVISION2. Returns the first LBA is aligned to
  /// a physical block boundary.
  ///
  EFI_LBA LowestAlignedLba;

  ///
  /// Only present if EFI_BLOCK_IO_PROTOCOL.Revision is greater than or equal to
  /// EFI_BLOCK_IO_PROTOCOL_REVISION2. Returns the number of logical blocks
  /// per physical block.
  ///
  UINT32 LogicalBlocksPerPhysicalBlock;

  ///
  /// Only present if EFI_BLOCK_IO_PROTOCOL.Revision is greater than or equal to
  /// EFI_BLOCK_IO_PROTOCOL_REVISION3. Returns the optimal transfer length
  /// granularity as a number of logical blocks.
  ///
  UINT32 OptimalTransferLengthGranularity;
} PEI_BLOCK_IO_MEDIA;

#define EFI_BLOCK_IO_PROTOCOL_REVISION  0x00010000
#define EFI_BLOCK_IO_PROTOCOL_REVISION2 0x00020001
#define EFI_BLOCK_IO_PROTOCOL_REVISION3 0x00020031

///
/// Revision defined in EFI1.1.
///
#define EFI_BLOCK_IO_INTERFACE_REVISION   EFI_BLOCK_IO_PROTOCOL_REVISION

///
///  This protocol provides control over block devices.
///
struct _PEI_BLOCK_IO_PPI {
  ///
  /// The revision to which the block IO interface adheres. All future
  /// revisions must be backwards compatible. If a future version is not
  /// back wards compatible, it is not the same GUID.
  ///
  UINT64              Revision;
  ///
  /// Pointer to the EFI_BLOCK_IO_MEDIA data for this device.
  ///
  PEI_BLOCK_IO_MEDIA  *Media;
  PEI_BLOCK_RESET     Reset;
  PEI_BLOCK_READ      ReadBlocks;
  PEI_BLOCK_WRITE     WriteBlocks;
  PEI_BLOCK_FLUSH     FlushBlocks;
};

//extern EFI_GUID gEfiBlockIoProtocolGuid;
extern EFI_GUID gPeiBlockIoPpiGuid;
#endif
