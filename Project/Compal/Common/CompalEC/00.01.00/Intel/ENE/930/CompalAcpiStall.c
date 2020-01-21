
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
