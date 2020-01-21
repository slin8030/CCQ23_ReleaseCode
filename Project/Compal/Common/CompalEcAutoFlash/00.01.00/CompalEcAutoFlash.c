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
 1.00      2016.06.08   Stan         Init version.
*/

#include "CompalEcAutoFlash.h"


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
  IN  OUT    CHAR8  *BiosRomEcVersion
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
    gST->ConOut->OutputString (gST->ConOut,  L"============== CompalEcAutoFlash ===============\r\n");
    gST->ConOut->OutputString (gST->ConOut,  L"\r\n");
    LeftBundary();
    Print(L"  Project    Name    : ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW);
    Print(L"%c%c%c%c%c\r\n", BiosRomEcVersion[5], BiosRomEcVersion[6], BiosRomEcVersion[7], BiosRomEcVersion[8], BiosRomEcVersion[9]);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    LeftBundary();
    Print(L"  Current Ec Version : ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
    Print(L"V%02x.%02x\r\n", OnboardEcVersion[0], OnboardEcVersion[1]);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    LeftBundary();
    Print(L"  New     Ec Version : ");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
    Print(L"V%02x.%02x\r\n", BiosRomEcVersion[0], BiosRomEcVersion[1]);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    gST->ConOut->OutputString (gST->ConOut,  L"\r\n");
    LeftBundary();
    gST->ConOut->OutputString (gST->ConOut,  L"================================================\r\n");
}

EFI_STATUS
CompalEcVersionCompare(
  IN  OUT    UINT8  *OnboardEcVersion,
  IN  OUT    CHAR8  *BiosRomEcVersion
  )
{
  EFI_STATUS        Status = EFI_UNSUPPORTED;
  UINT8             i;
  UINT16            MemEcVer = 0;

  //
  // get Current EcVersion by Ec CMD
  //
  Status = CompalECReadKBCVersion(OnboardEcVersion);

  MemEcVer += BiosRomEcVersion[0];
  MemEcVer += BiosRomEcVersion[1] << 8;

  if (Status == EFI_SUCCESS && MemEcVer != 0xFFFF) {
    DEBUG ((DEBUG_INFO, "[Compal][COM] Current : BiosRomEcVersion\n"));
    for (i = 0; i < 2; i++, *OnboardEcVersion++, *BiosRomEcVersion++) {
      DEBUG ((DEBUG_INFO, "%2d      : %2d \n", *OnboardEcVersion, *BiosRomEcVersion));
      if (*OnboardEcVersion < *BiosRomEcVersion) {
        Status = EFI_SUCCESS;
        break;
      } else {
        Status = EFI_UNSUPPORTED;
      }
    }
  }

  return Status;
}


EFI_STATUS
CompalEcAutoFlash (
  VOID
  )
{
  EFI_STATUS        Status;
  UINT8             *FlashingDataBuffer;
  UINT8             BlockCount;
  UINT8             OnboardEcVersion[4] = {0};
  CHAR8             *BiosRomEcVersion;
  UINT8             i;
  UINT8             ErasePolicy;

  //
  // Determinte erase polciy before flash.
  //
  ErasePolicy = ERASE_POLICY;

  //
  // get EcVersion from bios shadow memory.
  //
  BiosRomEcVersion = (UINT8 *)(UINTN)PcdGet32 (PcdCompalAutoLoadEcBase) + EC_VER_OFFSET;

  //
  // get ec data buffer and block size from bios shdaow memory.
  //
  FlashingDataBuffer = (UINT8 *)(UINTN)PcdGet32 (PcdCompalAutoLoadEcBase);
  BlockCount = (UINT8)(PcdGet32(PcdCompalAutoLoadEcSize) / EC_SIZE_TO_FLASH);

  //
  // Compare Ec version with bios rom before flashng ec bios.
  //
  Status = CompalEcVersionCompare(OnboardEcVersion, BiosRomEcVersion);

  //
  // According the eeprom value to enable or disable the CompalEcAutoFlash
  //
  if (Status == EFI_SUCCESS) {
    if ((CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & BIT0) == TRUE && \
        ((CompalECEepromReadByte(EC_AUTO_FLASH_OFFSET) & BIT1) >> BIT1) == FALSE) {
      Status = EFI_UNSUPPORTED;
    }
  }

  if (Status == EFI_SUCCESS) {

    //
    // Enabled the EC index io
    //
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, 0xEA);

    //
    // display message before flashing ec
    //
    CompalFlashMessage (OnboardEcVersion, BiosRomEcVersion);

    CompalECIdle(TRUE);
    
    for (i = 0; i < BlockCount; i++) {
      CompalSpiEcFlash(FlashingDataBuffer, EC_SIZE_TO_FLASH, i, ErasePolicy);
    }

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

    //
    // Waiting for system reboot.
    //
    while(1);

  }

  return Status;
}

EFI_STATUS
CompalEcAutoFlashEntryPoint (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
    EFI_STATUS      Status = EFI_SUCCESS;
    UINT8           AcPresent;

    DEBUG ((DEBUG_INFO, "[Compal][COM] CompalEcAutoFlashEntryPoint() - Entry\n"));

    if (EC_CHIP == UnknownEc)
      return EFI_UNSUPPORTED;

    AcPresent = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_EcNameSpace, SYS_STATUS) >> 7;

    if (AcPresent) {                // 0=OffLine, 1=OnLine
      //
      // Chceck whether we must be updated ec or not.
      //
      Status = CompalEcAutoFlash ();
    }

    DEBUG ((DEBUG_INFO, "[Compal][COM] CompalEcAutoFlashEntryPoint() - Exit\n"));

    return Status;
}
