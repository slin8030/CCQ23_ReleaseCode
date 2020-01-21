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

#include "CompalNTFSPeim.h"
#include "CompalMbr.h"
#include "CompalGpt.h"

EFI_STATUS
NTFSGetBpbInfo (
    IN      PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN OUT  PEI_NTFS_VOLUME        *Volume
)
/*++

Routine Description:

  Check if there is a valid NTFS in the corresponding Block device
  of the volume and if yes, fill in the relevant fields for the
  volume structure. Note there should be a valid Block device number
  already set.

Arguments:

  PrivateData  - Global memory map for accessing global variables.
  Volume  - On input, the BlockDeviceNumber field of the Volume
            should be a valid value. On successful output, all
            fields except the VolumeNumber field is initialized.

Returns:

  EFI_SUCCESS       - A NTFS is found and the volume structure is initialized.
  EFI_NOT_FOUND     - There is no NTFS on the corresponding device.
  EFI_DEVICE_ERROR  - There is something error while accessing device.

--*/
{
    EFI_STATUS                     Status;
    PEI_NTFS_BPB_Info              NTFSBpb;
    MASTER_BOOT_RECORD             MBRInfo;
    UINT8                          PartitionIndex = 0;
    UINT64                         PartitionStartingLBA = 0;
    EFI_PARTITION_TABLE_HEADER     PartitionHdr;
    GPT_PARTITION_TABLE            GPTPartitionTBL;
    EFI_GUID                       gEfiWindowsPartTypeGuid = EFI_WINDOWS_PART_TYPE_GUID;

    //
    // Read in the MBR
    //

    Status = NTFSReadDisk (
                 PrivateData,
                 Volume->BlockDeviceNo,
                 0,
                 sizeof (MASTER_BOOT_RECORD),
                 &MBRInfo
             );

    //
    // Check MBR signature
    //
    if ( MBRInfo.Signature != 0xAA55 ) {    // 0xAA55
        return EFI_DEVICE_ERROR;
    }

    if ( MBRInfo.Partition[0].OSIndicator == PMBR_GPT_PARTITION ) {

        //
        // Check EFI Partition Valid
        //

        Status = NTFSReadDisk (
                     PrivateData,
                     Volume->BlockDeviceNo,
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
                     Volume->BlockDeviceNo,
                     PartitionHdr.PartitionEntryLBA,
                     sizeof (GPT_PARTITION_TABLE),
                     &GPTPartitionTBL
                 );


        if ( EFI_ERROR (Status) ) {
            return Status;
        }

        for ( PartitionIndex = 0; PartitionIndex < 8 ; PartitionIndex++ ) {
            if ( CompareGuid (&GPTPartitionTBL.EFIPartition[ PartitionIndex ].PartitionTypeGUID, &gEfiWindowsPartTypeGuid) ) {
                PartitionStartingLBA = GPTPartitionTBL.EFIPartition[ PartitionIndex ].StartingLBA;
                break;
            }
        }
    } else {
        for ( PartitionIndex = 0; PartitionIndex < 4 ; PartitionIndex++ ) {
            if ( MBRInfo.Partition[ PartitionIndex ].OSIndicator == PartitionNTFSType ) {
                PartitionStartingLBA = (UINT64)( UNPACK_UINT32( MBRInfo.Partition[ PartitionIndex ].StartingLBA ) );
                break;
            }
        }
    }

    //
    // Read in the BPB
    //

    Status = NTFSReadDisk (
                 PrivateData,
                 Volume->BlockDeviceNo,
                 PartitionStartingLBA,
                 sizeof (PEI_NTFS_BPB_Info),
                 &NTFSBpb
             );

    if (EFI_ERROR (Status)) {
        return Status;
    }

    Volume->NTFSType = NTFS_UNKNOWN;

    //
    // Filter out those not a NTFS
    //

    //
    //check jump instruction
    //

//if (NTFSBpb.Ia32Jump[0] != 0xe9 && NTFSBpb.Ia32Jump[0] != 0xeb && NTFSBpb.Ia32Jump[0] != 0x49) {
//  return EFI_NOT_FOUND;
//}

    //
    //check NTFS signature
    //
    if (NTFSBpb.OemId[0]!='N' || NTFSBpb.OemId[1]!='T' || NTFSBpb.OemId[2]!='F' || NTFSBpb.OemId[3]!='S')
    {
        return EFI_NOT_FOUND;
    }
    else
    {
        Volume->NTFSType = NTFS;
    };

    //
    //check NTFS format those who are always '0'
    //
    if (NTFSBpb.ReservedZero1[0] || NTFSBpb.ReservedZero1[1] || NTFSBpb.ReservedZero1[2] || NTFSBpb.ReservedZero2)
    {
        return EFI_NOT_FOUND;
    };

    //
    //check byte per Sector and Sector per cluster which we support
    //
    if (NTFSBpb.BytesPerSector != 0x100 &&
            NTFSBpb.BytesPerSector != 0x200 &&
            NTFSBpb.BytesPerSector != 0x400 &&
            NTFSBpb.BytesPerSector != 0x800 &&
            NTFSBpb.SectorPerCluster != 1 &&
            NTFSBpb.SectorPerCluster != 2 &&
            NTFSBpb.SectorPerCluster != 4 &&
            NTFSBpb.SectorPerCluster != 8 &&
            NTFSBpb.SectorPerCluster != 16 &&
            NTFSBpb.SectorPerCluster != 32
       ) {
        return EFI_NOT_FOUND;
    }
    //
    //check Media which we support
    //
    if (NTFSBpb.MediaDescriptor != 0xf0 &&
            NTFSBpb.MediaDescriptor != 0xf8 &&
            NTFSBpb.MediaDescriptor != 0xf9 &&
            NTFSBpb.MediaDescriptor != 0xfb &&
            NTFSBpb.MediaDescriptor != 0xfc &&
            NTFSBpb.MediaDescriptor != 0xfd &&
            NTFSBpb.MediaDescriptor != 0xfe &&
            NTFSBpb.MediaDescriptor != 0xff &&
            //
            // FujitsuFMR
            //
            NTFSBpb.MediaDescriptor != 0x00 &&
            NTFSBpb.MediaDescriptor != 0x01 &&
            NTFSBpb.MediaDescriptor != 0xfa
       ) {
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}
