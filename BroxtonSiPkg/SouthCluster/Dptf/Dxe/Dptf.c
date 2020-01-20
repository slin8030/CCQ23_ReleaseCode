/** @file
  This DXE driver configures and supports DPTF.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Dptf.h"

///
///  Created DPTF Driver to disable DPTF Config Space.
///

/**
  Entry Point for this driver.This procedure does all the DPTF initialization and loads the ACPI tables.

  @param[in] ImageHandle       The firmware allocated handle for the EFI image.
  @param[in] SystemTable       A pointer to the EFI System Table.

  @retval    EFI_SUCCESS       The entry point is executed successfully.
  @retval    other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
DptfDriverEntry (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
#ifdef FSP_FLAG
  UINTN       McD0BaseAddress;
  UINT32      Data32;
  FSPS_UPD    *FspsUpd;

  FspsUpd = (FSPS_UPD *)GetFspSiliconInitUpdDataPointer ();

  if (!FspsUpd->FspsConfig.DptfEnabled) {
    McD0BaseAddress = MmPciAddress (0, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
    DEBUG ((EFI_D_INFO, "DPTF: McD0BaseAddress = 0x%x \n",McD0BaseAddress));
    DEBUG ((EFI_D_INFO, "DPTF: R_SA_MC_CAPID0_A = 0x%x \n",MmioRead32
		(McD0BaseAddress + R_SA_MC_CAPID0_A)));
    if (MmioRead32 (McD0BaseAddress + R_SA_MC_CAPID0_A) & BIT15 ) {
      DEBUG ((EFI_D_INFO,
		"DPTF: Write PSF Reg R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_OFFSET5 "));
      PchPcrAndThenOr32 (PID_PSF1, R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_CSE_BXTP
	  ,(UINT32) ~0, B_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_FUNDIS);
    }

    //
    // PCI CFG space function disable register.
    //
    MmioAnd32 (MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_DEVEN, (UINT32)~BIT1);

    PchPcrAndThenOr32 (PID_PSF1, R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_CSE_BXTP
	,(UINT32) ~0, B_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_FUNDIS);

    //
    // Store the PCI CFG space function disable register for the S3 resume path
    //
    Data32 = MmioRead32 (MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_DEVEN);
    DEBUG ((EFI_D_INFO, "DPTF: PCI CFG space DPTF function disable register = 0x%x \n",Data32));
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (MmPciBase (SA_MC_BUS,SA_MC_DEV,SA_MC_FUN) + R_SA_DEVEN),
      1,
      &Data32
      );

    DEBUG ((EFI_D_INFO, "DPTF: PSF fabric private configuration DPTF function disable register Address = 0x%x \n",
		SC_PCR_BASE_ADDRESS | ((UINT8) (0xA9) << 16) | (UINT16) (0x051C)));
    Data32 = MmioRead32 (SC_PCR_BASE_ADDRESS | ((UINT8)(0xA9) << 16) | (UINT16) (0x051C));
    DEBUG ((EFI_D_INFO,
		"DPTF: PSF fabric private configuration DPTF function disable register Data = 0x%x \n",
			Data32));
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (SC_PCR_BASE_ADDRESS | ((UINT8) (0xA9) << 16) | (UINT16) (0x051C)),
      1,
      &Data32
      );
  }
#endif

  return EFI_SUCCESS;
}