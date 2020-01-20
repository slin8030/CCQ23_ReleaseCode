/** @file

;******************************************************************************
;* Copyright (c) 2012-2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/*--
Module Name:

  ChipsetCmos.h

Abstract:


--*/

#ifndef _CHIPSET_CMOS_H_
#define _CHIPSET_CMOS_H_

#define R_CMOS_INDEX      0x70
#define R_CMOS_DATA       0x71
#define R_XCMOS_INDEX     0x72
#define R_XCMOS_DATA      0x73

//
// Table1  CMOS 0x00 - 0x0F are used by RTC. defined in ICHx spec
//         CMOS 0x10 - 0x7F are reserved for board specific use by Intel
// Table2  CMOS 0x00 - 0x7F are reserved for OEM use only
//

//
// Cmos Table2 definition is synchronized to CmosMap.h 
//
//
// CmosPerformanceFlag 0x34
//
#define   B_CMOS_MEMORY_INIT                          0x01
#define   B_CMOS_FORCED_REBOOT                        0x02
#define   B_CMOS_ALLOW_RESET                          0x04
#define   B_CMOS_WD_RUNNING_FROM_OS                   0x08
#define   B_CMOS_WD_FAILURE_STATUS_TO_OS              0x10
#define   B_CMOS_BIOS_RESET_PERF_SETTINGS_TO_OS       0x20
#define   B_CMOS_TCO_WDT_RESET                        0x40
//
// CmosACPITableFlagAddress 0x3D
//
#define   B_CMOS_HPET_ENABLED                         0x01
//
// CmosBootFlagAddress 0x3E
//
#define   B_CMOS_THERMAL_TRIP                         0x01
#define   B_CMOS_FORCE_ENTER_SETUP                    0x02
#define   B_CMOS_FORCE_NETWORK_BOOT                   0x04
#define   B_CMOS_TPM_ENABLED                          0x08

//
// CMOS VACANT range for OS loader/OS to use
//
#define EFI_CMOS_VACNAT_START_ADDRESS_L                      0x40
#define EFI_CMOS_VACNAT_END_ADDRESS_L                        0x7F

//[-start-160804-IB07400769-add]//

//
// Chipset Feature 0x2B
//
//#define B_CMOS_INTERNAL_UART                          BIT0
//#define B_CMOS_OUT_OF_MTRR                            BIT1
#define B_CMOS_S5_WAKE_ON_USB                         BIT2
//[-start-161117-IB07400815-add]//
#define B_CMOS_ACPI_CALLBACK_NOTIFY                   BIT3
//[-end-161117-IB07400815-add]//
//[-start-160923-IB07400789-add]//
#define B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT               BIT6 // 0: Disable, 1: Enable
//[-end-160923-IB07400789-add]//
#define B_CMOS_EFI_LEGACY_BOOT_ORDER                  BIT7 // 0: Legacy Device First, 1: EFI Device First

//
// Chipset Feature 0x2C
//
#define B_CMOS_TXE_DATA_UNAVAILABLE                   BIT0
//[-start-170620-IB07400876-add]//
#define B_CMOS_VAR_DATA_CLEAR_WORKAROUND              BIT1
//[-end-170620-IB07400876-add]//
//[-end-160804-IB07400769-add]//
//[-start-160905-IB07400778-add]//
//
// New Fast Crisis Recovery 0x17
//
//[-start-160907-IB07400780-modify]//
#define V_CMOS_FAST_RECOVERY_DISABLED                 0xFF
//[-end-160907-IB07400780-modify]//
#define V_CMOS_FAST_RECOVERY_ENABLED                  0x01
#define V_CMOS_FAST_RECOVERY_RESTORE_NV               0x02
//[-end-160905-IB07400778-add]//

typedef enum {
  
//
// 0x00 ~ 0x0F : OEM Reserve
//

  CmosCpuBspSelect          = 0x10,
  CmosCpuRatioOffset        = 0x12,
  CmosCpuCoreHtOffset       = 0x13,

  CmosCpuBistOffset         = 0x15,
  CmosCpuVmxOffset          = 0x16,
//[-start-160506-IB07220074-add]//
//[-start-160302-IB07220045-remove]//
  CmosRecoveryOnFlagAddress = 0x17,  // for New Fast Crisis
//[-end-160302-IB07220045-remove]//
//[-start-160824-IB07220130-add]//
  CmosSecureFlashS3Flag     = 0x18, // 0x18
//[-end-160824-IB07220130-add]//
//[-end-160506-IB07220074-add]//
//[-start-161117-IB07400815-add]//
  CmosRtPostCodeByte0       = 0x27,
  CmosRtPostCodeByte1       = 0x28,
  CmosRtPostCodeByte2       = 0x29,
  CmosRtPostCodeByte3       = 0x2A,
//[-end-161117-IB07400815-add]//
//[-start-160804-IB07400769-add]//
  CmosChipsetFeature        = 0x2B, // Chipset Feature
  CmosChipsetFeature2       = 0x2C, // Chipset Feature 2 - for silicon
//[-end-160804-IB07400769-add]//
  CmosS4WakeupFlagAddress   = 0x2E, // RC 0x6E
  CmosXpFlagAddress         = 0x2F, // RC 0x6F for LegacyBios
  CmosCapsuleAddress1       = 0x30, // RC 0x70 for S5Callback
  CmosCapsuleAddress2       = 0x31, // RC 0x71
  CmosCapsuleAddress3       = 0x32, // RC 0x72
  CmosCapsuleAddress4       = 0x33, // RC 0x73
  CmosPerformanceFlags      = 0x34, // RC 0x74 for WatchdogTimer Support
  AcpiTpmRequest            = 0x35, // RC 0x75
  AcpiTpmLastRequest        = 0x36, // RC 0x76
  AcpiTpmMord               = 0x37, // RC 0x77
  
//[-start-160603-IB06720411-add]//
// Temp solution for variable issue.
  CmosMrcBootVariableNvData = 0x38,
//[-end-160603-IB06720411-add]//
  

//[-start-160506-IB07220074-remove]//
// //[-start-160302-IB07220045-add]//
//   CmosRecoveryOnFlagAddress = 0x38, // for New Fast Crisis
// //[-end-160302-IB07220045-add]//
//[-end-160506-IB07220074-remove]//
  CmosAcpiTableFlagAddress  = 0x3D, // RC 0x7D
  CmosBootFlagAddress       = 0x3E, // RC 0x7E for LegacyBios
  CmosResetTypeByOs         = 0x3F, // OS reset request/types for next Android boot

  CmosAcpiVariableSet       = 0x6C, // Used by Memoryinit

//
// 0x40 ~ 0x57 : OEM Reserve
// 0x60 ~ 0x7F : OEM Reserve
//

} OPTION_CMOS_TABLE2;

#endif
