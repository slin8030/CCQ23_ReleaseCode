//;******************************************************************************
//;* Copyright (c) 1983-2010, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************

#include <Library/BaseLib.h>
#include <CompalEclib.h>
#include "CompalFlash.h"

UINTN               OemBatteryGasGauge = 0;
UINTN               OemBIOSRegLength = 0;
UINTN               ParamterIndex = 0;
UINTN               ECIdleCount = 0;
BOOLEAN             OemIsFlashECFlag = FALSE;
UINT16              AfterFlashSysBehavior = 0x02;

EFI_STATUS
OemCheckBeforeFlash (
    BEFORE_FLASH_INPUT_BUFFER             *PlatformInfoPtr,
    UINT16                                 UserInputStatus,
    BEFORE_FLASH_OUTPUT_BUFFER            *OemPlatformOutputInfoPtr
)
{
    UINT8                                  NumOfErrorCode;

    ParamterIndex = 0;
    OemPlatformOutputInfoPtr->Flash = ENABLED;
    OemPlatformOutputInfoPtr->NumOfErrorMsg = NO_ERROR;

    //
    // Check end user's input status
    //

    AfterFlashSysBehavior = OemGetBehaviorAfterFlash (UserInputStatus);

    if (UserInputStatus & CPVER) {
        OemPlatformOutputInfoPtr->Flash = DISABLED;
        OemPlatformOutputInfoPtr->NumOfErrorMsg = CPVER_ERROR;
        return EFI_SUCCESS;
    }

    if (!(UserInputStatus & PTEN)) {
        OemPlatformOutputInfoPtr->Flash = ENABLED;
        OemPlatformOutputInfoPtr->NumOfErrorMsg = NO_ERROR;
        OemPlatformOutputInfoPtr->SysBehavior = AfterFlashSysBehavior;
        return EFI_SUCCESS;
    }

    if ((UserInputStatus & ACEN))  {
        NumOfErrorCode = Func_Detect_AC (PlatformInfoPtr);
        if (!NumOfErrorCode) {
            OemPlatformOutputInfoPtr->SysBehavior = AfterFlashSysBehavior;
        } else {
            OemPlatformOutputInfoPtr->Flash = DISABLED;
            OemPlatformOutputInfoPtr->NumOfErrorMsg = NumOfErrorCode;
            return EFI_SUCCESS;
        }
    }

    if ((UserInputStatus & DCEN)) {
        NumOfErrorCode = Func_Detect_Battery (PlatformInfoPtr);
        if (!NumOfErrorCode) {
            OemPlatformOutputInfoPtr->SysBehavior = AfterFlashSysBehavior;
        } else {
            OemPlatformOutputInfoPtr->Flash = DISABLED;
            OemPlatformOutputInfoPtr->NumOfErrorMsg = NumOfErrorCode;
            return EFI_SUCCESS;
        }
    }

    if ((UserInputStatus & RESSEN)) {
        NumOfErrorCode = Func_Detect_BIOS_Regressive (PlatformInfoPtr);
        if (!NumOfErrorCode) {
            OemPlatformOutputInfoPtr->SysBehavior = AfterFlashSysBehavior;
        } else {
            OemPlatformOutputInfoPtr->Flash = DISABLED;
            OemPlatformOutputInfoPtr->NumOfErrorMsg = NumOfErrorCode;
            return EFI_SUCCESS;
        }
    }
    if ((UserInputStatus & PJMDEN)) {
        NumOfErrorCode = Func_Detect_ModelString (PlatformInfoPtr);
        if (!NumOfErrorCode) {
            OemPlatformOutputInfoPtr->SysBehavior = AfterFlashSysBehavior;
        } else {
            OemPlatformOutputInfoPtr->Flash = DISABLED;
            OemPlatformOutputInfoPtr->NumOfErrorMsg = NumOfErrorCode;
            return EFI_SUCCESS;
        }
    }

    return EFI_SUCCESS;
}

UINT16
OemCheckParamterInfo(
    CHAR8                            *OemPassInfoPtr
)
{
    UINTN                                  Index;
    INTN                                   Index2;
    CHAR16                                 DateBuffer[10];
    UINT16                                 UserInputStatus = 0x00;

    UserInputStatus |= (FHST | CPVER);

//
// Get end user's information form AP
//

    for (Index = 0, Index2 = 0; Index < OEM_PASS_INFO_SIZE; Index++, Index2++) {

        if (OemPassInfoPtr[Index] == FIELD_TAG) {
            DateBuffer[Index2] = OemPassInfoPtr[Index];
            UserInputStatus = OemCheckFlashStates (DateBuffer, UserInputStatus);

            Index2 = -1;
            continue;
        }

        if (OemPassInfoPtr[Index] == END_TAG) {
            if ((OemPassInfoPtr[Index - 2]  == ']') && (ParamterIndex == 1)) {
                UserInputStatus |= (FHST | PTEN);
            }
            break;
        }

        DateBuffer[Index2] = OemPassInfoPtr[Index];
    }

    return UserInputStatus;
}

UINT16
OemCheckFlashStates(
    CHAR16         *PassDateBuffer,
    UINT16          UserInputStatus
)
{
    CHAR16          CPVer[5];
    UINT8           OemCpVer = 0;
    UINTN           Index, Index2;
    UINTN           ConvertedStrLen;
    UINTN           OemCPVersion;

    OemCPVersion = COMMON_FLASH_VERSION;
    ParamterIndex++;

    if (StrCmp (PassDateBuffer, L"PTEN") == 0) {
        UserInputStatus |= PTEN ;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"ACEN") == 0) {
        UserInputStatus |= ACEN;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"DCEN") == 0) {
        UserInputStatus |= DCEN;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"RESSEN") == 0) {
        UserInputStatus |= RESSEN;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"PJMDEN") == 0) {
        UserInputStatus |= PJMDEN;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"FHOS") == 0) {
        UserInputStatus |= FHOS;
        UserInputStatus &= ~FHST;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"FHRST") == 0) {
        UserInputStatus |= FHRST;
        UserInputStatus &= ~FHST;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"FHST") == 0) {
        UserInputStatus |= FHST;
        return UserInputStatus;
    }

    if (StrCmp (PassDateBuffer, L"PTDIS") == 0) {
        UserInputStatus &= PTDIS;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"ACDIS") == 0) {
        UserInputStatus &=ACDIS;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"DCDIS") == 0) {
        UserInputStatus &= DCDIS;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"RESSDIS") == 0) {
        UserInputStatus &= RESSDIS;
        return UserInputStatus;
    }
    if (StrCmp (PassDateBuffer, L"PJMDDIS") == 0) {
        UserInputStatus &= PJMDDIS;
        return UserInputStatus;
    }

    if (PassDateBuffer[6] == '[') {
        for (Index = 7, Index2 = 0; PassDateBuffer[Index] != ']'; Index++, Index2++ ) {
            CPVer[Index2] = PassDateBuffer[Index];
        }
        HexStringToBuf (&OemCpVer, &Index2, CPVer, &ConvertedStrLen);
        if ((OemCpVer <= OemCPVersion) && (OemCpVer > 0)) {
            UserInputStatus &= ~CPVER;
            return UserInputStatus;
        }
    }

    return UserInputStatus;
}

VOID
OemFlashAllowStart(
    VOID
)
{
    if (ECIdleCount >= 1) {
        return;
    }

    CompalEcSmmIdle (TRUE);
    ECIdleCount++;

    return;
}

VOID
OemFlashCompletely(
    VOID
)
{
    CompalEcSmmIdle (FALSE);

    ECIdleCount = 0;

    switch (AfterFlashSysBehavior) {

    case SYSYEM_BACK_TO_OS:
        CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_SYSTEM_NOTIFI_CMD, EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_DONOTHING);
        break;
    case SYSTEM_DIRECTLY_SHUTDOWN:
        CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_SYSTEM_NOTIFI_CMD, EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_SHUTDOWN);
        break;
    case SYSTEM_DIRECTLY_REBOOT:
        CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_SYSTEM_NOTIFI_CMD, EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_REBOOT);
        break;
    }

    return;
}

UINT8
Func_Detect_AC (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
)
{
    UINT8             ACStatus = NO_ERROR;

    if (!PlatformInfo->ACPresent) {
        ACStatus = AC_ERROR;
    }
    return  ACStatus;

}

UINT8
Func_Detect_Battery (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
)
{
    UINT8             BatteryStates = NO_ERROR;

    if (!PlatformInfo->DCPresent) {
        BatteryStates = DC_ERROR;
        return BatteryStates;
    }
    if (PlatformInfo->BatteryGasGauge < OemBatteryGasGauge ) {
        BatteryStates = GAS_GAUGE_ERROR;
        return BatteryStates;
    }

    return  BatteryStates;
}

UINT8
Func_Detect_BIOS_Regressive (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
)
{
    UINT8       ProjectRegressiveStates = NO_ERROR;
    UINTN       NewProjectVersion = 0;
    UINTN       OldProjectVersion = 0;
    UINT8       Data;
    UINTN       Index, Index2;

    for (Index = OemBIOSRegLength, Index2 = 0; Index > 0; Index--) {
        Data = PlatformInfo->OldProjectVersion[Index - 1];

        if ((0x30 <= Data) && (Data <= 0x39)) {
            Data = Data - 0x30;
        }
        if ((0x41 <= Data) && (Data <= 0x5a)) {
            Data = Data - 0x37;
        }
        if ((0x61 <= Data) && (Data <= 0x7a)) {
            Data = Data - 0x57;
        }
        OldProjectVersion += Data * Pow (36, Index2);

        Data = PlatformInfo->NewProjectVersion[Index - 1];
        if ((0x30 <= Data) && (Data <= 0x39)) {
            Data = Data - 0x30;
        }
        if ((0x41 <= Data) && (Data <= 0x5a)) {
            Data = Data - 0x37;
        }
        if ((0x61 <= Data) && (Data <= 0x7a)) {
            Data = Data - 0x57;
        }

        NewProjectVersion += Data * Pow (36, Index2);
        Index2++;
    }


    if (OldProjectVersion >= NewProjectVersion) {
        ProjectRegressiveStates = RESS_ERROR;
    }

    return ProjectRegressiveStates;
}

UINT8
Func_Detect_ModelString (
    BEFORE_FLASH_INPUT_BUFFER   *PlatformInfo
)
{
    UINT8        ProjectNameStates = NO_ERROR;
    CHAR16       NewProjectName[PROJECT_COUNT][PROJECT_NAME_SIZE];
    CHAR16       OldProjectName[PROJECT_NAME_SIZE];
    UINTN        Index, Index2, Index3;

    for (Index = 0; PlatformInfo->OldProjectName[Index] != FIELD_TAG ; Index++) {
        OldProjectName[Index] = PlatformInfo->OldProjectName[Index];
    }
    OldProjectName[Index] = L'\0';
    for (Index = 0, Index2 = 0, Index3 = 0; PlatformInfo->NewProjectName[Index] != FIELD_TAG ; Index++, Index3++) {
        if (PlatformInfo->NewProjectName[Index] == ',') {
            NewProjectName[Index2][Index3] = L'\0';
            Index2++;
            Index3 = (UINTN)-1;
            continue;
        }
        NewProjectName[Index2][Index3] = PlatformInfo->NewProjectName[Index];
    }
    NewProjectName[Index2][Index3] = L'\0';



    for (Index = 0; Index <= Index2 ; Index++) {
        if (StrCmp (OldProjectName, NewProjectName[Index]) != 0) {
            ProjectNameStates = PJMD_ERROR;
        } else {
            ProjectNameStates = NO_ERROR;
            break;
        }
    }

    return ProjectNameStates;
}

UINT16
OemGetBehaviorAfterFlash (
    UINT16   BehaviorAfterFlash
)
{
    UINT16             BehaviorStates = FHRST;

    if (BehaviorAfterFlash & FHOS) {
        BehaviorStates = SYSYEM_BACK_TO_OS;
    }
    if (BehaviorAfterFlash & FHST) {
        BehaviorStates = SYSTEM_DIRECTLY_SHUTDOWN;
    }
    if (BehaviorAfterFlash & FHRST) {
        BehaviorStates = SYSTEM_DIRECTLY_REBOOT;
    }

    return BehaviorStates;
}

VOID
GetOemSetting (
    UINTN       BatteryGasGauge,
    UINTN       BIOSRegLength
)
{
    OemBatteryGasGauge = BatteryGasGauge;
    OemBIOSRegLength   = BIOSRegLength;
    return ;
}

UINTN
Pow(
    UINTN Value,
    UINTN Index
)
{
    if (Index == 0) {
        return 1;
    }

    return Value * Pow(Value, Index - 1);
}

EFI_STATUS
OemPassECFlash (
    BOOLEAN     OemIsEcFlash
)
{
    OemIsFlashECFlag = OemIsEcFlash;
    return EFI_SUCCESS;
}

/**
 Determines if a Unicode character is a hexadecimal digit.
 The test is case insensitive.

 @param[out] Digit              Pointer to byte that receives the value of the hex character.
 @param[in]  Char               Unicode character to test.

 @retval TRUE                   If the character is a hexadecimal digit.
 @retval FALSE                  Otherwise.
**/
BOOLEAN
IsHexDigit (
  OUT UINT8      *Digit,
  IN  CHAR16      Char
  )
{
  if ((Char >= L'0') && (Char <= L'9')) {
    *Digit = (UINT8) (Char - L'0');
    return TRUE;
  }

  if ((Char >= L'A') && (Char <= L'F')) {
    *Digit = (UINT8) (Char - L'A' + 0x0A);
    return TRUE;
  }

  if ((Char >= L'a') && (Char <= L'f')) {
    *Digit = (UINT8) (Char - L'a' + 0x0A);
    return TRUE;
  }

  return FALSE;
}
/**
 Converts Unicode string to binary buffer.
 The conversion may be partial.
 The first character in the string that is not hex digit stops the conversion.
 At a minimum, any blob of data could be represented as a hex string.

 @param[in, out] Buf               Pointer to buffer that receives the data.
 @param[in, out] Len               Length in bytes of the buffer to hold converted data.
                                   If routine return with EFI_SUCCESS, containing length of converted data.
                                   If routine return with EFI_BUFFER_TOO_SMALL, containg length of buffer desired.
 @param[in]      Str               String to be converted from.
 @param[out]     ConvertedStrLen   Length of the Hex String consumed.

 @retval EFI_SUCCESS               Routine Success.
 @retval EFI_BUFFER_TOO_SMALL      The buffer is too small to hold converted data.
**/
EFI_STATUS
HexStringToBuf (
  IN OUT UINT8                     *Buf,
  IN OUT UINTN                    *Len,
  IN     CHAR16                    *Str,
  OUT    UINTN                     *ConvertedStrLen  OPTIONAL
  )
{
  UINTN       HexCnt;
  UINTN       Idx;
  UINTN       BufferLength;
  UINT8       Digit;
  UINT8       Byte;

  //
  // Find out how many hex characters the string has.
  //
  for (Idx = 0, HexCnt = 0; IsHexDigit (&Digit, Str[Idx]); Idx++, HexCnt++);

  if (HexCnt == 0) {
    *Len = 0;
    return EFI_SUCCESS;
  }
  //
  // Two Unicode characters make up 1 buffer byte. Round up.
  //
  BufferLength = (HexCnt + 1) / 2;

  //
  // Test if  buffer is passed enough.
  //
  if (BufferLength > (*Len)) {
    *Len = BufferLength;
    return EFI_BUFFER_TOO_SMALL;
  }

  *Len = BufferLength;

  for (Idx = 0; Idx < HexCnt; Idx++) {
    IsHexDigit (&Digit, Str[HexCnt - 1 - Idx]);

    //
    // For odd charaters, write the lower nibble for each buffer byte,
    // and for even characters, the upper nibble.
    //
    if ((Idx & 1) == 0) {
      Byte = Digit;
    } else {
      Byte = Buf[Idx / 2];
      Byte &= 0x0F;
      Byte = (UINT8)(Byte | (Digit << 4));
    }

    Buf[Idx / 2] = Byte;
  }

  if (ConvertedStrLen != NULL) {
    *ConvertedStrLen = HexCnt;
  }

  return EFI_SUCCESS;
}
