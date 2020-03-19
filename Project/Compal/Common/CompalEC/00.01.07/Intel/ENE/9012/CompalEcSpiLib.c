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
 1.00      2016.06.08   Stan         Init version.
 1.01      2017.04.27   Ivan         Fix began flash ec that have not compare data after 0x10000.
*/

#include <CompalEcSpiLib.h>


UINT8
EcSpiRead (
  IN UINT16    ENESPIReg
  )
{
  IoWrite8 ((EC_INDEX_IO_BASE + 1), (UINT8)(ENESPIReg >> 8));
  IoWrite8 ((EC_INDEX_IO_BASE + 2), (UINT8)ENESPIReg);
  return (IoRead8 (EC_INDEX_IO_BASE + 3));
};

VOID
EcSpiWrite (
  IN UINT16    ENESPIReg,
  IN UINT8     ENESPICMD
  )
{
  IoWrite8 ((EC_INDEX_IO_BASE + 1), (UINT8)(ENESPIReg >> 8));
  IoWrite8 ((EC_INDEX_IO_BASE + 2), (UINT8)ENESPIReg);
  IoWrite8 ((EC_INDEX_IO_BASE + 3), ENESPICMD);
};


VOID
EcSpiWait (
  VOID
  )
{
  while ((EcSpiRead (ISP_SPICFG) & 0x02) != 0);
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
  IN         UINT8    *Data, 
  IN  OUT    UINT16   *EraseTable,
  IN         UINT32   Pages,
  IN  CONST  UINT8    ErasePolicy
  )
/*++

Routine Description:
  Flash EC speed up function

Arguments:
  Address       EC ROM Start address
  *Data         Data pointer
  *EraseTable   erase page table for speed up
  Pages         Total pages for flashing

Returns:
    
--*/
{
  UINT16     i, j, k;
  UINT16     index = 0;
  UINTN      StartAddress;
  UINT8      *StartDataAddress;
  UINT8      count = 0;
  UINT16     EcMappingAdress;

  StartAddress = Address;
  StartDataAddress = Data;
  
  switch (ErasePolicy) {
    //
    // read ec mapping windows to create a erase table
    //
    case EcMappingWindows:
    EcSpiWrite(FMWCSR, 0x80);    // Enable the function of flash mapping window.

    for (j = 0, k = 0; j < Pages; j++, k--, StartDataAddress += PAGE_SIZE) {

      //
      // caculate the mapping windows offset
      //
      if (k == 0) {
        EcSpiWrite(FMWBA, count++);
        k = EC_MAPPING_WINDOWS_SIZE / PAGE_SIZE;
      }

      //
      // caculate a address which should be read from ec mapping windows.
      //
      EcMappingAdress = EC_MAPPING_WINDOWS_BASE + (j % (EC_MAPPING_WINDOWS_SIZE / PAGE_SIZE)) * PAGE_SIZE;
      for (i = 0; i < PAGE_SIZE; i++) {
        //
        // compare spi data to decide which page should be earsed.
        //
        if (EcSpiRead(EcMappingAdress++) != StartDataAddress[i]) {
          EraseTable[index] = j;
          index++;
          break;
        }
      }

#ifdef PROGRESS_BAR_DISPLAY
      if (j % 20 == 0x00)
        ProgressBar((UINT8)(j / 20));
#endif
    }

    EcSpiWrite(FMWCSR, 0x00);    // Disable the function of flash mapping window.
    break;

    //
    // read ec index io to create a erase table
    //
    case EcIndexIo:
    for (j = (UINT16)(StartAddress / PAGE_SIZE); j < Pages ; j++) {
      Address = StartAddress     + PAGE_SIZE * j;
      Data    = StartDataAddress + PAGE_SIZE * j;

      EcSpiWait();
      EcSpiWrite (ISP_SPIADD2, ((Address >> 16) & 0xff));
      EcSpiWrite (ISP_SPIADD1, ((Address >> 8) & 0xff));

      for (i = 0; i < PAGE_SIZE; i++, Address++) {
        EcSpiWrite (ISP_SPIADD0, (Address & 0xff));
        EcSpiWrite (ISP_SPICMD, EFCMD_Read);
        EcSpiWait();
        //
        // compare spi data to decide which page should be earsed.
        //
        if (EcSpiRead(ISP_SPIDAT) != Data[i]) {
          EraseTable[index] = j;
          index++;
          break;
        }
      }
#ifdef PROGRESS_BAR_DISPLAY
      if (j % 20 == 0x00)
        ProgressBar((UINT8)(j / 20));
#endif
    }
    break;

    //
    // erase all of pages
    //
    case EraseAllPages:
    default :
    for (j = (UINT16)(StartAddress / PAGE_SIZE); j < Pages ; j++) {
      EraseTable[j] = j;
#ifdef PROGRESS_BAR_DISPLAY
      if (j % 20 == 0x00)
        ProgressBar((UINT8)(j / 20));
#endif
    }

    break;
  } // end of switch case

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
  UINT16   i;
  UINTN    StartAddress;

  StartAddress = (UINTN)Address;

  StartSpi();
  EcSpiWait();

  for (i = 0; i < BufferSize; i++, StartAddress++, *DataBuffer++) {
    EcSpiWrite (ISP_SPIADD2, ((StartAddress >> 16) & 0xff));
    EcSpiWrite (ISP_SPIADD1, ((StartAddress >> 8) & 0xff));
    EcSpiWrite (ISP_SPIADD0, (StartAddress & 0xff));
    EcSpiWrite (ISP_SPICMD, EFCMD_Read);
    EcSpiWait();
    *DataBuffer = EcSpiRead(ISP_SPIDAT);
  }
  
  StopSpi(); 

  return EFI_SUCCESS;
}

VOID
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
  UINTN       i, j;
  UINT32      Pages;
  UINT8       iDAC;
  UINT16      *EraseTable;
  UINT16      index = 0;
  UINTN       StartAddress;
  UINT8       *StartDataAddress;

  Pages = (UINT32) (FlashImageSize / PAGE_SIZE);

  EraseTable = AllocatePool ((Pages + 1) * sizeof(UINT16));

  for (i = 0; i < Pages + 1; i++) {
    EraseTable[i] = END_OF_ERASE_TABLE;
  }

  //
  // Step 1 : execute flash speed up function to create a erase table.
  //
  FlashSpeedUp(Address, FlashingDataBuffer, EraseTable, Pages, ErasePolicy);

  //
  // iDAC with different erase/program timing
  //
  iDAC = EcSpiRead(ISP_LPCCSR) | 0x10;
  EcSpiWrite (ISP_LPCCSR, iDAC);

  EcSpiWait();

  if (EcSpiRead (ISP_LPCCSR) & 0x10) {
  	EcSpiWrite(ISP_ECSTS, (EcSpiRead(ISP_ECSTS) & 0xFC) | 0x04);
    EcSpiWrite(ISP_CLKCFG2, 0x0F);
  }

  //
  // Step 2 : erase pages by EraseTable
  //
  index = 0;
  while( EraseTable[index] != END_OF_ERASE_TABLE ) {
    CompalPageErase(Address + (PAGE_SIZE * EraseTable[index]));
#ifdef PROGRESS_BAR_DISPLAY
    if ((index / 40 ) < 25)
      if (index % 40 == 0x00)
        ProgressBar(50 + (UINT8)(index / 40));
#endif
    index++;
  }

  //
  // iDAC with different erase/program timing.
  //
  iDAC = EcSpiRead(ISP_LPCCSR) | 0x10;
  EcSpiWrite (ISP_LPCCSR, iDAC);

  EcSpiWait();
  
  if (EcSpiRead (ISP_LPCCSR) & 0x10) { 
    EcSpiWrite (ISP_ECSTS, (EcSpiRead(ISP_ECSTS) & 0xFC) | 0x04); 
    EcSpiWrite (ISP_CLKCFG2, 0x08);
  }
    
  //
  // Step 3 : Programing Pages by EraseTable
  //
  index = 0;
  StartAddress = Address;
  StartDataAddress = FlashingDataBuffer;

  while( EraseTable[index] != END_OF_ERASE_TABLE ) {
    Address  = StartAddress     + PAGE_SIZE * EraseTable[index];
    FlashingDataBuffer = StartDataAddress + PAGE_SIZE * EraseTable[index];

    EcSpiWait();
    EcSpiWrite (ISP_SPICMD,EFCMD_ClrHVPL);         // clear page buffer.
    EcSpiWrite (ISP_SPIADD2, ((Address>>16) & 0xff) );
    EcSpiWrite (ISP_SPIADD1, ((Address>>8) & 0xff) );

    for ( j = 0 ; j < PAGE_SIZE ; j++) {
      EcSpiWrite (ISP_SPIADD0, (Address & 0xff) );
      EcSpiWrite (ISP_SPIDAT, *FlashingDataBuffer );
      EcSpiWrite (ISP_SPICMD, EFCMD_PageLatch); 
      Address++;
      *FlashingDataBuffer++;
    }
    EcSpiWrite (ISP_SPICMD, EFCMD_ProgramSP);      // Program page buffer to flash.
    index++;

#ifdef PROGRESS_BAR_DISPLAY
    //
    // Display progress bar during flashing ec.
    //
    if ((index / 40) < 25)
      if (index % 40 == 0x00)
        ProgressBar(75 + (UINT8)(index / 40));
#endif
  } // end of while loop

#ifdef PROGRESS_BAR_DISPLAY
  ProgressBar(100);
  Print(L"\r\n");
#endif

  if (EcSpiRead(ISP_LPCCSR) & 0x10) { 
#ifdef OEM_ENE9012_FLASH_DEBUG
    EcSpiWrite(0xFC22,EcSpiRead(0xFC22)&(~0x40)) ;
#endif
    EcSpiWait(); 
#ifdef OEM_ENE9012_FLASH_DEBUG
    EcSpiWrite(0xFC22,EcSpiRead(0xFC22)|0x40);
#endif
    EcSpiWrite (ISP_CLKCFG2, 0x1F); 
    EcSpiWrite (ISP_ECSTS, EcSpiRead(ISP_ECSTS) & 0xF8); 
  }
  
  FreePool (EraseTable);
}

EFI_STATUS
StartSpi (
  VOID    
  )
{
  UINT8    SPICFG;

  SPICFG = EcSpiRead (ISP_SPICFG);
  SPICFG |= 0x08;
  EcSpiWait();
  EcSpiWrite (ISP_SPICFG, SPICFG); 
  EcSpiWait();
  
  return EFI_SUCCESS;
}

EFI_STATUS
StopSpi (
  VOID    
  )
{
  UINT8    SPICFG;

  SPICFG = EcSpiRead (ISP_SPICFG);
  SPICFG &= ~0x08;
  EcSpiWait();
  EcSpiWrite (ISP_SPICFG, SPICFG); 
  EcSpiWait();
  
  return EFI_SUCCESS;
}

VOID
CompalPageErase (
  IN UINTN Address
)
{
  EcSpiWait();

  EcSpiWrite (ISP_SPIADD2, ((Address>>16) & 0xff) );
  EcSpiWrite (ISP_SPIADD1, ((Address>>8) & 0xff) );    
  EcSpiWrite (ISP_SPIADD0, (Address & 0xff) );

  EcSpiWrite (ISP_SPICMD, EFCMD_PageErase);

  return;
}

EFI_STATUS
CompalSpiEcFlash (
  IN        UINT8  *FlashingDataBuffer,
  IN        UINTN  SizeToFlash,
  IN        UINT8  DestBlockNo, 
  IN CONST  UINT8  ErasePolicy
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
  UINTN    FlashImageSize;
  UINTN    Address;


  /*
    0x1F000 ~ 0x1FFFF (EC EEPROM region)
    Total protect size = 4K (0x1000)
  */
  FlashImageSize = SizeToFlash;
  Address = DestBlockNo * 0x10000;

  if ((SizeToFlash != 0x10000) || (DestBlockNo == 1)) {
    FlashImageSize = SizeToFlash - 0x1000;
  }

  StartSpi();

  //
  // flash ec rom by FlashImageSize
  //
  CompalWriteEcRom(Address, FlashingDataBuffer, FlashImageSize, ErasePolicy);

  StopSpi(); 

  return EFI_SUCCESS;
}

VOID
CompalEcRomDump (
  IN OUT   CHAR8   *EcRomBuffer,
  IN       UINTN   SizeToFlash
  )
{
  UINTN      i, k;
  UINT8      count = 0;
  UINT16     EcMappingAdress = EC_MAPPING_WINDOWS_BASE;

  StartSpi();
  EcSpiWrite(FMWCSR, 0x80);    // Enable the function of flash mapping window.

#ifdef PROGRESS_BAR_DISPLAY
  Print(L" Compal RomDump Start.\r\n");
#endif 

  for (i = 0, k = 0; i < SizeToFlash; i++, k--) {
    //
    // caculate the mapping windows offset
    //
    if (k == 0) {
      EcSpiWrite(FMWBA, count++);
      k = EC_MAPPING_WINDOWS_SIZE;
      EcMappingAdress = EC_MAPPING_WINDOWS_BASE;
    }

    EcRomBuffer[i] = EcSpiRead(EcMappingAdress++);

#ifdef PROGRESS_BAR_DISPLAY
    //
    // Caculate the percent for ProgressBar
    //
    if (i % 0x51E == 0x00)
      if ((i / 0x51E) <= 100)
        ProgressBar((UINT8)(i / 0x51E));
#endif
  }

#ifdef PROGRESS_BAR_DISPLAY
  ProgressBar(100);
  Print(L"\r\n");
  Print(L" Compal RomDump Complete.\r\n");
#endif

  EcSpiWrite(FMWCSR, 0x00);    // Disable the function of flash mapping window.
  StopSpi();

}
