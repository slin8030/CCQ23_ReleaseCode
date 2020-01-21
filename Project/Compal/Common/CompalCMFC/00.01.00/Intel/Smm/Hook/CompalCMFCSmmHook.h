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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#ifndef _COMPAL_CMFC_SMM_HOOK_H_
#define _COMPAL_CMFC_SMM_HOOK_H_

#include <CompalEclib.h>
#include <CompalCMFCSmmHookProtocol.h>
#include <CompalGlobalNvsProtocol.h>
#include <Protocol/SmmBase2.h>
#include <SetupConfig.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/SmmVariable.h>

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
#define SwSmiNotSupport   0x00
#define SwSmiSuccess      0x01
#define SwSmiFail         0x02

#define  BootToDUAL       0x00
#define  BootToCSM        0x01
#define  BootToUEFI       0x02

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
CMFCGetBootType (
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
CMFCGetVendorName (
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

VOID
Memcpy (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
);

#endif
