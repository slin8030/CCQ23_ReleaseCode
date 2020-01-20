/** @file
  Library classes for enabling/disabling flash write access

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/FdSupportLib.h>
#include <Library/IrsiRegistrationLib.h>
#include <ScAccess.h>
#include <CpuRegs.h>
extern UINT8* mSpiBase;
EFI_STATUS
EnableFvbWrites (
  BOOLEAN EnableWrites
  )
{
  STATIC BOOLEAN     DisableCache = FALSE;
  UINT64             MsrValue;


  if (EnableWrites) {
    //
    // Disable cache when write enable
    //
    if (!DisableCache) {
      AsmDisableCache();
      MsrValue = AsmReadMsr64 (CACHE_IA32_MTRR_DEF_TYPE);
      MsrValue = MsrValue & ~B_CACHE_MTRR_VALID;
      AsmWriteMsr64 (CACHE_IA32_MTRR_DEF_TYPE, MsrValue);
      DisableCache = TRUE;
    }
    //
    // Enable flash writing
    //
//[-start-151124-IB08450330-modify]//
    MmioOr32 (
            MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_SC,
                PCI_DEVICE_NUMBER_SPI,
                PCI_FUNCTION_NUMBER_SPI,
                R_SPI_BCR
                ),
            B_SPI_BCR_BIOSWE
            );
    MmioAndThenOr32 (
            MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_SC,
                PCI_DEVICE_NUMBER_SPI,
                PCI_FUNCTION_NUMBER_SPI,
                R_SPI_BCR
                ),
            (UINT32)(~B_SPI_BCR_SRC),
            V_SPI_BCR_SRC_PREF_DIS_CACHE_DIS
            );
//[-end-151124-IB08450330-modify]//
  } else {
    //
    // Disable cache when write disable
    //
    if (DisableCache) {
      AsmEnableCache();
      MsrValue = AsmReadMsr64 (CACHE_IA32_MTRR_DEF_TYPE);
      MsrValue = MsrValue  | B_CACHE_MTRR_VALID;
      AsmWriteMsr64 (CACHE_IA32_MTRR_DEF_TYPE, MsrValue);
      DisableCache = FALSE;
    }
    //
    // Disable flash writing
    //
//[-start-151124-IB08450330-modify]//
    MmioAnd32 (
            MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_SC,
                PCI_DEVICE_NUMBER_SPI,
                PCI_FUNCTION_NUMBER_SPI,
                R_SPI_BCR
                ),
            (UINT32)(~B_SPI_BCR_BIOSWE)
            );
    MmioAndThenOr32 (
            MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_SC,
                PCI_DEVICE_NUMBER_SPI,
                PCI_FUNCTION_NUMBER_SPI,
                R_SPI_BCR
                ),
            (UINT32)(~B_SPI_BCR_SRC),
            V_SPI_BCR_SRC_PREF_EN_CACHE_EN            
            );
//[-end-151124-IB08450330-modify]//
  }
  
  return EFI_SUCCESS;
}


/**
  Enable flash device write access

  @param  FlashDevice           Pointer to the FLASH_DEVICE structure

  @retval EFI_SUCCESS           Flash device write access was successfully enabled
  @return Others                Failed to enable flash device write access

**/
EFI_STATUS
EFIAPI
FlashWriteEnable (
  IN FLASH_DEVICE               *FlashDevice
  )
{
  return EnableFvbWrites(TRUE);
  //return EFI_SUCCESS;
}

/**
  Disable flash device write access

  @param  FlashDevice           Pointer to the FLASH_DEVICE structure

  @retval EFI_SUCCESS           Flash device write access was successfully disabled
  @return Others                Failed to disable flash device write access

**/
EFI_STATUS
EFIAPI
FlashWriteDisable (
  IN FLASH_DEVICE               *FlashDevice
  )
{
  return EnableFvbWrites (FALSE);
  //return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FlashWriteEnableLibInit (
  VOID
  )
{
  //IrsiAddVirtualPointer ((VOID **)&mBiosWriteEnableMmio);
  return EFI_SUCCESS;
}

