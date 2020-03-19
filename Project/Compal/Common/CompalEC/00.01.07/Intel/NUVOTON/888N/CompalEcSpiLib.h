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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y

 Ver       Date         Who          Change
 --------  ----------   --------     ----------------------------------------------------
 1.00      2016.07.05   Stan         Init version.
*/

#ifndef _COMPAL_EC_SPI_LIBRARY_H
#define _COMPAL_EC_SPI_LIBRARY_H

#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#ifdef PROGRESS_BAR_DISPLAY
#include <Library/UefiLib.h>
#endif

#define EC_INDEX_IO_BASE                FixedPcdGet16(PcdEcIoBaseAddress)     // ec index io address

#define END_OF_ERASE_TABLE            0xFFFF

//
// erase table policy
//
typedef enum _NuvotonErasePolicy {
  EcWcb = 0,
  EcWcb2,
  EraseAllSectors 
} NuvotonErasePolicy;

//
// block and sector sizes
//
#define PAGE_SIZE_256B                0x100         // 256 Bytes Page size
#define SECTOR_SIZE_4KB               0x1000        // 4kBytes sector size
#define BLOCK_SIZE_32KB               0x00008000    // 32Kbytes block size
#define BLOCK_SIZE_64KB               0x00010000    // Common 64kBytes block size
#define MAX_FWH_SIZE                  0x00100000    // 8Mbit (Note that this can also be used for the 4Mbit )

//
// WCB Commands
//
#define FLASH_COMMANDS_INIT_OP        0x5A
#define WCB_ENTER_CODE                0xBECDAA55
#define ENTER_OP                      0x10
#define EXIT_OP                       0x20
#define RESET_EC_OP                   0x21
#define GOTO_BOOT_BLOCK_OP            0x22
#define ERASE_OP                      0x80
#define SECTOR_ERASE_OP               0x82
#define ADDRESS_SET_OP                0xA0
#define PROGRAM_OP                    0xB0
#define READ_IDS_OP                   0xC0
#define READ_SPI_OP                   0xD0


//
// WCB protocol  size is 16 bytes - maximum data size is 8 bytes
//
#define WCB_MAX_DATA                  8
#define WCB_SIZE_ERASE_CMD            5
#define WCB_SIZE_READ_IDS_CMD         1
#define WCB_SIZE_ENTER_CMD            5
#define WCB_SIZE_EXIT_CMD             1
#define WCB_SIZE_ADDRESS_SET_CMD      5
#define WCB_SIZE_BYTE_CMD             2
#define WCB_SIZE_WORD_CMD             (WCB_SIZE_BYTE_CMD + 1)
#define WCB_SIZE_DWORD_CMD            (WCB_SIZE_WORD_CMD + 2)
#define WCB_SIZE_LONGLONG_CMD         (WCB_SIZE_DWORD_CMD + 4)
#define WCB_SIZE_GENERIC_CMD          8   
#define WCB_MAX_READ_SIZE             12

//
// Get WCB base address via EC SIO
//
#define EC_SIO_INDEX                  0x2E
#define EC_SIO_DATA                   0x2F
#define EC_SIO_LDN_REG                0x07
#define EC_SIO_SHM_NUM                0x0F
#define EC_WCB_BASE                   0xF8

//
// Semaphore bits
//
#define SHAW2_SEM_EXE                 0x1
#define SHAW2_SEM_ERR                 0x40
#define SHAW2_SEM_RDY                 0x80

//
// Command buffer offsets
//
#define OFFSET_SEMAPHORE              0
#define OFFSET_COMMAND                3
#define OFFSET_DATA                   4             // Commands have either address or data
#define OFFSET_DATA_GENERIC           8             // Generic commands have both address and dat

#define NPCE885N                      0x1240EF      // Winbond
#define NPCE885N_2                    0x1240C8      // Giga


#pragma pack(1)
typedef struct
{
  UINT8   Read_Device_ID;
  UINT8   Write_Status_Enable;
  UINT8   Write_Enable;
  UINT8   Read_Status_Reg;
  UINT8   Write_Status_Reg;
  UINT8   Page_Program;
  UINT8   Sector_Erase;
  UINT8   Status_Busy_Mask;
  UINT8   Status_Reg_Val;
  UINT8   Program_Unit_Size;
  UINT8   Page_Size;
  UINT8   Extra_Command;  
} FLASH_DEVICE_COMMAND;

typedef struct WcbStruct
{
  UINT8   Command;
  union
  {
    FLASH_DEVICE_COMMAND    InitCommands;
    UINT32                  EnterCode;
    UINTN                   Address;
    UINT8                   Byte;
    UINT16                  Word;
    UINT32                  DWord;
    UINT64                  QWord;
  } Param;
} WcbStruct;

#pragma pack()

UINT32
GetWcbBase (
    VOID
  );

EFI_STATUS
SpiRunCmd (
  IN  UINT8   CommandLength
  );

EFI_STATUS
ShmGoToBlock (
  VOID
  );

EFI_STATUS
ShmResetEc (
  VOID
  );

EFI_STATUS
ShmInitPage (
  VOID
  );

UINT32
ShmReadID (
  VOID
  );

EFI_STATUS
ShmEnter (
  VOID
  );

EFI_STATUS
EraseSpi (
  IN  UINTN                     FlashAddress,
  IN  UINTN                     Size
  );

EFI_STATUS
ShmSetAddress (
  IN  UINTN    Address
  );

EFI_STATUS
SectorErase (
  IN  UINTN  ErseAddress
 );

EFI_STATUS
NuvotonProgramSpi (
  IN  UINTN                     FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINTN                     SpiBufferSize
  );

VOID
ShmRead96 (
  IN  OUT  UINT64                *SpiData,
  IN  OUT  UINT32                *SpiData2
  );

UINT64
ShmRead64 (
  VOID
  );

UINT32
ShmRead32 (
  VOID
  );

EFI_STATUS
ShmProgram64 (
  IN  UINT64    *Data
  );

EFI_STATUS
ShmExit (
  VOID
  );

VOID
ProgressBar (
  IN  UINT8  Percent
  );

VOID
FlashSpeedUp (
  IN         UINTN    Address,
  IN         UINT8    *FlashingDataBuffer, 
  IN  OUT    UINT16   *EraseTable,
  IN         UINT32   Sectors,
  IN  CONST  UINT8    ErasePolicy
  );

EFI_STATUS
CompalEcReadSPIData (
  IN      BOOLEAN    IdleMode,
  IN  OUT UINT8      *DataBuffer,
  IN      UINT8      *Address,
  IN      UINTN      BufferSize
  );


EFI_STATUS
CompalWriteEcRom (
  IN  UINTN         Address,
  IN  UINT8         *FlashingDataBuffer,
  IN  UINTN         FlashImageSize,
  IN  CONST  UINT8  ErasePolicy
  );

EFI_STATUS
CompalSpiEcFlash (
  IN         UINT8  *FlashingDataBuffer,
  IN         UINTN  SizeToFlash,
  IN         UINT8  DestBlockNo, 
  IN  CONST  UINT8  ErasePolicy
  );


VOID
CompalEcRomDump (
  IN OUT   CHAR8   *EcRomBuffer,
  IN       UINTN   SizeToFlash
  );

#ifdef PROGRESS_BAR_DISPLAY
extern EFI_SYSTEM_TABLE        *gST;
#endif

#endif

