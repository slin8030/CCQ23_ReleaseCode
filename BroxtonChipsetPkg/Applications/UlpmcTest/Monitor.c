/** @file
  Monitor Functions.

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
#include <Library/DebugLib.h>
#include <Library/I2cLib.h>

#include <Protocol/SimpleFileSystem.h>

#include <UlpmcTest.h>
#include <Monitor.h>
#include <WriteFile.h>
#include <Screen.h>
#include <Print.h>

extern EFI_BOOT_SERVICES     *gBS;

/**
  String To Hex.

  @param[in]  String        Pointer to string.
  
  @retval Value.            The Hex number.

**/
UINTN
StringToHex(
  IN   CHAR8  *String
 )
{
  UINT32  Value = 0;
  UINT8   Index = 0;

  while(String[Index] != 0)
  {
    DEBUG((EFI_D_ERROR, "%02x ", String[Index]));
    if((String[Index] >= '0') && (String[Index] <= '9')) {
      Value = Value * 16 + (String[Index] - '0');
    } else if((String[Index] >= 'a') && (String[Index] <= 'z')) {
      Value = Value * 16 + (String[Index] - 'a' + 10);
    } else if((String[Index] >= 'A') && (String[Index] <= 'Z')) {
      Value = Value * 16 + (String[Index] - 'A' + 10);
    }
    Index++;
  }
  return Value;
}

/**
  Get Monitor Item Data.

  @param[in]     String     Pointer to string.
  @param[in,out] Index      Pointer to Index of monitor device.
  
  @retval Data.             Monitor Item Data.

**/
UINT8
GetMonitorItemData(
  IN     CHAR8  *String,
  IN OUT UINTN  *Index
  )
{
  CHAR8  Temp[8];
  UINTN  Index0 = *Index;
  UINTN  Index1 = 0;
  UINT8  Data;

  while((String[Index0] != ',') && (String[Index0] != 0) && (Index1 < 2)) {
    Temp[Index1] = String[Index0];
    Index1++;
    Index0++;	
  }
  
  Index0++;
  Temp[Index1] = 0;
  *Index = Index0;
  
  Data = (UINT8)StringToHex(Temp);
  
  return Data;
}

/**
  Analyze Monitor String.

  @param[out] mReg          Monitor Item Data.
  @param[in]  String        Pointer to string.
  
  @retval None              None.

**/
VOID
AnalyzeMonitorString(
  OUT  MONITOR_REG   *mReg,
  IN   CHAR8         *String
  )
{
  UINTN Index = 0;
  
  mReg->MonitorAddress  = GetMonitorItemData(String, &Index);
  mReg->MonitorReg      = GetMonitorItemData(String, &Index);
  mReg->MonitorStartBit = GetMonitorItemData(String, &Index);
  mReg->MonitorEndBit   = GetMonitorItemData(String, &Index);
  mReg->MonitorValue    = GetMonitorItemData(String, &Index);
  mReg->ChangeAddress 	= GetMonitorItemData(String, &Index);
  mReg->ChangeReg       = GetMonitorItemData(String, &Index);
  mReg->ChangeStartBit	= GetMonitorItemData(String, &Index);
  mReg->ChangeEndBit	  = GetMonitorItemData(String, &Index);
  mReg->ChangeValue     = GetMonitorItemData(String, &Index);    
}


/**
  Get Monitor List.

  @param[in]   FilePtr      Pointer to file.
  @param[out]  mReg         Monitor Item Data.
  
  @retval EFI_SUCCESS       The process complete.

**/
EFI_STATUS
GetMonitorList(
  IN   EFI_FILE     *FilePtr,
  OUT  MONITOR_ITEM *mReg
  )
{
  EFI_STATUS  Status;
  UINTN       Index = 0;

  //
  // Get the number of monitor device
  //
  Index = 0;
  do {
    CHAR8  String[256]; 
    
    Status = ReadStringLine(FilePtr, 256, String);
    if (String[0] == ';') {
      continue;
    }
    Index++;
  } while(!EFI_ERROR(Status));

  //
  // Allocate the space to put the data
  //
  mReg->MonitorNum = (UINT8) Index;
  Status = gBS->AllocatePool (
  	               EfiBootServicesData,
                   sizeof(MONITOR_REG) * Index,
                   &mReg->MonitorRegs
                   );

  FilePtr->SetPosition(FilePtr, 0);

  //
  // Read the each device data
  //
  Index = 0;
  do {
    CHAR8  String[256]; 
    
    Status = ReadStringLine(FilePtr, 256, String);
    if (String[0] == ';') {
      continue;
    }
    AnalyzeMonitorString(&(mReg->MonitorRegs[Index]), String);
    Index++;
  } while(mReg->MonitorNum != Index);
  
  return EFI_SUCCESS;
}

/**
  Get Monitor Register Data.

  @param[out]  mReg         Monitor Item Data.
  
  @retval Data              Monitor Item Data.

**/
UINT8
GetMonitorRegisterData(
  IN  MONITOR_REG  mReg
  )
{
  UINT8  MarkBit = 1;
  UINT8  Index;
  UINT8  Data;

  //
  // Read Data from I2C
  //
  ByteReadI2C(ULPMC_BUS_NUM, mReg.MonitorAddress, mReg.MonitorReg, 1, &Data);  
  Data = Data >> mReg.MonitorStartBit;
  
  for(Index = mReg.MonitorStartBit; Index <= mReg.MonitorEndBit; Index++) {
    MarkBit = MarkBit * 2;
  }
  
  MarkBit = MarkBit - 1;
  Data = Data & MarkBit;
  
  return Data;
}

/**
  Set Monitor Register Data.

  @param[in]  mReg          Monitor Item Data.
  
  @retval None              None.

**/
VOID
SetMonitorRegisterData(
  IN  MONITOR_REG  mReg
  )
{
  UINT8  MarkLow = 1;
  UINT8  MarkHigh = 1;  
  UINT8  Index;
  UINT8  Data;
  UINT8  High;
  UINT8  Low;

  for(Index = 0; Index < mReg.ChangeStartBit; Index++) {
    MarkLow = MarkLow * 2;
  }
  MarkLow = MarkLow - 1;
  
  for(Index = mReg.ChangeEndBit; Index < 7; Index++) {
    MarkHigh = MarkHigh * 2;
  }
  MarkHigh = MarkHigh - 1;
	  
  ByteReadI2C(ULPMC_BUS_NUM, mReg.ChangeAddress, mReg.ChangeReg, 1, &Data);  
  High = Data & MarkHigh;
  Low  = Data & MarkLow;
  mReg.ChangeValue = mReg.ChangeValue << mReg.ChangeStartBit;
  Data = High | Low | mReg.ChangeValue;
  ByteWriteI2C(ULPMC_BUS_NUM, mReg.ChangeAddress, mReg.ChangeReg, 1, &Data);
  
}


/**
  Check Monitor Register.

  @param[in]  mReg          Monitor Item Data.
  @param[in]  FilePtr       Pointer to file.
  
  @retval None              None.

**/
VOID
CheckMonitorRegister(
  IN MONITOR_ITEM mReg,
  IN EFI_FILE     *FilePtr
  )
{
  UINT8  Index = 0;
  UINT8  Data = 0; 
  
  while(mReg.MonitorNum != Index) {
    
    Data = GetMonitorRegisterData (mReg.MonitorRegs[Index]);
    
    if(Data == mReg.MonitorRegs[Index].MonitorValue) {
      
      CHAR8   String[512];
      CHAR16  TempUnicodeStr[256];
      CHAR8   TempAsciiStr[256];	  
	  
      SetMonitorRegisterData (mReg.MonitorRegs[Index]);
	  
      ShowTimeBar (String);	
      SPrint(
        TempUnicodeStr, 
        256, 
        L"   Device[%02x]  Reg[%02x] reach[%02x]  Modify  Device[%02x]  Reg[%02x] to[%02x]\n", 
	      mReg.MonitorRegs[Index].MonitorAddress, 
	      mReg.MonitorRegs[Index].MonitorReg, 
	      mReg.MonitorRegs[Index].MonitorValue,
				mReg.MonitorRegs[Index].ChangeAddress, 
				mReg.MonitorRegs[Index].ChangeReg, 
				mReg.MonitorRegs[Index].ChangeValue
				);
      UnicodeStrToAsciiStr (TempUnicodeStr, TempAsciiStr);
      strcat (String, TempAsciiStr);
      WriteString2File (FilePtr, String);		  
	  
    }
    
    Index++;
  }
}
