
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

    CompalAcpiStall(0x08);

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
    SPICFG &= ~0x08;
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

    if (IdleMode ) CompalECWait (TRUE);

    StartAddress = (UINTN)FlashAddress;

    CompalISP_Start_SPI ();

    while (BufferSize) {
        A2 = (UINT8) ((StartAddress & 0xFF0000) >> 16);
        A1 = (UINT8) ((StartAddress & 0xFF00) >> 8);
        A0 = (UINT8) ((StartAddress & 0x00FF));

        CompalISP_Write (ISP_SPIADD2, A2);                // SPI address byte
        CompalWaitSPI_Cycle ();

        CompalISP_Write (ISP_SPIADD1, A1);                // SPI address byte
        CompalWaitSPI_Cycle ();

        CompalISP_Write (ISP_SPIADD0, A0);                // SPI address byte
        CompalWaitSPI_Cycle ();

        CompalISP_Write (ISP_SPICMD, 0x03);               // Read Command
        CompalWaitSPI_Cycle ();

        *DstAddress = CompalISP_Read (ISP_SPIDAT);

        StartAddress ++;
        DstAddress ++;
        BufferSize --;
    }

    CompalISP_Stop_SPI ();
    if (IdleMode ) CompalECWait (FALSE);

    return EFI_SUCCESS;
}
