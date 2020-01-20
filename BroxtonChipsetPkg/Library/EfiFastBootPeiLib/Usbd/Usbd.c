/**

Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/          

#include <UsbdPei.h>
//#include <Library/PlatformFsaLib.h>
#include <Library/PmicLib.h>
#include <Guid/PlatformInfo.h>
#include <Library/HobLib.h>
#include <SaAccess.h>
//these 2 header files are for GPIO
#include "PlatformBaseAddresses.h"
#include <Ppi/Stall.h>
#include <Library/BasememoryLib.h>
#include <Library/PrintLib.h>

EFI_PLATFORM_INFO_HOB       *mPlatformInfo=NULL;
EFI_PEI_USBDEVICEMODE_PPI   *gUdmPpi;

/* Fastboot configurable device strings. Must be NULL terminated */
static CHAR16 gStrProduct[] = {L'C', L'h', L'e', L'r', L'r', L'y', L't', L'r', L'a', L'i', L'l', L'\0'};
static CHAR16 gStrSerialNumber[32] = {0};
extern EFI_GUID gEfiEmmcSSNGuid;

//==========================content above is for SOC GPIO initialization in fastboot mode=========================
//
#define FAST_GPIO_INIT_END    0
#define OPCODE_USB_MMIO_READ 0
#define OPCODE_USB_MMIO_WRITE 1
#define PORTID_XHCI 0x5a

#define GPIO_RAISE 0x8102

EFI_STATUS
EFIAPI
ConfigureOtg (
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
  UINTN                 OtgPciMmBase;
  EFI_PHYSICAL_ADDRESS  OtgMmioBase0;
  UINT32                Buffer32;
  UINT32                PmcBase;
  UINTN                 PciD31F0RegBase;
  //UINT16                val=0;
  UINT32                val32=0;

  PciD31F0RegBase = MmPciAddress (0,
                        0,
                        PCI_DEVICE_NUMBER_LPC,
                        PCI_FUNCTION_NUMBER_LPC,
                        0
                        );

  PmcBase         = MmioRead32 (PciD31F0RegBase + R_LPC_PMC_BASE) & B_LPC_PMC_BASE_BAR;

  DEBUG ((EFI_D_INFO, "ConfigureOtg() Start\n"));

  OtgPciMmBase = MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_SC,
                   PCI_DEVICE_NUMBER_PCH_OTG,
                   PCI_FUNCTION_NUMBER_PCH_OTG,
                   0
                 );
  OtgMmioBase0 = 0;
  Buffer32     = 0;


  //power on OTG controller and PHY
  MmioAnd32 ((PmcBase + R_PCH_PMC_FUNC_DIS),~((UINT32)B_PCH_PMC_FUNC_DIS_OTG) );
  MmioAnd32 ((PmcBase + R_PCH_PMC_FUNC_DIS2),~((UINT32)B_PCH_PMC_FUNC_DIS2_OTG_SS_PHY) );
  //put OTG into D0
  MmioAnd32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_PMECTLSTS), ~((UINT32)B_PCH_OTG_PMECTLSTS_POWERSTATE));

  val32=MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS);
  DEBUG ((EFI_D_INFO, "R_PCH_PMC_FUNC_DIS:0x%x\n",  val32));
  val32=MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS2);
  DEBUG ((EFI_D_INFO, "R_PCH_PMC_FUNC_DIS2:0x%x\n", val32));
  val32=MmioRead32 (OtgPciMmBase + R_PCH_OTG_PMECTLSTS);
  DEBUG ((EFI_D_INFO, "R_PCH_OTG_PMECTLSTS:0x%x\n", val32));

  ///
  /// Check if device present
  ///
  if (MmioRead32 (OtgPciMmBase) != 0xFFFFFFFF) {
    //hard code a physical address . Will free it after fastboot mode
    OtgMmioBase0 = 0x80000000;
    DEBUG ((EFI_D_INFO, "Hard code OtgMmioBase:0x%x\n", OtgMmioBase0));

      
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {

      ///
      /// GPIOS_43/USB_ULPI_REFCLK is toggling by default, BIOS should disable it.BYT Platform HSD #4753278
      /// Until boot to OS and load USB driver, this clock will be enabled by driver.
      /// Must use USB device driver 1.0.0.143 and above.
      ///
      //MmioOr32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_GEN_REGRW1), B_PCH_OTG_ULPIPHY_REFCLK_DISABLE);
      //MmioWrite32((UINTN) (OtgPciMmBase + R_PCH_OTG_GEN_REGRW1), 
      //              (UINT32)(VOID *)(UINTN)(OtgPciMmBase + R_PCH_OTG_GEN_REGRW1));


      ///
      /// Disable Bus Master Enable & Memory Space Enable
      ///
      //MmioAnd32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD),  (UINT32) ~(B_PCH_OTG_STSCMD_BME | B_PCH_OTG_STSCMD_MSE));
      //MmioWrite32((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD), (UINT32)(VOID *)(UINTN)(OtgPciMmBase + R_PCH_OTG_STSCMD));

      ///
      /// Program BAR 0
      ///
      ASSERT (((OtgMmioBase0 & B_PCH_OTG_BAR0_BA) == OtgMmioBase0) && (OtgMmioBase0 != 0));
      MmioWrite32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0), (UINT32) (OtgMmioBase0 & B_PCH_OTG_BAR0_BA));
      //MmioWrite32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0),  (UINT32)(VOID *)(UINTN)(OtgPciMmBase + R_PCH_OTG_BAR0));

      ///
      /// Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD), (UINT32) (B_PCH_OTG_STSCMD_BME | B_PCH_OTG_STSCMD_MSE));
      //MmioWrite32((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD), 
      //              (UINT32)(VOID *)(UINTN)(OtgPciMmBase + R_PCH_OTG_STSCMD));

      ASSERT (MmioRead32 ((UINTN) OtgMmioBase0) != 0xFFFFFFFF);
      val32= MmioRead32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0));
      DEBUG ((EFI_D_INFO, "OTG OtgMmioBase:0x%x\n", val32));
     
    }
  } else {
    DEBUG ((EFI_D_ERROR, "USB3 OTG not present, skipping.\n"));
  }

  DEBUG ((EFI_D_INFO, "ConfigureOtg() End\n"));

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FastBootDataInit (
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  UINTN        OtgPciMmBase, OtgMmioBase0;
  EFI_STATUS    Status;
  UINT32      Value = 0;
  EFI_PEI_HOB_POINTERS   GuidHob;
  EFI_PEI_STALL_PPI          *StallPpi;
  UINTN                      Count;


  
  DEBUG ((DEBUG_INFO, "FASTBOOT: Fastboot start\nLocate the Usb Device Mode protocol:"));

  //
  // Get Platform Info
  //
  GuidHob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (GuidHob.Raw != NULL);
  mPlatformInfo = GET_GUID_HOB_DATA(GuidHob.Raw);

  Status = (*PeiServices)->LocatePpi (
                              PeiServices,
                              &gPeiUsbDeviceModePpiGuid,
                              0,
                              NULL,
                              (VOID **) &gUdmPpi);
  ASSERT(!EFI_ERROR(Status));
  
  Status = (*PeiServices)->LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, "FASTBOOT: %r\n", Status));

  if(EFI_ERROR(Status)){
    DEBUG ((DEBUG_INFO, "FASTBOOT: Usb Device Mode protocol not ready, exit\n"));
    return EFI_SUCCESS;
  }

//  Status = SocGpioInit_FFD8();
  DEBUG ((DEBUG_INFO, "FASTBOOT: GPIO init done.\n"));
  
  Status = ConfigureOtg(PeiServices);
  ASSERT(!EFI_ERROR(Status));

  OtgPciMmBase = MmPciAddress (0,
         DEFAULT_PCI_BUS_NUMBER_SC,
         PCI_DEVICE_NUMBER_PCH_OTG,
         PCI_FUNCTION_NUMBER_PCH_OTG,
         0
         );

  OtgMmioBase0 = MmioRead32((UINTN)(OtgPciMmBase + R_PCH_OTG_BAR0)) & B_PCH_OTG_BAR0_BA;

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call UsbdSetMmio - MMIO=%x\n", OtgMmioBase0)); 

  gUdmPpi->UsbdSetMmio(OtgMmioBase0);

  //
  //Workaround clear BIT "ulpiphy_refclk_disable"
  //
  OtgMmioBase0 = MmioRead32((UINTN)(OtgPciMmBase + R_PCH_OTG_BAR0));

  DEBUG ((DEBUG_INFO, "FASTBOOT: WA for OTG MMIO  BAR0=%x\n", OtgMmioBase0)); 
  MmioWrite32(OtgMmioBase0 + R_PCH_OTG_GEN_REGRW1,0);

  DEBUG ((DEBUG_INFO, "FASTBOOT: WA: GEN_REGRW1 =%x\n", MmioRead32((UINTN)(OtgMmioBase0 + R_PCH_OTG_GEN_REGRW1)))); 

  if (SocStepping() < SocB0) {  
     MsgBus32WriteEx(OPCODE_USB_MMIO_WRITE, PORTID_XHCI, 0x80d8, 0x300800, 0, 0, 0xa0);
     //
     // Workaround for USB can not connect. Delay 0.5 second.
     //
     for (Count = 0; Count < 5000; Count ++) {
       StallPpi->Stall (PeiServices, StallPpi, 100);
     }
     
  } else{
    MsgBus32WriteEx(OPCODE_USB_MMIO_WRITE, PORTID_XHCI, 0x80d8, 0x1300800, 0, 0, 0xa0);  
  }
  
  MsgBus32ReadEx(OPCODE_USB_MMIO_READ, PORTID_XHCI, 0x80d8, Value, 0, 0, 0xa0);
    

  DEBUG ((DEBUG_INFO, "FASTBOOT: Set ID MUX of the OTG port to device mode. Value = %x\n", Value)); 

  if (mPlatformInfo != NULL) {
    if (mPlatformInfo->BoardId == BOARD_ID_CHT_CR) {
      //
      //MMC1_RCLK: Change the Platform Mux from PMIC to SoC
      //
            DEBUG ((DEBUG_INFO, "FASTBOOT: Change the Platform Mux from PMIC to SoC\n"));
          MmioWrite32(CR_USB_MUX_EN, GPIO_RAISE);
            MicroSecondDelay(100000);
    }
  }

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call UsbdInitDCI\n"));
  gUdmPpi->UsbdInitDCI();

  return EFI_SUCCESS;
}

VOID
AsciiToUnicode (
  IN    CHAR8     *AsciiString,
  OUT   CHAR16    *UnicodeString
  )
{
  UINT8           Index;

  Index = 0;
  while (AsciiString[Index] != 0) {
    UnicodeString[Index] = (CHAR16)AsciiString[Index];
    Index++;
  }
}

EFI_STATUS
EFIAPI
FastBootStart (
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_PEI_HOB_POINTERS  Hob;
  UINT32                *HobData=NULL;
  UINT8                 StrSerialNumberASCII[32];
  
  DEBUG ((DEBUG_INFO, "FASTBOOT: Turn off VBUS to enable OTG\n"));
    if (mPlatformInfo != NULL) {
      if (mPlatformInfo->PmicId == PMIC_TYPE_CRC_PLUS) {
        PmicVbusControl(FALSE);
      }
    }

  DEBUG ((DEBUG_INFO, "FASTBOOT: Switch to USB device mode\n"));

  //
  // Get eMMC S/N HOB
  //
  Hob.Raw = GetFirstGuidHob(&gEfiEmmcSSNGuid);
  if (Hob.Raw == NULL) {
    DEBUG ((DEBUG_INFO, "Get eMMC S/N fail.\n"));
    IoWrite8(0x80, 0xAA);
    CpuDeadLoop();
  } else {
    HobData = GET_GUID_HOB_DATA(Hob.Guid);
    DEBUG ((DEBUG_INFO, "Got eMMC S/N.\n"));
  }

  SetMem(gStrSerialNumber, sizeof(gStrSerialNumber), 0);
  SetMem(StrSerialNumberASCII, sizeof(StrSerialNumberASCII), 0);
  if(HobData != NULL) {
    AsciiSPrint(StrSerialNumberASCII, sizeof(StrSerialNumberASCII), "CHTT%08x", *HobData);
    AsciiToUnicode (StrSerialNumberASCII, gStrSerialNumber);
  }  
  

  DEBUG ((DEBUG_INFO, "FASTBOOT: Before init\n"));
  FastbootParams.DevProductId     = 0x09EF;
  FastbootParams.DevBcd           = 0x09EF;
  FastbootParams.pDevProductStr   = gStrProduct;  //PrDuStr;
  FastbootParams.pDevSerialNumStr = gStrSerialNumber;  //SNStr;
  FastbootParams.KernelLoadAddr   = 0x01100000;
  FastbootParams.KernelEntryOffset= 0x1000;
  FastbootParams.TimeoutMs        = 0xA000;

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call FBInit\n"));
  gUdmPpi->FBInit(&FastbootParams);

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call FBStart\n"));
  gUdmPpi->FBStart();

  //DEBUG ((DEBUG_INFO, "FASTBOOT: deadloop for debugging\n"));
  //CpuDeadLoop();
  
  return EFI_SUCCESS;
}
