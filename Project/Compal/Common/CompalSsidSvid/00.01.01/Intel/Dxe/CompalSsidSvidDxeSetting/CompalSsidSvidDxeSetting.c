
#include "CompalSsidSvidDxeSetting.h"
#include <Include/ProjectSpecificSsidSvid.h>

COMPAL_SSID_SVID_DXE_PROTOCOL    *CompalSsidSvidDxeProtocol;

static COMPAL_SSID_SVID_SPECIAL_TABLE        CompalSsidSvidSpecialTable[] = {
    PRJ_SPECIFIC_SSID_SVID_TABLE
};

EFI_STATUS
ProgramNvidiaSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
)
{
  UINT64     BootScriptPciAddress;
  //
  // Program SSID / SSVID
  //
  PciExpressWrite32 (PCI_EXPRESS_LIB_ADDRESS(Bus, Dev, Func, 0x40), SsidSvid);

  BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Dev, Func, 0x40);
  S3BootScriptSavePciCfgWrite (
      S3BootScriptWidthUint32, 
      BootScriptPciAddress,
      1, 
      &SsidSvid);

  return EFI_SUCCESS;
}

EFI_STATUS
ProgramAmdSsidSvid (
  IN     UINT8    Bus,
  IN     UINT8    Dev,
  IN     UINT8    Func,
  IN     UINT32   SsidSvid
  )
{
  UINT64 BootScriptPciAddress;
  //
  // Program SSID / SSVID
  //
  PciExpressWrite32 (PCI_EXPRESS_LIB_ADDRESS(Bus, Dev, Func, 0x4C), SsidSvid);

  BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Dev, Func, 0x4C);
  S3BootScriptSavePciCfgWrite (
      S3BootScriptWidthUint32, 
      BootScriptPciAddress,
      1, 
      &SsidSvid);
 
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
