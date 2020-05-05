/** @file
  IoDecodeLib Sample Code, need to be modified for different chipset support
  
;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/IoLib.h>
#include "FchDecodeDef.h"

UINT8
GetFreeWideIo (
  IN UINT16                    Address
  )
{
  UINT8     WideIoReg;
  UINT32    PciRegAddr;
  UINT32    PciRegValue;
  UINT32    DecodeStartAddr;
  UINT32    DecodeRangeValue;
  UINT32    DecodeEnableValue;

  WideIoReg = 0xFF;
  DecodeStartAddr = (UINT32)(Address & 0xFE00);

  PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_FCH_IO_DECODE2);
  IoWrite32 (0xCF8, PciRegAddr);
  DecodeEnableValue = IoRead32 (0xCFC);
  PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_RANGE);
  IoWrite32 (0xCF8, PciRegAddr);
  DecodeRangeValue = IoRead32 (0xCFC);

  if ((DecodeEnableValue & B_WIDE_IO_0_EN) == 0) {
//    WideIoReg = 0;
//    return WideIoReg;
  } else if ((DecodeEnableValue & B_WIDE_IO_0_EN) != 0) {
    if ((DecodeRangeValue & B_WIDE_IO_0_16BYTE) == 0) {
      PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_0_PORT);
      IoWrite32 (0xCF8, PciRegAddr);
      PciRegValue = IoRead32 (0xCFC);
      PciRegValue &= 0x0000FFFF;
      if (PciRegValue == DecodeStartAddr) {
        return 0xAA;
      }
    }
  }

  if ((DecodeEnableValue & B_WIDE_IO_1_EN) == 0) {
    if (WideIoReg == 0xFF) {
      WideIoReg = 1;
    }
  } else if ((DecodeEnableValue & B_WIDE_IO_1_EN) != 0) {
    if ((DecodeRangeValue & B_WIDE_IO_1_16BYTE) == 0) {
      PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_0_PORT);
      IoWrite32 (0xCF8, PciRegAddr);
      PciRegValue = IoRead32 (0xCFC);
      PciRegValue &= (PciRegValue >> 16);
      if (PciRegValue == DecodeStartAddr) {
        return 0xAA;
      }
    }
  }

  if ((DecodeEnableValue & B_WIDE_IO_2_EN) == 0) {
    if (WideIoReg == 0xFF) {
      WideIoReg = 2;
    }
  } else if ((DecodeEnableValue & B_WIDE_IO_2_EN) != 0) {
    if ((DecodeRangeValue & B_WIDE_IO_2_16BYTE) == 0) {
      PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_2_PORT);
      IoWrite32 (0xCF8, PciRegAddr);
      PciRegValue = IoRead32 (0xCFC);
      PciRegValue &= 0x0000FFFF;
      if (PciRegValue == DecodeStartAddr) {
        return 0xAA;
      }
    }
  }

  return WideIoReg;
}

EFI_STATUS
AllocateWideIo (
  IN UINT16                    Address,
  IN UINT16                    Length
  )
{
  UINT8       WideIoReg;
  BOOLEAN     Is16ByteDecode;
  UINT32      PciRegAddr;
  UINT32      PciRegValue;
  UINT16      DecodeStartAddr;
  UINT16      DecodeLength;

  WideIoReg = GetFreeWideIo(Address);
  if (WideIoReg == 0xFF) {
    return EFI_OUT_OF_RESOURCES;
  } else if (WideIoReg == 0xAA) {
    return EFI_SUCCESS;
  }
  DecodeStartAddr = Address;
  DecodeLength = Length;

//  if ((DecodeStartAddr & 0xF) != 0) {
//    DecodeStartAddr &= 0xFFF0;
//    if ((Address + Length) > (0x10 + (Address & 0xFFF0)) ) {
//      DecodeLength += 0x10;
//    }
//  }
//  Is16ByteDecode = TRUE;
//  if (DecodeLength > 16) {
    Is16ByteDecode = FALSE;
    if ((DecodeStartAddr & 0x100) != 0) {
      DecodeStartAddr &= 0xFE00;
    }
//  }

  PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_RANGE);
  IoWrite32 (0xCF8, PciRegAddr);
  PciRegValue = IoRead32 (0xCFC);
  switch (WideIoReg) {
  case 0:
    if (Is16ByteDecode) {
      PciRegValue |= B_WIDE_IO_0_16BYTE;
      IoWrite32 (0xCFC, PciRegValue);
    } else {
      PciRegValue &= (~B_WIDE_IO_0_16BYTE);
      IoWrite32 (0xCFC, PciRegValue);
    }
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_0_PORT);
    IoWrite32 (0xCF8, PciRegAddr);
    PciRegValue = IoRead32 (0xCFC);
    PciRegValue &= 0xFFFF0000;
    PciRegValue += DecodeStartAddr;
    IoWrite32 (0xCFC, PciRegValue);
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_FCH_IO_DECODE2);
    IoWrite32 (0xCF8, PciRegAddr);
    PciRegValue = IoRead32 (0xCFC);
    PciRegValue |= B_WIDE_IO_0_EN;
    IoWrite32 (0xCFC, PciRegValue);
    break;

  case 1:
    if (Is16ByteDecode) {
      PciRegValue |= B_WIDE_IO_1_16BYTE;
      IoWrite32 (0xCFC, PciRegValue);
    } else {
      PciRegValue &= (~B_WIDE_IO_1_16BYTE);
      IoWrite32 (0xCFC, PciRegValue);
    }
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_0_PORT);
    IoWrite32 (0xCF8, PciRegAddr);
    PciRegValue = IoRead32 (0xCFC);
    PciRegValue &= 0x0000FFFF;
    PciRegValue += ((UINT32)DecodeStartAddr << 16);
    IoWrite32 (0xCFC, PciRegValue);
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_FCH_IO_DECODE2);
    IoWrite32 (0xCF8, PciRegAddr);
    PciRegValue = IoRead32 (0xCFC);
    PciRegValue |= B_WIDE_IO_1_EN;
    IoWrite32 (0xCFC, PciRegValue);
    break;

  case 2:
    if (Is16ByteDecode) {
      PciRegValue |= B_WIDE_IO_2_16BYTE;
      IoWrite32 (0xCFC, PciRegValue);
    } else {
      PciRegValue &= (~B_WIDE_IO_2_16BYTE);
      IoWrite32 (0xCFC, PciRegValue);
    }
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_WIDE_IO_2_PORT);
    IoWrite32 (0xCF8, PciRegAddr);
    PciRegValue = IoRead32 (0xCFC);
    PciRegValue &= 0xFFFF0000;
    PciRegValue += DecodeStartAddr;
    IoWrite32 (0xCFC, PciRegValue);
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_FCH_IO_DECODE2);
    IoWrite32 (0xCF8, PciRegAddr);
    PciRegValue = IoRead32 (0xCFC);
    PciRegValue |= B_WIDE_IO_2_EN;
    IoWrite32 (0xCFC, PciRegValue);
    break;

  default:
    return EFI_OUT_OF_RESOURCES;
    break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
IoDecodeControl (
  IN UINT16                    Address,
  IN UINT16                    Length
  ) 
{
  BOOLEAN     UseWideIo;
  UINT32      FixedDecodeBit;
  UINT8       DecodeRegGroup;
  UINT32      PciRegAddr;
  UINT32      PciRegValue;
  EFI_STATUS  Status;

  UseWideIo = FALSE;

  DecodeRegGroup = 0xFF;
  FixedDecodeBit = 0;
  switch (Address) {
  case 0x378:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_378_37F;
    }
    break;

  case 0x778:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_778_77F;
    }
    break;

  case 0x278:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_278_27F;
    }
    break;

  case 0x678:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_678_67F;
    }
    break;

  case 0x3BC:
    DecodeRegGroup = 0;
    if (Length > 4) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_3BC_3BF;
    }
    break;

  case 0x7BC:
    DecodeRegGroup = 0;
    if (Length > 4) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_7BC_7BF;
    }
    break;

  case 0x3F8:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_3F8_3FF;
    }
    break;

  case 0x2F8:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_2F8_2FF;
    }
    break;

  case 0x220:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_220_227;
    }
    break;

  case 0x228:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_228_22F;
    }
    break;

  case 0x238:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_238_23F;
    }
    break;

  case 0x2E8:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_2E8_2EF;
    }
    break;

  case 0x338:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_338_33F;
    }
    break;

  case 0x3E8:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_3E8_3EF;
    }
    break;

  case 0x230:
    DecodeRegGroup = 0;
    if (Length > 4) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_230_233;
    }
    break;

  case 0x240:
    DecodeRegGroup = 0;
    if (Length > 20) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_240_253;
    }
    break;

  case 0x260:
    DecodeRegGroup = 0;
    if (Length > 20) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_260_273;
    }
    break;

  case 0x280:
    DecodeRegGroup = 0;
    if (Length > 20) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_280_293;
    }
    break;

  case 0x300:
    DecodeRegGroup = 0;
    if (Length > 2) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_300_301;
    }
    break;

  case 0x310:
    DecodeRegGroup = 0;
    if (Length > 2) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_310_311;
    }
    break;

  case 0x320:
    DecodeRegGroup = 0;
    if (Length > 2) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_320_321;
    }
    break;

  case 0x330:
    DecodeRegGroup = 0;
    if (Length > 2) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_330_331;
    }
    break;

  case 0x530:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_530_537;
    }
    break;

  case 0x604:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_604_60B;
    }
    break;

  case 0xE80:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_E80_E87;
    }
    break;

  case 0xF40:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_F40_F47;
    }
    break;

  case 0x3F0:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_3F0_3F7;
    }
    break;

  case 0x370:
    DecodeRegGroup = 0;
    if (Length > 8) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_370_377;
    }
    break;

  case 0x200:
    DecodeRegGroup = 0;
    if (Length > 16) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_200_20F;
    }
    break;

  case 0x60:
    DecodeRegGroup = 0;
    FixedDecodeBit = B_IO_60_64;
    break;

  case 0x62:
    DecodeRegGroup = 0;
    FixedDecodeBit = B_IO_62_66;
    break;

  case 0x388:
    DecodeRegGroup = 0;
    if (Length > 2) {
      UseWideIo = TRUE;
    } else {
      FixedDecodeBit = B_IO_388_389;
    }
    break;

  case 0x2E:
    DecodeRegGroup = 1;
    FixedDecodeBit = B_IO_2E_2F;
    break;

  case 0x4E:
    DecodeRegGroup = 1;
    FixedDecodeBit = B_IO_4E_4F;
    break;

  default:
    UseWideIo = TRUE;
    FixedDecodeBit = 0;
    break;
  }

  if (UseWideIo) {
    Status  = AllocateWideIo (Address, Length);
    return Status;
  }

  switch (DecodeRegGroup) {
  case 0:
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_FCH_IO_DECODE1);
    break;

  case 1:
    PciRegAddr = (UINT32)FCH_PCI_LPC_REG_ADDR(R_FCH_IO_DECODE2);
    break;

  default:
    return EFI_OUT_OF_RESOURCES;
    break;
  }

  IoWrite32 (0xCF8, PciRegAddr);
  PciRegValue = IoRead32 (0xCFC);
  PciRegValue |= FixedDecodeBit;
  IoWrite32 (0xCFC, PciRegValue);

  return EFI_SUCCESS;
}
