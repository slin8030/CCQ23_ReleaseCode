
#ifndef _COMPAL_ISCT_H_
#define _COMPAL_ISCT_H_


#include <CompalEcLib.h>
#include <EcNameSpaceDef.h>
#include <SetupConfig.h>
#include <CompalISCTPpi.h>
#include <Library/IoLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>

#define ACPI_IO_BASE_ADDRESS                     0x1800

#define R_PCH_ACPI_PM1_STS                       0x00
#define R_PCH_ACPI_PM1_EN                        0x02
#define R_PCH_ACPI_PM1_CNT                       0x04

#define B_PCH_ACPI_PM1_STS_RTC                   BIT10
#define B_PCH_ACPI_PM1_STS_PCIE                  BIT14

#define ISCT_WAKE_UP_BY_OTHER_EVENT              0x00
#define ISCT_WAKE_UP_BY_EC                       0x01
#define ISCT_WAKE_UP_BY_RTC                      0x02
#define ISCT_WAKE_UP_BY_PCIE                     0x03

EFI_STATUS
EFIAPI
CompalISCTWakeReasonGet (
    IN    COMPAL_ISCT_PPI    * This
);

EFI_STATUS
EFIAPI
CompalEcIFFSEntryNotify (
    IN    COMPAL_ISCT_PPI    * This,
    IN    BOOLEAN            IFFSEntry
);

#endif
