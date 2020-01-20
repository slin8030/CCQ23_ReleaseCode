/** @file
  KeyBoard Input Functions.

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
#include <Print.h>

extern EFI_SYSTEM_TABLE    *gST;
extern EFI_BOOT_SERVICES   *gBS;
extern UINT8               gColumn;
extern UINT8               gRow;

/**
  Get Key.

  @param[out] Key           Input key.
  
  @retval None.             None.

**/
VOID
GetKey(
  OUT EFI_INPUT_KEY *Key
  )
{
  gBS->Stall(1000 * 1000);
  gST->ConIn->ReadKeyStroke (gST->ConIn, Key);   
}

/**
  Get Input key.

  @param[out] Key            Input key.
  
  @retval None.             None.

**/
VOID
GetInput(
  OUT EFI_INPUT_KEY *Key
  )
{
  UINTN  EventIndex;
  
  gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &EventIndex);  
  gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}

/**
  Get User Input String.

  @param[in]  Column        Column number.
  @param[in]  Row           Row number.
  @param[out] Data          Input data.
  
  @retval None.             None.

**/
VOID
GetUserInputString(
  IN  UINT8    Column,
  IN  UINT8    Row,
  OUT CHAR16  *Data
  )
{
  UINT8         Index = 0;  
  EFI_INPUT_KEY key;
  
  gST->ConOut->SetCursorPosition (gST->ConOut, Column, Row); 
   
  do {   
    key.ScanCode = 0;
    key.UnicodeChar = 0;
    GetInput (&key);    
    
    if (key.UnicodeChar == CHAR_BACKSPACE) {
      if (Index != 0) {
        Index--;
        Data[Index] = 0;
        PrintAt (Column, Row, L"                  ");	  
        PrintAt (Column, Row, L"%S", Data);	   
      }
      continue;	  
    }
    
    if(key.UnicodeChar == 0) {
      continue;
    }
    
    Data[Index] = key.UnicodeChar;
    Index++;
    Data[Index] = 0;
    PrintAt (Column, Row, L"                  ");	  
    PrintAt (Column, Row, L"%S", Data);
    
  } while((key.UnicodeChar != CHAR_CARRIAGE_RETURN) && (Index < 20));
  
  Data[Index - 1] = 0;
}

