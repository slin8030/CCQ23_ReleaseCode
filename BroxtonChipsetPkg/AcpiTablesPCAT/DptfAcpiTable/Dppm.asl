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
  
  //
  // DPPM Passive Policy 2.0
  //
  Name (DP2P, Package ()
  {
    ToUUID("9E04115A-AE87-4D1C-9500-0F3E340BFE75")
  })

  //
  // DPPM Passive Policy 1.0
  //
  Name (DPSP, Package ()
  {
    ToUUID("42A441D6-AE6A-462B-A84B-4A8CE79027D3")
  })

  // DPPM Active Policy
  //
  Name (DASP, Package ()
  {
    ToUUID("3A95C389-E4B8-4629-A526-C52C88626BAE")
  })

  //
  // DPPM Crtical Policy
  //
  Name (DCSP, Package()
  {
    ToUUID("97C68AE7-15FA-499c-B8C9-5DA81D606E0A")
  })

  //
  // Power Boss Policy
  //
  Name (POBP, Package ()
  {
    ToUUID("F5A35014-C209-46A4-993A-EB56DE7530A1")
  })

  //
  // Virtual Sensor Policy
  //
  Name (DVSP, Package ()
  {
    ToUUID("6ED722A7-9240-48a5-B479-31EEF723D7CF")
  })
}
