/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _ADMINISTER_SECURE_BOOT_H_
#define _ADMINISTER_SECURE_BOOT_H_

//
// Globally "AdministerSecureBoot" variable to specify whether the system is currently booting
// in administer secure boot enable (1) or not (0). All other values are reserved.
//
#define EFI_ADMINISTER_SECURE_BOOT_NAME        L"AdministerSecureBoot"

//
// Globally "SecureBootEnforce" variable to specify whether the system is currently booting
// with image verification (1) or not (0) in user mode. All other values are reserved.
//
#define EFI_SECURE_BOOT_ENFORCE_NAME           L"SecureBootEnforce"

//
// Globally "CustomSecurity" variable to specify whether the security database has been customized
// by user. Security database has been customized (1) or not (0). All other values are reserved.
//
#define EFI_CUSTOM_SECURITY_NAME               L"CustomSecurity"

//
// Globally "RestoreFactoryDefault" variable to specify whether the system is currently booting
// with resotring to factory default (1) or not (0). All other values are reserved.
//
#define EFI_RESTORE_FACOTRY_DEFAULT_NAME       L"RestoreFactoryDefault"


//
// Following are variable definitions for non-smi system
//

//
// Globally "AddHashImage" variable to add hash image to EFI_IMAGE_SECURITY_DATABASE (db)
//
#define EFI_ADD_HASH_IMMAGE_NAME             L"AddHashImage"
//
// Globally "ClearAllSecureSettings" variable to clear all secure boot data. Value (1)
// clears all secure boot data and system doesn't set this variable to variable storage.
// All other values are reserved.
//
#define EFI_CLEAR_ALL_SECURE_SETTINGS_NAME   L"ClearAllSecureSettings"
//
// Globally "ClearDeployedMode" variable to clear DeployedMode variable. Value (1)
// clears DeployedMode variable to 0 and system doesn't set this variable to variable storage.
// All other values are reserved.
//
#define EFI_CLEAR_DEPLOYED_MODE_NAME   L"ClearDeployedMode"

//
// Administer secure boot SMI relative sub-functions
//
#define ADD_HASH_IMAGE_FUN_NUM                 0x01
#define SECURE_BOOT_ENFORCE_FUN_NUM            0x02
#define CLEAR_ALL_SECURE_SETTINGS_FUN_NUM      0x03
#define RESTORE_FACOTRY_DEFAULT_FUN_NUM        0x04
#define UPDATE_PK_FUN_NUM                      0x05
#define UPDATE_KEK_FUN_NUM                     0x06
#define UPDATE_DB_FUN_NUM                      0x07
#define UPDATE_DBX_FUN_NUM                     0x08
#define CLEAR_DEPLOYED_MODE_VALUE_FUN_NUM      0x09

#define UPDATE_AUTHENTICATED_VARIABLE          0x00
#define APPEND_AUTHENTICATED_VARIABLE          0x01

#define PLATFORM_DISABLE_SECURE_BOOT_GUID \
  {0xf6168400, 0x2d9e, 0x4725, 0x93, 0x80, 0xf9, 0x1b, 0x7b, 0x78, 0x3c, 0x2e}

extern EFI_GUID gPlatformDisableSecureBootGuid;

#endif
