/** @file 
  Intel ACPI Reference Code for Intel Dynamic Power Performance Management

@copyright
 Copyright (c) 2013 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

Scope(\_SB.IETM)
{

  // PTTL (Participant Temperature Tolerance Level)
  // This object evaluates to an integer representing the temperature range within which any change
  // in participant temperature is considered acceptable and no policy actions will be taken by the
  // policy. The temperature tolerance level is expressed in the units of 10s of Kelvin.
  //
  Name (PTTL, 20) // ToleranceLevel

  // Used by passive policy 2.0
  //
  //  Name (PSVT, Package()
  //  {
  //    0x80000000,     // Revision: 1 = true proportional limit, 2 = depth limit
  //    Package() {
  //      "Source",     // Source device: String
  //      "Target",     // Target device: String
  //      0x80000000,   // Priority: for use by passive policy 2.0
  //      0x80000000,   // Sampling Period: in 1/10 of seconds unit
  //      0x80000000,   // Passive Temp: Threshold(1/10 of K) at which Passive Policy 2.0 will take action
  //      0x80000000,   // Domain: type under the source that is controlled
  //      0x80000000,   // Control Knob: type of control knob that need to be exercised
  //      0x80000000,   // Control Value: Depth Limit or Target State which policy will jump to on a thermal condition(based on revision).
  //      0x80000000,   // Step size for this specific control knob
  //      0x80000000,   // Limit Coefficient: 1/10 of units, used with StepSize to control limiting action
  //      0x80000000,   // UnLimit Coefficient: 1/10 of units, used with StepSize to control unlimiting action
  //      0x80000000    // Reserved1
  //    }
  //  })
  Name (PSVT, Package()
  {
    2,
    Package(){\_SB.PCI0.TCPU, \_SB.SEN1, 2, 300, 3032,  9, 0x00010000, 12000, 500, 10, 20, 0},
    Package(){\_SB.PCI0.TCPU, \_SB.SEN1, 2, 300, 3082,  9, 0x00010000,  9000, 500, 10, 20, 0},
    Package(){\_SB.PCI0.TCPU, \_SB.SEN1, 2, 300, 3132,  9, 0x00010000,  6000, 500, 10, 20, 0},
    Package(){\_SB.PCI0.TCPU, \_SB.SEN1, 1, 300, 3232,  9, 0x00010000, "MIN", 500, 10, 20, 0},
    Package(){\_SB.PCI0.TCPU, \_SB.PCI0.TCPU,1,50,3532, 9, 0x00010000, "MIN", 500, 10, 20, 0}
  })

}// end Scope(\_SB.IETM)
