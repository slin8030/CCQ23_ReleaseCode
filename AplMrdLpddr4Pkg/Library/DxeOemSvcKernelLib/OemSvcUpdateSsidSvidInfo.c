/** @file
  Provides an opportunity for OEM to decide what SSID/SVID to use.

;******************************************************************************
;* Copyright (c) 2013-2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Pi/PiDxeCis.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <ScPolicyCommon.h>
#include <Library/PciExpressLib.h>
#include <Library/S3BootScriptLib.h>

#include <Library/S3BootScriptLib.h>
#include <Library/IoLib.h>
#include <ScAccess.h>

EFI_STATUS
ProgramSsidSvid0xFFFF1002 (
  IN     UINT8    Bus,
  IN     UINT8    Dev,
  IN     UINT8    Func,
  IN OUT UINT32  *SsidSvid
  )
{
  UINT64 BootScriptPciAddress;
  //
  // Program SSID / SSVID
  //
  PciExpressWrite32 (PCI_EXPRESS_LIB_ADDRESS(Bus, Dev, Func, 0x4C), *SsidSvid);

  BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Dev, Func, 0x4C);
  S3BootScriptSavePciCfgWrite (
      S3BootScriptWidthUint32, 
      BootScriptPciAddress,
      1, 
      SsidSvid);
  
  return EFI_SUCCESS;
}

EFI_STATUS
ProgramSsidSvid0xFFFF10DE (
  IN     UINT8    Bus,
  IN     UINT8    Dev,
  IN     UINT8    Func,
  IN OUT UINT32  *SsidSvid
  )
{
  UINT64     BootScriptPciAddress;
  //
  // Program SSID / SSVID
  //
  PciExpressWrite32 (PCI_EXPRESS_LIB_ADDRESS(Bus, Dev, Func, 0x40), *SsidSvid);

  BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Dev, Func, 0x40);
  S3BootScriptSavePciCfgWrite (
      S3BootScriptWidthUint32, 
      BootScriptPciAddress,
      1, 
      SsidSvid);

  return EFI_SUCCESS;
}

STATIC OEM_SSID_SVID_TABLE SsidTable[] = {
  0x10DE, DEVICE_ID_DONT_CARE, ProgramSsidSvid0xFFFF10DE,
  0x1002, DEVICE_ID_DONT_CARE, ProgramSsidSvid0xFFFF1002,
  DEVICE_ID_DONT_CARE, DEVICE_ID_DONT_CARE, NULL
};

//[-start-151220-IB1127138-remove]//
// /**
//   Lock USB registers before boot
// 
//   @param[in] UsbConfig            The PCH Platform Policy for USB configuration
// 
// **/
// VOID
// UsbLateInit (
//   IN UINT32   SsidSvid
//   )
// {
//   UINT32  XhccCfg;
//   UINT32  XhccCfg2;
//   UINTN   XhciPciMmBase;
//   XhciPciMmBase   = MmPciAddress (
//                       0,
//                       DEFAULT_PCI_BUS_NUMBER_SC,
//                       PCI_DEVICE_NUMBER_XHCI,
//                       PCI_FUNCTION_NUMBER_XHCI,
//                       0
//                       );
//   DEBUG ((EFI_D_INFO, "UsbLateInit() - Start\n"));
// 
//   if (MmioRead32 (XhciPciMmBase + R_XHCI_VENDOR_ID) == B_XHCI_VENDOR_ID) {
//     return;
//   }
//   
//   MmioWrite32 (XhciPciMmBase + R_XHCI_SVID, SsidSvid);
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (XhciPciMmBase + R_XHCI_SVID),
//     1,
//     (VOID *) (UINTN) (XhciPciMmBase + R_XHCI_SVID)
//     );
//   
//   ///
//   /// CHV BIOS Spec, Section 18.2.5 Additional Settings for xHCI Controller
//   /// Note:
//   /// D20:F0:40h is write once register.
//   /// Unsupported Request Detected bit is write '1b' to clear
//   ///
// 
//   XhccCfg = MmioRead32 (XhciPciMmBase + R_XHCI_XHCC1);
//   XhccCfg |= (UINT32) (B_XHCI_XHCC1_URD);
//   ///
//   /// CHV BIOS Spec, Section 18.2.3 Locking xHCI Register Settings
//   /// After xHCI is initialized, BIOS should lock the xHCI configuration registers to RO.
//   /// This prevent any unintended changes. There is also a lockdown feature for OverCurrent
//   /// registers. BIOS should set these bits to lock down the settings prior to end of POST.
//   /// 1. Set Access Control bit at D20:F0:40h[31] to 1b to lock xHCI register settings.
//   /// 2. Set OC Configuration Done bit at D20:F0:44h[31] to lock overcurrent mappings from
//   ///    further changes.
//   ///
//   XhccCfg2 = MmioRead32 (XhciPciMmBase + R_XHCI_XHCC2);
//   ///
//   /// D20:F0:44h[31] is a RWO bit
//   /// So we program D20:F0:44h[25][24:22][19:14] here
//   /// Set D20:F0:0x44 [25] = 1b
//   /// Program D20:F0:0x44 [24:22] = 111b
//   /// Program D20:F0:0x44 [19:14] = 0x3F
//   ///
//   XhccCfg2 |= (UINT32) (B_XHCI_XHCC2_DREQBCC);
//   XhccCfg2 |= (UINT32) (B_XHCI_XHCC2_IDMARRSC);
//   XhccCfg2 |= (UINT32) (B_XHCI_XHCC2_OCCFDONE);
//   XhccCfg2 |= (UINT32) (B_XHCI_XHCC2_UNPPA);
//   MmioWrite32 (XhciPciMmBase + R_XHCI_XHCC2, (UINT32) (XhccCfg2));
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (XhciPciMmBase + R_XHCI_XHCC2),
//     1,
//     (VOID *) (UINTN) (XhciPciMmBase + R_XHCI_XHCC2)
//     );
//   
//   XhccCfg |= (UINT32) (B_XHCI_XHCC1_ACCTRL);
//   MmioWrite32 (XhciPciMmBase + R_XHCI_XHCC1, XhccCfg);
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (XhciPciMmBase + R_XHCI_XHCC1),
//     1,
//     &XhccCfg
//     );
//   DEBUG ((EFI_D_INFO, "UsbLateInit() - End\n"));
// }
//[-end-151220-IB1127138-remove]//

/**
  The OemSvc is used to update SSID/SVID value by OEM.

  @param[in]      Bus                   Bus number.
  @param[in]      Dev                   Device number.
  @param[in]      Func             	    Function number. 
  @param[in]      VendorID              Vendor ID.
  @param[in]      DeviceID              Device ID.
  @param[in]      ClassCode             Class Code.
  @param[in out]  SsidSvid              Pointer to SSID/SVID.

  @retval     EFI_UNSUPPORTED       Returns unsupported by default.
  @retval     EFI_SUCCESS           OEM handled SSID/SVID programming on this PCI device. Skip default kernel
                                    programming mechanism. 
  @retval     EFI_MEDIA_CHANGED     Updated SsidSvid value and returned this value for default programming mechanism.
**/
EFI_STATUS
OemSvcUpdateSsidSvidInfo (
  IN     UINT8     Bus,
  IN     UINT8     Dev,
  IN     UINT8     Func,
  IN     UINT16    VendorId,
  IN     UINT16    DeviceId,
  IN     UINT16    ClassCode,
  IN OUT UINT32   *SsidSvid
  )
{

//[-start-151220-IB1127138-modify]//
  EFI_STATUS                        Status;
  UINT8       Index;
  UINT16      Ssid;
  UINT16      Svid;

  Svid = V_INTEL_VENDOR_ID;
  Ssid = V_SC_DEFAULT_SID;
  
  Status = EFI_UNSUPPORTED;
  for (Index = 0; SsidTable[Index].SpecialSsidSvidFunction != NULL; Index++ ) {
    if (SsidTable[Index].VendorId == VendorId) {
      if ((SsidTable[Index].DeviceId == DEVICE_ID_DONT_CARE)
        || (SsidTable[Index].DeviceId == DeviceId)){
        Status = SsidTable[Index].SpecialSsidSvidFunction (Bus, Dev, Func, SsidSvid);
        break;
      }
    }
  }
  
  if (!EFI_ERROR(Status)) {
    return Status;
  }
  
  *SsidSvid = (UINT32) ((Ssid << 16) | Svid);
//[-end-151220-IB1127138-modify]//
  
  return EFI_MEDIA_CHANGED;
}
