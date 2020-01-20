/** @file
  Screen header file.
  
  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef __SCREEN_H__
#define __SCREEN_H__

#define EFI_BLACK           0x00
#define EFI_BLUE            0x01
#define EFI_GREEN           0x02
#define EFI_CYAN            (EFI_BLUE | EFI_GREEN)
#define EFI_RED             0x04
#define EFI_MAGENTA         (EFI_BLUE | EFI_RED)
#define EFI_BROWN	          (EFI_GREEN | EFI_RED)
#define EFI_LIGHTGRAY       (EFI_BLUE | EFI_GREEN | EFI_RED)
#define EFI_BRIGHT          0x08
#define EFI_DARKGRAY        (EFI_BRIGHT)
#define EFI_LIGHTBLUE       (EFI_BLUE | EFI_BRIGHT)
#define EFI_LIGHTGREEN      (EFI_GREEN | EFI_BRIGHT)
#define EFI_LIGHTCYAN       (EFI_CYAN | EFI_BRIGHT)
#define EFI_LIGHTRED        (EFI_RED | EFI_BRIGHT)
#define EFI_LIGHTMAGENTA    (EFI_MAGENTA | EFI_BRIGHT)
#define EFI_YELLOW          (EFI_BROWN | EFI_BRIGHT)
#define EFI_WHITE           (EFI_BLUE | EFI_GREEN | EFI_RED | EFI_BRIGHT)

#define EFI_TEXT_ATTR(f, b)    ((f) | ((b) << 4))

#define EFI_BACKGROUND_BLACK     0x00
#define EFI_BACKGROUND_BLUE      0x10
#define EFI_BACKGROUND_GREEN     0x20
#define EFI_BACKGROUND_CYAN      (EFI_BACKGROUND_BLUE | EFI_BACKGROUND_GREEN)
#define EFI_BACKGROUND_RED       0x40
#define EFI_BACKGROUND_MAGENTA   (EFI_BACKGROUND_BLUE | EFI_BACKGROUND_RED)
#define EFI_BACKGROUND_BROWN     (EFI_BACKGROUND_GREEN | EFI_BACKGROUND_RED)
#define EFI_BACKGROUND_LIGHTGRAY (EFI_BACKGROUND_BLUE | EFI_BACKGROUND_GREEN | EFI_BACKGROUND_RED)

#define VERSION_BAR_X     10
#define VERSION_BAR_Y     1

#define CHARGERINFO_X     2
#define CHARGERINFO_Y     3

#define CHARGERDATA_X     (CHARGERINFO_X + 3)
#define CHARGERDATA_Y     (CHARGERINFO_Y + 1)

#define BATTERYINFO_X     2
#define BATTERYINFO_Y     10

#define BATTERYDATA_X     (BATTERYINFO_X + 3)
#define BATTERYDATA_Y     (BATTERYINFO_Y + 1)

#define HELP_INFO_BAR     7


#define TIME_BAR_X        BATTERYDATA_X + 0x2E
#define TIME_BAR_Y        20

#define STATUS_BAR_X      2
#define STATUS_BAR_Y      17

EFI_STATUS
SetFrmMode (
  IN UINT8  FrmMode
  );

VOID
CleanScreen(
  IN VOID
  );  

VOID
GetFileName(
  OUT CHAR16 *Name
  );

VOID
ShowBatteryInformation(
  OUT  CHAR8  *String
  );

VOID
ShowChargerInformation(
  OUT  CHAR8  *String
  ); 

VOID
ShowFrameWork(
  IN VOID
  );

VOID
ShowChargerREG00Info(
  IN CHAR8  *String
  );

VOID
ShowChargerREG01Info(
  IN CHAR8  *String
  );

VOID
ShowChargerREG02Info(
  IN CHAR8  *String
  );

VOID
ShowChargerREG03Info(
  IN CHAR8  *String
  );

VOID
ShowChargerREG04Info(
  IN CHAR8  *String
  );

VOID
ShowChargerREG05Info(
  IN CHAR8  *String
  );

VOID
ShowChargerREG08Info(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG06Info(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG08Info(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG0AInfo(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG0CInfo(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG0EInfo(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG10Info(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG12Info(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG14Info(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG2AInfo(
  IN CHAR8  *String
  );

VOID
ShowBatteryREG2CInfo(
  IN CHAR8  *String
  );

VOID
ShowTimeBar(
  IN    CHAR8   *String
  );

VOID
ShowStatusInformation(
  IN  UINT8   Data
  );

#endif
