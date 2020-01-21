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

#include "CompalEcFlashToolLib.h"

VOID
CompalEcAutoFlashStatus (
  VOID
)
{
  if ((CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & BIT0) == TRUE && \
      ((CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & BIT1) >> BIT1) == FALSE) {
    Print(L"CompalEcAutoFlash Status : Disabled\n");
  } else {
    Print(L"CompalEcAutoFlash Status : Enabled\n");
  }
}

VOID
CompalEcAutoFlashToggle (
  VOID
  )
{
  if ((CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & BIT0) == TRUE && \
      ((CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & BIT1) >> BIT1) == FALSE) {
    CompalECEepromWriteByte(EC_AUTO_FLASH_OFFSET, (CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & ~BIT0));
    Print(L"\n CompalEcAutoFlash Status : Enabled\n");
  } else {
    CompalECEepromWriteByte(EC_AUTO_FLASH_OFFSET, (CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) | BIT0));
    CompalECEepromWriteByte(EC_AUTO_FLASH_OFFSET, (CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & ~BIT1));
    Print(L"\n CompalEcAutoFlash Status : Disabled\n");
  }
}

EFI_STATUS
CompalEcInformation (
  VOID
  )
{
  EFI_STATUS                         Status;

  Status = CompalEcReadProject();
  if (Status != EFI_SUCCESS) {
    Print(L"ERROR : Can't read Project Name from EC.\r\n");
    return Status;
  }

  Status = CompalEcReadVersion();
  if (Status != EFI_SUCCESS) {
    Print(L"ERROR : Can't read ec version.\r\n");
    return Status;
  }

  CompalEcAutoFlashStatus();

  switch (CompalEcChipDetect()) {
    case Ene90xx:
      Print(L"The EC chip is ENE.\r\n");
      break;

    case Nuvoton8xx:
      Print(L"The EC chip is NUVOTON.\r\n");
      break;

    case UnknownEc:
    default:
      Print(L"The EC chip is unknown.\r\n");
      break;
  }

  return Status;
}


UINT8
CompalEcChipDetect (
  VOID
  )
{
  CHAR8             KbcChipId;
  CHAR8             CmdSetVer;

  KbcChipId = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_GET_EC_INFORMATION, EC_CMD_GET_EC_INFORMATION_BOOTCODE_SIZE);
  CmdSetVer = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, 0x6C);

  if (CmdSetVer < 4 ? (KbcChipId & 0x0f):(KbcChipId >= 0x00 && KbcChipId <=0x0f)) {
    return Ene90xx;
  } else if(CmdSetVer < 4 ? (KbcChipId & 0x40):(KbcChipId >= 0x40 && KbcChipId <=0x4f)) {
    return Nuvoton8xx;
  } else {
    return UnknownEc;
  }
}


EFI_STATUS
CompalEcProjectCompare (
  IN CHAR8          *EcRomBuffer
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  CHAR8             ProjectCode[5];
  UINT8             i, j;
  CHAR8             *RomFileEcVersion;
  UINT8             index = 0;

  //
  // Detect EC chip id before flashing ec bios
  //
  RomFileEcVersion = (UINT8 *)(UINTN)EcRomBuffer + EC_VER_OFFSET;

  ProjectCode[0] = EC_CMD_GET_EC_INFORMATION_PROJECT_NAME;
  Status = CompalECReadCmd (EC_MEMORY_MAPPING_PORT, EC_CMD_GET_EC_INFORMATION, EC_CMD_GET_EC_INFORMATION_PROJECT_NAME_CMD_DATA_LEN, 0x05, ProjectCode);

  if (Status == EFI_SUCCESS) {
    for ( i = 0; i < 4; i++) {
      for (j = 0; j < 5; j++) {
        if(RomFileEcVersion[j + 5 + i * 0x10] != ProjectCode[j]) {
          Status = EFI_ABORTED;
          index = 0;
          break;
        } else {
          if (++index == 0x05) {
            Status = EFI_SUCCESS;
            goto Done;
          }
        }
      }
    }
  }

  Print(L"ERROR : Project Name is different with Rom file.\r\n");

Done:
  return Status;
}

EFI_STATUS
CompalEcReadProject (
  VOID
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  CHAR8             ProjectCode[5];
  UINT8             i;

  ProjectCode[0] = EC_CMD_GET_EC_INFORMATION_PROJECT_NAME;
  Status = CompalECReadCmd (EC_MEMORY_MAPPING_PORT, EC_CMD_GET_EC_INFORMATION, EC_CMD_GET_EC_INFORMATION_PROJECT_NAME_CMD_DATA_LEN, 0x05, ProjectCode);

  Print(L"Project Name : ");
  if (Status == EFI_SUCCESS) {
    for (i = 0; i < 5; i ++)
      Print(L"%c", ProjectCode[i]);
  }
  Print(L"\r\n");
  return Status;
}

EFI_STATUS
CompalEcReadVersion (
  VOID
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  UINT8             OnboardEcVersion[4] = {0};

  //
  // get Current EcVersion by Ec CMD
  //
  Status = CompalECReadKBCVersion(OnboardEcVersion);
  if (Status == EFI_SUCCESS) {
    Print(L"Current Ec Version : V");
    Print(L"%02X.",  OnboardEcVersion[0]);
    Print(L"%02X" ,  OnboardEcVersion[1]);
    Print(L"%c\r\n", OnboardEcVersion[2]);
  }
  return Status;
}

EFI_STATUS
CompalEcRomIdentify (
  IN  OUT    UINT8  *OnboardEcVersion,
  IN  OUT    CHAR8  *RomFileEcVersion,
  IN         CHAR8  *EcRomBuffer,
  IN         UINT8  EcChipId
  )
{
  EFI_STATUS        Status = EFI_UNSUPPORTED;
  UINT16            RomEcVer = 0;

  //
  // get Current EcVersion by Ec CMD
  //
  Status = CompalECReadKBCVersion(OnboardEcVersion);

  RomEcVer += RomFileEcVersion[0];
  RomEcVer += RomFileEcVersion[1] << 8;

  if (Status == EFI_SUCCESS && RomEcVer != 0xFFFF) {
      Status = EFI_SUCCESS;
  }

  if (EcChipId == Ene90xx) {
    if(EcRomBuffer[8] != 'E' || EcRomBuffer[9] != 'N' || EcRomBuffer[10] != 'E') {
      Print(L"ERROR : The Rom File isn't for ENE ec used.\r\n");
      Status = EFI_LOAD_ERROR;
    }
  } else if (EcChipId == Nuvoton8xx) {
    if(EcRomBuffer[8] == 'E' && EcRomBuffer[9] == 'N' && EcRomBuffer[10] == 'E') {
      Print(L"ERROR : The Rom File is for ENE ec used.\r\n");
      Status = EFI_LOAD_ERROR;
    }
  }

  return Status;
}

EFI_STATUS
ReadRomFile (
    IN OUT CHAR8          *EcRomBuffer,
    IN LIST_ENTRY         *ParamPackage
  )
{
  EFI_STATUS                         Status;
  CONST CHAR16                       *FileName;
  EFI_FILE_HANDLE                    FileHandle;
  EFI_FILE_INFO                      *FileInfo;
  UINTN                              SizeToFlash;

  SizeToFlash = (UINTN)EC_SIZE_TO_FLASH;

  //
  // Get ec rom file from file path
  //
  FileName = ShellCommandLineGetRawValue(ParamPackage, 1);

  Status = ShellOpenFileByName((CHAR16 *)FileName,
                               &FileHandle,
                               EFI_FILE_MODE_READ,
                               0
                               );

  if (EFI_ERROR (Status)) {
    Print (L"ERROR : Open File Failed\r\n");
    return Status;
  }

  FileInfo = ShellGetFileInfo (FileHandle);

  if (FileInfo == NULL) {
    Print (L"ERROR : Can not get file info from input file\r\n");
    return EFI_LOAD_ERROR;
  }

  if (FileInfo->FileSize != SizeToFlash) {
    Print (L"ERROR : The EC rom size isn't correct.\r\n");
    return EFI_BAD_BUFFER_SIZE;
  }

  Status = ShellReadFile (FileHandle, &SizeToFlash, (VOID *)EcRomBuffer);
  if (EFI_ERROR (Status)) {
    Print (L"ERROR: ReadFile from source device failed.\r\n");
  }

  ShellCloseFile (&FileHandle);

  return Status;
}


EFI_STATUS
DumpdRomFile (
    IN OUT CHAR8          *EcRomBuffer,
    IN LIST_ENTRY         *ParamPackage
  )
{
  EFI_STATUS                         Status;
  CONST CHAR16                       *FileName;
  EFI_FILE_HANDLE                    FileHandle;
  EFI_FILE_INFO                      *FileInfo;
  UINTN                              SizeToFlash;

  SizeToFlash = (UINTN)EC_SIZE_TO_FLASH;

  //
  // Get ec rom file from file path
  //
  FileName = ShellCommandLineGetRawValue(ParamPackage, 1);

  Status = ShellOpenFileByName((CHAR16 *)FileName,
                               &FileHandle,
                               EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
                               0
                               );

  if (EFI_ERROR (Status)) {
    Print (L"ERROR : Create File Failed\r\n");
    return Status;
  }

  FileInfo = ShellGetFileInfo (FileHandle);

  if (FileInfo == NULL) {
    Print (L"ERROR : Can not get file info from input file\r\n");
    return EFI_LOAD_ERROR;
  }

  //
  // write spi data to EcRomBuffer.
  //
  CompalEcRomDump(EcRomBuffer, SizeToFlash);

  Status = ShellWriteFile (FileHandle, &SizeToFlash, (VOID *)EcRomBuffer);

  if (EFI_ERROR (Status)) {
    Print(L"ERROR : WriteFile from source device failed.\r\n");
  }

  ShellCloseFile (&FileHandle);

  return Status;
}
