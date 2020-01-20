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

#include <Usbd.h>
#include <Library/PupdrPlatformLib.h>
#include <Guid/PlatformInfo.h>
#include <Library/HobLib.h>
#include <Library/I2CLib.h>
#include <Library/PmicLib/PmicWhiskeyCove.h>

EFI_PLATFORM_INFO_HOB               *mPlatformInfo=NULL;
EFI_RUNTIME_USBDEVICEMODE_PROTOCOL  *gUdmProtocol;

/* Fastboot configurable device strings. Must be NULL terminated */ 
static CHAR16 gStrProduct[] = {L'I', L'n', L't', L'e', L'l', L' ', L'A', L'n', L'd', L'r',L'o',L'i',L'd',L' ',L'A',L'D',L'B',L' ',L'I',L'n',L't',L'e',L'r',L'f',L'a',L'c',L'e',L'\0'};
static CHAR16 gStrSerialNumber[32] = {0};

EFI_STATUS
EFIAPI
FastBootDataInit (
VOID
  )
{
  UINT32        OtgPciMmBase, OtgMmioBase0;
  EFI_PEI_HOB_POINTERS   GuidHob;
  UINT8  Value;
  UINT8  ChgRegValue;
  UINTN  XhciPciMmBase;
  UINT32  XhciMmioBase;
  EFI_STATUS  Status;
  UINT32      BitValue, BitMask;

  //
  // Get Platform Info
  //
  GuidHob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (GuidHob.Raw != NULL);
  mPlatformInfo = GET_GUID_HOB_DATA(GuidHob.Raw);

  DEBUG ((DEBUG_INFO, "FASTBOOT: Fastboot start\n Locate the Usb Device Mode protocol:"));

  OtgPciMmBase = MmPciAddress (0,
         0,
         0x16,
         0,
         0
         );

  OtgMmioBase0 = MmioRead32((UINTN)(OtgPciMmBase + R_PCH_OTG_BAR0)) & B_PCH_OTG_BAR0_BA;

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call UsbdSetMmio - MMIO=%x\n", OtgMmioBase0)); 

  //gUdmProtocol->UsbdSetMmio(OtgMmioBase0);
  usbdSetMmioBar(OtgMmioBase0);

  //
  //Workaround clear BIT "ulpiphy_refclk_disable"
  //
  OtgMmioBase0 = MmioRead32((UINTN)(OtgPciMmBase + R_PCH_OTG_BAR1));

  DEBUG ((DEBUG_INFO, "FASTBOOT: WA for OTG MMIO  BAR1=%x\n", OtgMmioBase0)); 
  MmioWrite32(OtgMmioBase0 + 0xA0,0);

  DEBUG ((DEBUG_INFO, "FASTBOOT: WA: GEN_REGRW1 =%x\n", MmioRead32((UINTN)(OtgMmioBase0 + 0xA0)))); 

  //
  // Switching to XDCI, Device Mode
  //
  XhciPciMmBase = MmPciAddress (
                    0,
                    0,
                    PCI_DEVICE_NUMBER_XHCI,
                    PCI_FUNCTION_NUMBER_XHCI,
                    0
                  );

  XhciMmioBase = MmioRead32 (XhciPciMmBase + R_XHCI_MEM_BASE) & B_XHCI_MEM_BASE_BA;
  if (SocStepping() < SocB0) {
    MmioOr32 (
      ((UINT32)XhciMmioBase + 0x80D8),
      (UINT32) (BIT21|BIT20)
    );
  }else{
    MmioOr32 (
      ((UINT32)XhciMmioBase + 0x80D8),
      (UINT32) (BIT21| BIT20 | BIT24)
    );
    }
  //
  // 0x80DC register, has a status bit to acknowledge the role  change in Bit 29
  //

  BitMask  = (UINT32) (0x20000000);
  BitValue = (UINT32) (1 << 29);
  Status = PchMmioPoll32 (
            (UINTN) (XhciMmioBase + 0x80DC),
            BitMask,
            (~BitValue),
            1000
            );

   if (Status == EFI_TIMEOUT) {
      DEBUG ((EFI_D_ERROR , "Timeout while polling on xHCI BAR + 0x80DC [29] for 1b\n"));
    }

  if (mPlatformInfo != NULL) {
    if (mPlatformInfo->BoardId == BOARD_ID_CHT_CR) {
      //    
      //MMC1_RCLK: Change the Platform Mux from PMIC to SoC
      //
      MmioOr32 (CR_USB_MUX_EN, BIT1);
  }
  if (mPlatformInfo->PmicId==PMIC_TYPE_WC) {
    //Configure the WC PMIC Registers back to their default state
    //
      //Set CHGRCTRL1_REG [6]=1 for OTGMODE (i.e. for the Software Control on Charger)
      //
      Status = ByteReadI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_CHGRCTRL1_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value |= 0x40;  
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_CHGRCTRL1_REG, 1, &Value);
      if(!EFI_ERROR(Status)){
        DEBUG ((EFI_D_ERROR, "WC: CHGRCTRL1_REG Config passed"));
      }
      MicroSecondDelay(100000);
      //
      //Set Charger bq24292i: REG01[5:4]=01     for ChargeBattery Mode
      //
      Value = 0x6B; //Charger Slave Address
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRDADDR_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x01; //Charger Register Offset
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVROFFSET_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x2; //0x1 to Write, 0x2 to Read
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRCTRL_REG, 1, &Value);
      MicroSecondDelay(100000);
      //Read Charger Data from I2COVRRDDATA_REG
      Status = ByteReadI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRRDDATA_REG, 1, &ChgRegValue);
      MicroSecondDelay(100000);
      ChgRegValue &= 0xDF; //[5]=0
      ChgRegValue &= 0x10; //[4]=1
      Value = 0x6B; //Charger Slave Address
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRDADDR_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x01; //Charger Register Offset
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVROFFSET_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = ChgRegValue; //Write to I2COVRWRDATA_REG
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRWRDATA_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x1; //0x1 to Write, 0x2 to Read
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRCTRL_REG, 1, &Value);
      MicroSecondDelay(100000);
      if(!EFI_ERROR(Status)){
        DEBUG ((EFI_D_ERROR, "WC: <DnxFastBoot> CHG REG01 Config passed"));
      }
      
      //
      //Reset CHG REG05 [5:4]=01 to set the Watchdog timer back to default 40s
      //
      Value = 0x6B; //Charger Slave Address
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRDADDR_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x05; //Charger Register Offset
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVROFFSET_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x2; //0x1 to Write, 0x2 to Read
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRCTRL_REG, 1, &Value);
      MicroSecondDelay(100000);
      //Read Charger Data from I2COVRRDDATA_REG
      Status = ByteReadI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRRDDATA_REG, 1, &ChgRegValue);
      MicroSecondDelay(100000);
      ChgRegValue &= 0xDF; //[5]=0
      ChgRegValue |= 0x10; //[4]=1
      Value = 0x6B; //Charger Slave Address
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRDADDR_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x05; //Charger Register Offset
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVROFFSET_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = ChgRegValue; //Write to I2COVRWRDATA_REG
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRWRDATA_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value = 0x1; //0x1 to Write, 0x2 to Read
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_I2COVRCTRL_REG, 1, &Value);
      MicroSecondDelay(100000);
      if(!EFI_ERROR(Status)){
        DEBUG ((EFI_D_ERROR, "WC: <DnxFastBoot> CHG REG05 Config passed"));
      }
      
      //
      //Set CHGRCTRL1_REG [6]=0 for OTGMODE (i.e. to disable the Software Control on Charger)
      //
      Status = ByteReadI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_CHGRCTRL1_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value &= 0xBF;  
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_CHARGER_CHGRCTRL1_REG, 1, &Value);
      MicroSecondDelay(100000);
      if(!EFI_ERROR(Status)){
        DEBUG ((EFI_D_ERROR, "WC: <DnxFastBoot> CHGRCTRL1_REG Config passed"));
      }
      
      //
      //Set USBPHYCTRL_REG [0]=1 external PHY is out of reset, USB switches on and internal detection disabled
      //
      Status = ByteReadI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_USB_USBPHYCTRL_REG, 1, &Value);
      MicroSecondDelay(100000);
      Value |= 0x01;  //Set BIT0
      Status = ByteWriteI2C(WC_PMIC_I2C_CHANNEL_NUMBER, WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS, DEV3_USB_USBPHYCTRL_REG, 1, &Value);
      MicroSecondDelay(100000);
      if(!EFI_ERROR(Status)){
        DEBUG ((EFI_D_ERROR, "WC: <DnxFastBoot> USBPHYCTRL_REG Config passed"));
      }
      
    }
  }
  
  DEBUG ((DEBUG_INFO, "FASTBOOT: Call UsbdInitDCI\n"));
  //gUdmProtocol->UsbdInitDCI();
  usbdInitDCI();

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FastBootStart (
VOID
  )
{
  GetSSN((UINT8*)&gStrSerialNumber[0]);
  FastbootParams.DevProductId=0x0A65;
  FastbootParams.DevBcd=0x0A65;
  FastbootParams.pDevProductStr=gStrProduct;  //PrDuStr;
  FastbootParams.pDevSerialNumStr=gStrSerialNumber;  //SNStr;
  FastbootParams.KernelLoadAddr=0x01100000;
  FastbootParams.KernelEntryOffset=0x1000;
  FastbootParams.TimeoutMs=0x6000000;

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call FBInit\n"));
  //gUdmProtocol->FBInit(&FastbootParams);
  fbInit(&FastbootParams);

  DEBUG ((DEBUG_INFO, "FASTBOOT: Call FBStart\n"));
  //gUdmProtocol->FBStart();
  fbStart();

  return EFI_SUCCESS;
}
