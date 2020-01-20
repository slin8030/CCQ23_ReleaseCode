/** @file
  Screen related functions.

  Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/I2cLib.h>

#include <UlpmcTest.h>
#include <KBInput.h>
#include <Screen.h>
#include <Print.h>

extern EFI_RUNTIME_SERVICES  *gRT;
extern EFI_SYSTEM_TABLE      *gST;

/**
  SetFrmMode

  @param[in] FrmMode    The firmware allocated handle for the EFI image.  
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
SetFrmMode (
  IN UINT8  FrmMode
  )
{
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  switch (FrmMode) {   
  case 0:
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLACK));
    break;
  case 1:
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_YELLOW, EFI_BLACK));
    break;
  case 2:
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_RED, EFI_BLACK));
    break;
  case 3:
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_GREEN, EFI_BLACK));
    break;
  case 4:
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_CYAN, EFI_BLACK));
    break;
  default:
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLACK));
    gST->ConOut->EnableCursor (gST->ConOut, TRUE);
  }
  return EFI_SUCCESS;
}

/**
  Clean Screen

  @param[in]          
  
  @retval None.             The entry point is executed successfully.

**/
VOID
CleanScreen(
  IN  VOID
  )
{
  SetFrmMode(0);
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->EnableCursor (gST->ConOut, TRUE);
}

/**
  ShowFrameWork

  @param[in]          
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowFrameWork(
  IN  VOID
  )
{
  SetFrmMode(1);
  PrintAt(VERSION_BAR_X, VERSION_BAR_Y, L"Intel(R) Battery Monitor Utility V1.1");
  SetFrmMode(2);  
  PrintAt(CHARGERINFO_X, CHARGERINFO_Y, L"Charger Information:");
  PrintAt(BATTERYINFO_X, BATTERYINFO_Y, L"FG Information:");
  PrintAt(STATUS_BAR_X + 00, STATUS_BAR_Y + 00, L"Charger Status:");

  SetFrmMode(1);
  
  PrintAt(CHARGERDATA_X + 0x00, CHARGERDATA_Y + 0x00, L"Input Voltage Limit:     [        ]");
  PrintAt(CHARGERDATA_X + 0x25, CHARGERDATA_Y + 0x00, L"Input Current Limit:       [        ]");
  
  PrintAt(CHARGERDATA_X + 0x00, CHARGERDATA_Y + 0x01, L"Minimum System Voltage:  [        ]");
  PrintAt(CHARGERDATA_X + 0x25, CHARGERDATA_Y + 0x01, L"Fast Charge Current Limit: [        ]");

  PrintAt(CHARGERDATA_X + 0x00, CHARGERDATA_Y + 0x02, L"Pre-charge Current Limit:[        ]");
  PrintAt(CHARGERDATA_X + 0x25, CHARGERDATA_Y + 0x02, L"Termination Current:       [        ]");

  PrintAt(CHARGERDATA_X + 0x00, CHARGERDATA_Y + 0x03, L"Charge Voltage Limit:    [        ]");
  PrintAt(CHARGERDATA_X + 0x25, CHARGERDATA_Y + 0x03, L"Battery Recharge Threshold:[        ]");

  PrintAt(CHARGERDATA_X + 0x00, CHARGERDATA_Y + 0x04, L"Fast Charge Timer:       [        ]");
  PrintAt(CHARGERDATA_X + 0x25, CHARGERDATA_Y + 0x04, L"Charging status:           [        ]");   

  PrintAt(BATTERYDATA_X + 0x00, BATTERYDATA_Y + 0x00, L"TEMP:  [        ]");
  PrintAt(BATTERYDATA_X + 0x25, BATTERYDATA_Y + 0x00, L"VOLT:  [        ]");
 
  PrintAt(BATTERYDATA_X + 0x00, BATTERYDATA_Y + 0x01, L"FLAGS: [        ]");
  PrintAt(BATTERYDATA_X + 0x25, BATTERYDATA_Y + 0x01, L"NAC:   [        ]");
  
  PrintAt(BATTERYDATA_X + 0x00, BATTERYDATA_Y + 0x02, L"FAC:   [        ]");
  PrintAt(BATTERYDATA_X + 0x25, BATTERYDATA_Y + 0x02, L"RM:    [        ]");

  PrintAt(BATTERYDATA_X + 0x00, BATTERYDATA_Y + 0x03, L"FCC:   [        ]");
  PrintAt(BATTERYDATA_X + 0x25, BATTERYDATA_Y + 0x03, L"AI:    [        ]");

  PrintAt(BATTERYDATA_X + 0x00, BATTERYDATA_Y + 0x04, L"CC:    [        ]");
  PrintAt(BATTERYDATA_X + 0x25, BATTERYDATA_Y + 0x04, L"SOC:   [        ]");

}

/**
  Show Charger REG00 Info - Input Source Control Register

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerREG00Info(
  IN CHAR8  *String
  )
{
  UINT8   Data = 0;
  UINT8   Value = 0;
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT32  Summer = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x00, 1, &Data);  
  
  Value = Data >> 3;
  Value = Value & 0x0F;

  Summer = 3880;
  
  if ((Value & BIT0) != 0) {
    Summer = Summer + 80;
  }
  if ((Value & BIT1) != 0) {
    Summer = Summer + 160;
  }
  if ((Value & BIT2) != 0) {
    Summer = Summer + 320;
  }
  if ((Value & BIT3) != 0) {
    Summer = Summer + 640;
  }
  
  High = (UINT8)(Summer / 1000);
  Low = (UINT8)((Summer - (High * 1000)) / 10);
  
  PrintAt (CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x00, L"%d.%02dV", High, Low);
  SPrint (UnicodeStr, 32, L"%d.%02d,", High, Low);
  UnicodeStrToAsciiStr (UnicodeStr, AsciiStr);
  strcat (String, AsciiStr);

  Value = Data & 0x07;  
  switch(Value) {
  case 0:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"100mA");
    strcat (String, "100,");
    break;
  case 1:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"150mA");
    strcat (String, "150,");
    break;
  case 2:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"500mA");
    strcat (String, "500,");
    break;
  case 3:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"900mA");
    strcat (String, "900,");
    break;
  case 4:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"1200mA");
    strcat (String, "1200,");
    break;
  case 5:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"1500mA");
    strcat (String, "1500,");
    break;
  case 6:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"2000mA");
    strcat (String, "2000,");
    break;
  case 7:
    PrintAt (CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x00, L"3000mA");
    strcat (String, "3000,");
    break; 
  }
}

/**
  Show Charger REG01 Info - Power-On Configuration Register

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerREG01Info(
  IN  CHAR8  *String
  )
{
  UINT8   Data = 0;
  UINT8   Value = 0;
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT32  Summer = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];  
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x01, 1, &Data);  
  
  Value = Data >> 1;
  Value = Value & 0x07;

  Summer = 30;
  if((Value & BIT0) != 0) {
    Summer = Summer + 1;
  }
  if((Value & BIT1) != 0) {
    Summer = Summer + 2;
  }
  if((Value & BIT2) != 0) {
    Summer = Summer + 4;
  }
  
  High = (UINT8)(Summer /10);
  Low = (UINT8)(Summer - (High * 10));
  PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x01, L"%d.%dV", High, Low);
  SPrint(UnicodeStr, 32, L"%d.%d,", High, Low);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}


/**
  Show Charger REG03 Info - Pre-Charge/Termination Current Control Register

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerREG03Info(
  IN  CHAR8  *String
  )
{
  UINT8   Data = 0;
  UINT8   Value = 0;
  UINT32  Summer  = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x03, 1, &Data);  
  
  Value = Data >> 4;
  Value = Value & 0x0F;
  Summer = 128;
  if((Value & BIT0) != 0) {
    Summer = Summer + 128;
  }
  if((Value & BIT1) != 0) {
    Summer = Summer + 256;
  }
  if((Value & BIT2) != 0) {
    Summer = Summer + 512;
  }
  if((Value & BIT3) != 0) {
    Summer = Summer + 1024;
  }  
  PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x02, L"%04dmA", Summer);
  SPrint(UnicodeStr, 32, L"%04d,", Summer);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);

  Value = Data & 0x0F;
  Summer = 128;
  if((Value & BIT0) != 0) {
    Summer = Summer + 128;
  }
  if((Value & BIT1) != 0) {
    Summer = Summer + 256;
  }
  if((Value & BIT2) != 0) {
    Summer = Summer + 512;
  }
  if((Value & BIT3) != 0) {
    Summer = Summer + 1024;
  }  
  PrintAt(CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x02, L"%04dmA", Summer);
  SPrint(UnicodeStr, 32, L"%04d,", Summer);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show Charger REG04 Info - Charge Voltage Control Register

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerREG04Info(
  IN  CHAR8  *String
  )
{
  UINT8   Data = 0;
  UINT8   Value = 0;
  UINT8   High = 0;
  UINT16  Low = 0;  
  UINT32  Summer  = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x04, 1, &Data);  
  
  Value = Data >> 2;
  Value = Value & 0x3F;
  Summer = 3504;
  if((Value & BIT0) != 0) {
    Summer = Summer + 16;
  }
  if((Value & BIT1) != 0) {
    Summer = Summer + 32;
  }
  if((Value & BIT2) != 0) {
    Summer = Summer + 64;
  }
  if((Value & BIT3) != 0) {
    Summer = Summer + 128;
  }  
  if((Value & BIT4) != 0) {
    Summer = Summer + 256;
  }
  if((Value & BIT5) != 0) {
    Summer = Summer + 512;
  }  
  High = (UINT8)(Summer /1000);
  Low = (UINT16)(Summer - (High * 1000));
  
  PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x03, L"%d.%03dV", High, Low);
  SPrint(UnicodeStr, 32, L"%d.%03d,", High, Low);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);

   Value = Data & 0x01;
   if(Value == 1) {
     PrintAt(CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x03, L"300mV");
     strcat(String, "300,");
   } else {
     PrintAt(CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x03, L"100mV");
     strcat(String, "100,");
   }

}

/**
  Show Charger REG05 Info - Charge Termination/Timer Control Register

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerREG05Info(
  IN  CHAR8  *String
  )
{
  UINT8   Data = 0;
  UINT8   Value = 0;
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x05, 1, &Data);  
  
  Value = Data >> 1;
  Value = Value & 0x03;
  
  switch(Value) {
  case 0:
    PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x04, L"5 hrs");
    strcat(String, "5,");
    break;
  case 1:
    PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x04, L"8 hrs");
    strcat(String, "8,");
    break;
  case 2:
    PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x04, L"12hrs");
    strcat(String, "12,");
    break;
  case 3:
    PrintAt(CHARGERDATA_X + 0x1B, CHARGERDATA_Y + 0x04, L"20hrs");
    strcat(String, "20,");
    break;    
  }   
}

/**
  Show Charger REG08 Info - System Status Register

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerREG08Info(
  IN  CHAR8  *String
  )
{
  UINT8   Data = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x08, 1, &Data);  
  
  PrintAt(CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x04, L"%02x", Data);
  SPrint(UnicodeStr, 32, L"%02x,", Data);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
  ShowStatusInformation(Data);
}

/**
  Show ULPMC ChargeCurrent(), Command 0x2e,0x2f

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcChargeCurrent(
  IN  CHAR8  *String
  )
{
  UINT8   Data = 0;
  UINT8   Value = 0;
  UINT32  Summer = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x2E, 1, &Data);  
  
  Value = Data >> 2;
  Value = Value & 0x3F;
   Summer = 500;
  if((Value & BIT0) != 0) {
    Summer = Summer + 64;
  }
  if((Value & BIT1) != 0) {
    Summer = Summer + 128;
  }
  if((Value & BIT2) != 0) {
    Summer = Summer + 256;
  }
  if((Value & BIT3) != 0) {
    Summer = Summer + 512;
  }
  if((Value & BIT4) != 0) {
    Summer = Summer + 1024;
  }
  if((Value & BIT5) != 0) {
    Summer = Summer + 2048;
  }
  
  PrintAt(CHARGERDATA_X + 0x42, CHARGERDATA_Y + 0x01, L"%04dmA", Summer);
  SPrint(UnicodeStr, 32, L"%04d,", Summer);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC Temperature(), Command 0x06,0x07

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcTemperature(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  UINT16  Index0;
  UINT16  Index1;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x07, 1, &High);  
  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x06, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;

  Index0 = Value / 10;
  Index1 = Value - (Index0 * 10);
  
  PrintAt(BATTERYDATA_X + 0x09, BATTERYDATA_Y + 0x00, L"%03d.%01d K", Index0, Index1);
  SPrint(UnicodeStr, 32, L"%03d.%01d,", Index0, Index1);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC Voltage(), Command 0x08,0x09

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcVoltage(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x09, 1, &High);  
  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x08, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;
   
  PrintAt(BATTERYDATA_X + 0x2E, BATTERYDATA_Y + 0x00, L"%04dmV", Value);
  SPrint(UnicodeStr, 32, L"%04d,", Value);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC Flag(), Command 0x0a,0x0b

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcFlag(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x0B, 1, &High);  
  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x0A, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt(BATTERYDATA_X + 0x09, BATTERYDATA_Y + 0x01, L"%04x", Value);
  SPrint(UnicodeStr, 32, L"%d,", Value);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC NominalAvailableCapacity(), Command 0x0c,0x0d

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcNominalAvailableCapacity(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x0D, 1, &High);  
  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x0C, 1, &Low);  

  Value =High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt(BATTERYDATA_X + 0x2E, BATTERYDATA_Y + 0x01, L"%04dmAh", Value);
  SPrint(UnicodeStr, 32,  L"%04d,", Value);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC FullAvailableCapacity(), Command 0x0e,0x0f
  
  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcFullAvailableCapacity(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x0F, 1, &High);  
  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x0E, 1, &Low);  

  Value =High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt(BATTERYDATA_X + 0x09, BATTERYDATA_Y + 0x02, L"%04dmAh", Value);
  SPrint(UnicodeStr, 32,  L"%04d,", Value);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC RemainingCapacity(), Command 0x10,0x11

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcRemainingCapacity(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode(3);

  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x11, 1, &High);  
  ByteReadI2C(ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x10, 1, &Low);  

  Value =High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt(BATTERYDATA_X + 0x2E, BATTERYDATA_Y + 0x02, L"%04dmAh", Value);
  SPrint(UnicodeStr, 32,  L"%04d,", Value);
  UnicodeStrToAsciiStr(UnicodeStr, AsciiStr);
  strcat(String, AsciiStr);
}

/**
  Show ULPMC FullChanrgeCapacity(), Command 0x12,0x13

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcFullChanrgeCapacity(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode (3);

  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x13, 1, &High);  
  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x12, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt (BATTERYDATA_X + 0x09, BATTERYDATA_Y + 0x03, L"%04dmAh", Value);
  SPrint (UnicodeStr, 32,  L"%04d,", Value);
  UnicodeStrToAsciiStr (UnicodeStr, AsciiStr);
  strcat (String, AsciiStr);
}

/**
  Show ULPMC AverageCurrent(), Command 0x14,0x15

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcAverageCurrent(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode (3);

  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x15, 1, &High);  
  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x14, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt (BATTERYDATA_X + 0x2E, BATTERYDATA_Y + 0x03, L"%04dmA", Value);
  if ((Value & 0x8000) != 0) {
    Value = 0xFFFF - Value;
    SPrint (UnicodeStr, 32,  L"-%04d,", Value);
  } else {
    SPrint (UnicodeStr, 32,  L"%04d,", Value);
  }
  UnicodeStrToAsciiStr (UnicodeStr, AsciiStr);
  strcat (String, AsciiStr);
}

/**
  Show ULPMC CycleCount(), Command 0x2a/0x2b

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcCycleCount(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode (3);

  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x2B, 1, &High);  
  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x2A, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt (BATTERYDATA_X + 0x09, BATTERYDATA_Y + 0x04, L"%04x", Value);
  SPrint (UnicodeStr, 32,  L"%d,", Value);
  UnicodeStrToAsciiStr (UnicodeStr, AsciiStr);
  strcat (String, AsciiStr);
}

/**
  Show ULPMC StateOfCharge(), Command 0x2c/0x2d

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowUlpmcStateOfCharge(
  IN  CHAR8  *String
  )
{
  UINT8   High = 0;
  UINT8   Low = 0;
  UINT16  Value = 0;
  CHAR16  UnicodeStr[32];
  CHAR8   AsciiStr[32];    
  
  SetFrmMode (3);

  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x2D, 1, &High);  
  ByteReadI2C (ULPMC_BUS_NUM, ULPMC_SLAVEADDRESS, 0x2C, 1, &Low);  

  Value = High;
  Value = Value << 8;
  Value = Value + Low;
  
  PrintAt (BATTERYDATA_X + 0x2E, BATTERYDATA_Y + 0x04, L"%03d %%", Value);
  SPrint (UnicodeStr, 32,  L"%d\n", Value);
  UnicodeStrToAsciiStr (UnicodeStr, AsciiStr);
  strcat (String, AsciiStr);
}

/**
  ShowChargerInformation

  @param[out] String        Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowChargerInformation(
  OUT  CHAR8  *String
  )
{
  String[0] = 0;
  ShowFrameWork ();
//   ShowChargerREG00Info (String);
//   ShowChargerREG01Info (String);  
//   ShowChargerREG03Info (String); 
//   ShowChargerREG04Info (String); 
//   ShowChargerREG05Info (String); 
//   ShowChargerREG08Info (String); 
  ShowUlpmcChargeCurrent (String);  
  ShowUlpmcTemperature (String);
  ShowUlpmcVoltage (String);
  ShowUlpmcFlag (String);
  ShowUlpmcNominalAvailableCapacity (String);
  ShowUlpmcFullAvailableCapacity (String);
  ShowUlpmcRemainingCapacity (String);
  ShowUlpmcFullChanrgeCapacity (String);
  ShowUlpmcAverageCurrent (String);
  ShowUlpmcCycleCount (String);
  ShowUlpmcStateOfCharge (String);
}

/**
  ShowChargerREG00Info

  @param[out] String        Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowBatteryInformation(
  OUT  CHAR8  *String
  )
{
  String[0] = 0;
//   ShowChargerREG08Info (String); 
  ShowUlpmcTemperature (String);
  ShowUlpmcVoltage (String);
  ShowUlpmcFlag (String);
  ShowUlpmcNominalAvailableCapacity (String);
  ShowUlpmcFullAvailableCapacity (String);
  ShowUlpmcRemainingCapacity (String);  
  ShowUlpmcFullChanrgeCapacity (String);  
  ShowUlpmcAverageCurrent (String);
  ShowUlpmcStateOfCharge (String);
}

/**
  ShowTimeBar

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowTimeBar(
  IN  CHAR8   *String
  )
{
  EFI_TIME  CTime;
  CHAR16    UnicodeStr[128];

  SetFrmMode(4);
  
  gRT->GetTime (&CTime, NULL);
  
  SPrint (
    UnicodeStr, 
    128, 
    L"%04d-%02d-%02d %02d:%02d:%02d", 
    CTime.Year, 
    CTime.Month, 
    CTime.Day, 
    CTime.Hour, 
    CTime.Minute, 
    CTime.Second
    );
  
  PrintAt (TIME_BAR_X, TIME_BAR_Y, L"%s", UnicodeStr);
  UnicodeStrToAsciiStr (UnicodeStr, String);
  strcat (String, ",");
}

/**
  ShowStatusInformation

  @param[in] String         Pointer to string.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
ShowStatusInformation(
  IN  UINT8   Data
  )
{
  UINT8  Value = 0;
  
  SetFrmMode (4);
  
  Value = Data >> 6;
  Value = Value & 0x03;
  
  switch(Value) {
  case 0:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 01, L"-- UnKnow       ");
    break;
  case 1:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 01, L"-- USB Host     ");
    break;
  case 2:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 01, L"-- Adapter Port ");
    break;
  case 3:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 01, L"-- OTG          ");
    break; 
  }
  
  Value = Data >> 4;
  Value = Value & 0x03;

  switch(Value) {
  case 0:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 02, L"-- Not Charging ");
    break;
  case 1:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 02, L"-- Pre-Charge   ");
    break;
  case 2:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 02, L"-- Fast Charging");
    break;
  case 3:
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 02, L"-- Charge Done  ");
    break;  
  }

  if ((Data & 0x08) == 0) {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 03, L"-- Not DPM         ");       
  } else {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 03, L"-- VINDPM or IINDPM");    
  }

  if ((Data & 0x04) == 0) {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 04, L"-- Not Power Good  ");       
  } else {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 04, L"-- Power Good      ");    
  }

  if ((Data & 0x02) == 0) {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 04, L"-- Normal          ");       
  } else {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 04, L"-- TREG            ");    
  }

  if ((Data & 0x01) == 0) {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 04, L"-- Not in VSYSMIN regulation                ");       
  } else {
    PrintAt (STATUS_BAR_X + 03, STATUS_BAR_Y + 04, L"-- In VSYSMIN regulation Batterfy is too low");    
  }
  
}

/**
  GetFileName

  @param[in] Name           Pointer to Name String.
  
  @retval None.             The entry point is executed successfully.

**/
VOID
GetFileName(
  OUT  CHAR16 *Name
  )
{
  SetFrmMode (1);  
  PrintAt (8, HELP_INFO_BAR + 00, L"                                        ");
  PrintAt (8, HELP_INFO_BAR + 01, L" File Name: [                         ] ");
  PrintAt (8, HELP_INFO_BAR + 02, L"                                        ");
  GetUserInputString (24, 8, Name);
  SetFrmMode (0);
}

