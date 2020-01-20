/** @file
  CPU policy PPI produced by a platform driver specifying various
  expected CPU settings. This PPI is consumed by CPU PEI modules.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
#ifndef _BIOS_GUARD_CONFIG_H_
#define _BIOS_GUARD_CONFIG_H_

#include "BiosGuardDefinitions.h"
#include <Library/BootGuardLib.h>

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gBiosGuardConfigGuid;

#pragma pack(push, 1)

#define BIOS_GUARD_CONFIG_REVISION 1

/**
  Platform policies for BIOS Guard Configuration for all processor security features configuration.
  Platform code can pass relevant configuration data through this structure.
  @note The policies are marked are either <b>(Required)</b> or <b>(Optional)</b>
  - <b>(Required)</b> : This policy is recommended to be properly configured for proper functioning of reference code and silicon initialization
  - <b>(Optional)</b> : This policy is recommended for validation purpose only.
**/
typedef struct {
  CONFIG_BLOCK_HEADER Header;             ///< Offset 0 GUID number for main entry of config block
  BGUP_HEADER          BgupHeader;         ///< BIOS Guard update package header that will be packaged along with BIOS Guard script and update data.
  BGPDT                Bgpdt;              ///< BIOS Guard Platform Data Table contains all the platform data that will be parsed by BIOS Guard module.
  UINT64               BgpdtHash[4];       ///< Hash of the BGPDT that will be programmed to PLAT_FRMW_PROT_HASH_0/1/2/3 MSR.
  UINT8                EcCmdDiscovery;     ///< EC Command discovery.
  UINT8                EcCmdProvisionEav;  ///< EC Command Provision Eav.
  UINT8                EcCmdLock;          ///< EC Command Lock.
  UINT8                Rsvd;               ///< Reserved for DWORD alignment.
  BIOSGUARD_LOG        BiosGuardLog;       ///< BIOS Guard log.
} BIOS_GUARD_CONFIG;

#pragma pack(pop)
#endif
