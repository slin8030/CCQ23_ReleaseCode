/** @file
  This header file provides platform specific definitions used by other modules
  for platform specific initialization.

  This is not suitable for consumption by ASL or VRF files.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/
#ifndef _CMOSMAP_H_
#define _CMOSMAP_H_

///
///GENERAL USAGE GUIDELINES
///

/*
///
/// CMOS 0x00 - 0x0F are used by RTC. defined in ICHx spec
/// CMOS 0x10 - 0x7F are reserved for board specific use by Intel
/// CMOS 0x80 - 0xFF are reserved for OEM use only
///
*/

#define CmosIo_70 0x70
#define CmosIo_71 0x71
#define CmosIo_72 0x72
#define CmosIo_73 0x73

//
//PLATFORM SPECIFIC USAGE
//

#define CPU_HT_POLICY           0x50
#define CPU_HT_POLICY_ENABLED   0x01

#define TPM_POLICY            0x60
#define TPM_POLICY_ENABLED    0x01
//[-start-160526-IB10860200-remove]//
//#define CMOS_LCDPANELTYPE_REG           0x61
//#define CMOS_LCDPANELSCALING_REG        0x62
//#define CMOS_IGDBOOTTYPE_REG            0x63
//#define CMOS_BACKLIGHT_REG              0x64
//#define CMOS_LFP_PANEL_COLOR_DEPTH_REG  0x65
//#define CMOS_EDP_ACTIVE_LFP_CONFIG_REG  0x66
//#define CMOS_PRIMARY_DISPLAY_REG        0x67
//#define CMOS_IGD_DISPLAY_PIPE_B_REG     0x68
//#define CMOS_SDVOPANELTYPE_REG          0x69
//#define CMOS_CPV_STATE                  0x6A
//#define CMOS_PLATFORM_RESET_OS          0x80
//#define CMOS_CPU_BSP_SELECT             0x90
//#define CMOS_CPU_RATIO_OFFSET           0x92
//#define CMOS_ICH_PORT80_OFFSET          0x97
//
//#define CMOS_DATA_PORT	0x71
//#define CMOS_ADDR_PORT	0x70
//#define CMOS_BAD_REG	0xe
//
//#define CMOS_MAXRATIO_CONFIG_REG 0xEF
//
//#define CMOS_BOOT_REGISTER_REG              0x47
//#define RTC_ADDRESS_CENTURY                 0x32  
//#define RTC_ADDRESS_CENTURY_DEFAULT         0x20  ///<20th Century.BCD value
//
/////
///// Post Code value to be break at
/////
//#define CMOS_POST_CODE_BREAK_REG            0x48
//#define CMOS_POST_CODE_BREAK_1_REG          0x49
//#define CMOS_POST_CODE_BREAK_2_REG          0x4A
//#define CMOS_POST_CODE_BREAK_3_REG          0x4B
//
////
//// Debug Mask saved in CMOS
////
//#define CMOS_DEBUG_PRINT_LEVEL_REG          0x4C
//#define CMOS_DEBUG_PRINT_LEVEL_1_REG        0x4D
//#define CMOS_DEBUG_PRINT_LEVEL_2_REG        0x4E
//#define CMOS_DEBUG_PRINT_LEVEL_3_REG        0x4F 
////
//// CMOS usage Upper CMOS bank offsets:
////
////#define CMOS_CPU_BSP_SELECT             0x10
////#define CMOS_CPU_UP_MODE                0x11
////#define CMOS_CPU_RATIO_OFFSET           0x12
//
//#define CMOS_CPU_CORE_HT_OFFSET         0x13
//#define CMOS_EFI_DEBUG                  0x14
//#define CMOS_CPU_BIST_OFFSET            0x15
//#define CMOS_CPU_VMX_OFFSET             0x16
//#define CMOS_PORT80_OFFSET              0x17
//#define CMOS_PLATFORM_DESIGNATOR        0x18                          ///< Second bank CMOS location of Platform ID
//#define CMOS_VALIDATION_TEST_BYTE       0x19                          ///< BIT0 - Validation mailbox for UPonDP
//#define CMOS_SERIAL_BAUD_RATE           0x1A                          ///< 0=115200; 1=57600; 2=38400; 3=19200; 4=9600
//#define CMOS_DCU_MODE_OFFSET            0x1B                          
//#define CMOS_VR11_SET_OFFSET            0x1C                          
//#define CMOS_SBSP_TO_AP_COMM            0x20                          ///< SEC code use ONLY!!!
//#define TCG_CMOS_AREA_OFFSET            0x60                          ///< Also Change in Universal\Security\Tpm\PhysicalPresence\Dxe\PhysicalPresence.c &
//                                                                      ///< Also Change in Platform\IntelEpg\Thurley\Dxe\AcpiTables\Dsdt\Tpm.asi
//#define TCG_CMOS_MOR_AREA_OFFSET        (TCG_CMOS_AREA_OFFSET + 0x05) ///< Also Change in Platform\IntelEpg\Thurley\Dxe\AcpiTables\Dsdt\Tpm.asi
//
//#define EFI_CMOS_START_ADDRESS                      0x40
//#define EFI_CMOS_END_ADDRESS                        0x7F
//
//#define EFI_CMOS_CHECKSUM_ADDRESS                   0x4F
//#define EFI_CMOS_HYPERBOOT_FLAGS                    0x50
//#define   B_CMOS_FIRST_BOOT                         0x01
//#define   B_CMOS_BOOT_SUCCESS                       0x02
//#define   B_CMOS_HYPERBOOT_STATUS                   0x04
//#define   B_CMOS_HYPERBOOT_RECOVERY                 0x08
//#define   B_CMOS_BOOT_FAILED                        0x10
//#define   B_CMOS_BOOT_LOCK                          0x20
//#define EFI_CMOS_BOOT_CFG_FLAGS                     0x51
//#define   B_CMOS_BOOT_CFG_EXIST                     0x01
//#define   B_CMOS_BOOT_CFG_BOOT_MENU                 0x02
//#define   B_CMOS_BOOT_CFG_BOOT_CD                   0x04
//#define   B_CMOS_BOOT_CFG_BOOT_FDD                  0x08
//#define   B_CMOS_BOOT_CFG_BOOT_PXE                  0x10
//#define   B_CMOS_BOOT_CFG_BOOT_USB                  0x20
//#define   B_CMOS_BOOT_CFG_USB_FIRST                 0x40
//#define   B_CMOS_BOOT_CFG_BOOT_UEFI                 0x80 
//[-end-160526-IB10860200-remove]//
//[-start-160526-IB10860200-modify]//
#define EFI_CMOS_CRASHDUMP_ADDR_0                   0x58   //0x68
#define EFI_CMOS_CRASHDUMP_ADDR_1                   0x59   //0x69
#define EFI_CMOS_CRASHDUMP_ADDR_2                   0x5A   //0x6A
#define EFI_CMOS_CRASHDUMP_ADDR_3                   0x5B   //0x6B
#define EFI_CMOS_CRASHDUMP_TRIGGERED                0x5C   //0x6C
//[-end-160526-IB10860200-modify]//
//[-start-160526-IB10860200-remove]//
//#define EFI_CMOS_PENDING_ME_BIOS_ACTION             0x6D
//[-end-160526-IB10860200-remove]//
//[-start-160526-IB10860200-modify]//
#define EFI_CMOS_S4_WAKEUP_FLAG_ADDRESS             0x5D   //0x6E
//[-end-160526-IB10860200-modify]//
//[-start-160526-IB10860200-remove]//
//#define EFI_CMOS_XP_FLAG_ADDRESS                    0x6F
//#define EFI_CMOS_CAPSULE_ADDRESS_1                  0x70
//#define EFI_CMOS_CAPSULE_ADDRESS_2                  0x71
//#define EFI_CMOS_CAPSULE_ADDRESS_3                  0x72
//#define EFI_CMOS_CAPSULE_ADDRESS_4                  0x73
//#define EFI_CMOS_PERFORMANCE_FLAGS                  0x74
//#define   B_CMOS_MEMORY_INIT                          0x01
//#define   B_CMOS_FORCED_REBOOT                        0x02
//#define   B_CMOS_ALLOW_RESET                          0x04
//#define   B_CMOS_WD_RUNNING_FROM_OS                   0x08
//#define   B_CMOS_WD_FAILURE_STATUS_TO_OS              0x10
//#define   B_CMOS_BIOS_RESET_PERF_SETTINGS_TO_OS       0x20
//#define   B_CMOS_TCO_WDT_RESET                        0x40
//#define EFI_ACPI_TPM_REQUEST                        0x75
//#define EFI_ACPI_TPM_LAST_REQUEST                   0x76
//#define EFI_ACPI_TPM_MORD                           0x77
//#define EFI_CMOS_UCLK_DEFAULT                       0x78
//#define EFI_CMOS_CCLK_DEFAULT                       0x79
//#define EFI_CMOS_QCLK_DEFAULT                       0x7A
//#define EFI_CMOS_BURN_IN_MODE_FLAGS                 0x7C
//#define   B_CMOS_BIM_HANG                             0x01
//#define EFI_CMOS_ACPI_TABLE_FLAG_ADDRESS            0x7D
//#define   B_CMOS_HPET_ENABLED                        0x01
//#define EFI_CMOS_BOOT_FLAG_ADDRESS                  0x7E
//#define   B_CMOS_THERMAL_TRIP                         0x01
//#define   B_CMOS_FORCE_ENTER_SETUP                    0x02
//#define   B_CMOS_FORCE_NETWORK_BOOT                   0x04
//#define   B_CMOS_TPM_ENABLED                          0x08
//#define EFI_CMOS_SX_STATE_FLAG_ADDRESS              0x7F
//#define   B_CMOS_S5_SHUTDOWN                          0x01
//#define EFI_CMOS_BATTERY_CHARGING                   0xF0   
//[-end-160526-IB10860200-remove]//
//[-start-160526-IB10860200-modify]//
#define EFI_CMOS_CLEAN_RESET						0x5E   //0xFE    
#define   B_MAGIC_CLEAN_RESET_VALUE 				0x5F  //0xDD
//[-end-160526-IB10860200-modify]//
//[-start-160526-IB10860200-remove]//
//#define   B_MAGIC_INIT_VALUE						  0xBB
//
//#define EFI_CMOS_EOL  0xFFFF
//#define EFI_CMOS_CHECKSUM_EXCLUDES {EFI_CMOS_CHECKSUM_ADDRESS, \
//                                    EFI_CMOS_XP_FLAG_ADDRESS, \
//                                    EFI_ACPI_TPM_REQUEST, \
//                                    EFI_ACPI_TPM_LAST_REQUEST, \
//                                    EFI_ACPI_TPM_MORD, \
//                                    EFI_CMOS_BOOT_FLAG_ADDRESS, \
//                                    EFI_CMOS_S4_WAKEUP_FLAG_ADDRESS, \
//                                    EFI_CMOS_ACPI_TABLE_FLAG_ADDRESS, \
//                                    EFI_CMOS_SX_STATE_FLAG_ADDRESS, \
//                                    EFI_CMOS_PERFORMANCE_FLAGS, \
//                                    EFI_CMOS_BURN_IN_MODE_FLAGS, \
//                                    EFI_CMOS_HYPERBOOT_FLAGS, \
//                                    EFI_CMOS_BOOT_CFG_FLAGS, \
//                                    EFI_CMOS_EOL }
//[-end-160526-IB10860200-remove]//
#endif
