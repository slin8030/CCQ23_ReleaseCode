
#include <CompalECLib.h>

#define ISP_SPIADD0     0xA8
#define ISP_SPIADD1     0xA9
#define ISP_SPIADD2     0xAA
#define ISP_SPIDAT      0xAB
#define ISP_SPICMD      0xAC
#define ISP_SPICFG      0xAD
#define ISP_ISPDATR     0xAE
#define EC_RAM_BASE_0xFE00 0xFE00

EFI_STATUS
CompalISP_Write (
    IN UINT8     ENESPIReg,
    IN UINT8     ENESPICMD
)
{
    CompalECIndexIOWrite(EC_RAM_BASE_0xFE00+ENESPIReg,ENESPICMD);
    return EFI_SUCCESS;
};

UINT8
CompalISP_Read (
    IN UINT8     ENESPIReg
)
{
    UINT8 ReturnData;

    ReturnData = CompalECIndexIORead(EC_RAM_BASE_0xFE00 + ENESPIReg);
    return ReturnData;
};


VOID
CompalWaitSPI_Cycle (
)
{
    while ((CompalISP_Read (ISP_SPICFG) & 0x02) != 0);
}

EFI_STATUS
CompalISP_Start_SPI ()
{
    UINT8     SPICFG;

    CompalAcpiStall(0x18);

    SPICFG = CompalISP_Read (ISP_SPICFG);
    SPICFG |= 0x08;
    CompalISP_Write (ISP_SPICFG, SPICFG);                //enable force SPICS# low
    CompalWaitSPI_Cycle();

    CompalAcpiStall(0x04);

    return EFI_SUCCESS;
}


EFI_STATUS
CompalISP_Stop_SPI ()
{
    UINT8     SPICFG;

    CompalAcpiStall(0x04);

    SPICFG = CompalISP_Read (ISP_SPICFG);
    SPICFG &= ~0x18;
    CompalISP_Write (ISP_SPICFG, SPICFG);                //enable force SPICS# low
    CompalWaitSPI_Cycle();

    CompalAcpiStall(0x08);

    return EFI_SUCCESS;
}

UINT8
CompalISP_Action_SPI_RB (
    IN UINT8     CMD
)
{
    UINT8     StatusReg;

    CompalISP_Start_SPI ();
    CompalISP_Write (ISP_SPICMD, CMD);                   //SPI read ID command
    CompalWaitSPI_Cycle ();
    CompalISP_Write (ISP_SPICMD, 0x00);                  //SPI dummy byte
    CompalWaitSPI_Cycle ();
    StatusReg = CompalISP_Read (ISP_SPIDAT);
    CompalISP_Stop_SPI ();

    return StatusReg;
}

VOID
CompalWaitSPI_WIP (
)
{
    while (CompalISP_Action_SPI_RB (0x05) & 0x01) {
        CompalAcpiStall(0x10);
    }
}

EFI_STATUS
CompalEcReadSPIData (
    IN  BOOLEAN    IdleMode,
    IN  UINT8      *DstAddress,
    IN  UINT8      *FlashAddress,
    IN  UINTN      BufferSize
)
{
    UINTN       StartAddress;
    UINT8       A2, A1, A0;

    if (IdleMode ) 
        CompalECWait (TRUE);

    StartAddress = (UINTN)FlashAddress;

    A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
    A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
    A0 = (UINT8) ((StartAddress & 0x00FF));

    CompalWaitSPI_WIP();


    CompalISP_Start_SPI ();


    CompalISP_Write (ISP_SPICMD, 0x0B);                       // fast read


    CompalWaitSPI_Cycle ();


    CompalISP_Write (ISP_SPICMD, A2);		        // SPI address byte


    CompalWaitSPI_Cycle ();


    CompalISP_Write (ISP_SPICMD, A1);		        // SPI address byte


    CompalWaitSPI_Cycle ();


    CompalISP_Write (ISP_SPICMD, A0);		        // SPI address byte


    CompalWaitSPI_Cycle (); 
    CompalISP_Write (ISP_SPICMD, 0x00);		        // SPI dummy byte
    CompalWaitSPI_Cycle ();
    CompalISP_Read (ISP_SPIDAT);                                  // Read dummy byte
 
    while (BufferSize) {
        CompalISP_Write (ISP_SPICMD, 0x00);                    // SPI dummy byte
 
        CompalWaitSPI_Cycle ();
 
        *DstAddress = CompalISP_Read (ISP_SPIDAT);
  

        DstAddress ++;
   
        BufferSize --;
    }
 
    CompalISP_Stop_SPI ();
    if (IdleMode ) 
        CompalECWait (FALSE);

    return EFI_SUCCESS;
}
