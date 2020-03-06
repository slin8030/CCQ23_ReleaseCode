/*
 * (C) Copyright 2015 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2015 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y

*/

#include "UpdateSmbios.h"
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>
#include <Library/BaseMemoryLib.h>
#include <CompalEEPROMDxeProtocol.h>
#include <CompalEepromDefinition.h>
#include <T66CommonDefinition.h>

COMPAL_EEPROM_DXE_PROTOCOL        *mCompalEepromDxeProtocol;
EFI_SMBIOS_PROTOCOL               *mSmbios;

EFI_STATUS
EFIAPI
ReplaceSmbios (
  IN EFI_SMBIOS_PROTOCOL           *Smbios,
  IN COMPAL_EEPROM_DXE_PROTOCOL    *CompalEeprom
  );

/**
 Notify function to locate Smbios protocol.

 @param[in] Event     The Event this notify function registered to.
 @param[in] Context   Pointer to the context data registered to the Event.
**/
VOID
EFIAPI
SmBiosNotify (
    IN EFI_EVENT          Event,
    IN VOID              *Context
  )
{
  EFI_STATUS              Status;

  gBS->CloseEvent (Event);

  Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID **) &mSmbios
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  ReplaceSmbios (mSmbios, mCompalEepromDxeProtocol);
}


/**
 Notify function to locate CompalEEPROM protocol

 @param[in] Event     The Event this notify function registered to.
 @param[in] Context   Pointer to the context data registered to the Event.
**/
VOID
EFIAPI
CompalEepromNotify (
  IN EFI_EVENT                Event,
  IN VOID                    *Context
  )
{
  EFI_STATUS                  Status;

  gBS->CloseEvent (Event);

  Status = gBS->LocateProtocol (
                  &gCompalEEPROMDxeProtocolGuid,
                  NULL,
                  (VOID **) &mCompalEepromDxeProtocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  ReplaceSmbios (mSmbios, mCompalEepromDxeProtocol);
}


/**
 Replay Smbios record index number string

 @param[in] Smbios          EFI_SMBIOS_PROTOCOL
 @param[in] CompalEeprom    COMPAL_EEPROM_DXE_PROTOCOL
 @param[in] SmbiosHandle    Point to replay smbios recoder handle.
 @param[in] SmbiosStrIndex  Which string index to replace by eeprom
 @param[in] EepromReadPolicy  Read eeprom policy
                              0x00  Read EEPROM by EC command
                              0x01  Read EEPROM from BIOS SPI ROM
                              0x02  Read EEPROM from SPI direct
@param[in] OemEepromOffset  Read eeprom offset
@param[in] OemEepromSize    Read eeprom size

**/
EFI_STATUS
EFIAPI
SmbiosUpdateStringFromEeprom (
  IN EFI_SMBIOS_PROTOCOL           *Smbios,
  IN COMPAL_EEPROM_DXE_PROTOCOL    *CompalEeprom,
  IN EFI_SMBIOS_HANDLE             *SmbiosHandle,
  IN SMBIOS_TABLE_STRING            SmbiosStrIndex,
  IN EEPROM_READ_POLICY             EepromReadPolicy,
  IN UINT16                         OemEepromOffset,
  IN UINT16                         OemEepromSize
  )
{
  EFI_STATUS                      Status;
  UINT8                          *EepromStr;
  UINTN                           StrLen;
  UINTN                           StrIndex;

  EepromStr = NULL;
  EepromStr = AllocateZeroPool (OemEepromSize + 1); // +1 for string End '\0'
  if (EepromStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Read Serial number from BIOS ROM
  //
  CompalEeprom->CompalEepromDxeRead (
                  CompalEeprom,
                  OemEepromOffset,
                  OemEepromSize,
                  EepromReadPolicy,
                  EepromStr
                  );

  // Check string is valid;
  Status = EFI_NOT_FOUND;
  StrLen = AsciiStrLen (EepromStr);
  if (( StrLen > 0) && (StrLen <= OemEepromSize)) {
    //
    // Update Smbios string
    //
    StrIndex = SmbiosStrIndex;
    Status = Smbios->UpdateString (
                       Smbios,
                       SmbiosHandle,
                       &StrIndex,
                       EepromStr
                       );
  }

  FreePool (EepromStr);
  return Status;
}


/**
 Replay Smbios record index number string

 @param[in] Smbios          EFI_SMBIOS_PROTOCOL
 @param[in] CompalEeprom    COMPAL_EEPROM_DXE_PROTOCOL

 @retval EFI_OUT_OF_RESOURCES         Allocate memory fail.
 @retval EFI_NOT_FOUND                Smbios type or String not found.
 @retval EFI_UNSUPPORTED              String was not added because it is longer than the SMBIOS Table supports.
 @retval EFI_SUCCESS                  The function is executed successfully.
**/
EFI_STATUS
EFIAPI
SmbiosUpdateType01 (
  IN EFI_SMBIOS_PROTOCOL           *Smbios,
  IN COMPAL_EEPROM_DXE_PROTOCOL    *CompalEeprom
  )
{
  EFI_STATUS                      Status;
  SMBIOS_TABLE_TYPE1             *SmbiosType1;
  SMBIOS_TABLE_TYPE1             *TempSmbiosType;
  EFI_SMBIOS_HANDLE               SmbiosHandle;
  EFI_SMBIOS_TYPE                 Type;
  EFI_GUID                        Zero_GUID;
  EFI_GUID                        Eeprom_GUID;

  //
  // Get Smbios Type1(EFI_SMBIOS_TYPE_SYSTEM_INFORMATION) record
  //
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Type = EFI_SMBIOS_TYPE_SYSTEM_INFORMATION;
  SmbiosType1 = NULL;

  Status = Smbios->GetNext (
                      Smbios,
                      &SmbiosHandle,
                      &Type,
                      (EFI_SMBIOS_TABLE_HEADER **)&TempSmbiosType,
                      NULL
                      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }
  //
  // SmbiosType address(TempSmbiosType) will change after update string with Smbios protocol ,
  // so save the old address memory data to SMBIO_TABLE_TYPEX(SmbiosTypeX);
  //
  SmbiosType1 = AllocateCopyPool(sizeof (SMBIOS_TABLE_TYPE1),TempSmbiosType);
  if (SmbiosType1 == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  //
  // Update Serial Number
  //
  Status = SmbiosUpdateStringFromEeprom (
             Smbios,
             CompalEeprom,
             &SmbiosHandle,
             SmbiosType1->SerialNumber,
             DEFAULT_READ_EEPROM_POLICY,
             T66_EEPROM_SYSTEM_SN_OFFSET,
             T66_EEPROM_SYSTEM_SN_LENGTH
             );

  //
  // Update Product Name
  //
  Status = SmbiosUpdateStringFromEeprom (
             Smbios,
             CompalEeprom,
             &SmbiosHandle,
             SmbiosType1->ProductName,
             DEFAULT_READ_EEPROM_POLICY,
             T66_EEPROM_PRODUCT_NUMBER_OFFSET,
             T66_EEPROM_PRODUCT_NUMBER_LENGTH
             );

  //
  // Update SKU Number
  //
  Status = SmbiosUpdateStringFromEeprom (
             Smbios,
             CompalEeprom,
             &SmbiosHandle,
             SmbiosType1->SKUNumber,
             DEFAULT_READ_EEPROM_POLICY,
             T66_EEPROM_OFFSET_SKU_NUM,
             T66_EEPROM_LENGTH_SKU_NUM
             );
  //
  // Update UUID
  //
  CompalEeprom->CompalEepromDxeRead (
                  CompalEeprom,
                  T66_EEPROM_UUID_OFFSET,
                  T66_EEPROM_UUID_LENGTH,
                  DEFAULT_READ_EEPROM_POLICY,
                  (UINT8 *)&Eeprom_GUID
                  );

  ZeroMem (&Zero_GUID, sizeof (Zero_GUID));
  if (CompareGuid (&Eeprom_GUID, &Zero_GUID) == FALSE) {
    //
    // Eeprom UUID is valid , upate UUID
    //
    SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
    Type = EFI_SMBIOS_TYPE_SYSTEM_INFORMATION;
    Status = Smbios->GetNext (
                       Smbios,
                       &SmbiosHandle,
                       &Type,
                       (EFI_SMBIOS_TABLE_HEADER **)&SmbiosType1,
                       NULL
                       );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    CopyGuid (&SmbiosType1->Uuid , &Eeprom_GUID);
  }

  Status = EFI_SUCCESS;

Exit:
  if (SmbiosType1 != NULL) {
    FreePool(SmbiosType1);
  }
  return Status;

}

/**
 Update Smbios Type02

 @param[in] Smbios          EFI_SMBIOS_PROTOCOL
 @param[in] CompalEeprom    COMPAL_EEPROM_DXE_PROTOCOL

 @retval EFI_OUT_OF_RESOURCES         Allocate memory fail.
 @retval EFI_NOT_FOUND                Smbios type or String not found.
 @retval EFI_UNSUPPORTED              String was not added because it is longer than the SMBIOS Table supports.
 @retval EFI_SUCCESS                  The function is executed successfully.
**/
EFI_STATUS
EFIAPI
SmbiosUpdateType02 (
  IN EFI_SMBIOS_PROTOCOL           *Smbios,
  IN COMPAL_EEPROM_DXE_PROTOCOL    *CompalEeprom
  )
{
  EFI_STATUS                      Status;
  SMBIOS_TABLE_TYPE2             *SmbiosType2;
  EFI_SMBIOS_HANDLE               SmbiosHandle;
  EFI_SMBIOS_TYPE                 Type;

  //
  // Get Smbios Type2(EFI_SMBIOS_TYPE_SYSTEM_INFORMATION) record
  //
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Type = EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION;
  Status = Smbios->GetNext (
                      Smbios,
                      &SmbiosHandle,
                      &Type,
                      (EFI_SMBIOS_TABLE_HEADER **)&SmbiosType2,
                      NULL
                      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Update Serial Number
  //
  Status = SmbiosUpdateStringFromEeprom (
             Smbios,
             CompalEeprom,
             &SmbiosHandle,
             SmbiosType2->SerialNumber,
             DEFAULT_READ_EEPROM_POLICY,
             T66_EEPROM_BOARD_SN_OFFSET,
             T66_EEPROM_BOARD_SN_LENGTH
             );

Exit:
  return Status;

}

/**
 Update Smbios Type03

 @param[in] Smbios          EFI_SMBIOS_PROTOCOL
 @param[in] CompalEeprom    COMPAL_EEPROM_DXE_PROTOCOL

 @retval EFI_OUT_OF_RESOURCES         Allocate memory fail.
 @retval EFI_NOT_FOUND                Smbios type or String not found.
 @retval EFI_UNSUPPORTED              String was not added because it is longer than the SMBIOS Table supports.
 @retval EFI_SUCCESS                  The function is executed successfully.
**/
EFI_STATUS
EFIAPI
SmbiosUpdateType03 (
  IN EFI_SMBIOS_PROTOCOL           *Smbios,
  IN COMPAL_EEPROM_DXE_PROTOCOL    *CompalEeprom
  )
{
  EFI_STATUS                      Status;
  SMBIOS_TABLE_TYPE3             *SmbiosType3;
  SMBIOS_TABLE_TYPE3             *TempSmbiosType;
  EFI_SMBIOS_HANDLE               SmbiosHandle;
  EFI_SMBIOS_TYPE                 Type;

  //
  // Get Smbios Type3 (EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE) record
  //
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Type = EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE;
  SmbiosType3 = NULL;
  Status = Smbios->GetNext (
                      Smbios,
                      &SmbiosHandle,
                      &Type,
                      (EFI_SMBIOS_TABLE_HEADER **)&TempSmbiosType,
                      NULL
                      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  SmbiosType3 = AllocateCopyPool(sizeof (SMBIOS_TABLE_TYPE3),TempSmbiosType);
  if (SmbiosType3 == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Update Serial Number
  //
  Status = SmbiosUpdateStringFromEeprom (
             Smbios,
             CompalEeprom,
             &SmbiosHandle,
             SmbiosType3->SerialNumber,
             DEFAULT_READ_EEPROM_POLICY,
             T66_EEPROM_SYSTEM_SN_OFFSET,
             T66_EEPROM_SYSTEM_SN_LENGTH
             );

  //
  // Update AssetTag
  //
  Status = SmbiosUpdateStringFromEeprom (
             Smbios,
             CompalEeprom,
             &SmbiosHandle,
             SmbiosType3->AssetTag,
             DEFAULT_READ_EEPROM_POLICY,
             T66_EEPROM_OFFSET_ASSERT_TAG_NUM,
             T66_EEPROM_LENGTH_ASSERT_TAG_NUM
             );

Exit:
  if (SmbiosType3 != NULL) {
    FreePool(SmbiosType3);
  }
  return Status;
}

/**
 Replace Smbios data

 @param[in] Smbios          EFI_SMBIOS_PROTOCOL
 @param[in] CompalEeprom    COMPAL_EEPROM_DXE_PROTOCOL

 @retval EFI_INVALID_PARAMETER  Smbios or CompalEeprom point to null.
 @retval EFI_SUCCESS            The function is executed successfully.
**/
EFI_STATUS
EFIAPI
ReplaceSmbios (
  IN EFI_SMBIOS_PROTOCOL           *Smbios,
  IN COMPAL_EEPROM_DXE_PROTOCOL    *CompalEeprom
  )
{
  if ((Smbios == NULL) || (CompalEeprom == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  SmbiosUpdateType01 (Smbios, CompalEeprom);
  SmbiosUpdateType02 (Smbios, CompalEeprom);
  SmbiosUpdateType03 (Smbios, CompalEeprom);

  return EFI_SUCCESS;
}

/**
 Update Smbios data

 @param[in] ImageHandle
 @param[in] SystemTable

 @retval EFI_SUCCESS            The function is executed successfully.
**/
EFI_STATUS
EFIAPI
UpdateSmbios (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  EFI_EVENT               Event;
  VOID                   *Registration;

  //
  // Locate Smbios protocol
  //
  mSmbios = NULL;
  Status = gBS->LocateProtocol (
                  &gEfiSmbiosProtocolGuid,
                  NULL,
                  (VOID **) &mSmbios
                  );

  if (EFI_ERROR (Status)) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    SmBiosNotify,
                    NULL,
                    &Event
                    );

    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gEfiSmbiosProtocolGuid,
                      Event,
                      &Registration
                      );
    }
  }

  //
  // Locate CompalEeprom protocol
  //
  mCompalEepromDxeProtocol = NULL;
  Status = gBS->LocateProtocol (
                  &gCompalEEPROMDxeProtocolGuid,
                  NULL,
                  (VOID **) &mCompalEepromDxeProtocol
                  );

  if (EFI_ERROR (Status)) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    CompalEepromNotify,
                    NULL,
                    &Event
                    );

    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gCompalEEPROMDxeProtocolGuid,
                      Event,
                      &Registration
                      );
    }
  }
  ReplaceSmbios (mSmbios, mCompalEepromDxeProtocol);

  return EFI_SUCCESS;
}

