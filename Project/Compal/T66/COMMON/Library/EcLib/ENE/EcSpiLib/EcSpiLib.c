//;******************************************************************************
//;* Copyright (c) 1983-2006, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; Revision History:
//;
//; $Log: $
//;
//;------------------------------------------------------------------------------
//;
//; Abstract:
//;


#include <Library/EcSpiLib.h>
#include <Library/SpiAccess.h>
#include <CompalEclib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/SpiAccessLib.h>
#include <Framework/SmmCis.h>

STATIC SPI_CONFIG_BLOCK        *mSpiConfigBlock = NULL;
STATIC BOOLEAN                 mSpiDeviceType;
STATIC EFI_PHYSICAL_ADDRESS    mValidWriteRegionStart;
STATIC EFI_PHYSICAL_ADDRESS    mValidWriteRegionEnd;
STATIC EFI_PHYSICAL_ADDRESS    mValidReadRegionStart;
STATIC EFI_PHYSICAL_ADDRESS    mValidReadRegionEnd;
#define ACPI_TIMER_ADDR       0x408

//[-start-070922-IB03030019-remark]//
//UINT8
//IoRead8 (
//  IN  UINT16  Port
//  )
//{
//  UINT8 Data = 0;
//
//  _asm {
//    push   edx
//    mov    dx,  Port
//    in     al,  dx
//    mov    Data, al
//    pop    edx
//  }
//  return Data;
//}
//
//VOID
//IoOutput8 (
//  IN  UINT16  Port,
//  IN  UINT32  Data
//  )
//{
//  _asm {
//    pushad
//    mov    eax, Data
//    mov    dx,  Port
//    out    dx,  al
//    popad
//  }
//}
//
//[-end-070922-IB03030019-remark]//


UINT8
EC_ISP_Read (
//[PRJ] Start - ENE9012 flash
  //IN UINT8     ENESPIReg
  IN UINT16    ENESPIReg
//[PRJ] End - ENE9012 flash
  )
{
//[PRJ] Start - ENE9012 flash
  //IoOutput8 ((ENEIOBASE + 1), 0xFE);
  //IoOutput8 ((ENEIOBASE + 2), ENESPIReg);
  IoWrite8 ((ENEIOBASE + 1), (UINT8)(ENESPIReg >> 8));
  IoWrite8 ((ENEIOBASE + 2), (UINT8)ENESPIReg);
//[PRJ] End - ENE9012 flash

  return (IoRead8 (ENEIOBASE + 3));
};

EFI_STATUS
EC_ISP_Write (
//[PRJ] Start - ENE9012 flash
  //IN UINT8     ENESPIReg,
  IN UINT16    ENESPIReg,
//[PRJ] End - ENE9012 flash
  IN UINT8     ENESPICMD
  )
{
//[PRJ] Start - ENE9012 flash
  //IoOutput8 ((ENEIOBASE + 1), 0xFE);
  IoWrite8 ((ENEIOBASE + 1), (UINT8)(ENESPIReg >> 8));
//[-start-070922-IB03030019-add]//
//  EcSpiStall(10);
//[-end-070922-IB03030019-add]//
  //IoOutput8 ((ENEIOBASE + 2), ENESPIReg);
  IoWrite8 ((ENEIOBASE + 2), (UINT8)ENESPIReg);
//[PRJ] End - ENE9012 flash
//[-start-070922-IB03030019-add]//
//  EcSpiStall(10);
//[-end-070922-IB03030019-add]//
  IoWrite8 ((ENEIOBASE + 3), ENESPICMD);
//[-start-070922-IB03030019-add]//
//  EcSpiStall(10);
//[-end-070922-IB03030019-add]//

  return EFI_SUCCESS;
};


//[-start-070301-IB03030001-add]//
////[-start-0706013-IB03030004-modify]//
//VOID
//DelayPCI (
//  IN  UINT16  Count
//  )
//{
//
////[-start-070922-IB03030019-modify]//
//  EcSpiStall (Count);
////    AcpiStall (Count);
////[-end-070922-IB03030019-modify]//
//
////  while(Count > 0){
////    Count--;
////    _asm {
////      out    0xED, al
////    }
////  }
//}
////[-end-0706013-IB03030004-modify]//

UINT8
EC_ISP_Action_SPI_RB (
  IN UINT8     CMD
  );

EFI_STATUS
ISP_Action_SPI (
  IN UINT8     CMD
  );

EFI_STATUS
EC_ISP_Start_SPI ();

EFI_STATUS
EC_ISP_Stop_SPI ();

VOID
WaitECSPI_WIP (
  )
{
  while (EC_ISP_Action_SPI_RB (0x05) & 0x01) {
    EcSpiStall (0x10);
//    DelayPCI (0x10);
  };
}

VOID
EC_WaitSPI_Cycle (
  )
{
  while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);
}

VOID
DisableBPL (
  )
{
  UINT8		   StatusReg;

  StatusReg = EC_ISP_Action_SPI_RB (0x05);             // Read Status
  if (StatusReg & 0x3C) {
    ISP_Action_SPI (0x06);                          // Write Enable
    EC_ISP_Start_SPI ();
    EC_ISP_Write (ISP_SPICMD, 0x01);                   // Disable block protect
    EC_ISP_Write (ISP_SPICMD, 0x02);                   // Reset BP0~BP3
    EC_ISP_Stop_SPI ();
  }
}
//[-end-070301-IB03030001-add]//

EFI_STATUS
EC_ISP_Start_SPI ()
{
  //[-start-070301-IB03030001-modify]//
  UINT8     SPICFG;

  EcSpiStall (0x08);
//  DelayPCI (0x8);

  SPICFG = EC_ISP_Read (ISP_SPICFG);
  SPICFG |= 0x18;
  EC_ISP_Write (ISP_SPICFG, SPICFG);       //enable force SPICS# low

  EC_WaitSPI_Cycle();
//  DelayPCI (0x10);

//[-start-070922-IB03030019-modify]//
  EcSpiStall (0x04);
//  AcpiStall (4);
//[-end-070922-IB03030019-modify]//

////  UINT8     SPICFG;
//
////  SPICFG = ISP_Read (ISP_SPICFG);
////  SPICFG |= 0x18;
////  ISP_Write (ISP_SPICFG, SPICFG);       //enable force SPICS# low
//  ISP_Write (ISP_SPICFG, 0x18);       //enable force SPICS# low
  //[-end-070301-IB03030001-modify]//

  return EFI_SUCCESS;
}

EFI_STATUS
EC_ISP_Stop_SPI ()
{
//  UINT8     SPICFG;

  //[-start-070301-IB03030001-modify]//
  UINT8     SPICFG;

//  DelayPCI (0x10);
//[-start-070922-IB03030019-modify]//
  EcSpiStall (0x04);
//    AcpiStall (4);
//[-end-070922-IB03030019-modify]//

  SPICFG = EC_ISP_Read (ISP_SPICFG);

//[-start-071009-IB03030042-modify]//
  SPICFG &= ~0x18;
//  SPICFG &= ~0x10;
//  SPICFG |= ~0x08;
//[-end-071009-IB03030042-modify]//

  EC_ISP_Write (ISP_SPICFG, SPICFG);       //enable force SPICS# low

  EC_WaitSPI_Cycle();
  EcSpiStall (0x08);
//  DelayPCI (0x8);

//  EcStall (0x04);        // Delay 4 microseconds
////  SPICFG = ISP_Read (ISP_SPICFG);
////  SPICFG &= ~0x10;
////  SPICFG |= 0x08;
////  ISP_Write (ISP_SPICFG, SPICFG);       //disable force SPICS# low
//  ISP_Write (ISP_SPICFG, 0x08);       //disable force SPICS# low
  //[-end-070301-IB03030001-modify]//

  return EFI_SUCCESS;
}

EFI_STATUS
ISP_Action_SPI (
  IN UINT8     CMD
  )
{

  EC_ISP_Start_SPI();

  EC_ISP_Write (ISP_SPICMD, CMD);                   //SPI read ID command

  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Stop_SPI ();

  return EFI_SUCCESS;
}

EFI_STATUS
ISP_Action_SPI2 (
  IN UINT8     CMD,
  IN UINT8     A0,
  IN UINT8     A1,
  IN UINT8     A2
  )
{
  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, CMD);
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, A2);                    //SPI address 2
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, A1);                    //SPI address 1
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, A0);                    //SPI address 0
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Stop_SPI ();

  return EFI_SUCCESS;
}

UINT8
EC_ISP_Action_SPI_RB (
  IN UINT8     CMD
  )
{
  UINT8     StatusReg;

  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, CMD);                   //SPI read ID command
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, 0x00);                  //SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//
  StatusReg = EC_ISP_Read (ISP_SPIDAT);

  EC_ISP_Stop_SPI ();

  return StatusReg;
}

EFI_STATUS
ISP_Action_SPI_WB (
  IN UINT8   CMD,
  IN UINT8   Data
  )
{
  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, CMD);                    //SPI read ID command
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, Data);
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Stop_SPI ();

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
CheckValidRegion(
  IN EFI_PHYSICAL_ADDRESS       DstAddress,
  IN UINTN                      Size
  )
/*++

Routine Description:

  Check the address is fall in valid region or not

Arguments:

Returns:

  BOOLEAN

--*/
{
  return (BOOLEAN)(DstAddress >= mValidWriteRegionStart && DstAddress + Size - 1 <= mValidWriteRegionEnd);
}

/*
EFI_STATUS
EcRecognizeSPIDevice (
//[-start-070922-IB03030019-modify]//
//  IN FLASH_SPI_DEVICE           *FlashDevice
  IN VOID                       *SpiFlashDevice
//[-end-070922-IB03030019-modify]//
  )
*/
/*++

Routine Description:

  Detect and Initialize SPI flash part OpCode and other parameter through SouthBridge

Arguments:

Returns:

  EFI status

--*/
/*
{
  UINT8            CheckId[4];
  SPI_CONFIG_BLOCK *SpiConfigBlock;
  UINT32           Buffer = 0x00;

//[-start-070922-IB03030019-add]//
  FLASH_SPI_DEVICE *FlashDevice;
  FlashDevice = (FLASH_SPI_DEVICE*) SpiFlashDevice;
//[-end-070922-IB03030019-add]//

  CheckId[3] = 0x00;

  SpiConfigBlock = FlashDevice->SpiConfigBlock;

  //[-start-070301-IB03030001-add]//
  //ISP_Action_SPI (0x04);                          // Write Disable
  //[-end-070301-IB03030001-add]//

  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, SpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_READ_ID]);             // SPI read ID command
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, 0x00);		        // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//
  CheckId[0] = EC_ISP_Read (ISP_SPIDAT);

  EC_ISP_Write (ISP_SPICMD, 0x00);		        // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//
  CheckId[1] = EC_ISP_Read (ISP_SPIDAT);

  EC_ISP_Write (ISP_SPICMD, 0x00);		        // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//
  CheckId[2] = EC_ISP_Read (ISP_SPIDAT);

  EC_ISP_Stop_SPI ();

  Buffer = *(UINT32 *)CheckId;

//[-start-070922-IB03030019-modify]//
//  if (Buffer == FlashDevice->Id) {
  if (Buffer == FlashDevice->DeviceInfo.Id) {
//[-end-070922-IB03030019-modify]//
    mSpiConfigBlock = FlashDevice->SpiConfigBlock;
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}
*/

EFI_STATUS
EcEraseSPI(
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
  UINTN         NumBytes = Size;
  UINTN         DstAddress;
//[-start-070922-IB03030019-modify]//
//  UINTN         BlockEraseSize = mSpiConfigBlock->BlockMap[0] << 8;
  UINTN         BlockEraseSize = mSpiConfigBlock->BlockEraseSize;
//[-end-070922-IB03030019-modify]//

  UINT8         StatusReg, A1, A2;

  //[-start-070301-IB03030001-add]//
  WaitECSPI_WIP();
  DisableBPL ();
  //[-end-070301-IB03030001-add]//

  //_asm{ int 3}
  //
  // Make sure the flash address alignment on sector/block
  //
  FlashAddress &= ~(BlockEraseSize - 1);
  //
  // Calculate device offset
  //

//[-start-070922-IB03030019-modify]//
//  DstAddress = FlashAddress - (UINTN)(0x100000000 - (UINT64)(0x10000 << mSpiConfigBlock->SizeCode));
  DstAddress = FlashAddress - (UINTN)(0x100000000 - (UINT64)mSpiConfigBlock->DeviceSize);
//[-end-070922-IB03030019-modify]//

  StatusReg = EC_ISP_Action_SPI_RB (mSpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_READ_S]);             // Read Status

  ISP_Action_SPI (mSpiConfigBlock->PrefixMenu[SPI_PREFIX_INDEX_WRITE_EN]);                          // Write Enable

  StatusReg |= 0x02;

  ISP_Action_SPI_WB (mSpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_WRITE_S], StatusReg & 0x03);       // Write Status Register
  while (EC_ISP_Action_SPI_RB (mSpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_READ_S]) & 0x01);

  while ((INTN)NumBytes > 0)
  {
//    if (CheckValidRegion(DstAddress, BlockEraseSize))
//    {
      A2 = (UINT8) ((DstAddress & 0xFF0000) >> 16);
      A1 = (UINT8) ((DstAddress & 0xFF00) >> 8);

      ISP_Action_SPI (mSpiConfigBlock->PrefixMenu[SPI_PREFIX_INDEX_WRITE_EN]);                          // Write Enable

//      EcStall (50);    // Jonson dbg, [a]
      ISP_Action_SPI2 (mSpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_ERASE], 0x00, A1, A2);
//      EcStall (200);    // Jonson dbg, [a]

      while (EC_ISP_Action_SPI_RB (mSpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_READ_S]) & 0x01);
//[-start-070922-IB03030019-modify]//
  EcSpiStall (200);
//    AcpiStall (200);
//[-end-070922-IB03030019-modify]//

    DstAddress += BlockEraseSize;
    NumBytes   -= BlockEraseSize;
  }

  //[-start-070301-IB03030001-add]//
//  ISP_Action_SPI (0x04);                          // Write Disable
  //[-end-070301-IB03030001-add]//

  return EFI_SUCCESS;
}

EFI_STATUS
EcProgramSPI_A_BYTE (
  IN  UINTN                     StartAddress,
  IN  UINT8                     data
  )
{
  UINT8       A2, A1, A0;

  A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
  A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
  A0 = (UINT8) ((StartAddress & 0x00FF));

  //[-start-070301-IB03030001-Modify]//
  WaitECSPI_WIP();
  //while (ISP_Action_SPI_RB (0x05) & 0x01);
  //[-End-070301-IB03030001-Modify]//

  ISP_Action_SPI (0x06);                          // Write Enable

  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, 0x02);
  //[-start-070301-IB03030001-add]//
  EC_WaitSPI_Cycle();
  //[-end-070301-IB03030001-add]//

  EC_ISP_Write (ISP_SPICMD, A2);                     //SPI address 2
  //[-start-070301-IB03030001-add]//
  EC_WaitSPI_Cycle();
  //[-end-070301-IB03030001-add]//

  EC_ISP_Write (ISP_SPICMD, A1);                     //SPI address 1
  //[-start-070301-IB03030001-add]//
  EC_WaitSPI_Cycle();
  //[-end-070301-IB03030001-add]//

  EC_ISP_Write (ISP_SPICMD, A0);                   //SPI address 0
  //[-start-070301-IB03030001-add]//
  EC_WaitSPI_Cycle();
  //[-end-070301-IB03030001-add]//

  EC_ISP_Write (ISP_SPICMD, data);                 // First Byte

  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle();
  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Stop_SPI ();

  return EFI_SUCCESS;
}

EFI_STATUS
EcProgramSPI_BYTES (
  IN  UINTN                     StartAddress,
  IN  UINT8                     *data,
  IN  UINTN			BlockSize
  )
{
  UINT8       A2, A1, A0;
  UINTN       LoopCount;

  A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
  A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
  A0 = (UINT8) ((StartAddress & 0x00FF));

  WaitECSPI_WIP();

  ISP_Action_SPI (0x06);                          // Write Enable

  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, 0x02);
  EC_WaitSPI_Cycle();

  EC_ISP_Write (ISP_SPICMD, A2);                     //SPI address 2
  EC_WaitSPI_Cycle();

  EC_ISP_Write (ISP_SPICMD, A1);                     //SPI address 1
  EC_WaitSPI_Cycle();

  EC_ISP_Write (ISP_SPICMD, A0);                   //SPI address 0
  EC_WaitSPI_Cycle();

  for (LoopCount = 0; LoopCount < BlockSize; LoopCount++) {
    EC_ISP_Write (ISP_SPICMD, *data);
    EC_WaitSPI_Cycle();
    data++;
  }

  EC_ISP_Stop_SPI ();
//[-start-070922-IB03030019-modify]//
  EcSpiStall (2000);
//    AcpiStall (2000);
//[-end-070922-IB03030019-modify]//

  return EFI_SUCCESS;
}

EFI_STATUS
EcProgramSPI (
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
//[-start-070922-IB03030019-modify]//
  IN  UINTN                     *SpiBufferSize,
//  IN  UINTN                     BufferSize,
//[-end-070922-IB03030019-modify]//
  IN  UINTN                     LbaWriteAddress
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

  UINTN       StartAddress;
  UINTN       BlockSize;
//[-start-070922-IB03030019-add]//
  UINTN       BufferSize = *SpiBufferSize;
//[-end-070922-IB03030019-add]//

//  INT3 ();

  StartAddress = (UINTN)FlashAddress;

  //[-start-070301-IB03030001-modify]//
    WaitECSPI_WIP();
    DisableBPL();
//    ISP_Action_SPI (0x06);                          // Write Enable
//
//    ISP_Start_SPI ();
//
//    ISP_Write (ISP_SPICMD, 0x01);                   // Disable block protect
//    ISP_Write (ISP_SPICMD, 0x00);                   // Reset BP0~BP3
//
//    ISP_Stop_SPI ();
  //[-end-070301-IB03030001-modify]//

  //[-start-070301-IB03030001-modify]//
#if 0
    while (BufferSize!=0)
    {
      EcProgramSPI_A_BYTE(StartAddress, *SrcAddress);
      StartAddress ++;
      SrcAddress ++;
      BufferSize --;
    }
#else
    while (BufferSize!=0)
    {
      BlockSize = 0x100 - (StartAddress & 0xFF);
      if (BlockSize > BufferSize) BlockSize = BufferSize;
      EcProgramSPI_BYTES(StartAddress, SrcAddress, BlockSize);
      StartAddress += BlockSize;
      SrcAddress += BlockSize;
      BufferSize -= BlockSize;
    }
#endif
//    if ((A0 & 0x01) && (BufferSize > 0)) {
//      EcProgramSPI_A_BYTE(StartAddress, *SrcAddress ++);
//      StartAddress++;
//      BufferSize--;
//      A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
//      A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
//      A0 = (UINT8) ((StartAddress & 0x00FF));
//    }
//
//    ISP_Action_SPI (0x06);                          // Write Enable
//
//    while (ISP_Action_SPI_RB (0x05) & 0x01);
//
//    ISP_Start_SPI ();
//
//    if (BufferSize >= 2) {
//      ISP_Write (ISP_SPICMD, 0xAD);
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);
//
//      ISP_Write (ISP_SPICMD, A2);                     //SPI address 2
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);    //Wait SPI cycle finish
//
//      ISP_Write (ISP_SPICMD, A1);                     //SPI address 1
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
//
//      ISP_Write (ISP_SPICMD, A0);                   //SPI address 0
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);    //Wait SPI cycle finish
//
//      ISP_Write (ISP_SPICMD, *SrcAddress ++);            // First Byte
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);
//
//      ISP_Write (ISP_SPICMD, *SrcAddress ++);            // Second Byte
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);
//
//      BufferSize -= 2;
//      StartAddress += 2;
//    }
//
//    ISP_Stop_SPI ();
//
//    EcStall (0x0C);        // Delay 12 microseconds
//
//    for (LoopCount = 0; LoopCount < BufferSize; LoopCount += 2 )
//    {
//      if (LoopCount + 1 == BufferSize) {
//        break;
//      }
//
//      ISP_Start_SPI ();
//
//      ISP_Write (ISP_SPICMD, 0xAD);
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);    //Wait SPI cycle finish
//
//      ISP_Write (ISP_SPICMD, *SrcAddress ++);
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);
//
//      ISP_Write (ISP_SPICMD, *SrcAddress ++);
//      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);
//
//      ISP_Stop_SPI ();
//      StartAddress += 2;
//
//    EcStall (0x0C);        // Delay 12 microseconds
//
////      while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);         //Wait SPI cycle finish
//    }
//
////  ISP_Stop_SPI ();
////
////  DelayPCI (0x010C);
//  ISP_Action_SPI (0x04);
//  while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);        //Wait SPI cycle finish
//
//  if (LoopCount + 1 == BufferSize) {
//    EcProgramSPI_A_BYTE(StartAddress, *SrcAddress ++);
//  }
  //[-end-070301-IB03030001-modify]//

  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  // Jonson dbg, [d] WaitSPI_WIP();
  //ISP_Action_SPI (0x04);                          // Write Disable

  //while ((ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//
  EC_ISP_Action_SPI_RB (0x05);

  return EFI_SUCCESS;
}

//[-start-070301-IB03030001-add]//
EFI_STATUS
EcProgramSPI_SST (
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
//[-start-070922-IB03030019-modify]//
  IN  UINTN                     *SpiBufferSize,
//  IN  UINTN                     BufferSize,
//[-end-070922-IB03030019-modify]//
  IN  UINTN                     LbaWriteAddress
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

  UINTN       StartAddress;
  UINT8       A2, A1, A0;
  UINTN       LoopCount;
//[-start-070922-IB03030019-add]//
  UINTN       BufferSize = *SpiBufferSize;
//[-end-070922-IB03030019-add]//

  StartAddress = (UINTN)FlashAddress;

  A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
  A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
  A0 = (UINT8) ((StartAddress & 0x00FF));

  WaitECSPI_WIP();
  DisableBPL();
  if ((A0 & 0x01) && (BufferSize > 0)) {
    EcProgramSPI_A_BYTE(StartAddress, *SrcAddress);
    SrcAddress++;
    StartAddress++;
    BufferSize--;
    A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
    A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
    A0 = (UINT8) ((StartAddress & 0x00FF));
  }

  ISP_Action_SPI (0x06);                          // Write Enable

  EC_ISP_Start_SPI ();

  if (BufferSize >= 2) {
    EC_ISP_Write (ISP_SPICMD, 0xAD);
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, A2);                     //SPI address 2
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, A1);                     //SPI address 1
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, A0);                   //SPI address 0
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, *SrcAddress ++);            // First Byte
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, *SrcAddress ++);            // Second Byte
    EC_WaitSPI_Cycle();
    BufferSize -= 2;
    StartAddress += 2;
  }

  EC_ISP_Stop_SPI ();

//[-start-070922-IB03030019-modify]//
  EcSpiStall (0x0C);
//    AcpiStall (12);
//[-end-070922-IB03030019-modify]//

  for (LoopCount = 0; LoopCount < BufferSize; LoopCount += 2 )
  {
    if (LoopCount + 1 == BufferSize) {
      break;
    }
    EC_ISP_Start_SPI ();
    EC_ISP_Write (ISP_SPICMD, 0xAD);
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, *SrcAddress ++);
    EC_WaitSPI_Cycle();
    EC_ISP_Write (ISP_SPICMD, *SrcAddress ++);
    EC_WaitSPI_Cycle();
    EC_ISP_Stop_SPI ();
    StartAddress += 2;
//[-start-070922-IB03030019-modify]//
  EcSpiStall (0x0C);
//    AcpiStall (12);
//[-end-070922-IB03030019-modify]//

  }

  ISP_Action_SPI (0x04);                          // Write Disable
//  EcStall (2000);

  if (LoopCount + 1 == BufferSize) {
    EcProgramSPI_A_BYTE(StartAddress, *SrcAddress ++);
  }
  EC_WaitSPI_Cycle ();

  EC_ISP_Action_SPI_RB (0x05);
  return EFI_SUCCESS;
}
//[End-070301-IB03030001-Add]//

EFI_STATUS
//[-start-110301-IB05910092-modify]//
ReadECSPI (
//[-end-110301-IB05910092-modify]//
  IN  UINT8                     *DstAddress,
  IN  UINT8                     *FlashAddress,
  IN  UINTN                     BufferSize
  )
/*++

Routine Description:

  Read the SPI flash device with given address and size through SouthBridge

Arguments:

  DstAddress                    Destination Offset
  SrcAddress                    Source Offset
  BufferSize                    The size for programming

Returns:

  EFI status

--*/
{
  UINTN       StartAddress;
  UINT8       A2, A1, A0;

  StartAddress = (UINTN)FlashAddress;

  A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
  A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
  A0 = (UINT8) ((StartAddress & 0x00FF));

  WaitECSPI_WIP();                //[IB01160069 ++]

  EC_ISP_Start_SPI ();

  EC_ISP_Write (ISP_SPICMD, mSpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_READ]);             // SPI read ID command
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, A2);		        // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, A1);		        // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  EC_ISP_Write (ISP_SPICMD, A0);		        // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//

  while (BufferSize) {
    EC_ISP_Write (ISP_SPICMD, 0x00);		  // SPI dummy byte
  //[-start-070301-IB03030001-modify]//
  EC_WaitSPI_Cycle ();
  //while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);   //Wait SPI cycle finish
  //[-end-070301-IB03030001-modify]//
    *DstAddress = EC_ISP_Read (ISP_SPIDAT);

    DstAddress ++;
    BufferSize --;
    }

  EC_ISP_Stop_SPI ();

  return EFI_SUCCESS;
}
//[-end-110302-IB05910092-modify]//


//[-start-070930-IB03030038-add]//
VOID
FdSupportZeroMem (
  IN VOID   *Buffer,
  IN UINTN  Size
  )
/*++

Routine Description:

  Set Buffer to 0 for Size bytes.

Arguments:

  Buffer  - Memory to set.

  Size    - Number of bytes to set

Returns:

  None

--*/
{
  INT8  *Ptr;

  Ptr = Buffer;
  while (Size--) {
    *(Ptr++) = 0;
  }
}

extern EFI_BOOT_SERVICES  *gBS;

//[-end-070930-IB03030038-add]//

//[-start-070922-IB03030019-add]//

METHOD_SPIINIT                 *mSpiInit = NULL;

EFI_STATUS
EcInitSPI (
  EFI_SMM_SYSTEM_TABLE                    *mSmst
  )
{
  EFI_STATUS                            Status;

  if ((mSpiInit != NULL) && mSpiInit->Inited) {
    return EFI_SUCCESS;
  }
  if (mSmst == NULL) {

//[-start-070930-IB03030038-add]//
    ASSERT (gBS);
//[-end-070930-IB03030038-add]//

    Status = gBS->AllocatePool (EfiRuntimeServicesData, sizeof (METHOD_SPIINIT), &mSpiInit);
  } else {
   Status = mSmst->SmmAllocatePool (EfiRuntimeServicesData, sizeof (METHOD_SPIINIT), &mSpiInit);
  }

  FdSupportZeroMem (mSpiInit, sizeof (METHOD_SPIINIT));

  mSpiInit->Inited = TRUE;
  mSpiInit->RcrbRebaseDelta        = 0;
  mSpiInit->FlashAddressDelta      = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
EcConvertPointerSPI (
  VOID
  )
{
//[-start-070922-IB03030019-remark]//
//  UINTN         RuntimeRcrbAddress;
//  UINTN         FlashAddress;
//
//  if (mSpiInit->ConvertPointer) {
//    return EFI_SUCCESS;
//  }
//
//  RuntimeRcrbAddress = B_ICH_LPC_RCBA_BAR;
//  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(RuntimeRcrbAddress));
//  mSpiInit->RcrbRebaseDelta = RuntimeRcrbAddress - B_ICH_LPC_RCBA_BAR;
//  FlashAddress = (UINTN)(0x100000000 - (UINT64)mSpiConfigBlock->DeviceSize);
//  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(FlashAddress));
//  mSpiInit->FlashAddressBase = FlashAddress;
//  mSpiInit->FlashAddressDelta = FlashAddress - (UINTN)(0x100000000 - (UINT64)mSpiConfigBlock->DeviceSize);
//
//  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSpiInit->RcrbRebaseDelta));
//  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID **) &mSpiInit);
//  mSpiInit->ConvertPointer = TRUE;
//
//[-end-070922-IB03030019-remark]//

  return EFI_SUCCESS;
}

EFI_STATUS
//[-start-100520-IB05660016-modify]//
EcGetSpiPlatformFlashTable (
//[-end-100520-IB05660016-modify]//
  IN OUT   UINT8    *DataBuffer
  )
/*++
Routine Description:

  Get flash table from platform.

Arguments:

  DataBuffer      -IN: the input buffer address
                   OUT:the flash region table from rom file
Returns:

  EFI_SUCCESS           - Successfully returns

--*/
{

  return EFI_SUCCESS;
}

//[-start-100514-IB05660016-remove]//
//BOOLEAN
//CheckFlashRegionIsValid (
//  IN       UINT32    FlashRegion
//  )
///*++
//Routine Description:
//
//  Check the flash region whether is used
//
//Arguments:
//
//  FlashRegion     - Flash Region x Register (x = 0 - 3)
//
//Returns:
//
//  TRUE            - This  region is used
//  FALSE           - This  region is not used
//
//--*/
//{
//  BOOLEAN         Flag = TRUE;
//
////  //
////  // the Region Base must be programmed to 0x1FFFh and the Region Limit
////  // to 0x0000h to disable the region.
////  //
////  // Now, the tool programmed 0x0fff to base and 0x000 to limit to indicate
////  // this is region is not used.
////  //
////  if ((FlashRegion & 0x0fff) == 0x0fff && (FlashRegion >> 16 & 0x1fff) == 0) {
//
//  //
//  //The limit block address must be greater than or equal to the base block address
//  //
//  if ((FlashRegion & 0x1fff) > (FlashRegion >> 16 & 0x1fff)) {
//    Flag = FALSE;
//  }
//  return Flag;
//}
//[-end-100514-IB05660016-remove]//

VOID
EcSpiStall (
  IN  UINTN   Microseconds
)
{
  UINTN   Ticks;
  UINTN   Counts;
  UINT32  CurrentTick;
  UINT32  OriginalTick;
  UINTN  RemainingTick;

  if (Microseconds == 0) {
    return;
  }
  
    //
    // Don't use CpuIO PPI for IO port access here, it will result 915
    // platform recovery fail when using the floppy,because the CpuIO PPI is
    // located at the flash.Use the ASM file to replace it.
    //
    OriginalTick = IoRead32  (ICH_ACPI_TIMER_ADDR) & 0x00FFFFFF;

    CurrentTick   = OriginalTick;

    //
    // The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
    //
    Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
    //
    // The loops needed by timer overflow
    //
  Counts = Ticks / ICH_ACPI_TIMER_MAX_VALUE;
    //
    // remaining clocks within one loop
    //
  RemainingTick = Ticks % ICH_ACPI_TIMER_MAX_VALUE;
    //
    // not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
    // one I/O operation, and maybe generate SMI
    //
    while (Counts != 0) {
        CurrentTick = IoRead32 (ICH_ACPI_TIMER_ADDR) & 0x00FFFFFF;

        if (CurrentTick <= OriginalTick) {
            Counts--;
        }

        OriginalTick = CurrentTick;
    }

    while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
        OriginalTick  = CurrentTick;
        CurrentTick = IoRead32 (ICH_ACPI_TIMER_ADDR) & 0x00FFFFFF;
    }

    return;
}

//[-end-070922-IB03030019-add]//

//[PRJ][PRJ]Start - EC 901x flash function
//[-start-110523-IB08430000-add]//

EFI_STATUS 
EcReadSPI ( 
  IN     UINT8                   *DstAddress, 
  IN     UINT8                   *FlashAddress, 
  IN OUT UINTN                   BufferSize 
)
{
  EFI_STATUS                    Status;
//  FLASH_DEVICE                  *FlashDevicePtr;

//  Status = OemAutoDetectDevice (&FlashDevicePtr, NULL, PROTECT);
//#ifdef COMPAL_COMMON_CODE_SUPPORT
  CompalECWait (TRUE);
  Status = CompalEcReadSPIData (
             FALSE,
             DstAddress,
             FlashAddress,
             BufferSize
             );
  CompalECWait (FALSE);
//#else  
//  Status = ReadECSPI (DstAddress, FlashAddress, BufferSize);
//#endif
  
  return Status;
}

VOID
WriteEcRom (
  IN UINTN  Adr,
  IN UINT8  *Data,
  IN UINTN  Len
)
/*++

Routine Description:
  Write data to EC ROM

Arguments:
  Adr         EC ROM Start address
  *Data       Data pointer
  Len         Data length 

Returns:
  
--*/
{
  UINTN i;

  PageErase(Adr);

  EC_WaitSPI_Cycle();
//[PRJ] Start - ENE9012 flash
  if (EC_ISP_Read (ISP_LPCCSR) & 0x10) { 
    EC_ISP_Write (ISP_ECSTS, (EC_ISP_Read(ISP_ECSTS) & 0xFC) | 0x04); 
    EC_ISP_Write (ISP_CLKCFG2, 0x0F);
  }
//[PRJ] End - ENE9012 flash
  
  EC_ISP_Write (ISP_SPICMD,EFCMD_ClrHVPL); 
  EC_WaitSPI_Cycle();

  EC_ISP_Write (ISP_SPIADD2, ((Adr>>16) & 0xff) );
  EC_ISP_Write (ISP_SPIADD1, ((Adr>>8) & 0xff) );

  for ( i = 0 ; i < Len ; i++) {
    
  EC_ISP_Write (ISP_SPIADD0, (Adr & 0xff) );
  EC_ISP_Write (ISP_SPIDAT, *(Data+i) );
  EC_ISP_Write (ISP_SPICMD, EFCMD_PageLatch); 
  Adr ++;
  EC_WaitSPI_Cycle();
  
  }

  EC_ISP_Write (ISP_SPICMD, EFCMD_ProgramSP);  

//[PRJ] Start - ENE9012 flash
  if (EC_ISP_Read(ISP_LPCCSR) & 0x10) { 
#ifdef OEM_ENE9012_FLASH_DEBUG
   EC_ISP_Write(0xFC22,EC_ISP_Read(0xFC22)&(~0x40)) ;
#endif
   EC_WaitSPI_Cycle(); 
#ifdef OEM_ENE9012_FLASH_DEBUG
   EC_ISP_Write(0xFC22,EC_ISP_Read(0xFC22)|0x40);
#endif
   EC_ISP_Write (ISP_CLKCFG2, 0x1F); 
   EC_ISP_Write (ISP_ECSTS, EC_ISP_Read(ISP_ECSTS) & 0xF8); 
  }
//[PRJ] End - ENE9012 flash
  
  return;
}

EFI_STATUS
Start_SPI ()
{
   UINT8 SPICFG;

  EcSpiStall (0x08);
  SPICFG = EC_ISP_Read (ISP_SPICFG);
  SPICFG = 0x08;
  EC_WaitSPI_Cycle();
  EC_ISP_Write (ISP_SPICFG, SPICFG); 
  
  EcSpiStall (0x04);
  return EFI_SUCCESS;

}

EFI_STATUS
Stop_SPI ()
{
   UINT8 SPICFG;

  EcSpiStall (0x08);
  SPICFG = 0;
  EC_WaitSPI_Cycle();
  EC_ISP_Write (ISP_SPICFG, SPICFG); 
  
  EcSpiStall (0x04);
  return EFI_SUCCESS;

}

VOID
  PageErase (
  IN UINTN Adr
)
{
  EC_WaitSPI_Cycle();

//[PRJ] Start - ENE9012 flash
  if (EC_ISP_Read (ISP_LPCCSR) & 0x10) {
  	EC_ISP_Write(ISP_ECSTS, (EC_ISP_Read(ISP_ECSTS) & 0xFC) | 0x04);
	EC_ISP_Write(ISP_CLKCFG2, 0x30);
  }
//[PRJ] End - ENE9012 flash

  EC_ISP_Write (ISP_SPIADD2, ((Adr>>16) & 0xff) );
  EC_ISP_Write (ISP_SPIADD1, ((Adr>>8) & 0xff) );    
  EC_ISP_Write (ISP_SPIADD0, (Adr & 0xff) );

  EC_ISP_Write (ISP_SPICMD, EFCMD_PageErase);

//[PRJ] Start - ENE9012 flash
  if (EC_ISP_Read (ISP_LPCCSR) & 0x10) {
#ifdef OEM_ENE9012_FLASH_DEBUG
  	EC_ISP_Write(0xFC22,EC_ISP_Read(0xFC22)&(~0x40)) ;  
#endif
  	EC_WaitSPI_Cycle();
#ifdef OEM_ENE9012_FLASH_DEBUG
	EC_ISP_Write(0xFC22,EC_ISP_Read(0xFC22)|0x40);
#endif
	EC_ISP_Write (ISP_CLKCFG2, 0x1F);
	EC_ISP_Write (ISP_ECSTS, EC_ISP_Read(ISP_ECSTS) & 0xF8);
  }
//[PRJ] End - ENE9012 flash


  return;
}
VOID
ReadENEData_9012 (
  IN  UINT8  LowAddress,
  IN  UINT8  HighAddress,
  OUT UINT8  *OutData
)
/*!

Routine Description: \n

  - Read Data from EC controller SPI Flash

Arguments:

  - LowAddress - EC controller Low byte address
  - HighAddress - EC controller High byte address 
  - OutData - Register value

Returns:
	- N/A
*/
{
    IoWrite8 ((ENEIOBASE + 1), HighAddress);
    IoWrite8 ((ENEIOBASE + 2), LowAddress);
    *OutData = IoRead8(ENEIOBASE +3);
}

//[-end-110523-IB08430000-add]//
//[PRJ]End
