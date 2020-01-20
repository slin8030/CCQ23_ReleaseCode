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

Scope (DGPU_SCOPE) {
    OperationRegion (AOPR, SystemMemory, 0x55AA55AA, 0x55AA55AA)
    Field (AOPR, AnyAcc, Lock, Preserve)
    {
        RSVD,       8,  // Reserved
    }
}
