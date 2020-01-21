
#ifndef _COMPAL_SSID_SVID_DXE_SETTING_H_
#define _COMPAL_SSID_SVID_DXE_SETTING_H_

#include <IndustryStandard/Pci22.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
//#include <PchRegs/PchRegsUsb.h>
#include <Library/MemoryAllocationLib.h>
#include <CompalSsidSvidDxeProtocol.h>
#include <Library/PciExpressLib.h>
#include <Library/S3BootScriptLib.h>
//#include <PchRegs/PchRegsLpc.h>

//
// The LPC related register used to locate GPIO base
//
#define  LPC_BUS                        0x00
#define  LPC_DEV                        0x1f
#define  LPC_FUN                        0x02
#define    R_ACPI_GPIO_BASE             0x48
#define    ACPI_GPIO_BASE_MASK          0xFFF8
#define  LPC_PCI_GPIOBASE               PCI_LIB_ADDRESS (LPC_BUS, LPC_DEV, LPC_FUN, R_ACPI_GPIO_BASE)


EFI_STATUS
ProgramNvidiaSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
);

EFI_STATUS
ProgramAmdSsidSvid (
  IN     UINT8    Bus,
  IN     UINT8    Dev,
  IN     UINT8    Func,
  IN     UINT32   SsidSvid
  );

extern EFI_BOOT_SERVICES  *gBS;

#endif
