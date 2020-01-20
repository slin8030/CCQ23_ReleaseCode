/** @file
  This is the equates file for HT (Hyper-threading) support

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#define VacantFlag             0x00
#define NotVacantFlag          0xff

#define LockLocation                   (0x1000 - 0x0200)
#define StackStartAddressLocation      (LockLocation + 0x04)
#define StackSizeLocation              (LockLocation + 0x08)
#define CProcedureLocation             (LockLocation + 0x0C)
#define GdtrLocation                   (LockLocation + 0x10)
#define IdtrLocation                   (LockLocation + 0x16)
#define BufferStartLocation            (LockLocation + 0x1C)
#define PmodeOffsetLocation            (LockLocation + 0x20)
#define AcpiCpuDataAddressLocation     (LockLocation + 0x24)
#define MtrrValuesAddressLocation      (LockLocation + 0x28)
#define FinishedCountAddressLocation   (LockLocation + 0x2C)
#define WakeupCountAddressLocation     (LockLocation + 0x30)
#define SerializeLockAddressLocation   (LockLocation + 0x34)
#define MicrocodeAddressLocation       (LockLocation + 0x38)
#define BootScriptAddressLocation      (LockLocation + 0x3C)
#define StartStateLocation             (LockLocation + 0x40)
#define VirtualWireMode                (LockLocation + 0x44)
#define SemaphoreCheck                 (LockLocation + 0x48)
#define PeiServices                    (LockLocation + 0x4C)
#define PeiStall                       (LockLocation + 0x50)
#define CpuPerfCtrlValue               (LockLocation + 0x54)
#define SiCpuPolicyPpi                 (LockLocation + 0x5C)
#define MpSystemDataAddressLocation    (LockLocation + 0x64)
#define MpServicePpiAddressLocation    (LockLocation + 0x68)
#define CArgumentLocation              (LockLocation + 0x6C)
#define BistBufferLocation             (LockLocation + 0x70)