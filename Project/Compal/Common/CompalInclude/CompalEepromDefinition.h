/* (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00                             Init version.
 1.01      16/05/19  Vanessa      Follow Common EEPROM define 20160323(V0.14)
*/
#ifndef _COMPAL_EEPROM_DEFINITION_H_
#define _COMPAL_EEPROM_DEFINITION_H_

//
// EEPROM Common Definition
//
#define OEM_EEPROM_DATA_LENGTH                        0x800     // Max data of EEPROM 
#define OEM_EEPROM_MODIFY_FLAG                        0xEB      // Common design  
#define OEM_EEPROM_BE_MODIFIED_FLAG                   0xAA      // indicates at least a data byte of the EC EEPROM had been modified  
#define READ_POLICY                                   0x01      // 0x00 --> Read EEPROM Data by EC 
                                                                // 0x01 --> Read EEPROM Data from Switchable Platform
                                                                // 0x02 --> Read EEPROM Data from share ROM Platform
#define OEM_EEPROM_OSTYPE_OFFSET                      0xEC      // Common design  
#define OEM_EEPROM_OSTYPE_LENGTH                      1         // Common design  
#define OEM_EEPROM_UEFI_BOOT_TYPE_OFFSET              0xF3      // Common design  
#define OEM_EEPROM_UEFI_BOOT_TYPE_LENGTH              1         // Common design  
#define OEM_EEPROM_SECURE_BOOT_STATUS_OFFSET          0xF4      // Common design  
#define OEM_EEPROM_SECURE_BOOT_STATUS_LENGTH          1         // Common design  

#define OEM_EEPROM_SWTBANK_LENGTH                     0x1000    // A bank size, only for backward compatible with 8KB(4KB*2) switchale(flip/flop) EEPROM platform
  #define OEM_EEPROM_OFFSET_SWTBANK_VAILD_FLAG          (OEM_EEPROM_SWTBANK_LENGTH - 1)     // EEPROM's offset of SWTBANK_VAILD_FLAG
  #define OEM_EEPROM_SWTBANK_VALID_FLAG                 0xA5    // indicates the bank of EEPROM is activated(in use state) now.															

//
// A30 EEPROM Offset/Length Definition
//
#define A30_EEPROM_SN_OFFSET                          0x00      // Electronic serial number offset 0x00 - 0x1F
#define A30_EEPROM_SN_LENGTH                          0x20      // 32 Bytes
#define A30_EEPROM_BUNDLE_NO_OFFSET                   0x20      // Bundle number offset 0x20 - 0x3F
#define A30_EEPROM_BUNDLE_NO_LENGTH                   0x20      // 32 Bytes
#define A30_EEPROM_MANUFACTURE_OFFSET                 0x40      // Manufacture offset 0x40 - 0x4F
#define A30_EEPROM_MANUFACTURE_LENGTH                 0x10      // 16 Bytes
#define A30_EEPROM_SYSTEM_VER_OFFSET                  0x50      // System version offset 0x50 - 0x6F
#define A30_EEPROM_SYSTEM_VER_LENGTH                  0x20      // 32 Bytes
#define A30_EEPROM_UUID_OFFSET                        0x70      // UUID offset 0x70 - 0x7F
#define A30_EEPROM_UUID_LENGTH                        0x10      // 16 Bytes
#define A30_EEPROM_PRODUCT_NAME_OFFSET                0x80      // Product name offset 0x80 - 0x9F
#define A30_EEPROM_PRODUCT_NAME_LENGTH                0x20      // 32 Bytes
#define A30_EEPROM_OEM_ID_OFFSET                      0x260     // OemID bank2 offset 60~65 
#define A30_EEPROM_OEM_ID_LENGTH                      0x06      // 6 Bytes
#define A30_EEPROM_OEM_TABLEID_OFFSET                 0x270     // OemTableID bank2 offset 70~77 
#define A30_EEPROM_OEM_TABLEID_LENGTH                 0x08      // 8 Bytes

//
// A31 EEPROM Offset/Length Definition
//
#define A31_EEPROM_USER_PASSWORD_OFFSET               0x00      // User Password offset 0x00 - 0x01
#define A31_EEPROM_USER_PASSWORD_LENGTH               0x02      // 2 Bytes
#define A31_EEPROM_USER_PASSWORD_FLAG_OFFSET          0x02      // User Password flag offset 0x02
#define A31_EEPROM_USER_PASSWORD_FLAG_LENGTH          0x01      // 1 Byte
#define A31_EEPROM_SUPER_PASSWORD_OFFSET              0x03      // Super Password offset 0x03 - 0x04
#define A31_EEPROM_SUPER_PASSWORD_LENGTH              0x02      // 2 Bytes
#define A31_EEPROM_SUPER_PASSWORD_FLAG_OFFSET         0x05      // Super Password flag offset 0x05
#define A31_EEPROM_SUPER_PASSWORD_FLAG_LENGTH         0x01      // 1 Byte
#define A31_EEPROM_BRANDING_FLAG_OFFSET               0x06      // Branding flag offset 0x06 - 0x07
#define A31_EEPROM_BRANDING_FLAG_LENGTH               0x02      // 2 Bytes
#define A31_EEPROM_TOUCHPAD_FLAG_OFFSET               0x08      // TouchPad flag offset 0x08
#define A31_EEPROM_TOUCHPAD_FLAG_LENGTH               0x01      // 1 Byte
#define A31_EEPROM_L6_PROJECT_FLAG_OFFSET             0x09      // L6 project flag offset 0x09
#define A31_EEPROM_L6_PROJECT_FLAG_LENGTH             0x01      // 1 Byte
#define A31_EEPROM_FIRST_BOOT_FLAG_OFFSET             0x0A      // First Boot flag offset 0x0A
#define A31_EEPROM_FIRST_BOOT_FLAG_LENGTH             0x01      // 1 Byte
#define A31_EEPROM_TV_WWAN_FLAG_OFFSET                0x0B      // TV&WWAN flag offset 0x0B
#define A31_EEPROM_TV_WWAN_FLAG_LENGTH                0x01      // 1 Byte
#define A31_EEPROM_EPP_ID_OFFSET                      0x10      // EPPID offset 0x10 - 0x2F
#define A31_EEPROM_EPP_ID_LENGTH                      0x20      // 32 Bytes
#define A31_EEPROM_SERVICE_TAG_OFFSET                 0x30      // Service Tag offset 0x30 - 0x3F
#define A31_EEPROM_SERVICE_TAG_LENGTH                 0x10      // 16 Bytes
#define A31_EEPROM_ASSET_TAG_OFFSET                   0x40      // Asset Tag offset 0x40 - 0x4F
#define A31_EEPROM_ASSET_TAG_LENGTH                   0x10      // 16 Bytes
#define A31_EEPROM_LAN_MAC_ADDR_OFFSET                0xA0      // LAN Mac Address offset 0xA0 - 0xA5
#define A31_EEPROM_LAN_MAC_ADDR_LENGTH                0x06      // 6 Bytes
#define A31_EEPROM_MFG_MODE_OFFSET                    0xA7      // MFG mode offset 0xA7
#define A31_EEPROM_MFG_MODE_LENGTH                    0x01      // 1 Byte
#define A31_EEPROM_EC_MFG_MODE_OFFSET                 0xA8      // EC MFG mode offset 0xA8
#define A31_EEPROM_EC_MFG_MODE_LENGTH                 0x01      // 1 Byte
#define A31_EEPROM_MFG_DISABLE_OFFSET                 0xA9      // MFG Disable offset 0xA9
#define A31_EEPROM_MFG_DISABLE_LENGTH                 0x01      // 1 Byte
#define A31_EEPROM_COMPUTRACE_OFFSET                  0xAA      // Computrace offset 0xAA
#define A31_EEPROM_COMPUTRACE_LENGTH                  0x01      // 1 Byte
#define A31_EEPROM_COMPUTRACE_VALIDATION_KEY_OFFSET   0xAB      // Computrace validation key offset 0xAB
#define A31_EEPROM_COMPUTRACE_VALIDATION_KEY_LENGTH   0x04      // 4 Bytes
#define A31_EEPROM_SLP20_FLAG_OFFSET                  0xB0      // SLP 2.0 Flag offset 0xB0
#define A31_EEPROM_SLP20_FLAG_LENGTH                  0x01      // 1 Bytes
#define A31_EEPROM_FIRST_BOOT_AFTER_FLASH_OFFSET      0xB1      // 1's boot after flash offset 0xB1
#define A31_EEPROM_FIRST_BOOT_AFTER_FLASH_LENGTH      0x01      // 1 Bytes
#define A31_EEPROM_OCCUR_FLAG_OFFSET                  0xB2      // Occur Flag offset 0xB2
#define A31_EEPROM_OCCUR_FLAG_LENGTH                  0x01      // 1 Bytes
#define A31_EEPROM_ENERGY_STAR_OFFSET                 0xB3      // Energy Star offset 0xB3
#define A31_EEPROM_ENERGY_STAR_LENGTH                 0x01      // 1 Bytes
#define A31_EEPROM_1394_GUID_OFFSET                   0xE0      // 1394 GUID offset 0xE0
#define A31_EEPROM_1394_GUID_LENGTH                   0x08      // 8 Bytes
#define A31_EEPROM_FIRST_POWER_ON_OFFSET              0xE8      // First time power on offset 0xE8 - 0xEA
#define A31_EEPROM_FIRST_POWER_ON_LENGTH              0x03      // 3 Bytes
#define A31_EEPROM_FLASH_COMPLETE_OFFSET              0xED      // Flash Procedure complete offset 0xED
#define A31_EEPROM_FLASH_COMPLETE_LENGTH              0x01      // 1 Byte
#define A31_EEPROM_MFG_SHADOW_OFFSET                  0xEE      // MFG shadow offset 0xEE
#define A31_EEPROM_MFG_SHADOW_LENGTH                  0x01      // 1 Byte

//
// A32 EEPROM Offset/Length Definition
//
#define A32_EEPROM_SN_OFFSET                          0x00      // Electronic serial number offset 0x00 - 0x09
#define A32_EEPROM_SN_LENGTH                          0x0A      // 10 Bytes
#define A32_EEPROM_MODEL_ID_OFFSET                    0x0A      // Model ID offset 0x0A - 0x29
#define A32_EEPROM_MODEL_ID_LENGTH                    0x20      // 32 Bytes
#define A32_EEPROM_GUID_OFFSET                        0x2A      // GUID offset 0x2A - 0x31
#define A32_EEPROM_GUID_LENGTH                        0x08      // 8 Bytes
#define A32_EEPROM_UUID_OFFSET                        0x32      // UUID offset 0x32 - 0x41
#define A32_EEPROM_UUID_LENGTH                        0x10      // 16 Bytes
#define A32_EEPROM_SKU_NUMBER_OFFSET                  0x42      // Sku number offset 0x42 -0x4D
#define A32_EEPROM_SKU_NUMBER_LENGTH                  0x0C      // 12 Bytes
#define A32_EEPROM_LOC_OFFSET                         0x4E      // Localization code offset 0x4E - 0x50
#define A32_EEPROM_LOC_LENGTH                         0x03      // 3 Bytes
#define A32_EEPROM_LAN_MAC_ADDR_OFFSET                0x51      // LAN Mac Address offset 0x51 - 0x56
#define A32_EEPROM_LAN_MAC_ADDR_LENGTH                0x06      // 6 Bytes
#define A32_EEPROM_CONFIGBYTE1_PARAM_OFFSET           0x57      // ConfigByte1 Param offset 0x57 
#define A32_EEPROM_CONFIGBYTE1_PARAM_LENGTH           0x01      // 1 Byte
#define A32_EEPROM_CHANNEL_ID_OFFSET                  0x58      // Channel ID offset 0x58 
#define A32_EEPROM_CHANNEL_ID_LENGTH                  0x01      // 1 Byte
#define A32_EEPROM_OS_SKU_OFFSET                      0x59      // OS sku offset 0x59
#define A32_EEPROM_OS_SKU_LENGTH                      0x01      // 1 Byte
#define A32_EEPROM_QUICKPLAY_MODE_OFFSETD             0x5A      // Quick Play MODE offset 0x5A
#define A32_EEPROM_QUICKPLAY_MODE_LENGTH              0x01      // 1 Byte
#define A32_EEPROM_WALLPAPER_FLAG_OFFSET              0x5B      // Wallpaper flag offset 0x5B
#define A32_EEPROM_WALLPAPER_FLAG_LENGTH              0x01      // 1 Byte
#define A32_EEPROM_BURNING_SWITCH_OFFSET              0x5C      // Data Burning switch offset 0x5C
#define A32_EEPROM_BURNING_SWITCH_LENGTH              0x01      // 1 Byte
#define A32_EEPROM_WWAN_CARRIER_OFFSET                0x5D      // WWAN Carrier offset 0x5D
#define A32_EEPROM_WWAN_CARRIER_LENGTH                0x01      // 1 Byte
#define A32_EEPROM_WWAN_ANTENNA_OFFSET                0x5E      // WWAN Antenna offset 0x5E
#define A32_EEPROM_WWAN_ANTENNA_LENGTH                0x01      // 1 Byte
#define A32_EEPROM_GPS_ANTENNA_OFFSET                 0x5F      // GPS Antenna offset 0x5F
#define A32_EEPROM_GPS_ANTENNA_LENGTH                 0x01      // 1 Byte
#define A32_EEPROM_MEDIA_SMART_FLAG_OFFSET            0x60      // MediaSmart flag offset 0x60
#define A32_EEPROM_MEDIA_SMART_FLAG_LENGTH            0x01      // 1 Byte
#define A32_EEPROM_PLATFORM_REVERSION_OFFSET          0x61      // Platform Reversion offset 0x61 - 0x63
#define A32_EEPROM_PLATFORM_REVERSION_LENGTH          0x03      // 3 Bytes
#define A32_EEPROM_ODD_CONFIG_OFFSET                  0x64      // ODD Configuration offset 0x64
#define A32_EEPROM_ODD_CONFIG_LENGTH                  0x01      // 1 Byte
#define A32_EEPROM_WIN7_CONFIG_OFFSET                 0x65      // Win7 Configuration offset 0x65
#define A32_EEPROM_WIN7_CONFIG_LENGTH                 0x01      // 1 Byte
#define A32_EEPROM_IMAGE_ID_OFFSET                    0x66      // Image ID offset 0x66
#define A32_EEPROM_IMAGE_ID_LENGTH                    0x01      // 1 Byte
#define A32_EEPROM_WARRANTY_START_YEAR_OFFSET         0x67      // Warranty START YEAR offset 0x67 - 0x6A
#define A32_EEPROM_WARRANTY_START_YEAR_LENGTH         0x04      // 4 Bytes
#define A32_EEPROM_WARRANTY_START_MONTH_OFFSET        0x6B      // Warranty START Month offset 0x6B - 0x6C
#define A32_EEPROM_WARRANTY_START_MONTH_LENGTH        0x02      // 4 Bytes
#define A32_EEPROM_WARRANTY_START_DAY_OFFSET          0x6D      // Warranty START Day offset 0x6D - 0x6E
#define A32_EEPROM_WARRANTY_START_DAY_LENGTH          0x02      // 4 Bytes
#define A32_EEPROM_CT_NUMBER_OFFSET                   0x6F      // CT Number offset 0x6F - 0x81
#define A32_EEPROM_CT_NUMBER_LENGTH                   0x13      // 19 Bytes
#define A32_EEPROM_ESTAR_LOGO_OFFSET                  0x82      // eStar Logo offset 0x82 
#define A32_EEPROM_ESTAR_LOGO_LENGTH                  0x01      // 1 Byte
#define A32_EEPROM_SKU_NUMBER1_OFFSET                 0x83      // Sku number1 offset 0x83 - 0x8F 
#define A32_EEPROM_SKU_NUMBER1_LENGTH                 0x0D      // 13 Byte

//
// ABO EEPROM Offset/Length Definition
//
#define ABO_EEPROM_SERIAL_NUMBER_OFFSET               0x00      // Electronic serial number offset 0x00 - 0x1F    
#define ABO_EEPROM_SERIAL_NUMBER_LENGTH               0x16      // 22 Bytes
#define ABO_EEPROM_SPEAKER_TYPE_OFFSET                0x20      // Speaker type offset 0x20
#define ABO_EEPROM_SPEAKER_TYPE_LENGTH                0x01      // 1 Byte
#define ABO_EEPROM_PROJECT_ID_OFFSET                  0x30      // Project id offset 0x30
#define ABO_EEPROM_PROJECT_ID_LENGTH                  0x01      // 1 Byte
#define ABO_EEPROM_BOOT_LOGO_OFFSET                   0x3B      // Boot logo offset 0x3B
#define ABO_EEPROM_BOOT_LOGO_LENGTH	                  0x01      // 1 Byte
#define ABO_EEPROM_COUNTRYTYPE_OFFSET                 0x3C      // Country type offset 0x3C
#define ABO_EEPROM_COUNTRYTYPE_LENGTH                 0x01      // 1 Byte
#define ABO_EEPROM_COMPUTRACE_OFFSET                  0x3D      // Computrace offset 0x3D
#define ABO_EEPROM_COMPUTRACE_LENGTH                  0x01      // 1 Byte
#define ABO_EEPROM_MANUFACTURER_NAME_OFFSET           0x40      // Manufacture offset 0x40 - 0x4F    
#define ABO_EEPROM_MANUFACTURER_NAME_LENGTH           0x10      // 16 Bytes
#define ABO_EEPROM_UUID_OFFSET                        0x70      // UUID offset 0x70 - 0x7F   
#define ABO_EEPROM_UUID_LENGTH                        0x10      // 16 Bytes
#define ABO_EEPROM_PRODUCT_NAME_OFFSET                0x80      // Product name offset 0x80 - 0x9F   
#define ABO_EEPROM_PRODUCT_NAME_LENGTH                0x20      // 32 Bytes
#define ABO_EEPROM_ASSET_TAG_OFFSET                   0xA0      // Asset tag offset 0xA0 -0xBF   
#define ABO_EEPROM_ASSET_TAG_LENGTH                   0x16      // 22 Bytes
#define ABO_EEPROM_1394_OFFSET                        0xE0      // 1394 offset 0xE0 - 0xE7   
#define ABO_EEPROM_1394_LENGTH                        0x08      // 8 Bytes
#define ABO_EEPROM_SLP21_OFFSET                       0xE9      // SLP 2.1 offset 0xE9   
#define ABO_EEPROM_SLP21_LENGTH                       0x01      // 1 Byte
#define ABO_EEPROM_TOUCHPAD_ID_OFFSET                 0xEE      // Touchpad type offset 0xEE
#define ABO_EEPROM_TOUCHPAD_ID_LENGTH                 0x01      // 1 Byte
#define ABO_EEPROM_KEYBOARD_TYPE_OFFSET               0xF0      // Keyboard type offset 0xF0
#define ABO_EEPROM_KEYBOARD_TYPE_LENGTH               0x01      // 1 Byte
#define ABO_EEPROM_DESIGN_MODE_OFFSET                 0xF1      // Design mode offset 0xF1
#define ABO_EEPROM_DESIGN_MODE_LENGTH                 0x01      // 1 Byte
#define ABO_EEPROM_IRST_SETTING_OFFSET                0xF5      // iRST setting offset 0xF5
#define ABO_EEPROM_IRST_SETTING_LENGTH                0x01      // 1 Byte
#define ABO_EEPROM_TPM_SUPPORT_OFFSET                 0xF6      // TPM support offset 0xF6
#define ABO_EEPROM_TPM_SUPPORT_LENGTH                 0x01      // 1 Byte
#define ABO_EEPROM_TXT_SUPPORT_OFFSET                 0xF7      // TXT support offset 0xF7
#define ABO_EEPROM_TXT_SUPPORT_LENGTH                 0x01      // 1 Byte
#define ABO_EEPROM_ADAPTOR_TYPE_OFFSET                0xF9      // Adaptor type offset 0xF9
#define ABO_EEPROM_ADAPTOR_TYPE_LENGTH                0x01      // 1 Byte
#define ABO_EEPROM_TPM_PROVISION_OFFSET               0xFC      // TPM provision offset 0xFC
#define ABO_EEPROM_TPM_PROVISION_LENGTH               0x01      // 1 Byte

//
// C38 EEPROM Offset/Length Definition
//
#define C38_EEPROM_ODM_BOARD_NUMBER_OFFSE             0x00      // ODM board number offset 0x00 - 0x1F
#define C38_EEPROM_ODM_BOARD_NUMBER_LENGTH            0x20      // 32 Bytes
#define C38_EEPROM_MAC_ADDR_OFFSE                     0x40      // Mac Address offset 0x40 - 0x45
#define C38_EEPROM_MAC_ADDR_LENGTH                    0x06      // 6 Bytes
#define C38_EEPROM_PRODUCT_NUMBER_OFFSET              0x50      // Product number offset 0x50 - 0x6F
#define C38_EEPROM_PRODUCT_NUMBER_LENGTH              0x20      // 20 Bytes
#define C38_EEPROM_UUID_OFFSET                        0x70      // UUID offset 0x70 - 0x7F
#define C38_EEPROM_UUID_LENGTH                        0x10      // 16 Bytes
#define C38_EEPROM_MACHINE_TYPE_MODEL_OFFSET          0x80      // UUID offset 0x80 - 0x9F
#define C38_EEPROM_MACHINE_TYPE_MODEL_LENGTH          0x20      // 32 Bytes
#define C38_EEPROM_SN_OFFSET                          0xA0      // Electronic serial number offset 0xA0 - 0xBF
#define C38_EEPROM_SN_LENGTH                          0x20      // 32 Bytes
#define C38_EEPROM_MANUFATURE_FLAG_OFFSET             0xDE      // Manufacture flag offset 0xDE
#define C38_EEPROM_MANUFATURE_FLAG_LENGTH             0x01      // 1 Byte
#define C38_EEPROM_RANDOM_NUMBER_OFFSET               0xDF      // Random number offset 0xDF
#define C38_EEPROM_RANDOM_NUMBER_LENGTH               0x01      // 1 Byte
#define C38_EEPROM_1394_GUID_OFFSET                   0xE0      // 1394 GUID offset 0xE0 - 0xE7
#define C38_EEPROM_1394_GUID_LENGTH                   0x08      // 8 Bytes
#define C38_EEPROM_VGA_TYPE_OFFSET                    0xE8      // VGA type offset - 0xE8
#define C38_EEPROM_VGA_TYPE_LENGTH                    0x01      // 1 Bytes
#define C38_EEPROM_ENERGY_STAR_FLAG_OFFSET            0xE9      // Eeprom modify flag offset - 0xE9
#define C38_EEPROM_ENERGY_STAR_FLAG_LENGTH            0x01      // 1 Bytes
#define C38_EEPROM_TOUCHPAD_ID_OFFSET                 0xEF      // TouchPad ID offset - 0xEF
#define C38_EEPROM_TOUCHPAD_ID_LENGTH                 0x01      // 1 Bytes
#define C38_EEPROM_KEYBOARD_TYPE_OFFSET               0xF0      // Keyboar type offset - 0xF0
#define C38_EEPROM_KEYBOARD_TYPE_LENGTH               0x01      // 1 Bytes

//
// T66 EEPROM Offset/Length Definition
//
#define T66_EEPROM_SYSTEM_SN_OFFSET                   0x00      // System serial number offset 0x00 - 0x1F
#define T66_EEPROM_SYSTEM_SN_LENGTH                   0x20      // 32 Bytes
#define T66_EEPROM_BOARD_SN_OFFSET                    0x20      // Board serial number offset 0x20 - 0x3F
#define T66_EEPROM_BOARD_SN_LENGTH                    0x20      // 32 Bytes
#define T66_EEPROM_PRODUCT_NUMBER_OFFSET              0x80      // Product number offset 0x80 - 0x9F
#define T66_EEPROM_PRODUCT_NUMBER_LENGTH              0x20      // 32 Bytes
//[PRJ]++ >>>> Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
#define T66_EEPROM_OFFSET_ALS_CALIBRATION             0x1A0     // ALS Calibration
#define T66_EEPROM_LENGTH_ALS_CALIBRATION             0x08      // 8 Bytes
#define T66_EEPROM_OFFSET_SKU_NUM                     0x110     // SKU Number offset 0x110 - 0x11F
#define T66_EEPROM_OFFSET_ASSERT_TAG_NUM              0x200     // Asset Tag Number offset 0x200 - 0x23F
#define T66_EEPROM_LENGTH_SKU_NUM                     0x10      // 16 Bytes
#define T66_EEPROM_LENGTH_ASSERT_TAG_NUM              0x40      // 64 Bytes
//[PRJ]++ <<<< Modify for support VirtualEEPROMVerifyTool and CMFCVerify   
//
// T88 EEPROM Offset/Length Definition
//
#define T88_EEPROM_SN_OFFSET                          0x00      // Electronic serial number offset 0x00 - 0x3F
#define T88_EEPROM_SN_LENGTH                          0x40      // 64 Bytes
#define T88_EEPROM_MB_VERSION_OFFSET                  0x50      // M/B Version offset 0x50 - 0x53
#define T88_EEPROM_MB_VERSION_LENGTH                  0x04      // 4 Bytes
#define T88_EEPROM_MAC_ADDR_OFFSET                    0x6A      // Mac Address offset 0x6A - 0x6F
#define T88_EEPROM_MAC_ADDR_LENGTH                    0x06      // 6 Bytes
#define T88_EEPROM_OEM_STRING_OFFSET                  0xB0      // OEM string offset 0xB0 - 0xCB
#define T88_EEPROM_OEM_STRING_LENGTH                  0x1C      // 28 Bytes
#define T88_EEPROM_DMI_TYPE11_STRING_OFFSET           0xCC      // DMI Type11 string offset 0xB0 - 0xCB
#define T88_EEPROM_DMI_TYPE11_STRING_LENGTH           0x14      // 20 Bytes
#define T88_EEPROM_BUILD_STAGE_OFFSET                 0xE0      // Build stage offset 0xE0
#define T88_EEPROM_BUILD_STAGE_LENGTH                 0x01      // 1 Bytes
#define T88_EEPROM_SHIPPING_OS_OFFSET                 0xE1      // Shipping OS offset 0xE1
#define T88_EEPROM_SHIPPING_OS_LENGTH                 0x01      // 1 Bytes
#define T88_EEPROM_CIR_FLAG_OFFSET                    0xE2      // CIR flag offset 0xE1
#define T88_EEPROM_CIR_FLAG_LENGTH                    0x01      // 1 Bytes

#endif
