//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/
#include "Utilities.h"

BOOLEAN
StringToUint32(
  CONST CHAR16 *pString,
  UINTN * pValue
  )
{
  UINT32 Value;
  UINT32 Digit;

  //  Check for a hex prefix
  Value = 0;
  if (( '0' == pString [0])
    && (( 'x' == pString [1] ) || ( 'X' == pString [1])))
  {
    //  Convert the value to hex
    while ( 0 != * pString )
    {
      //  Upper case the digit
      Digit = * pString;
      if ( 'a' <= Digit )
      {
        Digit &= ~ 0x20;
      }

      //  Validate the digit
      if (( '0' <= Digit ) && ( '9' >= Digit ))
      {
        Digit -= '0';
      }
      else if (( 'A' <= Digit ) && ( 'F' >= Digit ))
      {
        Digit -= 'A' - 10;
      }
      else
      {
        //  Bad conversion
        return FALSE;
      }

      Value <<= 4;
      Value += Digit;
      pString += 1;
    }
  }
  else
  {
    //  Convert the value to decimal
    while ( 0 != * pString )
    {
      Digit = * pString;
      if (( '0' > * pString ) || ( '9' < * pString ))
      {
        //  Bad conversion
        return FALSE;
      }

      Value *= 10;
      Value += Digit - '0';
      pString += 1;
    }
  }

  //  Return the value
  * pValue = Value;
  return TRUE;
}

BOOLEAN
StringToUint16(
  CONST CHAR16 *pString,
  UINT16 * pValue
  )
{
  UINT16 Value;
  UINT16 Digit;

  //  Check for a hex prefix
  Value = 0;
  if (( '0' == pString [0])
    && (( 'x' == pString [1] ) || ( 'X' == pString [1])))
  {
    //  Convert the value to hex
    while ( 0 != * pString )
    {
      //  Upper case the digit
      Digit = * pString;
      if ( 'a' <= Digit )
      {
        Digit &= ~ 0x20;
      }

      //  Validate the digit
      if (( '0' <= Digit ) && ( '9' >= Digit ))
      {
        Digit -= '0';
      }
      else if (( 'A' <= Digit ) && ( 'F' >= Digit ))
      {
        Digit -= 'A' - 10;
      }
      else
      {
        //  Bad conversion
        return FALSE;
      }

      Value <<= 4;
      Value += Digit;
      pString += 1;
    }
  }
  else
  {
    //  Convert the value to decimal
    while ( 0 != * pString )
    {
      Digit = * pString;
      if (( '0' > * pString ) || ( '9' < * pString ))
      {
        //  Bad conversion
        return FALSE;
      }

      Value *= 10;
      Value += Digit - '0';
      pString += 1;
    }
  }

  //  Return the value
  * pValue = Value;
  return TRUE;
}

BOOLEAN
StringToUint8(
  CONST CHAR16 *pString,
  UINT8 * pValue
  )
{
  UINT8 Value;
  UINT8 Digit;

  //  Check for a hex prefix
  Value = 0;
  if (( '0' == pString [0])
    && (( 'x' == pString [1] ) || ( 'X' == pString [1])))
  {
    //  Convert the value to hex
    while ( 0 != * pString )
    {
      //  Upper case the digit
      Digit = (UINT8)(* pString);
      if ( 'a' <= Digit )
      {
        Digit &= ~ 0x20;
      }

      //  Validate the digit
      if (( '0' <= Digit ) && ( '9' >= Digit ))
      {
        Digit -= '0';
      }
      else if (( 'A' <= Digit ) && ( 'F' >= Digit ))
      {
        Digit -= 'A' - 10;
      }
      else
      {
        //  Bad conversion
        return FALSE;
      }

      Value <<= 4;
      Value += Digit;
      pString += 1;
    }
  }
  else
  {
    //  Convert the value to decimal
    while ( 0 != * pString )
    {
      Digit = (UINT8)(* pString);
      if (( '0' > * pString ) || ( '9' < * pString ))
      {
        //  Bad conversion
        return FALSE;
      }

      Value *= 10;
      Value += Digit - '0';
      pString += 1;
    }
  }

  //  Return the value
  * pValue = Value;
  return TRUE;
}


BOOLEAN 
GetWriteArray(
  IN CHAR16 *WriteString,
  OUT UINT8 *CmdWriteBufferLength,
  OUT UINT8 *CmdWriteBuffer
  )
{
    BOOLEAN ParseResult = FALSE;
    UINT8 WriteValueCount = 0;  //Counter of write value. No larger than Input CmdWriteBufferLength;
    UINT8 ParseCursor = 0;   //Cursor for parsing the writestring.
    

    if(CmdWriteBuffer == NULL||*CmdWriteBufferLength==0){
        ParseResult = FALSE;
        goto _ExitParse;
    }

    //
    //The array requires a start of "{" and an end of "}".In case the input array is larger than
    //specified length, truncate it to the input length; In case it's smaller than specified length,
    //output CmdWriteBufferLength will indicate the actual length.
    //{0x18,0x2,0x3f}
    //
    if('{' != WriteString[0]){
        //
        //Invalid array start indicator
        //
        Print(L"Invalid write buffer format. Must start with {.\n");
        ParseResult = FALSE;
        goto _ExitParse;
    }
    WriteString++; 

_ParseLoop:
    while((WriteString[ParseCursor] != ',' && WriteString[ParseCursor] != '}') && ParseCursor<4 && (WriteValueCount+1 <= (*CmdWriteBufferLength))){
        ParseCursor++;
    }
    //
    //Go forward at most 4 chars and then check existence of "," or "}". else these is a parse error and we exit
    //
    if(WriteString[ParseCursor] == ','){ //pending another 
        WriteString[ParseCursor] = 0;   //
        ParseResult = StringToUint8(WriteString, CmdWriteBuffer+WriteValueCount);
        //Print(L"Input %d: %d,\n", WriteValueCount, CmdWriteBuffer[WriteValueCount]);
        if(!ParseResult){
            Print(L"Parse write input error.\n");
            goto _ExitParse;
        }
        WriteValueCount++;
        WriteString += ParseCursor+1;
        ParseCursor = 0;
    
        goto _ParseLoop;
    
    }
    else if(WriteString[ParseCursor] == '}'){//Finished the last
        WriteString[ParseCursor] = 0;
        ParseResult = StringToUint8(WriteString, CmdWriteBuffer+WriteValueCount);
        //Print(L"Final Input %d: %d,\n", WriteValueCount, CmdWriteBuffer[WriteValueCount]);
        if(!ParseResult){
            goto _ExitParse;
        }
        *CmdWriteBufferLength = WriteValueCount+1;
    }
    else{ //For all other case, fail the parse and return
        Print(L"Encounterred an error case, stop parsing input write buffer.\n");
        //ParseResult = FALSE;
    }
   
_ExitParse:

    return ParseResult;
}