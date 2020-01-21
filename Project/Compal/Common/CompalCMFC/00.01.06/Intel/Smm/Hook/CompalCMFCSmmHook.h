/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
 1.01      12/26/14  Champion Liu Add CMFC new function for CMFC spec 00.00.13.
           01/05/15  Champion Liu Add CMFC 0x14, sub_fun 0x05 for 00.00.15 spec.
 1.02      01/08/15  Champion Liu Merge CMFC 0x24/0x25 function.
           01/15/15  Champion Liu Add CMFC 0x14, sub_fun 0x03/0x04 for 00.00.14 spec.
           02/02/15  Champion Liu Add CMFC 0x14, sub_fun 0x06 for 00.00.16 spec.
           02/09/15  Champion Liu Modify to access EC EEPPROM by "COMPAL_EEPROM_SMM_PROTOCOL" method.
 1.03      06/23/15  Champion Liu Add CMFC 0x21, sub_fun 0x11/0x12 for 00.00.17 spec.
           06/26/15  Champion Liu Add CMFC 0x20, sub_fun 0x11, "USB Boot/ODD Boot" function for 00.00.17 spec.
           08/14/15  Champion Liu Add CMFC 0x21, sub_fun 0x13/0x14 for 00.00.18 spec.
 1.04      09/02/15  Champion Liu Release CompalCMFC version to 1.04 for 00.00.18 spec.
*/

#ifndef _COMPAL_CMFC_SMM_HOOK_H_
#define _COMPAL_CMFC_SMM_HOOK_H_

#include <CompalEclib.h>
#include <CompalCMFCSmmHookProtocol.h>
#include <CompalEEPROMSmmProtocol.h>
#include <CompalGlobalNvsProtocol.h>
#include <Protocol/SmmBase2.h>
#include <SetupConfig.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/SmmVariable.h>
#include <Library/PcdLib.h>

extern EFI_GUID  gEfiGenericVariableGuid;
extern EFI_GUID  gEfiGlobalVariableGuid;

extern EFI_BOOT_SERVICES  *gBS;

//
// Define BBS Device Types (Insyde)
//
#define BBS_FLOPPY        0x01
#define BBS_HARDDISK      0x02
#define BBS_CDROM         0x03
#define BBS_USB_HARDDISK  0x04
#define BBS_USB           0x05
#define BBS_EMBED_NETWORK 0x06
#define BBS_BEV_DEVICE    0x80
#define BBS_UNKNOWN       0xff

//
// Define CMFC Device Types
//
#define  HDD0             0x01
#define  CDROM            0x02
#define  LAN              0x03
#define  USBFloppy        0x04
#define  ESATA            0x1E
#define  USBCDROM         0x18
#define  USBHDD           0x18
#define  HDD1             0x1D
//
// Define SMI Status
//
#define SwSmiFail         0x0000
#define SwSmiSuccess      0x0001
#define SwSmiNotSupport   0x8000

#define  BootToDUAL       0x00
#define  BootToCSM        0x01
#define  BootToUEFI       0x02

#define OEM_EEPROM_READ_FROM_EC                0x00

#define  OEM_EEPROM_BOOT_TYPE_OFFSET           0xF3
#define    OEM_EEPROM_PXE_BOOT_TO_LAN_ENABLED  0x07
#define    OEM_EEPROM_SET_SMT_DEFAULT          0xFF

#define  OEM_EEPROM_OSTYPE_FLAG                0xEC
  #define  OEM_EEPROM_OSTYPE_NonOS             0x01
  #define  OEM_EEPROM_OSTYPE_Linux             0x02
  #define  OEM_EEPROM_OSTYPE_WinXP             0x03
  #define  OEM_EEPROM_OSTYPE_Vista             0x04
  #define  OEM_EEPROM_OSTYPE_Win7              0x05
  #define  OEM_EEPROM_OSTYPE_Win8              0x06
  #define  OEM_EEPROM_OSTYPE_Win8Pro           0x07
  #define  OEM_EEPROM_OSTYPE_Win7Win8          0x08

#define EC_NAME_BAT_CONNECT_OFFSET             0xBE     // BIT[0]
#define EC_NAME_AC_CONNECT_OFFSET              0xA3     // BIT[7]
#define EC_NAME_BAT_INFO_OFFSET                0xC0     // BIT[7:0]
#define EC_NAME_BAT_DESIGN_CAPACITY_LOW        0xCA     // BIT[7:0]
#define EC_NAME_BAT_DESIGN_CAPACITY_HIGH       0xCB     // BIT[7:0]
#define EC_NAME_BAT_FULL_CHARGE_CAPACITY_LOW   0xCC     // BIT[7:0]
#define EC_NAME_BAT_FULL_CHARGE_CAPACITY_HIGH  0xCD     // BIT[7:0]
#define EC_NAME_BAT_REMAINING_CAPACITY_LOW     0xC2     // BIT[7:0]
#define EC_NAME_BAT_REMAINING_CAPACITY_HIGH    0xC3     // BIT[7:0]
#define EC_NAME_BAT_CHARGE_CURRENT_LOW         0xD8     // BIT[7:0]
#define EC_NAME_BAT_CHARGE_CURRENT_HIGH        0xD9     // BIT[7:0]
#define EC_NAME_BAT_CURRENT_LOW                0xD0     // BIT[7:0]
#define EC_NAME_BAT_CURRENT_HIGH               0xD1     // BIT[7:0]
#define EC_NAME_BAT_CYCLE_COUNTER              0xCF     // BIT[7:0]
#define EC_NAME_BAT_AVERAGE_TEMPERATURE        0xD7     // BIT[7:0]

#define  A31_SET_COLD_BOOT                     0x0001
#define  A31_SET_WARM_BOOT                     0x0002
#define  A31_ERASE_EEPROM                      0x0003
#define  A31_GET_COMPUTRACE_STATUS             0x0004
#define  A31_ENABLE_COMPUTRACE                 0x0005
#define  A31_DISABLE_COMPUTRACE                0x0006
#define  A31_RESTORE_COMPUTRACE                0x0007
#define  A31_DETECT_TPM                        0x0008

#define  A32_WRITE_EEPROM                      0x0001
#define  A32_READ_EEPROM                       0x0002
#define  A32_DISABLE_PXE_BOOT                  0x0003
#define  A32_ENABLE_PXE_BOOT                   0x0004
#define  A32_DISABLE_ISCT_BOOT                 0x0005
#define  A32_ENABLE_ISCT_BOOT                  0x0006
#define  A32_DISABLE_IRST_BOOT                 0x0007
#define  A32_ENABLE_IRST_BOOT                  0x0008
#define  A32_DISABLE_LEGACY_SUPPORT            0x0009
#define  A32_ENABLE_LEGACY_SUPPORT             0x000A
#define  A32_RELAD_DEFAULT                     0x000B
#define  A32_WRITE_FACTORY_MODE                0x000C
#define  A32_SET_ALL_WIRELESS_ON               0x000D
#define  A32_HDD_BOOT_FIRST                    0x000E
#define  A32_LAN_BOOT_FIRST                    0x000F
#define  A32_CLEAR_ERROR_LOG                   0x0010
#define  A32_PCID_VERSION                      0x0011
#define  A32_ERROR_LOG_BASE_ADDRESS            0x0012
#define  A32_CMFC_VERSION                      0x0013
#define  A32_VGA_TYPE                          0x0014
#define  A32_PSENSOR_STATUS                    0x0015

#define  ABO_GET_CPU_VPRO_STATUS               0x0001

#define  C38_GET_EC_VERSION                    0x0001
#define  C38_GET_EC_REVISION_NUMBER            0x0002
#define  C38_CLEAR_BATT_FIRST_USING_TIME       0x0010
#define  C38_CTRL_BATT_CAP_TO_ARG2_DEFINE      0x0011
#define  C38_DISABLE_CTRL_BATT_CAPACITY        0x0012
#define  C38_ENABLE_VIBRATOR                   0x0020
#define  C38_DISABLE_VIBRATOR                  0x0021
#define  C38_ENABLE_USB_20Y_CABLE              0x0022
#define  C38_DISABLE_USB_20Y_CABLE             0x0023
#define  C38_GET_1ST_PSENSOR_STATUS            0x0024
#define  C38_GET_2ND_PSENSOR_STATUS            0x0025
#define  C38_GET_DOCKING_STATUS                0x0026
#define  C38_GET_PANEL_TYPE                    0x0027
#define  C38_GET_GUID_OF_NFC                   0x0030
#define  C38_GET_GUID_OF_ROLLBACK              0x0031
#define  C38_GET_GUID_OF_MTM                   0x0032
#define  C38_GET_GUID_OF_OS_LICENSE            0x0033
#define  C38_GET_VARIABLE_NAME_OF_NFC          0x0040
#define  C38_GET_VARIABLE_NAME_OF_ROLLBACK     0x0041
#define  C38_GET_VARIABLE_NAME_OF_MTM          0x0042
#define  C38_GET_VARIABLE_NAME_OF_OS_LICENSE   0x0043
#define  C38_RET_BIT_OFFSET_OF_DIGITIZER       0x0050
#define  C38_RET_BIT_OFFSET_OF_NFC             0x0051
#define  C38_RET_BIT_OFFSET_OF_FINGERPRINT     0x0052
#define  C38_RET_BIT_OFFSET_OF_AUTO_DETECT     0x0053
#define  C38_RET_BYTE_OFFSET_OF_ROLLBACK       0x0054
#define  C38_DISABLE_OS_OPTIMIZED_DEFAULT      0x0060
#define  C38_ENABLE_OS_OPTIMIZED_DEFAULT       0x0061
#define  C38_GET_OS_OPTIMIZED_DEFAULT          0x0062

EFI_STATUS
EFIAPI
CMFCSwSMIHookMain (
    IN     COMPAL_CMFC_SMM_HOOK_PROTOCOL       *This,
    IN     COMPAL_SW_SMI_COMMON_SMM_HOOK_ID    OverrideType,
    ...
);

EFI_STATUS
CMFCGetBootDeviceSequence (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootDeviceSequence (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBootDeviceGUID(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBootDeviceNAME(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootDeviceValue(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCLoadBiosDefualt (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCLoadDefualtGUID (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCLoadDefualtName (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCLoadEepromGUID (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCLoadEepromName (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetMFGReservedArea (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetOnBoardSPDdata(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeCsm (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeUEFI (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeWPEAndPXEB (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeWPEAndNetB (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeDUAL (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetPxeBootToLanEnabled(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetSmtDefault(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeCSMBAndUSBB(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootTypeCSMBAndODDB(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBootType (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetBootType (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetTPMStatus (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetTPMStatus (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetDPTFStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetDPTFStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetSecureBootDisable(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetSecureBootEnable(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetSecureBootGUID(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetSecureBootNAME(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetSecureBootValue(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetSecureBoot (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeNonOS(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeLinux(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeWinXP(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeVista(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeWin7(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeWin8(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeWin8Pro(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSTypeWin7Win8(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetOSType (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetOSType (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetVendorName (
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetUUIDGuid(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetSerialNumGuid(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetProductNameGuid(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetMBSerialNumGuid(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetOSTypeGuid(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetUUIDName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetSerialNumName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetProductName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetMBSerialNumName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetOSTypeName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCClearSupervisorPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCClearUserPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCCheckSupervisorPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCCheckUserPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCCheckHDPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetACConnectStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatConnectStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatDesignCapacity(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatFullChargeCapacity(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatRemainingCapacity(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatChargeCurrent(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatCurrent(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatCycleCounter(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetBatAverageTemperature(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetACPowerSource(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCShippingModeEnable(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSystemFullChargeLEDOn(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSystemChargeLEDOn(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCAllLEDOn(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCLEDOutOfTestMode(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCSetLEDOnOffBlink(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCGetLEDStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA31Func90(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA31Func91(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA31GetEDIDOrPN(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA32Func92(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA32Func93(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCABOFunc94(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCABOFunc95(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA51Func96(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCA51Func97(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCC38Func98(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

EFI_STATUS
CMFCC38Func99(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
);

VOID
Memcpy (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
);

#endif
