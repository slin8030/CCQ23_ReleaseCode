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

/*STATIC CHAR8 OtherChars[] = {'0','1','2','3','4','5','6','7',
                             '8','9','\\','.','_','^','$','~',
                             '!','#','%','&','-','{','}','(',
                             ')','@','`','\'', '\0'};
*/

//
// Defines
//
#define CHAR_NTFS_VALID  0x01

//STATIC
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
{
    if ('a' <= a && a <= 'z') {
        return (CHAR16) (a - 0x20);
    } else {
        return a;
    }
}

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

  Reads a block of data from the block device by calling
  underlying Block i/o service

--*/
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    PrivateData - add argument and description to function comment
// GC_TODO:    BlockDeviceNo - add argument and description to function comment
// GC_TODO:    Lba - add argument and description to function comment
// GC_TODO:    BufferSize - add argument and description to function comment
// GC_TODO:    Buffer - add argument and description to function comment
// GC_TODO:    EFI_DEVICE_ERROR - add return value to function comment
// GC_TODO:    EFI_DEVICE_ERROR - add return value to function comment
{
    EFI_STATUS            Status;
    PEI_NTFS_BLOCK_DEVICE  *BlockDev;

    if (BlockDeviceNo > PEI_NTFS_MAX_BLOCK_DEVICE - 1) {
        return EFI_DEVICE_ERROR;
    }

    Status    = EFI_SUCCESS;
    BlockDev  = &(PrivateData->BlockDevice[BlockDeviceNo]);

    if (BufferSize > MultU64x32 (BlockDev->LastBlock - CHS + 1, BlockDev->BlockSize)) {
        return EFI_DEVICE_ERROR;
    }

    if (!BlockDev->Logical) {
        //
        // Status = BlockDev->ReadFunc
        //  (PrivateData->PeiServices, BlockDev->PhysicalDevNo, Lba, BufferSize, Buffer);
        //
        Status = BlockDev->BlockIo->ReadBlocks (
                     PrivateData->PeiServices,
                     BlockDev->BlockIo,
                     BlockDev->PhysicalDevNo,
                     CHS,
                     BufferSize,
                     Buffer
                 );

    } else {
        Status = NTFSReadDisk (
                     PrivateData,
                     BlockDev->ParentDevNo,
                     BlockDev->StartingPos + MultU64x32 (CHS,
                                                         BlockDev->BlockSize),
                     BufferSize,
                     Buffer
                 );
    }

    return Status;
}

EFI_STATUS
NTFSGetBlockSize (
    IN  PEI_NTFS_PRIVATE_DATA   *PrivateData,
    IN  UINTN                  BlockDeviceNo,
    OUT UINTN                  *BlockSize
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PrivateData   - GC_TODO: add argument description
  BlockDeviceNo - GC_TODO: add argument description
  BlockSize     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
    EFI_STATUS  Status;

    Status      = EFI_SUCCESS;

    *BlockSize  = PrivateData->BlockDevice[BlockDeviceNo].BlockSize;
    return Status;
}

EFI_STATUS
NTFSGetCacheBlock (
    IN  PEI_NTFS_PRIVATE_DATA  *PrivateData,
    IN  UINTN                 BlockDeviceNo,
    IN  UINT64                Lba,
    OUT CHAR8                 **CachePtr
)
/*++

Routine Description:

  Find a cache block designated to specific Block device and Lba.
  If not found, invalidate an oldest one and use it. (LRU cache)

Arguments:

  PrivateData        - the global memory map.
  BlockDeviceNo - the Block device.
  Lba           - the Logical Block Address
  CachePtr      - Ptr to the starting address of the memory holding the data;

Returns:

  EFI_SUCCESS       - OK.
  EFI_DEVICE_ERROR  - Something error while accessing media.

 --*/
{
    EFI_STATUS            Status;
    PEI_NTFS_CACHE_BUFFER  *CacheBuffer;

    /*UINT32                Lru;*/
    INTN                  Index;
    static UINT8          Seed;

    Status      = EFI_SUCCESS;
    CacheBuffer = NULL;

    //
    // go through existing cache buffers
    //
    for (Index = 0; Index < PEI_NTFS_CACHE_SIZE; Index++) {
        CacheBuffer = &(PrivateData->CacheBuffer[Index]);
        if (CacheBuffer->Valid && CacheBuffer->BlockDeviceNo == BlockDeviceNo && CacheBuffer->Lba == Lba) {
            break;
        }
    }

    if (Index < PEI_NTFS_CACHE_SIZE) {
        *CachePtr = (CHAR8 *) CacheBuffer->Buffer;

        /*    //
        // Update Lru values. each Buffer's lru ranges from 0 to PEI_NTFS_CACHE_SIZE
        //
        Lru = CacheBuffer->Lru;
        for (Index = 0; Index < PEI_NTFS_CACHE_SIZE; Index ++) {
          if (PrivateData->CacheBuffer[Index].Lru > Lru) {
            PrivateData->CacheBuffer[Index].Lru --;
          }
        }
        CacheBuffer->Lru = PEI_NTFS_CACHE_SIZE - 1;
        */
        return EFI_SUCCESS;
    }
    //
    // We have to find an invalid cache buffer
    //
    for (Index = 0; Index < PEI_NTFS_CACHE_SIZE; Index++) {
        if (!PrivateData->CacheBuffer[Index].Valid) {
            break;
        }
    }
    //
    // Not found, we have to invalidate one
    //

    /*if (Index == PEI_NTFS_CACHE_SIZE) {
     for (Index = 0; Index < PEI_NTFS_CACHE_SIZE; Index ++) {
        if (PrivateData->CacheBuffer[Index].Lru == 0) {
          break;
        }
      }
      if (Index == PEI_NTFS_CACHE_SIZE) {
        Index = 0;
      }
    }*/

    //
    // Use the cache buffer
    //
    if (Index == PEI_NTFS_CACHE_SIZE) {
        Index = (Seed++) % PEI_NTFS_CACHE_SIZE;
    }

    CacheBuffer                 = &(PrivateData->CacheBuffer[Index]);

    CacheBuffer->BlockDeviceNo  = BlockDeviceNo;
    CacheBuffer->Lba            = Lba;
    Status                      = NTFSGetBlockSize (PrivateData, BlockDeviceNo, &CacheBuffer->Size);
    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    /*  //
    // Update the Lru values
    //
    Lru = CacheBuffer->Lru;
    for (Index = 0; Index < PEI_NTFS_CACHE_SIZE; Index ++) {
      if (PrivateData->CacheBuffer[Index].Lru > Lru) {
        PrivateData->CacheBuffer[Index].Lru --;
      }
    }
    CacheBuffer->Lru = PEI_NTFS_CACHE_SIZE - 1;
    */

    //
    // Read in the data
    //
    Status = NTFSReadBlock (
                 PrivateData,
                 BlockDeviceNo,
                 Lba,
                 CacheBuffer->Size,
                 CacheBuffer->Buffer
             );
    if (EFI_ERROR (Status)) {
        return EFI_DEVICE_ERROR;
    }

    CacheBuffer->Valid  = TRUE;
    *CachePtr           = (CHAR8 *) CacheBuffer->Buffer;

    return Status;
}

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

  Disk reading.

Arguments:

  PrivateData          - the global memory map;
  BlockDeviceNo   - the block device to read;
  StartingAddress - the starting address.
  Size            - the amount of data to read.
  Buffer          - the buffer holding the data

Returns:

  EFI_SUCCESS       - OK.
  EFI_DEVICE_ERROR  - Something error.

--*/
{
    EFI_STATUS          Status;
    UINTN               BlockSize;
    CHAR8              *BufferPtr;
    CHAR8              *CachePtr;
    UINT64              CHS;

    CHS = StartingAddress;

    Status    = EFI_SUCCESS;
    BufferPtr = Buffer;
    Status    = NTFSGetBlockSize (PrivateData, BlockDeviceNo, &BlockSize);
    if (EFI_ERROR (Status))
    {
        return EFI_DEVICE_ERROR;
    };

    // Read underrun
    Status = NTFSGetCacheBlock (PrivateData, BlockDeviceNo, CHS, &CachePtr);

    if (EFI_ERROR (Status))
    {
        return EFI_DEVICE_ERROR;
    };

    Status = NTFSReadBlock (PrivateData, BlockDeviceNo, CHS, Size, BufferPtr);

    return Status;
}
