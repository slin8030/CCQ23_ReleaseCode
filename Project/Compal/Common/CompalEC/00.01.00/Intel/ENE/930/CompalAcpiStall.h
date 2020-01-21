
#include <Library/IoLib.h>

#define R_PCH_ACPI_PM1_TMR              0x08
#define V_PCH_ACPI_TMR_FREQUENCY        3579545
#define B_PCH_ACPI_PM1_TMR_VAL          0xFFFFFF
#define V_PCH_ACPI_PM1_TMR_MAX_VAL      0x1000000       ///< The timer is 24 bit overflow

#define DEFAULT_PCI_BUS_NUMBER_PCH      0
#define PCI_DEVICE_NUMBER_PCH_LPC       31
#define PCI_FUNCTION_NUMBER_PCH_LPC     0
#define R_PCH_LPC_ACPI_BASE             0x40

#ifndef _COMPAL_ACPI_STALL_H_
#define _COMPAL_ACPI_STALL_H_

UINT16
CompalGetAcpiTimerBaseAddress (
    VOID
);

VOID
CompalAcpiStall (
    IN  UINT32   Microseconds
);

#endif
