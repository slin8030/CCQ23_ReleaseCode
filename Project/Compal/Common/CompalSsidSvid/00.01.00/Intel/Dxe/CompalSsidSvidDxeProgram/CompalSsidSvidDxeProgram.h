
#ifndef _COMPAL_SSID_SVID_DXE_PROGRAM_H_
#define _COMPAL_SSID_SVID_DXE_PROGRAM_H_

#include <IndustryStandard/Pci22.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <PchRegs/PchRegsUsb.h>
#include <Library/MemoryAllocationLib.h>
#include <CompalSsidSvidDxeProtocol.h>
#include <Library/S3BootScriptLib.h>


#define EFI_PCI_CAPABILITY_PTR                0x34
#define EFI_PCI_ADDRESS(bus, dev, func, reg) \
    ((UINT64) ((((UINTN) bus) << 24) + (((UINTN) dev) << 16) + (((UINTN) func) << 8) + ((UINTN) reg)))

EFI_STATUS
ProgramSpecificSsidSvid (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT16                           VendorId,
    IN  UINT16                           DeviceId,
    IN  UINT32                           SsidSvid,
    IN  COMPAL_SSID_SVID_SPECIAL_TABLE   *CompalSsidSvidSpecialTable
);

VOID
ProgramSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT16       VendorId,
    IN  UINT16       DeviceId,
    IN  UINT32       SsidSvid
);

EFI_STATUS
ProgramEhciSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
);

EFI_STATUS
ProgramXhciSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
);

EFI_STATUS
PciFindCapId (
    IN  UINT8        Bus,
    IN  UINT8        Device,
    IN  UINT8        Function,
    IN  UINT8        CapId,
    OUT UINT8        *CapHeader
);

EFI_STATUS
CompalSsidSvidDxeWrite (
    IN  UINT32                           SsidSvid,
    IN  COMPAL_SSID_SVID_SPECIAL_TABLE   *CompalSsidSvidSpecialTable
);

EFI_STATUS
CompalSsidSvidDxePciWrite (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT8                            Offset,
    IN  EFI_BOOT_SCRIPT_WIDTH            DataWidth,
    IN  VOID                             *DataPtr
);

EFI_STATUS
CompalSsidSvidDxePciRead (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT8                            Offset,
    IN  EFI_BOOT_SCRIPT_WIDTH            DataWidth,
    IN  VOID                             *DataPtr
);

extern EFI_BOOT_SERVICES  *gBS;

#endif
