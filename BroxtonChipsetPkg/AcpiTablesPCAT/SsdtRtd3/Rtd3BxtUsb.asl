/** @file
  Power resource and wake capability for USB ports

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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

  //
  // Define _PR0, _PR3 PowerResource Package
  //
  Name (_PR0, Package(){UPPR})
  Name (_PR3, Package(){UPPR})

  PowerResource(UPPR, 0, 0)
  {
    Method(_STA)
    {
      Return(0x01)
    }

    Method(_ON)
    {
    }

    Method(_OFF)
    {
    }
  }

  //
  // _SxW, in Sx, the lowest power state supported to wake up the system
  // _SxD, in Sx, the highest power state supported by the device
  // If OSPM supports _PR3 (_OSC, Arg3[2]), 3 represents D3hot; 4 represents D3cold, otherwise 3 represents D3.
  //  
  Method(_S0W, 0, Serialized)
  {
    Return(0x3) // return 3 (D3hot)
  }
  Method (_S3D, 0, Serialized)
  {
    Return (0x2)
  }
  Method (_S3W, 0, Serialized)
  {
    Return (0x3)
  }
  Method (_S4D, 0, Serialized)
  {
    Return (0x2)
  }
  Method (_S4W, 0, Serialized)
  {
    Return (0x3)
  }

