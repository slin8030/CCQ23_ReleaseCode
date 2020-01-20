/** @file
  Chipset Setup Configuration Definitions

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CHIPSET_SETUP_CONFIG_H_
#define _CHIPSET_SETUP_CONFIG_H_
#ifndef VFRCOMPILE
#include <Uefi/UefiInternalFormRepresentation.h>
#endif
#include <KernelSetupConfig.h>

#define FORMSET_ID_GUID_ADVANCE   {0xc6d4769e, 0x7f48, 0x4d2a, 0x98, 0xe9, 0x87, 0xad, 0xcc, 0xf3, 0x5c, 0xcc}
#define FORMSET_ID_GUID_POWER     {0xa6712873, 0x925f, 0x46c6, 0x90, 0xb4, 0xa4, 0x0f, 0x86, 0xa0, 0x91, 0x7b}

//
// Chipset definitions
//
#define LPSS_I2C_DEVICE_NUM             8
#define LPSS_HSUART_DEVICE_NUM          4
#define LPSS_SPI_DEVICE_NUM             3
#define SC_MAX_SATA_PORTS               2
//
// PCIe limits
//
#define SC_MAX_PCIE_ROOT_PORTS          SC_BXTP_PCIE_MAX_ROOT_PORTS
#define SC_BXT_PCIE_MAX_ROOT_PORTS      2
#define SC_BXTX_PCIE_MAX_ROOT_PORTS     4
#define SC_BXTP_PCIE_MAX_ROOT_PORTS     6

//[-start-161228-IB04530802-add]//
#define SC_MAX_USB2_PORTS               SC_BXTP_MAX_USB2_PORTS
#define SC_BXT_MAX_USB2_PORTS           3
#define SC_BXTP_MAX_USB2_PORTS          8

#define SC_MAX_USB3_PORTS               SC_BXTP_MAX_USB3_PORTS
#define SC_BXT_MAX_USB3_PORTS           2
#define SC_BXTP_MAX_USB3_PORTS          6
//[-end-161228-IB04530802-add]//

//
//  IGD
//
#define SCU_IGD_BOOT_TYPE_DISABLE       0x00
#define SCU_IGD_BOOT_TYPE_VBIOS_DEFAULT 0x00
#define SCU_IGD_BOOT_TYPE_VBT_DEFAULT   0x00
#define SCU_IGD_BOOT_TYPE_CRT           0x01
#define SCU_IGD_BOOT_TYPE_LFP           0x08

//
// Reclaim test count : When SetupConfig.ReclaimLongrunTest != 0, SetupUtility
// will write SetupConfig with FAST_RECLAIM_COUNT times.
//
#define FAST_RECLAIM_COUNT               35

//[-start-151211-IB03090424-modify]//
//[-start-160509-IB03090427-modify]//
#define HDAUDIO_FEATURES             10
//[-end-160509-IB03090427-modify]//
#define HDAUDIO_PP_MODULES           32
#define GUID_CHARS_NUMBER            37 // 36 GUID chars + null termination
//[-end-151211-IB03090424-modify]//
//
// These macro definition consumes by SetupUtiltiyLib
//
#define IDE_MODE                         0
#define AHCI_MODE                        1
#define RAID_MODE                        2
#define SATA_MODE_AHCI                   AHCI_MODE
#define SATA_MODE_RAID                   RAID_MODE

//
// Setup Formset label
//
#define IDE_UPDATE_LABEL                  0x1005

//
// Form ID: See UefiSetupUtilityDxe\Configuration.h
// 0 ~ 42 are reserved for RC
//
#define BOOT_CONFIGUATION_FORM_ID         0x50
#define ACPI_FEATURE_FORM_ID              0x51

//
//  Form ID : Thermal
//
#define OTHER_VFR_GOTO_LABEL              0x1d04
#define OTHER_VFR_GOTO_LABEL_END          0x1d05

//
// Callback key value
//
#define KEY_VT_CONFIG                     0x2064
#define KEY_XHCI_PREBOOT_SUPPORT          0x600
#define KEY_OTHER_VFR_GOTO_BASE           0x3000
//[-start-161008-IB07400794-add]//
#define KEY_DATA_CLEAR_KEYS               0x1239
#define KEY_OS_SELECTION                  0x3020
//[-end-161008-IB07400794-add]//
//[-start-161022-IB07400802-add]//
#define KEY_IOTG_RTEN                     0x1240
//[-end-161022-IB07400802-add]//

#define TARGET_TPM_FTPM                    0
#define TARGET_TPM_DTPM                    1

//[-start-160923-IB07400789-add]//
#include <SegChipsetFeatureSetupConfig.h>
//[-end-160923-IB07400789-add]//

#pragma pack(1)

#define SETUP_DATA CHIPSET_CONFIGURATION

//
//  Setup Utility Structure
//
typedef struct {
  //
  // Kernel system configuration (offset 0~229, total 230 bytes)
  //
  #define _IMPORT_KERNEL_SETUP_
  #include <KernelSetupData.h>
  #undef _IMPORT_KERNEL_SETUP_

  //
  // Chipset system configuration (offset 230~700, total 470 bytes)
  //
  #define _IMPORT_CHIPSET_SPECIFIC_SETUP_
  #include <ChipsetSpecificSetupData.h>
  #undef _IMPORT_CHIPSET_SPECIFIC_SETUP_
//==========================================================================================================
//==========================================================================================================
//----------------------------------------------------------------------------------------------------------
// Start area for OEM team. The following area is used by OEM team to modify.
// The total size of variable in this part are fixed (70bytes). That means if you need to add or remove
// variables, please modify the OEMRSV buffer size as well.
//----------------------------------------------------------------------------------------------------------
//OEM_Start
//Offset(530);
  UINT8         OEMRSV[70];                        // Reserve for OEM team
//OEM_End
//----------------------------------------------------------------------------------------------------------
// End of area for OEM team use.
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
// Start area for ODM team. The following area is used by ODM to modify.
// The total size of variable in this part are fixed (100bytes). That means if you need to add or remove
// variables, please modify the ODMRSV buffer size as well.
//----------------------------------------------------------------------------------------------------------
//OEM_Start
//Offset(600);
  UINT8         ODMRSV[100];                       // Reserve for ODM
//OEM_End
} CHIPSET_CONFIGURATION;

//[-start-151123-IB11270134-add]//
typedef struct {
  UINT8         IdeDevice0;
  UINT8         IdeDevice1;
  UINT8         IdeDevice2;
  UINT8         IdeDevice3;
  UINT8         IdeDevice4;
  UINT8         IdeDevice5;
  UINT8         IdeDevice6;
  UINT8         IdeDevice7;
} ADVANCE_CONFIGURATION;
//[-end-151123-IB11270134-add]//
#pragma pack()

#endif
