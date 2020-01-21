/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y

 Ver       Date         Who          Change
 --------  ----------   --------     ----------------------------------------------------
 1.00      2016.07.20   Stan         Init version.
*/

#include "CompalEcFlash.h"


SHELL_PARAM_ITEM ParamList[] = {
  {L"-f", TypePosition},
  {L"-a", TypePosition},
  {L"-mc", TypeStart},
  {L"-?", TypeFlag},
  {L"-t", TypeFlag},
  {L"-h", TypeFlag},
  {L"-i", TypeFlag},
  {L"-eccw", TypeStart},
  {L"-eccr", TypeStart},
  {L"-dump", TypeStart},
  { NULL, TypeMax}
};


VOID
LeftBundary (
  VOID
  )
{
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    gST->ConOut->OutputString (gST->ConOut,  L"                              ");
    gST->ConOut->OutputString (gST->ConOut,  L"                              ");
}


VOID
CompalFlashMessage (
  IN  OUT    UINT8  *OnboardEcVersion,
  IN  OUT    CHAR8  *RomFileEcVersion
  )
{
    gST->ConOut->ClearScreen  (gST->ConOut);
    gST->ConOut->OutputString (gST->ConOut,  L"\n\n\n\n\n\n\n\n\n\n");
    LeftBundary();
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    gST->ConOut->OutputString (gST->ConOut,  L"                                                \r\n");
    LeftBundary();
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    gST->ConOut->OutputString (gST->ConOut,  L"  PLEASE DO NOT PLUG OUT YOUR BATTERY AND AC.   \r\n");
    LeftBundary();
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_RED);
    gST->ConOut->OutputString (gST->ConOut,  L"                                                \r\n");
    gST->ConOut->OutputString (gST->ConOut,  L"\r\n");
    LeftBundary();
    gST->ConOut->OutputString (gST->ConOut,  L"================ CompalEcFlash =================\r\n");
    gST->ConOut->OutputString (gST->ConOut,  L"\r\n");
    LeftBundary();
    Print(L"  Project    Name    : ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW);
    Print(L"%c%c%c%c%c\r\n", RomFileEcVersion[5], RomFileEcVersion[6], RomFileEcVersion[7], RomFileEcVersion[8], RomFileEcVersion[9]);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    LeftBundary();
    Print(L"  Current Ec Version : ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
    Print(L"V%02x.%02x\r\n", OnboardEcVersion[0], OnboardEcVersion[1]);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    LeftBundary();
    Print(L"  New     Ec Version : ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
    Print(L"V%02x.%02x\r\n", RomFileEcVersion[0], RomFileEcVersion[1]);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    gST->ConOut->OutputString (gST->ConOut,  L"\r\n");
    LeftBundary();
    gST->ConOut->OutputString (gST->ConOut,  L"================================================\r\n");
    LeftBundary();
    Print(L"             CompalEcFlash %s Created by TDSW\r\n\r\n", FLASH_TOOL_VERSION);
}

EFI_STATUS
CompalEcFlash (
  IN        CHAR8        *EcRomBuffer,
  IN CONST  UINT8        ErasePolicy
  )
{
  EFI_STATUS        Status;
  UINT8             *FlashingDataBuffer;
  UINT8             OnboardEcVersion[4] = {0};
  CHAR8             *RomFileEcVersion;
  UINT8             EcChipId;

  //
  // Detect EC chip id before flashing ec bios
  //
  EcChipId = CompalEcChipDetect();

  //
  // get EcVersion from bios shadow memory.
  //
  RomFileEcVersion = (UINT8 *)(UINTN)EcRomBuffer + EC_VER_OFFSET;

  //
  // get ec data buffer and block size from rom file.
  //
  FlashingDataBuffer = (UINT8 *)(UINTN)EcRomBuffer;

  //
  // Identify Ec rom from input buffer before flashng ec bios.
  //
  Status = CompalEcRomIdentify(OnboardEcVersion, RomFileEcVersion, EcRomBuffer, EcChipId);

  if (Status == EFI_SUCCESS) {

    //
    // Enabled the EC index io
    //
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, 0xEA);

    //
    // display message before flashing ec
    //
    CompalFlashMessage (OnboardEcVersion, RomFileEcVersion);

    CompalECIdle(TRUE);

    //
    // According erase policy to flash ec bios.
    //
    CompalSpiEcFlash(FlashingDataBuffer, EC_SIZE_TO_FLASH, 0, ErasePolicy);

    CompalECIdle(FALSE);

    LeftBundary();
    Print(L"Flash ec completed, system will be cold boot.\r\n");

    //
    // Disabled the EC index io
    //
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, 0xEB);

    //
    // Inform ec the flash process was completed.
    //
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_SYSTEM_NOTIFI_CMD, EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_FLASH_COMPLETE);

    //
    // After flashing EC, system must be global reset.
    //
    IoWrite8(0xcf9, 0x0e);
  }

  return Status;
}


EFI_STATUS
CompalCommandParse (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                         Status;
  LIST_ENTRY                         *ParamPackage;
  EFI_HII_HANDLE                     mHiiHandle;
  CHAR8                              *EcRomBuffer;
  UINT8                              AcPresent;
  UINT8                              Command;
  UINT8                              Data;
  UINT8                              EcBuffer[8] = {0};
  UINTN                              i;
  UINT8                              EcChipId;
  UINT8                              ErasePolicy = 0;

  //
  // Detected EC Chip before parsing Parameter
  //
  EcChipId = CompalEcChipDetect();

  if(EcChipId != EC_CHIP) {
    Print (L"ERROR : Don't support this chip.\r\n");
    return EFI_UNSUPPORTED;
  }

  Status = gBS->AllocatePool (EfiBootServicesData, EC_SIZE_TO_FLASH, (VOID **) &EcRomBuffer);
  if (EFI_ERROR (Status)) {
    Print (L"ERROR : Can't allocate memory for Rom file.\r\n");
    return Status;
  }

  ZeroMem (EcRomBuffer, EC_SIZE_TO_FLASH);

  //
  // Register our string package with HII and return the handle to it.
  //
  mHiiHandle = HiiAddPackages (&gEfiCallerIdGuid, ImageHandle, CompalEcFlashStrings, NULL);
  ASSERT (mHiiHandle != NULL);

  Status = ShellCommandLineParseEx (ParamList, &ParamPackage, NULL, FALSE, FALSE);
  if (EFI_ERROR (Status)) {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_INVALID_INPUT), mHiiHandle);
    goto Done;
  }

  if (ShellCommandLineGetFlag (ParamPackage, L"-?") || \
      ShellCommandLineGetFlag (ParamPackage, L"-h")) {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_COMPAL_ECFLASH_HELP), mHiiHandle);
    Print(L"  ================================= CompalEcFlash %s Created by TDSW\r\n", FLASH_TOOL_VERSION);
    goto Done;

  } else if ((ShellCommandLineGetFlag(ParamPackage, L"-f") || ShellCommandLineGetFlag(ParamPackage, L"-a")) \
           && ShellCommandLineGetCount(ParamPackage) == 2) {

    AcPresent = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_EcNameSpace, SYS_STATUS) >> 7;
    if (AcPresent) {                // 0=OffLine, 1=OnLine

      Status = ReadRomFile(EcRomBuffer, ParamPackage);
      if (Status != EFI_SUCCESS) {
        goto Done;
      }

      if (!ShellCommandLineGetFlag (ParamPackage, L"-mc"))
        Status = CompalEcProjectCompare(EcRomBuffer);

      if (Status != EFI_SUCCESS) {
        goto Done;
      }

      if (ShellCommandLineGetFlag(ParamPackage, L"-f")) {
          ErasePolicy = FLASH_SPEED_UP;
      } else if (ShellCommandLineGetFlag(ParamPackage, L"-a")) {
          ErasePolicy = ERASE_ALL_BLOCK;
      }

      Status = CompalEcFlash (EcRomBuffer, ErasePolicy);
      if (Status == EFI_UNSUPPORTED)
        Print(L"ERROR : Can't read ec version.\r\n");

    } else {
        Print(L"ERROR : Please plug in your AC.\r\n");
    }
    goto Done;

  } else if (ShellCommandLineGetFlag (ParamPackage, L"-i") && ShellCommandLineGetCount(ParamPackage) == 1) {

    Status = CompalEcInformation();
    if (Status != EFI_SUCCESS) {
      Print(L"ERROR : Can't get EC information.\r\n");
    }
    goto Done;

  } else if (ShellCommandLineGetFlag (ParamPackage, L"-t") && ShellCommandLineGetCount(ParamPackage) == 1) {

    CompalEcAutoFlashToggle();
    goto Done;

  } else if (ShellCommandLineGetFlag (ParamPackage, L"-eccw")) {

    if (ShellCommandLineGetCount(ParamPackage) == 0x03) {
      Command = (UINT8)ShellHexStrToUintn(ShellCommandLineGetRawValue(ParamPackage, 1));
      Data    = (UINT8)ShellHexStrToUintn(ShellCommandLineGetRawValue(ParamPackage, 2));
      Print(L"\nWrite C%02X:D%02X to EC\r\n",Command, Data);
      CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, Command, Data);
    } else {
      Print(L"\nERROR : Parameter isn't correct.\r\n");
    }
    goto Done;

  } else if (ShellCommandLineGetFlag (ParamPackage, L"-eccr")) {

    if (ShellCommandLineGetCount(ParamPackage) == 0x03) {
      Command = (UINT8)ShellHexStrToUintn(ShellCommandLineGetRawValue(ParamPackage, 1));
      Data    = (UINT8)ShellHexStrToUintn(ShellCommandLineGetRawValue(ParamPackage, 2));
      Print(L"\nRead C%02X:D%02X from EC\r\n",Command, Data);
      EcBuffer[0] = Data;
      Status = CompalECReadCmd (EC_MEMORY_MAPPING_PORT, Command, 0x10, 0x10, EcBuffer);
      Print(L"HEX   : ");
      for (i = 0; i < 8; i++)
        Print(L"%2X ", EcBuffer[i]);
      Print(L"\r\n");
      Print(L"ASCII : ");
      for (i = 0; i < 8; i++)
        Print(L"%c", EcBuffer[i]);
      Print(L"\r\n");
    } else {
      Print(L"\nERROR : Parameter isn't correct.\r\n");
    }
    goto Done;

  } else if (ShellCommandLineGetFlag(ParamPackage, L"-dump") && ShellCommandLineGetCount(ParamPackage) == 2) {

    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, 0xEA);
    CompalECIdle(TRUE);

    Status = DumpdRomFile(EcRomBuffer, ParamPackage);

    CompalECIdle(FALSE);
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, 0xEB);
    goto Done;

  } else {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_COMPAL_ECFLASH_HELP), mHiiHandle);
    Print(L"  ================================= CompalEcFlash %s Created by TDSW\r\n", FLASH_TOOL_VERSION);
    goto Done;
  }

Done:
  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (ParamPackage);
  HiiRemovePackages (mHiiHandle);
  gBS->FreePool(EcRomBuffer);

  return Status;
}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
CompalEcFlashEntryPoint (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS      Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "[Compal][COM] CompalEcFlashEntryPoint() - Entry\n"));

  Status = CompalCommandParse(&ImageHandle, SystemTable);

  DEBUG ((DEBUG_INFO, "[Compal][COM] CompalEcFlashEntryPoint() - Exit\n"));

  return Status;
}
