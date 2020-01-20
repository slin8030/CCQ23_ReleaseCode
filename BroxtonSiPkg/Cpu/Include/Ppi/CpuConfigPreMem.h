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
#ifndef _CPU_CONFIG_PREMEM_H_
#define _CPU_CONFIG_PREMEM_H_


//
// Extern the GUID for protocol users.
//
extern EFI_GUID gCpuConfigPreMemGuid;

#pragma pack(push, 1)

//
// CPU_CONFIG_PREMEM revisions
//
#define CPU_CONFIG_PREMEM_REVISION 1

/**
  Platform Policies for CPU features configuration Platform code can enable/disable/configure features through this structure.
**/
typedef struct {
  CONFIG_BLOCK_HEADER Header;                ///< Offset 0 GUID number for main entry of config block
  //UINT8                Reserved[4];           ///< Offset Reserved for future use
  UINT32 BistOnReset                     : 1; ///< Enable or Disable BIST on Reset; <b>0: Disable</b>; 1 Enable.
  UINT32 Txt                             : 1; ///< Enable or Disable TXT; <b>0: Disable</b>; 1: Enable.
  UINT32 SkipStopPbet                    : 1; ///< Skip Stop PBET Timer <b>0: Disabled</b>, 1: Enabled
  UINT32 BiosGuard                       : 1; ///< Enable or Disable BIOS Guard; 0: Disable, <b>1:Enable</b>
  UINT32 EnableC6Dram                    : 1; ///< C6DRAM Memory. <b>0: C6DRAM Disabled</b>; 1: C6DRAM Enabled.
  /**
  SGX feature configuration. <b>0: SGX Disabled</b>, 1: SGX Enabled.
  Use below logic to set this policy
  @dot
  digraph G {
    subgraph cluster_c0 {
      node [shape = box];
      b1[label="Disable SGX policy" fontsize=12 style=filled color=lightblue];
      b2[label="Delete EPC_BIOS_CONFIG variable" fontsize=12 style=filled color=lightblue];
      b3e[label="Enable SGX policy" fontsize=12 style=filled color=lightblue];
      b3a[label="Enable SGX policy" fontsize=12 style=filled color=lightblue];
      b4[label="Create/Update EPC_BIOS_CONFIG" fontsize=12 style=filled color=lightblue];
      b5[label="Continue booting" fontsize=12 style=filled color=lightblue];
      b6[label="Max (EPC_OS_CONFIG.EpcSize, PRMRR size policy)" fontsize=12 style=filled color=lightblue];

      node [shape = diamond,style=filled,color=lightblue];
      d1[label="BIOS setup option" fontsize=12];
      d2[label="EPC_OS_CONFIG variable present?" fontsize=12];
      d3[label="EPC_OS_CONFIG.EpcSize valid?" fontsize=12];
      d4a[label="PRMRR size policy valid?" fontsize=12 style=filled color=lightblue];
      d4e[label="PRMRR size policy valid?" fontsize=12 style=filled color=lightblue];
      d7[label="EPC_BIOS_CONFIG variable present?" fontsize=12];

      d1 -> b1 [label = "Disable"]
      d1 -> d4e [label = "Enable"]
      d1 -> d2 [label = "Software Controlled"]

      subgraph cluster_enabledisable {
        node[style=filled];
        b1 -> d7
        d7 -> b2 [label = "Yes"]

        node[style=filled];
        d4e -> b3e [label = "Yes"]
        d4e -> b1 [label = "No"]
        b3e -> d7
      }

      subgraph cluster_swctrl {
        node[style=filled];
        d2 -> d3 [label = "Yes"]
        d3 -> d4a [label = "No"]
        d3 -> b6 [label = "Yes"]
        b6 -> b3a
        b3a -> b4
        d2 -> d4a [label = "No"]
      }

      d4a -> b1 [label = "No"]
      d4a -> b3a [label = "yes"]
      b2 -> b5
      b4 -> b5
      d7 -> b5 [label = "No"]
    }
  }
  @enddot

  **/

  UINT32 FlashWearOutProtection          : 1; ///< Flash Wear-Out Protection; <b>0: Disable</b>; 1: Enable
  UINT32 RsvdBits                        :25; ///< Bits reserved for DWORD alignment.

  UINT16 TotalFlashSize;                      ///< Total Flash Size on the system in KB
  UINT16 BiosSize;                            ///< BIOS Size in KB

} CPU_CONFIG_PREMEM;

#pragma pack(pop)
#endif
