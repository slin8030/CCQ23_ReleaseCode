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

#ifndef _COMPAL_FWRECOVERY_FMT_H_
#define _COMPAL_FWRECOVERY_FMT_H_

//
// Boot Sector
//
#pragma pack(1)
//
// NTFS BPB structure
//
typedef struct {
    UINT8    Ia32Jump[3];
    CHAR8    OemId[8];
    UINT16   BytesPerSector;
    UINT8    SectorPerCluster;
    UINT16   ReservedSectors;
    UINT8    ReservedZero1[3];
    UINT16   NotUseByNTFS1;
    UINT8    MediaDescriptor;
    UINT16   ReservedZero2;
    UINT16   SectorsPerTrack;
    UINT16   NumberOfHeads;
    UINT32   HiddenSectors;
    UINT32   NotUseByNTFS2;
    UINT32   NotUseByNTFS3;
    UINT64   TotalSectors;
    UINT64   LogicalClusterNumberOfMFT;
    UINT64   LogicalClusterNumberOfMFTMirr;
    UINT32   ClustersPerFileRecordSegment;
    UINT32   ClustersPerIndexBlock;
    UINT64   VolumeSerialNumber;
    UINT32   CheckSum;
    UINT8    BootStrapCode[426];
    UINT8    EndOfSectorMarker[2];//55h AAh
} PEI_NTFS_BPB_Info;

//
// Buffer structure for MFT Record
//
typedef struct {
    UINT8 data[1024];
} NTFS_MFT_Buff;

//
// Buffer structure for half of Index Record
// size of index record is 4096, but this value is too big to be compiled successfully
// Set index size as 2048 is due to this value is the maximal value which could be successfully compiled of 2 to power of n
//
typedef struct {
    UINT8 data[2048];
} NTFS_INDX_Buff;


//
// Buffer structure for read the data from storage (one sector)
//
typedef struct {
    UINT8 data[512];
} NTFS_Buffer_Sector;

//
// structure for read partition information
//
typedef struct {
    UINT8  Jumpcode[3];                       //0x0000 3 Jump to the boot loader routine
    UINT32 Signature;                         //0x0003 4 System Id: "NTFS"
    UINT32 SignatureSpace;                    //0x0007 4 System Id: "    "
    UINT16 BytePerSector;                     //0x000B 2 Bytes per sector
    UINT8  SectorPerCluster;                  //0x000D 1 Sectors per cluster
    UINT8  Unused1[7];                        //0x000E 7 Unused
    UINT8  MediaDescriptor;                   //0x0015 1 Media descriptor (a)
    UINT8  Unused2[2];                        //0x0016 2 Unused
    UINT16 SectorPerTrack;                    //0x0018 2 Sectors per track
    UINT16 NumberOfHeads;                     //0x001A 2 Number of heads
    UINT8  Unused3[8];                        //0x001C 8 Unused
    UINT8  Unused4[4];                        //0x0024 4 Usually 80 00 80 00 (b)
    UINT64 NumberOfSector;                    //0x0028 8 Number of sectors in the volume
    UINT64 MFTAddr;                           //0x0030 8 LCN of VCN 0 of the $MFT
    UINT64 MFTMirrAddr;                       //0x0038 8 LCN of VCN 0 of the $MFTMirr
    UINT32 ClusterPerMFTRecord;               //0x0040 4 Clusters per MFT Record (c)
    UINT32 ClusterPerIndexRecord;             //0x0044 4 Clusters per Index Record (c)
    UINT8  PartitionBody[440];                // 512-0x48
} NTFSPartitionTable;

#pragma pack()

#endif
