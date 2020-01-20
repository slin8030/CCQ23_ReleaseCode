/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    Platform.c

Abstract:

    This is a generic template for a child of the IchSmm driver.

 
--*/

//[-start-151229-IB03090424-modify]//
#include "SmmPlatform.h"
#include <Protocol/CpuIo2.h>
#include <Library/SideBandLib.h>
#include <Library/DxeInsydeChipsetLib.h>

#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)   
#include <Protocol/H2OSmmChipsetServices.h>
#include <SmiTable.h> 
//[-start-160415-IB08450340-modify]//
#include <Library/EcMiscLib.h>
//[-end-160415-IB08450340-modify]//
#endif

#include <Library/ReportStatusCodeLib.h>
//[-start-170410-IB07400857-modify]//
//#if BXTI_PF_ENABLE
#include <Guid/PlatformInfo.h>
//#endif
//[-end-170410-IB07400857-modify]//
//[-start-160609-IB07400742-add]//
#include <Library/PostCodeLib.h>
#include <PostCode.h>
//[-end-160609-IB07400742-add]//
//[-start-161109-IB07400810-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-161109-IB07400810-add]//
//[-start-161124-IB07400817-add]//
#include <Library/ScPlatformLib.h>
#include <Private/Library/ScInitCommonLib.h>
//[-end-161124-IB07400817-add]//
//[-start-161230-IB07400829-add]//
#include <ChipsetPostCode.h>
//[-end-161230-IB07400829-add]//
//[-start-170626-IB07400880-add]//
#include <ChipsetCmos.h>
#include <Library/CmosLib.h>
#define USB_CONFIG_PREPARE_TO_SLEEP1  0x01
#define USB_CONFIG_PREPARE_TO_SLEEP2  0x02
#define USB_CONFIG_READY_TO_SHUTDOWN  0x03
//[-end-170626-IB07400880-add]//

#define NMI_REG_OFFSET 0x3330
#define NMI_NOW BIT0
#define NMI2SMI_EN BIT2

//[-start-161109-IB07400810-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// Apollo Lake RVP Pins:
//
// GPIO_22(A26) -- GPIO_22_R -- V5P0_3P3_SSD (SATA Power)
// GPIO_35(C41) -- TCH_PNL_PG -- V3P3_1P8_DX_TCH (Touch Pannel Power)
//
#define N_GPIO_22             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x00B0
#define N_GPIO_35             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0118
#endif
//[-end-161109-IB07400810-add]//

//
//
// Local variables
//
typedef struct {
  UINT8     Device;
  UINT8     Function;
} EFI_PCI_BUS_MASTER;

EFI_PCI_BUS_MASTER  mPciBm[] = {
  { PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1 },
};

//
// If need to support more than 1 key sequence, use below register range for programming.
//   Key set 1 index range: 0x00 - 0x0E
//   Key set 2 index range: 0x30 - 0x3E
//   Key set 3 index range: 0x40 - 0x4E
//
#if (defined KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT) && KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT
EFI_KEY_SEQUENCE S5WakePowerKeySequence[] = {
   {0x00, 0x37},
   {0x01, 0xF0},
   {0x02, 0xE0},
   {0x03, 0x37},
   {0x04, 0xE0},
   {0x05, 0x00},
   {0x06, 0x00},
   {0x07, 0x00},
   {0x08, 0x00},
   {0x09, 0x00},
   {0x0A, 0x00},
   {0x0B, 0x00},
   {0x0C, 0x00},
   {0x0D, 0x00},
   {0x0E, 0x00},
  };
UINTN S5WakePowerKeySequenceCount = sizeof (S5WakePowerKeySequence) / sizeof(EFI_KEY_SEQUENCE);

EFI_KEY_SEQUENCE S5WakeAltPrintScreenSequence[] = {
   {0x00, 0x84},
   {0x01, 0xF0},
   {0x02, 0x84},
   {0x03, 0x00},
   {0x04, 0x00},
   {0x05, 0x00},
   {0x06, 0x00},
   {0x07, 0x00},
   {0x08, 0x00},
   {0x09, 0x00},
   {0x0A, 0x00},
   {0x0B, 0x00},
   {0x0C, 0x00},
   {0x0D, 0x00},
   {0x0E, 0x00},
  };
UINTN S5WakeAltPrintScreenSequenceCount = sizeof (S5WakeAltPrintScreenSequence) / sizeof(EFI_KEY_SEQUENCE);
#endif

BOARD_AA_NUMBER_DECODE DefectiveBoardIdTable [] = {
  {"E76523", 302},  //BLKD510MO
//  {"E76523", 302},  //BOXD510MO
  {"E76525", 301},  //LAD510MO
  {"E67982", 303}   //LAD510MOV
};

UINTN                                   DefectiveBoardIdTableSize = sizeof (DefectiveBoardIdTable) / sizeof (BOARD_AA_NUMBER_DECODE);

EFI_SMM_BASE2_PROTOCOL                  *mSmmBase;
EFI_SMM_SYSTEM_TABLE2                   *mSmst;
UINT16                                  mAcpiBaseAddr;
CHIPSET_CONFIGURATION                   mSystemConfiguration;
EFI_SMM_VARIABLE_PROTOCOL               *mSmmVariable;
EFI_GLOBAL_NVS_AREA_PROTOCOL            *mGlobalNvsAreaPtr;

#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)    
UINT16                                  mPM1_SaveState16;
UINT32                                  mGPE_SaveState32;
H2O_SMM_CHIPSET_SERVICES_PROTOCOL       *mSmmChipsetSvc = NULL;
#endif
BOOLEAN                                 mSetSmmVariableProtocolSmiAllowed = TRUE;

#if (defined KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT) && KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT
EFI_WPC83627_POLICY_PROTOCOL            *LpcWpc83627Policy;
UINT16                                  Ps2KeyboardPresence;
UINT8                                   mWakeOnS5KeyboardVariable;
#endif

//
// Variables. Need to initialize this from Setup
//
BOOLEAN                                 mWakeOnLanS5Variable;
//[-start-170314-IB07400847-modify]//
UINT8                                   mWakeOnRtcVariable;
//BOOLEAN                                 mWakeOnLanVariable;
//UINT8                                   mWakeupDay;
//UINT8                                   mWakeupHour;
//UINT8                                   mWakeupMinute;
//UINT8                                   mWakeupSecond;
//UINT8                                   mDeepStandby=0;
//[-end-170314-IB07400847-modify]//

//
// Use an enum. 0 is Stay Off, 1 is Last State, 2 is Stay On
//
UINT8                                   mAcLossVariable;

//
// APM 1.2 State variables
//
UINT8                                   mAPMInterfaceConnectState;
BOOLEAN                                 mAPMEnabledState;
BOOLEAN                                 mAPMEngagedState;

//[-start-161230-IB07400829-modify]//
static
UINT8 mTco1Sources[] = {
  IchnTcoTimeout,
  IchnNmi
};
//[-end-161230-IB07400829-modify]//

UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );

VOID
S4S5ProgClock();

EFI_STATUS
SaveRestoreState (
  IN BOOLEAN                        SaveRestoreFlag
  );

//[-start-170314-IB07400847-remove]//
//EFI_STATUS
//S5SleepWakeOnRtcCallBack (
//  IN  EFI_HANDLE                   DispatchHandle,
//  IN  EFI_SMM_SX_REGISTER_CONTEXT  *DispatchContext
//  );
//[-end-170314-IB07400847-remove]//

#if (defined KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT) && KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT
VOID
S5Ps2WakeFromKbcSxCallBack(
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext
  );
#endif

#if (defined KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT) && KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT
VOID
EnableS5WakeFromKBC();
#endif

//[-start-170314-IB07400847-modify]//
EFI_STATUS
EnableS5WakeOnRtc(
  VOID
  );

EFI_STATUS
EfiSmmGetTime (
  IN OUT EFI_TIME *Time
  );

EFI_STATUS
EfiSmmSetWakeupTime (
  IN BOOLEAN    DateAlarmEnable,
  IN EFI_TIME   *Time
  );
//[-end-170314-IB07400847-modify]//

EFI_STATUS
Stall (
  IN UINTN              Microseconds
  );

UINT8
HexToBcd(
  UINT8 HexValue
  );

UINT8
BcdToHex(
  IN UINT8 BcdValue
  );

//[-start-161101-IB03090435-remove]//
//#if defined( ECC_SUPPORT ) && ( ECC_SUPPORT != 0 )
//EFI_STATUS
//EfiSmmGetTime (
//  IN OUT EFI_TIME *Time
//  );
//#endif
//[-end-161101-IB03090435-remove]//

VOID
CpuSmmSxWorkAround(
  );

//[-start-160421-IB10860195-add]//
VOID
S3SaveRuntimeScriptTable (
  );

VOID
S3SaveRuntimeScriptTable (
  )
{
  UINT32 Data32;
  UINT16 Data16;
  UINT8  Data8;
  UINT32 DwordData;
  UINT32 PciOffset;


//[-start-160609-IB07400742-remove]//
//  IoWrite8(0x80, 0x53);  
//[-end-160609-IB07400742-remove]//

  for (PciOffset = 0x10; PciOffset < 0x0cf; PciOffset += 4) { 
    DwordData = MmioRead32 (MmPciAddress (0,  SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, PciOffset));    
    
    S3BootScriptSaveMemWrite( 
      S3BootScriptWidthUint32,
      MmPciAddress (0,  SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, PciOffset),
      1,
      &DwordData
      );

  }

//[-start-160609-IB07400742-remove]//
//  IoWrite8(0x80, 0x53);  
//[-end-160609-IB07400742-remove]//
 
  //
  // Save I/O ports to S3 script table
  //

  //
  // Selftest KBC
  //
  Data8 = 0xAA;
  S3BootScriptSaveIoWrite (
    S3BootScriptWidthUint8,
    0x64,
    (UINTN)1,
    &Data8
    );

  Data32 = IoRead32(mAcpiBaseAddr + R_SMI_EN);
  S3BootScriptSaveIoWrite (
     S3BootScriptWidthUint32,
     (mAcpiBaseAddr + R_SMI_EN),
     1,
     &Data32
     );
  //
  // Save B_ICH_TCO_CNT_LOCK so it will be done on S3 resume path.
  //
  Data16 = IoRead16(mAcpiBaseAddr + R_TCO1_CNT);
  S3BootScriptSaveIoWrite (
     S3BootScriptWidthUint16,
     mAcpiBaseAddr + R_TCO1_CNT,
     1,
     &Data16
     );

}
//[-end-160421-IB10860195-add]//


#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)    
EFI_STATUS
EFIAPI
OsResetSmi (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  EFI_STATUS                                Status;

//  mSmmOemServices->Funcs[COMMON_SMM_OS_RESET_CALLBACK] (
//                     mSmmOemServices,
//                     COMMON_SMM_OS_RESET_CALLBACK_ARG_COUNT
//                     );
  Status = OemSvcOsResetCallback ();
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcKernelLib OsResetCallback, Status : %r\n", Status ) );
  if ( Status == EFI_SUCCESS ) {
    return Status;
  }
  //
  // Avoid slave harddisk could not attach problem
  // need to set reset command in IDE controller.
  // But it need to take time.
  //
  do {
//[-start-160914-IB07400784-modify]//
    mSmmChipsetSvc->ResetSystem (EfiResetWarm);
//[-end-160914-IB07400784-modify]//
  } while(1);
  return Status;
}
#endif

VOID
StallInternalFunction(
  IN UINTN              Microseconds
  )
{
  UINTN                 Ticks;
  UINTN                 Counts;
  UINT32                CurrentTick;
  UINT32                OriginalTick;
  UINT64                RemainingTick;
  UINT16                mAcpiBaseAddr;

  mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

//[-start-180813-IB07400997-modify]//
  OriginalTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//

  CurrentTick = OriginalTick;

  //
  //The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
  Counts = Ticks / V_ACPI_PM1_TMR_MAX_VAL;  //The loops needed by timer overflow
  RemainingTick = Ticks % V_ACPI_PM1_TMR_MAX_VAL;  //remaining clocks within one loop

  //
  //not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  //one I/O operation, and maybe generate SMI
  //

  while (Counts != 0) {
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
    if (CurrentTick < OriginalTick) {
//[-end-180813-IB07400997-modify]//
      Counts --;
    }
    OriginalTick = CurrentTick;
  }

  while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick) ) {
    OriginalTick = CurrentTick;
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//
  }
}

EFI_STATUS
Stall (
  IN UINTN              Microseconds
  )
/*++

Routine Description:

  Waits for at least the given number of microseconds.

Arguments:

  Microseconds          - Desired length of time to wait

Returns:

  EFI_SUCCESS           - If the desired amount of time passed.

--*/
{
  if (Microseconds == 0) {
    return EFI_SUCCESS;
  }

  StallInternalFunction(Microseconds);

  return EFI_SUCCESS;
}
//[-start-170314-IB07400847-add]//
EFI_STATUS
EFIAPI
ChipsetMiscSwSmiCallback (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  UINT8       Function;
  EFI_STATUS  Status;

  Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, 0xB3, 1, &Function);
  DEBUG ((EFI_D_ERROR, "ChipsetMiscSwSmiCallback, Function = %x\n", Function));
  
  switch (Function) {

  case CHIPSET_MISC_SW_SMI_SHUTDOWN_HOOK:
    DEBUG ((EFI_D_ERROR, "Chipset Shutdown Hook!!\n"));
    
    if (mWakeOnRtcVariable != SCU_S5_WAKE_ON_RTC_DISABLED) {
      DEBUG ((EFI_D_ERROR, "Chipset Shutdown Hook: Enable S5 Wake On RTC!!\n"));
      EnableS5WakeOnRtc();
    }

    if (mAcLossVariable == 2) {
      DEBUG ((EFI_D_ERROR, "Chipset Shutdown Hook: Config After G3 settings!!\n"));
      SetAfterG3On (TRUE);
    }
//[-start-170626-IB07400880-add]//
    if (mSystemConfiguration.S5WakeOnUsb) {
      DEBUG ((EFI_D_ERROR, "Chipset Shutdown Hook: Config Wake on USB settings!!\n"));      
      Stall(500 * 1000);
      WakeOnUsbFromS5(USB_CONFIG_READY_TO_SHUTDOWN);
    } 
//[-end-170626-IB07400880-add]//
//[-start-171115-IB07400928-add]//
    //
    // If Wake On PME is disabled, disable all PCIE Wake Event (Reset -s)
    //
    if ((!mSystemConfiguration.WakeOnPME)) {
  	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE0_EN));
  	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE1_EN));
  	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE2_EN));
  	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE3_EN));
    } 
    else if (mSystemConfiguration.WakeOnPME == SCU_WAKE_ON_PME_FORCE_ENALBED) {
      IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE0_EN);
      IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE1_EN);
      IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE2_EN);
      IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE3_EN);
    }
//[-end-171115-IB07400928-add]//
    break;
    
  case 0x00: 
  default:
    break;
  }
  
  //
  // Clear Function Number
  //
  Function = 0x00;
  Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, 0xB3, 1, &Function);
  
  return EFI_SUCCESS;
}
//[-end-170314-IB07400847-add]//
//[-start-170626-IB07400880-add]//
VOID
WakeOnUsbFromS5 (
  IN UINT8  ConfigType // 1: Prepare to Sleep, 2: Ready to Shutdown   
  )
{
  UINT32    XhciPciBase;
  UINT32    UsbMmio;
  UINT32    Data32;
  UINT16    Data16;
  UINTN     Index;
  UINT32    PortScx;

  XhciPciBase = (UINT32)MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_XHCI, PCI_FUNCTION_NUMBER_XHCI, 0);
  DEBUG ((EFI_D_INFO, "XhciPciBase = %x\n", XhciPciBase));
  
  if (ConfigType == USB_CONFIG_PREPARE_TO_SLEEP1) { // Prepare to Sleep1
    //
    // Put XHCI to D0
    //
    Data32 = MmioRead32 (XhciPciBase + R_XHCI_PWR_CNTL_STS);
    Data32 &= ~V_XHCI_PWR_CNTL_STS_PWR_STS_D3;
    MmioWrite32 (XhciPciBase + R_XHCI_PWR_CNTL_STS, Data32);
    
    //
    // Enable XHCI MMIO
    //
    Data32 = MmioRead32 (XhciPciBase + R_XHCI_COMMAND_REGISTER);
    Data32 |= (B_XHCI_COMMAND_BME | B_XHCI_COMMAND_MSE);
    MmioWrite32 (XhciPciBase + R_XHCI_COMMAND_REGISTER, Data32);
    
    //
    // Enable XHCI SMI
    //
    Data32 = IoRead32 (mAcpiBaseAddr + R_SMI_EN); 
    Data32 |= B_SMI_EN_LEGACY_USB3;
    IoWrite32 (mAcpiBaseAddr + R_SMI_EN, Data32); 
    
    UsbMmio = MmioRead32 (XhciPciBase + R_XHCI_MEM_BASE) & B_XHCI_MEM_BASE_BA;
    
//[-start-170628-IB07400881-add]//
    //
    // Workaround for XHCI reset hang for Win7
    //
    Data32 = MmioRead32 (UsbMmio + 0x880C);
    Data32 |= (BIT30 | BIT31);
    MmioWrite32 (UsbMmio + 0x880C, Data32);
//[-end-170628-IB07400881-add]//

    //
    // Enable SMI on OS Ownership changed
    //
    Data16 = MmioRead16 (UsbMmio + R_XHCI_USB_LEGACY_CTRL_STS);
    Data16 |= B_XHCI_SMI_ON_OS_OWNERSHIP;
    MmioWrite16 (UsbMmio + R_XHCI_USB_LEGACY_CTRL_STS, Data16);

    //
    // Switch OwnerShip to BIOS
    //
    Data32 = MmioRead32 (UsbMmio + R_XHCI_USB_LEGACY_SUPPORT);
    Data32 &= ~B_XHCI_HC_OS_OWNED_SEMAPHORE;
    Data32 |= B_XHCI_HC_BIOS_OWNED_SEMAPHORE;
    MmioWrite32 (UsbMmio + R_XHCI_USB_LEGACY_SUPPORT, Data32);
  }
  
  if (ConfigType == USB_CONFIG_PREPARE_TO_SLEEP2) { // Prepare to Sleep2
    //
    // Disable XHCI SMI
    //
    Data32 = IoRead32 (mAcpiBaseAddr + R_SMI_EN); 
    Data32 &= ~B_SMI_EN_LEGACY_USB3;
    IoWrite32 (mAcpiBaseAddr + R_SMI_EN, Data32); 
  }

  if (ConfigType == USB_CONFIG_READY_TO_SHUTDOWN) { // Ready to Shutdown
    //
    // Enable XHCI PME
    //
    Data32 = MmioRead32 (XhciPciBase + R_XHCI_PWR_CNTL_STS);
    Data32 |= B_XHCI_PWR_CNTL_STS_PME_EN;
    MmioWrite32 (XhciPciBase + R_XHCI_PWR_CNTL_STS, Data32);

    UsbMmio = MmioRead32 (XhciPciBase + R_XHCI_MEM_BASE) & B_XHCI_MEM_BASE_BA;

    //
    // Suspend XHCI port
    //
    for (Index = 0; Index < 8; Index++) {
      PortScx = MmioRead32 (UsbMmio + R_XHCI_PORTSC01USB2 + Index * 0x10);
      DEBUG ((EFI_D_INFO, "Index = %x, PortScx = %x\n", Index, PortScx));
      if (PortScx & (B_XHCI_PORTSCXUSB2_PED | B_XHCI_PORTSCXUSB2_CCS)) {
        PortScx &= ~B_XHCI_PORTSCXUSB2_PED;
        PortScx |= (BIT6 | BIT5) | B_XHCI_PORTSCXUSB2_LWS; // Put to U3
        MmioWrite32 (UsbMmio + R_XHCI_PORTSC01USB2 + Index * 0x10, PortScx);
      }
    }
    //
    // Clear XHCI PME Status
    //
    Data32 = MmioRead32 (XhciPciBase + R_XHCI_PWR_CNTL_STS);
    Data32 |= B_XHCI_PWR_CNTL_STS_PME_STS;
    MmioWrite32 (XhciPciBase + R_XHCI_PWR_CNTL_STS, Data32);
    
    //
    // Put XHCI to D3
    //
    Data32 = MmioRead32 (XhciPciBase + R_XHCI_PWR_CNTL_STS);
    Data32 |= V_XHCI_PWR_CNTL_STS_PWR_STS_D3;
    MmioWrite32 (XhciPciBase + R_XHCI_PWR_CNTL_STS, Data32);
    
    //
    // Enable PME_B0 (Enable PCI device in Bus 0 to wakeup system)
    //
    Data32 = IoRead32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN); 
    Data32 |= B_ACPI_GPE0a_EN_XHCI_PME_EN;
    IoWrite32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, Data32); 
    //
    // Clear PME_B0 Status
    //
    Data32 = IoRead32 (mAcpiBaseAddr + R_ACPI_GPE0a_STS); 
    Data32 |= B_ACPI_GPE0a_STS_PME_B0;
    IoWrite32 (mAcpiBaseAddr + R_ACPI_GPE0a_STS, Data32); 
  }
}


EFI_STATUS
EFIAPI
PrepareToSleepS5Callback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
  )
{
  UINT8   Data8;
  
  Data8 = IoRead8 (SW_SMI_PORT + 1);
  if (Data8 == 0x01) {
    WakeOnUsbFromS5(USB_CONFIG_PREPARE_TO_SLEEP1);
  } else if (Data8 == 0x02){
    WakeOnUsbFromS5(USB_CONFIG_PREPARE_TO_SLEEP2);
  }
  IoWrite8 (SW_SMI_PORT + 1, 0);
  
  return EFI_SUCCESS;
}
//[-end-170626-IB07400880-add]//

//[-start-190314-IB07401090-add]//
#ifndef DISABLE_MSR_CTR_OVERRIDE_HANDLER
//
//Clearing the MMIO CFLUSH bit i.e 4th bit. Set to 0, error will get logged but not signalled. 
//
EFI_STATUS 
MsrOverrideCallback ( 
  IN  EFI_HANDLE                    DispatchHandle, 
  IN  CONST VOID                    *DispatchContext, 
  IN  OUT VOID                      *CommBuffer  OPTIONAL, 
  IN  UINTN                         *CommBufferSize  OPTIONAL 
  ) 
{ 
  UINT64           Msr; 

  Msr = AsmReadMsr64 (IA32_MC4_CTL); 
  Msr &= ~(MMIO_CFLUSH(4)); 
  AsmWriteMsr64 (IA32_MC4_CTL, Msr); 
  Msr = AsmReadMsr64 (IA32_MC4_CTL); 
  
  DEBUG ((DEBUG_INFO, " IA32_MC4_CTL : %x \n",Msr)); 
  return EFI_SUCCESS; 
} 
#endif
//[-end-190314-IB07401090-add]//

//[-start-160318-IB07400711-add]//

EFI_STATUS
Chipset2InitPlatformSmm (
  IN EFI_SMM_POWER_BUTTON_DISPATCH2_PROTOCOL   *PowerButtonDispatch,
  IN EFI_SMM_ICHN_DISPATCH_PROTOCOL            *IchnDispatch,
  IN EFI_SMM_SW_DISPATCH2_PROTOCOL             *SwDispatch,
  IN EFI_SMM_SX_DISPATCH2_PROTOCOL             *SxDispatch
  )
{   
//[-start-170626-IB07400880-add]//
  UINT8                         CmosData;
//[-end-170626-IB07400880-add]//
//[-start-170314-IB07400847-add]//
  EFI_SMM_SW_REGISTER_CONTEXT   SwContext2;
  EFI_HANDLE                    Handle;
  EFI_STATUS                    Status;
  
  //
  // Register the Chipset Misc SMI
  //
  SwContext2.SwSmiInputValue = CHIPSET_MISC_SW_SMI;
  Status = SwDispatch->Register (
    SwDispatch,
    ChipsetMiscSwSmiCallback,
    &SwContext2,
    &Handle
    );
  ASSERT_EFI_ERROR (Status);
//[-end-170314-IB07400847-add]//
//[-start-170626-IB07400880-add]//
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature); 
  if (mSystemConfiguration.S5WakeOnUsb) {
    //
    // Register S5 prepare to sleep SW SMI 
    //
    SwContext2.SwSmiInputValue = WAKE_ON_USB_FROM_S5_SMI;
    Status = SwDispatch->Register (
      SwDispatch,
      PrepareToSleepS5Callback,
      &SwContext2,
      &Handle
      );
    ASSERT_EFI_ERROR (Status);
    
    //
    // Enable S5WakeOnUsb
    //
    CmosData |= B_CMOS_S5_WAKE_ON_USB;
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData); 
    
  } else {
    //
    // Disable S5WakeOnUsb
    //
    CmosData &= ~B_CMOS_S5_WAKE_ON_USB;
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData); 
  }
//[-end-170626-IB07400880-add]//
//[-start-190314-IB07401090-add]//
#ifndef DISABLE_MSR_CTR_OVERRIDE_HANDLER
    // 
    // Register the MSR CTL override Handler 
    // 
    SwContext2.SwSmiInputValue = MSR_CTR_OVERRIDE_HANDLER; 
    Status = SwDispatch->Register ( 
      SwDispatch, 
      MsrOverrideCallback , 
      &SwContext2, 
      &Handle 
      ); 
    ASSERT_EFI_ERROR (Status); 
#endif    
//[-end-190314-IB07401090-add]//

  return EFI_SUCCESS;
}
//[-end-160318-IB07400711-add]//
//[-start-161006-IB07400792-add]//
#ifdef SMI_POWER_BUTTON_DEBUG

#include <Protocol/SmmCpuSaveState.h>
#include <Protocol/SmmCpu.h>

EFI_STATUS
ChipsetPowerButtonCallbackDebug (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
  )
{
  EFI_STATUS                    Status;
  UINTN                         Width;
  UINTN                         CpuIndex;
  STATIC EFI_SMM_CPU_STATE      CpuSaveState;
  EFI_SMM_CPU_PROTOCOL          *SmmCpu = NULL;
  EFI_SMM_SAVE_STATE_REGISTER   Register;

  Status = mSmst->SmmLocateProtocol (&gEfiSmmCpuProtocolGuid, NULL, (VOID**) &SmmCpu);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Status = EFI_SUCCESS;

  //
  // Search all CPU
  //
  for (CpuIndex = 0; CpuIndex < mSmst->NumberOfCpus; CpuIndex++) {
    CopyMem (&CpuSaveState, mSmst->CpuSaveState[CpuIndex], sizeof (EFI_SMM_CPU_STATE));
    
    Width = sizeof (UINT16);
    Register = EFI_SMM_SAVE_STATE_REGISTER_CS;
    SmmCpu->ReadSaveState (
               SmmCpu,
               Width,
               Register,
               CpuIndex,
#ifdef MDE_CPU_IA32
               &CpuSaveState.x86._CS
#else
               &CpuSaveState.x64._CS
#endif
               );
    
#ifdef MDE_CPU_IA32
    Width = sizeof (UINT32);
#else
    Width = sizeof (UINT64);
#endif
    Register = EFI_SMM_SAVE_STATE_REGISTER_RIP;
    SmmCpu->ReadSaveState (
               SmmCpu,
               Width,
               Register,
               CpuIndex,
#ifdef MDE_CPU_IA32
               &CpuSaveState.x86._EIP
#else
               &CpuSaveState.x64._RIP
#endif
               );
    
    Width = sizeof (UINT16);
    Register = EFI_SMM_SAVE_STATE_REGISTER_RAX;
    SmmCpu->ReadSaveState (
               SmmCpu,
               Width,
               Register,
               CpuIndex,
#ifdef MDE_CPU_IA32
               &CpuSaveState.x86._EAX
#else
               &CpuSaveState.x64._RAX
#endif
               );
    
    Register = EFI_SMM_SAVE_STATE_REGISTER_RDX;
    SmmCpu->ReadSaveState (
               SmmCpu,
               Width,
               Register,
               CpuIndex,
#ifdef MDE_CPU_IA32
               &CpuSaveState.x86._EDX
#else
               &CpuSaveState.x64._RDX
#endif
               );
    //
    // Dump CPU Stack
    //
    if (CpuIndex == 0) {
#ifdef MDE_CPU_IA32
      DEBUG ((EFI_D_ERROR, "CpuIndex = %x, ", CpuIndex));
      DEBUG ((EFI_D_ERROR, "CS:EIP = 0x%04x:0x%016lx\n", CpuSaveState.x64._CS, CpuSaveState.x64._EIP));
#else
      DEBUG ((EFI_D_ERROR, "CpuIndex = %x, ", CpuIndex));
      DEBUG ((EFI_D_ERROR, "CS:RIP = 0x%04x:0x%016lx\n", CpuSaveState.x64._CS, CpuSaveState.x64._RIP));
#endif 
    } 

  }
#ifdef INSYDE_DEBUGGER
  __debugbreak();
#endif
  return Status;
}
#endif
//[-end-161006-IB07400792-add]//

/**
 Initializes the SMM Platfrom Driver

 @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
InitializePlatformSmm (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;
  BOOLEAN                                   InSmm;
  UINT8                                     Index;
  EFI_HANDLE                                Handle;
  EFI_SMM_POWER_BUTTON_DISPATCH2_PROTOCOL   *PowerButtonDispatch;
  EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT     PowerButtonContext;
  EFI_SMM_ICHN_DISPATCH_PROTOCOL            *IchnDispatch;
  EFI_SMM_ICHN_DISPATCH_CONTEXT             IchnContext;  
  EFI_SMM_SW_DISPATCH2_PROTOCOL             *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT               SwContext2;
  EFI_SMM_SX_DISPATCH2_PROTOCOL             *SxDispatch;
  EFI_SMM_SX_REGISTER_CONTEXT               EntryRegisterContext; 
  UINTN                                     VarSize;
  EFI_BOOT_MODE                             BootMode;
  VOID                                 *AcpiRestoreAcpiCallbackStartEvent;
  VOID                                 *AcpiRestoreAcpiCallbackDoneEvent;
  VOID                                 *AcpiEnableAcpiEvent;
  VOID                                 *AcpiDisableAcpiEvent;
  UINT32                               Data32And;
  UINT32                               Data32Or;
  VOID                                 *FullSystemConfig;
//[-start-160421-IB08450342-remove]//
  //UINTN                                     Size;
  //UINT8                                     BootType;
//[-end-160421-IB08450342-remove]//
  
  Handle = NULL;

  // Set default values for APM 1.2 states
  mAPMInterfaceConnectState = APM_NOT_CONNECTED;
  mAPMEnabledState = TRUE;
  mAPMEngagedState = TRUE;

  //
  // Initialize the EFI Runtime Library
  //

  Status = gBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, (VOID **)&mSmmBase);
  InSmm = FALSE;
  if (!EFI_ERROR (Status)) {
    mSmmBase->InSmm (mSmmBase, &InSmm);
  }

  //
  //  Locate the Global NVS Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &mGlobalNvsAreaPtr
                  );
//[-start-161026-IB07400805-modify]//
//  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    Handle = NULL;
    Status = gSmst->SmmInstallProtocolInterface (
      &Handle,
      &gEfiGlobalNvsAreaProtocolGuid,
      EFI_NATIVE_INTERFACE,
      mGlobalNvsAreaPtr
      );
    DEBUG ((EFI_D_ERROR, "InitializePlatformSmm: mGlobalNvsAreaPtr = %x\n", mGlobalNvsAreaPtr));
    DEBUG ((EFI_D_ERROR, "InitializePlatformSmm: mGlobalNvsAreaPtr->Area = %x\n", mGlobalNvsAreaPtr->Area));
    ASSERT_EFI_ERROR (Status);
  }
//[-end-161026-IB07400805-modify]//

    //
    // Initialize global variables
    //
    mSmst = gSmst;

    //
    // Get the ACPI Base Address
    //

    mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

//[-start-160421-IB08450342-remove]//
    //BootType = DUAL_BOOT_TYPE;
    //Size = sizeof (UINT8);
    //Status = SystemTable->RuntimeServices->GetVariable (
    //                L"BootType",
    //                &gSystemConfigurationGuid,
    //                NULL,
    //                &Size,
    //                &BootType
    //                );
    //ASSERT_EFI_ERROR (Status);
//[-end-160421-IB08450342-remove]//

    VarSize = PcdGet32 (PcdSetupConfigSize);
    FullSystemConfig = AllocatePool (VarSize);
    ASSERT (FullSystemConfig != NULL);
    Status = SystemTable->RuntimeServices->GetVariable(
                                             SETUP_VARIABLE_NAME,
                                             &gSystemConfigurationGuid,
                                             NULL,
                                             &VarSize,
                                             FullSystemConfig
                                             );
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe ((CHIPSET_CONFIGURATION *)FullSystemConfig, sizeof (CHIPSET_CONFIGURATION));  
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Get setup variable default value failed! \n"));
      }
    }
    ASSERT (sizeof(CHIPSET_CONFIGURATION) <= VarSize);
    CopyMem (&mSystemConfiguration, FullSystemConfig, sizeof(CHIPSET_CONFIGURATION));
    FreePool (FullSystemConfig);
    if (!EFI_ERROR(Status)) {
      mAcLossVariable = mSystemConfiguration.StateAfterG3;

      //
      // If LAN is disabled, WOL function should be disabled too.
      //

//[-start-160429-IB03090426-add]//
//[-start-161102-IB07400807-remove]//
//      mWakeOnLanVariable = mSystemConfiguration.Wol;
//[-end-161102-IB07400807-remove]//
//[-end-160429-IB03090426-add]//

//[-start-170314-IB07400847-modify]//
      mWakeOnRtcVariable = mSystemConfiguration.WakeOnS5;
//[-end-170314-IB07400847-modify]//
    }

    BootMode = GetBootModeHob ();

    //
    // Get the Power Button protocol
    //
    Status = gSmst->SmmLocateProtocol(&gEfiSmmPowerButtonDispatch2ProtocolGuid,
                                      NULL,
                                      (VOID **) &PowerButtonDispatch);
    ASSERT_EFI_ERROR(Status);

    if (BootMode != BOOT_ON_FLASH_UPDATE) {
      //
      // Register for the power button event
      //
      PowerButtonContext.Phase = EfiPowerButtonEntry;
      Status = PowerButtonDispatch->Register(
                                       PowerButtonDispatch,
//[-start-161006-IB07400792-modify]//
#ifdef SMI_POWER_BUTTON_DEBUG
                                       ChipsetPowerButtonCallbackDebug,
#else
                                       ChipsetPowerButtonCallback,
#endif                                       
//[-end-161006-IB07400792-modify]//
                                       &PowerButtonContext,
                                       &Handle
                                       );
      ASSERT_EFI_ERROR(Status);
    }

    //
    // Get the Sx dispatch protocol
    //
    Status = gSmst->SmmLocateProtocol (&gEfiSmmSxDispatch2ProtocolGuid,
                                       NULL,
                                       (VOID **) &SxDispatch
                                       );
    ASSERT_EFI_ERROR(Status);

    //
    // Register entry phase call back function
    //
    EntryRegisterContext.Type  = SxS3;
    EntryRegisterContext.Phase = SxEntry;

    Status = SxDispatch->Register (
                           SxDispatch,
                           S3SleepEntryCallBack,
                           &EntryRegisterContext,
                           &Handle
                           );


    EntryRegisterContext.Type  = SxS4;
    Status = SxDispatch->Register (
                           SxDispatch,
                           S4SleepEntryCallBack,
                           &EntryRegisterContext,
                           &Handle
                           );
    ASSERT_EFI_ERROR(Status);


    EntryRegisterContext.Type  = SxS5;
    Status = SxDispatch->Register (
                           SxDispatch,
                           S5SleepEntryCallBack,
                           &EntryRegisterContext,
                           &Handle
                           );
    ASSERT_EFI_ERROR(Status);

    Status = SxDispatch->Register (
                           SxDispatch,
                           S5SleepAcLossCallBack,
                           &EntryRegisterContext,
                           &Handle
                           );
    ASSERT_EFI_ERROR(Status);
#if 0
#if (TABLET_PF_ENABLE == 1)
    Status = SxDispatch->Register (
                           SxDispatch,
                           S5SleepCapsuleCallBack,
                           &EntryRegisterContext,
                           &Handle
                           );
    ASSERT_EFI_ERROR(Status);
#endif
#endif
    //
    //  Get the Sw dispatch protocol
    //
    Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid,
                                       NULL,
                                       (VOID **) &SwDispatch
                                       );
    ASSERT_EFI_ERROR(Status);

    //
    // Register the TPM PTS & SMBS Handler
    //
    SwContext2.SwSmiInputValue = TPM_PTS;
    Status = SwDispatch->Register (
                            SwDispatch,
                            TpmPtsSmbsCallback,
                            &SwContext2,
                            &Handle
                            );

    ASSERT_EFI_ERROR (Status);

    // Get the ICHn protocol
    //
    Status = gSmst->SmmLocateProtocol(&gEfiSmmIchnDispatchProtocolGuid,
                                      NULL,
                                      (VOID **) &IchnDispatch);
    ASSERT_EFI_ERROR(Status);

//[-start-161101-IB03090435-remove]//
//    //
//    // Register for the ECC event.
//    //
//#if defined( ECC_SUPPORT ) && ( ECC_SUPPORT != 0 )
//    IchnContext.Type = IchnMch;
//    Status = IchnDispatch->Register(
//                              IchnDispatch,
//                              EccCallback,
//                              &IchnContext,
//                              &Handle
//                              );
//    ASSERT_EFI_ERROR( Status );
//#endif
//[-end-161101-IB03090435-remove]//

//[-start-161230-IB07400829-remove]//
//    //
//    // Register for the Watchdog timer (TCO Timeout).
//    //
//    IchnContext.Type = IchnTcoTimeout;
//[-end-161230-IB07400829-remove]//
//[-start-160616-IB07250254-remove]//
//#if RVVP_ENABLE == 0
//#ifdef CRASHDUMP_SUPPORT
//    if (mSystemConfiguration.OsSelection == 1) {  // AOS
//#endif
//      Status = IchnDispatch->Register(
//                               IchnDispatch,
//                               WatchdogCallback,
//                               &IchnContext,
//                               &Handle
//                               );
//#ifdef CRASHDUMP_SUPPORT
//    } else {
//      Status = IchnDispatch->Register(
//                               IchnDispatch,
//                               EnableWatchdogCallback,
//                               &IchnContext,
//                               &Handle
//                               );
//    }
//#endif
//    ASSERT_EFI_ERROR( Status );
//#endif
//[-end-160616-IB07250254-remove]//
    Status = mSmst->SmmLocateProtocol (
                          &gH2OSmmChipsetServicesProtocolGuid,
                          NULL,
                          (VOID **)&mSmmChipsetSvc
                          );
    ASSERT_EFI_ERROR (Status);

    //
    //Register the OS restart set features disable
    //
    SwContext2.SwSmiInputValue = SMM_OS_RESET_SMI_VALUE;
    Status = SwDispatch->Register (
      SwDispatch,
      OsResetSmi,
      &SwContext2,
      &Handle
      );
    ASSERT_EFI_ERROR (Status);

    AcpiRestoreAcpiCallbackStartEvent = NULL;
    Status = mSmst->SmmRegisterProtocolNotify (
                &gAcpiRestoreCallbackStartProtocolGuid,
                PlatformRestoreAcpiCallbackStart,
                &AcpiRestoreAcpiCallbackStartEvent
                );  
    ASSERT_EFI_ERROR (Status);

    AcpiRestoreAcpiCallbackDoneEvent = NULL;
    Status = mSmst->SmmRegisterProtocolNotify (
                &gAcpiRestoreCallbackDoneProtocolGuid,
                PlatformRestoreAcpiCallbackDone,
                &AcpiRestoreAcpiCallbackDoneEvent
                );  
    ASSERT_EFI_ERROR (Status);

    //
    // Register ACPI enable handler
    //
//[-start-160421-IB08450342-modify]//
    if (FeaturePcdGet(PcdEnableSmiEnableAcpi) || (mSystemConfiguration.BootType == LEGACY_BOOT_TYPE)) {
//[-end-160421-IB08450342-modify]//
      AcpiEnableAcpiEvent = NULL;
      Status = mSmst->SmmRegisterProtocolNotify (
                  &gAcpiEnableCallbackStartProtocolGuid,
                  EnableAcpiCallback,
                  &AcpiEnableAcpiEvent
                  );  
      ASSERT_EFI_ERROR (Status);
      //
      // Register ACPI disable handler
      //
      AcpiDisableAcpiEvent = NULL;
      Status = mSmst->SmmRegisterProtocolNotify (
                  &gAcpiDisableCallbackStartProtocolGuid,
                  DisableAcpiCallback,
                  &AcpiDisableAcpiEvent
                  );  
      ASSERT_EFI_ERROR (Status);
    }

    //
    // Register for the events that may happen that we do not care.
    // This is true for SMI related to TCO since TCO is enabled by BIOS WP
    //
#if RVVP_ENABLE == 0
    for (Index = 0; Index < sizeof(mTco1Sources)/sizeof(UINT8); Index++) {
      IchnContext.Type = mTco1Sources[Index];
      Status = IchnDispatch->Register(
                                IchnDispatch,
                                DummyTco1Callback,
                                &IchnContext,
                                &Handle
                                );
      ASSERT_EFI_ERROR( Status );
    }
#endif

    //
    // Lock TCO_EN bit.
    //
#if RVVP_ENABLE ==0
//[-start-160817-IB03090432-modify]//
//[-start-161230-IB07400829-remove]//
    //
    // Lock TCO by SCU setting, configure in ScInit.c ScOnEndOfDxe()
    //
//    IoWrite16 (mAcpiBaseAddr + R_TCO1_CNT, IoRead16 (mAcpiBaseAddr + R_TCO1_CNT) | B_TCO1_CNT_LOCK);
//[-end-161230-IB07400829-remove]//
//[-end-160817-IB03090432-modify]//
#endif

    //
    // Set to power on from G3 dependent on WOL instead of AC Loss variable in order to support WOL from G3 feature.
    //
    //
    // Set wake from G3 dependent on AC Loss variable and Wake On LAN variable.
    // This is because no matter how, if WOL enabled or AC Loss variable not disabled, the board needs to wake from G3 to program the LAN WOL settings.
    // This needs to be done after LAN enable/disable so that the PWR_FLR state clear not impacted the WOL from G3 feature.
    //
//[-start-170314-IB07400847-modify]//
    if (mAcLossVariable == 0x01) {
      SetAfterG3On (TRUE);
    } else {
      SetAfterG3On (FALSE);
    }
//[-end-170314-IB07400847-modify]//

#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
  //
  // Enable SW SMI and Global SMI for S3 resume.
  // This enables SMI and SW SMI as early as possible for S3 resume to prevent from missing SW SMI.
  //
  Data32And = 0xFFFFFFFF;
  Data32Or = (B_SMI_EN_APMC | B_SMI_EN_GBL_SMI);
  S3BootScriptSaveIoReadWrite (
    S3BootScriptWidthUint32,
    (UINTN)(mAcpiBaseAddr + R_SMI_EN),
    &Data32Or,
    &Data32And
    );
#endif

//[-start-160318-IB07400711-add]//
  Chipset2InitPlatformSmm (
    PowerButtonDispatch,
    IchnDispatch,
    SwDispatch,
    SxDispatch
    );
//[-end-160318-IB07400711-add]//

  return EFI_SUCCESS;
}

EFI_STATUS
SmmReadyToBootCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  )
{
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;
  
  if (mSetSmmVariableProtocolSmiAllowed)
  {
    // It is okay to use gBS->LocateProtocol here because
    // we are still in trusted execution.
  Status = gSmst->SmmLocateProtocol(&gEfiSmmVariableProtocolGuid,
                                    NULL,
                                    (VOID **) &mSmmVariable);

    ASSERT_EFI_ERROR(Status);

    // mSetSmmVariableProtocolSmiAllowed will prevent this function from
    // being executed more than 1 time.
    mSetSmmVariableProtocolSmiAllowed = FALSE;
  }
  return Status;
}

EFI_STATUS
S3SleepEntryCallBack (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  )
/*++

Routine Description:

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering

  DispatchContext - The predefined context which contained sleep type and phase

Returns:

  EFI_SUCCESS           - Operation successfully performed

--*/
{
  EFI_STATUS              Status;
//[-start-161110-IB07400810-modify]//
//[-start-170529-IB07400873-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  UINT32                  Data32;
#endif  
#ifdef SIC_GPIO12_EXAMPLE_CODE
  UINT8                   BoardId;
#endif  
//[-end-170529-IB07400873-modify]//
//[-end-161110-IB07400810-modify]//

  Status = EFI_SUCCESS;
  
//[-start-160609-IB07400742-add]//
  POST_CODE ((SMM_S3_SLEEP_CALLBACK));
  DEBUG ((EFI_D_INFO, "S3SleepEntryCallBack!!\n"));
//[-end-160609-IB07400742-add]//

  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdProgressCodeS3SuspendStart));
  
  if (!FeaturePcdGet(PcdEnableSmiEnableAcpi)) {
    SaveRestoreState (FALSE);
  }
  
//[-start-161109-IB07400810-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  if (!IsIOTGBoardIds()) { // for RVP
//[-start-161019-IB07220143-add]//
    //
    // Set GPIO_22 to low, SATA port0 drive power pin.
    //
    Data32 = MmioRead32(PcdGet32(PcdP2SBBaseAddress) + N_GPIO_22);
    Data32 = Data32 & ~(0x0100);
    Data32 = Data32 & ~(0x1);
    MmioWrite32(PcdGet32(PcdP2SBBaseAddress) + N_GPIO_22, Data32);
//[-end-161019-IB07220143-add]//
  
    //
    // Set GPIO_35 to low
    //
    Data32 = MmioRead32(PcdGet32(PcdP2SBBaseAddress) + N_GPIO_35);
    Data32 = Data32 & ~(0x0100);
    Data32 = Data32 & ~(0x1);
    MmioWrite32(PcdGet32(PcdP2SBBaseAddress) + N_GPIO_35, Data32);
  }
#endif  
//[-end-161109-IB07400810-modify]//
  
  //
  // Enable GPIO TIER1 SCI EN bit for LID wake function.
  //
  IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, B_ACPI_GPE0a_EN_GPIO_TIER1_SCI_EN);

//[-start-170410-IB07400857-modify]//
//[-start-170529-IB07400873-modify]//
//#if BXTI_PF_ENABLE
#ifdef SIC_GPIO12_EXAMPLE_CODE
//[-end-170529-IB07400873-modify]//
  //
  // Enable GPIO TIER1 SCI EN bit for GPE0b_EN register.
  //
  BoardId = MultiPlatformGetBoardIds();
  if ( (BoardId == BOARD_ID_LFH_CRB) || (BoardId == BOARD_ID_OXH_CRB) || (BoardId == BOARD_ID_JNH_CRB) ) {
      IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0b_EN, B_ACPI_GPE0b_EN_GPIO_TIER1_SCI_EN);
      DEBUG ((DEBUG_INFO, "GPE0b : 0x%08x \n", R_ACPI_GPE0b_EN));
  }
#endif
//[-end-170410-IB07400857-modify]//
//[-start-160429-IB03090426-add]//
//[-start-161102-IB07400807-remove]//
//  //
//  // Enable PCIE WAKE EN bit if WOL feature is enabled
//  //
//  if(mWakeOnLanVariable) {
//	  IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, B_ACPI_GPE0a_EN_PCIE_WAKE0_EN);
//	  IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, B_ACPI_GPE0a_EN_PCIE_WAKE1_EN);
//	  IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, B_ACPI_GPE0a_EN_PCIE_WAKE2_EN);
//  }
//[-end-161102-IB07400807-remove]//
//[-end-160429-IB03090426-add]//
//[-start-161118-IB07400817-add]//
  //
  // If Wake On PME is disabled, disable all PCIE Wake Event (S3)
  //
  if ((!mSystemConfiguration.WakeOnPME)) {
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE0_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE1_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE2_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE3_EN));
  }
//[-start-171115-IB07400928-add]//
  else if (mSystemConfiguration.WakeOnPME == SCU_WAKE_ON_PME_FORCE_ENALBED) {
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE0_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE1_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE2_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE3_EN);
  }
//[-end-171115-IB07400928-add]//
//[-end-161118-IB07400817-add]//
  //
  // Workaround for S3 wake hang if C State is enabled
  //
  CpuSmmSxWorkAround();
  
//[-start-160421-IB10860195-add]//
  S3SaveRuntimeScriptTable();
//[-end-160421-IB10860195-add]//

//[-start-160824-IB07220130-add]//
  WakeToProcessPendingCapsule (mAcpiBaseAddr, FixedPcdGet16 (PcdSecureFlashWakeFromS3Time));
//[-end-160824-IB07220130-add]//
//[-start-160609-IB07400742-add]//
  //
  // OemServices
  //
  Status = OemSvcS3Callback ();
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcKernelLib S3Callback, Status : %r\n", Status ) );
//[-end-160609-IB07400742-add]//
  return Status;
}

VOID
CpuSmmSxWorkAround(
  )
{
  UINT64           MsrValue;

  MsrValue = AsmReadMsr64 (MSR_PMG_CST_CONFIG);

  if (MsrValue & B_EFI_MSR_PMG_CST_CONFIG_CST_CONTROL_LOCK) {
    //
    // Cannot do anything if the register is locked.
    //
    return;
  }

  if (MsrValue & B_EFI_MSR_PMG_CST_CONFIG_IO_MWAIT_REDIRECTION_ENABLE) {
    //
    // If C State enabled, disable it before going into S3
    // The MSR will be restored back during S3 wake
    //
    MsrValue &= ~B_EFI_MSR_PMG_CST_CONFIG_IO_MWAIT_REDIRECTION_ENABLE;
    AsmWriteMsr64 (MSR_PMG_CST_CONFIG, MsrValue);
  }
}

VOID
ClearP2PBusMaster(
  )
{
  UINT8             Command;
  UINT8             Index;

  for (Index = 0; Index < sizeof(mPciBm)/sizeof(EFI_PCI_BUS_MASTER); Index++) {
    Command = MmioRead8 (
                MmPciAddress (0,
                  DEFAULT_PCI_BUS_NUMBER_SC,
                  mPciBm[Index].Device,
                  mPciBm[Index].Function,
                  PCI_COMMAND_OFFSET
                )
              );
    Command &= ~EFI_PCI_COMMAND_BUS_MASTER;
    MmioWrite8 (
      MmPciAddress (0,
        DEFAULT_PCI_BUS_NUMBER_SC,
        mPciBm[Index].Device,
        mPciBm[Index].Function,
        PCI_COMMAND_OFFSET
      ),
      Command
    );
  }
}

VOID
SetAfterG3On (
  BOOLEAN Enable
  )
/*++

Routine Description:

  Set the AC Loss to turn on or off.

Arguments:

Returns:

  None

--*/
{
  UINT8             PmCon1;

  //
  // SC handling portion
  //
  PmCon1 = MmioRead8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  PmCon1 &= ~B_PMC_GEN_PMCON_AFTERG3_EN;
  if (Enable) {
    PmCon1 |= B_PMC_GEN_PMCON_AFTERG3_EN;
  }
  MmioWrite8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, PmCon1);
}

/**
 When a power button event happens, it shuts off the machine.

 @param [in]   DispatchHandle   Handle of this dispatch function
 @param [in]   DispatchContext  Pointer to the dispatch function's context

 @retval       None

**/
EFI_STATUS
ChipsetPowerButtonCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                              *DispatchContext,
  IN  OUT VOID                                *CommBuffer  OPTIONAL,
  IN  UINTN                                   *CommBufferSize  OPTIONAL
  )
{
  //
  // Check what the state to return to after AC Loss. If Last State, then
  // set it to Off.
  //
  UINT16                     data16;
  EFI_STATUS                 Status;
  Status = EFI_SUCCESS;

//[-start-160324-IB07400711-add]//
  DEBUG ((EFI_D_ERROR, "ChipsetPowerButtonCallback!!\n"));
//[-end-160324-IB07400711-add]//
//[-start-170314-IB07400847-modify]//
  if (mWakeOnRtcVariable != SCU_S5_WAKE_ON_RTC_DISABLED) {
    EnableS5WakeOnRtc();
  }

  if (mAcLossVariable == 2) {
    SetAfterG3On (TRUE);
  }
//[-end-170314-IB07400847-modify]//
  ClearP2PBusMaster();

  data16 = (UINT16)(IoRead16(mAcpiBaseAddr + R_ACPI_GPE0a_EN));
  data16 &= B_ACPI_GPE0a_EN_PCIE_GPE_EN;
  
//[-start-170626-IB07400880-add]//
  if (mSystemConfiguration.S5WakeOnUsb) {
    WakeOnUsbFromS5(USB_CONFIG_READY_TO_SHUTDOWN);
  }
//[-end-170626-IB07400880-add]//
//[-start-171115-IB07400928-add]//
  //
  // If Wake On PME is disabled, disable all PCIE Wake Event (Power button)
  //
  if ((!mSystemConfiguration.WakeOnPME)) {
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE0_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE1_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE2_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE3_EN));
  } 
  else if (mSystemConfiguration.WakeOnPME == SCU_WAKE_ON_PME_FORCE_ENALBED) {
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE0_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE1_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE2_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE3_EN);
  }
//[-end-171115-IB07400928-add]//

  Status = OemSvcPowerButtonCallback ();

  //
  // Clear Sleep SMI Status
  //
  IoWrite16 (
    mAcpiBaseAddr + R_SMI_STS,
    (UINT16)(IoRead16 (mAcpiBaseAddr + R_SMI_STS) | B_SMI_STS_ON_SLP_EN)
    );

  //
  // Clear Sleep Type Enable
  //
  IoWrite16 (
    mAcpiBaseAddr + R_SMI_EN,
    (UINT16)(IoRead16 (mAcpiBaseAddr + R_SMI_EN) & (~B_SMI_EN_ON_SLP_EN))
    );

  //
  // Clear Power Button Status
  //
  IoWrite16 (mAcpiBaseAddr + R_ACPI_PM1_STS, B_ACPI_PM1_STS_PWRBTN);

  //
  // Shut it off now!
  //
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5);
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_CNT, B_ACPI_PM1_CNT_SLP_EN | V_ACPI_PM1_CNT_S5);

  //
  // Should not return
  //
  CpuDeadLoop();
  return EFI_SUCCESS;
}

VOID
PmeCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{

}

//[-start-161101-IB03090435-remove]//
//#if defined( ECC_SUPPORT ) && ( ECC_SUPPORT != 0 )
//VOID
//EccCallback (
//  IN  EFI_HANDLE                              DispatchHandle,
//  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
//  )
///*++
//
//Routine Description:
//
//Arguments:
//
//Returns:
//
//  None
//
//--*/
//{
//  EFI_STATUS            Status;
//  UINT8                 Index;
//  EFI_TIME              CurrentTime;
//  UINTN                 Size;
//  CHIPSET_CONFIGURATION  SystemConfiguration;
//  UINT16                EccSmiCmd;
//  UINT16                EccErrSts;
//  EFI_EVENT_TYPE        EventType;
//  UINT32                MemoryChannel;
//  EFI_EVENT_LOG_STORE   mCurrentEventLogStore[] = {
//  { (UINT8) EfiProcessorThermalTrip,  0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMultiBitECCErrorChA,   0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiSingleBitECCErrorChA,  0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiCmosBatteryFailure,    0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiCmosChecksumError,     0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiCmosTimerNotSet,       0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiKeyboardNotFound,      0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMemorySizeDecrease,    0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiIntruderDetection,     0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiSpdTolerant,           0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMemOptiDual,           0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMemOptiSingle,         0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMultiBitECCErrorChB,   0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiSingleBitECCErrorChB,  0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMeCh0Err,              0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMeInitFailure,         0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMeHeciCommError,       0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiHeciInitError,         0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiAtaSmartError,         0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiAaNumberError,         0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F },
//  { (UINT8) EfiMeEOPMsgError,         0xFFFFFFFF,  0x3F, 0xF, 0x1F, 0x1F, 0x3F, 0x3F }
// };
//
//  EccSmiCmd = McD0PciCfg16( MC_REG_SMICMD );
//  //
//  // Check if SMI generation for ECC errors enabled
//  //
//  if(!((EccSmiCmd & BIT0) || (EccSmiCmd & BIT1))) {
//    return;
//  }
//
//  EccErrSts = McD0PciCfg16( MC_REG_ERRSTS );
//  //
//  // Check if ECC errors generated
//  //
//  if(!((EccErrSts & BIT0) || (EccErrSts & BIT1))) {
//    return;
//  }
//
//  //
//  // Get memory channel where ECC error being generated
//  //
//  MemoryChannel = McD0PciCfg32 ( MC_REG_DEAP );
//
//  //
//  // If both single- and multi-bit ECC errors generated at the same time,
//  // then handle multi-bit ECC error first
//  //
//  if (EccErrSts & BIT1) {
//    //
//    // Multi-bit ECC error
//    //
//    EventType = (MemoryChannel & BIT0) ? EfiMultiBitECCErrorChB : EfiMultiBitECCErrorChA;
//  }
//  else {
//    //
//    // Single-bit ECC error
//    //
//    EventType = (MemoryChannel & BIT0) ? EfiSingleBitECCErrorChB : EfiSingleBitECCErrorChA;
//  }
//
//  //
//  // Make sure EFI_SMM_VARIABLE_PROTOCOL is available
//  //
//  if (!mSmmVariable) {
//    //
//    // Clear ECC error status
//    //
//    ClearEccErrSts(EventType);
//    return;
//  }
//
//  Size = sizeof(CHIPSET_CONFIGURATION);
//  //
//  // Retrieve the Setup Configuration Variable from NVRAM
//  //
//  Status = mSmmVariable->SmmGetVariable (gEfiNormalSetupName,
//                             &gEfiNormalSetupGuid,
//                             NULL,
//                             &Size,
//                             &SystemConfiguration
//                             );
//
//  if (EFI_ERROR(Status) || SystemConfiguration.EventLogging == 0 ||
//      SystemConfiguration.EccEventLogging == 0) {
//    //
//    // Clear ECC error status
//    //
//    ClearEccErrSts(EventType);
//    return;
//  }
//
//  //
//  // Get the current state of Events variable
//  //
//  Size = sizeof(mCurrentEventLogStore);
//  Status = mSmmVariable->SmmGetVariable ((CHAR16 *)&gEventName,
//                             &gEventNameGuid,
//                             NULL,
//                             &Size,
//                             &mCurrentEventLogStore
//                             );
//  //
//  // Continue even if the Events variable does not exist; we will
//  // still use the default state of Events variable
//  //
//
//  for (Index = 0; Index < EfiMaximumEventType; Index++) {
//    //
//    // Find the event type entry in the local buffer
//    //
//    if (mCurrentEventLogStore[Index].Type == (UINT8) EventType) {
//      //
//      // Check if full
//      //
//      if (mCurrentEventLogStore[Index].Count == 0) {
//        break;
//      }
//
//      //
//      // Increment the Count by 1
//      //
//      mCurrentEventLogStore[Index].Count <<= 1;
//
//      //
//      // Create a time if this is the first time
//      //
//      if (mCurrentEventLogStore[Index].Time.Month == 0xF) {
//        EfiSmmGetTime(&CurrentTime);
//        if (CurrentTime.Year >= 2005) {
//          mCurrentEventLogStore[Index].Time.Year    = (CurrentTime.Year - 2005);
//          mCurrentEventLogStore[Index].Time.Month   = CurrentTime.Month;
//          mCurrentEventLogStore[Index].Time.Day     = CurrentTime.Day;
//          mCurrentEventLogStore[Index].Time.Hour    = CurrentTime.Hour;
//          mCurrentEventLogStore[Index].Time.Minute  = CurrentTime.Minute;
//          mCurrentEventLogStore[Index].Time.Second  = CurrentTime.Second;
//        }
//      }
//
//      //
//      // Set the event in the storage
//      //
//      Size = sizeof(mCurrentEventLogStore);
//      Status = mSmmVariable->SetVariable ((CHAR16 *)&gEventName,
//                                 &gEventNameGuid,
//                                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
//                                 Size,
//                                 &mCurrentEventLogStore
//                                 );
//      break;
//    }
//  }
//
//  //
//  // Clear ECC error status
//  //
//  ClearEccErrSts(EventType);
//}
//
//VOID
//ClearEccErrSts (
//  IN  EFI_EVENT_TYPE  EventType
//  )
///*++
//
//Routine Description:
//
//Arguments:
//
//Returns:
//
//  None
//
//--*/
//{
//  if (EventType == EfiMultiBitECCErrorChA || EventType == EfiMultiBitECCErrorChB) {
//    McD0PciCfg16And( MC_REG_ERRSTS, BIT1);
//
//    //
//    // For multi-bit ECC error, clear Port 70h bit 7 to enable all NMI sources
//    //
//    IoWrite8( 0x70, IoRead8( 0x74 ) & ~BIT7 );     // Port 70h is aliased to port 74h
//
//    //
//    // Now, generate an NMI (clear NMI2SMI_EN bit and then set NMI_NOW bit)
//    //
//    IoWrite16( mAcpiBaseAddr + R_TCO1_CNT, IoRead16( mAcpiBaseAddr + R_TCO1_CNT ) & ~B_NMI2SMI_EN );
//    IoWrite16( mAcpiBaseAddr + R_TCO1_CNT, IoRead16( mAcpiBaseAddr + R_TCO1_CNT ) | B_NMI_NOW );
//  }
//  else {
//    McD0PciCfg16And( MC_REG_ERRSTS, BIT0);
//  }
//}
//#endif
//[-end-161102-IB03090435-remove]//

VOID
EFIAPI
WatchdogCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  )
/*++

Routine Description:

  When a watchdog event happens, clear status & timeout status but do not
  restart timer.

Arguments:

Returns:

  None

--*/
{
  //
  // Clear SMI status
  //
  IoWrite16 (mAcpiBaseAddr + R_SMI_STS, B_SMI_STS_TCO);

//[-start-160817-IB03090432-modify]//
  if (!(IoRead16 (mAcpiBaseAddr + R_TCO1_CNT) & (B_TCO1_CNT_TMR_HLT))) {
//[-end-160817-IB03090432-modify]//

    //
    // Clear NMI2SMI_EN before setting NMI_NOW
    //
    SideBandAndThenOr32(
      SB_ITSS_PORT,
      NMI_REG_OFFSET,
      0xFFFFFFFB, // MI2SMI_EN
      0
      );
    SideBandAndThenOr32(
      SB_ITSS_PORT,
      NMI_REG_OFFSET,
      0xFFFFFFFF,
      NMI_NOW
      );

  } else {
    IoWrite16 (mAcpiBaseAddr + R_TCO_TMR, 4);
    //
    // Wait for next timeout
    //
    CpuDeadLoop ();
  }
}

EFI_STATUS
S5SleepAcLossCallBack (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  )
/*++

Routine Description:

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering

  DispatchContext - The predefined context which contained sleep type and phase

Returns:

--*/
{
  EFI_STATUS            Status;
  Status = EFI_SUCCESS;
  
  //
  // Check what the state to return to after AC Loss. If Last State, then
  // set it to Off.
  //
//[-start-170314-IB07400847-modify]//
  if (mAcLossVariable == 2) {
    SetAfterG3On (TRUE);
  }
//[-end-170314-IB07400847-modify]//
  //
  // OemServices
  //
  Status = OemSvcS5AcLossCallback ();
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcKernelLib S5AcLossCallback, Status : %r\n", Status ) );
  return Status;
}

EFI_STATUS
S5SleepCapsuleCallBack (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  )
/*++

Routine Description:

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering

  DispatchContext - The predefined context which contained sleep type and phase

Returns:

--*/
{
  EFI_STATUS            Status;
  UINTN                 CapsuleDataPtr;
  UINT8                 CmosData;
  UINT8                 CmosAddress;
  UINT32                Pm1Cnt;
  UINT32                TempData32;
  UINT8                 i;
  UINT8                 WakeupHour;
  UINT8                 WakeupMinute;
  UINT8                 WakeupSecond;
  BOOLEAN               MinuteIncrement;
  BOOLEAN               HourIncrement;
  BOOLEAN               UseRtcWorkaround;
  DMI_DATA              DmiDataVariable;
  UINTN                 Size;
  CHAR8                 BoardAaNumber[6];
  CHAR8                 BoardFabNumber[3];
  UINT8                 CurrentBoardFabNumber;
  BOOLEAN               FoundAANum;
  UINTN                 BoardIdIndex;

  CapsuleDataPtr = 0;
  MinuteIncrement = FALSE;
  HourIncrement = FALSE;
  UseRtcWorkaround = FALSE;
  FoundAANum = FALSE;
  Status = EFI_SUCCESS;
  
  ClearP2PBusMaster();

  //
  // Check if it is due to a capsule update
  //
  for (CmosAddress = CmosCapsuleAddress4; CmosAddress >= CmosCapsuleAddress1; CmosAddress--) {
    CapsuleDataPtr <<= 8;
    IoWrite8(PCAT_CMOS_2_ADDRESS_REGISTER, CmosAddress);
    CmosData = IoRead8(PCAT_CMOS_2_DATA_REGISTER);
    CapsuleDataPtr += (UINTN)CmosData;
  }

  if (CapsuleDataPtr) {
    //
    // Get board AA# and FAB# to determine if we need to use RTC alarm workaround or GPIO bounce back method.
    //
    Size = sizeof (DMI_DATA);
    SetMem(&DmiDataVariable, Size, 0xFF);
    Status = mSmmVariable->SmmGetVariable (
               DMI_DATA_NAME,
               &gDmiDataGuid,
               NULL,
               &Size,
               &DmiDataVariable
               );
    if (!EFI_ERROR (Status)) {
      CopyMem(&BoardAaNumber, ((((UINT8*)&DmiDataVariable.BaseBoardVersion)+2)), 6);
      for (BoardIdIndex = 0; BoardIdIndex < DefectiveBoardIdTableSize; BoardIdIndex++) {
        if (AsciiStrnCmp(DefectiveBoardIdTable[BoardIdIndex].BoardAaNumber, BoardAaNumber, 6) == 0) {
          //
          // Check if it's older FAB. Do workaround for older FAB.
          //
          CopyMem(&BoardFabNumber, ((((UINT8*)&DmiDataVariable.BaseBoardVersion)+2+7)), 3);
          CurrentBoardFabNumber = ((BoardFabNumber[0]&0x0F)*100) + ((BoardFabNumber[1]&0x0F)*10) + ((BoardFabNumber[2]&0x0F)*1);
          if (CurrentBoardFabNumber <= DefectiveBoardIdTable[BoardIdIndex].BoardFabNumber) {
            UseRtcWorkaround = TRUE;
          }
          FoundAANum = TRUE;
          break;
        }
      }

      if(!FoundAANum) {
        //
        // Add check for AA#'s that is programmed without the AA as leading chars.
        //
        CopyMem(&BoardAaNumber, (((UINT8*)&DmiDataVariable.BaseBoardVersion)), 6);
        for (BoardIdIndex = 0; BoardIdIndex < DefectiveBoardIdTableSize; BoardIdIndex++) {
          if (AsciiStrnCmp(DefectiveBoardIdTable[BoardIdIndex].BoardAaNumber, BoardAaNumber, 6) == 0) {
            //
            // Check if it's older FAB. Do workaround for older FAB.
            //
            CopyMem(&BoardFabNumber, ((((UINT8*)&DmiDataVariable.BaseBoardVersion)+7)), 3);
            CurrentBoardFabNumber = ((BoardFabNumber[0]&0x0F)*100) + ((BoardFabNumber[1]&0x0F)*10) + ((BoardFabNumber[2]&0x0F)*1);
            if (CurrentBoardFabNumber <= DefectiveBoardIdTable[BoardIdIndex].BoardFabNumber) {
              UseRtcWorkaround = TRUE;
            }
            FoundAANum = TRUE;
            break;
          }
        }
      }

      if (FoundAANum != TRUE) {
        UseRtcWorkaround = TRUE;
      }
    }

    if (!UseRtcWorkaround) {
      //
      // Generate automatic wake up. by toggling a GPIO to generate SMI wake up event
      // Enable GPIO14 to generate wake event
      //
      TempData32 = IoRead32(mAcpiBaseAddr + R_ACPI_GPE0a_EN);
      TempData32 |= 0x40000000;
      IoWrite32(mAcpiBaseAddr + R_ACPI_GPE0a_EN, TempData32);

    } else {
      //
      // WORKAROUND: This is a workaround to hardware issue where board circuit is not designed to take care
      //             of the S3-Hot condition that chipset not supported.
      //

      //
      // Iflash will have priority on RTC so override it!!
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_B);
      IoWrite8(PCAT_RTC_DATA_REGISTER, (IoRead8(PCAT_RTC_DATA_REGISTER) & ~B_RTC_ALARM_INT_ENABLE));

      //
      // Set Date for everyday
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_D);
      CmosData = IoRead8(PCAT_RTC_DATA_REGISTER);
      CmosData &= ~(B_RTC_DATE_ALARM_MASK);
      for(i = 0 ; i < 0xffff ; i++){
        IoWrite8(PCAT_RTC_DATA_REGISTER, CmosData);
        Stall(1);
        if((IoRead8(PCAT_RTC_DATA_REGISTER) & B_RTC_DATE_ALARM_MASK) == 0){
          break;
        }
      }

      //
      // Set Second for current + 2s
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_SECOND);
      WakeupSecond = IoRead8(PCAT_RTC_DATA_REGISTER);
      WakeupSecond = BcdToHex(WakeupSecond);
      if(!mSystemConfiguration.FlashSleepDelay)
        WakeupSecond += 2;
      else
        WakeupSecond += 20;
      if (WakeupSecond >= 60) {
        WakeupSecond -= 60;
        MinuteIncrement = TRUE;
      }
      WakeupSecond = HexToBcd(WakeupSecond);
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_SECOND_ALARM);
      for(i = 0 ; i < 0xffff ; i++){
        IoWrite8(PCAT_RTC_DATA_REGISTER, WakeupSecond);
        Stall(1);
        if(IoRead8(PCAT_RTC_DATA_REGISTER) == WakeupSecond){
          break;
        }
      }

      //
      // Set Minute
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_MINUTE);
      WakeupMinute = IoRead8(PCAT_RTC_DATA_REGISTER);
      if (MinuteIncrement) {
        WakeupMinute = BcdToHex(WakeupMinute);
        WakeupMinute++;
        if (WakeupMinute >= 60) {
          WakeupMinute -= 60;
          HourIncrement = TRUE;
        }
        WakeupMinute = HexToBcd(WakeupMinute);
      }
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_MINUTE_ALARM);
      for(i = 0 ; i < 0xffff ; i++){
        IoWrite8(PCAT_RTC_DATA_REGISTER, WakeupMinute);
        Stall(1);
        if(IoRead8(PCAT_RTC_DATA_REGISTER) == WakeupMinute){
          break;
        }
      }

      //
      // Set Hour
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_HOUR);
      WakeupHour = IoRead8(PCAT_RTC_DATA_REGISTER);
      if (HourIncrement) {
        WakeupHour = BcdToHex(WakeupHour);
        WakeupHour++;
        if (WakeupHour >= 24) {
          WakeupHour -= 24;
        }
        WakeupHour = HexToBcd(WakeupHour);
      }
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_HOUR_ALARM);
      for(i = 0 ; i < 0xffff ; i++){
        IoWrite8(PCAT_RTC_DATA_REGISTER, WakeupHour);
        Stall(1);
        if(IoRead8(PCAT_RTC_DATA_REGISTER) == WakeupHour){
          break;
        }
      }

      //
      // Wait for UIP to arm RTC alarm
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_A);
      while (IoRead8(PCAT_RTC_DATA_REGISTER) & 0x80);

      //
      // Read RTC register 0C to clear pending RTC interrupts
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_C);
      IoRead8(PCAT_RTC_DATA_REGISTER);

      //
      // Enable RTC Alarm Interrupt
      //
      IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_B);
      IoWrite8(PCAT_RTC_DATA_REGISTER, IoRead8(PCAT_RTC_DATA_REGISTER) | B_RTC_ALARM_INT_ENABLE);

      //
      // Clear ICH RTC Status
      //
      IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_STS, B_ACPI_PM1_STS_RTC);

      //
      // Enable ICH RTC event
      //
      IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_EN,
                  (UINT16)(IoRead16(mAcpiBaseAddr + R_ACPI_PM1_EN) | B_ACPI_PM1_EN_RTC));

      //
      // Note: No need to disarm the RTC alarm interrupt after RTC event fired as this will be done at Platform DXE driver.
      //
    }

    //
    // Change the S5 to an S3
    //
    Pm1Cnt = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_CNT);
    Pm1Cnt &= (~0x1c00);
    Pm1Cnt |= 0x1400;
    IoWrite32(mAcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);
  }
  return EFI_SUCCESS;
}

/**
 CallBack before S4.

 @param [in]   DispatchHandle   The handle of this callback, obtained when registering
 @param [in]   DispatchContext  The predefined context which contained sleep type and phase

 @retval       None

**/
EFI_STATUS
S4SleepEntryCallBack (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST VOID                   *DispatchContext,
  IN  OUT VOID                     *CommBuffer  OPTIONAL,
  IN  UINTN                        *CommBufferSize  OPTIONAL
  )
{
  EFI_STATUS    Status;
  Status = EFI_SUCCESS;  
  
//[-start-160609-IB07400742-add]//
  POST_CODE ((SMM_S4_SLEEP_CALLBACK));
  DEBUG ((EFI_D_INFO, "S4SleepEntryCallBack!!\n"));
//[-end-160609-IB07400742-add]//

//[-start-161118-IB07400817-add]//
  //
  // If Wake On PME is disabled, disable all PCIE Wake Event (S4)
  //
  if ((!mSystemConfiguration.WakeOnPME)) {
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE0_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE1_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE2_EN));
	  IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE3_EN));
  }
//[-start-171115-IB07400928-add]//
  else if (mSystemConfiguration.WakeOnPME == SCU_WAKE_ON_PME_FORCE_ENALBED) {
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE0_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE1_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE2_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE3_EN);
  }
//[-end-171115-IB07400928-add]//
//[-end-161118-IB07400817-add]//
  
  //
  // Enable/Disable USB Charging
  //
  
  //
  // OemServices
  //
  Status = OemSvcS4Callback ();
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcKernelLib S4Callback, Status : %r\n", Status ) );

//[-start-170314-IB07400847-add]//
  if (mAcLossVariable == 2) {
    SetAfterG3On (TRUE);
  }
//[-end-170314-IB07400847-add]//
  return Status;
}

/**
 This function either writes to or read from global register table the data of
 Gpio, SVID, LevelEdge, Pci, CpuState, Mtrr, Sio, Kbc, and Pic registers.

 @param [in]   SaveRestoreFlag  True: write data to SMM IO registers.
                                False: read data from IO to global registers.

 @retval EFI_SUCCESS

**/
EFI_STATUS
SaveRestoreState (
  IN BOOLEAN                        SaveRestoreFlag
  )
{
  EFI_STATUS              Status;
  UINTN                   CpuIndex;
  
//[-start-160421-IB10860195-remove]//
//[-start-160411-IB10860194-add]//
// Status = SaveRestoreSmiEnable(SaveRestoreFlag);
//[-end-160411-IB10860194-add]//
//[-start-160421-IB10860195-remove]//

  Status = SaveRestorePMC (SaveRestoreFlag);

#if ENBDT_PF_ENABLE
  Status = SaveRestorePci (SaveRestoreFlag);
#endif

  Status = SaveRestoreMtrr (&SaveRestoreFlag);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (SaveRestoreFlag) {
    for (CpuIndex = 0; CpuIndex < mSmst->NumberOfCpus; CpuIndex++) {
      mSmst->SmmStartupThisAp (SaveRestoreMtrr, CpuIndex, &SaveRestoreFlag);
    }
  }

  return  EFI_SUCCESS;
}

//[-start-170104-IB07400829-remove]//
//
//typedef struct {
//  UINT8   RpDev;
//  UINT8   RpFun;
//} PCIE_ROOT_PORT_TABLE;
//
//PCIE_ROOT_PORT_TABLE mPcieRpTable[] = {
//  {PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1}, // (0/0x14/0)
//  {PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2}, // (0/0x14/1)
//  {PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3}, // (0/0x13/0)
//  {PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4}, // (0/0x13/1)
//  {PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5}, // (0/0x13/2)
//  {PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2, PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6}  // (0/0x13/3)
//};
//
//UINTN mPcieRpTableSize = sizeof (mPcieRpTable) / sizeof (PCIE_ROOT_PORT_TABLE);
//
//UINT32 
//FindPcieCapIdAddress (
//  IN UINT32  PcieBase,
//  IN UINT8   FindCapId
//  ) 
//{
//  UINT8 CapIdOffset;
//  UINT8 CapId;
//    
//  CapIdOffset = MmioRead8 (PcieBase + 0x34);
//  while (CapIdOffset != 0) {
//    CapId = MmioRead8 (PcieBase + CapIdOffset);
//    if (CapId == FindCapId) {
//      return (PcieBase + CapIdOffset);
//    }
//    CapIdOffset = MmioRead8 (PcieBase + CapIdOffset + 1);
//  }
//
//  return 0;
//}
//
//VOID
//S5WakeOnLanConfig (
//  VOID
//  )
//{
//  UINTN       Index;
//  UINT32      RootPortPcieBase;
//  UINTN       DevBus;
//  UINT32      DevPcieBase;
//  UINT32      RpPmRegAddress;
//  UINT32      RpPmReg;
//  UINT32      RpCmdReg;
//  UINT32      DevPmRegAddress;
//  UINT32      Data32;
//
//  for (Index = 0; Index < mPcieRpTableSize; Index ++) {
//    RootPortPcieBase = (UINT32)MmPciAddress(0, 0, mPcieRpTable[Index].RpDev, mPcieRpTable[Index].RpFun, 0);
//
//    //
//    // Check PCIe Root Port exist or not
//    //
//    if (MmioRead32(RootPortPcieBase) == 0xFFFFFFFF) {
//      continue;
//    }
//    
//    //
//    // Only support PCIe device behind the root port
//    //
//    DevBus = MmioRead8 (RootPortPcieBase + 0x19);
//    DevPcieBase = (UINT32)MmPciAddress(0, DevBus, 0, 0, 0);
//
//    //
//    // Put PCIe Root Port to D0 state
//    //
//    RpPmRegAddress = FindPcieCapIdAddress (RootPortPcieBase, 0x01) + 4;
//    RpPmReg = MmioRead32(RpPmRegAddress);
//    Data32 = RpPmReg & (~(BIT1 | BIT0));
//    MmioWrite32 (RpPmRegAddress, Data32);
//
//    //
//    // Enable PCIe Root Port MMIO decode
//    //
//    RpCmdReg = MmioRead32(RootPortPcieBase + 0x04);
//    Data32 = RpCmdReg | (BIT2 | BIT1);
//    MmioWrite32 (RootPortPcieBase + 0x04, Data32);
//
//    //
//    // Check PCIe Device exist or not
//    //
//    if (MmioRead32(DevPcieBase) != 0xFFFFFFFF) {
//      //
//      // Clear PCIe device PME status and enable PME and put to D3
//      //
//      DevPmRegAddress = FindPcieCapIdAddress (DevPcieBase, 0x01) + 4;
//      Data32 = MmioRead32(DevPmRegAddress);
//      Data32 |= (BIT15 | BIT8 | BIT1 | BIT0);
//      MmioWrite32 (DevPmRegAddress, Data32);
//    }
//
//    //
//    // Restore PCIe Command Register (Disable MMIO)
//    // Restore PCIe Root Port State (Put to D3 state)
//    //
//    MmioWrite32 (RootPortPcieBase + 0x04, RpCmdReg);
//    MmioWrite32 (RpPmRegAddress, RpPmReg);
//  }
//}
//[-end-170104-IB07400829-remove]//

/**
 CallBack before S5.

 @param [in]   DispatchHandle   The handle of this callback, obtained when registering
 @param [in]   DispatchContext  The predefined context which contained sleep type and phase

 @retval       None

**/
EFI_STATUS
S5SleepEntryCallBack (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  )
{
  EFI_STATUS    Status = EFI_SUCCESS;
  
//[-start-160609-IB07400742-add]//
  POST_CODE ((SMM_S5_SLEEP_CALLBACK));
  DEBUG ((EFI_D_INFO, "S5SleepEntryCallBack!!\n"));
//[-end-160609-IB07400742-add]//

//[-start-161118-IB07400817-add]//
//[-start-171115-IB07400928-modify]//
  //
  // If Wake On PME is disabled, disable all PCIE Wake Event (S5)
  //
  if ((!mSystemConfiguration.WakeOnPME)) {
    IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE0_EN));
    IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE1_EN));
    IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE2_EN));
    IoAnd32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)(~B_ACPI_GPE0a_EN_PCIE_WAKE3_EN));
  } 
  else if (mSystemConfiguration.WakeOnPME == SCU_WAKE_ON_PME_FORCE_ENALBED) {
//[-start-170104-IB07400829-remove]//
//    S5WakeOnLanConfig ();
//[-end-170104-IB07400829-remove]//
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE0_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE1_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE2_EN);
    IoOr32 (mAcpiBaseAddr + R_ACPI_GPE0a_EN, (UINT32)B_ACPI_GPE0a_EN_PCIE_WAKE3_EN);
  }
//[-end-171115-IB07400928-modify]//
//[-end-161118-IB07400817-add]//
  //
  // Enable/Disable USB Charging
  //
  

//[-start-170314-IB07400847-add]//
  if (mWakeOnRtcVariable != SCU_S5_WAKE_ON_RTC_DISABLED) {
    EnableS5WakeOnRtc();
  }
//[-end-170314-IB07400847-add]//
//[-start-170626-IB07400880-add]//
  if (mSystemConfiguration.S5WakeOnUsb) {
    WakeOnUsbFromS5(USB_CONFIG_READY_TO_SHUTDOWN);
  } 
//[-end-170626-IB07400880-add]//

  //
  // OemServices
  //
  Status = OemSvcS5Callback ();
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcKernelLib S5Callback, Status : %r\n", Status ) );
  return Status;
}

#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)    
/**
 SMI handler to restore ACPI mode

 @param [in]   Event
 @param [in]   Context

 @retval None.

**/
EFI_STATUS
EFIAPI
PlatformRestoreAcpiCallbackStart (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  UINT32 SmiEn;

  //
  // Disable SW SMI Timer
  //
  SmiEn = IoRead32(mAcpiBaseAddr + R_SMI_EN);
  SmiEn &= ~(B_SMI_EN_SWSMI_TMR);
  IoWrite32(mAcpiBaseAddr + R_SMI_EN, SmiEn);

  SaveRestoreState (TRUE);

  return EFI_SUCCESS;
}


/**
 SMI handler to restore ACPI mode

 @param [in]   Event
 @param [in]   Context

 @retval None.

**/
EFI_STATUS
EFIAPI
PlatformRestoreAcpiCallbackDone (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  EFI_OVERRIDE_ASPM_PROTOCOL   *OverrideAspmProtocol;
  EFI_STATUS                   Status;

  Status = mSmst->SmmLocateProtocol(&gEfiOverrideAspmProtocolGuid,
                                NULL,
                                (VOID **)&OverrideAspmProtocol
                               );
  if (!EFI_ERROR (Status)) {
    Status = OverrideAspmProtocol->OverrideAspmFunc ();
  }
  return EFI_SUCCESS;
}

//[-start-161117-IB07400815-add]//
VOID
NotifyAcpiCallbackByCmos (
  VOID
  )
{
  UINT8   CmosData;
  
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
  CmosData |= (B_CMOS_ACPI_CALLBACK_NOTIFY);
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
}
//[-end-161117-IB07400815-add]//

EFI_STATUS
EnableAcpiCallback (
  IN  CONST VOID                    *DispatchContext,
  IN  VOID                          *Interface,
  IN  EFI_HANDLE                    Handle
  )
/*++

Routine Description:
  SMI handler to enable ACPI mode

  Dispatched on reads from APM port with value 0xA0

  Disables the SW SMI Timer.
  ACPI events are disabled and ACPI event status is cleared.
  SCI mode is then enabled.

   Disable SW SMI Timer

   Clear all ACPI event status and disable all ACPI events
   Disable PM sources except power button
   Clear status bits

   Disable GPE0 sources
   Clear status bits

   Disable GPE1 sources
   Clear status bits

   Guarantee day-of-month alarm is invalid (ACPI 1.0 section 4.7.2.4)

   Enable SCI

Arguments:
  DispatchHandle  - EFI Handle
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:
  Nothing

--*/
{
  UINT32 SmiEn;
  UINT16 Pm1Cnt;
  UINT32 RegData32;
//[-start-151123-IB02950555-remove]//
//  UINT32 Gpe0aEn;
//[-end-151123-IB02950555-remove]//

//[-start-160415-IB08450340-modify]//
  //
  // Disable SW SMI Timer, SMI from USB & Intel Specific USB 2
  //
  SmiEn = IoRead32(mAcpiBaseAddr + R_SMI_EN);
//[-start-160930-IB07400791-modify]//
  //
  // Do not trun off XHCI SMI for handoff XHCI controller under OS
  //
//  SmiEn &= ~(B_SMI_EN_SWSMI_TMR | B_SMI_EN_LEGACY_USB3);
  SmiEn &= ~(B_SMI_EN_SWSMI_TMR);
//[-end-160930-IB07400791-modify]//
  IoWrite32(mAcpiBaseAddr + R_SMI_EN, SmiEn);
  
  RegData32 = IoRead32(mAcpiBaseAddr + R_SMI_STS);
  RegData32 |= B_SMI_STS_SWSMI_TMR;
  IoWrite32(mAcpiBaseAddr + R_SMI_STS, RegData32);
  
  //
  //  Disable PM sources except power button
  //
  mPM1_SaveState16 = IoRead16 (mAcpiBaseAddr + R_ACPI_PM1_EN);  
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_EN, B_ACPI_PM1_EN_PWRBTN);
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_STS, 0xffff);
  //
  // Guarantee day-of-month alarm is invalid (ACPI 5.0 Section 4.8.2.4 "Real Time Clock Alarm")
  // Clear Status D reg VM bit, Date of month Alarm to make Data in CMOS RAM is no longer Valid
  //
  IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_D);
  IoWrite8 (PCAT_RTC_DATA_REGISTER, 0x0); 

  //
  // Enable SCI
  //
  Pm1Cnt = IoRead16(mAcpiBaseAddr + R_ACPI_PM1_CNT);
  Pm1Cnt |= B_ACPI_PM1_CNT_SCI_EN;
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);


  //
  // Disable EC SMI notify and enable EC ACPI mode
  //
//[-start-161205-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
//[-end-170111-IB07400832-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  //
  // CRB EC code
  //
  DisableEcSmiMode ();
  EnableEcAcpiMode (TRUE);
#else
  //
  // OEM EC code
  //
#endif
#endif
//[-end-161205-IB07400822-modify]//
  
//[-end-160415-IB08450340-modify]//
  
  SaveRestoreState (FALSE);
//[-start-161117-IB07400815-add]//
  //
  // Notify ACPI callback via CMOS
  //
  NotifyAcpiCallbackByCmos ();
//[-end-161117-IB07400815-add]//
  return EFI_SUCCESS;
}

EFI_STATUS
DisableAcpiCallback (
  IN  CONST VOID                    *DispatchContext,
  IN  VOID                          *Interface,
  IN  EFI_HANDLE                    Handle
  )
/*++

Routine Description:
  SMI handler to disable ACPI mode

  Dispatched on reads from APM port with value 0xA1

  ACPI events are disabled and ACPI event status is cleared.
  SCI mode is then disabled.
   Clear all ACPI event status and disable all ACPI events
   Disable PM sources except power button
   Clear status bits
   Disable GPE0 sources
   Clear status bits
   Disable GPE1 sources
   Clear status bits
   Disable SCI

Arguments:
  DispatchHandle  - EFI Handle
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:
  Nothing

--*/
{
  UINT16 Pm1Cnt;
  
//[-start-160415-IB08450340-modify]//
  EnableEcAcpiMode (FALSE);
  //EnableEcSmiMode ();
  
  //
  // Disable SCI
  //
  Pm1Cnt = IoRead16(mAcpiBaseAddr + R_ACPI_PM1_CNT);
  Pm1Cnt &= ~B_ACPI_PM1_CNT_SCI_EN;
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);
  
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_STS, 0xffff);
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_EN, mPM1_SaveState16);
//[-end-160415-IB08450340-modify]//

  return EFI_SUCCESS;
}
#endif

VOID
DummyTco1Callback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  )
/*++

Routine Description:

  When an unknown event happen.

Arguments:

Returns:

  None

--*/
{
//[-start-161230-IB07400829-add]//
  switch (DispatchContext->Type) {

  case IchnMch:
    break;

  case IchnY2KRollover:
    break;

  case IchnTcoTimeout:
    //
    // action here (TCO first timeout)
    //
    // set for tco timeout -> hw reboot
    //
    CHIPSET_POST_CODE ((SMM_TCO_TIME_OUT_REBOOT));
    MmioAnd8 (PMC_BASE_ADDRESS + R_PMC_PM_CFG, (UINT8) ~B_PMC_PM_CFG_NO_REBOOT);
    break;

  case IchnOsTco:
    break;

  case IchnNmi:
    break;

  default:
    break;
  }
  return;
//[-end-161230-IB07400829-add]//
}

UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL     *Start;

  if (DevicePath == NULL) {
    return 0;
  }

  //
  // Search for the end of the device path structure
  //
  Start = DevicePath;
  while (!IsDevicePathEnd (DevicePath)) {
    DevicePath = NextDevicePathNode (DevicePath);
  }

  //
  // Compute the size and add back in the size of the end device path structure
  //
  return ((UINTN)DevicePath - (UINTN)Start) + sizeof(EFI_DEVICE_PATH_PROTOCOL);
}

//[-start-170314-IB07400847-add]//
EFI_STATUS
EnableS5WakeOnRtc(
  VOID
  )
{
  EFI_STATUS        Status;
  EFI_TIME          RtcWakeTime;
  UINT16            Data16;
  
  //
  // Get Current Time
  //
  Status = EfiSmmGetTime (&RtcWakeTime); // BCD mode only
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  switch (mWakeOnRtcVariable) {

  case SCU_S5_WAKE_ON_RTC_BY_EVERY_DAY:
  case SCU_S5_WAKE_ON_RTC_BY_DAY_OF_MONTH:
    
    RtcWakeTime.Day     = mSystemConfiguration.WakeOnS5DayOfMonth;
    RtcWakeTime.Hour    = mSystemConfiguration.WakeOnS5Time.Hour;
    RtcWakeTime.Minute  = mSystemConfiguration.WakeOnS5Time.Minute;
    RtcWakeTime.Second  = mSystemConfiguration.WakeOnS5Time.Second;
    
    if (mWakeOnRtcVariable == SCU_S5_WAKE_ON_RTC_BY_EVERY_DAY) { // By Day
      EfiSmmSetWakeupTime (FALSE, &RtcWakeTime);
    } else if (mWakeOnRtcVariable == SCU_S5_WAKE_ON_RTC_BY_DAY_OF_MONTH) { // By Month
      EfiSmmSetWakeupTime (TRUE, &RtcWakeTime);
    }
    break;
    
  case SCU_S5_WAKE_ON_RTC_BY_SLEEP_TIME:
    
    Data16 = RtcWakeTime.Second + mSystemConfiguration.S5WakeOnRtcAfterSec;
    RtcWakeTime.Second = Data16 % 60;
    Data16 = RtcWakeTime.Minute + (Data16 / 60);
    RtcWakeTime.Minute = Data16 % 60;
    Data16 = RtcWakeTime.Hour + (Data16 / 60);
    RtcWakeTime.Hour   = Data16 % 24;
    EfiSmmSetWakeupTime (FALSE, &RtcWakeTime);
    break;

  case SCU_S5_WAKE_ON_RTC_BY_OS_UTILITY:
  default:
    //
    // Enable RTC Alarm
    //
    IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_B);
    IoOr8 (PCAT_RTC_DATA_REGISTER, B_RTC_ALARM_INT_ENABLE);
    break;
  }

  //
  // Clear ICH RTC Status
  //
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_STS, B_ACPI_PM1_STS_RTC);
  
  //
  // Enable ICH RTC event
  //
  IoOr16 (mAcpiBaseAddr + R_ACPI_PM1_EN, B_ACPI_PM1_EN_RTC);

  return EFI_SUCCESS;
}
//[-end-170314-IB07400847-add]//

UINT8
HexToBcd(
  IN UINT8 HexValue
  )
{
  UINTN   HighByte;
  UINTN   LowByte;

  HighByte    = (UINTN)HexValue / 10;
  LowByte     = (UINTN)HexValue % 10;

  return ((UINT8)(LowByte + (HighByte << 4)));
}

UINT8
BcdToHex(
  IN UINT8 BcdValue
  )
{
  UINTN   HighByte;
  UINTN   LowByte;

  HighByte    = (UINTN)((BcdValue >> 4) * 10);
  LowByte     = (UINTN)(BcdValue & 0x0F);

  return ((UINT8)(LowByte + HighByte));
}

#if (defined KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT) && KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT
VOID
S5Ps2WakeFromKbcSxCallBack(
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:
  This routine is called for soft off.

Arguments:

Returns:

--*/
{
  EnableS5WakeFromKBC();
}
#endif

#if (defined KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT) && KEYBOARD_WAKE_FROM_S5_SETUP_SUPPORT
VOID
EnableS5WakeFromKBC()
/*++

Routine Description:
  This routine enabled keyboard specific key wake from S5 function.
  This routine is triggered when system is shutting down to S5.

Arguments:

Returns:

--*/
{
  UINT8              KBCstatus;
  UINT32             TimeOutkbc;
  UINT8              R_Times;
  UINT32             Temp32;
  UINT8              Temp8;
  UINTN              Index;
  EFI_KEY_SEQUENCE   *ScancodeTable;
  UINTN              ScancodeTableSize;

  //
  // Reset the keyboard device - 10 times for proper reset
  //
  R_Times = 0;
  for ( R_Times=0 ; R_Times <= 10; R_Times ++ ) {
    IoWrite8 (0x60, 0xFF);
    TimeOutkbc=0;
    for ( TimeOutkbc = 0; TimeOutkbc < 65536; TimeOutkbc += 30 ) {
      KBCstatus = IoRead8 (0x60);
      if ( KBCstatus == 0xFA ) {
        break;
      }
    }
  }

  //
  // Enter SIO configuration mode
  //
  IoWrite8(INDEX_PORT, 0x87);
  IoWrite8(INDEX_PORT, 0x87);

  //
  // Check if this is Wpc83627DHG
  //
  IoWrite8(INDEX_PORT, REG_DEVICE_ID);
  if (IoRead8(DATA_PORT) != SIO_W83627DHG) {
    return;
  }

  //
  // Select ACPI logical device
  //
  IoWrite8(INDEX_PORT, REG_LOGICAL_DEVICE);
  IoWrite8(DATA_PORT, SIO_ACPI);

  IoWrite8(INDEX_PORT, ACPI_WAKEUP_EN);
  Temp8 = IoRead8(DATA_PORT);
  Temp8 &= ~(BIT7|BIT5|BIT1|BIT0);
  if(mWakeOnS5KeyboardVariable) {
    Temp8 |= (BIT6);
  } else {
    Temp8 &= ~(BIT6);
  }
  IoWrite8(DATA_PORT, Temp8);

//  //
//  // Disable keyboard wakeup event from S1/S2 and following CRE0 bit 0 setting
//  //
//  IoWrite8(INDEX_PORT, ACPI_POWER_CONTROL);
//  Temp8 = HfIoRead8(DATA_PORT);
//  Temp8 &= ~(BIT3 | BIT2);
//  IoWrite8(DATA_PORT, Temp8);

  //
  // Set specific wake key
  //   Key set 1 index range: 0x00 - 0x0E
  //   Key set 2 index range: 0x30 - 0x3E
  //   Key set 3 index range: 0x40 - 0x4E
  //
  ScancodeTable = NULL;
  ScancodeTableSize = 0;
  if (mWakeOnS5KeyboardVariable == 0x01){
    ScancodeTable = S5WakePowerKeySequence;
    ScancodeTableSize = S5WakePowerKeySequenceCount;
  } else if (mWakeOnS5KeyboardVariable == 0x02){
    ScancodeTable = S5WakeAltPrintScreenSequence;
    ScancodeTableSize = S5WakeAltPrintScreenSequenceCount;
  }
  if (ScancodeTable != NULL) {
    for (Index = 0; Index < ScancodeTableSize; Index++) {
      IoWrite8(INDEX_PORT, ACPI_KBC_WAKE_INDEX);
      IoWrite8(DATA_PORT, ScancodeTable[Index].CRE1Data);
      IoWrite8(INDEX_PORT, ACPI_KBC_WAKE_DATA);
      IoWrite8(DATA_PORT, ScancodeTable[Index].CRE2Data);
    }
  }

  //
  // Clear all status registers
  //
//  IoWrite8(INDEX_PORT, ACPI_WAK_STS);
  IoWrite8(INDEX_PORT, ACPI_WAKE_STS);
  IoWrite8(DATA_PORT, IoRead8(DATA_PORT));
  IoWrite8(INDEX_PORT, ACPI_IRQ_STS);
  IoWrite8(DATA_PORT, IoRead8(DATA_PORT));

  //
  // Exit SIO configuration mode
  //
  IoWrite8(INDEX_PORT, 0xAA);

  //
  // Clear all PM1 status
  //
  IoWrite16(mAcpiBaseAddr + R_ACPI_PM1_STS, 0xFFFF);

  //
  // Clear all GPE0 status
  //
  Temp32 = IoRead32(mAcpiBaseAddr + R_ICH_ACPI_GPE0a_EN);
  IoWrite32(mAcpiBaseAddr + R_ACPI_GPE0a_EN, 0);
  IoWrite32(mAcpiBaseAddr + R_ACPI_GPE0a_STS, 0xFFFFFFFF);

  //
  // Set specific IOPME (GPI13) wake
  //
  IoWrite32(mAcpiBaseAddr + R_ACPI_GPE0a_EN, (Temp32 | BIT29));

}
#endif

EFI_STATUS
TpmPtsSmbsCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  CONST VOID                    *DispatchContext,
  IN  OUT VOID                      *CommBuffer  OPTIONAL,
  IN  UINTN                         *CommBufferSize  OPTIONAL
  )
/*++

Routine Description:

   TPM Handler

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:

  None.

--*/
{
 // EFI_STATUS    Status = EFI_SUCCESS;         
  UINT8         TpmSetting;

  //
  // Setting MORD
  //
  TpmSetting = AcpiTpmMord;
  mSmst->SmmIo.Io.Write (&mSmst->SmmIo, SMM_IO_UINT8, 0x72, 1, &TpmSetting);
  TpmSetting = mGlobalNvsAreaPtr->Area->MorData;
  mSmst->SmmIo.Io.Write (&mSmst->SmmIo, SMM_IO_UINT8, 0x73, 1, &TpmSetting);

  return EFI_SUCCESS;
}
//[-end-151229-IB03090424-modify]//
