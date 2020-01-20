//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#ifndef __IOSF_H__
#define __IOSF_H__

#include <Uefi.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>

#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellParameters.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

//
// Msg Bus Registers
//
#define MC_MCR      0x000000D0   //Cunit Message Control Register
#define MC_MDR      0x000000D4   //Cunit Message Data Register
#define MC_MCRX     0x000000D8   //Cunit Message Control Register Extension

#define EC_BASE     0xE0000000

//
// MSG BUS API
//
#define MSG_BUS_ENABLED          0x000000F0
#define MSGBUS_MASKHI            0xFFFFFF00
#define MSGBUS_MASKLO            0x000000FF
#define MESSAGE_DWORD_EN         BIT4 | BIT5 | BIT6 | BIT7



#define UEFIMsgBus32Read( OPCODE, PORTID, Register, Dbuff) \
{ \
  MmioWrite32(EC_BASE + MC_MCRX, (Register & MSGBUS_MASKHI)); \
  MmioWrite32(EC_BASE + MC_MCR, (UINT32) ((OPCODE << 24) | (PORTID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN)); \
  (UINT32)(Dbuff) = MmioRead32(EC_BASE + MC_MDR); \
}

#define UEFIMsgBus32Write(OPCODE, PORTID, Register, Dbuff) \
{ \
  MmioWrite32(EC_BASE + MC_MCRX, (Register & MSGBUS_MASKHI)); \
  MmioWrite32(EC_BASE + MC_MDR, Dbuff); \
  MmioWrite32(EC_BASE + MC_MCR, (UINT32)((OPCODE << 24) | (PORTID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN)); \
}


VOID
DoShowMsgData(
  IN  UINT8   OPCode,
  IN  UINT8   PortID,
  IN  UINT32  m_Reg,
  OUT UINT32  *Data
  );

VOID
DoWriteMsgData(
  IN UINT8   OPCode,
  IN UINT8   PortID,
  IN UINT32  m_Reg,
  IN UINT32  Data
  );


#endif
