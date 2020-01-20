/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
 Platform Initialization Driver.

@copyright
  Copyright (c) 1999 - 2016 Intel Corporation. All rights reserved.
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.

@par Specification Reference:
**/
//[-start-160216-IB03090424-modify]//
#include "PlatformDxe.h"
#include "Platform.h"
#include <ScPolicyCommon.h>
#include <Library/I2clib.h>
#include <Library/TimerLib.h>
//#include <Library/PmicLib/PmicReg_DollarCove_XPOWERS.h>
#include <Rsci.h>
#include <Library/SideBandLib.h>
#include <ScRegs/RegsPcu.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/EcMiscLib.h>
#include <Library/SteppingLib.h>
#include <Library/HeciMsgLib.h>
//[-start-160628-IB07400749-remove]//
//#include <Library/PlatformCmosLib.h>
//[-end-160628-IB07400749-remove]//
//[-start-170418-IB15590028-add]//
#include <Library/DxeServicesTableLib.h>
#include <Library/S3BootScriptLib.h>
//[-end-170418-IB15590028-add]//
#include <Library/DxeInsydeChipsetLib.h>
#include <Ppi/SiPolicyPpi.h>
//[-start-160628-IB07400749-add]//
#include <Library/SteppingLib.h>
//[-end-160628-IB07400749-add]//
//[-start-160809-IB07400769-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160809-IB07400769-add]//
//[-start-160914-IB07400784-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160914-IB07400784-add]//
//[-start-160923-IB07400788-add]//
#include <Guid/EventLegacyBios.h>
#include <Library/CmosLib.h>
//[-end-160923-IB07400788-add]//
//[-start-160923-IB07400789-add]//
#include <Protocol/SetupUtility.h>
//[-end-160923-IB07400789-add]//
//[-start-190322-IB07401094-add]//
#include <Guid/EventGroup.h>
//[-end-190322-IB07401094-add]//

//
// Usb driver load protocol
//

#define EFI_MEMORY_PORT_IO  0x4000000000000000ULL
#define WC_PMIC_READ_INSTRUCTION_DELAY 10
#define WC_PMIC_WRITE_INSTRUCTION_DELAY 100

#define ENABLE    1
#define DISABLE   0

#define CUNIT_MCHBAR_REG                           0x48
#define PUNIT_SA_MEM_MCHBAR_MAP                    0x7000
#define MAILBOX_DATA                               0x80
#define MAILBOX_INTERFACE                          0x84
#define IPC_MEM_BAR_REG                            0x10

//[-start-160721-IB03090429-add]//
#define RESET_GENERATOR_PORT 0xCF9
//[-end-160721-IB03090429-add]//

extern EFI_GUID gEfiBootMediaHobGuid;

CHIPSET_CONFIGURATION       mSystemConfiguration;
//[-start-160803-IB07220122-add]//
UINT32                      mSetupVariableAttributes;
//[-end-160803-IB07220122-add]//
EFI_EVENT                   mReadyToBootEvent;
EFI_HANDLE                  mImageHandle;
UINTN                       mPciLanCount = 0;
VOID                        *mPciLanInfo;
VOID                             *mGopBrightnessCallbackReg;
extern EFI_GUID                  gEfiGopDisplayBrightnessProtocolGuid;
GOP_DISPLAY_BRIGHTNESS_PROTOCOL  *mGopDisplayProtocol;
EFI_PLATFORM_INFO_HOB            *mPlatformInfo;
BOOLEAN                          mPMCReadyToBootFirstTime = TRUE;
//[-start-190401-IB16530008-add]//
BOOLEAN                          mSeCPrepareForUpdateFlag = FALSE;
//[-end-190401-IB16530008-add]//

VOID
InitPlatformSaPolicy (
  IN CHIPSET_CONFIGURATION* SystemConfiguration
  );

EFI_STATUS
UsbLegacyPlatformInstall (
  );

VOID
InitPs2PlatformPolicy(
  );

VOID
InitTcoReset (
  );

//[-start-170418-IB15590028-add]//  
EFI_STATUS
AddHeciBarIntoMemIoDescriptor (
  VOID
  );  
//[-end-170418-IB15590028-add]//

typedef struct {
  EFI_PHYSICAL_ADDRESS    BaseAddress;
  UINT64                  Length;
} MEMORY_MAP;
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
VOID
EFIAPI
EnableAcpiCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  UINT32  RegData32;
  UINT16  Pm1Cnt;
  UINT16  AcpiBase;
//[-start-151123-IB02950555-remove]//
//  UINT32  Gpe0aEn;  
//[-end-151123-IB02950555-remove]//
  
  PostCode (SMM_ACPI_ENABLE_START);  //SW_SMI_ACPI_ENABLE 

  AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  DEBUG ((EFI_D_INFO, "EnableAcpiCallback: AcpiBase = %x\n", AcpiBase));

  //
  // Disable SW SMI Timer, SMI from USB & Intel Specific USB 2
  //
  RegData32 = IoRead32(AcpiBase + R_SMI_EN);
  RegData32 &= ~(B_SMI_EN_SWSMI_TMR | B_SMI_EN_LEGACY_USB3);
  IoWrite32(AcpiBase + R_SMI_EN, RegData32);

  RegData32 = IoRead32(AcpiBase + R_SMI_STS);
  RegData32 |= B_SMI_STS_SWSMI_TMR;
  IoWrite32(AcpiBase + R_SMI_STS, RegData32);

  //
  // Disable PM sources except power button
  //
  IoWrite16(AcpiBase + R_ACPI_PM1_EN, B_ACPI_PM1_EN_PWRBTN);
  IoWrite16(AcpiBase + R_ACPI_PM1_STS, 0xffff);

  //
  // Guarantee day-of-month alarm is invalid (ACPI 5.0 Section 4.8.2.4 "Real Time Clock Alarm")
  // Clear Status D reg VM bit, Date of month Alarm to make Data in CMOS RAM is no longer Valid
  //
  IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_D);
  IoWrite8 (PCAT_RTC_DATA_REGISTER, 0x0);

  //
  // Enable SCI
  //
  Pm1Cnt = IoRead16(AcpiBase + R_ACPI_PM1_CNT);
  Pm1Cnt |= B_ACPI_PM1_CNT_SCI_EN;
  IoWrite16(AcpiBase + R_ACPI_PM1_CNT, Pm1Cnt);

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

  OemSvcEnableAcpiCallback ();
  PostCode (SMM_ACPI_ENABLE_END);  //SW_SMI_ACPI_ENABLE 
}


UINT32 mTestValue;

/**

  XHCI decode callback for SkyLake/Cherrytrail/Braswell periodic SMI workaround 

  @param  Event                 Event.
  @param  Context               Context.

  @retval EFI_SUCCESS

**/
VOID
EFIAPI
XhciDecodeCallback (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  mTestValue = *(UINT32*)(UINTN)(0xe00a0000);
}

VOID
EFIAPI
InitPciDevPME (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
//

#ifdef SATA_SUPPORT
///Program SATA PME_EN
  SataPciCfg32Or (R_SATA_PMCS, B_SATA_PMCS_PMEE);
#endif
}

/**
  This function will be called during ExitBootServices to set a value in SSRAM.
  SV needs this to be done after BIOS is done, but before OS starts.

  @params  Event - 
  @params  Context - 
  
  @retval  None
**/
VOID
EFIAPI
SsramExitBootServiceCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  UINT32 Addr32;

  DEBUG ((EFI_D_INFO, "Setting Ssram value to indicate ExitBootServices.\n"));
    // In non-SV build, Dev13 is changed to ACPI mode, see PMCReadyToBoot(),
    // so we need to depend on the PCD to get the BaseAddr.
    Addr32 = PcdGet32(PcdPmcSsramBaseAddress0);
  MmioOr32( Addr32, (UINT32)0x1E <<27 );   //[31:27] of offest 0 = 0x1E
}


/**
  This function will be called during ExitBootServices to set a value in SSRAM.
  SV needs this to be done after BIOS is done, but before OS starts.

  @params  Event - 
  @params  Context - 
  
  @retval  None
**/
VOID
EFIAPI
SsramReadyToBootCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  UINT32 Addr32;

  DEBUG ((EFI_D_INFO, "Setting Ssram value to indicate ReadyToBoot.\n"));
    // In non-SV build, Dev13 is changed to ACPI mode, see PMCReadyToBoot(),
    // so we need to depend on the PCD to get the BaseAddr.
    Addr32 = PcdGet32(PcdPmcSsramBaseAddress0);
  MmioOr32( Addr32, (UINT32)0x1C <<27 );   //[31:27] of offest 0 = 0x1C
}

//[-start-161018-IB06740518-add]//
//[-start-161022-IB07400802-modify]//
//#if (BXTI_PF_ENABLE == 1)
//
// Doc#570618, Rev 1.4, Section 2.5 Sample Code to Enable RT Mode in BIOS  
//
VOID
InitRT (
  IN CHIPSET_CONFIGURATION  SystemConfiguration
)
{
  UINT32   Data;
  if (SystemConfiguration.RTEn != 0) { //RT is enabled
    //ISM disable
    Data = SideBandRead32(SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG); //PSF_1_PSF_GLOBAL_CONFIG
    Data = Data & 0xFFFFFFF7; //enLCG (Local Clock Gate enable) = 0
    SideBandWrite32(SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG, Data);
    Data = SideBandRead32(SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG);
    DEBUG((EFI_D_INFO, "PSF_1_PSF_GLOBAL_CONFIG = 0x%x.\n", Data));
  }
}
//#endif
//[-end-161022-IB07400802-modify]//
//[-end-161018-IB06740518-add]//

VOID
EFIAPI
PMCReadyToBoot (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
  EFI_STATUS                    Status;
  UINT32                        PciBar0RegOffset;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
  EFI_GUID                      SSRAMBASEGuid = { 0x9cfa1ece, 0x4488, 0x49be, { 0x9a, 0x4b, 0xe9, 0xb5, 0x11, 0x82, 0x65, 0x77 } };

//[-start-161018-IB06740518-modify]//
#ifndef FSP_WRAPPER_FLAG
  UINT32                        Data32;
#endif
//[-end-161018-IB06740518-modify]//

  if (mPMCReadyToBootFirstTime == FALSE) {
    return;
  }
  mPMCReadyToBootFirstTime = FALSE;

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  if (!EFI_ERROR(Status)) {
    //
    // Save PMC IPC1 Bar0 (PcdPmcIpc1BaseAddress0) to GNVS and size is 0x2000(8KB).
    //
//[-start-160801-IB03090430-modify]//
    PciBar0RegOffset = (UINT32)MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, R_PMC_BASE);
    DEBUG((DEBUG_INFO, " IPC1 BAR0 MMIO = %x \n ", (MmioRead32(PciBar0RegOffset) & 0xFFFFE000)));
//[-end-160801-IB03090430-modify]//
    GlobalNvsArea->Area->IPCBar0Address = (MmioRead32(PciBar0RegOffset) & 0xFFFFE000);
    GlobalNvsArea->Area->IPCBar0Length = 0x2000;
    //
    // Save PMC IPC1 Bar1 (PcdPmcIpc1BaseAddress0) to GNVS and size is 0x2000(8KB).
    //
//[-start-160801-IB03090430-modify]//
    PciBar0RegOffset = (UINT32)MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC, (PCI_FUNCTION_NUMBER_PMC_SSRAM), 0x10);
    DEBUG((DEBUG_INFO, "  SSRAM MMIO Bar = %x \n ", MmioRead32 (PciBar0RegOffset) & 0xFFFFFFF0));
//[-end-160801-IB03090430-modify]//
    GlobalNvsArea->Area->SSRAMBar0Address = MmioRead32 (PciBar0RegOffset) & 0xFFFFFFF0;
    GlobalNvsArea->Area->SSRAMBar0Length = 0x2000;
    //
    // Expose the SSRAM Bar0 to variable which will be used by AOS
    //
    Status = gRT->SetVariable (L"SSRAMBASE", &SSRAMBASEGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, sizeof(UINT32), &(GlobalNvsArea->Area->SSRAMBar0Address));

//[-start-161018-IB06740518-modify]//
#ifndef FSP_WRAPPER_FLAG
    SideBandAndThenOr32(
       SB_PORTID_PSF3,
       R_PCH_PCR_PSF3_T0_SHDW_PMC_PMC_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
       0xFFFFFFFF,
       (B_PCH_PCR_PSFX_T0_SHDW_PCIEN_MEMEN | B_PCH_PCR_PSFX_T0_SHDW_PCIEN_IOEN)
       );

    //
    // Switch the PMC IPC1 to ACPI mode.
    //
    SideBandAndThenOr32(
       SB_PORTID_PSF3,
       R_PCH_PCR_PSF3_T0_SHDW_PMC_PMC_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_CFG_DIS,
       0xFFFFFFFF,
       B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
       );

    //
    // Switch the PMC SSRAM to ACPI mode.
    //
    SideBandAndThenOr32(
      SB_PMC_IOSF2OCP_PORT,
      R_PMC_IOSF2OCP_PCICFGCTRL3,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS | B_PMC_IOSF2OCP_PCICFGCTRL3_BAR1_DISABLE3
      );

    //
    // Program PMC ACPI IRQ.
    //
    Data32 = SideBandRead32(SB_PMC_IOSF2OCP_PORT, R_PMC_IOSF2OCP_PCICFGCTRL2);
    Data32 |= BIT1;
    Data32 |= 0x00028000;
    SideBandWrite32(SB_PMC_IOSF2OCP_PORT, R_PMC_IOSF2OCP_PCICFGCTRL2, Data32);

    Data32 = SideBandRead32(SB_PMC_IOSF2OCP_PORT, R_PMC_IOSF2OCP_PCICFGCTRL2);
    DEBUG((EFI_D_INFO, "  PMC Interrupt value= %x \n ", Data32));
#endif
//[-end-161018-IB06740518-modify]//

    //
    // Update PUnit--IA mailbox reg address to GNVS. (register refer to P-Unit Regs.doc_review.pdf)
    //
    PciBar0RegOffset = (UINT32)MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, 0, 0, CUNIT_MCHBAR_REG);
    PciBar0RegOffset = (MmioRead32(PciBar0RegOffset) & 0xFFFF7000);
    GlobalNvsArea->Area->IPCBIOSMailBoxData = (UINT32)PciBar0RegOffset + MCHBAR_RANGE_PUNIT + PUNIT_MAILBOX_DATA;
    GlobalNvsArea->Area->IPCBIOSMailBoxInterface = (UINT32)PciBar0RegOffset + MCHBAR_RANGE_PUNIT + PUNIT_MAILBOX_INTERFACE;
    DEBUG((EFI_D_INFO, "  BIOS MAIL-BOX Data= %x \n ", GlobalNvsArea->Area->IPCBIOSMailBoxData));
    DEBUG((EFI_D_INFO, "  BIOS MAIL-BOX Interface= %x \n ", GlobalNvsArea->Area->IPCBIOSMailBoxInterface));
  }
}

VOID
EFIAPI
SaveS3PchSpiRegister (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  //
  // Saved SPI Firmware Memory Mapped I/O Registers for S3 resume used.
  //
  UINT32 Data32;
  // Protected Region 0 Register (32 bits) - (SPI_BASE_ADDRESS + 0x74)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR0)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR0)));
  // Protected Region 1 Register (32 bits) - (SPI_BASE_ADDRESS + 0x78)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR1)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR1)));

  // Protected Region 2 Register (32 bits) - (SPI_BASE_ADDRESS + 0x7c)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR2)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR2)));

  // Protected Region 3 Register (32 bits) - (SPI_BASE_ADDRESS + 0x80)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR3)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR3)));

  // Protected Region 4 Register (32 bits) - (SPI_BASE_ADDRESS + 0x84)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR4)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_PR4)));
  // Software Sequencing Flash Control Status Register (32 bits) - (SPI_BASE_ADDRESS + 0x90)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_SSFCS)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_SSFCS)));
  // Prefix Opcode Configuration Register (16 bits) - (SPI_BASE_ADDRESS + 0x94)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint16,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_PREOP),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_SPI_PREOP));
  
  // Opcode Type Configuration (16 bits) - (SPI_BASE_ADDRESS + 0x96)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint16,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_OPTYPE),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_SPI_OPTYPE));
  // Opcode Menu Configuration 0 (32bits) - (SPI_BASE_ADDRESS + 0x98)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_OPMENU0)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_OPMENU0)));
  // Opcode Menu Configuration 1 (32bits) - (SPI_BASE_ADDRESS + 0x9C)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + (R_SPI_OPMENU1)),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_SPI_OPMENU1)));
  // Flash Descriptor Observability Control Register (32 bits) - (SPI_BASE_ADDRESS + 0xB0)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_FDOC),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_SPI_FDOC));


  // Lower Vendor Specific Component Capabilities Register (32 bits) - (SPI_BASE_ADDRESS + 0xC4)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_LVSCC),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_SPI_LVSCC));
  // Upper Vendor Specific Component Capabilities Register (32 bits) - (SPI_BASE_ADDRESS + 0xC8)
  Data32 = MmioRead32((UINTN)(SPI_BASE_ADDRESS + R_SPI_UVSCC));
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_UVSCC),
                         1,
                         &Data32);
  // Hardware Sequencing Flash Status Register (16bits) - (SPI_BASE_ADDRESS + 0x04)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint16,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_HSFS),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_SPI_HSFS));
  // BIOS Control Register  - (SPI_BASE_ADDRESS + 0xFC)
  S3BootScriptSaveMemWrite (
                         EfiBootScriptWidthUint32,
                         (UINTN)(SPI_BASE_ADDRESS + R_SPI_BCR),
                         1,
                         (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_SPI_BCR));
  return;
}

/**
  Saves UART2 Base Address To Restore on S3 Resume Flows

  @param[in] Event     A pointer to the Event that triggered the callback.
  @param[in] Context   A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ConfigureUart2OnReadyToBoot (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
  EFI_STATUS              Status;
  UINTN                   Uart2BaseAddressRegister = 0;

  //
  // Save the UART2 BAR As It Will Need Restored Manually
  // On S3 Resume to Match the Value Written to the DBG2 ACPI Table
  //

  Uart2BaseAddressRegister = MmPciAddress (
    0,
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART2,
    R_LPSS_IO_BAR
    );

  mSystemConfiguration.Uart2KernelDebugBaseAddress = (UINT32) (MmioRead32 (Uart2BaseAddressRegister) & B_LPSS_IO_BAR_BA);

//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = SetSystemConfigData (&mSystemConfiguration, PcdGet32 (PcdSetupConfigSize));
  Status = gRT->SetVariable (
                SETUP_VARIABLE_NAME,
                &gSystemConfigurationGuid,
                mSetupVariableAttributes,
                PcdGet32 (PcdSetupConfigSize),
                &mSystemConfiguration
                );
//[-end-160803-IB07220122-modify]//

  ASSERT_EFI_ERROR (Status);
}

VOID
ReadyToBootFunction (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
  EFI_STATUS                      Status;
  EFI_ISA_ACPI_PROTOCOL           *IsaAcpi;
  EFI_ISA_ACPI_DEVICE_ID          IsaDevice;
  UINTN                           Size;
  UINT16                          State;
//[-start-160809-IB07400769-add]//
  UINT8                           CmosData;
//[-end-160809-IB07400769-add]//
//[-start-160816-IB07400771-add]//

//[-start-190401-IB16530008-remove]//
// UINTN                           VarSize;
//[-end-190401-IB16530008-remove]//

//[-end-160816-IB07400771-add]//

  Size = sizeof(UINT16);
  Status = gRT->GetVariable (
                  VAR_EQ_FLOPPY_MODE_DECIMAL_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &Size,
                  &State
                  );

  //
  // Disable Floppy Controller if needed
  //
  Status = gBS->LocateProtocol (&gEfiIsaAcpiProtocolGuid, NULL, (VOID **)&IsaAcpi);

  if (!EFI_ERROR(Status) && (State == 0x00)) {
    IsaDevice.HID = EISA_PNP_ID(0x604);
    IsaDevice.UID = 0;
    Status = IsaAcpi->EnableDevice(IsaAcpi, &IsaDevice, FALSE);
  }

  // save LAN info to a variable
  gRT->SetVariable (
         L"PciLanInfo",
         &gEfiPciLanInfoGuid,
         EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
         mPciLanCount * sizeof(PCI_LAN_INFO),
         mPciLanInfo
         );

  gBS->FreePool (mPciLanInfo);
//[-start-170418-IB15590028-add]//  
//[-start-190322-IB07401094-remove]//
//  AddHeciBarIntoMemIoDescriptor ();
//[-end-190322-IB07401094-remove]//
  gBS->CloseEvent(Event);
//[-end-170418-IB15590028-add]//    
  
//[-start-160809-IB07400769-add]//
//[-start-190401-IB16530008-modify]//
  if (mSeCPrepareForUpdateFlag == 1) {
    
    mSeCPrepareForUpdateFlag =0;
//[-end-190401-IB16530008-modify]//
	  Status = HeciIfwiPrepareForUpdate();
    if (!EFI_ERROR (Status)) {
      //
      // Set flag to skip all HECI/fTPM access when TXE data region Unavailable
      //
      CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2); 
      CmosData |= B_CMOS_TXE_DATA_UNAVAILABLE; // TXE Data Unavailable
      WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData);
    }
  }
  DEBUG((EFI_D_INFO, "mSeCPrepareForUpdateFlag HeciIfwiPrepareForUpdate Status = %r\n", Status));
//[-end-160809-IB07400769-add]//
  return;
}

#if defined(FIRMWARE_ID_BACKWARD_COMPATIBLE) && (FIRMWARE_ID_BACKWARD_COMPATIBLE != 0)
/**
Routine Description:
  Initializes the BIOS FIRMWARE ID from the FIRMWARE_ID build variable.
**/
STATIC
VOID
InitFirmwareId(
  )
{
  EFI_STATUS   Status;
  CHAR16       FirmwareIdNameWithPassword[] = FIRMWARE_ID_NAME_WITH_PASSWORD;

  //
  // First try writing the variable without a password in case we are
  // upgrading from a BIOS without password protection on the FirmwareId
  //
  Status = gRT->SetVariable(
      (CHAR16 *)&gFirmwareIdName,
      &gFirmwareIdGuid,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS |
      EFI_VARIABLE_RUNTIME_ACCESS,
      sizeof( FIRMWARE_ID ) - 1,
      FIRMWARE_ID
      );

  if (Status == EFI_INVALID_PARAMETER) {

    //
    // Since setting the firmware id without the password failed,
    // a password must be required.
    //
    Status = gRT->SetVariable(
      (CHAR16 *)&FirmwareIdNameWithPassword,
      &gFirmwareIdGuid,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS,
      sizeof( FIRMWARE_ID ) - 1,
      FIRMWARE_ID
      );
  }
}
#endif

VOID
InitSeC(
  VOID
  )
{
  EFI_STATUS Status;
  DXE_SEC_POLICY_PROTOCOL   *SeCPlatformPolicy;
  EFI_PEI_HOB_POINTERS      HobPtr;
  MBP_CURRENT_BOOT_MEDIA    *BootMediaData;
  SEC_OPERATION_PROTOCOL    *SeCOp;

  DEBUG((EFI_D_INFO, "InitSeC  ++\n"));

  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &SeCPlatformPolicy);

  if(EFI_ERROR(Status)) {
    return;
  }

  SeCPlatformPolicy->SeCConfig.EndOfPostEnabled = mSystemConfiguration.SeCEOPEnable;
  DEBUG((EFI_D_INFO, "InitSeC mDxePlatformSeCPolicy->SeCConfig.EndOfPostEnabled %x %x\n", SeCPlatformPolicy->SeCConfig.EndOfPostEnabled,mSystemConfiguration.SeCEOPEnable));

  //
  // Get Boot Media Hob, save to SystemConfigData for next S3 cycle.
  //
  DEBUG ((EFI_D_INFO, "CSE Boot Device record is %x from SystemConfigData\n", mSystemConfiguration.CseBootDevice));
  HobPtr.Guid   = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  BootMediaData = (MBP_CURRENT_BOOT_MEDIA*) GET_GUID_HOB_DATA (HobPtr.Guid);

  if (mSystemConfiguration.CseBootDevice != BootMediaData->PhysicalData) {
      mSystemConfiguration.CseBootDevice= (UINT8) BootMediaData->PhysicalData;
      DEBUG ((EFI_D_INFO, "Current CSE Boot Device is %x. Update to SystemConfigData\n", BootMediaData->PhysicalData));
//[-start-160806-IB07400769-modify]//
    ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//     Status = SetSystemConfigData (&mSystemConfiguration, sizeof(CHIPSET_CONFIGURATION));
      Status = gRT->SetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    mSetupVariableAttributes,
                    PcdGet32 (PcdSetupConfigSize),
                    &mSystemConfiguration
                    );
//[-end-160803-IB07220122-modify]//
  }

  Status = gBS->LocateProtocol (
                  &gEfiSeCOperationProtocolGuid,
                  NULL,
                  (VOID **) &SeCOp
                  );
  if(EFI_ERROR(Status)) {
    return;
  }

  Status = SeCOp->PerformSeCOperation(SEC_OP_CHECK_UNCONFIG);
  Status = SeCOp->PerformSeCOperation(SEC_OP_CHECK_HMRFPO);

}

//[-start-160802-IB03090430-remove]//
//VOID
//InitPlatformDirectIrqs (
//  VOID
//  )
//{
//
//  UINT16                                AcpiBaseAddr = 0;
//
//  DEBUG((EFI_D_INFO, "InitPlatformDirectIrqs() - Start\n"));
//  
//  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
//  
//  IoAnd32 (
//    AcpiBaseAddr + R_DIRECT_IRQ_EN,
//    (UINT32)(~(B_DIRECT_IRQ_EN_XHCI_EN | B_DIRECT_IRQ_EN_XDCI_EN))
//    );
//
//  IoOr32 (
//    AcpiBaseAddr + R_ACPI_GPE0a_EN,
//    (B_ACPI_GPE0a_EN_XHCI_PME_EN | B_ACPI_GPE0a_EN_XDCI_PME_EN)
//    );
//    
//  // 
//  // Write the SCI IRQ to ACPI Bridge in PMC
//  //
//  MmioAndThenOr32 (
//    GCR_BASE_ADDRESS + R_PMC_IRQ_SEL_2,
//    0x0000FFFF,
//    (V_PMC_IRQ_SEL_2_SCI_IRQ_9 << N_PMC_IRQ_SEL_2_SCIS) +
//    (41 << N_PMC_IRQ_SEL_2_DIR_IRQ_SEL_PMIC)
//    );
//  
//   DEBUG((EFI_D_INFO, "InitPlatformDirectIrqs() - End\n"));
//}
//[-end-160802-IB03090430-remove]//

//[-start-170418-IB15590028-add]//
EFI_STATUS
AddHeciBarIntoMemIoDescriptor (
  VOID
  )
{
  EFI_STATUS           Status;
  UINTN                Heci2BarAddr;
  UINTN                Heci3BarAddr;
  UINT64               Address;
  UINT64               BaseAddress;
  UINT64               Length;
  UINT64               Attributes;
  UINT16               CommandReg;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR  HeciBar0MemorySpaceDescriptor;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *PciRootBridgeIo;  

  //
  // Search PSP Device's PCI IO Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  (VOID **)&PciRootBridgeIo
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Address = EFI_PCI_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI2_FUNCTION_NUMBER, PCI_BASE_ADDRESSREG_OFFSET);
  Status = PciRootBridgeIo->Pci.Read (PciRootBridgeIo, EfiPciIoWidthUint32, Address, 1, &Heci2BarAddr);
  ASSERT_EFI_ERROR (Status);
  Heci2BarAddr = Heci2BarAddr & 0xFFFFFFF0;

  Address = EFI_PCI_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI2_FUNCTION_NUMBER, PCI_COMMAND_OFFSET);
  Status = PciRootBridgeIo->Pci.Read (PciRootBridgeIo, EfiPciIoWidthUint16, Address, 1, &CommandReg);
  ASSERT_EFI_ERROR (Status);
  CommandReg = CommandReg | EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE;
  Status = PciRootBridgeIo->Pci.Write (PciRootBridgeIo, EfiPciIoWidthUint16, Address, 1, &CommandReg);
  ASSERT_EFI_ERROR (Status);

  S3BootScriptSavePciCfgWrite (
    S3BootScriptWidthUint16,
    Address,
    1,
    &CommandReg
    );

  Address = EFI_PCI_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI3_FUNCTION_NUMBER, PCI_BASE_ADDRESSREG_OFFSET);
  Status = PciRootBridgeIo->Pci.Read (PciRootBridgeIo, EfiPciIoWidthUint32, Address, 1, &Heci3BarAddr);
  ASSERT_EFI_ERROR (Status);
  Heci3BarAddr = Heci3BarAddr & 0xFFFFFFF0;

  Address = EFI_PCI_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI3_FUNCTION_NUMBER, PCI_COMMAND_OFFSET);
  Status = PciRootBridgeIo->Pci.Read (PciRootBridgeIo, EfiPciIoWidthUint16, Address, 1, &CommandReg);
  ASSERT_EFI_ERROR (Status);
  CommandReg = CommandReg | EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_MEMORY_SPACE;
  Status = PciRootBridgeIo->Pci.Write (PciRootBridgeIo, EfiPciIoWidthUint16, Address, 1, &CommandReg);
  ASSERT_EFI_ERROR (Status);

  S3BootScriptSavePciCfgWrite (
    S3BootScriptWidthUint16,
    Address,
    1,
    &CommandReg
    );

  BaseAddress = ((EFI_PHYSICAL_ADDRESS) Heci2BarAddr) & 0xFFFFFFFF;
  Length      = 0x1000;

  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &HeciBar0MemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes  = HeciBar0MemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status      = gDS->SetMemorySpaceAttributes (BaseAddress, Length, Attributes);
  ASSERT_EFI_ERROR (Status);


  BaseAddress = ((EFI_PHYSICAL_ADDRESS) Heci3BarAddr) & 0xFFFFFFFF;

  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &HeciBar0MemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes  = HeciBar0MemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status      = gDS->SetMemorySpaceAttributes (BaseAddress, Length, Attributes);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
//[-end-170418-IB15590028-add]//

//[-start-190322-IB07401094-add]//
VOID
EFIAPI
EndOfDxeCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  AddHeciBarIntoMemIoDescriptor();
  gBS->CloseEvent(Event);
}
//[-end-190322-IB07401094-add]//

/*++

  Routine Description:

  This is the standard EFI driver point for the Driver. This
    driver is responsible for setting up any platform specific policy or
    initialization information.

  Arguments:

  ImageHandle  -  Handle for the image of this driver.
  SystemTable  -  Pointer to the EFI System Table.

  Returns:

  EFI_SUCCESS  -  Policy decisions set.

--*/
EFI_STATUS
EFIAPI
InitializePlatform (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                          Status;
  UINTN                               VarSize;
  EFI_HANDLE                          Handle;
//[-start-160517-IB03090427-modify]//
  EFI_EVENT                           EfiExitBootServicesEvent;
//[-end-160517-IB03090427-modify]//
//[-start-190322-IB07401094-add]//
  EFI_EVENT                           EndOfDxeEvent;
//[-end-190322-IB07401094-add]//
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL     *PciRootBridgeIo;  
  VOID                                *Registration;
  EFI_EVENT                           NotifyEvent;
//[-start-160914-IB07400784-add]//
//#if (BXTI_PF_ENABLE == 1)
  EFI_PLATFORM_INFO_HOB               *PlatformInfoHob;
  EFI_PEI_HOB_POINTERS                Hob;
//#endif
//[-end-160914-IB07400784-add]//
//[-start-170620-IB07400876-add]//
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND 
  UINT8                               CmosData;
#endif
//[-end-170620-IB07400876-add]//

  ReportStatusCodeEx (
         EFI_PROGRESS_CODE,
         EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_INIT,
         0,
         &gEfiCallerIdGuid,
         NULL,
         NULL,
         0
         );

  mImageHandle = ImageHandle;

  //
  // Make a new handle and install the protocol
  //
  Handle = NULL;
  Status = gBS->LocateProtocol (
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  (VOID **)&PciRootBridgeIo
                  );
  ASSERT_EFI_ERROR (Status);

//[-start-160628-IB07400749-remove]//
//  if (!CheckCmosBatteryStatus()) {
//    UINT8                 Buffer8 = 0;
//    UINTN                 SizeOfNvStore = sizeof(CHIPSET_CONFIGURATION);
//
//    DEBUG ((EFI_D_INFO, "InitializePlatform: Battery lost once, reset system configuration to default!!\n"));
//
//    //
//    // Get default variable value
//    //
//
//    //
//    // SC_PO: Use PlatformConfigDataLib instead of GetVariable.
//    //
////[-start-160803-IB07220122-modify]//
////     Status = GetSystemConfigData (&mSystemConfiguration, &SizeOfNvStore);
//    Status = gRT->GetVariable (
//                    SETUP_VARIABLE_NAME,
//                    &gSystemConfigurationGuid,
//                    &mSetupVariableAttributes,
//                    &SizeOfNvStore,
//                    &mSystemConfiguration
//                    );
////[-end-160803-IB07220122-modify]//
//    if (EFI_ERROR (Status)) {
//      Status = GetChipsetSetupVariableDxe (&mSystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
//    } 
//    
//    if (!EFI_ERROR (Status)) {
//      if(SizeOfNvStore >= sizeof(CHIPSET_CONFIGURATION)) {
//
////[-start-160826-IB03090433-add]//
//        //
//        // Get PCD value which was updated in PEI Phase during 'TpmPolicyInit' where this PCD is set to
//        // TRUE/FALSE based on PTT Enabled/Disabled from CSE Response for 'HeciGetMeFwCapability'.
//        // The setup variable needs to be updated here, as in PEI Phase Setvariable is not supported.
//        // The same will be referred in PEI phase in the same function 'TpmPolicyInit' during S3 Resume time.
//        //
//        if (PcdGetBool(PcdCsePttEnable)) {
//          mSystemConfiguration.CsePttStatus = 1;
//        }
//        else {
//          mSystemConfiguration.CsePttStatus = 0;
//        }
////[-end-160826-IB03090433-add]//
//
////[-start-160803-IB07220122-modify]//
////         Status = SetSystemConfigData (&mSystemConfiguration, sizeof(CHIPSET_CONFIGURATION));
//        Status = gRT->SetVariable (
//                        SETUP_VARIABLE_NAME,
//                        &gSystemConfigurationGuid,
//                        mSetupVariableAttributes,
//                        sizeof(CHIPSET_CONFIGURATION),
//                        &mSystemConfiguration
//                        );
////[-end-160803-IB07220122-modify]//
//        ASSERT_EFI_ERROR (Status);
//
//        Buffer8 = MmioRead8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
//
//        //
//        // CMOS Battery then clear status
//        //
//        if (Buffer8 & B_PMC_GEN_PMCON_RTC_PWR_STS) {
//          Buffer8 &= ~B_PMC_GEN_PMCON_RTC_PWR_STS;
//          MmioWrite8 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, Buffer8);
//        }
//      }
//    }
//  }
//[-end-160628-IB07400749-remove]//

  //
  // SC_PO: Use PlatformConfigDataLib instead of GetVariable.
  //
//[-start-160714-IB07220114-add]//
  VarSize = PcdGet32 (PcdSetupConfigSize);
//[-end-160714-IB07220114-add]//
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData (&mSystemConfiguration, &VarSize);
  Status = gRT->GetVariable (
                SETUP_VARIABLE_NAME,
                &gSystemConfigurationGuid,
                &mSetupVariableAttributes,
                &VarSize,
                &mSystemConfiguration
                );  

//[-end-160803-IB07220122-modify]//
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&mSystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
	  return Status;
    }
  }

  
//[-start-190401-IB16530008-add]//
  if (mSystemConfiguration.SeCPrepareForUpdate == 1) {
    
    mSystemConfiguration.SeCPrepareForUpdate = 0;
    mSeCPrepareForUpdateFlag = 1;
    
    VarSize = PcdGet32 (PcdSetupConfigSize);
    Status = gRT->SetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    mSetupVariableAttributes,
                    VarSize,
                    &mSystemConfiguration
                    );
    ASSERT_EFI_ERROR(Status);
	  //gBS->Stall(2000000); 
  }  
//[-end-190401-IB16530008-add]//
//[-start-160912-IB07400782-add]//
  //
  // Get PCD value which was updated in PEI Phase during 'TpmPolicyInit' where this PCD is set to
  // TRUE/FALSE based on PTT Enabled/Disabled from CSE Response for 'HeciGetMeFwCapability'.
  // The setup variable needs to be updated here, as in PEI Phase Setvariable is not supported.
  // The same will be referred in PEI phase in the same function 'TpmPolicyInit' during S3 Resume time.
  //
  if (PcdGetBool(PcdCsePttEnable)) {
    mSystemConfiguration.CsePttStatus = 1;
  }
  else {
    mSystemConfiguration.CsePttStatus = 0;
  }
//[-start-160914-IB07400784-add]//
//#if (BXTI_PF_ENABLE == 1)
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfoHob = GET_GUID_HOB_DATA(Hob.Raw);
  ASSERT (PlatformInfoHob != NULL);

  //
  // Update IsIOTGBoardIds for Setup usage
  //
//  mSystemConfiguration.BoardId = PlatformInfoHob->BoardId;
//[-start-161022-IB07400803-modify]//
  mSystemConfiguration.IsIOTGBoardIds = (UINT8)IsIOTGBoardIds();
//[-end-161022-IB07400803-modify]//

//[-start-161022-IB07400802-add]//
  if (mSystemConfiguration.IsIOTGBoardIds) {
    InitRT(mSystemConfiguration);   //Real Time
  }
//[-end-161022-IB07400802-add]//
//[-start-161008-IB07400794-modify]//
#ifdef APOLLOLAKE_CRB
  if (mSystemConfiguration.IsIOTGBoardIds) {
    if (mSystemConfiguration.PlatformSettingEn == 0) {
      switch (PlatformInfoHob->BoardId) {
        case BOARD_ID_BFH_IVI:
        case BOARD_ID_MRB_IVI:
          mSystemConfiguration.Hsic1Support = 1;
          break;
        case BOARD_ID_MNH_RVP:
        case BOARD_ID_APL_RVP_1A:
        case BOARD_ID_APL_RVP_1C_LITE:
//          mSystemConfiguration.VbtSelect = 4;
          break;
        case BOARD_ID_OXH_CRB:
        case BOARD_ID_JNH_CRB:
        case BOARD_ID_LFH_CRB:
          if (PlatformInfoHob->PmicVersion == 0) {
            mSystemConfiguration.EnableRenderStandby = 0;
          }
//          for (PortIndex = 0; PortIndex < SC_MAX_PCIE_ROOT_PORTS; PortIndex++) {
//            mSystemConfiguration.PcieRootPortEn[PortIndex]  = 2;
//          }
          break;
        default:
          break;
      }
      if (PlatformInfoHob->OsSelPss == SiAndroid) {
//[-start-170829-IB07400902-modify]//
        mSystemConfiguration.OsSelection = SiAndroid;
//        mSystemConfiguration.SecureBoot                 = FALSE;
        mSystemConfiguration.Cg8254                     = FALSE;
        mSystemConfiguration.MonitorMwaitEnable         = 0;
        mSystemConfiguration.TcoLock                    = 0;
        mSystemConfiguration.Ipc1Enable                 = 1;
        mSystemConfiguration.ScHdAudioNhltEndpointDmic  = 1;
        mSystemConfiguration.ScHdAudioNhltEndpointBt    = 0;
        mSystemConfiguration.ScHdAudioIoBufferOwnership = 0;
        mSystemConfiguration.LowPowerS0Idle             = TRUE;
        mSystemConfiguration.PmSupport                  = TRUE;
        mSystemConfiguration.ScUsbOtg                   = 0;
//[-end-170829-IB07400902-modify]//
      }
      mSystemConfiguration.PlatformSettingEn = 1;
    }

    mSystemConfiguration.PmicSetupDefault = 0;
    switch (PlatformInfoHob->BoardId) {
      case BOARD_ID_OXH_CRB:
      case BOARD_ID_JNH_CRB:
      case BOARD_ID_LFH_CRB:
        if (PlatformInfoHob->PmicVersion == 0) {
          mSystemConfiguration.PmicSetupDefault = 1;
          mSystemConfiguration.MaxPkgCState = 0;
        }
        break;
      case BOARD_ID_BFH_IVI:
      case BOARD_ID_MRB_IVI:
      case BOARD_ID_MNH_RVP:
      default:
        break;
    }
  }
#endif  
//[-end-161008-IB07400794-modify]//
//#endif
//[-end-160914-IB07400784-add]//
  Status = gRT->SetVariable (
    SETUP_VARIABLE_NAME,
    &gSystemConfigurationGuid,
    mSetupVariableAttributes,
    sizeof(CHIPSET_CONFIGURATION),
    &mSystemConfiguration
    );
//[-end-160912-IB07400782-add]//

  Status = OemSvcHookPlatformDxe (&mSystemConfiguration, PciRootBridgeIo, TRUE);
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcHookPlatformDxe 1, Status : %r\n", Status ) );

//[-start-170620-IB07400876-add]//
#ifdef AUTO_RUN_DATA_CLEAR_WORKAROUND
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2);
  if (CmosData & B_CMOS_VAR_DATA_CLEAR_WORKAROUND) { // Auto Run Data Clear if Variable space full
    CmosData &= ~(B_CMOS_VAR_DATA_CLEAR_WORKAROUND);
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData); 
    mSystemConfiguration.RpmbDataClear = 1;
  }
#endif
//[-end-170620-IB07400876-add]//

//[-start-160720-IB03090429-add]//
  //
  // Check for RPMB/NVM Data Clear flag set, if yes, then clear RPMB
  //
  if (mSystemConfiguration.RpmbDataClear == 1) {
	  //
	  // Clear the flag before issuing the command and store in setupconfiguration.
	  // Because after Issuing Ifwi Prepare for Update, CSE will be in Reset state.
	  //
	  DEBUG((DEBUG_INFO, "RPMB Flag Set. First Clear this Flag and update on Setup and then Clear RPMB-NVM Data\n "));
	  mSystemConfiguration.RpmbDataClear = 0;

//[-start-160803-IB07220122-modify]//
//     Status = SetSystemConfigData (&mSystemConfiguration, sizeof(CHIPSET_CONFIGURATION));
	  Status = gRT->SetVariable (
              		  SETUP_VARIABLE_NAME,
              		  &gSystemConfigurationGuid,
              		  mSetupVariableAttributes,
              		  VarSize,
              		  &mSystemConfiguration
              		  );
//[-end-160803-IB07220122-modify]//
	  ASSERT_EFI_ERROR(Status);

	  //
	  // CSE to Clear RPMB (NVM storage), it requires to receive the commands in sequence
	  // i.e., from Host side, 'IFWI Prepare For Update' command should be issued first for the
	  // purpose of keeping CSE in reset state and then 'Data Clear' Command to clear NVM.
	  // If CSE is not in reset state, then Data clear will not happen.
	  //  
	  gBS->Stall(2000000);   // Wait for 2 sec before issue IFWI Prepare for Update command. 
	                         // Looks after setup variable, CSE takes some time, when immediately
	                         // issue this cmd then cse not in reset and returns failure.
	                         // This delay is not on regular boot.
	  Status = HeciIfwiPrepareForUpdate();
	  if (Status == EFI_SUCCESS) {
		  //
		  // CSE Data Clear Command for TXE Compliance Test
		  //
		  Status = HeciDataClear();
		  if (Status == EFI_SUCCESS) {
			  DEBUG((DEBUG_INFO, " NVM / RPMB Data region Cleared SUCCESSFULLY\n "));
		  }
		  else {
			  DEBUG((DEBUG_INFO, "NVM / RPMB Data Clear FAILED\n "));
		  }
	  }
	  //
	  // Reset the system (EfiResetCold)
	  // Note: At this point, 'IFWI Prepare for Update' command is given to CSE and CSE is in reset state.
	  // Invoking gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL); Notifies HECI_SYSTEM_RESET by writing 
	  // into port 0xB2, this causes system hang and should not be issued. Hence directly writing into port 0xCF9
	  // 
	  DEBUG((DEBUG_INFO, "Restarting the System - Cold Reset\n\n "));
	  IoWrite8(RESET_GENERATOR_PORT, 0x2);
	  IoWrite8(RESET_GENERATOR_PORT, 0xE);
  }
//[-end-160720-IB03090429-add]//

  
  if (EFI_ERROR(Status)) {

    Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
               ReadyToBootFunction,
               NULL,
               &mReadyToBootEvent
               );
    
    // Create a ReadyToBoot Event to run the PME init process
    Status = EfiCreateEventReadyToBootEx (
                                    TPL_CALLBACK,
                                    InitPciDevPME,
                                    NULL,
                                    &mReadyToBootEvent
                                    );

  // Configure UART2 at ReadyToBoot if kernel debug is enabled (setup for S3 resume)
  if (GetBxtSeries() == BxtP && mSystemConfiguration.OsDbgEnable) {
    Status = EfiCreateEventReadyToBootEx (TPL_CALLBACK,
                                    ConfigureUart2OnReadyToBoot,
                                    NULL,
                                    &mReadyToBootEvent
                                    );
  }

  // Initialize BXT Platform Policy
  //SiPolicy is consumed by HSTI Silicon driver
//[-start-160413-IB03090426-modify]//
//#ifdef HSTI_SUPPORT
  InitSiPolicy();
//#endif
//[-end-160413-IB03090426-modify]//

    ReportStatusCodeEx (
           EFI_PROGRESS_CODE,
           EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP1,
           0,
           &gEfiCallerIdGuid,
           NULL,
           NULL,
           0
           );
    
    InitPlatformSaPolicy(&mSystemConfiguration);
    
    UsbLegacyPlatformInstall ();

    InitPs2PlatformPolicy();

    InitTcoReset();

    ReportStatusCodeEx (
           EFI_PROGRESS_CODE,
           EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP2,
           0,
           &gEfiCallerIdGuid,
           NULL,
           NULL,
           0
           );
    //
    // Install PCI Bus Driver Hook
    //
    PciBusDriverHook();

    EfiCreateProtocolNotifyEvent (
      &gExitPmAuthProtocolGuid,
      TPL_NOTIFY,
      SaveS3PchSpiRegister,
      NULL,
      &Registration
      );

    ReportStatusCodeEx (
           EFI_PROGRESS_CODE,
           EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP3,
           0,
           &gEfiCallerIdGuid,
           NULL,
           NULL,
           0
           );

#if defined(FIRMWARE_ID_BACKWARD_COMPATIBLE) && (FIRMWARE_ID_BACKWARD_COMPATIBLE != 0)
    //
    // Re-write Firmware ID if it is changed
    //
    InitFirmwareId();
#endif

    ReportStatusCodeEx (
           EFI_PROGRESS_CODE,
           EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP4,
           0,
           &gEfiCallerIdGuid,
           NULL,
           NULL,
           0
           );

    //
    // Direct IRQ (PMC) programming
    //
//    InitPlatformDirectIrqs();
  

    InitSeC();

    ReportStatusCodeEx (
           EFI_PROGRESS_CODE,
           EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_INIT_DONE,
           0,
           &gEfiCallerIdGuid,
           NULL,
           NULL,
           0
           );

//[-start-160517-IB03090427-modify]//
    // IPC1 and SRAM update before boot
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    PMCReadyToBoot,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &EfiExitBootServicesEvent
                    );

//[-start-160704-IB07220105-add]//
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    PMCReadyToBoot,
                    NULL,
                    &gEfiEventLegacyBootGuid,
                    &EfiExitBootServicesEvent
                    );
//[-end-160704-IB07220105-add]//

    if (mSystemConfiguration.BootType != LEGACY_BOOT_TYPE) {  //Legacy ACPI OS does not support ExitBootServices
      if (!FeaturePcdGet(PcdEnableSmiEnableAcpi)) {
      
        Status = gBS->CreateEventEx (
                        EVT_NOTIFY_SIGNAL,
                        TPL_NOTIFY,
                        EnableAcpiCallback,
                        NULL,
                        &gEfiEventExitBootServicesGuid,
                        &EfiExitBootServicesEvent
                        );
      }
    }
    
 }

  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcHookPlatformDxe 2, Status : %r\n", Status ) );

  //
  // Workaround for SkyLake/Cherrytrail/Braswell to make the xHCI periodic SMI can be generated by keep decode xHCI PCI space
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  XhciDecodeCallback,
                  NULL,
                  &NotifyEvent
                  );
  if (!EFI_ERROR(Status)) {
    Status = gBS->SetTimer (
                    NotifyEvent,
                    TimerPeriodic,
                    4000
                    );
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SsramExitBootServiceCallback,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &EfiExitBootServicesEvent
                  );
  
//[-start-190322-IB07401094-add]//
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  EndOfDxeCallback,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &EndOfDxeEvent
                  );
//[-end-190322-IB07401094-add]//

  Status = EfiCreateEventReadyToBootEx(
                  TPL_CALLBACK,
                  SsramReadyToBootCallback,
                  NULL,
                  &mReadyToBootEvent
                  );
  
//[-start-160923-IB07400788-add]//
//[-start-160923-IB07400789-modify]//
  Chipset2InitializePlatform (ImageHandle, SystemTable);
//[-end-160923-IB07400789-modify]//
//[-end-160923-IB07400788-add]//

  return EFI_SUCCESS;
//[-end-160517-IB03090427-modify]//
}
//[-end-160216-IB03090424-modify]//

