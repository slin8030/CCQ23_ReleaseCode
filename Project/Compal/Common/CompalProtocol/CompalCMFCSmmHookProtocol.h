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
 1.01      12/26/14  Champion Liu Add CMFC new function for CMFC spec 00.00.13.
           01/05/14  Champion Liu Add CMFC 0x14, sub_fun 0x05 for 00.00.15 spec.
 1.02      01/08/14  Champion Liu Merge CMFC 0x24/0x25 function.
           01/15/14  Champion Liu Add CMFC 0x14, sub_fun 0x03/0x04 for 00.00.14 spec.
           02/02/15  Champion Liu Add CMFC 0x14, sub_fun 0x06 for 00.00.16 spec.
 1.03      06/23/15  Champion Liu Add CMFC 0x21, sub_fun 0x11/0x12 for 00.00.17 spec.
           08/14/15  Champion Liu Add CMFC 0x21, sub_fun 0x13/0x14 for 00.00.18 spec.
 1.04      09/02/15  Champion Liu Release CompalCMFC version to 1.04 for 00.00.18 spec.
*/

#ifndef _COMPAL_CMFC_SMM_HOOK_PROTOCOL_H_
#define _COMPAL_CMFC_SMM_HOOK_PROTOCOL_H_

//
// COMPAL CMFC SMM HOOK PROTOCOL GUID
//
#define COMPAL_CMFC_SMM_HOOK_PROTOCOL_GUID \
  { \
    0xcbeb63d0, 0x76e3, 0x11e0, 0xa1, 0xf0, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66  \
  }

typedef struct _COMPAL_CMFC_SMM_HOOK_PROTOCOL COMPAL_CMFC_SMM_HOOK_PROTOCOL;

typedef enum {
    CMFC_GET_BOOT_DEVICE_SEQUENCE,
    CMFC_SET_BOOT_DEVICE_SEQUENCE,
    CMFC_GET_BOOT_DEVICE_GUID,
    CMFC_GET_BOOT_DEVICE_NAME,
    CMFC_SET_BOOT_DEVICE_VALUE,
    CMFC_BIOS_LOAD_DEFUALT,
    CMFC_BIOS_DEFUALT_GUID,
    CMFC_BIOS_DEFUALT_NAME,
    CMFC_BIOS_EEPROM_GUID,
    CMFC_BIOS_EEPROM_NAME,
    CMFC_GET_MFG_RESERVED_AREA,
    CMFC_GET_ONBOARD_SPD_DATA,
    CMFC_BOOT_TYPE_Csm,
    CMFC_BOOT_TYPE_UEFI,
    CMFC_BOOT_TYPE_WPEAndPXEB,
    CMFC_BOOT_TYPE_WPEAndNetB,
    CMFC_BOOT_TYPE_DUAL,
    CMFC_PXE_BOOT_TO_LAN_ENABLED,
    CMFC_SET_SMT_DEFAULT,
    CMFC_GET_BOOT_TYPE,
    CMFC_SET_BOOT_TYPE,
    CMFC_SECURE_BOOT_DIS,
    CMFC_SECURE_BOOT_ENA,
    CMFC_SECURE_BOOT_GUID,
    CMFC_SECURE_BOOT_NAME,
    CMFC_SECURE_BOOT_VALUE,
    CMFC_GET_SECURE_BOOT,
    CMFC_GET_TPM_STATUS,
    CMFC_SET_TPM_STATUS,
    CMFC_GET_DPTF_STATUS,
    CMFC_SET_DPTF_STATUS,
    CMFC_OS_TYPE_NonOS,
    CMFC_OS_TYPE_Linux,
    CMFC_OS_TYPE_WinXP,
    CMFC_OS_TYPE_Vista,
    CMFC_OS_TYPE_Win7,
    CMFC_OS_TYPE_Win8,
    CMFC_OS_TYPE_Win8Pro,
    CMFC_OS_TYPE_Win7Win8,
    CMFC_GET_OS_TYPE,
    CMFC_SET_OS_TYPE,
    CMFC_GET_VENDOR_NAME,
    CMFC_GET_UUID_GUID,
    CMFC_GET_SERIALNAME_GUID,
    CMFC_GET_PRODUCTNAME_GUID,
    CMFC_GET_MBSERIALNUM_GUID,
    CMFC_GET_OSTYPE_GUID,
    CMFC_GET_UUID_NAME,
    CMFC_GET_SERIALNUM_NAME,
    CMFC_GET_PRODUCTNAME_NAME,
    CMFC_GET_MBSERIALNUM_NAME,
    CMFC_GET_OSTYPE_NAME,
    CMFC_CLEAR_SUPERVISOR_PASSWORD,
    CMFC_CLEAR_USER_PASSWORD,
    CMFC_CHECK_SUPERVISOR_PASSWORD,
    CMFC_CHECK_USER_PASSWORD,
    CMFC_CHECK_HD_PASSWORD,
    CMFC_GET_AC_CONNECT_STATUS,
    CMFC_GET_BAT_STATUS,
    CMFC_GET_BAT_CONNECT_STATUS,
    CMFC_GET_BAT_DESIGN_CAPACITY,
    CMFC_GET_BAT_FULL_CHARGE_CAPACITY,
    CMFC_GET_BAT_REMAINING_CAPACITY,
    CMFC_GET_BAT_CHARGE_CURRENT,
    CMFC_GET_BAT_CURRENT,
    CMFC_GET_BAT_CYCLE_COUNTER,
    CMFC_GET_BAT_AVERAGE_TEMPERATURE,
    CMFC_SET_AC_POWER_SOURCE,
    CMFC_SHIPPING_MODE_ENABLE,
    CMFC_SYSTEM_FULL_CHARGE_LED_ON,
    CMFC_SYSTEM_CHARGE_LED_ON,
    CMFC_ALL_LED_ON,
    CMFC_LED_OUT_OF_TEST_MODE,
    CMFC_SET_LED_ON_OFF_BLINK,
    CMFC_GET_LED_STATUS,
    CMFC_A31_FUNC_90,
    CMFC_A31_FUNC_91,
    CMFC_A31_GET_EDID_OR_PN,
    CMFC_A32_FUNC_92,
    CMFC_A32_FUNC_93,
    CMFC_ABO_FUNC_94,
    CMFC_ABO_FUNC_95,
    CMFC_A51_FUNC_96,
    CMFC_A51_FUNC_97,
    CMFC_C38_FUNC_98,
    CMFC_C38_FUNC_99
} COMPAL_SW_SMI_COMMON_SMM_HOOK_ID;

//
// Member functions
//
typedef
EFI_STATUS
(EFIAPI *COMPAL_CMFC_SMM_OVERRIDE) (
    IN     COMPAL_CMFC_SMM_HOOK_PROTOCOL       * This,
    IN     COMPAL_SW_SMI_COMMON_SMM_HOOK_ID    OverrideType,
    ...
);

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gCompalCMFCSmmHookProtocolGuid;

//
// CMFC SMM HOOK PROTOCOL
//
typedef struct _COMPAL_CMFC_SMM_HOOK_PROTOCOL {
    COMPAL_CMFC_SMM_OVERRIDE  CompalCMFCSMMHookDispatch;
} COMPAL_CMFC_SMM_HOOK_PROTOCOL;

#endif
