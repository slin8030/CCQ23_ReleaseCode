/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  Clock generator setting for MultiPlatform.

  Copyright (c) 2010 - 2016, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <BoardClkGens.h>
#include <ChipsetSetupConfig.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/BaseMemoryLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/MemoryAllocationLib.h>

//
// For GCC, the optimize option can't put into library
//
#ifndef __GNUC__
#pragma optimize( "", off )
#endif

#define CLKGEN_EN 1
#define EFI_DEBUG 1

CLOCK_GENERATOR_DETAILS   mSupportedClockGeneratorTable[] =
{
  { ClockGeneratorCk410, CK410_GENERATOR_ID , CK410_GENERATOR_SPREAD_SPECTRUM_BYTE, CK410_GENERATOR_SPREAD_SPECTRUM_BIT },
  { ClockGeneratorCk505, CK505_GENERATOR_ID , CK505_GENERATOR_SPREAD_SPECTRUM_BYTE, CK505_GENERATOR_SPREAD_SPECTRUM_BIT }
};

EFI_STATUS
ConfigureClockGenerator (
  IN     EFI_PEI_SERVICES              **PeiServices,
  IN     EFI_PEI_SMBUS_PPI                 *SmbusPpi,
  IN     CLOCK_GENERATOR_TYPE          ClockType,
  IN     UINT8                         ClockAddress,
  IN     UINTN                         ConfigurationTableLength,
  IN OUT UINT8                         *ConfigurationTable
  )
/*++

Routine Description:

  Configure the clock generator using the SMBUS PPI services.

  This function performs a block write, and dumps debug information.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  ClockType                 - Clock generator's model name.
  ClockAddress              - SMBUS address of clock generator.
  ConfigurationTableLength  - Length of configuration table.
  ConfigurationTable        - Pointer of configuration table.

Returns:

  EFI_SUCCESS - Operation success.

--*/
{

  EFI_STATUS                    Status;
  EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress;
  UINT8                         Buffer[MAX_CLOCK_GENERATOR_BUFFER_LENGTH];
  UINTN                         Length;
  EFI_SMBUS_DEVICE_COMMAND      Command;
#if CLKGEN_CONFIG_EXTRA
  UINT8                         j;
#endif
  //
  // Verify input arguments
  //
  ASSERT_EFI_ERROR (ConfigurationTableLength >= 6);
  ASSERT_EFI_ERROR (ConfigurationTableLength <= MAX_CLOCK_GENERATOR_BUFFER_LENGTH);
  ASSERT_EFI_ERROR (ClockType < ClockGeneratorMax);
  ASSERT_EFI_ERROR (ConfigurationTable != NULL);

  //
  // Read the clock generator
  //
  SlaveAddress.SmbusDeviceAddress = ClockAddress >> 1;
  Length = sizeof (Buffer);
  Command = 0;
  Status = SmbusPpi->Execute (
    PeiServices,
    SmbusPpi,
    SlaveAddress,
    Command,
    EfiSmbusReadBlock,
    FALSE,
    &Length,
    Buffer
    );
  ASSERT_EFI_ERROR (Status);

#ifdef EFI_DEBUG
  {
    UINT8 i;
    for (i = 0; i < sizeof (Buffer); i++) {
      DEBUG((EFI_D_ERROR, "CK505 default Clock Generator Byte %d: %x\n", i, Buffer[i]));
    }
#if CLKGEN_EN
    for (i = 0; i < ConfigurationTableLength; i++) {
      DEBUG((EFI_D_ERROR, "BIOS structure Clock Generator Byte %d: %x\n", i, ConfigurationTable[i]));
    }
#endif
  }
#endif

  DEBUG((EFI_D_ERROR, "Expected Clock Generator ID is %x, expecting %x\n", mSupportedClockGeneratorTable[ClockType].ClockId,(Buffer[7]&0xF)));
  //
  // Program clock generator
  //
  Command = 0;
#if CLKGEN_EN
#if CLKGEN_CONFIG_EXTRA
  for (j = 0; j < ConfigurationTableLength; j++) {
    Buffer[j] = ConfigurationTable[j];
  }

  Buffer[30] = 0x00;

  Status = SmbusPpi->Execute (
    PeiServices,
    SmbusPpi,
    SlaveAddress,
    Command,
    EfiSmbusWriteBlock,
    FALSE,
    &Length,
    Buffer
    );
#else
  Status = SmbusPpi->Execute (
    PeiServices,
    SmbusPpi,
    SlaveAddress,
    Command,
    EfiSmbusWriteBlock,
    FALSE,
    &ConfigurationTableLength,
    ConfigurationTable
    );
#endif // CLKGEN_CONFIG_EXTRA
#else
    ConfigurationTable[4] = (ConfigurationTable[4] & 0x3) | (Buffer[4] & 0xFC);
    Command = 4;
    Length = 1;
  Status = SmbusPpi->Execute (
    PeiServices,
    SmbusPpi,
    SlaveAddress,
    Command,
    EfiSmbusWriteBlock,
    FALSE,
    &Length,
    &ConfigurationTable[4]
    );
#endif //CLKGEN_EN
  ASSERT_EFI_ERROR (Status);

//  //
//  // Dump contents after write
//  //
  #ifdef EFI_DEBUG
    {
      UINT8   i;
    SlaveAddress.SmbusDeviceAddress = ClockAddress >> 1;
    Length = sizeof (Buffer);
      Command = 0;
      Status =  SmbusPpi->Execute (
        PeiServices,
        SmbusPpi,
        SlaveAddress,
        Command,
        EfiSmbusReadBlock,
        FALSE,
        &Length,
        Buffer
        );

      for (i = 0; i < ConfigurationTableLength; i++) {
        DEBUG((EFI_D_ERROR, "Clock Generator Byte %d: %x\n", i, Buffer[i]));
      }
    }
    #endif

  return EFI_SUCCESS;
}

UINT8
ReadClockGeneratorID (
  IN     EFI_PEI_SERVICES              **PeiServices,
  IN     EFI_PEI_SMBUS_PPI                 *SmbusPpi,
  IN     UINT8                         ClockAddress
  )
/*++

Routine Description:

  Configure the clock generator using the SMBUS PPI services.

  This function performs a block write, and dumps debug information.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  ClockType                 - Clock generator's model name.
  ClockAddress              - SMBUS address of clock generator.
  ConfigurationTableLength  - Length of configuration table.
  ConfigurationTable        - Pointer of configuration table.

Returns:

  EFI_SUCCESS - Operation success.

--*/
{
  EFI_STATUS                    Status;
  EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress;
  UINT8                         Buffer[MAX_CLOCK_GENERATOR_BUFFER_LENGTH];
  UINTN                         Length;
  EFI_SMBUS_DEVICE_COMMAND      Command;
//  EFI_PEI_SMBUS_PPI             *SmbusPpi;



  //
  // Read the clock generator
  //
  SlaveAddress.SmbusDeviceAddress = ClockAddress >> 1;
  Length = sizeof (Buffer);
  Command = 0;
  Status = SmbusPpi->Execute (
    PeiServices,
    SmbusPpi,
    SlaveAddress,
    Command,
    EfiSmbusReadBlock,
    FALSE,
    &Length,
    Buffer
    );

  //
  // Sanity check that the requested clock type is present in our supported clocks table
  //
  DEBUG((EFI_D_ERROR, "Expected Clock Generator ID is 0x%x\n", Buffer[7]));

  return (Buffer[7]);
}

EFI_STATUS
ConfigurePlatformClocks (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *SmbusPpi
  )
/*++

Routine Description:

  Configure the clock generator to enable free-running operation.  This keeps
  the clocks from being stopped when the system enters C3 or C4.

Arguments:

  None

Returns:

  EFI_SUCCESS    The function completed successfully.

--*/
{
//
// Comment it out for now
// Not supported by Hybrid model.
//
  EFI_STATUS                    Status;
  UINT8                         *ConfigurationTable;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariableServices;
//[-end-160803-IB07220122-add]//

  CLOCK_GENERATOR_TYPE          ClockType = ClockGeneratorCk505;
  UINT8                         ConfigurationTable_Desktop[] = CLOCK_GENERATOR_SETTINGS_DESKTOP;
  UINT8                         ConfigurationTable_Mobile[] = CLOCK_GENERATOR_SETTINGS_MOBILE;
  UINT8                         ConfigurationTable_Tablet[] = CLOCK_GENERATOR_SEETINGS_TABLET;

  EFI_PLATFORM_INFO_HOB         *PlatformInfoHob;
  BOOLEAN                       EnableSpreadSpectrum;
  UINT8                         ClockGenID=0;
  CHIPSET_CONFIGURATION         SystemConfiguration;

  UINTN                         Length;
  EFI_SMBUS_DEVICE_COMMAND      Command;
  EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress;
  UINT8                         Data;

  UINT8                         ClockAddress = CLOCK_GENERATOR_ADDRESS;
  UINTN                         VariableSize;

  //
  // Obtain Platform Info from HOB.
  //
  Status = GetPlatformInfoHob ((CONST EFI_PEI_SERVICES **) PeiServices, &PlatformInfoHob);
  ASSERT_EFI_ERROR (Status);

  DEBUG((EFI_D_ERROR, "PlatformInfo protocol is working in ConfigurePlatformClocks()...%x\n",PlatformInfoHob->PlatformFlavor));

//[-start-160803-IB07220122-add]//
  Status = (*PeiServices)->LocatePpi (
                              PeiServices,
                              &gEfiPeiReadOnlyVariable2PpiGuid,
                              0,
                              NULL,
                              (VOID **) &VariableServices
                              );
//[-end-160803-IB07220122-add]//
//  //
//  // Locate SMBUS PPI
//  //
  Status = (**PeiServices).LocatePpi (
                             (CONST EFI_PEI_SERVICES **) PeiServices,
                             &gEfiPeiSmbusPpiGuid,
                             0,
                             NULL,
                             &SmbusPpi
                             );
  ASSERT_EFI_ERROR (Status);

  Data  = 0;
  SlaveAddress.SmbusDeviceAddress = ClockAddress >> 1;
  Length = 1;
  Command = 0x87;   //Control Register 7 Vendor ID Check
  Status = ((EFI_PEI_SMBUS_PPI *) SmbusPpi)->Execute (
                                               PeiServices,
                                               SmbusPpi,
                                               SlaveAddress,
                                               Command,
                                               EfiSmbusReadByte,
                                               FALSE,
                                               &Length,
                                               &Data
                                               );

  if (EFI_ERROR (Status) || ((Data & 0x0F) != CK505_GENERATOR_ID)) {
      DEBUG((EFI_D_ERROR, "Clock Generator CK505 Not Present, vendor ID on board is %x\n",(Data & 0x0F)));
      return EFI_SUCCESS;
}
  ClockGenID = Data & 0x0F;

  EnableSpreadSpectrum = FALSE;

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
  ZeroMem (&SystemConfiguration, VariableSize);
  
  //
  // SC_PO: Use PlatformConfigDataLib instead of GetVariable.
  //
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SystemConfiguration, &VariableSize);
    Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//
  if(!EFI_ERROR (Status)){
    EnableSpreadSpectrum = SystemConfiguration.EnableClockSpreadSpec;
  }

  //
  // Perform platform-specific intialization dependent upon Board ID:
  //
  DEBUG((EFI_D_ERROR, "Board ID is %x. Platform ID is %x\n",PlatformInfoHob->BoardId,PlatformInfoHob->PlatformFlavor));


  switch (PlatformInfoHob->BoardId) {
    case BOARD_ID_BL_RVP:
    case BOARD_ID_BL_FFRD:
    case BOARD_ID_BB_RVP:
    case BOARD_ID_AV_SVP:
    default:
      switch(PlatformInfoHob->PlatformFlavor) {
      case FlavorTablet:
        ConfigurationTable = ConfigurationTable_Tablet;
        Length = sizeof (ConfigurationTable_Tablet);
        break;
      case FlavorMobile:
        ConfigurationTable = ConfigurationTable_Mobile;
        Length = sizeof (ConfigurationTable_Mobile);
        break;
      case FlavorDesktop:
      default:
        ConfigurationTable = ConfigurationTable_Desktop;
        Length = sizeof (ConfigurationTable_Desktop);
        break;
      }
    break;
    }

  //
  // Perform common clock initialization:
  //
  // Program Spread Spectrum function.
  //

  if (EnableSpreadSpectrum)
  {
    ConfigurationTable[mSupportedClockGeneratorTable[ClockType].SpreadSpectrumByteOffset] |= mSupportedClockGeneratorTable[ClockType].SpreadSpectrumBitOffset;
  } else {
    ConfigurationTable[mSupportedClockGeneratorTable[ClockType].SpreadSpectrumByteOffset] &= ~(mSupportedClockGeneratorTable[ClockType].SpreadSpectrumBitOffset);
  }

  //pre-silicon start
  if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
#if CLKGEN_EN
    Status = ConfigureClockGenerator (PeiServices, SmbusPpi, ClockType, ClockAddress, Length, ConfigurationTable);
    ASSERT_EFI_ERROR (Status);
#endif // CLKGEN_EN
  }//pre-silicon end

  return EFI_SUCCESS;
}


//static EFI_PEI_NOTIFY_DESCRIPTOR    mNotifyList= {
//    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
//    &gEfiPeiSmbusPpiGuid,
//    ConfigurePlatformClocks
//};
static EFI_PEI_NOTIFY_DESCRIPTOR    mNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK| EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiPeiSmbusPpiGuid,
    ConfigurePlatformClocks
  }
};

EFI_STATUS
InstallPlatformClocksNotify (
  IN CONST EFI_PEI_SERVICES           **PeiServices
  )
{
  EFI_STATUS                    Status;

  DEBUG ((EFI_D_INFO, "InstallPlatformClocksNotify()...\n"));

  Status = (*PeiServices)->NotifyPpi(PeiServices, &mNotifyList[0]);
  ASSERT_EFI_ERROR (Status);
  return EFI_SUCCESS;

}

#ifndef __GNUC__
#pragma optimize( "", on )
#endif
