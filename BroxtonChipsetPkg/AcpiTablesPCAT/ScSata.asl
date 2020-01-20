/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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


Scope(\_SB.PCI0)
{
  //
  // Serial ATA Host Controller - Device 18, Function 0
  //
  Device(SATA) {
    Name(_ADR,0x00120000)

    Device(PRT0)
    {
      Name(_ADR,0x0000FFFF)  // Port 0
    }
    Device(PRT1)
    {
      Name(_ADR,0x0001FFFF)  // Port 1
    }
    //
    // SATA Methods pulled in via SSDT.
    //
    OperationRegion(SATR, PCI_Config, 0x74,0x4)
    Field(SATR,WordAcc,NoLock,Preserve) {
      Offset(0x00), // 0x74, PMCR
          ,   8,
      PMEE,   1,    //PME_EN
          ,   6,
      PMES,   1     //PME_STS
    }

    Method (_STA, 0x0, NotSerialized)
    {
      //Enable SATA controller PME_EN bit
      Store (1, \_SB.PCI0.SATA.PMEE)
      Return(0xf)
    }

    Method(_DSW, 3)
    {
      If(Arg1)
      { // Entering Sx, need to disable WAKE# from generating runtime PME
          Store(0, \_SB.PCI0.SATA.PMEE)
      } Else {  // Staying in S0
        If(LAnd(Arg0, Arg2)) // Exiting D0 and arming for wake
        { // Set PME
          Store(1, \_SB.PCI0.SATA.PMEE)
        } Else { // Disable runtime PME, either because staying in D0 or disabling wake
          Store(0, \_SB.PCI0.SATA.PMEE)
        }
      }
    } // End _DSW
  }
}


