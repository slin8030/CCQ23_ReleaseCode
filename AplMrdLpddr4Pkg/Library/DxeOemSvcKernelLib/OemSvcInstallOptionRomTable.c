/** @file
  Provide OEM to install the PCI Option ROM table and Non-PCI Option ROM table.

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

#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <ScAccess.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <SaRegs.h>

//
// data type definitions
//

//
// Global variables for PCI Option Roms
//
#define NULL_ROM_FILE_GUID \
  { \
    0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
  }

#define ONBOARD_VIDEO_OPTION_ROM_FILE_GUID \
  { \
    0x8dfae5d4, 0xb50e, 0x4c10, 0x96, 0xe6, 0xf2, 0xc2, 0x66, 0xca, 0xcb, 0xb6 \
  }

#define ONBOARD_VIDEO_OPTION_ROM_FILE_GUID_2 \
  { \
    0x5264d44a, 0x7c8c, 0x4289, 0xbb, 0x5d, 0xd1, 0xb5, 0xce, 0x42, 0x57, 0xb2 \
  }

#define IDE_RAID_OPTION_ROM_FILE_GUID \
  { \
    0x501737ab, 0x9d1a, 0x4856, 0x86, 0xd3, 0x7f, 0x12, 0x87, 0xfa, 0x5a, 0x55 \
  }

#define IDE_AHCI_OPTION_ROM_FILE_GUID \
  { \
    0xB017C09D, 0xEDC1, 0x4940, 0xB1, 0x3E, 0x57, 0xE9, 0x56, 0x60, 0xC9, 0x0F \
  }
                               
#define PXE_UNDI_OPTION_ROM_FILE_GUID \
  { \
    0x4c316c9a, 0xafd9, 0x4e33, 0xae, 0xab, 0x26, 0xc4, 0xa4, 0xac, 0xc0, 0xf7 \
  }

//
// Global variables for Non-PCI Option Roms
//
#define SYSTEM_ROM_FILE_GUID \
  { \
    0x1547B4F3, 0x3E8A, 0x4FEF, 0x81, 0xC8, 0x32, 0x8E, 0xD6, 0x47, 0xAB, 0x1A \
  }

#define NULL_ROM_FILE_GUID \
  { \
    0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
  }

#define PXE_BASE_OPTION_ROM_FILE_GUID \
  { \
    0x6f2bc426, 0x8d83, 0x4f17, 0xbe, 0x2c, 0x6b, 0x1f, 0xcb, 0xd1, 0x4c, 0x80 \
  }

#define BIS_OPTION_ROM_FILE_GUID \
  { \
    0xAE21640A, 0x21D2, 0x4ec4, 0xA9, 0x96, 0x86, 0xC2, 0x6C, 0x9E, 0xD0, 0x91 \
  }

#define BIS_LOADER_FILE_GUID \
  { \
    0xf14aa356, 0x9774, 0x481c, 0xb5, 0xf1, 0x79, 0xc1, 0xb5, 0xea, 0xe8, 0xa1 \
  }

#define OEM_I15VARS_FILE_GUID \
  { \
    0x340AD445, 0x06EF, 0x43f1, 0x89, 0x97, 0xFC, 0xD6, 0x7F, 0xC2, 0x7E, 0xEF \
  }

#define OEM_INT15CB_FILE_GUID \
  { \
    0x340AD445, 0x06EF, 0x43f1, 0x89, 0x97, 0xFC, 0xD6, 0x7F, 0xC2, 0x7E, 0xF0 \
  }

#define TPMMP_GUID \
  { \
    0x4B8D2F76, 0xD9E7, 0x46dc, 0xB5, 0xED, 0xEF, 0xEF, 0x94, 0x76, 0xAF, 0x4A \
  }

//
// module variables
//

//
// Pci Option Rom Table
//
EFI_GUID mPxeFileGuid = PXE_UNDI_OPTION_ROM_FILE_GUID;

PCI_OPTION_ROM_TABLE      PciOptionRomTable[] = {
  //
  // Video Option Rom
  //
  {
    ONBOARD_VIDEO_OPTION_ROM_FILE_GUID,
    V_SA_IGD_VID,
    V_SA_IGD_DID
  },
  {
    ONBOARD_VIDEO_OPTION_ROM_FILE_GUID,
    V_SA_IGD_VID,
    V_SA_IGD_DID_BXTP
  },
//[-start-160317-IB03090425-add]//
  {
    ONBOARD_VIDEO_OPTION_ROM_FILE_GUID,
    V_SA_IGD_VID,
    V_SA_IGD_DID_BXTP_1
  },
//[-end-160317-IB03090425-add]//
  {
    NULL_ROM_FILE_GUID,
    0xffff,
    0xffff
  }
};

//
// Non Pci Option Rom Table
//

//
// System Rom table
//
SYSTEM_ROM_TABLE    SystemRomTable[] = {
  {
    //
    // CSM16 binary
    //
    SYSTEM_ROM_FILE_GUID,
    TRUE,
    SYSTEM_ROM
  },
  {
    NULL_ROM_FILE_GUID,
    FALSE,
    MAX_NUM
  }
};


// 
// Oem Int table 
//
SYSTEM_ROM_TABLE    SystemOemIntTable[] = {
  {
    NULL_ROM_FILE_GUID,
    FALSE,
    MAX_NUM
  }
};

    
//
//  Service ROM Table
//
SERVICE_ROM_TABLE  ServiceRomTable[] = {
  {
    NULL_ROM_FILE_GUID,
    FALSE,
    FALSE,
    MAX_NUM
  }
};

//
// TPM Rom table
//
SYSTEM_ROM_TABLE    TpmTable[] = {
  {
    //
    // TPMMP binary
    //
    TPMMP_GUID,
    TRUE,
    TPM_ROM
  },
  {
    NULL_ROM_FILE_GUID,
    FALSE,
    MAX_NUM
  }
};

//
//  AMT ROM Table
//
SERVICE_ROM_TABLE  AMTRomTable[] = {
  {
    NULL_ROM_FILE_GUID,
    FALSE,
    FALSE,
    MAX_NUM
  }
};

/**
  Provide OEM to install the PCI Option ROM table and Non-PCI Option ROM table. 
  The detail refers to the document "OptionRomTable Restructure User Guide".

  @param[in]   *RomType              The type of option rom. This parameter decides which kind of option ROM table will be access.
  @param[out]  **RomTable            A pointer to the option ROM table.
  
  @retval      EFI_SUCCESS           Get Option ROM Table info failed.
  @retval      EFI_MEDIA_CHANGED     Get Option ROM Table info success.
  @retval      Others                Depends on customization.
**/
EFI_STATUS 
OemSvcInstallOptionRomTable (
  IN  UINT8                                 RomType,
  OUT VOID                                  **mOptionRomTable
  )
{
  VOID                                  *OptionRomTable;
  UINTN                                 OptionRomTableSize;

  switch (RomType) {
  case PCI_OPROM:
    OptionRomTable      = (void *)PciOptionRomTable;
    OptionRomTableSize  = sizeof(PciOptionRomTable);
    break;    
    
  case SYSTEM_ROM:
    OptionRomTable      = (void *)SystemRomTable;
    OptionRomTableSize  = sizeof(SystemRomTable);
    break;

  case SYSTEM_OEM_INT_ROM:
    OptionRomTable      = (void *)SystemOemIntTable;
    OptionRomTableSize  = sizeof(SystemOemIntTable);
    break;    
    
  case SERVICE_ROM:
    OptionRomTable      = (void *)ServiceRomTable;
    OptionRomTableSize  = sizeof(ServiceRomTable);
    break;    

  case TPM_ROM:
    OptionRomTable      = (void *)TpmTable;
    OptionRomTableSize  = sizeof(TpmTable);
    break; 
    
  case AMT_ROM:
    OptionRomTable      = (void *)AMTRomTable;
    OptionRomTableSize  = sizeof(AMTRomTable);
    break; 
    
  default:
    return EFI_SUCCESS;
  }

  (*mOptionRomTable)   = AllocateZeroPool (OptionRomTableSize);
  CopyMem ((*mOptionRomTable), OptionRomTable, OptionRomTableSize);

  return EFI_MEDIA_CHANGED;
}

