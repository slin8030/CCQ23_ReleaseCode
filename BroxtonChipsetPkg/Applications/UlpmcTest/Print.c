/** @file
  Print Functions.

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
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Print.h>

extern EFI_SYSTEM_TABLE  *gST;

static CHAR16 mHexStr[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
                            L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };

/**
  VSPrint worker function that prints a Value as a hex number in Buffer

  @param[in]  Buffer        Location to place ascii hex string of Value.
  @param[in]  Value         Hex value to convert to a string in Buffer.
  @param[in]  Flags         Flags to use in printing Hex string, see file header for details.
  @param[in]  Width         Width of hex value.
  
  @retval Index             Number of characters printed.

**/
UINTN
EfiValueToHexStr (
  IN  OUT CHAR16  *Buffer,
  IN  UINT64      Value,
  IN  UINTN       Flags,
  IN  UINTN       Width
  )
{
  CHAR16  TempBuffer[CHARACTER_NUMBER_FOR_VALUE];
  CHAR16  *TempStr;
  CHAR16  Prefix;
  CHAR16  *BufferPtr;
  UINTN   Count;
  UINTN   Index;

  TempStr   = TempBuffer;
  BufferPtr = Buffer;

  //
  // Count starts at one since we will null terminate. Each iteration of the
  // loop picks off one nibble. Oh yea TempStr ends up backwards
  //
  Count = 0;

  if (Width > CHARACTER_NUMBER_FOR_VALUE - 1) {
    Width = CHARACTER_NUMBER_FOR_VALUE - 1;
  }

  do {
    Index = ((UINTN)Value & 0xf);
    *(TempStr++) = mHexStr[Index];
    Value = RShiftU64 (Value, 4);
    Count++;
  } while (Value != 0);

  if (Flags & PREFIX_ZERO) {
    Prefix = '0';
  } else {
    Prefix = ' ';
  }

  Index = Count;
  if (!(Flags & LEFT_JUSTIFY)) {
    for (; Index < Width; Index++) {
      *(TempStr++) = Prefix;
    }
  }

  //
  // Reverse temp string into Buffer.
  //
  if (Width > 0 && (UINTN) (TempStr - TempBuffer) > Width) {
    TempStr = TempBuffer + Width;
  }
  Index = 0;
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
    Index++;
  }

  *BufferPtr = 0;
  return Index;
}

/**
  SPrint function to process format and place the results in Buffer.

  @param[in]  Buffer        Wide char buffer to print the results of the parsing of Format into.
  @param[in]  BufferSize    Maximum number of characters to put into buffer. Zero means no 
                            limit.
  @param[in]  Format        Format string see file header for more details.
  @param[in]  ...           Vararg list consumed by processing Format.

  @retval Return            Number of characters printed.

**/
UINTN
SPrint (
  OUT CHAR_W        *Buffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR_W  *Format,
  ...
  )
{
  UINTN   Return;
  VA_LIST Marker;

  VA_START (Marker, Format);
  Return = VSPrint (Buffer, BufferSize, Format, Marker);
  VA_END (Marker);

  return Return;
}

/**
  VSPrint worker function that parses flag and width information from the 
  Format string and returns the next index into the Format string that needs
  to be parsed. See file headed for details of Flag and Width.

  @param[in]  Format        Current location in the VSPrint format string.
  @param[in]  Flags         Returns flags
  @param[in]  Width         Returns width of element
  @param[in]  Marker        Vararg list that may be paritally consumed and returned.

  @retval Format            Pointer indexed into the Format string for all the information parsed
                            by this routine.

**/
STATIC
CHAR_W *
GetFlagsAndWidth (
  IN  CHAR_W      *Format, 
  OUT UINTN       *Flags, 
  OUT UINTN       *Width,
  IN OUT  VA_LIST *Marker
  )
{
  UINTN   Count;
  BOOLEAN Done;

  *Flags = 0;
  *Width = 0;
  for (Done = FALSE; !Done; ) {
    Format++;

    switch (*Format) {

    case '-': *Flags |= LEFT_JUSTIFY; break;
    case '+': *Flags |= PREFIX_SIGN;  break;
    case ' ': *Flags |= PREFIX_BLANK; break;
    case ',': *Flags |= COMMA_TYPE;   break;
    case 'L':
    case 'l': *Flags |= LONG_TYPE;    break;

    case '*':
      *Width = VA_ARG (*Marker, UINTN);
      break;

    case '0':
      *Flags |= PREFIX_ZERO;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      Count = 0;
      do {
        Count = (Count * 10) + *Format - '0';
        Format++;
      } while ((*Format >= '0')  &&  (*Format <= '9'));
      Format--;
      *Width = Count;
      break;

    default:
      Done = TRUE;
    }
  }
  return Format;
}

/*++
  VSPrint function to process format and place the results in Buffer. Since a 
  VA_LIST is used this rountine allows the nesting of Vararg routines. Thus 
  this is the main print working routine

  @param[in]  StartOfBuffer Unicode buffer to print the results of the parsing of Format into.
  @param[in]  BufferSize    Maximum number of characters to put into buffer. Zero means 
                            no limit.
  @param[in]  FormatString  Unicode format string see file header for more details.
  @param[in]  Marker        Vararg list consumed by processing Format.

  @retval Format            Number of characters printed.

*/
UINTN
EFIAPI
VSPrint (
  OUT CHAR_W        *StartOfBuffer,
  IN  UINTN         BufferSize,
  IN  CONST CHAR_W  *FormatString,
  IN  VA_LIST       Marker
  )
{
  CHAR16    TempBuffer[CHARACTER_NUMBER_FOR_VALUE];
  CHAR_W    *Buffer;
  CHAR8     *AsciiStr;
  CHAR16    *UnicodeStr;
  CHAR_W    *Format;
  UINTN     Index;
  UINTN     Flags;
  UINTN     Width;
  UINTN     Count;
  UINTN     NumberOfCharacters;
  UINTN     BufferLeft;
  UINT64    Value;
  EFI_GUID  *TmpGUID;

  //
  // Process the format string. Stop if Buffer is over run.
  //

  Buffer              = StartOfBuffer;
  Format              = (CHAR_W *) FormatString;
  NumberOfCharacters  = BufferSize / sizeof (CHAR_W);
  BufferLeft          = BufferSize;
  for (Index = 0; (*Format != '\0') && (Index < NumberOfCharacters - 1); Format++) {
    if (*Format != '%') {
      if ((*Format == '\n') && (Index < NumberOfCharacters - 2)) {
        //
        // If carage return add line feed
        //
        Buffer[Index++] = '\r';
        BufferLeft -= sizeof (CHAR_W);
      }

      Buffer[Index++] = *Format;
      BufferLeft -= sizeof (CHAR_W);
    } else {
      
      //
      // Now it's time to parse what follows after %
      //
      Format = GetFlagsAndWidth (Format, &Flags, &Width, &Marker);
      switch (*Format) {
      case 'X':
        Flags |= PREFIX_ZERO;
        Width = sizeof (UINT64) * 2;

      //
      // break skiped on purpose
      //
      case 'x':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = VA_ARG (Marker, UINT64);
        } else {
          Value = VA_ARG (Marker, UINTN);
        }

        EfiValueToHexStr (TempBuffer, Value, Flags, Width);
        UnicodeStr = TempBuffer;

        for (; (*UnicodeStr != '\0') && (Index < NumberOfCharacters - 1); UnicodeStr++) {
          Buffer[Index++] = *UnicodeStr;
        }
        break;

      case 'd':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = VA_ARG (Marker, UINT64);
        } else {
          Value = (UINTN) VA_ARG (Marker, UINTN);
        }

        EfiValueToString (TempBuffer, Value, Flags, Width);
        UnicodeStr = TempBuffer;

        for (; (*UnicodeStr != '\0') && (Index < NumberOfCharacters - 1); UnicodeStr++) {
          Buffer[Index++] = *UnicodeStr;
        }
        break;

      case 's':
      case 'S':
        UnicodeStr = (CHAR16 *) VA_ARG (Marker, CHAR_W *);
        if (UnicodeStr == NULL) {
          UnicodeStr = L"<null string>";
        }

        for (Count = 0; (*UnicodeStr != '\0') && (Index < NumberOfCharacters - 1); UnicodeStr++, Count++) {
          Buffer[Index++] = *UnicodeStr;
        }
        //
        // Add padding if needed
        //
        for (; (Count < Width) && (Index < NumberOfCharacters - 1); Count++) {
          Buffer[Index++] = ' ';
        }

        break;

      case 'a':
        AsciiStr = (CHAR8 *) VA_ARG (Marker, CHAR8 *);
        if (AsciiStr == NULL) {
          AsciiStr = "<null string>";
        }

        for (Count = 0; (*AsciiStr != '\0') && (Index < NumberOfCharacters - 1); AsciiStr++, Count++) {
          Buffer[Index++] = (CHAR_W) * AsciiStr;
        }
        //
        // Add padding if needed
        //
        for (; (Count < Width) && (Index < NumberOfCharacters - 1); Count++) {
          Buffer[Index++] = ' ';
        }
        break;

      case 'c':
        Buffer[Index++] = (CHAR_W) VA_ARG (Marker, UINTN);
        break;

      case 'g':
        TmpGUID = VA_ARG (Marker, EFI_GUID *);
        if (TmpGUID != NULL) {
          Index += GuidToString (
                    TmpGUID,
                    &Buffer[Index],
                    BufferLeft
                    );
        }
        break;

      case 't':
        Index += TimeToString (
                  VA_ARG (Marker, EFI_TIME *), 
                  &Buffer[Index],
                  BufferLeft
                  );
        break;

      case 'r':
        Index += EfiStatusToString (
                  VA_ARG (Marker, EFI_STATUS), 
                  &Buffer[Index],
                  BufferLeft
                  );
        break;

      case '%':
        Buffer[Index++] = *Format;
        break;

      default:
        //
        // if the type is unknown print it to the screen
        //
        Buffer[Index++] = *Format;
      }

      BufferLeft = BufferSize - Index * sizeof (CHAR_W);
    }
  }

  Buffer[Index++] = '\0';

  return &Buffer[Index] - StartOfBuffer;
}


/*++
  VSPrint worker function that prints a Value as a decimal number in Buffer

  @param[in]  SBuffer       Location to place ascii decimal number string of Value.
  @param[in]  SValue        Decimal value to convert to a string in Buffer.
  @param[in]  SFlags        Flags to use in printing decimal string, see file header for details.
  @param[in]  SWidth        Width of hex value.

  @retval Index             Number of characters printed.

*/
UINTN
EfiValueToString (
  IN  OUT CHAR16  *Buffer,
  IN  INT64       Value,
  IN  UINTN       Flags,
  IN  UINTN       Width
  )
{
  CHAR16    TempBuffer[CHARACTER_NUMBER_FOR_VALUE];
  CHAR16    *TempStr;
  CHAR16    *BufferPtr;
  UINTN     Count;
  UINTN     ValueCharNum;
  UINTN     Remainder;
  CHAR16    Prefix;
  UINTN     Index;
  BOOLEAN   ValueIsNegative;
  UINT64    TempValue;

  TempStr         = TempBuffer;
  BufferPtr       = Buffer;
  Count           = 0;
  ValueCharNum    = 0;
  ValueIsNegative = FALSE;

  if (Width > CHARACTER_NUMBER_FOR_VALUE - 1) {
    Width = CHARACTER_NUMBER_FOR_VALUE - 1;
  }

  if (Value < 0) {
    Value           = -Value;
    ValueIsNegative = TRUE;
  }

  do {
    TempValue = Value;
    Value = (INT64)DivU64x32 ((UINT64)Value, 10);
    Remainder = (UINTN)((UINT64)TempValue - 10 * Value);
    *(TempStr++) = (CHAR16)(Remainder + '0');
    ValueCharNum++;
    Count++;
    if ((Flags & COMMA_TYPE) == COMMA_TYPE) {
      if (ValueCharNum % 3 == 0 && Value != 0) {
        *(TempStr++) = ',';
        Count++;
      }
    }
  } while (Value != 0);

  if (ValueIsNegative) {
    *(TempStr++)    = '-';
    Count++;
  }

  if ((Flags & PREFIX_ZERO) && !ValueIsNegative) {
    Prefix = '0';
  } else {
    Prefix = ' ';
  }

  Index = Count;
  if (!(Flags & LEFT_JUSTIFY)) {
    for (; Index < Width; Index++) {
      *(TempStr++) = Prefix;
    }
  }

  //
  // Reverse temp string into Buffer.
  //
  if (Width > 0 && (UINTN) (TempStr - TempBuffer) > Width) {
    TempStr = TempBuffer + Width;
  }
  Index = 0;
  while (TempStr != TempBuffer) {
    *(BufferPtr++) = *(--TempStr);
    Index++;
  }

  *BufferPtr = 0;
  return Index;
}


/*++
  VSPrint worker function that prints EFI_STATUS as a string. If string is
  not known a hex value will be printed.

  @param[in]  Status        EFI_STATUS sturcture to print.
  @param[in]  Buffer        Buffer to print EFI_STATUS message string into.
  @param[in]  BufferSize    Size of Buffer.
  
  @retval Format            Number of characters printed.

*/
UINTN
EfiStatusToString (
  IN EFI_STATUS   Status,
  OUT CHAR_W      *Buffer,
  IN  UINTN       BufferSize
  )
{
  UINTN   Size;
  CHAR8   *Desc;

  Desc = NULL;
  
  //
  // Can't use global Status String Array as UINTN is not constant for EBC
  //
  if (Status == EFI_SUCCESS) { Desc = "Success"; } else 
  if (Status == EFI_LOAD_ERROR) { Desc = "Load Error"; } else
  if (Status == EFI_INVALID_PARAMETER) { Desc = "Invalid Parameter"; } else
  if (Status == EFI_UNSUPPORTED) { Desc = "Unsupported"; } else
  if (Status == EFI_BAD_BUFFER_SIZE) { Desc = "Bad Buffer Size"; } else
  if (Status == EFI_BUFFER_TOO_SMALL) { Desc = "Buffer Too Small"; } else
  if (Status == EFI_NOT_READY) { Desc = "Not Ready"; } else
  if (Status == EFI_DEVICE_ERROR) { Desc = "Device Error"; } else
  if (Status == EFI_WRITE_PROTECTED) { Desc = "Write Protected"; } else
  if (Status == EFI_OUT_OF_RESOURCES) { Desc = "Out of Resources"; } else
  if (Status == EFI_VOLUME_CORRUPTED) { Desc = "Volume Corrupt"; } else
  if (Status == EFI_VOLUME_FULL) { Desc = "Volume Full"; } else
  if (Status == EFI_NO_MEDIA) { Desc = "No Media"; } else
  if (Status == EFI_MEDIA_CHANGED) { Desc = "Media changed"; } else
  if (Status == EFI_NOT_FOUND) { Desc = "Not Found"; } else
  if (Status == EFI_ACCESS_DENIED) { Desc = "Access Denied"; } else
  if (Status == EFI_NO_RESPONSE) { Desc = "No Response"; } else
  if (Status == EFI_NO_MAPPING) { Desc = "No mapping"; } else
  if (Status == EFI_TIMEOUT) { Desc = "Time out"; } else
  if (Status == EFI_NOT_STARTED) { Desc = "Not started"; } else
  if (Status == EFI_ALREADY_STARTED) { Desc = "Already started"; } else
  if (Status == EFI_ABORTED) { Desc = "Aborted"; } else
  if (Status == EFI_ICMP_ERROR) { Desc = "ICMP Error"; } else
  if (Status == EFI_TFTP_ERROR) { Desc = "TFTP Error"; } else
  if (Status == EFI_PROTOCOL_ERROR) { Desc = "Protocol Error"; } else
  if (Status == EFI_WARN_UNKNOWN_GLYPH) { Desc = "Warning Unknown Glyph"; } else
  if (Status == EFI_WARN_DELETE_FAILURE) { Desc = "Warning Delete Failure"; } else
  if (Status == EFI_WARN_WRITE_FAILURE) { Desc = "Warning Write Failure"; } else
  if (Status == EFI_WARN_BUFFER_TOO_SMALL) { Desc = "Warning Buffer Too Small"; } 

  //
  // If we found a match, copy the message to the user's buffer. Otherwise
  // sprint the hex status code to their buffer.
  //
  if (Desc != NULL) {
    Size = SPrint (Buffer, BufferSize, STRING_W ("%a"), Desc);
  } else {
    Size = SPrint (Buffer, BufferSize, STRING_W ("%X"), Status);
  }

  return Size - 1;
}


/*++
  VSPrint worker function that prints an EFI_GUID.

  @param[in]  Guid          Pointer to GUID to print.
  @param[in]  Buffer        Buffe to print Guid into.
  @param[in]  BufferSize    Size of Buffer.

  @retval Format            Number of characters printed.

*/
STATIC
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN  CHAR_W    *Buffer,
  IN  UINTN     BufferSize
  )
{
  UINTN Size;

  Size = SPrint (
          Buffer,
          BufferSize,
          STRING_W ("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
          (UINTN)Guid->Data1,
          (UINTN)Guid->Data2,
          (UINTN)Guid->Data3,
          (UINTN)Guid->Data4[0],
          (UINTN)Guid->Data4[1],
          (UINTN)Guid->Data4[2],
          (UINTN)Guid->Data4[3],
          (UINTN)Guid->Data4[4],
          (UINTN)Guid->Data4[5],
          (UINTN)Guid->Data4[6],
          (UINTN)Guid->Data4[7]
          );

  //
  // SPrint will null terminate the string. The -1 skips the null
  //
  return Size - 1;
}

/*++
  VSPrint worker function that prints EFI_TIME.

  @param[in]  Time         Pointer to EFI_TIME sturcture to print.
  @param[in]  Buffer       Buffer to print Time into.
  @param[in]  BufferSize   Size of Buffer.
  
  @retval Size             Number of characters printed.

*/
STATIC
UINTN
TimeToString (
  IN EFI_TIME   *Time,
  OUT CHAR_W    *Buffer,
  IN  UINTN     BufferSize
  )
{ 
  UINTN Size;

  Size = SPrint (
            Buffer,
            BufferSize, 
            STRING_W ("%02d/%02d/%04d  %02d:%02d"),
            (UINTN)Time->Month,
            (UINTN)Time->Day,
            (UINTN)Time->Year,
            (UINTN)Time->Hour,
            (UINTN)Time->Minute
            );

  //
  // SPrint will null terminate the string. The -1 skips the null
  //
  return Size - 1;
}

/**
  The internal function prints to the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
  protocol instance.

  @param Column          The position of the output string.
  @param Row             The position of the output string.
  @param Out             The EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL instance.
  @param Fmt             The format string.
  @param Args            The additional argument for the variables in the format string.

  @return Number of Unicode character printed.

**/
UINTN
PrintInternal (
  IN UINTN                            Column,
  IN UINTN                            Row,
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *Out,
  IN CHAR16                           *Fmt,
  IN VA_LIST                          Args
  )
{
  CHAR16  *Buffer;
  CHAR16  *BackupBuffer;
  UINTN   Index;
  UINTN   PreviousIndex;
  UINTN   Count;

  //
  // For now, allocate an arbitrarily long buffer
  //
  Buffer        = AllocateZeroPool (0x10000);
  BackupBuffer  = AllocateZeroPool (0x10000);
  ASSERT (Buffer);
  ASSERT (BackupBuffer);

  if (Column != (UINTN) -1) {
    Out->SetCursorPosition (Out, Column, Row);
  }

  UnicodeVSPrint (Buffer, 0x10000, Fmt, Args);

  Out->Mode->Attribute = Out->Mode->Attribute & 0x7f;

  Out->SetAttribute (Out, Out->Mode->Attribute);

  Index         = 0;
  PreviousIndex = 0;
  Count         = 0;

  do {
    for (; (Buffer[Index] != NARROW_CHAR) && (Buffer[Index] != WIDE_CHAR) && (Buffer[Index] != 0); Index++) {
      BackupBuffer[Index] = Buffer[Index];
    }

    if (Buffer[Index] == 0) {
      break;
    }
    //
    // Null-terminate the temporary string
    //
    BackupBuffer[Index] = 0;

    //
    // Print this out, we are about to switch widths
    //
    Out->OutputString (Out, &BackupBuffer[PreviousIndex]);
    Count += StrLen (&BackupBuffer[PreviousIndex]);

    //
    // Preserve the current index + 1, since this is where we will start printing from next
    //
    PreviousIndex = Index + 1;

    //
    // We are at a narrow or wide character directive.  Set attributes and strip it and print it
    //
    if (Buffer[Index] == NARROW_CHAR) {
      //
      // Preserve bits 0 - 6 and zero out the rest
      //
      Out->Mode->Attribute = Out->Mode->Attribute & 0x7f;
      Out->SetAttribute (Out, Out->Mode->Attribute);
    } else {
      //
      // Must be wide, set bit 7 ON
      //
      Out->Mode->Attribute = Out->Mode->Attribute | EFI_WIDE_ATTRIBUTE;
      Out->SetAttribute (Out, Out->Mode->Attribute);
    }

    Index++;

  } while (Buffer[Index] != 0);

  //
  // We hit the end of the string - print it
  //
  Out->OutputString (Out, &BackupBuffer[PreviousIndex]);
  Count += StrLen (&BackupBuffer[PreviousIndex]);

  FreePool (Buffer);
  FreePool (BackupBuffer);
  return Count;
}


/**
  Prints a formatted unicode string to the default console.

  @param  Fmt        Format string
  @param  ...        Variable argument list for format string.

  @return Length of string printed to the console.

**/
UINTN
EFIAPI
ConsolePrint (
  IN CHAR16   *Fmt,
  ...
  )
{
  VA_LIST Args;

  VA_START (Args, Fmt);
  return PrintInternal ((UINTN) -1, (UINTN) -1, gST->ConOut, Fmt, Args);
}


/**
  Prints a unicode string to the default console,
  using L"%s" format.

  @param  String     String pointer.

  @return Length of string printed to the console

**/
UINTN
PrintString (
  IN CHAR16       *String
  )
{
  return ConsolePrint (L"%s", String);
}


/**
  Prints a chracter to the default console,
  using L"%c" format.

  @param  Character  Character to print.

  @return Length of string printed to the console.

**/
UINTN
PrintChar (
  CHAR16       Character
  )
{
  return ConsolePrint (L"%c", Character);
}


/**
  Prints a formatted unicode string to the default console, at
  the supplied cursor position.

  @param  Column     The cursor position to print the string at.
  @param  Row        The cursor position to print the string at.
  @param  Fmt        Format string.
  @param  ...        Variable argument list for format string.

  @return Length of string printed to the console

**/
UINTN
EFIAPI
PrintAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16    *Fmt,
  ...
  )
{
  VA_LIST Args;

  VA_START (Args, Fmt);
  return PrintInternal (Column, Row, gST->ConOut, Fmt, Args);
}


/**
  Prints a unicode string to the default console, at
  the supplied cursor position, using L"%s" format.

  @param  Column     The cursor position to print the string at.
  @param  Row        The cursor position to print the string at
  @param  String     String pointer.

  @return Length of string printed to the console

**/
UINTN
PrintStringAt (
  IN UINTN     Column,
  IN UINTN     Row,
  IN CHAR16    *String
  )
{
  return PrintAt (Column, Row, L"%s", String);
}


/**
  Prints a chracter to the default console, at
  the supplied cursor position, using L"%c" format.

  @param  Column     The cursor position to print the string at.
  @param  Row        The cursor position to print the string at.
  @param  Character  Character to print.

  @return Length of string printed to the console.

**/
UINTN
PrintCharAt (
  IN UINTN     Column,
  IN UINTN     Row,
  CHAR16       Character
  )
{
  return PrintAt (Column, Row, L"%c", Character);
}
