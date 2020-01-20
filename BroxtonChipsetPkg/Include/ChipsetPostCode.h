/** @file
  Post code definitions

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CHIPSET_POSTCODE_H_
#define _CHIPSET_POSTCODE_H_

#include <Library/PostCodeLib.h>

//
// Please use UART POST CODE to check HIGH-BYTE POST CODE.
//
#ifdef CHIPSET_POST_CODE_SUPPORT
#define CHIPSET_POST_CODE(PostCode) POST_CODE(PostCode)
#else
#define CHIPSET_POST_CODE(PostCode)
#endif

//
// Platform Init Pre Memory
//
#define PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE         0x02

#define PEI_PLATFORM_INIT_PRE_MEM_START             (0x01 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_SC_BASE_INIT      (0x02 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_GPIO_INIT         (0x03 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_SEC_UMA_ENTRY     (0x04 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_SI_POLICY_INIT    (0x05 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_READ_BXT_IDS      (0x06 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_SMIP_INIT         (0x07 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_ISH_GPIO          (0x08 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_UPDATE_BOOT_MODE  (0x09 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_SETTING_8254      (0x0A | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_RTC_HANDLER       (0x0B | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_OEM_EC_INIT       (0x0C | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_OEM_INIT_STAGE1   (0x0D | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_RECOVERY_CHECK    (0x0E | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_CPU_POLICY_INIT   (0x0F | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_GET_STAGE2_SPI    (0x10 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_GET_STAGE2_IMAGE  (0x11 | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))
#define PEI_PLATFORM_INIT_PRE_MEM_END               (0xED | (PEI_PLATFORM_INIT_PRE_MEM_HIGH_BYTE << 8))


//
// MRC POST CODE: 0x01xx
//
#define PEI_MRC_POST_CODE_HIGH_BYTE                 0x05

//[-start-160307-IB07400708-add]//
//
// FRC 0.7.0 Post Code
//
// PeiSaInitLib.c
#define PEI_POST_MEM_SA_INIT_ENTRY                  0x0A00
#define PEI_INSTALL_SA_HOBS_START                   0x0A02
#define PEI_INIT_PEI_DISPLAY_START                  0x0A03
#define PEI_PROGRAM_EC_BASE_START                   0x0A0A
#define PEI_SA_AUNIT_INIT_START                     0x0A0B
#define PEI_INIT_HYBRID_GRAPHICS_START              0x0A0C
#define PEI_INIT_IPU_DEVICE_START                   0x0A14
#define PEI_PROGRAM_SVID_SID_START                  0x0A18
#define PEI_INIT_PAVP_START                         0x0A32
#define PEI_SETTING_PCODE_RST_CPL                   0x0A61
#define PEI_PROGRAM_GRAPHICS_PM_INIT_START          0x0A63
#define PEI_SA_PEI_POLICY_DUMP_START                0x0A64
#define PEI_POST_MEM_SA_INIT_EXIT                   0x0A7F

// SiInitPreMem.c
#define PEI_FRC_PRE_MEM_NORTH_CLUSTER_ENTRY         0x0A00
#define PEI_FRC_INIT_PRE_MEM_GRAPHICS               0x0A50
#define PEI_FRC_PRE_MEM_NORTH_CLUSTER_EXIT          0x0A7F

// PeiDisplayInitLib.c
#define PEI_GRAPHICS_NOTIFY_CALL_BACK_START         0x0A04
#define PEI_CALL_PPI_AND_FILL_FRAME_BUFFER          0x0A05
#define PEI_GRAPHICS_PPI_INIT_START                 0x0A06
#define PEI_GRAPHICS_PPI_GET_MODE_START             0x0A07
#define PEI_FILL_FRAME_BUFFER_AND_SHOW_LOGO_START   0x0A08
#define PEI_GRAPHICS_NOTIFY_CALL_BACK_END           0x0A09

// ScInit.c
#define PEI_FRC_SC_INIT_START                       0x0B00
#define PEI_FRC_SC_INIT_PROGRAM_HSIO                0x0B01
#define PEI_FRC_SC_INIT_INIT_SMBUS                  0x0B02
#define PEI_FRC_SC_INIT_INIT_LPC                    0x0B03
#define PEI_FRC_SC_INIT_INIT_SATA                   0x0B04
#define PEI_FRC_SC_INIT_INIT_PCIE                   0x0B05
#define PEI_FRC_SC_INIT_SC_USB_INIT                 0x0B06
#define PEI_FRC_SC_INIT_CONFIG_OTG                  0x0B07
#define PEI_FRC_SC_INIT_CONFIG_HDA                  0x0B08
#define PEI_FRC_SC_INIT_CONFIG_GMM                  0x0B09
#define PEI_FRC_SC_INIT_CONFIG_LPSS                 0x0B0A
#define PEI_FRC_SC_INIT_CONFIG_SCC                  0x0B0B
#define PEI_FRC_SC_INIT_CONFIG_ISH                  0x0B0C
#define PEI_FRC_SC_INIT_CONFIG_ITSS                 0x0B0D
#define PEI_FRC_SC_INIT_CONFIG_INTERRUPTS           0x0B0E
#define PEI_SC_ON_END_OF_PEI_START                  0x0B40
#define PEI_SC_ON_END_OF_PEI_END                    0x0B4F
#define PEI_FRC_SC_INIT_END                         0x0B7F

// ScInitPreMem.c
#define PEI_FRC_SC_PRE_MEM_ENTRY                    0x0B00
#define PEI_FRC_SC_EARLY_INIT                       0x0B02
#define PEI_FRC_SC_CONFIG_PCIE_POWER_SEQ            0x0B10
#define PEI_FRC_SC_PRE_MEM_EXIT                     0x0B7F

// PowerMgmtInitPeim.c
#define PEI_INIT_POWER_MGMT_START                   0x0C60
#define PEI_INIT_POWER_MGMT_GLOBAL_VARIABLES_START  0x0C61
#define PEI_CREATE_CUSTOM_FVID_TABLE                0x0C62
#define PEI_SET_PPM_FLAGS                           0x0C63
#define PEI_USER_CONFIG_PPM_FLAGS                   0x0C64
#define PEI_INIT_POWER_MGMT_GLOBAL_VARIABLES_END    0x0C68
#define PEI_INIT_PPM_START                          0x0C69
#define PEI_INIT_PPM_END                            0x0C78
#define PEI_INIT_POWER_MGMT_END                     0x0C7F

// CpuCommonLib.c
#define CPU_COMMON_LIB_GET_MAX_ENABLED_COUNT_START  0x0C81
#define CPU_COMMON_LIB_GET_MAX_ENABLED_COUNT_END    0x0C84

// CpuInitPeim.c
//[-start-160330-IB07400712-remove]//
//#define PEI_CPU_ON_POLICY_INSTALLED_START           0x0C12
//#define PEI_INIT_XMM_SUPPORT                        0x0C16
//#define PEI_CPU_ON_POLICY_INSTALLED_END             0x0C1F
//[-end-160330-IB07400712-remove]//

// CpuInitPreResetCpl.c, PeiCpuPowerOnConfigLib.c
#define PEI_CPU_INIT_PRE_RESET_CPL_START            0x0C31
#define PEI_NUM_OF_ACTIVE_CORE_CHANGED              0x0C34
#define PEI_CPU_INIT_PRE_RESET_CPL_END              0x0C36

// CpuS3Lib.c
#define PEI_S3_S3_INIT_CPU_START                    0x0C40
#define PEI_S3_INIT_FEATURE_LOG_START               0x0C41
#define PEI_S3_INIT_FEATURE_LOG_END                 0x0C44
#define PEI_S3_S3_WAKE_UP_APS_START                 0x0C45
#define PEI_S3_S3_WAKE_UP_APS_END                   0x0C48
#define PEI_S3_INIT_MICROCODE_START                 0x0C49
#define PEI_S3_BUILD_GUID_DATA_HOB_START            0x0C4D
#define PEI_S3_CPU_S3_CALLBACK_START                0x0C51
#define PEI_S3_CPU_S3_CALLBACK_END                  0x0C54
#define PEI_S3_S3_INIT_CPU_DONE                     0x0C5F

// Features.c
#define PEI_INIT_FEATURE_PER_SETUP_START            0x0C0E
#define PEI_INIT_FEATURE_PER_SETUP_END              0x0C12
#define PEI_CPU_INIT_START                          0x0C30
#define PEI_COLLECT_PROCESSOR_FEATURE_START         0x0C37
#define PEI_PROGRAM_PROCESSOR_FEATURE_START         0x0C38
#define PEI_LOCK_FEATURE_BIT_START                  0x0C3A
#define PEI_CPU_INIT_DONE                           0x0C3F

// IdleStates.c
#define PEI_INTI_C_STATE                            0x0C6B

// MiscFunctions.c
#define PEI_INTI_MISC_FEATURES                      0x0C6D
#define PEI_PPM_LOCK_DONE_END                       0x0C70

// MpService.c
#define PEI_SETUP_MP_SERVICE_ENV                    0x0C20
#define PEI_INTI_CPU_DATA_HUB_START                 0x0C3B
#define PEI_INTI_CPU_DATA_HUB_END                   0x0C3E
#define PEI_CPU_MP_SERVICE_END                      0x0C9F

// PeiCpuPolicyLib.c
#define PEI_CREAT_CPU_CONFIG_BLOCKS_START           0x0C00
#define PEI_CREAT_CPU_CONFIG_BLOCKS_END             0x0C0F
#define PEI_CREAT_CPU_CONFIG_BLOCKS_END2            0x0C2F

// PerformanceStates.c
#define PEI_INIT_P_STATES_START                     0x0C6A
#define PEI_SET_BOOT_P_STATES_END                   0x0C6F
#define PEI_INIT_PPM_POST_START                     0x0C6E
#define PEI_INIT_PPM_POST_END                       0x0C74

// Thermal.c
#define PEI_FRC_INIT_THERMAL                        0x0C6C
//[-end-160307-IB07400708-add]//

//
// SI Init POST CODE
//
#define PEI_FRC_SI_INIT_HIGH_BYTE                   0x10

#define PEI_FRC_SI_INIT_START                       (0x01 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_SA_PUINT_INIT               (0x02 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_INIT_MP                     (0x03 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_CPU_INIT                    (0x04 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_S3_INIT_CPU                 (0x05 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_CPU_PM_INIT                 (0x06 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_SA_INIT                     (0x07 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SI_INIT_END                         (0xED | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))

//#define PEI_FRC_SC_INIT_START                       (0x11 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_ENABLE_P2SB_BASE            (0x12 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_PROGRAM_HSIO                (0x13 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_INIT_RTC                    (0x14 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_MISC_SC_CONFIG_INIT         (0x15 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_INIT_IOAPIC                 (0x16 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_INIT_SMBUS                  (0x17 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_INIT_LPC                    (0x18 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_INIT_SATA                   (0x19 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_INIT_PCIE                   (0x19 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_PROGRAM_SVID_SID            (0x1A | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_INIT_SPI                    (0x1B | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_SIRQ                 (0x1C | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_SC_PM_INIT                  (0x1D | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_MISC_PM              (0x1E | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_ADD_PM               (0x1F | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_S0IX                 (0x20 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_MISC_ITEMS           (0x21 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_CLOCK_GATING         (0x22 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_POWER_GATING         (0x23 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_SC_USB_INIT                 (0x24 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_OTG                  (0x25 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_HDA                  (0x26 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_GMM                  (0x27 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_LPSS                 (0x28 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_SCC                  (0x29 | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_ISH                  (0x2A | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_IOSF                 (0x2B | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_CONFIG_ITSS                 (0x2C | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_CONFIG_PMC                  (0x2D | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
#define PEI_FRC_SC_INIT_VTD_PEI_INIT                (0x2E | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))
//#define PEI_FRC_SC_INIT_END                         (0x2F | (PEI_FRC_SI_INIT_HIGH_BYTE << 8))

//
// Chipset POST CODE
//
#define PEI_FRC_CHECK_BOOT_DEVICE_START             0xCCB0
#define PEI_FRC_CHECK_BOOT_DEVICE_END               0xCCB1
//[-start-160618-IB07400744-add]//
#define PEI_RECOVERY_RESTORE_NV_DATA                0xCCB2
#define PEI_HECI_DEVICE_ERROR_RETRY                 0xCCB3
//[-end-160618-IB07400744-add]//
//[-start-160620-IB07400745-add]//
#define PEI_WAIT_MP_WAKE_UP_APS                     0xCCB4
//[-end-160620-IB07400745-add]//
//[-start-160629-IB07400749-add]//
#define PEI_CMOS_USING_SETUP_DEFAULT                0xCCB5
#define DXE_CMOS_RESTORE_SETUP_DEFAULT              0xCCB6
//[-end-160629-IB07400749-add]//
//[-start-160808-IB07400769-add]//
#define SMI_CHIPSET_FEATURE                         0xCCB7
//[-end-160808-IB07400769-add]//
//[-start-161002-IB07400791-add]//
#define PEI_SETUP_DATA_IN_MEMORY_MISMATCH           0xCCB8
#define DXE_SETUP_DATA_IN_MEMORY_MISMATCH           0xCCB9
//[-end-161002-IB07400791-add]//

//
// Error/Reboot/Shutdown POST CODE
//
#define PEI_FRC_FUN_DISABLE_UPDATE_REBOOT           0xEEE0
#define DXE_SAVE_MEMORY_CONFIG_REBOOT               0xEEE1
#define PEI_FRC_HECI_TAKE_OWNER_SHIP_FAIL           0xEEE2
#define PEI_FRC_SC_INIT_IPC_NOT_READY               0xEEE3
#define PEI_FRC_SC_INIT_IPC_CMD_BUSY_TIMEOUT        0xEEE4
#define PEI_FRC_PLATFORM_INIT_SET_SCU_DATA_FAIL     0xEEE5
#define PEI_MRC_CONFIG_MEMORY_FAIL                  0xEEE6
#define PEI_UNKNOWN_BOOT_SOURCE                     0xEEE7
//[-start-160517-IB07400729-add]
#define PEI_MRC_MEMORY_INIT_RETRY                   0xEEE8
#define PEI_MRC_REC_EN_DELAY_FAIL                   0xEEE9 //RecEnDelay
#define PEI_MRC_TX_DQS_DELAY_FAIL                   0xEEEA //TxDqsDelay
//[-end-160517-IB07400729-add]//
//[-start-160620-IB07400745-add]//
#define PEI_MP_WAKE_UP_AP_ERROR                     0xEEEB
//[-end-160620-IB07400745-add]//
//[-start-161208-IB07400824-add]//
#define DXE_MEMORY_CONSISTENCY_CHECK                0xEEEC
//[-end-161208-IB07400824-add]//
//[-start-161230-IB07400829-add]//
#define SMM_TCO_TIME_OUT_REBOOT                     0xEEED
//[-end-161230-IB07400829-add]//
#endif
