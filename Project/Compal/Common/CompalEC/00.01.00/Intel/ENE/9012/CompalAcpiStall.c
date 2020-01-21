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

#include <Library/PciCf8Lib.h>
#include <CompalAcpiStall.h>

UINT16
CompalGetAcpiTimerBaseAddress (
    VOID
)
{
    UINT16   AcpiTimerAddr;

    //
    // Must use the PciCf8
    //
    AcpiTimerAddr = PciCf8Read16 ( PCI_CF8_LIB_ADDRESS( DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_ACPI_BASE) );
    AcpiTimerAddr &= (~ (1 << 0));
    AcpiTimerAddr += R_PCH_ACPI_PM1_TMR;

    return AcpiTimerAddr;
}

VOID
CompalAcpiStall (
    IN  UINT32   Microseconds
)
{
    UINT32   Ticks;
    UINT32   Counts;
    UINT32   CurrentTick;
    UINT32   OriginalTick;
    UINT32   RemainingTick;
    UINT16   AcpiTimerAddr;

    if (Microseconds == 0) {
        return;
    }

    AcpiTimerAddr = CompalGetAcpiTimerBaseAddress();

    //
    // Don't use CpuIO PPI for IO port access here, it will result 915
    // platform recovery fail when using the floppy,because the CpuIO PPI is
    // located at the flash.Use the ASM file to replace it.
    //
    OriginalTick = IoRead32 (AcpiTimerAddr) & 0x00FFFFFF;

    CurrentTick   = OriginalTick;

    //
    // The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
    //
    Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
    //
    // The loops needed by timer overflow
    //
    Counts = Ticks / V_PCH_ACPI_PM1_TMR_MAX_VAL;
    //
    // remaining clocks within one loop
    //
    RemainingTick = Ticks % V_PCH_ACPI_PM1_TMR_MAX_VAL;
    //
    // not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
    // one I/O operation, and maybe generate SMI
    //
    while (Counts != 0) {
        CurrentTick = IoRead32 (AcpiTimerAddr) & 0x00FFFFFF;

        if (CurrentTick <= OriginalTick) {
            Counts--;
        }

        OriginalTick = CurrentTick;
    }

    while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
        OriginalTick  = CurrentTick;
        CurrentTick = IoRead32 (AcpiTimerAddr) & 0x00FFFFFF;
    }

    return;
}
