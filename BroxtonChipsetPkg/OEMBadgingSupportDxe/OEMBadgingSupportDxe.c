//;******************************************************************************
//;* Copyright (c) 2012-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; 
//; Module Name:
//; 
//;   OemOEMBadgingSupportDxe.c
//; 
//; Abstract:
//; 
//;   Source file for EFI OEM badging support driver.
//; 

#include <Uefi.h>
#include <ChipsetSetupConfig.h>
#include <OEMBadgingString.h>
#include <Guid/Pcx.h>
#include <Protocol/SetupUtility.h>
#include <OEMBadgingSupportDxe.h>
#include <Library/BadgingSupportLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <Library/DebugLib.h>
extern OEM_BADGING_STRING mOemBadgingString[];
extern OEM_BADGING_STRING mOemBadgingStringAfterSelect[BADGING_AFTER_SELECT_COUNT][BADGING_STRINGS_COUNT];
extern OEM_BADGING_STRING mOemBadgingStringInTextMode[];
extern OEM_BADGING_STRING mOemBadgingStringAfterSelectInTextMode[BADGING_AFTER_SELECT_COUNT][BADGING_STRINGS_COUNT_TEXT_MODE];

EFI_STATUS
InitializeBadgingSupportProtocol (
  IN    EFI_HANDLE                      ImageHandle,
  IN    EFI_SYSTEM_TABLE                *SystemTable,
  IN    EFI_OEM_BADGING_LOGO_DATA       *BadgingData,
  IN    UINTN                           NumberOfLogo,
  IN    OEM_VIDEO_MODE_SCR_STR_DATA     *OemVidoeModeScreenStringData,
  IN    UINTN                           NumberOfVidoeModeScreenStringDataCount,
  IN    OEM_BADGING_STRING              *OemBadgingString,
  IN    OEM_BADGING_STRING              **OemBadgingStringAfterSelect,
  IN    UINTN                           NumberOfString
);

BOOLEAN
IntelInside (
  VOID
  );

BOOLEAN
IntelDuCore (
  VOID
  );

EFI_OEM_BADGING_LOGO_DATA mBadgingData[] = {
  {EFI_DEFAULT_PCX_LOGO_GUID,
   EfiBadgingSupportFormatPCX,
   EfiBadgingSupportDisplayAttributeCenter,
   0,
   0,
   NULL,
   EfiBadgingSupportImageLogo
  },
  //
  // BIOS Vendor Insyde Badge
  //
  {
    EFI_INSYDE_BOOT_BADGING_GUID,
    EfiBadgingSupportFormatBMP,
    EfiBadgingSupportDisplayAttributeCustomized,
    0,
    0,
    NULL,
    EfiBadgingSupportImageBoot
  },
  {EFI_INSYDE_BOOT_BADGING_GUID,
   EfiBadgingSupportFormatBMP,
   EfiBadgingSupportDisplayAttributeCenter,
   0,
   0,
   NULL,
   EfiBadgingSupportImageBadge
  }
};

OEM_VIDEO_MODE_SCR_STR_DATA mOemVidoeModeScreenStringData[] = {
  //Bios Setup String
  {
    640,
    480,
    OemEnterSetupStr,
    400,
    392
  },
  {
    800,
    600,
    OemEnterSetupStr,
    500,
    490
  },
  {
    1024,
    768,
    OemEnterSetupStr,
    640,
    628
  },
  //Port80String
  {
    640,
    480,
    OemPort80CodeStr,
    600,
    440
  },
  {
    800,
    600,
    OemPort80CodeStr,
    750,
    550
  },
  {
    1024,
    768,
    OemPort80CodeStr,
    960,
    704
  },
  //Quality String
  {
    640,
    480,
    OemBuildQualityStr,
    280,
    232
  },
  {
    800,
    600,
    OemBuildQualityStr,
    350,
    290
  },
  {
    1024,
    768,
    OemBuildQualityStr,
    448,
    372
  }
};


/**
  Entry point of EFI OEM Badging Support driver

  @param[in]  ImageHandle - image handle of this driver
  @param[in]  SystemTable - pointer to standard EFI system table

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurred when executing this entry point.

**/
EFI_STATUS
OEMBadgingEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_SETUP_UTILITY_PROTOCOL            *EfiSetupUtility;
  CHIPSET_CONFIGURATION                 *SetupVariable;
  UINTN                                 StringCount;
  OEM_BADGING_STRING                    *OemBadgingString;
  EFI_OEM_BADGING_LOGO_DATA             *BadgingDataPointer;
  UINTN                                 BadgingDataSize;
  OEM_BADGING_STRING                    *OemBadgingStringPointer;
  OEM_BADGING_STRING                    *OemBadgingStringInTextModePointer;
  OEM_BADGING_STRING                    *OemBadgingStringAfterSelectPointer;
  OEM_BADGING_STRING                    *OemBadgingStringAfterSelectInTextModePointer;
  
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&EfiSetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  SetupVariable = (CHIPSET_CONFIGURATION *)(EfiSetupUtility->SetupNvData);

  //
  // Select OEM badging strings based on graphic mode or text mode.
  //
  StringCount = SetupVariable->QuietBoot ? BADGING_STRINGS_COUNT : BADGING_STRINGS_COUNT_TEXT_MODE;
  BadgingDataPointer = mBadgingData;
  BadgingDataSize = sizeof (mBadgingData) / sizeof (EFI_OEM_BADGING_LOGO_DATA);
  OemBadgingStringPointer = mOemBadgingString;
  OemBadgingStringInTextModePointer = mOemBadgingStringInTextMode;
  OemBadgingStringAfterSelectPointer = &mOemBadgingStringAfterSelect[0][0];
  OemBadgingStringAfterSelectInTextModePointer = &mOemBadgingStringAfterSelectInTextMode[0][0];
  Status = OemSvcUpdateOemBadgingLogoData (
             &BadgingDataPointer,
             &BadgingDataSize,
             &OemBadgingStringPointer,
             &OemBadgingStringInTextModePointer,
             &StringCount,
             &OemBadgingStringAfterSelectPointer,
             &OemBadgingStringAfterSelectInTextModePointer
             );

  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcUpdateOemBadgingLogoData, Status : %r\n", Status));
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  OemBadgingString = SetupVariable->QuietBoot ? OemBadgingStringAfterSelectPointer : OemBadgingStringAfterSelectInTextModePointer;

  Status = InitializeBadgingSupportProtocol (ImageHandle,
                                             SystemTable,
                                             BadgingDataPointer,
                                             BadgingDataSize,
                                             mOemVidoeModeScreenStringData,
                                             sizeof (mOemVidoeModeScreenStringData) / sizeof (OEM_VIDEO_MODE_SCR_STR_DATA),
                                             SetupVariable->QuietBoot ? OemBadgingStringPointer : OemBadgingStringInTextModePointer,
                                             (OEM_BADGING_STRING**) OemBadgingString,
                                             StringCount
                                             );
  return Status;
}

BOOLEAN
IntelInside (
  VOID
  )
{
  UINT32 RegEax;
  UINT32 RegEbx;
  UINT32 RegEcx;
  UINT32 RegEdx;

  //
  // Is a GenuineIntel CPU?
  //
  AsmCpuid (0x0, &RegEax, &RegEbx, &RegEcx, &RegEdx);
  if ((RegEbx != SIGNATURE_32('G','e','n','u')) || (RegEdx != SIGNATURE_32('i','n','e','I')) || (RegEcx != SIGNATURE_32('n','t','e','l'))) {
    return FALSE;
  }

  return TRUE;
}

BOOLEAN
IntelDuCore (
  VOID
  )
{
  UINT32 RegEax;
  UINT32 RegEbx;
  UINT32 RegEcx;
  UINT32 RegEdx;
  UINT32 NumOfThreading = 1;
  UINTN NumOfLogicalProcess = 0;
  UINTN NumOfCores = 0;
  // Feature Information
  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, &RegEbx, &RegEcx, &RegEdx);

  if (((RegEax & FAMILYMODEL_MASK) >= FAMILYMODEL_PRESCOTT_F4x) ||((RegEax & FAMILYMODEL_MASK) == FAMILY_CONROE)) {

    NumOfLogicalProcess = (UINTN) ((RegEbx >> 16) & 0xf);
    NumOfThreading = (UINT32) ((RegEbx >> 16) & 0xf);
    AsmCpuid (EFI_CPUID_CACHE_PARAMS, &RegEax, &RegEbx, &RegEcx, &RegEdx);

    NumOfCores = (UINTN) ((RegEax >> 26) & 0x3f);

    NumOfCores += 1;

    if (NumOfCores >= 1)
      return TRUE;
    else
      return FALSE;
    }

  return  FALSE;
}
