/*++

Copyright (c)  1999 - 2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SmmSmbus.h

Abstract:

    Contains information used to read and write to SMBus devices.

Revision History

--*/

#ifndef SMM_SMBUS_H
#define SMM_SMBUS_H

#define MAX_SMB_BUFFER 0x20

#include <Protocol/SmbusHc.h>
//
// SM_BUS:RestrictedBegin & 
//
EFI_STATUS
BlockSmBusWrite(
  IN      EFI_SMBUS_DEVICE_ADDRESS  Address,
  IN      UINTN                     Command,
  IN      UINTN                     Length,
  IN      VOID                      *Buffer
  );

EFI_STATUS
BlockSmBusRead(
  IN      EFI_SMBUS_DEVICE_ADDRESS  Address,
  IN      UINTN                     Command,
  IN OUT  UINTN                     *Length,
  IN OUT  VOID                      *Buffer
  );
//
// SM_BUS:RestrictedEnd
//
EFI_STATUS
Stall (
  IN UINTN              Microseconds
  );

#endif
