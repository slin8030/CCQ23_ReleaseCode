/** @file
  Clovertrail I2C test application

  This application tests "UEFI Entropy-Gathering Protocol" 

Copyright (c) 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/          
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>



#define OTG_ACCESS_DELAY 1

//TUSB1211 register
#define VENDORID_L  0x0
#define VENDORID_H  0x1
#define VENDOR_SPECIFIC6  0x8e


//OTG core register
#define R_PCH_OTG_GUSB2PHYACC  0xc280
#define B_PCH_OTG_DISABLE_ULPI_DRIVER  BIT26
#define B_PCH_OTG_NEW_REQUEST          BIT25
#define B_PCH_OTG_VSTATUS_DONE         BIT24
#define B_PCH_OTG_VSTATUS_BUSY         BIT23
#define B_PCH_OTG_REG_WR               BIT22
#define B_PCH_OTG_REG_ADDR             (BIT21|BIT20|BIT19|BIT18|BIT17|BIT16)
#define B_PCH_OTG_REG_EXT_ADDR         (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10|BIT9|BIT8)
#define B_PCH_OTG_REG_DATA             (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define DEFAULT_PCI_BUS_NUMBER_SC  0
#define PCI_DEVICE_NUMBER_PCH_OTG           22
#define PCI_FUNCTION_NUMBER_PCH_OTG         0

#define R_PCH_OTG_BAR0                      0x10  // BAR 0
#define B_PCH_OTG_BAR0_BA                   0xFFE00000 // Base Address

#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )


EFI_STATUS
EFIAPI
UlpiWrite8 (
  IN      UINT8                     Register,
  IN      UINT8                     Value
  )
{
  UINTN          OtgPciMmBase, OtgMmioBase0;
  UINT32         val=0;
  UINT32         cnt = 0;

  OtgPciMmBase = MmPciAddress (0,
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_PCH_OTG,
               PCI_FUNCTION_NUMBER_PCH_OTG,
               0x0
             );
  OtgMmioBase0 = MmioRead32((UINTN)(OtgPciMmBase + R_PCH_OTG_BAR0)) & B_PCH_OTG_BAR0_BA;

  //enable extend access (immediate access in mapped in extended access)
  val  = ((0x2f << 16) & B_PCH_OTG_REG_ADDR);
  val |= ((Register << 8) & B_PCH_OTG_REG_EXT_ADDR);
  val |= B_PCH_OTG_REG_WR;
  val |= B_PCH_OTG_NEW_REQUEST;
  val |= (Value & B_PCH_OTG_REG_DATA);
  MmioWrite32 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_GUSB2PHYACC), val);
  //Print(L"OtgPciMmBase:0x%x OtgMmioBase0:0x%x\n", OtgPciMmBase, OtgMmioBase0);
  //Print(L"UlpiWrite8 R_PCH_OTG_GUSB2PHYACC:0x%x\n", val);
  val = MmioRead32 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_GUSB2PHYACC));
  while(B_PCH_OTG_VSTATUS_DONE != (B_PCH_OTG_VSTATUS_DONE & val)){
    val = MmioRead32 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_GUSB2PHYACC));
    MicroSecondDelay ( OTG_ACCESS_DELAY );
    cnt++;
    if( cnt > 10){
      break;
    }
  }

  return EFI_SUCCESS;
}

UINT8
EFIAPI
UlpiRead8 (
  IN      UINT8                     Register
  )
{
  UINTN          OtgPciMmBase, OtgMmioBase0;
  UINT32         val=0;
  UINT32         cnt = 0;

  OtgPciMmBase = MmPciAddress (0,
               DEFAULT_PCI_BUS_NUMBER_SC,
               PCI_DEVICE_NUMBER_PCH_OTG,
               PCI_FUNCTION_NUMBER_PCH_OTG,
               0x0
             );
  OtgMmioBase0 = MmioRead32((UINTN)(OtgPciMmBase + R_PCH_OTG_BAR0)) & B_PCH_OTG_BAR0_BA;

  //enable extend access (immediate access in mapped in extended access)
  val  = ((0x2f << 16) & B_PCH_OTG_REG_ADDR);
  val |= ((Register << 8) & B_PCH_OTG_REG_EXT_ADDR);
  val &= ~(B_PCH_OTG_REG_WR);
  val |= B_PCH_OTG_NEW_REQUEST;
  MmioWrite32 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_GUSB2PHYACC), val);
  //Print(L"OtgPciMmBase:0x%x OtgMmioBase0:0x%x\n", OtgPciMmBase, OtgMmioBase0);
  //Print(L"UlpiRead8 R_PCH_OTG_GUSB2PHYACC:0x%x\n", val);
  val = MmioRead32 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_GUSB2PHYACC));
  while(B_PCH_OTG_VSTATUS_DONE != (B_PCH_OTG_VSTATUS_DONE & val)){
    val = MmioRead32 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_GUSB2PHYACC));
    MicroSecondDelay ( OTG_ACCESS_DELAY );
    cnt++;
    if( cnt > 10){
      break;
    }
  }
   
  return (val & B_PCH_OTG_REG_DATA);

}


//usage
void usage(void)
{
  Print(L"Usage: \n");
  Print(L"   UlpiTest.efi read <offset> \n");
  Print(L"   UlpiTest.efi write <offset> <value>\n");
  Print(L"   UlpiTest.efi dump \n");
  return;
}

//register dump
void dump(void)
{
  UINT32 reg= 0;

  for (; reg <= 0xFF; reg++)    {
     Print (L"reg:0x%x:0x%x\n", reg, UlpiRead8((UINT8)reg));
  }
  return ;
}

/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  //EFI_STATUS			            Status;
  UINTN  offset;
  UINTN  val=0;
  CHAR8 AsciiStr[20]={0};
  
  if (Argc < 2) {
    usage();
    return EFI_INVALID_PARAMETER;
  }
 
  UnicodeStrToAsciiStr(Argv[1], AsciiStr);

  if(0 == AsciiStrCmp(AsciiStr, "read")) {
    if (Argc != 3) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset    = StrHexToUintn (Argv[2]);
    val = UlpiRead8((UINT8)offset) ;
    Print(L"ULPI read offset 0x%x value:0x%x\n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "write"))   {
    if (Argc != 4) {
       usage();
       return EFI_INVALID_PARAMETER;
    }
    offset  = StrHexToUintn (Argv[2]);
    val     = StrHexToUintn (Argv[3]);
    UlpiWrite8 ((UINT8)offset, (UINT8)val) ;
    Print(L"ULPI write offset 0x%x value:0x%x \n", offset, val);
  }else if(0 == AsciiStrCmp(AsciiStr, "dump"))   {
    dump();
  }
  return EFI_SUCCESS;
}
