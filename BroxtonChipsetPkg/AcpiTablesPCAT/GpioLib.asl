/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
include ("BxtPGpioDefine.asl")
//
// GPIO Access Library
//
Scope(\_SB)
{
  //
  // Get Pad Configuration DW0 register value
  //
  Method(GPC0, 0x1, Serialized)
  {
    //
    // Arg0 - (GpioCommunityAddress + Gpio MMIO_Offset)
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset)
    Store(Arg0, Local0)
    OperationRegion(PDW0, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW0, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        TEMP,32
    }
    Return(TEMP)
  }

  //
  // Set Pad Configuration DW0 register value
  //
  Method(SPC0, 0x2, Serialized)
  {
    //
    // Arg0 - (GpioCommunityAddress + Gpio MMIO_Offset)
    // Arg1 - Value for DW0 register
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset)
    Store(Arg0, Local0)
    OperationRegion(PDW0, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW0, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        TEMP,32
    }
    Store(Arg1,TEMP)
  }

  //
  // Get Pad Configuration DW1 register value
  //
  Method(GPC1, 0x1, Serialized)
  {
    //
    // Arg0 -Local0 = (GpioCommunityAddress + Gpio MMIO_Offset) + 0x4
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset) + 0x4
    Store( Add( Arg0, 0x4), Local0)
    OperationRegion(PDW1, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW1, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        TEMP,32
    }
    Return(TEMP)
  }

  //
  // Set Pad Configuration DW1 register value
  //
  Method(SPC1, 0x2, Serialized)
  {
    //
    // Arg0 - (GpioCommunityAddress + Gpio MMIO_Offset) + 0x4
    // Arg1 - Value for DW1 register
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset) + 0x4
    Store( Add( Arg0, 0x4), Local0)
    OperationRegion(PDW1, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW1, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        TEMP,32
    }
    Store(Arg1,TEMP)
  }

  //
  // Get GPI Input Value
  //
  Method(GGIV, 0x1, Serialized)
  {
    //
    // Arg0 - (GpioCommunityAddress + Gpio MMIO_Offset)
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset)
    Store( Arg0, Local0)
    OperationRegion(PDW0, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW0, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        ,    1,
        TEMP,1,
        ,    30
    }
    Return(TEMP)
  }

  //
  // Get GPO Output Value
  //
  Method(GGOV, 0x1, Serialized)
  {
    //
    // Arg0 - (GpioCommunityAddress + Gpio MMIO_Offset)
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset)
    Store( Arg0, Local0)
    OperationRegion(PDW0, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW0, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        TEMP,1,
        ,    31
    }
    Return(TEMP)
  }

  //
  // Set GPO Output Value
  //
  Method(SGOV, 0x2, Serialized)
  {
    //
    // Arg0 - (GpioCommunityAddress + Gpio MMIO_Offset)
    // Arg1 - Value of GPIO Tx State
    //

    // Local0 = (GpioCommunityAddress + Gpio MMIO_Offset)
    Store( Arg0, Local0)
    OperationRegion(PDW0, SystemMemory, Or (P2BA, Local0), 4)
    Field(PDW0, AnyAcc, NoLock, Preserve) {
      Offset(0x0),
        TEMP,1,
        ,    31
    }
    Store(Arg1,TEMP)
  }

} // \_SB Scope


