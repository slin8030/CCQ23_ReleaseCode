/** @file
  This file contains the tests for the BootFirmwareMediaProtection BIT

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include "HstiSiliconDxe.h"

/**
  Run tests for BootFirmwareMediaProtection bit
**/
VOID
CheckBootFirmwareMediaProtection (
  VOID
  )
{
  EFI_STATUS      Status;
  BOOLEAN         Result;
  UINTN           SpiBase;
  UINTN           SpiBAR0;
  UINTN           LpcBase;
  UINT32          LpcBC;
  UINT8           BiosControl;
  UINT16          Hsfs;
  UINT32          Frap;
  UINT32          Data32;
  CHAR16          *HstiErrorString;

  if ((mFeatureImplemented[0] & HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION) == 0) {
    return;
  }

  Result = TRUE;

  LpcBase = MmPciBase(DEFAULT_PCI_BUS_NUMBER_SC,PCI_DEVICE_NUMBER_PCH_LPC,PCI_FUNCTION_NUMBER_PCH_LPC);
  SpiBase = MmPciBase(DEFAULT_PCI_BUS_NUMBER_SC,PCI_DEVICE_NUMBER_SPI,PCI_FUNCTION_NUMBER_SPI);
  SpiBAR0 = MmioRead32(SpiBase + R_SPI_BASE) & B_SPI_BASE_BAR;

  DEBUG((DEBUG_INFO, "  Table 3-5. SPI Flash and BIOS Security Configuration\n"));
  DEBUG((DEBUG_INFO, "    1. BIOS write-protection\n"));
  //
  //Set SMM_BWP, WPD and LE bit
  //
  BiosControl = MmioRead8(SpiBase + R_SPI_BCR);
  DEBUG((DEBUG_INFO, "   SpiBase = %x\n", SpiBase));
  DEBUG((DEBUG_INFO, "   BiosControl = %x\n", BiosControl));

  if (((BiosControl & B_SPI_BCR_BLE) == 0) ||
    ((BiosControl & B_SPI_BCR_BIOSWE) != 0) ||
    ((BiosControl & B_SPI_BCR_SMM_BWP) == 0)){

    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_1, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_1);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    2. SPI flash descriptor security override pin-strap\n"));
  Hsfs = MmioRead16(SpiBAR0 + R_SPI_HSFS);
  if ((Hsfs & B_SPI_HSFS_FDOPSS) == 0) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_2, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_2);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    3. SPI controller configuration is locked\n"));
  if ((Hsfs & B_SPI_HSFS_FLOCKDN) == 0) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_3, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_3);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    4. BIOS Interface Lock\n"));
  LpcBC = MmioRead32(LpcBase + R_PCH_LPC_BC);

  if ((LpcBC & B_PCH_LPC_BC_BILD) == 0) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_4, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_4);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    5. BIOS Top Swap Mode\n"));

  if ((BiosControl & B_PCH_LPC_BC_TS) != 0) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_5, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_5);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    6. SEC Firmware Status\n"));

  Data32 = PciRead32(PCI_LIB_ADDRESS(0, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, 0x40));
  DEBUG((DEBUG_INFO, "   SEC_FW_STS = %x\n", Data32));
  Data32 &= 0x0000000F; //Check only the lower nibble [3:0] should indicate that SEC FW is in NORMAL WORKING STATE
  if (Data32 != SEC_STATE_NORMAL) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_8, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_8);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    7. BIOS Flash Descriptor Valid\n"));

  if ((Hsfs & B_SPI_HSFS_FDV) == 0) {
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_9, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_9);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  Frap = MmioRead32(SpiBAR0 + R_SPI_FRAP);
  DEBUG((DEBUG_INFO, "   Frap = %x\n", Frap));

  DEBUG ((DEBUG_INFO, "    8. BIOS Region Flash Write Access\n"));

  if (((Frap & B_SPI_FRAP_BRWA_MASK)>>8) != (B_SPI_FRAP_BRWA_SETTING)) {
    DEBUG ((DEBUG_INFO, "    8. BIOS Region Flash Write Access - Error\n"));
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_A ,HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_A);
    Status = HstiLibAppendErrorString (
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR (Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG ((DEBUG_INFO, "    9. BIOS Region Flash Read Access\n"));

  if ((Frap & B_SPI_FRAP_BRRA_MASK) != B_SPI_FRAP_BRRA_SETTING) {
    DEBUG ((DEBUG_INFO, "    9. BIOS Region Flash Read Access - Error\n"));
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_A ,HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_A);
    Status = HstiLibAppendErrorString (
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR (Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }
  DEBUG((DEBUG_INFO, "    10. BIOS Master Read Access\n"));

  if (((Frap & B_SPI_FRAP_BMRAG_MASK)>>16) != 0) {
    DEBUG((DEBUG_INFO, "   10. BIOS Master Read Access - Error\n"));
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_A, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_A);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  DEBUG((DEBUG_INFO, "    11. BIOS Master Write Access\n"));

  if (((Frap & B_SPI_FRAP_BMWAG_MASK)>>24) != 0) {
    DEBUG((DEBUG_INFO, "    11. BIOS Master Write Access\n"));
    HstiErrorString = BuildHstiErrorString(HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_CODE_A, HSTI_SPI_FLASH_CONFIGURATION, HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION_ERROR_STRING_A);
    Status = HstiLibAppendErrorString(
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               HstiErrorString
               );
    ASSERT_EFI_ERROR(Status);
    Result = FALSE;
    FreePool(HstiErrorString);
  }

  //
  // ALL PASS
  //
  if (Result) {
    Status = HstiLibSetFeaturesVerified (
               PLATFORM_SECURITY_ROLE_PLATFORM_REFERENCE,
               NULL,
               0,
               HSTI_BYTE0_BOOT_FIRMWARE_MEDIA_PROTECTION
               );
    ASSERT_EFI_ERROR (Status);
  }
  return;
}
