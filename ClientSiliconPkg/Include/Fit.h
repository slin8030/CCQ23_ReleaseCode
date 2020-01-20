/** @file
  This file contains definitions for FIT table entries
  including error string definitions

@copyright
 Copyright (c) 2015 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains 'Framework Code' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may not be modified, except as allowed by
 additional terms of your license agreement.

@par Specification
**/

#ifndef __FIT_H__
#define __FIT_H__
//
// FIT definition
//
#define FIT_TABLE_TYPE_HEADER         0
#define FIT_TABLE_TYPE_MICROCODE      1
#define FIT_TABLE_TYPE_STARTUP_ACM    2
#define FIT_TABLE_TYPE_BIOS_MODULE    7
#define FIT_TABLE_TYPE_TPM_POLICY     8
#define FIT_TABLE_TYPE_BIOS_POLICY    9
#define FIT_TABLE_TYPE_TXT_POLICY     10
#define FIT_TABLE_TYPE_SKIP           0x7F

#define FIT_TABLE_TYPE_KEY_MANIFEST           11
#define FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST   12

typedef struct {
  UINT64 Address;
  UINT8  Size[3];
  UINT8  Reserved;
  UINT16 Version;
  UINT8  Type : 7;
  UINT8  Cv   : 1;
  UINT8  Chksum;
} FIRMWARE_INTERFACE_TABLE_ENTRY;
#endif
