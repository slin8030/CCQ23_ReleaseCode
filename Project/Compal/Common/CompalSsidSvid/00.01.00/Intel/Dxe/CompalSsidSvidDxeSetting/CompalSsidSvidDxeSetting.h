
#ifndef _COMPAL_SSID_SVID_DXE_SETTING_H_
#define _COMPAL_SSID_SVID_DXE_SETTING_H_

#include <IndustryStandard/Pci22.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <PchRegs/PchRegsUsb.h>
#include <Library/MemoryAllocationLib.h>
#include <CompalSsidSvidDxeProtocol.h>
#include <PchRegs/PchRegsLpc.h>

//
// The LPC related register used to locate GPIO base
//
#define  LPC_BUS                        0x00
#define  LPC_DEV                        0x1f
#define  LPC_FUN                        0x00
#define    R_ACPI_GPIO_BASE             0x48
#define    ACPI_GPIO_BASE_MASK          0xFFF8
#define  LPC_PCI_GPIOBASE               PCI_LIB_ADDRESS (LPC_BUS, LPC_DEV, LPC_FUN, R_ACPI_GPIO_BASE)

// For VAL50 Default SSID/SVID
#define OEM_DEFAULT_SSID             0x0062
#define OEM_DEFAULT_SVID             0x14C0
#define OEM_DEFAULT_SSID_SVID        (UINT32)((UINT32)(OEM_DEFAULT_SSID << 16) | OEM_DEFAULT_SVID)

// [PRJ] Start - SSID/SVID for DIS only
#define OEM_DIS_VGA_SSID             0x0063
#define OEM_DIS_VGA_SVID             0x14C0
#define OEM_DIS_VGA_SSID_SVID        (UINT32)((UINT32)(OEM_DIS_VGA_SSID << 16) | OEM_DIS_VGA_SVID)
// [PRJ] End

EFI_STATUS
ProgramNvidiaSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
);

extern EFI_BOOT_SERVICES  *gBS;

#endif
