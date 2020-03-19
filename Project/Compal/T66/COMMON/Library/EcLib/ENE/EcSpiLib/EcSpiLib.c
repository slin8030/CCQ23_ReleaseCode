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
#ifdef COMPAL_COMMON_CODE_SUPPORT
#ifdef COMPAL_EC_SUPPORT
#include <CompalEclib.h>
#endif
#endif
#include <Library/IoLib.h>
UINT8
EC_ISP_Read (
  IN UINT16    ENESPIReg
  )
{
#if defined (COMPAL_COMMON_CODE_SUPPORT) && defined (COMPAL_EC_SUPPORT)
  IoWrite8 ((ENEIOBASE + 1), (UINT8)(ENESPIReg >> 8));
  IoWrite8 ((ENEIOBASE + 2), (UINT8)ENESPIReg);

  return (IoRead8 (ENEIOBASE + 3));
#else
  return 0;
#endif

};

EFI_STATUS
EC_ISP_Write (
  IN UINT16    ENESPIReg,
  IN UINT8     ENESPICMD
  )
{
#if defined (COMPAL_COMMON_CODE_SUPPORT) && defined (COMPAL_EC_SUPPORT)
  IoWrite8 ((ENEIOBASE + 1), (UINT8)(ENESPIReg >> 8));
  IoWrite8 ((ENEIOBASE + 2), (UINT8)ENESPIReg);
  IoWrite8 ((ENEIOBASE + 3), ENESPICMD);
#endif

  return EFI_SUCCESS;
};

VOID
EC_WaitSPI_Cycle (
  )
{
  while ((EC_ISP_Read (ISP_SPICFG) & 0x02) != 0);
}

VOID
PageErase (
  IN UINTN Adr
)
{
  EC_WaitSPI_Cycle();

  if (EC_ISP_Read (ISP_LPCCSR) & 0x10) {
  	EC_ISP_Write(ISP_ECSTS, (EC_ISP_Read(ISP_ECSTS) & 0xFC) | 0x04);
	  EC_ISP_Write(ISP_CLKCFG2, 0x30);
  }

  EC_ISP_Write (ISP_SPIADD2, ((Adr>>16) & 0xff) );
  EC_ISP_Write (ISP_SPIADD1, ((Adr>>8) & 0xff) );    
  EC_ISP_Write (ISP_SPIADD0, (Adr & 0xff) );

  EC_ISP_Write (ISP_SPICMD, EFCMD_PageErase);

  if (EC_ISP_Read (ISP_LPCCSR) & 0x10) {
  	EC_WaitSPI_Cycle();
	  EC_ISP_Write (ISP_CLKCFG2, 0x1F);
	  EC_ISP_Write (ISP_ECSTS, EC_ISP_Read(ISP_ECSTS) & 0xF8);
  }
  return;
}

VOID
EcSpiStall (
  IN  UINTN   Microseconds
  )
/*++

Routine Description:

  Delay for at least the request number of microseconds.
  Timer used is DMA refresh timer, which has 15us granularity.
  You can call with any number of microseconds, but this
  implementation cannot support 1us granularity.

Arguments:

  Microseconds - Number of microseconds to delay.

Returns:

  None

--*/
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

  if (EC_ISP_Read (ISP_LPCCSR) & 0x10) { 
    EC_ISP_Write (ISP_ECSTS, (EC_ISP_Read(ISP_ECSTS) & 0xFC) | 0x04); 
    EC_ISP_Write (ISP_CLKCFG2, 0x0F);
  }

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

  if (EC_ISP_Read(ISP_LPCCSR) & 0x10) { 
    EC_WaitSPI_Cycle(); 
    EC_ISP_Write (ISP_CLKCFG2, 0x1F); 
    EC_ISP_Write (ISP_ECSTS, EC_ISP_Read(ISP_ECSTS) & 0xF8); 
  }

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
