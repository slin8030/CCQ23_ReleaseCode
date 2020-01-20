/*++

Copyright (c)  1999 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    LpcSio.c

Abstract: Sio implementation

Revision History

--*/

#include "LpcDriver.h"
#include <Library/S3BootScriptLib.h>

VOID
WriteRegister (
  IN  UINT8   Index,
  IN  UINT8   Data
  );

typedef struct {
  UINT8 Register;
  UINT8 Value;
} EFI_SIO_TABLE;

EFI_SIO_TABLE mSioTable[] = {
  //
  // Init keyboard controller
  //
  { REG_LOGICAL_DEVICE, SIO_KEYBOARD },
  { BASE_ADDRESS_HIGH, 0x00 },
  { BASE_ADDRESS_LOW, 0x60 },
  { BASE_ADDRESS_HIGH2, 0x00 },
  { BASE_ADDRESS_LOW2, 0x64 },
  { PRIMARY_INTERRUPT_SELECT, 0x01 },
  { ACTIVATE, 0x1 },

  //
  // Init Mouse controller
  //
  { REG_LOGICAL_DEVICE, SIO_MOUSE },
  { BASE_ADDRESS_HIGH, 0x00 },
  { BASE_ADDRESS_LOW, 0x60 },
  { BASE_ADDRESS_HIGH2, 0x00 },
  { BASE_ADDRESS_LOW2, 0x64 },
  { PRIMARY_INTERRUPT_SELECT, 0x0c },
  { ACTIVATE, 0x1 },

  { REG_LOGICAL_DEVICE, SIO_COM },
  { BASE_ADDRESS_HIGH, 0x03 },
  { BASE_ADDRESS_LOW, 0xf8 },
  { PRIMARY_INTERRUPT_SELECT, 0x04 },
  { ACTIVATE, 0x1 },


};

VOID
LPCWPCE791SetDefault ()
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINT8           Index;

  for (Index = 0; Index < sizeof(mSioTable)/sizeof(EFI_SIO_TABLE); Index++) {
    WriteRegisterAndSaveToScript (mSioTable[Index].Register, mSioTable[Index].Value);
  }

  return;
}

VOID
DisableLogicalDevice (
  UINT8       DeviceId
  )
{
  WriteRegisterAndSaveToScript (REG_LOGICAL_DEVICE, DeviceId);
  WriteRegisterAndSaveToScript (ACTIVATE, 0);
  WriteRegisterAndSaveToScript (BASE_ADDRESS_HIGH, 0);
  WriteRegisterAndSaveToScript (BASE_ADDRESS_LOW, 0);

  return;
}

VOID
WriteRegister (
  IN  UINT8   Index,
  IN  UINT8   Data
  )
{
//[-start-160216-IB03090424-modify]//
  IoWrite8 (CONFIG_PORT, Index);
  IoWrite8 (DATA_PORT, Data);
//[-end-160216-IB03090424-modify]//

  return;
}

VOID
WriteRegisterAndSaveToScript (
  IN  UINT8   Index,
  IN  UINT8   Data
  )
{
  UINT8  Buffer[2];

//[-start-160216-IB03090424-modify]//
  IoWrite8 (CONFIG_PORT, Index);
  IoWrite8 (DATA_PORT, Data);
//[-end-160216-IB03090424-modify]//

  Buffer[0] = Index;
  Buffer[1] = Data;
  S3BootScriptSaveIoWrite (
    EfiBootScriptWidthUint8,
    INDEX_PORT,
    2,
    Buffer
    );

  return;
}

