/** @file
  ACPI DSDT table

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

Scope (\_SB.PCI0) {
  //
  // Integrated Sensor Hub - Device 17, Function 0
  //  
  Device(ISH0) { 
    Name(_ADR, 0x00110000)
    Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }

    //--------------------
    //  Intel Proprietary Wake up Event solution
    //--------------------
    Method(_DSM, 0x4, Serialized)
    {
      If(LEqual(Arg0, ToUUID("1730E71D-E5DD-4A34-BE57-4D76B6A2FE37"))) {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero)) {
          // Revision 0
          If(LEqual(Arg1, Zero)) {
            Return(Buffer(One) { 0x03 }) // There are 1 function defined other than Query.
          } Else {
            Return(0) // Revision mismatch
          }
        }
        // Function 1 : 
        If(LEqual(Arg2, One)) {
          Store(DerefOf(Index(Arg3, Zero)), Local0)
          If(LEqual(Local0, Zero)) {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
            //
            // CRB EC device
            //
            \_SB.HIDD.HPEM(27) 
#else
            //
            // OEM EC device
            //
#endif
#endif
//[-end-161206-IB07400822-modify]//
            P8XH(0,0x5C)
          } Else {
		    //Handle sleep, dock, un-dock events here
		  }
          Return(0)
        } Else {
          Return(0) // Function number mismatch but normal return.
        }
      } Else {
        Return(Buffer(One) { 0x00 }) // Guid mismatch
      }
    }

    Method (_PS0, 0, NotSerialized) { // _PS0: Power State 0
    }

    Method (_PS3, 0, NotSerialized) { // _PS3: Power State 3
    }

  }
    
}//end scope