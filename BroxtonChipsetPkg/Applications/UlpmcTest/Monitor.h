/** @file
  Monitor header file.

  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <Protocol/SimpleFileSystem.h>

typedef struct{
  UINT8  MonitorAddress;
  UINT8  MonitorReg;
  UINT8  MonitorStartBit;
  UINT8  MonitorEndBit;
  UINT8  MonitorValue;
  UINT8  ChangeAddress;
  UINT8  ChangeReg;
  UINT8  ChangeStartBit;
  UINT8  ChangeEndBit;
  UINT8  ChangeValue;  
}MONITOR_REG;

typedef struct{
  UINT8        MonitorNum;
  MONITOR_REG  *MonitorRegs;
}MONITOR_ITEM;

EFI_STATUS
GetMonitorList(
  IN  EFI_FILE      *Fp,
  OUT MONITOR_ITEM  *mReg
  );

UINT8
GetMonitorItemData(
  IN     CHAR8   *String,
  IN OUT UINTN   *Index
  );

UINTN
StringToHex(
  IN  CHAR8      *String
  );

VOID
AnalyzeMonitorString(
  OUT  MONITOR_REG  *mReg,
  IN   CHAR8        *String
  );

VOID
CheckMonitorRegister(
  IN  MONITOR_ITEM  mReg,
  IN  EFI_FILE      *Fp
  );

UINT8
GetMonitorRegisterData(
  IN  MONITOR_REG  mReg
  );

VOID
SetMonitorRegisterData(
  IN  MONITOR_REG mReg
  );

#endif

