/** @file
  OEM can provide a way to get replace string for OEM depend.
  It's like DMI Tool but difference.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeOemSvcKernelLib.h>
//[-start-161112-IB07400812-add]//
#include <Library/DebugLib.h>
#include <Library/MultiPlatformBaseLib.h>
#include <Library/PrintLib.h>
//[-end-161112-IB07400812-add]//


/**
  This service will be call by each time process SMBIOS string.
  OEM can provide a way to get replace string for OEM depend.
  
  @param[in]  Type           SMBIOS type number.
  @param[in]  FieldOffset    Field offset.
  @param[out] StrLength      String length.
  @param[out] String         String point.
  
  @retval    EFI_UNSUPPORTED       Returns unsupported by default.
  @retval    EFI_SUCCESS           N/A.
  @retval    EFI_MEDIA_CHANGED     The value of OUT parameter is changed. 
**/
EFI_STATUS
OemSvcDxeGetSmbiosReplaceString (
  IN  EFI_SMBIOS_TYPE   Type,
  IN  UINT8             FieldOffset,
  OUT UINTN            *StrLength,
  OUT CHAR8           **String
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/
  
//[-start-161112-IB07400812-add]//
#ifdef APOLLOLAKE_CRB
  STATIC UINT8 SlotNumber = 0;

  if (Type == EFI_SMBIOS_TYPE_SYSTEM_SLOTS) {
    
    switch (SlotNumber) {
    case 0:
      //
      // APL-I Oxbow Hill slot 1: J6B1
      // Apollo Lake RVP slot 1 : J6C1
      //
      if (IsIOTGBoardIds()) {
        AsciiSPrint (*String, *StrLength + 1, "J6B1;");
      } else {
        AsciiSPrint (*String, *StrLength + 1, "J6C1;");
      }
      break;
    case 1:
      //
      // Apollo Lake RVP slot 2 : J7C1
      // APL-I Oxbow Hill slot 2: N/A
      //
      if (IsIOTGBoardIds()) {
        AsciiSPrint (*String, *StrLength + 1, "N/A ;");
      } else {
        AsciiSPrint (*String, *StrLength + 1, "J7C1;");
      }
      break;
    default:
      break;
    }
    DEBUG ((EFI_D_ERROR, "FieldOffset = %x\n", FieldOffset));
    DEBUG ((EFI_D_ERROR, "*StrLength = %x\n", *StrLength));
    DEBUG ((EFI_D_ERROR, "*String = %a\n", *String));
    SlotNumber++;
  }
#endif
//[-end-161112-IB07400812-add]//

  return EFI_UNSUPPORTED;
}
