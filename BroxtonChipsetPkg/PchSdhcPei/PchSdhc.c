/** @file
  Implementation of PchSdhcPei module for Crisis Recovery

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "PchSdhc.h"
#include <ScAccess.h>
#include <SaCommonDefinitions.h>

#define APL_SD_CARD_WORKAROUND

#ifdef APL_SD_CARD_WORKAROUND
#include <Library/GpioLib.h>
#include <Library/SideBandLib.h>
#endif

UINT32  mSocEmmcDevNumber   = 0x1C;
UINT32  mSocSdDevNumber     = 0x1B;
BOOLEAN mSocEmmcExist       = FALSE;
BOOLEAN mSocSdCardExist     = FALSE;
BOOLEAN mPciSdCardExist     = FALSE;

UINT32  mRootBridge;
UINT32  mPciAddress;

typedef struct {
  UINT8 PI;
  UINT8 SubClassCode;
  UINT8 BaseCode;
} PCI_CLASSC;

//
// PPI interface function
//
STATIC
EFI_STATUS
GetSdhcController (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN       PEI_SD_CONTROLLER_PPI      *This,
  IN       UINT8                      SdControllerId,
  OUT      UINTN                      *ControllerType,
  OUT      UINT64                     *PciCfgAddress
  );  
  
STATIC  
EFI_STATUS  
EFIAPI  
EndOfPeiPpiNotifyCallback (  
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN CONST EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN       VOID                       *Ppi
  );                                
                                    
//                                  
// Globals                          
//                                  
STATIC PEI_SD_CONTROLLER_PPI mSdControllerPpi = { GetSdhcController };
                                    
STATIC EFI_PEI_PPI_DESCRIPTOR mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiSdControllerPpiGuid,
  NULL
};

STATIC EFI_PEI_NOTIFY_DESCRIPTOR mNotifyList = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiEndOfPeiSignalPpiGuid,
  EndOfPeiPpiNotifyCallback
};

EFI_STATUS
SocSdhcInit (
  IN       EFI_FFS_FILE_HEADER        *FfsHeader,
  IN CONST EFI_PEI_SERVICES           **PeiServices
  )
{
  //TODO: DLL setting
  return EFI_SUCCESS;
}

//
// Helper function
//
STATIC
EFI_STATUS
EnableSdhcController (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN       PCH_SDHC_DEVICE            *PeiPchSdhcDev,
  IN       UINT8                      SdControllerId
  );

/**

 @param [in]   FfsHeader
 @param [in]   PeiServices


**/
EFI_STATUS
PeimInitializePchSdhc (
  IN       EFI_FFS_FILE_HEADER        *FfsHeader,
  IN CONST EFI_PEI_SERVICES           **PeiServices
  )
{
  EFI_STATUS              Status;
  EFI_PEI_PCI_CFG2_PPI    *PciCfgPpi;
  PCH_SDHC_DEVICE         *PeiPchSdhcDev;

  SocSdhcInit(FfsHeader, PeiServices);
  
  //
  // Get PciCfgPpi at first.
  //
  PciCfgPpi = (**PeiServices).PciCfg;

  PeiPchSdhcDev = (PCH_SDHC_DEVICE *)AllocatePages (1);
  ASSERT (PeiPchSdhcDev != NULL);
  ZeroMem (PeiPchSdhcDev, sizeof(PeiPchSdhcDev));

  PeiPchSdhcDev->Signature            = PEI_PCH_SDHC_SIGNATURE;
  PeiPchSdhcDev->SdControllerPpi      = mSdControllerPpi;
  PeiPchSdhcDev->PpiList              = mPpiList;
  PeiPchSdhcDev->PpiList.Ppi          = &PeiPchSdhcDev->SdControllerPpi;
  PeiPchSdhcDev->PciCfgPpi            = PciCfgPpi;
  PeiPchSdhcDev->NotifyList           = mNotifyList;
  PeiPchSdhcDev->TotalSdControllers   = 3; //Soc eMMC, Soc SD, Pci-E SD card.

  //
  // Assign resources and enable EMMC controllers
  //
  PeiPchSdhcDev->MemBase = SOC_EMMC_MMIO_ADDRESS;
  Status = EnableSdhcController (PeiServices, PeiPchSdhcDev, ID_SOC_EMMC);
  if (!EFI_ERROR(Status)) {
    mSocEmmcExist = TRUE;
  }
  
  PeiPchSdhcDev->MemBase = SOC_SD_MMIO_ADDRESS;
  Status = EnableSdhcController (PeiServices, PeiPchSdhcDev, ID_SOC_SD);
  if (!EFI_ERROR(Status)) {
    mSocSdCardExist = TRUE;
  }
  
  PeiPchSdhcDev->MemBase = PCI_SD_MMIO_ADDRESS;
  Status = EnableSdhcController (PeiServices, PeiPchSdhcDev, ID_PCI_SD);
  if (!EFI_ERROR(Status)) {
    mPciSdCardExist = TRUE;
  }

  if (!(mSocEmmcExist | mSocSdCardExist | mPciSdCardExist)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Install SD Controller PPI
  //
  Status = (**PeiServices).InstallPpi (
                             PeiServices,
                             &PeiPchSdhcDev->PpiList
                             );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  //
  // Install notification in order to reset the SDHC
  //
  Status = (**PeiServices).NotifyPpi (
                             PeiServices,
                             &PeiPchSdhcDev->NotifyList
                             );

  return Status;
}


/**
 Retrieve SDHC controller information

 @param [in]   PeiServices              Pointer to the PEI Services Table.
 @param [in]   This                     Pointer to PEI_SD_CONTROLLER_PPI
 @param [in]   SdControllerId           SDHC Controller ID
 @param [out]  ControllerType           Result SDHC controller type
 @param [out]  PciCfgAddress            Result SDHC base address

 @retval EFI_INVALID_PARAMETER          Invalid SdControllerId is given
 @retval EFI_SUCCESS                    SDHC controller information is retrieved successfully

**/
STATIC
EFI_STATUS
GetSdhcController (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN       PEI_SD_CONTROLLER_PPI      *This,
  IN       UINT8                      SdControllerId,
  OUT      UINTN                      *ControllerType,
  OUT      UINT64                     *PciCfgAddress
  )
{
  PCH_SDHC_DEVICE         *PeiPchSdhcDev;

  PeiPchSdhcDev = PCH_SDHC_DEVICE_FROM_THIS (This);

  if (SdControllerId >= PeiPchSdhcDev->TotalSdControllers) {
    return EFI_INVALID_PARAMETER;
  }
  *ControllerType = 0x00;
  *PciCfgAddress  = 0x00;
  
  if ((mSocEmmcExist) && (SdControllerId == ID_SOC_EMMC)) {
    *ControllerType = PEI_SDHC_CONTROLLER;
    *PciCfgAddress  = EFI_PCI_ADDRESS (0, mSocEmmcDevNumber, 0, 0);
  }
  if ((mSocSdCardExist) && (SdControllerId == ID_SOC_SD)) {
    *ControllerType = PEI_SDHC_CONTROLLER;
    *PciCfgAddress  = EFI_PCI_ADDRESS (0, mSocSdDevNumber, 0, 0);
  }
  if ((mPciSdCardExist) && (SdControllerId == ID_PCI_SD)) {
    *ControllerType = PEI_SDHC_CONTROLLER;
    *PciCfgAddress  = PeiPchSdhcDev->PciCfgAddress;
  }

  if (*PciCfgAddress == 0x00) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EnableSocSdhcController (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN       PCH_SDHC_DEVICE            *PeiPchSdhcDev,
  IN       UINT8                      SdControllerId
  )
{
  EFI_PEI_PCI_CFG2_PPI     *PciCfgPpi;
  PCI_CLASSC               PciClass;
  UINT32                   VidDid;
  UINT8                    Data8;
#ifdef APL_SD_CARD_WORKAROUND
  UINT32                   SdCardCdDw0;
#endif

  PciCfgPpi   = PeiPchSdhcDev->PciCfgPpi;

  //
  // Enable eMMC
  //
  if (SdControllerId == ID_SOC_EMMC) {
    VidDid = MmPci32(0, 0, mSocEmmcDevNumber, 0, PCI_VENDOR_ID_OFFSET);
    DEBUG ((EFI_D_INFO, "SOC eMMC : VidDid = 0x%08x\n", VidDid));

    PciClass.BaseCode     = MmPci8(0, 0, mSocEmmcDevNumber, 0, PCI_CLASSCODE_OFFSET + 2);
    PciClass.SubClassCode = MmPci8(0, 0, mSocEmmcDevNumber, 0, PCI_CLASSCODE_OFFSET + 1);
    PciClass.PI           = MmPci8(0, 0, mSocEmmcDevNumber, 0, PCI_CLASSCODE_OFFSET);

    if ((PciClass.BaseCode != PCI_CLASS_SYSTEM_PERIPHERAL) ||
        (PciClass.SubClassCode != PCI_SUBCLASS_SD_HOST_CONTROLLER) ||
        ((PciClass.PI != PCI_IF_STANDARD_HOST_NO_DMA) && (PciClass.PI != PCI_IF_STANDARD_HOST_SUPPORT_DMA))
       ) {
      return  EFI_UNSUPPORTED;
    }

    //
    // Enable SDHC
    //
    Data8 = MmPci8(0, 0, mSocEmmcDevNumber, 0, PCI_COMMAND_OFFSET);

    //
    // Enable STATUSCOMMAND BME & MSE
    // Bit 1 : MSE   ;  Bit 2 : BME
    //
    Data8 |= 0x06;

    MmPci8(0, 0, mSocEmmcDevNumber, 0, PCI_COMMAND_OFFSET)          = Data8;
    MmPci32(0, 0, mSocEmmcDevNumber, 0, PCI_BASE_ADDRESSREG_OFFSET) = SOC_EMMC_MMIO_ADDRESS;
  }
  
  //
  // Enable SD
  //
  if (SdControllerId == ID_SOC_SD) {
    VidDid = MmPci32(0, 0, mSocSdDevNumber, 0, PCI_VENDOR_ID_OFFSET);
    DEBUG ((EFI_D_INFO, "SOC SD Card : VidDid = 0x%08x\n", VidDid));

    PciClass.BaseCode     = MmPci8(0, 0, mSocSdDevNumber, 0, PCI_CLASSCODE_OFFSET + 2);
    PciClass.SubClassCode = MmPci8(0, 0, mSocSdDevNumber, 0, PCI_CLASSCODE_OFFSET + 1);
    PciClass.PI           = MmPci8(0, 0, mSocSdDevNumber, 0, PCI_CLASSCODE_OFFSET);

    if ((PciClass.BaseCode != PCI_CLASS_SYSTEM_PERIPHERAL) ||
        (PciClass.SubClassCode != PCI_SUBCLASS_SD_HOST_CONTROLLER) ||
        ((PciClass.PI != PCI_IF_STANDARD_HOST_NO_DMA) && (PciClass.PI != PCI_IF_STANDARD_HOST_SUPPORT_DMA))
       ) {
      return  EFI_UNSUPPORTED;
    }

    //
    // Enable SDHC
    //
    Data8 = MmPci8(0, 0, mSocSdDevNumber, 0, PCI_COMMAND_OFFSET);

    //
    // Enable STATUSCOMMAND BME & MSE
    // Bit 1 : MSE   ;  Bit 2 : BME
    //
    Data8 |= 0x06;

    MmPci8(0, 0, mSocSdDevNumber, 0, PCI_COMMAND_OFFSET)          = Data8;
    MmPci32(0, 0, mSocSdDevNumber, 0, PCI_BASE_ADDRESSREG_OFFSET) = SOC_SD_MMIO_ADDRESS;
    
    //
    // Check GPIO for SD card CD pin for Apollo Lake
    //
#ifdef APL_SD_CARD_WORKAROUND
    //
    // Read SdCard CD pin (GPIO_177) Bit 1
    //
    SdCardCdDw0 = GpioRead (SOUTHWEST, 0x5D0);
    //
    // Feedback invert value of CD pin to GRR3
    //
    SideBandAndThenOr32 (0xD6, 0x608, ~(UINT32)(0x20), SdCardCdDw0 & 0x02 ? 0x00 : 0x20);
#endif    
  }

  return EFI_SUCCESS;
}

/**
 GC_TODO: Add function description

 @param [in]   PeiServices      GC_TODO: add argument description
 @param [in]   PeiPchSdhcDev    GC_TODO: add argument description
 @param [in]   SdControllerId   GC_TODO: add argument description

 @retval EFI_INVALID_PARAMETER  GC_TODO: Add description for return value
 @retval EFI_SUCCESS            GC_TODO: Add description for return value

**/
STATIC
EFI_STATUS
EnableSdhcController (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN       PCH_SDHC_DEVICE            *PeiPchSdhcDev,
  IN       UINT8                      SdControllerId
  )
{
  UINTN                     Index;
  EFI_PEI_PCI_CFG2_PPI      *PciCfgPpi;
  UINT32                    Bridge;
  UINT32                    Address;
  UINT32                    Register;

  if (SdControllerId >= PeiPchSdhcDev->TotalSdControllers) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SdControllerId == ID_SOC_EMMC) || (SdControllerId == ID_SOC_SD)) {
    return EnableSocSdhcController (PeiServices, PeiPchSdhcDev, SdControllerId);
  }
  
  PciCfgPpi = PeiPchSdhcDev->PciCfgPpi;
  //
  // Discover SDHC through PCIE bridge
  //
  for (Index = 0; Index < PCH_NUMBER_OF_PCIE_BRIDGES; Index ++) {
    //
    // Setup appropriate value to PCIE bridge
    //
    Bridge   = PCH_PCIE_BRIDGE_ADDRESS + (Index << 8);
    Register = (SDHC_BUS_NUMBER << 8) + (SDHC_BUS_NUMBER << 16);
    
    //
    // Assign bus number to PCIE bridge
    //
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 Bridge | ROOT_BRIDGE_BUS_REGISTER,
                 &Register
                 );
    //
    // Discover SDHC
    //
    Address  = (SDHC_BUS_NUMBER << 24) + (SDHC_FUNCTION_NUMBER << 8);
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 Address,
                 &Register
                 );
    if (Register == 0xffff) {
      //
      // SDHC not found, clear bus number to PCIE bridge
      //
      goto error;
    }

    //
    // Check the class code 
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 Address | CLASSC_REGISTER,
                 &Register
                 );
    Register &= 0xffffff00;
    if (Register != SDC_CLASSC) {
      //
      // Not SDHC, clear bus number to PCIE bridge
      //
      goto error;
    }
    PeiPchSdhcDev->RootBridge = Bridge;
    PeiPchSdhcDev->PciCfgAddress = Address;
    //
    // Assign address range for root bridge
    //
    Register = ((PCI_SD_MMIO_ADDRESS + 0x00100000) & 0xfff00000) + ((PCI_SD_MMIO_ADDRESS & 0xfff00000) >> 16);
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 Bridge | ROOT_BRIDGE_ADDRESS_REGISTER,
                 &Register
                 );
    //
    // Assign address prefetchable range for root bridge
    //
    Register = ((PCI_SD_MMIO_ADDRESS + 0x00200000) & 0xfff00000) + (((PCI_SD_MMIO_ADDRESS + 0x00100000) & 0xfff00000) >> 16);
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 Bridge | ROOT_BRIDGE_ADDRESS_REGISTER + 4,
                 &Register
                 );
    //
    // Assign base address register to SDHC
    //
    Register = PCI_SD_MMIO_ADDRESS;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 Address | SDHC_BASE_ADDRESS_REGISTER,
                 &Register
                 );
    //
    // Enable root bridge
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 Bridge | COMMAND_REGISTER,
                 &Register
                 );
    Register |= 0x06;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 Bridge | COMMAND_REGISTER,
                 &Register
                 );
    //
    // Enable SDHC
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 Address | COMMAND_REGISTER,
                 &Register
                 );
    Register |= 0x06;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 Address | COMMAND_REGISTER,
                 &Register
                 );
    return EFI_SUCCESS;
error:
    Register = 0;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 Bridge | ROOT_BRIDGE_BUS_REGISTER,
                 &Register
                 );
  }

  return EFI_NOT_FOUND;
}


/**
 Register notify ppi to reset the SDHC.


 @param[in]   PeiServices               Pointer to the PEI Services Table.
 @param[in]   NotifyDescriptor          Pointer to the notify descriptor

 @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
EFIAPI
EndOfPeiPpiNotifyCallback (
  IN CONST EFI_PEI_SERVICES           **PeiServices,
  IN CONST EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN       VOID                       *Ppi
  )
{
  EFI_PEI_PCI_CFG2_PPI        *PciCfgPpi;
  UINT8                       *BaseAddress;
  UINT32                      Register;
  PCH_SDHC_DEVICE             *PeiPchSdhcDev;
  
  PeiPchSdhcDev = PCH_SDHC_DEVICE_FROM_NOTIFY_DESC (NotifyDescriptor);

  if (mSocEmmcExist) {
    //
    // Reset the HC
    //
    BaseAddress = (UINT8*)SOC_EMMC_MMIO_ADDRESS;
    BaseAddress = BaseAddress + (*(UINT32*)(UINTN)BaseAddress & 0xff);
    
    //
    // Halt HC first
    //
    Register = *(UINT32*)(UINTN)BaseAddress;
    Register &= ~0x01;
    *(UINT32*)(UINTN)BaseAddress = Register;

    MicroSecondDelay (1);
    
    //
    // HCReset
    //
    Register = *(UINT32*)(UINTN)BaseAddress;
    Register |= 0x02;
    *(UINT32*)(UINTN)BaseAddress = Register;
    
    //
    // Get PciCfgPpi at first.
    //
    PciCfgPpi = (**PeiServices).PciCfg;
    
    //
    // Disable SDHC
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->PciCfgAddress | COMMAND_REGISTER,
                 &Register
                 );
    Register &= ~0x06;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->PciCfgAddress | COMMAND_REGISTER,
                 &Register
                 );
  }
  
  if (mSocSdCardExist) {
    //
    // Reset the HC
    //
    BaseAddress = (UINT8*)SOC_SD_MMIO_ADDRESS;
    BaseAddress = BaseAddress + (*(UINT32*)(UINTN)BaseAddress & 0xff);
    
    //
    // Halt HC first
    //
    Register = *(UINT32*)(UINTN)BaseAddress;
    Register &= ~0x01;
    *(UINT32*)(UINTN)BaseAddress = Register;

    MicroSecondDelay (1);
    
    //
    // HCReset
    //
    Register = *(UINT32*)(UINTN)BaseAddress;
    Register |= 0x02;
    *(UINT32*)(UINTN)BaseAddress = Register;
    
    //
    // Get PciCfgPpi at first.
    //
    PciCfgPpi = (**PeiServices).PciCfg;
    
    //
    // Disable SDHC
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->PciCfgAddress | COMMAND_REGISTER,
                 &Register
                 );
    Register &= ~0x06;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->PciCfgAddress | COMMAND_REGISTER,
                 &Register
                 );
  }

  if (mPciSdCardExist) {
    //
    // Reset the HC
    //
    BaseAddress = (UINT8*)PCI_SD_MMIO_ADDRESS;
    BaseAddress = BaseAddress + (*(UINT32*)(UINTN)BaseAddress & 0xff);
    
    //
    // Halt HC first
    //
    Register = *(UINT32*)(UINTN)BaseAddress;
    Register &= ~0x01;
    *(UINT32*)(UINTN)BaseAddress = Register;

    MicroSecondDelay (1);
    
    //
    // HCReset
    //
    Register = *(UINT32*)(UINTN)BaseAddress;
    Register |= 0x02;
    *(UINT32*)(UINTN)BaseAddress = Register;
    
    //
    // Get PciCfgPpi at first.
    //
    PciCfgPpi = (**PeiServices).PciCfg;
    
    //
    // Disable SDHC
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->PciCfgAddress | COMMAND_REGISTER,
                 &Register
                 );
    Register &= ~0x06;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->PciCfgAddress | COMMAND_REGISTER,
                 &Register
                 );
    //
    // Disable root bridge
    //
    Register = 0;
    PciCfgPpi->Read (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->RootBridge | COMMAND_REGISTER,
                 &Register
                 );
    Register &= ~0x06;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint16,
                 PeiPchSdhcDev->RootBridge | COMMAND_REGISTER,
                 &Register
                 );
    //
    // Clear bus number for root bridge
    //
    Register = 0;
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 PeiPchSdhcDev->RootBridge | ROOT_BRIDGE_BUS_REGISTER,
                 &Register
                 );
    //
    // Clear address range for root bridge
    //
    PciCfgPpi->Write (
                 PeiServices,
                 PciCfgPpi,
                 EfiPeiPciCfgWidthUint32,
                 PeiPchSdhcDev->RootBridge | ROOT_BRIDGE_ADDRESS_REGISTER,
                 &Register
                 );
  }

  return EFI_SUCCESS;

}

