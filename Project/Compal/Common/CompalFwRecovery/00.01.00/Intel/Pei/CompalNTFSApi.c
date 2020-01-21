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

#include <Ppi/Stall.h>
#include "CompalNTFSPeim.h"
#include "CompalMbr.h"
#include "CompalGpt.h"
#include "CompalNTFSFwRecoveryGuid.h"

#define PEI_NTFS_RECOVERY_FILE (CHAR16 *)PcdGetPtr(PcdPeiRecoveryFile)

PEI_NTFS_PRIVATE_DATA  *mPrivateData = NULL;

STATIC
EFI_STATUS
UpdateBlocksAndVolumes (
  PEI_NTFS_PRIVATE_DATA            *PrivateData
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PrivateData - private pointer

Returns:

  EFI_SUCCESS - get success of failure storage volume

--*/
{
    EFI_STATUS                      Status;
    EFI_PEI_PPI_DESCRIPTOR          *TempPpiDescriptor;
    UINTN                           BlockIoPpiInstance;
    EFI_PEI_RECOVERY_BLOCK_IO_PPI   *BlockIoPpi;
    UINTN                           NumberBlockDevices;
    UINTN                           Index;
    EFI_PEI_BLOCK_IO_MEDIA          Media;
    PEI_NTFS_VOLUME                 Volume;
    EFI_PEI_SERVICES              **PeiServices;

    PeiServices = (EFI_PEI_SERVICES **) GetPeiServicesTablePointer ();

    //
    // Clean up caches
    //
    for (Index = 0; Index < PEI_NTFS_CACHE_SIZE; Index++) {
        PrivateData->CacheBuffer[Index].Valid = FALSE;
    }

    PrivateData->BlockDeviceCount = 0;

    //
    // Find out all Block Io Ppi instances within the system
    // Assuming all device Block Io Peims are dispatched already
    //
    for (BlockIoPpiInstance = 0; BlockIoPpiInstance < PEI_NTFS_MAX_BLOCK_IO_PPI; BlockIoPpiInstance++) {
        Status = (**PeiServices).LocatePpi (
                     PeiServices,
                     &gEfiPeiVirtualBlockIoPpiGuid,
                     BlockIoPpiInstance,
                     &TempPpiDescriptor,
                     &BlockIoPpi
                 );
        if (EFI_ERROR (Status)) {
            //
            // Done with all Block Io Ppis
            //
            break;
        }

        //
        // Device count
        //
        Status = BlockIoPpi->GetNumberOfBlockDevices (
                     PeiServices,
                     BlockIoPpi,
                     &NumberBlockDevices
                 );
        if (EFI_ERROR (Status)) {
            continue;
        }

        //
        // How many devices and the data of devices (not only NTFS, we can use for NTFS)
        //
        for (Index = 0; Index < NumberBlockDevices && PrivateData->BlockDeviceCount < PEI_NTFS_MAX_BLOCK_DEVICE; Index++) {

            Status = BlockIoPpi->GetBlockDeviceMediaInfo (
                         PeiServices,
                         BlockIoPpi,
                         Index,
                         &Media
                     );
            if (EFI_ERROR (Status) || !Media.MediaPresent) {
                continue;
            }

            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].BlockSize = (UINT32) Media.BlockSize;
            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].LastBlock = Media.LastBlock;

            if (Media.DeviceType == MaxDeviceType) {
                PrivateData->BlockDevice[PrivateData->BlockDeviceCount].IoAlign   = 4;
            } else {
                PrivateData->BlockDevice[PrivateData->BlockDeviceCount].IoAlign   = 0;
            }

            //
            // Not used here
            //
            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].Logical           = FALSE;
            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].PartitionChecked  = FALSE;

            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].BlockIo           = BlockIoPpi;
            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].PhysicalDevNo     = (UINT8) Index;
            PrivateData->BlockDevice[PrivateData->BlockDeviceCount].DevType           = Media.DeviceType;

            PrivateData->BlockDeviceCount++;
        }
    }

    //
    // Find out all logical devices
    //
    NTFSFindPartitions (PrivateData);

    //
    // Build up file system volume array
    //
    PrivateData->VolumeCount = 0;
    for (Index = 0; Index < PrivateData->BlockDeviceCount; Index++) {
        Volume.BlockDeviceNo  = Index;
        Status                = NTFSGetBpbInfo (PrivateData, &Volume);
        if (EFI_ERROR (Status))
            break;

        if (Volume.NTFSType == NTFS) {
            //
            // Add the detected volume to the volume array
            //
            CopyMem ( (UINT8 *) &(PrivateData->Volume[PrivateData->VolumeCount]),
                      (UINT8 *) &Volume,
                      sizeof (PEI_NTFS_VOLUME) );

            PrivateData->VolumeCount += 1;
            if (PrivateData->VolumeCount >= PEI_NTFS_MAX_VOLUME) {
                break;
            }
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BlockIoNotifyEntry (
    IN EFI_PEI_SERVICES           **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
    IN VOID                       *Ppi
)
/*++

Routine Description:

  BlockIo installation nofication function. Find out all the current BlockIO
  PPIs in the system and add them into private data. Assume there is

Arguments:

  PeiServices - General purpose services available to every PEIM.

Returns:

  Status -  EFI_SUCCESS if Ok

--*/
// GC_TODO:    NotifyDescriptor - add argument and description to function comment
// GC_TODO:    Ppi - add argument and description to function comment
{
    UpdateBlocksAndVolumes (mPrivateData);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CompalNTFSPeimEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
)
/*++

Routine Description:

  Installs the Device Recovery Module PPI, Initialize BlockIo Ppi
  installation notification

Arguments:

  PeiServices - General purpose services available to every PEIM.

Returns:

  Status -  EFI_SUCCESS if Ok

--*/
// GC_TODO:    FfsHeader - add argument and description to function comment
// GC_TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// GC_TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
{
    EFI_STATUS            Status;
    PEI_NTFS_PRIVATE_DATA *PrivateData;
    EFI_PHYSICAL_ADDRESS  Address;

//__debugbreak();
//    PrivateData = (PEI_NTFS_PRIVATE_DATA *) AllocateZeroPool (sizeof (PEI_NTFS_PRIVATE_DATA));
//    if (PrivateData == NULL) {
//      return EFI_OUT_OF_RESOURCES;
//    }

    Status = (**PeiServices).AllocatePages (
                            PeiServices,
                            EfiBootServicesCode,
                            (sizeof (PEI_NTFS_PRIVATE_DATA) - 1) / PEI_NTFS_MEMMORY_PAGE_SIZE + 1,
                            &Address
                            );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    PrivateData = (PEI_NTFS_PRIVATE_DATA *) (UINTN) Address;

    //
    // Initialize Private Data (to zero, as is required by subsequent operations)
    //
    (*PeiServices)->SetMem ((UINT8 *) PrivateData, sizeof (PEI_NTFS_PRIVATE_DATA), 0);

    //
    // Initialize Private Data (to zero, as is required by subsequent operations)
    //
    PrivateData->Signature    = PEI_NTFS_PRIVATE_DATA_SIGNATURE;
    PrivateData->PeiServices  = (EFI_PEI_SERVICES **)PeiServices;

    //
    // Installs Ppi
    //
    PrivateData->DeviceRecoveryPpi.GetNumberRecoveryCapsules  = GetNumberRecoveryCapsules;
    PrivateData->DeviceRecoveryPpi.GetRecoveryCapsuleInfo     = GetRecoveryCapsuleInfo;
    PrivateData->DeviceRecoveryPpi.LoadRecoveryCapsule        = LoadRecoveryCapsule;

    PrivateData->PpiDescriptor.Flags                          = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
    PrivateData->PpiDescriptor.Guid = &gEfiPeiDeviceRecoveryModulePpiGuid;
    PrivateData->PpiDescriptor.Ppi = &PrivateData->DeviceRecoveryPpi;

    Status = (**PeiServices).InstallPpi (PeiServices, &PrivateData->PpiDescriptor);
    if (EFI_ERROR (Status))
        return Status;


    //
    // Other initializations
    //
    PrivateData->BlockDeviceCount = 0;

    UpdateBlocksAndVolumes (PrivateData);

    //
    // PrivateData is allocated now, set it to the module variable
    //
    mPrivateData = PrivateData;

    //
    // Installs Block Io Ppi notification function
    //
    PrivateData->NotifyDescriptor.Flags =
        (
            EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK |
            EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST
        );
    PrivateData->NotifyDescriptor.Guid    = &gEfiPeiVirtualBlockIoPpiGuid;
    PrivateData->NotifyDescriptor.Notify  = BlockIoNotifyEntry;

    return (**PeiServices).NotifyPpi (PeiServices, &PrivateData->NotifyDescriptor);
}

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

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{

    PEI_NTFS_PRIVATE_DATA  *PrivateData;
    UINTN                 VolumeCount;
    UINTN                 RecoveryCapsuleCount;

    PrivateData               = PEI_NTFS_PRIVATE_DATA_FROM_THIS (This);
    PrivateData->PeiServices  = PeiServices;

    //
    // Search each volume in the root directory for the Recovery capsule
    //
    VolumeCount           = GetNumberOfVolumes (PrivateData);
    RecoveryCapsuleCount  = 0;

    if (VolumeCount == 1) {
        RecoveryCapsuleCount++;
    }

    *NumberRecoveryCapsules = RecoveryCapsuleCount;

    return EFI_SUCCESS;
}

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

  EFI_OUT_OF_RESOURCES - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value

--*/
{
    EFI_STATUS            Status;
    PEI_NTFS_PRIVATE_DATA  *PrivateData;
    UINTN                 Index;
    UINTN                 VolumeCount;
    UINTN                 RecoveryCapsuleCount;
    PEI_FILE_INFO         *FileInfo;
    EFI_PHYSICAL_ADDRESS  Address;
    UINTN                 BufferSize;

    PrivateData               = PEI_NTFS_PRIVATE_DATA_FROM_THIS (This);
    PrivateData->PeiServices  = PeiServices;

//    FileInfo = (PEI_FILE_INFO *) AllocateZeroPool (sizeof (PEI_FILE_INFO));
//    if (FileInfo == NULL) {
//        return EFI_OUT_OF_RESOURCES;
//    }
    BufferSize                = sizeof (PEI_FILE_INFO) + NTFS_MAX_FILE_PATH_LENGTH * sizeof (CHAR16);
    Status = (**PeiServices).AllocatePages (
                            (CONST EFI_PEI_SERVICES **)PeiServices,
                            EfiBootServicesCode,
                            (BufferSize - 1) / PEI_NTFS_MEMMORY_PAGE_SIZE + 1,
                            &Address
                            );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    FileInfo = (PEI_FILE_INFO *) (UINTN) Address;

    //
    // Search each volume in the root directory for the Recovery capsule
    //
    VolumeCount           = GetNumberOfVolumes (PrivateData);
    RecoveryCapsuleCount  = 0;

    Index=0;

    Status = NTFSGetRecoveryFile(PrivateData, Index, PEI_NTFS_RECOVERY_FILE, FileInfo);

    if (EFI_ERROR (Status))
        return Status;

    CopyMem ( (UINT8 *) CapsuleType,
              (UINT8 *) &gCompalNTFSFwRecoveryGuid,
              sizeof (EFI_GUID) );

    *Size = (UINTN) FileInfo->FileSize;

    return EFI_SUCCESS;

}

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

  EFI_OUT_OF_RESOURCES - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value

--*/
{

    EFI_STATUS            Status;
    PEI_NTFS_PRIVATE_DATA  *PrivateData;
    UINT32 DataRunArrayCount;
    UINT32 TotalFileSize;
    PrivateData               = PEI_NTFS_PRIVATE_DATA_FROM_THIS (This);
    PrivateData->PeiServices  = PeiServices;

//#ifdef USE_FAST_CRISIS_RECOVERY
//  EFI_HOB_MEMORY_ALLOCATION_MODULE	*MemoryHob;
//
//  MemoryHob = NULL;
//#endif

    DataRunArrayCount = 0;
    TotalFileSize = 0;
    Status = 0;

    //
    // Combine all seperated Data
    //
    while (PrivateData->File[0].FileSize != 0)
    {

        Status = NTFSReadDisk (
                     PrivateData,
                     0,
                     PrivateData->File[DataRunArrayCount].CurrentPos,
                     PrivateData->File[DataRunArrayCount].FileSize,
                     ((UINT8 *)Buffer) + TotalFileSize
                 );
        TotalFileSize = TotalFileSize + PrivateData->File[DataRunArrayCount].FileSize;
        DataRunArrayCount++;
        if (Status != 0)
            return EFI_SUCCESS;
    }

//#ifdef USE_FAST_CRISIS_RECOVERY
//      if (Status == EFI_SUCCESS) {
//        //
//        //Create a Hob transfer the .fd memory base address and ROM part PEI start address to BDS
//        //
//        Status = (*PeiServices)->CreateHob (
//                                      PeiServices,
//                                      EFI_HOB_TYPE_MEMORY_ALLOCATION,
//                                      (UINT16) (sizeof (EFI_HOB_MEMORY_ALLOCATION_MODULE)),
//                                      &MemoryHob
//                                      );
//          if (EFI_ERROR (Status)) {
//            return Status;
//          }
//          MemoryHob->MemoryAllocationHeader.MemoryBaseAddress = (UINTN) Buffer;
//          MemoryHob->MemoryAllocationHeader.Name = gEfiRecoveryFileAddressGuid;
//        }
//#endif    

    return 0;
}

UINTN
GetNumberOfVolumes (
    IN  PEI_NTFS_PRIVATE_DATA  *PrivateData
)
/*++

Routine Description:

  Get the volume number in current storage

Arguments:

  PrivateData - GC_TODO: add argument description

Returns:

  UINTN - number of volume in current storage

--*/
{
    return PrivateData->VolumeCount;
}

EFI_STATUS
DataRunGetData(
    IN OUT UINT8 *DataRun_HeaderPtr,
    IN UINT64 firstPartitionOffset,
    IN OUT UINT64 *DataRunSizeArrayPtr,
    IN OUT UINT64 *DataRunOffsetArrayPtr,
    IN OUT UINT32 *DataRunIndexCount,
    IN OUT UINT32  SectorsPerCluster
)
/*++

Routine Description:

  analyze the data run procdure

Arguments:

  DataRun_HeaderPtr      - data run header pointer
  firstPartitionOffset    - partition offset
  DataRunSizeArrayPtr    - data run size array pointer
  DataRunOffsetArrayPtr  - data run offset array pointer
  DataRunIndexCount      - data run counter pointer
  SectorsPerCluster      - the number of pages of a cluster (a page unit is 512 bytes)

Returns:

  EFI_SUCCESS - get success or failure of data run

--*/
{
    EFI_STATUS Status;
    UINT64 *DataRunOffsetAddr;
    UINT8 DataRunOffsetArraytmp[8];
    UINT8 DataRunSizeArraytmp[8];
    UINT8 DataRunOffsetLength;
    UINT8 DataRunSizeLength;
    UINT32 *DataRun_ptr2;
    UINT64 *DataRunSize;	// Unit:Cluster
    UINT8 DataRuntmp;
    UINT32 DataRunCount;
    UINT64 DataRunSizeSector;
    UINT64 DataRunOffset;
    UINT64 DataRunOffsetLCN;
    UINT64 DataRunOffsetLCN_previous;
    UINT8 DataRunHeader;
    UINT64 NegCheck; // Check if the data run offset value is negative
    UINT64 ResDigit; // Reserved digit of data run Offset for transfering to 2's complement if data run offset is negative

    //
    // Data initialization
    //
    Status = EFI_SUCCESS;
    DataRunHeader = *DataRun_HeaderPtr;
    DataRunCount=0;
    DataRunOffsetLCN_previous = firstPartitionOffset;

    while (DataRunHeader!=0x00) {

        //
        // Clean the buffer
        //
        for (DataRuntmp=0; DataRuntmp<8; DataRuntmp++) {
            DataRunOffsetArraytmp[DataRuntmp]=0;
            DataRunSizeArraytmp[DataRuntmp]=0;
        }

        //
        // Get the length of each element in data run size & offset array from data run header
        //
        DataRunOffsetLength = (DataRunHeader & HighestFourBits) >> 4;
        DataRunSizeLength = DataRunHeader & LowestFourBits;

        //
        // Calculate offset to next data run header
        //
        DataRuntmp = DataRunOffsetLength + DataRunSizeLength + 1;

        //
        // If data run size or offset overflow return error
        //
        if ((DataRunSizeLength > 8) || (DataRunOffsetLength > 8)) {
            return EFI_DEVICE_ERROR;
        }
        else {

            //
            // Get data run size
            //
            while (DataRunSizeLength>0) {
                DataRun_ptr2 = (UINT32 *)((UINT8 *)DataRun_HeaderPtr + DataRunSizeLength);
                DataRunSizeArraytmp[DataRunSizeLength-1] = (UINT8)*DataRun_ptr2;
                DataRunSizeLength--;
            }

            DataRun_ptr2 = (UINT32 *)&DataRunSizeArraytmp[0];
            DataRunSize = (UINT64 *)DataRun_ptr2;
            DataRunSizeLength = DataRunHeader & LowestFourBits;

            //
            // Get data run offset
            //
            while (DataRunOffsetLength>0) {
                DataRun_ptr2 = (UINT32 *)((UINT8 *)DataRun_HeaderPtr + DataRunOffsetLength + DataRunSizeLength);
                DataRunOffsetArraytmp[DataRunOffsetLength-1] = (UINT8)*DataRun_ptr2;
                DataRunOffsetLength--;
            }

            DataRun_ptr2 = (UINT32 *)&DataRunOffsetArraytmp[0];
            DataRunOffsetAddr = (UINT64 *)DataRun_ptr2;

            //
            // Transfer Data run size from cluster to page unit, which is 512 bytes (Here a page is a sector)
            //
            DataRunSizeSector =  MultU64x32(*DataRunSize, SectorsPerCluster);
            *(DataRunSizeArrayPtr+DataRunCount) = DataRunSizeSector;
            DataRunOffset = *DataRunOffsetAddr;

            //
            // Determine if the DataRunOffset is negative (The highest digit of negative integer is 1)
            //
            DataRunOffsetLength = (DataRunHeader & HighestFourBits) >> 4;
            NegCheck = (HighestBit<<((DataRunOffsetLength-1)*8));
            ResDigit = MathPow( 2, (UINT32)(DataRunOffsetLength*8)) - 1;

            if ((DataRunOffset & NegCheck) == NegCheck)
            {
                //
                // DataRunOffset is negative, and transfer the value to 2's complement
                // The address of this data run pointing is the previous data run address subtracts this 2's complement
                //
                DataRunOffset = ((~DataRunOffset) + 1) & ResDigit;
                DataRunOffsetLCN = MultU64x32(DataRunOffset, SectorsPerCluster);
                DataRunOffsetLCN_previous -= DataRunOffsetLCN;
            }
            else
            {
                //
                // DataRunOffset is positive
                // Transfer Data run offset from cluster to page unit, which is 512 bytes (Here a page is a sector)
                // The address of this data run pointing is the previous data run address adds this data run offset
                //
                DataRunOffsetLCN = MultU64x32(DataRunOffset, SectorsPerCluster);
                DataRunOffsetLCN_previous += DataRunOffsetLCN;
            }

            *(DataRunOffsetArrayPtr+DataRunCount) = DataRunOffsetLCN_previous;
            DataRunCount++;
            *DataRunIndexCount = DataRunCount;
        }

        //
        // Point to next data run header
        //
        DataRun_HeaderPtr = ((UINT8 *)DataRun_HeaderPtr + DataRuntmp);
        DataRunHeader = *DataRun_HeaderPtr;
    }

    return Status;
}


EFI_STATUS
GetTargetPartition(
    IN OUT PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT UINTN                 VolumeIndx,
    IN OUT UINT64        *TargetPartitionOffset
)
/*++

Routine Description:

  get the target partition entry sector

Arguments:

  PrivateData             - GC_TODO: add argument description
  VolumeIndx              - volume index number
  TargetPartitionOffset    - first parition offset pointer

Returns:

  EFI_SUCCESS - get or doesn`t get the partition

--*/
{
    EFI_STATUS                     Status = EFI_SUCCESS;
    MASTER_BOOT_RECORD             NTFSMBR;
    UINT8                          PartitionIndex = 0;
    EFI_PARTITION_TABLE_HEADER     PartitionHdr;
    GPT_PARTITION_TABLE            GPTPartitionTBL;
    EFI_GUID                       gEfiWindowsPartTypeGuid = EFI_WINDOWS_PART_TYPE_GUID;

    //
    // Get MBR Information
    //
    Status = NTFSReadDisk ( PrivateData,
                            VolumeIndx,
                            0,
                            sizeof (MASTER_BOOT_RECORD),
                            &NTFSMBR );
    //
    // Check MBR Signature
    //
    if ( NTFSMBR.Signature != 0xAA55 )    // 0xAA55
    {
        return EFI_DEVICE_ERROR;
    }

    if ( NTFSMBR.Partition[0].OSIndicator == PMBR_GPT_PARTITION ) {

        //
        // Check EFI Partition Valid
        //

        Status = NTFSReadDisk (
                     PrivateData,
                     VolumeIndx,
                     PRIMARY_PART_HEADER_LBA,
                     sizeof (EFI_PARTITION_TABLE_HEADER),
                     &PartitionHdr
                 );

        if ( EFI_ERROR (Status) ) {
            return Status;
        }

        if ( CompareMem ( &PartitionHdr.Header.Signature, EFI_PTAB_HEADER_ID, sizeof (UINT64) ) != 0 ) {
            return EFI_NOT_FOUND;
        }

        //
        // Read the EFI Partition Entries
        //

        Status = NTFSReadDisk (
                     PrivateData,
                     VolumeIndx,
                     PartitionHdr.PartitionEntryLBA,
                     sizeof (GPT_PARTITION_TABLE),
                     &GPTPartitionTBL
                 );


        if ( EFI_ERROR (Status) ) {
            return Status;
        }

        for ( PartitionIndex = 0; PartitionIndex < 8 ; PartitionIndex++ ) {
            if ( CompareGuid (&GPTPartitionTBL.EFIPartition[ PartitionIndex ].PartitionTypeGUID, &gEfiWindowsPartTypeGuid) ) {
                *TargetPartitionOffset = GPTPartitionTBL.EFIPartition[ PartitionIndex ].StartingLBA;
                break;
            }
        }
    } else {
        for ( PartitionIndex = 0; PartitionIndex < 4 ; PartitionIndex++ ) {
            if ( NTFSMBR.Partition[ PartitionIndex ].OSIndicator == PartitionNTFSType ) {
                *TargetPartitionOffset = (UINT64) ( UNPACK_UINT32( NTFSMBR.Partition[ PartitionIndex ].StartingLBA ) );
                break;
            }
        }
    }

    return Status;
}

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
{
    EFI_STATUS Status;
    NTFSPartitionTable TargetPartitionInfo;
    UINT64 TargetPartitionOffset;
    UINT64 *TargetPartitionOffsetPtr;
    NTFS_MFT_Buff NTFS_MFT_Buffer;
    UINT32 SectorsPerCluster;
    UINT64 MFT_Address;
    UINT64 *MFT_AddressPtr;
    UINT64 TargetMFTNo;
    UINT64 TargetMFTAddr;       // Target MFT address in sector
    UINT64 *TargetMFTNoPtr;
    UINT8 FileNameArray[MaxFileNameLength]={0};
    UINT8 *FileNameArrayPtr;
    UINT16 FileNameArrayLocation;
    UINT16 *FileNameArrayLocationPtr;
    UINT32 FileNameCount;
    UINT8 GetTargetFileMFT;
    UINT8 *GetTargetFileMFTPtr;
    UINT8 *TargetFileAddrPtr;
    UINT64 TargetFileSize;
    UINT64 TargetFileAddr;
    UINT16 TargetBodyOffset;
    UINT64 DataRunOffsetArray[MaxDataRunNumber]={0};
    UINT64 DataRunSizeArray[MaxDataRunNumber]={0};
    UINT32 DataRunCount;
    UINT32 DataRunArrayCount;
    UINT64 TotalFileSize;
    UINT8  UpdateSeqArray[4];    // Update sequence array
    UINT16 UpdateSeqArraySize;   // Size in words of Update Sequence Number & Array
    UINT16 UpdateSeqCount;       // The count of update sequence array

    //
    // Data initialization
    //
    Status = EFI_SUCCESS;
    GetTargetFileMFT = 0;
    GetTargetFileMFTPtr = &GetTargetFileMFT;

    //
    // Get MBR information and get the target partition table offset
    //
    TargetPartitionOffsetPtr = (UINT64 *)&TargetPartitionOffset;

    Status = GetTargetPartition(PrivateData, VolumeIndx, TargetPartitionOffsetPtr);

    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Read the partition information
    //
    Status = NTFSReadDisk (
                 PrivateData,
                 VolumeIndx,
                 TargetPartitionOffset,
                 sizeof (NTFSPartitionTable),
                 &TargetPartitionInfo
             );

    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Check NTFS signature
    // If this HDD is not NTFS partition, return error
    //
    if (TargetPartitionInfo.Signature!='SFTN') {
        return EFI_LOAD_ERROR;
    }

    //
    // Get the address of $MFT and the number of sectors per cluster
    //
    SectorsPerCluster = ((UINT32)TargetPartitionInfo.SectorPerCluster*(UINT32)TargetPartitionInfo.BytePerSector)/BytesPerPage;  //The unit of function NTFSReadDisk is 512 bytes
    MFT_Address = MultU64x32(TargetPartitionInfo.MFTAddr, SectorsPerCluster) + TargetPartitionOffset;
    MFT_AddressPtr = (UINT64 *)&MFT_Address;

    //
    // Set the target number to 5 which is the MFT number of $root
    //
    TargetMFTNo=5;
    TargetMFTNoPtr = (UINT64 *)&TargetMFTNo;

    //
    // Analyze File Path set in PeiNTFS.inf
    //
    for (FileNameCount=0; FileNameCount<MaxFileNameLength; FileNameCount++) {
        if (*(FileName+FileNameCount+1)=='\\') {
            FileNameArray[FileNameCount*2] = 0xee;
            FileNameArray[FileNameCount*2+1] = 0;
        }
        else if (*(FileName+FileNameCount+1)==0) {
            FileNameArray[FileNameCount*2]=(UINT8)*(FileName+FileNameCount);
            FileNameArray[FileNameCount*2+1] = 0;
            FileNameArray[FileNameCount*2+2] = 0xff;
            FileNameCount=MaxFileNameLength;
        }
        else {
            FileNameArray[FileNameCount*2]=(UINT8)*(FileName+FileNameCount);
            FileNameArray[FileNameCount*2+1]=0;
        }
    }

    FileNameArrayPtr = (UINT8 *)&FileNameArray[0];
    FileNameArrayLocation = 0;
    FileNameArrayLocationPtr = &FileNameArrayLocation;

    //
    // Get the target MFT FILE address of the recovery file rom
    //
    TargetMFTAddr = SearchFileRec(PrivateData, VolumeIndx, FileNameArrayPtr, FileNameArrayLocationPtr, MFT_AddressPtr, TargetPartitionOffsetPtr, TargetMFTNoPtr, GetTargetFileMFTPtr, SectorsPerCluster);

    //
    // Read the target MFT FILE
    //
    Status = NTFSReadDisk (
                 PrivateData,
                 VolumeIndx,
                 TargetMFTAddr,
                 sizeof (NTFS_MFT_Buff),
                 &NTFS_MFT_Buffer
             );

    //
    // The last 2 bytes in each sector are filled by the sequence number used to check if this sector is correctly written (Each MFT FILE has the same value)
    // Get the original values of the last 2 bytes in each sector from 0x32, and store them in UpdateSeqArray
    // Fill in UpdateSeqArray to their original addresses in order(the last 2 bytes in each sector)
    //
    UpdateSeqArraySize = *(UINT16 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_SIZE];
    for (UpdateSeqCount = 0; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
    {
        UpdateSeqArray[UpdateSeqCount*2] = *(UINT8 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_ARRAY+2*UpdateSeqCount];
        UpdateSeqArray[UpdateSeqCount*2+1] = *(UINT8 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_ARRAY+(2*UpdateSeqCount)+1];
        if ((UpdateSeqCount*sizeof (NTFS_Buffer_Sector)) < sizeof (NTFS_MFT_Buff))
        {
            if ( *(UINT16 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_NO] == *(UINT16 *)&NTFS_MFT_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount])
            {
                //
                // The sequence number of each MFT FILE stored in 0x30 (2 bytes)
                // Check if the last 2 bytes in each sector are the same with the sequence number
                //
                *(UINT8 *)&NTFS_MFT_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2];
                *(UINT8 *)&NTFS_MFT_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2+1];
            }
        }
    }

    //
    // Get the attribute 0x80 for target recovery file address and size
    //
    TargetFileAddrPtr = (UINT8 *)&NTFS_MFT_Buffer.data[0];
    TargetBodyOffset = SearchTargetBodyType(0x80,sizeof(NTFS_MFT_Buff),TargetFileAddrPtr);

    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    TargetFileAddrPtr = TargetFileAddrPtr + TargetBodyOffset;

    //
    // Determine whether this attribute 0x80 is Non-resident or not
    // If this attribute is non-resident (non-resident flag is 1), the data run starts from 0x40 plus twice of the attribute's name length
    // Else (resident), the data run starts from 0x18 plus twice of the attribute's name length
    //
    if (*(TargetFileAddrPtr+8)==1) {
        TargetFileAddrPtr= TargetFileAddrPtr + NONRESIDENT_ATTR_NAME + *(TargetFileAddrPtr+9)*2;
    }
    else {
        TargetFileAddrPtr= TargetFileAddrPtr + RESIDENT_ATTR_NAME + *(TargetFileAddrPtr+9)*2;
    }

    //
    // Analyze data run
    //
    DataRunCount = 0;
    Status = DataRunGetData(TargetFileAddrPtr, *TargetPartitionOffsetPtr, &DataRunSizeArray[0], &DataRunOffsetArray[0], &DataRunCount, SectorsPerCluster);

    //
    // Transfer file size of data run array from cluster to sector and summarize the recovery file size
    //
    TotalFileSize = 0;
    DataRunArrayCount = 0;

    while (DataRunSizeArray[DataRunArrayCount] != 0)
    {
        TargetFileAddr = DataRunOffsetArray[DataRunArrayCount];
        TargetFileSize = MultU64x32(DataRunSizeArray[DataRunArrayCount], BytesPerPage);

        PrivateData->File[DataRunArrayCount].CurrentPos = TargetFileAddr;
        PrivateData->File[DataRunArrayCount].FileSize = (UINTN)TargetFileSize;
        TotalFileSize = TotalFileSize + TargetFileSize;
        DataRunArrayCount++;
    }
    FileInfo->FileSize = TotalFileSize;

    return Status;
}


UINT16
SearchTargetBodyType(
    IN OUT UINT64 TargetBodyType,
    IN OUT UINT16 BufferLength,
    IN OUT UINT8 *TargetBodyTypeAddrPtr
)
/*++

Routine Description:

  Search the MFT FILE body type (attribute)

Arguments:

  TargetBodyType             - target body type
  BufferLength               - Buffer length
  *TargetBodyTypeAddrPtr     - pointer to buffer header

Returns:

  UINT16  BufferCount        - The offset of target attribute is relative to TargetBodyTypeAddrPtr

--*/
{
    EFI_STATUS Status;
    UINT64 BodyType;
    UINT32 BodySize;
    UINT8 CheckFlag;
    UINT32 BufferCount;
    Status = EFI_SUCCESS;

    CheckFlag = 0;
    BufferCount = 0;

    //
    // Check the MFT signature
    //
    if (*(UINT32 *)TargetBodyTypeAddrPtr !='ELIF') {
        return 0;
    }

    //
    // Get the real File Record size
    //
    BufferLength = *(UINT16 *)(TargetBodyTypeAddrPtr + RealFileRecordSize);

    //
    // Get the first attribute offset at 0x14
    //
    TargetBodyTypeAddrPtr +=FirstAttrOffset;
    BodySize = *(UINT16 *)TargetBodyTypeAddrPtr;
    BufferCount+=(UINT16)BodySize;
    BodyType = *(UINT32 *)((UINT8 *)TargetBodyTypeAddrPtr + (UINT8)BodySize - FirstAttrOffset);

    //
    // Point to the first attribute
    //
    TargetBodyTypeAddrPtr = TargetBodyTypeAddrPtr - FirstAttrOffset + BodySize;

    //
    // Determine if this attribute is the target one
    // If not, get the next attribute start by adding this attribute bodysize
    // Output of TargetBodyTypeAddrPtr is the absolute address of target attribute
    //
    while (CheckFlag!=0xff) {
        if (TargetBodyType == BodyType) {
            CheckFlag=0xff;
        }
        else {
            BodySize = *(UINT32 *)(TargetBodyTypeAddrPtr+4);
            BufferCount+=(UINT16)BodySize;
            TargetBodyTypeAddrPtr = TargetBodyTypeAddrPtr + BodySize;
            BodyType = *(UINT32 *)TargetBodyTypeAddrPtr;
        }
        if (BufferCount>=BufferLength) {
            return 0;
        }
    }

    return (UINT16)BufferCount;
}


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

  *PrivateData                   - private data
  VolumeIndx                     - volume index number
  *FileNameArrayPtr              - file name array pointer
  *FileNameArrayLocationPtr      - file name array location pointer
  *MFT_AddressPtr                - MFT address pointer
  *TargetPartitionOffsetPtr       - partition offset pointer
  *TargetMFTNoPtr                - traget MFT number pointer
  *GetTargetFileMFTPtr           - current MFT number pointer
  SectorsPerCluster              - the number of pages of a cluster (a page unit is 512 bytes)

Returns:

  UINT64 - return the target MFT address

--*/
{
    EFI_STATUS Status;
    UINT64 TargetMFTNo;
    UINT8 TargetFileNameArray[MaxFileNameLength]={0};
    UINT8 *TargetFileNameArrayPtr;
    UINT16 TargetFileNameArrayCount;
    UINT16 *TargetFileNameArrayCountPtr;
    UINT8 CheckFlag;
    UINT16 TargetBodyOffset;
    NTFS_MFT_Buff NTFS_MFT_Buffer;
    UINT8 *TargetFileBodyTypePtr;
    UINT64 DataRunSizeArray[MaxINDXDataRun]={0};
    UINT64 DataRunOffsetArray[MaxINDXDataRun]={0};
    UINT32 DataRunCount;
    // Because a single sector could fail, it's important for NTFS to be able to detect errors
    // For this purpose the sectors have Fixups, which are kept in an Update Sequence Array.
    UINT8  UpdateSeqArray[4];    // Update sequence array
    UINT16 UpdateSeqArraySize;   // Size in words of Update Sequence Number & Array
    UINT16 UpdateSeqCount;       // The count of update sequence array
    UINT64 MFTSizeCount;         // The count of MFT size in sector
    UINT8  DataRunOrder = 0;     // The element order of datarun array
    UINT64 MFTDataRunSize[MaxINDXDataRun]={0};    // Store the size of MFT FILEs in $MFT
    UINT64 MFTDataRunOffset[MaxINDXDataRun]={0};  // Store the start address of MFT FILEs in $MFT
    UINT32 MFTDataRunCount;
    UINT64 MFTNoSector;      // The MFT number transfer to sector
    UINT32 PagesPerMFT;       // The number of page per MFT FILE (unit is the same as function NTFSReadDisk 512 bytes)

    //
    // Data initialization
    //
    Status = EFI_SUCCESS;
    CheckFlag = 0;
    TargetMFTNo = *TargetMFTNoPtr;

    //
    // Get the Target file Name from while file Path
    //
    TargetFileNameArrayCount = 0;
    TargetFileNameArrayPtr = (UINT8 *)&TargetFileNameArray[0];

    //
    // Read $MFT FILE
    // Determine if this address is the position for MFT FILE by FILE signature
    //
    Status = NTFSReadDisk (
                 PrivateData,
                 VolumeIndx,
                 *MFT_AddressPtr,
                 sizeof (NTFS_MFT_Buff),
                 &NTFS_MFT_Buffer
             );

    if (*(UINT32 *)&NTFS_MFT_Buffer.data[0] !='ELIF') {
        return EFI_DEVICE_ERROR;
    }

    //
    // The last 2 bytes in each sector are filled by the sequence number used to check if this sector is correctly written (Each MFT FILE has the same value)
    // Get the original values of the last 2 bytes in each sector from 0x32, and store them in UpdateSeqArray
    // Fill in UpdateSeqArray to their original addresses in order(the last 2 bytes in each sector)
    //
    UpdateSeqArraySize = *(UINT16 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_SIZE];
    for (UpdateSeqCount = 0; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
    {
        UpdateSeqArray[UpdateSeqCount*2] = *(UINT8 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_ARRAY+2*UpdateSeqCount];
        UpdateSeqArray[UpdateSeqCount*2+1] = *(UINT8 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_ARRAY+(2*UpdateSeqCount)+1];
        if ((UpdateSeqCount*sizeof (NTFS_Buffer_Sector)) < sizeof (NTFS_MFT_Buff))
        {
            if ( *(UINT16 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_NO] == *(UINT16 *)&NTFS_MFT_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount])
            {
                //
                // The sequence number of each MFT FILE stored in 0x30 (2 bytes)
                // Check if the last 2 bytes in each sector are the same with the sequence number
                //
                *(UINT8 *)&NTFS_MFT_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2];
                *(UINT8 *)&NTFS_MFT_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2+1];
            }
        }
    }

    //
    // Calculate the number of pages of MFT FILE in the unit of NTFSReadDisk (512 bytes)
    //
    TargetFileBodyTypePtr = (UINT8 *)&NTFS_MFT_Buffer.data[0];
    PagesPerMFT = *(UINT32 *)&NTFS_MFT_Buffer.data[MFT_FILE_RECORD_SIZE]/BytesPerPage;

    //
    // Get attribute 0x80 data run in $MFT to determine if there is discontinuous MFT FILEs
    // If there is more than one elements in data run array, the MFT FILEs is discontinuous
    //
    TargetBodyOffset = SearchTargetBodyType(0x80,sizeof(NTFS_MFT_Buff),TargetFileBodyTypePtr);

    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    TargetFileBodyTypePtr = TargetFileBodyTypePtr + TargetBodyOffset;

    //
    // Determine whether this attribute 0x80 is Non-resident or not
    // If this attribute is non-resident (non-resident flag is 1), the data run starts from 0x40 plus twice of the attribute's name length
    // Else (resident), the data run starts from 0x18 plus twice of the attribute's name length
    //
    if (*(TargetFileBodyTypePtr+8)==1) {
        TargetFileBodyTypePtr = TargetFileBodyTypePtr + NONRESIDENT_ATTR_NAME + *(TargetFileBodyTypePtr+9)*2;
    }
    else {
        TargetFileBodyTypePtr = TargetFileBodyTypePtr + RESIDENT_ATTR_NAME + *(TargetFileBodyTypePtr+9)*2;
    }

    //
    // Analyze $MFT Attribute 0x80 Data Run for the possibility of discontinuous MFT FILEs
    //
    DataRunCount = 0;

    Status = DataRunGetData(TargetFileBodyTypePtr, *TargetPartitionOffsetPtr, &MFTDataRunSize[0], &MFTDataRunOffset[0], &MFTDataRunCount, SectorsPerCluster);

    MFTSizeCount = MFTDataRunSize[0];
    MFTNoSector = MultU64x32(TargetMFTNo, PagesPerMFT);

    //
    // If the amount of MFT FILEs is too large, all MFT FILEs will be seperated into several parts.
    // And their information will be stored in the data run of attribute 0x80 in $MFT.
    // Determine which part the current MFT FILE is in.
    //
    while (MFTNoSector >= MFTSizeCount)
    {
        DataRunOrder++;
        if (MFTNoSector < (MFTSizeCount + MFTDataRunSize[DataRunOrder]))
        {
            MFTNoSector -= MFTSizeCount;
        }
        MFTSizeCount += MFTDataRunSize[DataRunOrder];
    }

    //
    // CheckFlag=0xff => get the folder name
    // CheckFlag=0xf0 => get the target recovery file name
    //
    while (CheckFlag!=0xff && CheckFlag!=0xf0) {
        if (*(FileNameArrayPtr + TargetFileNameArrayCount) == 0xee) {
            TargetFileNameArray[TargetFileNameArrayCount] = *(FileNameArrayPtr+TargetFileNameArrayCount);
            CheckFlag=0xff;
        }
        else if (*(FileNameArrayPtr + TargetFileNameArrayCount) == 0xff) {
            TargetFileNameArray[TargetFileNameArrayCount] = *(FileNameArrayPtr+TargetFileNameArrayCount);
            CheckFlag=0xf0;
            *GetTargetFileMFTPtr = 0xff;
        }
        else {
            TargetFileNameArray[TargetFileNameArrayCount] = *(FileNameArrayPtr+TargetFileNameArrayCount);
        }
        TargetFileNameArrayCount++;
    }
    *FileNameArrayLocationPtr = TargetFileNameArrayCount + 1;

    TargetFileNameArrayCount --;
    TargetFileNameArrayCountPtr = &TargetFileNameArrayCount;

    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Get the current target MFT FILE
    // Determine if this address is the position for MFT FILE by FILE signature
    //
    Status = NTFSReadDisk (
                 PrivateData,
                 VolumeIndx,
                 MFTDataRunOffset[DataRunOrder] + MFTNoSector,
                 sizeof (NTFS_MFT_Buff),
                 &NTFS_MFT_Buffer
             );

    if (*(UINT32 *)&NTFS_MFT_Buffer.data[0] !='ELIF') {
        return EFI_DEVICE_ERROR;
    }

    TargetFileBodyTypePtr = (UINT8 *)&NTFS_MFT_Buffer;

    //
    // The last 2 bytes in each sector are filled by the sequence number used to check if this sector is correctly written (Each MFT FILE has the same value)
    // Get the original values of the last 2 bytes in each sector from 0x32, and store them in UpdateSeqArray
    // Fill in UpdateSeqArray to their original addresses in order(the last 2 bytes in each sector)
    //
    UpdateSeqArraySize = *(UINT16 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_SIZE];
    for (UpdateSeqCount = 0; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
    {
        UpdateSeqArray[UpdateSeqCount*2] = *(UINT8 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_ARRAY+2*UpdateSeqCount];
        UpdateSeqArray[UpdateSeqCount*2+1] = *(UINT8 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_ARRAY+(2*UpdateSeqCount)+1];
        if ((UpdateSeqCount*sizeof (NTFS_Buffer_Sector)) < sizeof (NTFS_MFT_Buff))
        {
            if ( *(UINT16 *)&NTFS_MFT_Buffer.data[MFT_UPDATE_SEQ_NO] == *(UINT16 *)&NTFS_MFT_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount])
            {
                //
                // The sequence number of each MFT FILE stored in 0x30 (2 bytes)
                // Check if the last 2 bytes in each sector are the same with the sequence number
                //
                *(UINT8 *)&NTFS_MFT_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2];
                *(UINT8 *)&NTFS_MFT_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2+1];
            }
        }
    }

    //
    // Search target Body type (attribute) in MFT FILE, if there is no attribute 0xa0, then search attribute 0x90.
    // If there is a large Index which means there is a lot of files or folders in a index file, the file attributes index allocation 0xa0 are present
    // If the index is small enough to be stored completely in the root node, there is only attribute index root 0x90.
    // TargetBodyOffset is the offset relative to TargetFileBodyTypePtr of target attribute
    //
    TargetBodyOffset = SearchTargetBodyType(0xa0,sizeof(NTFS_MFT_Buff),TargetFileBodyTypePtr);

    if (TargetBodyOffset==0) {

        //
        // TargetBodyOffset is the offset relative to TargetFileBodyTypePtr of target attribute
        //
        TargetBodyOffset = SearchTargetBodyType(0x90,sizeof(NTFS_MFT_Buff),TargetFileBodyTypePtr);
        if (TargetBodyOffset==0) {
            return EFI_DEVICE_ERROR;
        }
        else {
            TargetFileBodyTypePtr = TargetFileBodyTypePtr + TargetBodyOffset;

            //
            // 0x90 file attribute is always resident
            // Right TargetFileBodyTypePtr is the start address of $INDX_Root 0x90 attribute header
            // Left  TargetFileBodyTypePtr is the start address of Index root
            //
            TargetFileBodyTypePtr = TargetFileBodyTypePtr + RESIDENT_ATTR_NAME + *(TargetFileBodyTypePtr+9)*2;

            TargetMFTNo = SearchFileNameInIndexTbl_90(PrivateData,VolumeIndx,TargetFileNameArray,TargetFileNameArrayCountPtr,TargetFileBodyTypePtr);
            if (TargetMFTNo==0) {
                return EFI_DEVICE_ERROR;
            }
        }
    }
    else {
        TargetFileBodyTypePtr = TargetFileBodyTypePtr + TargetBodyOffset;

        //
        // Determine whether this attribute 0xa0 is Non-resident or not
        // If this attribute is non-resident (non-resident flag is 1), the data run starts from 0x40 plus twice of the attribute's name length
        // Else (resident), the data run starts from 0x18 plus twice of the attribute's name length
        //
        if (*(TargetFileBodyTypePtr+8)==1) {
            TargetFileBodyTypePtr= TargetFileBodyTypePtr + NONRESIDENT_ATTR_NAME + *(TargetFileBodyTypePtr+9)*2;
        }
        else {
            TargetFileBodyTypePtr= TargetFileBodyTypePtr + RESIDENT_ATTR_NAME + *(TargetFileBodyTypePtr+9)*2;
        }

        //
        // Analyze data run
        //
        DataRunCount = 0;

        Status = DataRunGetData(TargetFileBodyTypePtr, *TargetPartitionOffsetPtr, &DataRunSizeArray[0], &DataRunOffsetArray[0], &DataRunCount, SectorsPerCluster);

        TargetMFTNo = SearchFileNameInIndexTbl(PrivateData, VolumeIndx, TargetFileNameArray, TargetFileNameArrayCountPtr , &DataRunSizeArray[0], &DataRunOffsetArray[0], &DataRunCount);
        if (TargetMFTNo==0) {
            return EFI_DEVICE_ERROR;
        }
    }

    *TargetMFTNoPtr = TargetMFTNo;

    //
    // The target recovery file rom has been found *GetTargetFileMFTPtr == 0xff
    // If the amount of MFT FILEs is too large, all MFT FILEs will be seperated into several parts.
    // And their information will be stored in the data run of attribute 0x80 in $MFT.
    // Determine which part the current MFT FILE is in.
    // And return the absolute address of the MFT FILE of the target recovery rom
    //
    if (*GetTargetFileMFTPtr == 0xff)
    {
        DataRunOrder = 0;
        MFTSizeCount = MFTDataRunSize[0];
        MFTNoSector = MultU64x32(TargetMFTNo, PagesPerMFT);

        while (MFTNoSector >= MFTSizeCount)
        {
            DataRunOrder++;
            if (MFTNoSector < (MFTSizeCount + MFTDataRunSize[DataRunOrder]))
            {
                MFTNoSector -= MFTSizeCount;
            }
            MFTSizeCount += MFTDataRunSize[DataRunOrder];
        }
        return MFTDataRunOffset[DataRunOrder] + MFTNoSector;
    }
    //
    // The target recovery file rom has not been found *GetTargetFileMFTPtr != 0xff
    // Continue finding the MFT address of the next layer folder or file in path directory
    //
    else
    {
        FileNameArrayPtr = (UINT8 *)((UINT8 *)FileNameArrayPtr + (UINT8)*FileNameArrayLocationPtr);
        return SearchFileRec(PrivateData, VolumeIndx, FileNameArrayPtr, FileNameArrayLocationPtr, MFT_AddressPtr, TargetPartitionOffsetPtr, TargetMFTNoPtr, GetTargetFileMFTPtr, SectorsPerCluster);
    }
}

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

  FileNameArrayPtr                - file name pointer
  FileNameArrayLocationPtr        - point to file name array location
  TargetFileNameArrayPtr          - file name array pointer
  TargetFileNameArrayCountPtr     - file name counter pointer

Returns:

  EFI_SUCCESS - if doesn`t get the correct file name

--*/
{
    EFI_STATUS Status;
    UINT16 TargetFileNameArrayCount=0;
    UINT8 FolderCheckFlag=0;
    UINT16 FileNameArrayCount;

    //
    // Data initialization
    //
    Status = EFI_SUCCESS;
    FileNameArrayCount = (UINT8)*FileNameArrayLocationPtr;

    //
    // Put the current file/folder name in TargetFileNameArray and add '00' in file path
    // change the FileNameArrayLocationPtr to the next file path
    //
    while (FolderCheckFlag!=0xff) {
        if (*(FileNameArrayPtr+TargetFileNameArrayCount) == 0xff) {
            *(TargetFileNameArrayPtr + 2*TargetFileNameArrayCount) = *(FileNameArrayPtr + TargetFileNameArrayCount);
            FolderCheckFlag=0xff;
        }
        else if (*(FileNameArrayPtr+TargetFileNameArrayCount) == 0x00 || TargetFileNameArrayCount >= MaxFileNameLength) {
            return EFI_LOAD_ERROR;
        }
        else {
            *(TargetFileNameArrayPtr + 2*TargetFileNameArrayCount) = *(FileNameArrayPtr + TargetFileNameArrayCount);
            *(TargetFileNameArrayPtr + 2*TargetFileNameArrayCount+1) = 0;
        }
        TargetFileNameArrayCount++;
    }

    //
    // Location to next file/folder name
    //
    *FileNameArrayLocationPtr = *(UINT16 *)(TargetFileNameArrayPtr + TargetFileNameArrayCount);

    //
    // subtract 1 due to the first 0xee
    //
    TargetFileNameArrayCount--;
    *TargetFileNameArrayCountPtr = TargetFileNameArrayCount*2-1;

    return Status;
}

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

  *PrivateData                      -  Private Data
  VolumeIndx                        - Volume index number
  *TargetFileNameArray              - Target file name array pointer
  *TargetFileNameArrayCountPtr 		- Target file name counter pointer

Returns:

  UINT64 - Target MFT number

--*/
{
    UINT64 CurrentMFTNo;
    UINT8 FileCheckFlag;
    UINT8 *CurrentFileNamePtr;
    UINT8 CurrentFileNameLength;
    UINT32 CurrentIndexSize;
    UINT32 IndexSizeCount = 0;
    UINT16 CurrentMFTIndexEntrySize;

    //
    // Attribute 0x90 is seperated into 4 parts: Standard Attribute Header, Index Root, Index Header, and Index Entry.
    // CurrentPtr is the end of Attribute header and it also is the start of Index root, offset 0x18 is the allocated size of the Index Entries of INDX header
    // Skip INDX Root and INDX Header which size are both 0x10; therefore, CurrentPtr is the start of index entry
    //
    CurrentIndexSize = *(UINT32 *)((UINT8 *)CurrentPtr + 0x18);
    CurrentPtr +=0x20;

    while (IndexSizeCount <= CurrentIndexSize) {

        //
        // 0x00 8  bytes MFT Reference of the file (FILE record number)
        // A MFT reference number consists of a 6-byte index into the mft and a 2-byte sequence number used to detect stale references.
        // 0x08 2  bytes Size of this index entry
        // 0x50 1   byte Length of filename (F)
        // 0x52 2F bytes Filename
        // CurrentPtr is the end of Attribute header and it also is the start of Index root, offset 0x18 is the allocated size of the Index Entries of INDX header
        // Skip INDX Root and INDX Header which size are both 0x10; therefore, CurrentPtr is the start of index entry
        //
        CurrentFileNamePtr = (UINT8 *)((UINT8 *)CurrentPtr + FILENAME_OFFSET);
        CurrentFileNameLength = *((UINT8 *)CurrentFileNamePtr - 2);
        CurrentMFTNo = *(UINT64 *)CurrentPtr & SixBytes;
        CurrentMFTIndexEntrySize = *(UINT16 *)((UINT8 *)CurrentPtr + INDX_ENTRY_SIZE);

        //
        // Compare file name, if we got the target file/folder name, and then return its MFT number
        //
        FileCheckFlag = CompareFileName(TargetFileNameArray, TargetFileNameArrayCountPtr, CurrentFileNamePtr, &CurrentFileNameLength);

        if (FileCheckFlag == 0) {
            return CurrentMFTNo;
        }

        //
        // Get the next index entry, and repeat the procedure above until find the target file/folder name
        // If it cannot find the same file/ folder name in these index entries, then return 0
        //
        IndexSizeCount += (UINT32)CurrentMFTIndexEntrySize;
        CurrentPtr = CurrentPtr + CurrentMFTIndexEntrySize;
        if (*CurrentPtr==0) {
            break;
        }
    }
    return 0;
}

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

  *PrivateData                      -  Private Data
  VolumeIndx                        - Volume index number
  *TargetFileNameArray              - Target file name array pointer
  *TargetFileNameArrayCountPtr      - Target file name counter pointer
  *DataRunSizeArray                 - Data run size array   (Unit:sector)
  *DataRunOffsetArray               - Data run offset array (Unit:sector)
  *DataRunCount                     - Data run counter

Returns:

  UINT64 - Target MFT number

--*/
{
    UINT64 CurrentMFTNumber;
    UINT32 DataRunCountIndex;           // the order of the pointed data run element
    NTFS_INDX_Buff NTFS_INDX_Buffer;    // Set index size as 2048 is due to this value is the maximal value which could be successfully compiled of 2 to power of n
    EFI_STATUS Status;

    UINT64 *CurrentMFTNumberPtr;
    UINT16 CurrentMFTIndexEntrySize;
    UINT8 *CurrentFileNamePtr;
    UINT8 FileCheckFlag;
    UINT8 CurrentFileNameLength;

    UINT64 DataRunSizeCount;            // The sector order relative to the INDX beginning of the pointed data run element
    UINT8  GetFileFlag;
    UINT64 DataRunOffsetCount;          // The absolute start address to get the target MFT number
    UINT32 MFTNoOffset;                 // The relative start address of current index entry which stores MFT No
    UINT32 INDXLenOffset;               // The relative offset of current index entry length
    UINT32 FileLenOffset;               // The relative offset of the file/folder name length in current index entry
    UINT32 FileNameOffset;              // The relative offset of the file/folder name in current index entry
    UINT32 FileNameEndOffset;           // The relative end offset of the file/folder name in current index entry
    UINT32 Remainder;
    // Because a single sector could fail, it's important for NTFS to be able to detect errors
    // For this purpose the sectors have Fixups, which are kept in an Update Sequence Array.
    UINT8  UpdateSeqArray[16];          // Update sequence array
    UINT16 UpdateSeqArraySize;          // Size in words of Update Sequence Number & Array
    UINT16 UpdateSeqCount;              // The count of update sequence array
    UINT16 UpdateSeqNo;                 // The update sequence number
    UINT32 PagesPerINDX;                 // The number of pages of INDX (page unit is 512 bytes)
    UINT64 Result;
    //
    // Data initialization
    //
    Status = EFI_SUCCESS;
    CurrentMFTNumber = 0;
    DataRunCountIndex = 0;
    DataRunOffsetCount = 0;
    DataRunSizeCount = 0;
    MFTNoOffset = 0;

    //
    // Check if data run data error
    //
    if (*DataRunSizeArray==0||*DataRunOffsetArray==0||*DataRunCount==0) {
        return 0;
    }

    //
    // Search file name in data run array, and if the target file is found, return its MFT number
    // Determine if the pointed data run order is over the number of element of data run array
    //
    while (DataRunCountIndex < *DataRunCount) {

        //
        // DataRunOffsetArray is the array stores each index beginning address
        // DataRunCountIndex is the order of the pointed data run element
        //
        DataRunOffsetCount = *((UINT64 *)DataRunOffsetArray + (UINT64)DataRunCountIndex);

        //
        // Determine if the number of index files is over the size of the pointed data run element
        // If it is over the pointed element size, then find the next index start of data run element
        //
        while ( DataRunSizeCount < *((UINT64 *)DataRunSizeArray + (UINT64)DataRunCountIndex)) {

            GetFileFlag = 0;
            Status = NTFSReadDisk (
                         PrivateData,
                         VolumeIndx,
                         DataRunOffsetCount + DataRunSizeCount,
                         sizeof (NTFS_INDX_Buff),
                         &NTFS_INDX_Buffer
                     );

            //
            // Check the index file signature
            //
            if (*(UINT32 *)&NTFS_INDX_Buffer.data[0]!='XDNI') {
                return EFI_DEVICE_ERROR;
            }

            //
            // 0x20 4 bytes for Allocated size relative to 0x18 of the Index Entries
            // The unit of PagesPerINDX is the same as NTFSReadDisk (512 bytes per unit)
            // The sequence number of each INDX cluster stored in 0x28 (2 bytes)
            //
            PagesPerINDX = (*(UINT32 *)&NTFS_INDX_Buffer.data[INDX_FILE_RECORD_SIZE] + 0x18)/BytesPerPage;
            UpdateSeqNo = *(UINT16 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_NO];

            //
            // The last 2 bytes in each sector are filled by the sequence number used to check if this sector is correctly written (Each INDX cluster has the same value)
            // Get the original values of the last 2 bytes in each sector from 0x2A, and store them in UpdateSeqArray
            // Fill in UpdateSeqArray to their original addresses in order(the last 2 bytes in each sector)
            //
            UpdateSeqArraySize = *(UINT16 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_SIZE];

            for (UpdateSeqCount = 0; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
            {
                UpdateSeqArray[UpdateSeqCount*2] = *(UINT8 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_ARRAY+2*UpdateSeqCount];
                UpdateSeqArray[UpdateSeqCount*2+1] = *(UINT8 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_ARRAY+(2*UpdateSeqCount)+1];
                if ((UpdateSeqCount*sizeof (NTFS_Buffer_Sector)) < sizeof (NTFS_INDX_Buff))
                {
                    //
                    // Check if the last 2 bytes in each sector are the same with the sequence number
                    //
                    if ( UpdateSeqNo == *(UINT16 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount])
                    {
                        *(UINT8 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2];
                        *(UINT8 *)&NTFS_INDX_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*UpdateSeqCount] = UpdateSeqArray[UpdateSeqCount*2+1];
                    }
                }
            }
            //
            // Get the first index entry offset address
            // Standard index header 0x18 4 bytes Offset relative to 0x18 to the Index Entries
            // MFTNoOffset is the MFT number offset in the current index entry
            // Index record header 0x00 8 bytes MFT Reference of the file (FILE record number)
            //
            MFTNoOffset = 0x18;
            MFTNoOffset += *(UINT32 *)&NTFS_INDX_Buffer.data[0x18];

            //
            // The first Index entry absolute address is relative to 0x18
            // A MFT reference number consists of a 6-byte index into the mft and a 2-byte sequence number used to detect stale references.
            //
            CurrentMFTNumberPtr = (UINT64 *)((UINT8 *)&NTFS_INDX_Buffer.data[0] + MFTNoOffset);
            CurrentMFTNumber = *CurrentMFTNumberPtr & SixBytes;

            while (GetFileFlag != 0xff) {

                //
                // Index record header 0x08 2  bytes Size of this index entry
                // Index record header 0x50 1   byte Length of filename (F)
                // Index record header 0x52 2F bytes Filename
                // Calculate the relative addresses of INDXLenOffset, FileLenOffset, FileNameOffset, and FileNameEndOffset in a 2048-Byte data buffer
                //
                INDXLenOffset     = MFTNoOffset + INDX_ENTRY_SIZE;
                FileLenOffset     = MFTNoOffset + FILENAME_LENGTH;
                FileNameOffset    = MFTNoOffset + FILENAME_OFFSET;

                //
                // FileNameEndOffset is the end of the current index entry
                // If FileLenOffset is in the same 2048-byte data buffer as current one, then get its value directly to obtain FileNameEndOffset.
                // If it exceeds the current 2048-byte data buffer, then get the next 2048-byte buffer to obtain FileNameEndOffset.
                // But it should return back to the original 2048-byte buffer
                //
                if (FileLenOffset <  sizeof(NTFS_INDX_Buff))
                {
                    FileNameEndOffset = FileNameOffset + (*(UINT8 *)&NTFS_INDX_Buffer.data[FileLenOffset] * 2) - 1;
                }
                else
                {

                    Status = NTFSReadDisk (
                                 PrivateData,
                                 VolumeIndx,
                                 DataRunOffsetCount + DataRunSizeCount + (sizeof(NTFS_INDX_Buff)/BytesPerPage),
                                 sizeof (NTFS_INDX_Buff),
                                 &NTFS_INDX_Buffer
                             );
                    //
                    // Get the update sequence array in current index FILE for the pointed address
                    //
                    Result = DivU64x32Remainder ( DataRunSizeCount + (sizeof(NTFS_INDX_Buff)/BytesPerPage), PagesPerINDX, &Remainder);
                    Status = UpdateSeqData( PrivateData, VolumeIndx, DataRunOffsetCount + DataRunSizeCount + (sizeof(NTFS_INDX_Buff)/BytesPerPage), Remainder, UpdateSeqArray, UpdateSeqNo, UpdateSeqArraySize);

                    for (UpdateSeqCount = (UINT16)Remainder; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
                    {

                        //
                        // Check if the last 2 bytes in each sector are the same with the sequence number
                        //
                        if ( UpdateSeqNo == *(UINT16 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)])
                        {
                            *(UINT8 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2];
                            *(UINT8 *)&NTFS_INDX_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2+1];
                        }
                    }

                    FileNameEndOffset = FileNameOffset + (*(UINT32 *)&NTFS_INDX_Buffer.data[FileLenOffset - sizeof(NTFS_INDX_Buff)] * 2) - 1;

                    Status = NTFSReadDisk (
                                 PrivateData,
                                 VolumeIndx,
                                 DataRunOffsetCount + DataRunSizeCount,
                                 sizeof (NTFS_INDX_Buff),
                                 &NTFS_INDX_Buffer
                             );

                    //
                    // Get the update sequence array in current index FILE for the pointed address
                    //
                    Result = DivU64x32Remainder ( DataRunSizeCount, PagesPerINDX, &Remainder);
                    Status = UpdateSeqData( PrivateData, VolumeIndx, DataRunOffsetCount + DataRunSizeCount, Remainder, UpdateSeqArray, UpdateSeqNo, UpdateSeqArraySize);

                    for (UpdateSeqCount = (UINT16)Remainder; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
                    {

                        //
                        // Check if the last 2 bytes in each sector are the same with the sequence number
                        //
                        if ( UpdateSeqNo == *(UINT16 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)])
                        {
                            *(UINT8 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2];
                            *(UINT8 *)&NTFS_INDX_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2+1];
                        }
                    }
                }

                //
                // There are three possible situations used to analyze the index record header to compare the file name.
                // Use the calculated information above to determine if the current index entry is in the same 2048-byte data
                // 1. If MFTNoOffset, INDXLenOffset, FileLenOffset, FileNameOffset, and FileNameEndOffset are all larger than or equal to 2048, then get the next 2048-byte buffer.
                // The goal is to get the MFT number of current index record.
                //
                if ( (MFTNoOffset >= sizeof(NTFS_INDX_Buff))  && (INDXLenOffset >= sizeof(NTFS_INDX_Buff)) && (FileLenOffset >= sizeof(NTFS_INDX_Buff)) && (FileNameOffset >= sizeof(NTFS_INDX_Buff)) && (FileNameEndOffset >= sizeof(NTFS_INDX_Buff)) ) {
                    DataRunSizeCount += 4;

                    Status = NTFSReadDisk (
                                 PrivateData,
                                 VolumeIndx,
                                 DataRunOffsetCount + DataRunSizeCount,
                                 sizeof (NTFS_INDX_Buff),
                                 &NTFS_INDX_Buffer
                             );

                    //
                    // Get the update sequence array in current index FILE for the pointed address
                    //
                    Result = DivU64x32Remainder ( DataRunSizeCount, PagesPerINDX, &Remainder);
                    Status = UpdateSeqData( PrivateData, VolumeIndx, DataRunOffsetCount + DataRunSizeCount, Remainder, UpdateSeqArray, UpdateSeqNo, UpdateSeqArraySize);

                    for (UpdateSeqCount = (UINT16)Remainder; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
                    {

                        //
                        // Check if the last 2 bytes in each sector are the same with the sequence number
                        //
                        if ( UpdateSeqNo == *(UINT16 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)])
                        {
                            *(UINT8 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2];
                            *(UINT8 *)&NTFS_INDX_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2+1];
                        }
                    }

                    //
                    // Modify MFTNoOffset to the offset in the current new 2048-byte buffer.
                    // A MFT reference number consists of a 6-byte index into the mft and a 2-byte sequence number used to detect stale references.
                    //
                    MFTNoOffset -= sizeof(NTFS_INDX_Buff);
                    CurrentMFTNumberPtr = (UINT64 *)((UINT8 *)&NTFS_INDX_Buffer.data[0] + MFTNoOffset);
                    CurrentMFTNumber = *CurrentMFTNumberPtr & SixBytes;
                }

                //
                // 2. If MFTNoOffset, INDXLenOffset, FileLenOffset, FileNameOffset, and FileNameEndOffset are all smaller than 2048, then get the current MFT number directly.
                //
                if ( (MFTNoOffset < sizeof(NTFS_INDX_Buff)) && (INDXLenOffset < sizeof(NTFS_INDX_Buff)) && (FileLenOffset < sizeof(NTFS_INDX_Buff)) && (FileNameOffset < sizeof(NTFS_INDX_Buff)) && (FileNameEndOffset < sizeof(NTFS_INDX_Buff)) ) {
                    CurrentMFTNumberPtr = (UINT64 *)((UINT8 *)&NTFS_INDX_Buffer.data[0] + MFTNoOffset);
                    CurrentMFTNumber = *CurrentMFTNumberPtr & SixBytes;
                }

                //
                // 3. If there is one of INDXLenOffset, FileLenOffset, FileNameOffset, and FileNameEndOffset larger than or equal to 2048, then get the next two sectors to get all correct information we need.
                //
                if ( (INDXLenOffset >= sizeof(NTFS_INDX_Buff)) || (FileLenOffset >= sizeof(NTFS_INDX_Buff)) || (FileNameOffset >= sizeof(NTFS_INDX_Buff)) || (FileNameEndOffset >= sizeof(NTFS_INDX_Buff)) ) {
                    DataRunSizeCount +=2;

                    Status = NTFSReadDisk (
                                 PrivateData,
                                 VolumeIndx,
                                 DataRunOffsetCount + DataRunSizeCount,
                                 sizeof (NTFS_INDX_Buff),
                                 &NTFS_INDX_Buffer
                             );

                    //
                    // Get the update sequence array in current index FILE for the pointed address
                    //
                    Result = DivU64x32Remainder ( DataRunSizeCount, PagesPerINDX, &Remainder);
                    Status = UpdateSeqData( PrivateData, VolumeIndx, DataRunOffsetCount + DataRunSizeCount, Remainder, UpdateSeqArray, UpdateSeqNo, UpdateSeqArraySize);

                    for (UpdateSeqCount = (UINT16)Remainder; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
                    {

                        //
                        // Check if the last 2 bytes in each sector are the same with the sequence number
                        //
                        if ( UpdateSeqNo == *(UINT16 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)])
                        {
                            *(UINT8 *)&NTFS_INDX_Buffer.data[SecondtoLastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2];
                            *(UINT8 *)&NTFS_INDX_Buffer.data[LastByte + sizeof (NTFS_Buffer_Sector)*(UpdateSeqCount-Remainder)] = UpdateSeqArray[UpdateSeqCount*2+1];
                        }
                    }

                    MFTNoOffset -= (sizeof(NTFS_INDX_Buff)/2);
                    CurrentMFTNumberPtr = (UINT64 *)((UINT8 *)&NTFS_INDX_Buffer.data[0] + MFTNoOffset);
                    CurrentMFTNumber = *CurrentMFTNumberPtr & SixBytes;
                }

                //
                // Get the length of the current index entry and find the next index entry
                // Get the address of file name in current index entry
                //
                CurrentMFTIndexEntrySize = *(UINT16 *)((UINT8 *)CurrentMFTNumberPtr + 8);
                MFTNoOffset+=(UINT32)CurrentMFTIndexEntrySize;
                CurrentFileNamePtr = (UINT8 *)((UINT8 *)CurrentMFTNumberPtr + FILENAME_OFFSET);

                //
                // If CurrentMFTNumber is 0, but file name is not $MFT, that is here is the end of this index, then get the next data run.
                //
                if (CurrentMFTNumber == 0 && !(*CurrentFileNamePtr=='$'&&*((UINT8 *)CurrentFileNamePtr+2)=='M'&&*((UINT8 *)CurrentFileNamePtr+4)=='F'&&*((UINT8 *)CurrentFileNamePtr+6)=='T')) {
                    DataRunSizeCount = DivU64x32Remainder ( DataRunSizeCount, PagesPerINDX, &Remainder) + 1;
                    DataRunSizeCount = MultU64x32(DataRunSizeCount, PagesPerINDX);
                    GetFileFlag = 0xff;
                }

                CurrentFileNameLength = *((UINT8 *)CurrentFileNamePtr - 2);

                //
                // Compare file name, if find the target file/folder name return its MFT number
                //
                FileCheckFlag = CompareFileName(TargetFileNameArray, TargetFileNameArrayCountPtr, CurrentFileNamePtr, &CurrentFileNameLength);

                if (FileCheckFlag == 0) {
                    return CurrentMFTNumber;
                }

                //
                // Next MFT number address = current MFT number address + current index entry size
                //
                CurrentMFTNumberPtr = (UINT64 *)((UINT8 *)CurrentMFTNumberPtr + (UINT8)CurrentMFTIndexEntrySize);
                CurrentMFTNumber = *CurrentMFTNumberPtr & SixBytes;
            }
        }
        //
        // Get the index start address of the next data run element
        //
        DataRunSizeCount = 0;
        DataRunCountIndex++;
        GetFileFlag = 0;
    }
    return 0;
}

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
{
    UINT8 FileCheckFlag;
    UINT8 FileNameCount;
    FileNameCount = 0;
    FileCheckFlag = 2;
    //
    // If file length is not match, then return 0x01
    //
    if (*(TargetFileNameArrayCountPtr) != ((UINT16)*(CurrentFileNameLength))*2) {
        return 0x01;
    }

    //
    // Compare file name
    //
    for (FileNameCount=0; FileNameCount<*TargetFileNameArrayCountPtr; FileNameCount++) {
        if ( ToUpper(*(TargetFileNameArray+FileNameCount)) == ToUpper(*(CurrentFileNamePtr+FileNameCount)) ) {
            FileCheckFlag = 0;
        }
        else {
            FileCheckFlag = 2;
            break;
        }
    }
    return FileCheckFlag;
}

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
{
    UINT64 Result;
    UINT32 Count;
    Result = 1;

    for (Count = 0; Count < Exp; Count++)
    {
        Result = MultU64x32(Result , Base);
    }
    return Result;
}

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
{
    EFI_STATUS Status;
    NTFS_INDX_Buff NTFS_INDX_Buffer;    // Set index size as 2048 is due to this value is the maximal value which could be successfully compiled of 2 to power of n
    UINT16 UpdateSeqCount;              // The count of update sequence array


    Status = EFI_SUCCESS;

    if ( Remainder != 0 ) {
        Address = Address - (UINT64)Remainder;
    }

    Status = NTFSReadDisk (
                 PrivateData,
                 VolumeIndx,
                 Address,
                 sizeof (NTFS_INDX_Buff),
                 &NTFS_INDX_Buffer
             );

    //
    // Check the index file signature
    //
    if (*(UINT32 *)&NTFS_INDX_Buffer.data[0]!='XDNI') {
        return EFI_DEVICE_ERROR;
    }
    //
    // The sequence number of each INDX cluster stored in 0x28 (2 bytes)
    //
    UpdateSeqNo = *(UINT16 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_NO];

    //
    // The last 2 bytes in each sector are filled by the sequence number used to check if this sector is correctly written (Each INDX cluster has the same value)
    // Get the original values of the last 2 bytes in each sector from 0x2A, and store them in UpdateSeqArray
    //
    UpdateSeqArraySize = *(UINT16 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_SIZE];

    for (UpdateSeqCount = 0; UpdateSeqCount < (UpdateSeqArraySize - 1); UpdateSeqCount++)
    {
        UpdateSeqArray[UpdateSeqCount*2] = *(UINT8 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_ARRAY+2*UpdateSeqCount];
        UpdateSeqArray[UpdateSeqCount*2+1] = *(UINT8 *)&NTFS_INDX_Buffer.data[INDX_UPDATE_SEQ_ARRAY+(2*UpdateSeqCount)+1];
    }
    return Status;
}
