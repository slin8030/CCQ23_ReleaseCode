/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#ifndef _COMPAL_FWRECOVERY_PEIM_H_
#define _COMPAL_FWRECOVERY_PEIM_H_

//
// Following includes should be replaced by standard PEIM type headers
//
#include "CompalNTFSApi.h"
#include "CompalNTFSFmt.h"

//
// Definitions
//

#define PEI_NTFS_CACHE_SIZE        4
#define PEI_NTFS_MAX_BLOCK_SIZE    8192
#define NTFS_MAX_FILE_NAME_LENGTH  128
#define NTFS_MAX_FILE_PATH_LENGTH  256
#define PEI_NTFS_MAX_BLOCK_DEVICE  64
#define PEI_NTFS_MAX_BLOCK_IO_PPI  32
#define PEI_NTFS_MAX_VOLUME        64
#define PEI_NTFS_MAX_FILE          256
#define PEI_NTFS_MEMMORY_PAGE_SIZE 0x1000
#define MaxFileNameLength 128
#define MaxDataRunNumber 142
#define MaxINDXDataRun 32
#define FirstAttrOffset  0x14
#define RealFileRecordSize  0x18

//
// Constant usually apprears
//
#define BytesPerPage 512
#define HighestBit   0x80
#define HighestFourBits 0xf0
#define LowestFourBits 0xf
#define SixBytes  0xffffffffffff
#define SecondtoLastByte 0x1FE
#define LastByte 0x1FF

//
// MFT FILE Header Offset defined in NTFS specification
//
#define MFT_UPDATE_SEQ_SIZE  0x06  // The offset of Size in words of Update Sequence Number & Array
#define MFT_UPDATE_SEQ_NO    0x30  // The offset of Update Sequence Number
#define MFT_UPDATE_SEQ_ARRAY 0x32  // The offset of Update Sequence Array
#define MFT_FILE_RECORD_SIZE 0x1C  // The offset of allocated size of the MFT FILE record

//
// INDEX FILE Header Offset defined in NTFS specification
//
#define INDX_UPDATE_SEQ_SIZE  0x06  // The offset of Size in words of Update Sequence Number & Array
#define INDX_UPDATE_SEQ_NO    0x28  // The offset of Update Sequence Number
#define INDX_UPDATE_SEQ_ARRAY 0x2A  // The offset of Update Sequence Array
#define INDX_FILE_RECORD_SIZE 0x20  // The offset of allocated size of the INDX FILE record

//
// INDEX record (entry) header Offset defined in NTFS specification
//
#define INDX_ENTRY_SIZE  0x08  // The offset of Size of this index entry
#define FILENAME_LENGTH  0x50  // The offset of Length of filename
#define FILENAME_OFFSET  0x52  // The offset of filename

//
// Attribute Offset defined in NTFS specification
//
#define RESIDENT_ATTR_NAME     0x18  // The offset of a resident attribute's name
#define NONRESIDENT_ATTR_NAME  0x40  // The offset of a nonresident attribute's name

//
// Data Structures
//
//
// The block device structure for transfer device information
//
typedef struct {
    UINT32                          BlockSize;
    UINT64                          LastBlock;
    UINT32                          IoAlign;
    BOOLEAN                         Logical;
    BOOLEAN                         PartitionChecked;

    //
    // Following fields only valid for logical device
    //
    CHAR8                           PartitionFlag[8];
    UINT64                          StartingPos;
    UINTN                           ParentDevNo;

    //
    // Following fields only valid for physical device
    //
    EFI_PEI_BLOCK_DEVICE_TYPE       DevType;
    //
    // PEI_READ_BLOCKS             ReadFunc;
    //
    EFI_PEI_RECOVERY_BLOCK_IO_PPI   *BlockIo;
    UINT8                           PhysicalDevNo;
} PEI_NTFS_BLOCK_DEVICE;
//
// the Volume structure
//

typedef struct {
    UINTN         BlockDeviceNo;
    UINTN         VolumeNo;
    UINT64        VolumeSize;
    UINTN         MaxCluster;
    CHAR16        VolumeLabel[NTFS_MAX_FILE_NAME_LENGTH];
    PEI_NTFS_TYPE  NTFSType;
    UINT64        NTFSPos;
    UINT32        SectorSize;
    UINT32        ClusterSize;
    UINT64        FirstClusterPos;
} PEI_NTFS_VOLUME;
//
// File instance
//

typedef struct {
    BOOLEAN         Valid;

    PEI_NTFS_VOLUME  *Volume;
    CHAR16          FileName[NTFS_MAX_FILE_NAME_LENGTH];
    UINT64          DirEntryPos;
    UINT64          ParentStartingCluster;

    BOOLEAN         IsRootDir;
    BOOLEAN         IsFixedRootDir;

    UINT32          StartingCluster;
    UINT64          CurrentPos;
    UINT32          StraightReadAmount;
    UINT32          CurrentCluster;

    UINT8           Attributes;
    UINTN          FileSize;
    CHAR16          FilePath[NTFS_MAX_FILE_PATH_LENGTH];
} PEI_NTFS_FILE;



//
// Cache Buffer
//

typedef struct {
    BOOLEAN Valid;
    UINTN   BlockDeviceNo;
    UINT64  Lba;
    UINT32  Lru;
    UINT64  Buffer[PEI_NTFS_MAX_BLOCK_SIZE / 8];
    UINTN   Size;
} PEI_NTFS_CACHE_BUFFER;
//
// Private Data.
// This structure abstracts the whole memory usage in NTFS PEIM.
// The entry point routine will get a chunk of memory (by whatever
// means) whose size is sizeof(PEI_NTFS_PRIVATE_DATA), which is clean
// in both 32 and 64 bit environment. The boundary of the memory chunk
// should be 64bit aligned.
//

#define PEI_NTFS_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('n', 't', 'f', 's')

typedef struct {
    UINTN                                 Signature;
    EFI_PEI_SERVICES                      **PeiServices;
    EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    DeviceRecoveryPpi;
    EFI_PEI_PPI_DESCRIPTOR                PpiDescriptor;
    EFI_PEI_NOTIFY_DESCRIPTOR             NotifyDescriptor;

    UINT8                                 UnicodeCaseMap[0x300];
    CHAR8                                 *EngUpperMap;
    CHAR8                                 *EngLowerMap;
    CHAR8                                 *EngInfoMap;

    UINT64                                BlockData[PEI_NTFS_MAX_BLOCK_SIZE / 8];
    UINTN                                 BlockDeviceCount;
    PEI_NTFS_BLOCK_DEVICE                  BlockDevice[PEI_NTFS_MAX_BLOCK_DEVICE];
    UINTN                                 VolumeCount;
    PEI_NTFS_VOLUME                       Volume[PEI_NTFS_MAX_VOLUME];
    PEI_NTFS_FILE                         File[PEI_NTFS_MAX_FILE];
    PEI_NTFS_CACHE_BUFFER                 CacheBuffer[PEI_NTFS_CACHE_SIZE];
    VOID 									*NTFSBuffer;
} PEI_NTFS_PRIVATE_DATA;
#define PEI_NTFS_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
          PEI_NTFS_PRIVATE_DATA, \
          DeviceRecoveryPpi, \
          PEI_NTFS_PRIVATE_DATA_SIGNATURE \
      )

//
// Extract INT32 from char array
//
#define UNPACK_INT32(a) \
  (INT32) ((((UINT8 *) a)[0] << 0) | (((UINT8 *) a)[1] << 8) | (((UINT8 *) a)[2] << 16) | (((UINT8 *) a)[3] << 24))

//
// Extract UINT32 from char array
//
#define UNPACK_UINT32(a) \
  (UINT32) ((((UINT8 *) a)[0] << 0) | (((UINT8 *) a)[1] << 8) | (((UINT8 *) a)[2] << 16) | (((UINT8 *) a)[3] << 24))

//
// Cross used funtions
//


//
// API functions
//
UINTN
GetNumberOfVolumes (
    IN  PEI_NTFS_PRIVATE_DATA  *PrivateData
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PrivateData - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
GetNumberRecoveryCapsules (
    IN EFI_PEI_SERVICES                               **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI             *This,
    OUT UINTN                                         *NumberRecoveryCapsules
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PeiServices             - GC_TODO: add argument description
  This                    - GC_TODO: add argument description
  NumberRecoveryCapsules  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
GetRecoveryCapsuleInfo (
    IN  EFI_PEI_SERVICES                              **PeiServices,
    IN  EFI_PEI_DEVICE_RECOVERY_MODULE_PPI            *This,
    IN  UINTN                                         CapsuleInstance,
    OUT UINTN                                         *Size,
    OUT EFI_GUID                                      *CapsuleType
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PeiServices     - GC_TODO: add argument description
  This            - GC_TODO: add argument description
  CapsuleInstance - GC_TODO: add argument description
  Size            - GC_TODO: add argument description
  CapsuleType     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
LoadRecoveryCapsule (
    IN OUT EFI_PEI_SERVICES                         **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI           *This,
    IN UINTN                                        CapsuleInstance,
    OUT VOID                                        *Buffer
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PeiServices     - GC_TODO: add argument description
  This            - GC_TODO: add argument description
  CapsuleInstance - GC_TODO: add argument description
  Buffer          - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
NTFSReadBlock (
    IN PEI_NTFS_PRIVATE_DATA   *PrivateData,
    IN UINTN                  BlockDeviceNo,
    IN UINT64                 CHS,
    IN UINTN                  BufferSize,
    IN VOID                   *Buffer
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PrivateData   - GC_TODO: add argument description
  BlockDeviceNo - GC_TODO: add argument description
  LBA           - GC_TODO: add argument description
  BufferSize    - GC_TODO: add argument description
  Buffer        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
NTFSGetBpbInfo (
    IN PEI_NTFS_PRIVATE_DATA              *PrivateData,
    IN OUT	PEI_NTFS_VOLUME		  *Volume
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PrivateData - GC_TODO: add argument description
  Volume      - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
NTFSReadDisk (
    IN  PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN  UINTN                 BlockDeviceNo,
    IN  UINT64                StartingAddress,
    IN  UINTN                 Size,
    OUT VOID                  *Buffer
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PrivateData     - GC_TODO: add argument description
  BlockDeviceNo   - GC_TODO: add argument description
  StartingAddress - GC_TODO: add argument description
  Size            - GC_TODO: add argument description
  Buffer          - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
NTFSFindPartitions (
    IN  PEI_NTFS_PRIVATE_DATA  *PrivateData
)
/*++

Routine Description:

  This function finds partitions (logical devices) in physical block devices.

Arguments:

  NULL

Returns:

  VOID.

--*/
;

EFI_STATUS
DataRunGetData(
    IN OUT UINT8 *DataRun_HeaderPtr,
    IN UINT64 firstPartitionOffset,
    IN OUT UINT64 *DataRunSizeArrayPtr,
    IN OUT UINT64 *DataRunOffsetArrayPtr,
    IN OUT UINT32 *DataRunIndexCount,
    IN OUT UINT32 SectorsPerCluster
)
/*++

Routine Description:

  analyze the data run procdure

Arguments:

  DataRun_HeaderPtr      - data run header pointer
  firstPartitionOffset   - partition offset
  DataRunSizeArrayPtr    - data run size array pointer
  DataRunOffsetArrayPtr - data run offset array pointer
  DataRunIndexCount     - data run counter pointer
  SectorsPerCluster        - the number of pages of a cluster (a page unit is 512 bytes)

Returns:

  EFI_SUCCESS - get success or failure of data run

--*/
;

EFI_STATUS
NTFSGetRecoveryFile(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN OUT CHAR16                *FileName,
    IN OUT PEI_FILE_INFO         *FileInfo
)
/*++

Routine Description:

  search the target file name in NTFS storage

Arguments:

  PrivateData  -
  VolumeIndx - volume index number
  FileName     - File path pointer
  FileInfo        - file information structure

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
;

EFI_STATUS
GetTargetPartition(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN OUT UINT64 *TargetPartitionOffset
)
/*++

Routine Description:

  get the target partition entry sector

Arguments:

  PrivateData             - GC_TODO: add argument description
  VolumeIndx            - volume index number
  TargetPartitionOffset - first parition offset pointer

Returns:

  EFI_SUCCESS - get or doesn`t get the partition

--*/
;

UINT64
SearchFileRec(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN OUT UINT8 *FileNameArrayPtr,
    IN OUT UINT16 *FileNameArrayLocationPtr,
    IN OUT UINT64 *MFT_AddressPtr,
    IN OUT UINT64 *TargetPartitionOffsetPtr,
    IN OUT UINT64 *TargetMFTNoPtr,
    IN OUT UINT8 *GetTargetFileMFTPtr,
    IN OUT UINT32 SectorsPerCluster
)
/*++

Routine Description:

  Search file name in NTFS storage procdure

Arguments:

  *PrivateData                      - private data
  VolumeIndx                        - volume index number
  *FileNameArrayPtr              - file name array pointer
  *FileNameArrayLocationPtr  - file name array location pointer
  *MFT_AddressPtr              - $MFT address pointer
  *TargetPartitionOffsetPtr       - partition offset pointer
  *TargetMFTNoPtr                - traget MFT number pointer
  *GetTargetFileMFTPtr          - current MFT number pointer
    SectorsPerCluster        - the number of pages of a cluster (a page unit is 512 bytes)

Returns:

  UINT64 - return the MFT number

--*/
;

EFI_STATUS
GetFileNameLevel(
    IN OUT UINT8 *FileNameArrayPtr,
    IN OUT UINT16 *FileNameArrayLocationPtr,
    IN OUT UINT16 *TargetFileNameArrayPtr,
    IN OUT UINT16 *TargetFileNameArrayCountPtr
)
/*++

Routine Description:

  GC_TODO:

Arguments:

  FileNameArrayPtr                   - file name pointer
  FileNameArrayLocationPtr       - point to file name array location
  TargetFileNameArrayPtr          - file name array pointer
  TargetFileNameArrayCountPtr - file name counter pointer

Returns:

  EFI_SUCCESS - if doesn`t get the correctly file name

--*/
;

UINT16
SearchTargetBodyType(
    IN OUT UINT64 TargetBodyType,
    IN OUT UINT16 BufferLength,
    IN OUT UINT8 *TargetBodyTypeAddrPtr
)
/*++

Routine Description:

  Search the MFT table body type

Arguments:

  TargetBodyType              - target body type
  BufferLength                   - Buffer length
  *TargetBodyTypeAddrPtr - pointer to buffer header

Returns:

  UINT16  - target body type offset

--*/
;

UINT64
SearchFileNameInIndexTbl(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN OUT UINT8 *TargetFileNameArray,
    IN OUT UINT16 *TargetFileNameArrayCountPtr,
    IN OUT UINT64 *DataRunSizeArray,
    IN OUT UINT64 *DataRunOffsetArray,
    IN OUT UINT32 *DataRunCount
)
/*++

Routine Description:

  GC_TODO: Search file name in Index table

Arguments:

  *PrivateData                           -  Private Data
  VolumeIndx                             - Volume index number
  *TargetFileNameArray              - Target file name array pointer
  *TargetFileNameArrayCountPtr - Target file name counter pointer
  *DataRunSizeArray                  - Data run size array
  *DataRunOffsetArray               - Data run offset array
  *DataRunCount                       - Data run counter

Returns:

  UINT64 - Target MFT number

--*/
;

UINT8
CompareFileName(
    IN OUT UINT8 *TargetFileNameArray,
    IN OUT UINT16 *TargetFileNameArrayCountPtr,
    IN OUT UINT8 *CurrentFileNamePtr,
    IN OUT UINT8 *CurrentFileNameLength
)
/*++

Routine Description:

  GC_TODO: Compare the file Name

Arguments:

  *TargetFileNameArray              - Target file name array pointer
  *TargetFileNameArrayCountPtr - Target file name counter pointer
  *CurrentFileNamePtr                - Current file name array pointer
  *CurrentFileNameLength           - Current file name counter pointer

Returns:

  UINT8 - 0 => get the target file name
               2 => doesn`t get the target file name

--*/
;

UINT64
SearchFileNameInIndexTbl_90(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN OUT UINT8 *TargetFileNameArray,
    IN OUT UINT16 *TargetFileNameArrayCountPtr,
    IN OUT UINT8 *CurrentPtr
)
/*++

Routine Description:

  GC_TODO: Search file name in Index table

Arguments:

  *PrivateData                           -  Private Data
  VolumeIndx                             - Volume index number
  *TargetFileNameArray              - Target file name array pointer
  *TargetFileNameArrayCountPtr - Target file name counter pointer

Returns:

  UINT64 - Target MFT number

--*/
;

UINT64
MathPow(
    IN UINT32 Base,
    IN UINT32 Exp
)
/*
  Multiplies a 32-bit unsigned integer Base by a 32-bit unsigned integer Exp times and
  generates a 64-bit unsigned result. It also means Base to the power of Exp.

  @param  Base  A 32-bit unsigned value.
  @param  Exp   A 32-bit unsigned value.

  @return Result = Base^Exp.
*/
;

EFI_STATUS
UpdateSeqData(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN UINT64 Address,
    IN UINT32  Remainder,
    IN OUT UINT8 *UpdateSeqArray,
    IN OUT UINT16 UpdateSeqNo,
    IN OUT UINT16 UpdateSeqArraySize
)
/*
Routine Description:

  Get Update Sequence Array in Current Index FILE

Arguments:

  PrivateData             - GC_TODO: add argument description
  VolumeIndx              - volume index number
  Address                 - the pointed address
  Remainder               - the sector order in current index file of the pointed address
  UpdateSeqArray          - Update sequence array
  UpdateSeqNo             - The update sequence number
  UpdateSeqArraySize      - Size in words of Update Sequence Number & Array

Returns:

  EFI_SUCCESS - get success or failure of the Update Sequence Array
*/
;

// ===== {BENSON} Start: 03/27/2012 ===== //
CHAR16
ToUpper (
    CHAR16  a
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  a - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;
// ===== {BENSON}   End: 03/27/2012 ===== //

#endif // _FAT_PEIM_H_
