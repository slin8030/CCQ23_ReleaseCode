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

#include "CompalNTFSpeim.h"
#include "CompalEltorito.h"
#include "CompalMbr.h"

BOOLEAN
NTFSFindEltoritoPartitions (
    IN  PEI_NTFS_PRIVATE_DATA *PrivateData,
    IN  UINTN                ParentBlockDevNo
);

BOOLEAN
NTFSFindMbrPartitions (
    IN  PEI_NTFS_PRIVATE_DATA *PrivateData,
    IN  UINTN                ParentBlockDevNo
);

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
// GC_TODO:    PrivateData - add argument and description to function comment
{
    BOOLEAN Found;
    UINTN   Index;

    do {
        Found = FALSE;

        for (Index = 0; Index < PrivateData->BlockDeviceCount; Index++) {
            if (!PrivateData->BlockDevice[Index].PartitionChecked) {
                Found = NTFSFindMbrPartitions (PrivateData, Index);
                if (!Found) {
                    Found = NTFSFindEltoritoPartitions (PrivateData, Index);
                }
            }
        }
    } while (Found && PrivateData->BlockDeviceCount <= PEI_NTFS_MAX_BLOCK_DEVICE);
}

BOOLEAN
NTFSFindEltoritoPartitions (
    IN  PEI_NTFS_PRIVATE_DATA *PrivateData,
    IN  UINTN                ParentBlockDevNo
)
/*++

Routine Description:

  This function finds Eltorito partitions. Main algorithm
  is ported from DXE partition driver.

Arguments:

  PrivateData            - The global memory map
  ParentBlockDevNo  - The parent block device

Returns

  TRUE    - New partitions are detected and logical block devices are
            added to block device array
  FALSE   - No New partitions are added;

--*/
// GC_TODO: function comment is missing 'Returns:'
{
    EFI_STATUS              Status;
    BOOLEAN                 Found;
    PEI_NTFS_BLOCK_DEVICE    *BlockDev;
    PEI_NTFS_BLOCK_DEVICE    *ParentBlockDev;
    UINT32                  VolDescriptorLba;
    UINT32                  Lba;
    CDROM_VOLUME_DESCRIPTOR *VolDescriptor;
    ELTORITO_CATALOG        *Catalog;
    UINTN                   Check;
    UINTN                   Index;
    UINTN                   MaxIndex;
    UINT16                  *CheckBuffer;
    UINT32                  SubBlockSize;
    UINT32                  SectorCount;
    UINT32                  VolSpaceSize;

    if (ParentBlockDevNo > PEI_NTFS_MAX_BLOCK_DEVICE - 1) {
        return FALSE;
    }

    Found           = FALSE;
    ParentBlockDev  = &(PrivateData->BlockDevice[ParentBlockDevNo]);
    VolSpaceSize    = 0;

    //
    // CD_ROM has the fixed block size as 2048 bytes
    //
    if (ParentBlockDev->BlockSize != 2048) {
        return FALSE;
    }

    VolDescriptor = (CDROM_VOLUME_DESCRIPTOR *) PrivateData->BlockData;
    Catalog       = (ELTORITO_CATALOG *) VolDescriptor;

    //
    // the ISO-9660 volume descriptor starts at 32k on the media
    // and CD_ROM has the fixed block size as 2048 bytes, so...
    //
    VolDescriptorLba = 15;
    //
    // ((16*2048) / Media->BlockSize) - 1;
    //
    // Loop: handle one volume descriptor per time
    //
    while (TRUE) {

        VolDescriptorLba += 1;
        if (VolDescriptorLba > ParentBlockDev->LastBlock) {
            //
            // We are pointing past the end of the device so exit
            //
            break;
        }

        Status = NTFSReadBlock (
                     PrivateData,
                     ParentBlockDevNo,
                     VolDescriptorLba,
                     ParentBlockDev->BlockSize,
                     VolDescriptor
                 );
        if (EFI_ERROR (Status)) {
            break;
        }
        //
        // Check for valid volume descriptor signature
        //
        if (VolDescriptor->Type == CDVOL_TYPE_END ||
                CompareMem (VolDescriptor->Id, CDVOL_ID, sizeof (VolDescriptor->Id)) != 0
           ) {
            //
            // end of Volume descriptor list
            //
            break;
        }
        //
        // Read the Volume Space Size from Primary Volume Descriptor 81-88 byte
        //
        if (VolDescriptor->Type == CDVOL_TYPE_CODED) {
            VolSpaceSize = VolDescriptor->VolSpaceSize[0];
        }
        //
        // Is it an El Torito volume descriptor?
        //
        if (CompareMem (
                    VolDescriptor->SystemId,
                    CDVOL_ELTORITO_ID,
                    sizeof (CDVOL_ELTORITO_ID) - 1
                ) != 0) {
            continue;
        }
        //
        // Read in the boot El Torito boot catalog
        //
        Lba = UNPACK_INT32 (VolDescriptor->EltCatalog);
        if (Lba > ParentBlockDev->LastBlock) {
            continue;
        }

        Status = NTFSReadBlock (
                     PrivateData,
                     ParentBlockDevNo,
                     Lba,
                     ParentBlockDev->BlockSize,
                     Catalog
                 );
        if (EFI_ERROR (Status)) {
            continue;
        }
        //
        // We don't care too much about the Catalog header's contents, but we do want
        // to make sure it looks like a Catalog header
        //
        if (Catalog->Catalog.Indicator != ELTORITO_ID_CATALOG || Catalog->Catalog.Id55AA != 0xAA55) {
            continue;
        }

        Check       = 0;
        CheckBuffer = (UINT16 *) Catalog;
        for (Index = 0; Index < sizeof (ELTORITO_CATALOG) / sizeof (UINT16); Index += 1) {
            Check += CheckBuffer[Index];
        }

        if (Check & 0xFFFF) {
            continue;
        }

        MaxIndex = ParentBlockDev->BlockSize / sizeof (ELTORITO_CATALOG);
        for (Index = 1; Index < MaxIndex; Index += 1) {
            //
            // Next entry
            //
            Catalog += 1;

            //
            // Check this entry
            //
            if (Catalog->Boot.Indicator != ELTORITO_ID_SECTION_BOOTABLE || Catalog->Boot.Lba == 0) {
                continue;
            }

            SubBlockSize  = 512;
            SectorCount   = Catalog->Boot.SectorCount;

            switch (Catalog->Boot.MediaType) {

            case ELTORITO_NO_EMULATION:
                SubBlockSize  = ParentBlockDev->BlockSize;
                SectorCount   = Catalog->Boot.SectorCount;
                break;

            case ELTORITO_HARD_DISK:
                break;

            case ELTORITO_12_DISKETTE:
                SectorCount = 0x50 * 0x02 * 0x0F;
                break;

            case ELTORITO_14_DISKETTE:
                SectorCount = 0x50 * 0x02 * 0x12;
                break;

            case ELTORITO_28_DISKETTE:
                SectorCount = 0x50 * 0x02 * 0x24;
                break;

            default:
                SectorCount   = 0;
                SubBlockSize  = ParentBlockDev->BlockSize;
                break;
            }

            if (SectorCount < 2) {
                SectorCount = (VolSpaceSize > ParentBlockDev->LastBlock + 1) ?
                              (UINT32)(ParentBlockDev->LastBlock - Catalog->Boot.Lba + 1) : (UINT32)(VolSpaceSize - Catalog->Boot.Lba);
            }
            //
            // Register this partition
            //
            if (PrivateData->BlockDeviceCount < PEI_NTFS_MAX_BLOCK_DEVICE) {

                Found                       = TRUE;

                BlockDev                    = &(PrivateData->BlockDevice[PrivateData->BlockDeviceCount]);

                BlockDev->BlockSize         = SubBlockSize;
                BlockDev->LastBlock         = SectorCount - 1;
                BlockDev->IoAlign           = ParentBlockDev->IoAlign;
                BlockDev->Logical           = TRUE;
                BlockDev->PartitionChecked  = FALSE;
                BlockDev->StartingPos       = MultU64x32 (Catalog->Boot.Lba, ParentBlockDev->BlockSize);
                BlockDev->ParentDevNo       = ParentBlockDevNo;

                PrivateData->BlockDeviceCount++;
            }
        }
    }

    ParentBlockDev->PartitionChecked = TRUE;

    return Found;

}

BOOLEAN
PartitionValidMbr (
    IN  MASTER_BOOT_RECORD      *Mbr,
    IN  EFI_PEI_LBA             LastLba
)
/*++

Routine Description:
  Test to see if the Mbr buffer is a valid MBR

Arguments:
  Mbr     - Parent Handle
  LastLba - Last Lba address on the device.

Returns:
  TRUE  - Mbr is a Valid MBR
  FALSE - Mbr is not a Valid MBR

--*/
{
    UINT32  StartingLBA;
    UINT32  EndingLBA;
    UINT32  NewEndingLBA;
    INTN    i;
    INTN    j;
    BOOLEAN MbrValid;

    if (Mbr->Signature != MBR_SIGNATURE) {
        return FALSE;
    }
    //
    // The BPB also has this signature, so it can not be used alone.
    //
    MbrValid = FALSE;
    for (i = 0; i < MAX_MBR_PARTITIONS; i++) {
        if (Mbr->Partition[i].OSIndicator == 0x00 || UNPACK_UINT32 (Mbr->Partition[i].SizeInLBA) == 0) {
            continue;
        }

        MbrValid    = TRUE;
        StartingLBA = UNPACK_UINT32 (Mbr->Partition[i].StartingLBA);
        EndingLBA   = StartingLBA + UNPACK_UINT32 (Mbr->Partition[i].SizeInLBA) - 1;
        if (EndingLBA > LastLba) {
            //
            // Compatability Errata:
            //  Some systems try to hide drive space with thier INT 13h driver
            //  This does not hide space from the OS driver. This means the MBR
            //  that gets created from DOS is smaller than the MBR created from
            //  a real OS (NT & Win98). This leads to BlockIo->LastBlock being
            //  wrong on some systems FDISKed by the OS.
            //
            //  return FALSE Because no block devices on a system are implemented
            //  with INT 13h
            //
            return FALSE;
        }

        for (j = i + 1; j < MAX_MBR_PARTITIONS; j++) {
            if (Mbr->Partition[j].OSIndicator == 0x00 || UNPACK_INT32 (Mbr->Partition[j].SizeInLBA) == 0) {
                continue;
            }

            NewEndingLBA = UNPACK_UINT32 (Mbr->Partition[j].StartingLBA) + UNPACK_UINT32 (Mbr->Partition[j].SizeInLBA) - 1;
            if (NewEndingLBA >= StartingLBA && UNPACK_UINT32 (Mbr->Partition[j].StartingLBA) <= EndingLBA) {
                //
                // This region overlaps with the i'th region
                //
                return FALSE;
            }
        }
    }
    //
    // Non of the regions overlapped so MBR is O.K.
    //
    return MbrValid;
}

BOOLEAN
NTFSFindMbrPartitions (
    IN  PEI_NTFS_PRIVATE_DATA *PrivateData,
    IN  UINTN                ParentBlockDevNo
)
/*++

Routine Description:

  This function finds Mbr partitions. Main algorithm
  is ported from DXE partition driver.

Arguments:

  PrivateData            - The global memory map
  ParentBlockDevNo  - The parent block device

Returns

  TRUE    - New partitions are detected and logical block devices are
            added to block device array
  FALSE   - No New partitions are added;

--*/
// GC_TODO: function comment is missing 'Returns:'
{
    EFI_STATUS            Status;
    MASTER_BOOT_RECORD    *Mbr;
    UINTN                 Index;
    BOOLEAN               Found;
    PEI_NTFS_BLOCK_DEVICE  *ParentBlockDev;
    PEI_NTFS_BLOCK_DEVICE  *BlockDev;

    if (ParentBlockDevNo > PEI_NTFS_MAX_BLOCK_DEVICE - 1) {
        return FALSE;
    }

    ParentBlockDev  = &(PrivateData->BlockDevice[ParentBlockDevNo]);

    Found           = FALSE;
    Mbr             = (MASTER_BOOT_RECORD *) PrivateData->BlockData;

    Status = NTFSReadBlock (
                 PrivateData,
                 ParentBlockDevNo,
                 0,
                 ParentBlockDev->BlockSize,
                 Mbr
             );

    if (EFI_ERROR (Status) || !PartitionValidMbr (Mbr, ParentBlockDev->LastBlock)) {
        goto Done;
    }
    //
    // We have a valid mbr - add each partition
    //
    for (Index = 0; Index < MAX_MBR_PARTITIONS; Index++) {
        if (Mbr->Partition[Index].OSIndicator == 0x00 || UNPACK_INT32 (Mbr->Partition[Index].SizeInLBA) == 0) {
            //
            // Don't use null MBR entries
            //
            continue;
        }
        //
        // Register this partition
        //
        if (PrivateData->BlockDeviceCount < PEI_NTFS_MAX_BLOCK_DEVICE) {

            Found                       = TRUE;

            BlockDev                    = &(PrivateData->BlockDevice[PrivateData->BlockDeviceCount]);

            BlockDev->BlockSize         = MBR_SIZE;
            BlockDev->LastBlock         = UNPACK_INT32 (Mbr->Partition[Index].SizeInLBA) - 1;
            BlockDev->IoAlign           = ParentBlockDev->IoAlign;
            BlockDev->Logical           = TRUE;
            BlockDev->PartitionChecked  = FALSE;
            BlockDev->StartingPos = MultU64x32 (
                                        UNPACK_INT32 (Mbr->Partition[Index].StartingLBA),
                                        ParentBlockDev->BlockSize
                                    );
            BlockDev->ParentDevNo = ParentBlockDevNo;

            PrivateData->BlockDeviceCount++;
        }
    }

Done:

    ParentBlockDev->PartitionChecked = TRUE;
    return Found;
}
