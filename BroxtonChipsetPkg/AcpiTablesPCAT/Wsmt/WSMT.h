/** @file
  This file describes WSMT table.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

typedef struct {
  UINT32  Signature;                   //0
  UINT32  Length;                      //4
  UINT8   Revision;                    //8
  UINT8   Checksum;                    //9
  UINT8   OEMID[6];                    //10
  UINT8   OEMTableID[8];               //16
  UINT32  OEMRevision;                 //24
  UINT32  CreatorID;                   //28
  UINT32  CreatorRevision;             //32
  UINT32  ProtectionFlags;             //36
} ACPI_WINDOWS_SMM_SECURITY_MITIGATIONS_STRUCTURE;

#define ACPI_WINDOWS_SMM_SECURITY_MITIGATIONS_STRUCTURE_SIGNATURE  SIGNATURE_32('W', 'S', 'M', 'T')

#define ACPI_WINDOWS_SMM_SECURITY_MITIGATIONS_STRUCTURE_REVISION   0x01

//
//note: ProtectionFlags  , BIT0=FIXED_COMM_BUFFERS , BIT1=COMM_BUFFER_NESTED_PTR_PROTECTION , BIT2=SYSTEM_RESOURCE_PROTECTION
//

/*
FIXED_COMM_BUFFERS - Firmware setting this bit should refer to the SMM Communication ACPI Table defined in the UEFI 2.6 specification.
                     Firmware should also consider all other possible data exchanges between SMM and non-SMM,
                     including but not limited to EFI_SMM_COMMUNICATION_PROTOCOL, ACPINVS in ASL code,
                     general purpose registers as buffer pointers, etc.

COMM_BUFFER_NESTED_PTR_PROTECTION - Firmware setting this bit must also set the FIXED_COMM_BUFFERS bit.

SYSTEM_RESOURCE_PROTECTION - After ExitBootServices(), firmware setting this bit shall not allow any software to make changes
                             to the locations of IOMMU's, interrupt controllers, PCI Configuration Space,
                             the Firmware ACPI Control Structure (FACS), or any registers reported through ACPI fixed tables
                             (e.g. PMx Control registers, reset register, etc.).
                             This also includes disallowing changes to RAM layout and ensuring that decodes to RAM
                             and any system resources as described above take priority over software configurable registers.
                             For example, if software configures a PCI Express BAR to overlay RAM,
                             accesses by the CPU to the affected system physical addresses must decode to RAM.
*/

#define ACPI_WINDOWS_SMM_SECURITY_MITIGATIONS_PROTECTION_FLAGS              0x00000000
#define FIXED_COMM_BUFFERS                                                  BIT0
#define COMM_BUFFER_NESTED_PTR_PROTECTION                                   BIT1
#define SYSTEM_RESOURCE_PROTECTION                                          BIT2
