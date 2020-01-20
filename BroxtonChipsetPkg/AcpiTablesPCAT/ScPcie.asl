/**@file

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

  OperationRegion(PXCS,PCI_Config,0x00,0x380)
  Field(PXCS,AnyAcc, NoLock, Preserve)
  {      
    Offset(0),
    VDID, 32,
    Offset(0x5A), // SLSTS[7:0] - Slot Status Register
    ABPX, 1,      // 0, Attention Button Pressed
    , 2,
    PDCX, 1,      // 3, Presence Detect Changed
    , 2,
    PDSX, 1,      // 6, Presence Detect State       
    , 1,
    Offset(0x60), // RSTS - Root Status Register
    , 16,
    PSPX, 1,      // 16,  PME Status
    Offset(0xA4),
    D3HT, 2,      // Power State
    Offset(0xD8), // MPC - Miscellaneous Port Configuration Register
    , 30,
    HPEX, 1,      // 30,  Hot Plug SCI Enable
    PMEX, 1,      // 31,  Power Management SCI Enable 
  }

  Field(PXCS,AnyAcc, NoLock, WriteAsZeros)
  {      
    Offset(0xDC), // SMSCS - SMI/SCI Status Register
    , 30,
    HPSX, 1,      // 30,	Hot Plug SCI Status
    PMSX, 1       // 31,	Power Management SCI Status 
  }

  Device(PXSX)
  {
    Name(_ADR, 0x00000000)

    // NOTE:  Any PCIE Hot-Plug dependency for this port is
    // specific to the CRB.  Please modify the code based on
    // your platform requirements.
	
    Name(_PRW, Package(){8,4}) //Wake bit for WiFi is 8
  }

  //
  // PCI_EXP_STS Handler for PCIE Root Port
  //
  Method(HPME,0,Serialized)
  {
  
    If(PMSX) {
      //
      // Clear the PME SCI status bit with timeout
      //
      Store(200,Local0)
      While(Local0) {
        //
        // Clear PME SCI Status
        //
        Store(1, PMSX)
        //
        // If PME SCI Status is still set, keep clearing it.
        // Otherwise, break the while loop.
        //
        If(PMSX) {
          Decrement(Local0)
        } else {
          Store(0,Local0)
        }
      }
      
      //
      // Notify PCIE Endpoint Devices
      //
      Notify(PXSX, 0x02)
    }
  }
