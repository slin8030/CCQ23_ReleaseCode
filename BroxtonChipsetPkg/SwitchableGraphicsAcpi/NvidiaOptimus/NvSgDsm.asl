/** @file

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

Scope (IGPU_SCOPE)
{
    Method (_INI, 0)
    {
        Store (DID1, Index (TLPK, 0x00))
        Store (DID2, Index (TLPK, 0x02))
        Store (DID3, Index (TLPK, 0x04))
        Store (DID4, Index (TLPK, 0x06))
        Store (DID5, Index (TLPK, 0x08))
        Store (DID6, Index (TLPK, 0x0A))
        Store (DID7, Index (TLPK, 0x0C))
        Store (DID2, Index (TLPK, 0x0E))
        Store (DID1, Index (TLPK, 0x0F))
        Store (DID2, Index (TLPK, 0x11))
        Store (DID3, Index (TLPK, 0x12))
        Store (DID2, Index (TLPK, 0x14))
        Store (DID4, Index (TLPK, 0x15))
        Store (DID2, Index (TLPK, 0x17))
        Store (DID5, Index (TLPK, 0x18))
        Store (DID2, Index (TLPK, 0x1A))
        Store (DID6, Index (TLPK, 0x1B))
        Store (DID2, Index (TLPK, 0x1D))
        Store (DID7, Index (TLPK, 0x1E))
    }

    Method (MXMX, 1, Serialized)
    {
        Return (Zero)
    }

    Method (MXDS, 1, Serialized)
    {
        Return (Zero)
    }

    // Toggle List Package
    Name (TLPK, Package(0x20) {
        Ones, 0x2C,
        Ones, 0x2C,
        Ones, 0x2C,
        Ones, 0x2C,
        Ones, 0x2C,
        Ones, 0x2C,
        Ones, 0x2C,
        Ones, Ones, 0x2C,
        Ones, Ones, 0x2C,
        Ones, Ones, 0x2C,
        Ones, Ones, 0x2C,
        Ones, Ones, 0x2C,
        Ones, Ones, 0x2C
    })

    //
    // _DSM Device Specific Method
    //
    //   Arg0:   UUID      Unique function identifier
    //   Arg1:   Integer   Revision Level
    //   Arg2:   Integer   Function Index (0 = Return Supported Functions)
    //   Arg3:   Package   Parameters
    //
    Method (_DSM, 4, SERIALIZED)
    {
        Store (And (Arg2, 0xFF), Local0)

        //
        // Check for Nvidia Optimus _DSM  UUID
        //
        // NVOP_DSM_GUID {A486D8F8-0BDA-471B-A72B-6042A6B5BEE0}
        //
        If (LEqual (Arg0, ToUUID ("A486D8F8-0BDA-471B-A72B-6042A6B5BEE0")))
        {
            Return (DGPU_SCOPE.NVOP (Arg0, Arg1, Arg2, Arg3))
        }

        //
        // Check for Nvidia Hybrid _DSM  UUID
        //
        If (LEqual (Arg0, ToUUID("9D95A0A0-0060-4D48-B34D-7E5FEA129FD4")))
        {
            Return (STATUS_ERROR_UNSUPPORTED)
        }

        //
        // Generic unspecified error code
        //
        Return (STATUS_ERROR_UNSPECIFIED)  // MXM_ERROR_UNSPECIFIED
    }
}
