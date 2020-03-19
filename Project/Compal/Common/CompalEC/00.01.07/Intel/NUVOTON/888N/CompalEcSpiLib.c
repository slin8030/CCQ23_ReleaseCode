/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2020 Compal Electronics, Inc.. All rights reserved.

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

#include <CompalEcSpiLib.h>

static UINT32  WcbBaseAddress = 0x00;
WcbStruct  WCB;

UINT32
GetWcbBase (
    VOID
  )
/*++

Routine Description:

  Get WCB base address via SHM Shard RAM WINDOWS 2

Arguments:

Returns:

  WCB base address

--*/
{
  UINT32    WcbBaseAddress = 0x00;
  UINT8     i;

  //
  // Select Shared Memory LDN
  // 
  IoWrite8(EC_SIO_INDEX, EC_SIO_LDN_REG);
  IoWrite8(EC_SIO_DATA,  EC_SIO_SHM_NUM);

  //
  // Get WCB base address from SIO
  //
  for (i = 0; i < 4; i++) {
    IoWrite8(EC_SIO_INDEX, EC_WCB_BASE + i);
    WcbBaseAddress |= (IoRead8(EC_SIO_DATA) << (8 * i));
  }

  return WcbBaseAddress;
}


EFI_STATUS
SpiRunCmd (
  IN  UINT8   CommandLength
  )
{
  UINT8   SPIStatus;

  //
  // 1. Wait for RDY bit to be 0.
  //
  do {
    SPIStatus = MmioRead8 ((UINTN) WcbBaseAddress);
  } while (SPIStatus & SHAW2_SEM_RDY);

  //
  // 2. Write the command to the WCB.
  //
  CopyMem ((VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA), &(WCB.Param), CommandLength - 1);
  CopyMem ((VOID*)(UINTN)(WcbBaseAddress + OFFSET_COMMAND), &(WCB.Command), 1);

  //
  // 3. Set EXE to 1.
  //
  MmioOr8 ((UINTN) WcbBaseAddress, SHAW2_SEM_EXE);

  if (WCB.Command == RESET_EC_OP) return EFI_SUCCESS;

  //
  // 4. Wait for RDY bit to be 1.
  //
  do {
    SPIStatus = MmioRead8 ((UINTN) WcbBaseAddress);
  } while (!(SPIStatus & SHAW2_SEM_RDY));

  //
  // 5. Clear EXE to 0.
  //
  MmioAnd8 ((UINTN) WcbBaseAddress, ~SHAW2_SEM_EXE & 0xFF);

  return EFI_SUCCESS;
}

EFI_STATUS
ShmGoToBlock (
  VOID
  )
{
  WCB.Command = GOTO_BOOT_BLOCK_OP;
  SpiRunCmd(WCB_SIZE_EXIT_CMD);

  return EFI_SUCCESS;
}

EFI_STATUS
ShmResetEc (
  VOID
  )
{
  WCB.Command = RESET_EC_OP;
  SpiRunCmd(WCB_SIZE_EXIT_CMD);

  return EFI_SUCCESS;
}


UINT32
ShmReadID(
  )
{
  UINT32  RomId;

  ZeroMem ((VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA) , sizeof (UINT32));
  WCB.Command = READ_IDS_OP;
  SpiRunCmd(WCB_SIZE_READ_IDS_CMD);      // Execute the Command in WCB
  RomId = (UINT32)(MmioRead32 ((UINTN)WcbBaseAddress + OFFSET_DATA) & 0xFFFFFFFF);
  return RomId;
}

EFI_STATUS
ShmInitPage (
  )
{
  UINT32                      SpiID;
  FLASH_DEVICE_COMMAND WCB_Init_FLASH = {0x9F, 0x06, 0x06, 0x05, 0x01, 0x02, 0x20, 0x01, 0x00, 0x02, 0x00, 0x47};

  MmioAnd8 ((UINTN) WcbBaseAddress, ~SHAW2_SEM_EXE & 0xFF);
  WCB.Param.InitCommands = WCB_Init_FLASH;
  WCB.Command = FLASH_COMMANDS_INIT_OP;
  SpiRunCmd (1 + sizeof(FLASH_DEVICE_COMMAND));

  //
  // Try command to read SPI ID
  //
  SpiID = ShmReadID();

  //
  // If read ID same as table ID, return success.
  //
  if ((NPCE885N == SpiID) || (NPCE885N_2 == SpiID)) {
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;

}

EFI_STATUS
ShmEnter (
  )
{
  MmioAnd8 (WcbBaseAddress, ~SHAW2_SEM_EXE & 0xFF);
  WCB.Command = ENTER_OP;
  WCB.Param.EnterCode = WCB_ENTER_CODE;
  SpiRunCmd (WCB_SIZE_ENTER_CMD);

  return EFI_SUCCESS;
}

EFI_STATUS
EraseSpi (
  IN  UINTN                     FlashAddress,
  IN  UINTN                     Size
  )
/*++

Routine Description:

  Erase the SPI flash device from LbaWriteAddress through SouthBridge

Arguments:

  FlashAddress              Erase Address

Returns:

  EFI status

--*/
{
  UINTN       SectorEraseCount;

  SectorEraseCount = Size / SECTOR_SIZE_4KB;
  ShmSetAddress ((UINT32)(UINTN)FlashAddress);
  while (SectorEraseCount) {
    SectorErase (FlashAddress);
    SectorEraseCount--;
    FlashAddress += SECTOR_SIZE_4KB;
    ShmSetAddress ((UINT32)(UINTN)FlashAddress);
  }

  return EFI_SUCCESS;

}

EFI_STATUS
ShmSetAddress (
  IN  UINTN    Address
  )
{
  WCB.Command = ADDRESS_SET_OP;
  WCB.Param.Address = Address;           // Address to program
  SpiRunCmd(WCB_SIZE_ADDRESS_SET_CMD);	 // Execute the Command in WCB

  return EFI_SUCCESS;
}

EFI_STATUS
SectorErase (
  IN  UINTN  ErseAddress
 )
{
  WCB.Command = ERASE_OP;                // Erase Command
  WCB.Param.Address = ErseAddress;       // Address which resides in the sector to be erased
  SpiRunCmd(WCB_SIZE_ERASE_CMD);

  return EFI_SUCCESS;
}

EFI_STATUS
NuvotonProgramSpi (
  IN  UINTN                     FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINTN                     SpiBufferSize
  )
/*++

Routine Description:

  Write the SPI flash device with given address and size through SouthBridge

Arguments:

  FlashAddress                  Destination Offset
  SrcAddress                    Source Offset
  BufferSize                    The size for programming
  LbaWriteAddress               Write Address

Returns:

  EFI status

--*/
{
  UINTN         WriteCount;
  UINTN         Index;
  EFI_STATUS    Status;

  WriteCount = SpiBufferSize / 8;
  for(Index = 0; Index < WriteCount; Index++)
  {
    Status = ShmSetAddress ((UINT32)(UINTN)FlashAddress);
    if (EFI_ERROR (Status))  {
        return Status;
      }
    Status = ShmProgram64 ((UINT64*)SrcAddress);
    if (EFI_ERROR (Status))  {
        return Status;
      }
    SrcAddress = SrcAddress + sizeof(UINT64);
    FlashAddress = FlashAddress + sizeof(UINT64);
  }

  return EFI_SUCCESS;
}

VOID
ShmRead96 (
  IN  OUT  UINT64                *SpiData,
  IN  OUT  UINT32                *SpiData2
  )
{
  ZeroMem ((VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA) , WCB_MAX_READ_SIZE);
  WCB.Command = READ_SPI_OP + WCB_MAX_READ_SIZE;
  SpiRunCmd(WCB_MAX_READ_SIZE + 1);

  *SpiData  = (UINT64)(MmioRead64 ((UINTN)WcbBaseAddress + OFFSET_DATA));
  *SpiData2 = (UINT32)(MmioRead32 ((UINTN)WcbBaseAddress + OFFSET_DATA + sizeof(UINT64)));
}

UINT64
ShmRead64 (
  VOID
  )
{
  UINT64    SpiData;

  ZeroMem ((VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA) , sizeof (UINT64));
  WCB.Command = (READ_SPI_OP + 0x08) | sizeof(UINT64);
  SpiRunCmd(WCB_SIZE_LONGLONG_CMD);

  SpiData = (UINT64)(MmioRead64 ((UINTN)WcbBaseAddress + OFFSET_DATA));

  return SpiData;
}

UINT32
ShmRead32 (
  VOID
  )
{
  UINT32    SpiData;

  ZeroMem ((VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA) , sizeof (UINT32));
  WCB.Command = READ_SPI_OP + 0x04;
  SpiRunCmd(WCB_SIZE_DWORD_CMD);

  SpiData = (UINT32)(MmioRead32 ((UINTN)WcbBaseAddress + OFFSET_DATA));
  return SpiData;
}



EFI_STATUS
ShmProgram64 (
  IN  UINT64    *Data
  )
{
  WCB.Command = PROGRAM_OP | sizeof(UINT64);
  WCB.Param.QWord = *Data;
  SpiRunCmd(WCB_SIZE_LONGLONG_CMD);

  return EFI_SUCCESS;
}

EFI_STATUS
ShmExit (
  VOID
  )
{
  WCB.Command = EXIT_OP;
  SpiRunCmd(WCB_SIZE_EXIT_CMD);

  return EFI_SUCCESS;
}


#ifdef PROGRESS_BAR_DISPLAY
VOID
ProgressBar (
  IN  UINT8  Percent
  )
{
  UINT8    i, SpaceIndex;
  CHAR8    Space;

  SpaceIndex = 33;
  Space = 0x20;

  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  gST->ConOut->OutputString (gST->ConOut,  L"                              ");
  gST->ConOut->OutputString (gST->ConOut,  L"                              ");
  Print(L"Progress : ");

  gST->ConOut->SetAttribute (gST->ConOut, EFI_BACKGROUND_GREEN);
  for (i = 0; i < (Percent / (100 / SpaceIndex)); i++)
    Print(L"%c", Space);

  gST->ConOut->SetAttribute (gST->ConOut, EFI_BACKGROUND_LIGHTGRAY);
  for (i = 0; i < SpaceIndex - (Percent / (100 / SpaceIndex)); i++)
    Print(L"%c", Space);

  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print(L" %3d%%\r", Percent);
}
#endif


VOID
FlashSpeedUp (
  IN         UINTN    Address,
  IN         UINT8    *FlashingDataBuffer, 
  IN  OUT    UINT16   *EraseTable,
  IN         UINT32   Sectors,
  IN  CONST  UINT8    ErasePolicy
  )
/*++

Routine Description:
  Flash EC speed up function

Arguments:
  Address              EC ROM Start address
  *FlashingDataBuffer  Data pointer
  *EraseTable          erase page table for speed up
  Sectors              Total sectors for flashing

Returns:
    
--*/
{
  UINT8      i;
  UINT16     j;
  UINT16     index = 0;
  UINT64     SpiData;
  UINTN      StartAddress;
  UINT8      *StartDataAddress;
  UINT64     *FlashingData64;
  UINT32     *FlashingData32;
  UINT32     SpiData2;
  UINT16     SectorDataCount;

  StartAddress     = Address;
  StartDataAddress = FlashingDataBuffer;

  switch (ErasePolicy) {
    //
    // read ec wcb to create a erase table
    //
    case EcWcb:
      for (i = (UINT8)(StartAddress / SECTOR_SIZE_4KB); i < Sectors; i++) {
        Address            = StartAddress     + SECTOR_SIZE_4KB * i;
        FlashingDataBuffer = StartDataAddress + SECTOR_SIZE_4KB * i;
        ShmSetAddress(Address);

        for (j = 0; j < SECTOR_SIZE_4KB / WCB_MAX_DATA; j++, FlashingDataBuffer += 8) {
          //ShmSetAddress(Address + j * WCB_MAX_DATA);
          SpiData = ShmRead64();
          FlashingData64 = (UINT64 *) FlashingDataBuffer;

          //
          // compare spi data to decide which sector should be earsed.
          //
          if (SpiData != *FlashingData64) {
            EraseTable[index] = i;
            index++;
            break;
          }
        }
#ifdef PROGRESS_BAR_DISPLAY
        ProgressBar(10 + i);
#endif
      }
      break;

    //
    // read ec wcb to create a erase table
    //
    case EcWcb2:
      for (i = (UINT8)(StartAddress / SECTOR_SIZE_4KB); i < Sectors; i++) {
        Address            = StartAddress     + SECTOR_SIZE_4KB * i;
        FlashingDataBuffer = StartDataAddress + SECTOR_SIZE_4KB * i;
        ShmSetAddress(Address);
        SectorDataCount = SECTOR_SIZE_4KB;

        while(SectorDataCount) {
          if (SectorDataCount == 4) {
            SpiData2 = ShmRead32();
            FlashingData32 = (UINT32 *) FlashingDataBuffer;
            SectorDataCount = 0;

            //
            // compare spi data to decide which sector should be earsed.
            //
            if (SpiData2 != *FlashingData32) {
              EraseTable[index] = i;
              index++;
              break;
            }
          } else {
            ShmRead96(&SpiData, &SpiData2);
            FlashingData64 = (UINT64 *) FlashingDataBuffer;
            FlashingData32 = (UINT32 *) (FlashingDataBuffer + sizeof(UINT64));
            SectorDataCount -= WCB_MAX_READ_SIZE;

            //
            // compare spi data to decide which sector should be earsed.
            //
            if ((SpiData != *FlashingData64) || (SpiData2 != *FlashingData32)) {
              EraseTable[index] = i;
              index++;
              break;
            }
          }

          FlashingDataBuffer += WCB_MAX_READ_SIZE;
        }
#ifdef PROGRESS_BAR_DISPLAY
        ProgressBar(10 + i);
#endif   
      }
      break;

    //
    // erase all of sectors
    //
    case EraseAllSectors :
    default :
      for (i = (UINT8)(StartAddress / SECTOR_SIZE_4KB); i < Sectors ; i++) {
        EraseTable[i] = i;
#ifdef PROGRESS_BAR_DISPLAY
        ProgressBar(10 + i);
#endif        
      }
      break;
  }

}

EFI_STATUS
CompalEcReadSPIData (
  IN      BOOLEAN    IdleMode,
  IN  OUT UINT8      *DataBuffer,
  IN      UINT8      *Address,
  IN      UINTN      BufferSize
)
/*++

Routine Description:
  Read data from EC ROM

Arguments:
  Adress            EC ROM Start address
  *DataBuffer       Data pointer
  BufferSize        Buffer size

Returns:
  
--*/
{

  UINT64     SpiData;
  UINT32     SpiData2;
  

  WcbBaseAddress = GetWcbBase();

  ShmSetAddress((UINTN)Address);

  while(BufferSize) {
    if (BufferSize < WCB_MAX_READ_SIZE) {
      if (BufferSize == 0x04) {
        SpiData2 = ShmRead32();
        CopyMem (DataBuffer, (VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA), BufferSize);
      } if (BufferSize == 0x08) {
        SpiData = ShmRead64();
        CopyMem (DataBuffer, (VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA), BufferSize);
      }
      BufferSize = 0x00;
    } else {
      WCB.Command = READ_SPI_OP + WCB_MAX_READ_SIZE;
      SpiRunCmd(WCB_MAX_READ_SIZE + 1);
      CopyMem (DataBuffer, (VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA), WCB_MAX_READ_SIZE);
      DataBuffer += WCB_MAX_READ_SIZE;
      BufferSize -= WCB_MAX_READ_SIZE;
    }
  }

  return EFI_SUCCESS;
}


EFI_STATUS
CompalWriteEcRom (
  IN  UINTN         Address,
  IN  UINT8         *FlashingDataBuffer,
  IN  UINTN         FlashImageSize,
  IN  CONST  UINT8  ErasePolicy
)
/*++

Routine Description:
  Write data to EC ROM

Arguments:
  Address                EC ROM Start address
  *FlashingDataBuffer    Data pointer
  FlashImageSize         Data length 

Returns:
  
--*/
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT16      *EraseTable;
  UINT8       Sectors;
  UINT16      i;
  UINT8       index = 0;
  UINTN       StartAddress;
  UINT8       *StartDataAddress;

  
  StartAddress = Address;
  StartDataAddress = FlashingDataBuffer;
  Sectors = (UINT8)(FlashImageSize / SECTOR_SIZE_4KB);

  EraseTable = AllocatePool ((Sectors+ 1) * sizeof(UINT16));
  
  for (i = 0; i < Sectors + 1; i++) {
    EraseTable[i] = END_OF_ERASE_TABLE;
  }
  
  //
  // Step 1 : Init Flash
  //
  Status = ShmInitPage();
  if (EFI_ERROR (Status))  {
    return Status;
  }
#ifdef PROGRESS_BAR_DISPLAY
  ProgressBar(5);
#endif

  //
  // Step 2 : Enter Flash Update
  //
  Status = ShmEnter();
  if (EFI_ERROR (Status))  {
    return Status;
  }
#ifdef PROGRESS_BAR_DISPLAY
  ProgressBar(10);
#endif

  //
  // Step 3 : create a erase table for speed up.
  //
  FlashSpeedUp(Address, FlashingDataBuffer, EraseTable, Sectors, ErasePolicy);  

  //
  // Step 4 : Erase flash device
  //
  index = 0;
  while (EraseTable[index] != END_OF_ERASE_TABLE) {
    Address = StartAddress + EraseTable[index] * SECTOR_SIZE_4KB;
    EraseSpi (Address, SECTOR_SIZE_4KB);
#ifdef PROGRESS_BAR_DISPLAY
    if (index < 30)
      ProgressBar(40 + index);
#endif
    index++;
  }

  //
  // Step 5 : Program flash device
  //
  index = 0;
  while (EraseTable[index] != END_OF_ERASE_TABLE) {
    Address            = StartAddress     + EraseTable[index] * SECTOR_SIZE_4KB;
    FlashingDataBuffer = StartDataAddress + EraseTable[index] * SECTOR_SIZE_4KB;
    NuvotonProgramSpi (Address, FlashingDataBuffer, SECTOR_SIZE_4KB);
#ifdef PROGRESS_BAR_DISPLAY
    if (index < 30)
      ProgressBar(70 + index);
#endif
    index++;
  }

  //
  // Step 6 : Exit Flash Update
  //
  //Status = ShmExit();
  Status = ShmGoToBlock();
  if (EFI_ERROR (Status))  {
    return Status;
  }

#ifdef PROGRESS_BAR_DISPLAY
  ProgressBar(100);
  Print(L"\r\n");
#endif

  FreePool (EraseTable);

  return Status;
}


EFI_STATUS
CompalSpiEcFlash (
  IN         UINT8  *FlashingDataBuffer,
  IN         UINTN  SizeToFlash,
  IN         UINT8  DestBlockNo, 
  IN  CONST  UINT8  ErasePolicy
  )
/*++

Routine Description:
  Caculate the FlashImageSize for flashing

Arguments:
  *FlashingDataBuffer      Ec spi data buffer from bios rom.
  SizeToFlash              Ec size for flashing
  DestBlockNo              Destination block number

Returns:
  
--*/
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINTN       FlashImageSize;
  UINTN       Address;

  /*
    0x1F000 ~ 0x1FFFF (EC EEPROM region)
    Total protect size = 4K (0x1000)
  */
  FlashImageSize = SizeToFlash;
  Address = DestBlockNo * 0x10000;

  if ((SizeToFlash != 0x10000) || (DestBlockNo == 1)) {
    FlashImageSize = SizeToFlash - 0x1000;
  }

  //
  // Get WCB base address via EC SIO before flashing.
  //
  WcbBaseAddress = GetWcbBase();

  //
  // flash ec rom by FlashImageSize
  //
  Status = CompalWriteEcRom(Address, FlashingDataBuffer, FlashImageSize, ErasePolicy);

  return Status;
}

VOID
CompalEcRomDump (
  IN OUT   CHAR8   *EcRomBuffer,
  IN       UINTN   SizeToFlash
  )
{
  UINTN      Address = 0;
  UINT64     SpiData;
  UINT32     SpiData2;

  WcbBaseAddress = GetWcbBase();

#ifdef PROGRESS_BAR_DISPLAY
  Print(L" Compal RomDump Start.\r\n");
#endif

  ShmSetAddress(Address);

  while(SizeToFlash) {
    if (SizeToFlash < WCB_MAX_READ_SIZE) {
      SpiData2 = SizeToFlash == 0x04 ? ShmRead32() : 0x00;
      SpiData  = SizeToFlash == 0x08 ? ShmRead64() : 0x00;
      CopyMem (EcRomBuffer, (VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA), SizeToFlash);
      SizeToFlash = 0x00;
    } else {
      WCB.Command = READ_SPI_OP + WCB_MAX_READ_SIZE;
      SpiRunCmd(WCB_MAX_READ_SIZE + 1);
      CopyMem (EcRomBuffer, (VOID*)(UINTN)(WcbBaseAddress + OFFSET_DATA), WCB_MAX_READ_SIZE);
      EcRomBuffer += WCB_MAX_READ_SIZE;
      SizeToFlash -= WCB_MAX_READ_SIZE;
    }

    Address += WCB_MAX_READ_SIZE;

#ifdef PROGRESS_BAR_DISPLAY
    //
    // Caculate the percent for ProgressBar
    //
    if (Address % (0x6D * WCB_MAX_READ_SIZE) == 0x00)
      if ((Address / (0x6D * WCB_MAX_READ_SIZE)) <= 100)
        ProgressBar((UINT8)(Address / (0x6D * WCB_MAX_READ_SIZE)));
#endif
  }

#ifdef PROGRESS_BAR_DISPLAY
  ProgressBar(100);
  Print(L"\r\n");
  Print(L" Compal RomDump Complete.\r\n");
#endif
}
