
#include "CompalSsidSvidDxeSetting.h"

COMPAL_SSID_SVID_DXE_PROTOCOL    *CompalSsidSvidDxeProtocol;

static COMPAL_SSID_SVID_SPECIAL_TABLE        CompalSsidSvidSpecialTable[] = {
//{     VendorId,      DeviceId,  Offset,         SsidSvid,           CompalSsidSvidSpecialTable },
//  {       0x8086,        0x0104,    0x2C,  DONT_WRITE_SSID,                               NULL }, /* Sandy Bridge */
//  {       0x8086,        0x1C2D,    0x2C,    PLATFORM_SSID,  ExampleSsidSpecialProgramFunction }, /* INTEL EHCI */
// [PRJ] Start - Modify Code for DIS/UMA/OPTIMUS mode
    {       0x10DE,        0x0DF5,    0x40,    PLATFORM_SSID,              ProgramNvidiaSsidSvid },
    {       0x10DE,        0x0BEA,    0x40,    PLATFORM_SSID,              ProgramNvidiaSsidSvid },
// [PRJ] End - Modify Code for DIS/UMA/OPTIMUS mode
    { END_OF_TABLE,  END_OF_TABLE,    0x2C,    PLATFORM_SSID,                               NULL }  /* End of Table */
};

EFI_STATUS
ProgramNvidiaSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
)
{
    UINT16     GpioOffset;
    UINT32     OptimusEnable, DgpuPresent, NvidiaSsidSvid;

    GpioOffset = (UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_LVL2;
    OptimusEnable = IoRead32(GpioOffset) & BIT6;

    GpioOffset = (UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_LVL3;
    DgpuPresent = IoRead32(GpioOffset) & BIT3;

    // For DIS only (OPTIMUS_EN#-HIGH, DGPU_PRSNT#-LOW), set SSID/SVID: 0x14C00063
    if ((OptimusEnable != 0) && (DgpuPresent == 0)) {
        NvidiaSsidSvid = OEM_DIS_VGA_SSID_SVID;
    } else {
        NvidiaSsidSvid = OEM_DEFAULT_SSID_SVID;
    }

    //
    // Program SSID & SVID as DID & VID.
    //
    CompalSsidSvidDxeProtocol->CompalSsidSvidDxePciWrite (Bus, Dev, Func, 0x40, WIDTH32, &NvidiaSsidSvid);

    return EFI_SUCCESS;
}

UINT32
GetPlatformSsidSvid (
    VOID
)
/*++
  Routine Description:
    Get default platform SSID/SVID

  Arguments:
    N/A

  Returns:
    SsidSvid     Default platform SSID/SVID

--*/
{
    UINT32    SsidSvid;

    //
    // Depend on project define, EX: EC board ID, CPU type, etc.
    //
    SsidSvid = OEM_DEFAULT_SSID_SVID;

    return SsidSvid;
}

EFI_STATUS
CompalSsidSvidDxeSettingEntry (
    IN EFI_HANDLE              ImageHandle,
    IN EFI_SYSTEM_TABLE        *SystemTable
)
/*++
  Routine Description:
    This is a driver to program Sub Vendor ID and Sub Device ID

  Arguments:
    ImageHandle     - Handle for the image of this driver
    SystemTable     - Pointer to the EFI System Table

  Returns:
    EFI_SUCCESS     - Protocol successfully started and installed

--*/
{
    gBS->LocateProtocol (&gCompalSsidSvidDxeProtocolGuid, NULL, &CompalSsidSvidDxeProtocol);

    CompalSsidSvidDxeProtocol->CompalSsidSvidDxeWrite ( GetPlatformSsidSvid(), CompalSsidSvidSpecialTable );

    return EFI_SUCCESS;
}
