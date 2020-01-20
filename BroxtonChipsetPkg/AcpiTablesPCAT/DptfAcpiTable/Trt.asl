/** @file
  Intel ACPI Reference Code for Dynamic Platform & Thermal Framework 

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
  // _TRT (Thermal Relationship Table)
  //
  // Arguments: (0)
  //   None
  // Return Value:
  //   A variable-length Package containing a list of Thermal Relationship Packages as described below.
  //
  // Return Value Information
  //   Package {
  //   ThermalRelationship[0] // Package
  //    ...
  //   ThermalRelationship[n] // Package
  //   }
  //
  Name(_TRT, Package()
  {
    // Source                       Target             Influence  Period    Reserved
    Package(){\_SB.PCI0.TCPU,       \_SB.SEN1,                10,    100,   0, 0, 0, 0}
  })
  
  // TRTR (Thermal Relationship Table Revision)
  //
  // This object evaluates to an integer value that defines the revision of the _TRT object. 
  //
  // Arguments: (0)
  //   None
  // Return Value:
  // 0: Traditional TRT as defined by the ACPI Specification.
  // 1: Priority based TRT
  //
  Method(TRTR)
  {
    Return(TRTV)
  }

} // End Scope(\_SB.IETM)