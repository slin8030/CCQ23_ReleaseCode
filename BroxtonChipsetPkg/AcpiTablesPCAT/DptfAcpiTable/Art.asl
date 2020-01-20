/** @file 
  Intel ACPI Reference Code for Intel Dynamic Power Performance Management


@copyright
 Copyright (c) 1999 - 2015 Intel Corporation. All rights reserved
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

    // _ART (Active Cooling Relationship Table)
    //
    // Arguments:
    //   None
    // Return Value:
    //   A variable-length Package containing a Revision ID and a list of Active Relationship Packages as described below:
    //
    // Return Value Information
    //   Package {
    //   Revision, // Integer - Current revision is: 0
    //   ActiveRelationship[0] // Package
    //   ...
    //   ActiveRelationship[n] // Package
    //   }
    //
    Name(_ART, Package()
    {
      0,    // Revision
      //        Source     Target        Weight, AC0MaxLevel, AC1MaxLevel,  AC2MaxLevel,  AC3MaxLevel,  AC4MaxLevel,  AC5MaxLevel,  AC6MaxLevel,  AC7MaxLevel,  AC8MaxLevel,  AC9MaxLevel
      Package(){\_SB.TFN1, \_SB.PCI0.TCPU,  100,         100,          80,           50,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF},
      Package(){\_SB.TFN1, \_SB.SEN1,       100,         100,          80,           50,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF},
      Package(){\_SB.TFN1, \_SB.GEN1,       100,         100,          80,           50,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF},
      Package(){\_SB.TFN1, \_SB.GEN2,       100,         100,          80,           50,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF},
      Package(){\_SB.TFN1, \_SB.GEN3,       100,         100,          80,           50,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF},
      Package(){\_SB.TFN1, \_SB.GEN4,       100,         100,          80,           50,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF,   0xFFFFFFFF}
     })

} // End of Scope \_SB.IETM

