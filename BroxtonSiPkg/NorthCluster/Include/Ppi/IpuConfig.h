/** @file
  IPU policy definitions

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

@par Specification Reference:
**/
#ifndef _IPU_CONFIG_H_
#define _IPU_CONFIG_H_

#pragma pack(1)

#define IPU_CONFIG_REVISION 1

extern EFI_GUID gIpuConfigGuid;

typedef struct {
  CONFIG_BLOCK_HEADER  Header;     ///< Offset 0-23 Config Block Header
  UINT8  SaIpuEnable;              ///< Enable SA IPU device: 0=Disable, <b>1=Enable</b>
  UINT8  IpuAcpiMode;              ///< Set IPU ACPI mode: <b>0=AUTO</b>, 1=IGFX Child device, 2=ACPI device
  UINT32 IpuMmAdr;                 ///< Address of IPU MMIO Bar IpuMmAdr: Default is <b>0x90000000</b>
} IPU_CONFIG;

#pragma pack()

#endif // _IPU_CONFIG_H_
