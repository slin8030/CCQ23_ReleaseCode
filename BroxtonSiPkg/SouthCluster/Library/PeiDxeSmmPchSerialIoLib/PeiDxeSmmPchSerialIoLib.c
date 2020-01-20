/** @file
  PCH Serial IO Lib implementation.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <IndustryStandard/Pci30.h>
#include <ScAccess.h>
#include <ScRegs/RegsLpss.h>
#include <ScRegs/RegsPsf.h>
#include <Library/ScPcrLib.h>
#include <Library/ScInfoLib.h>
#include <Library/MmPciLib.h>
//#include <Library/GpioNativeLib.h>
#include <Library/ScSerialIoLib.h>

typedef struct {
  UINT32 Bar0;
  UINT32 Bar1;
} SERIAL_IO_CONTROLLER_DESCRIPTOR;

typedef struct {
  UINT8  DevNum;
  UINT8  FuncNum;
} SERIAL_IO_BDF_NUMBERS;

typedef struct {
  UINT16 PciDevIdLp;
  CHAR8  AcpiHid[SERIALIO_HID_LENGTH];
} SERIAL_IO_ID;

GLOBAL_REMOVE_IF_UNREFERENCED SERIAL_IO_BDF_NUMBERS mSerialIoBdf [15] =
{
  {PCI_DEVICE_NUMBER_LPSS_I2C0,  PCI_FUNCTION_NUMBER_LPSS_I2C0},
  {PCI_DEVICE_NUMBER_LPSS_I2C0,  PCI_FUNCTION_NUMBER_LPSS_I2C1},
  {PCI_DEVICE_NUMBER_LPSS_I2C0,  PCI_FUNCTION_NUMBER_LPSS_I2C2},
  {PCI_DEVICE_NUMBER_LPSS_I2C0,  PCI_FUNCTION_NUMBER_LPSS_I2C3},
  {PCI_DEVICE_NUMBER_LPSS_I2C1,  PCI_FUNCTION_NUMBER_LPSS_I2C4},
  {PCI_DEVICE_NUMBER_LPSS_I2C1,  PCI_FUNCTION_NUMBER_LPSS_I2C5},
  {PCI_DEVICE_NUMBER_LPSS_I2C1,  PCI_FUNCTION_NUMBER_LPSS_I2C6},
  {PCI_DEVICE_NUMBER_LPSS_I2C1,  PCI_FUNCTION_NUMBER_LPSS_I2C7},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART0},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART1},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART2},
  {PCI_DEVICE_NUMBER_LPSS_HSUART, PCI_FUNCTION_NUMBER_LPSS_HSUART3},
  {PCI_DEVICE_NUMBER_LPSS_SPI,  PCI_FUNCTION_NUMBER_LPSS_SPI0},
  {PCI_DEVICE_NUMBER_LPSS_SPI,  PCI_FUNCTION_NUMBER_LPSS_SPI1},
  {PCI_DEVICE_NUMBER_LPSS_SPI,  PCI_FUNCTION_NUMBER_LPSS_SPI2}
};

GLOBAL_REMOVE_IF_UNREFERENCED SERIAL_IO_CONTROLLER_DESCRIPTOR mSerialIoAcpiAddress [15] =
{
  {LPSS_I2C0_TMP_BAR0 + 0x0000, LPSS_I2C0_TMP_BAR0 + 0x1000},
  {LPSS_I2C0_TMP_BAR0 + 0x10000,LPSS_I2C0_TMP_BAR0 + 0x11000},
  {LPSS_I2C0_TMP_BAR0 + 0x20000,LPSS_I2C0_TMP_BAR0 + 0x21000},
  {LPSS_I2C0_TMP_BAR0 + 0x30000,LPSS_I2C0_TMP_BAR0 + 0x31000},
  {LPSS_I2C0_TMP_BAR0 + 0x40000,LPSS_I2C0_TMP_BAR0 + 0x41000},
  {LPSS_I2C0_TMP_BAR0 + 0x50000,LPSS_I2C0_TMP_BAR0 + 0x51000},
  {LPSS_I2C0_TMP_BAR0 + 0x60000,LPSS_I2C0_TMP_BAR0 + 0x61000},
  {LPSS_I2C0_TMP_BAR0 + 0x70000,LPSS_I2C0_TMP_BAR0 + 0x71000},
  {LPSS_I2C0_TMP_BAR0 + 0x80000,LPSS_I2C0_TMP_BAR0 + 0x81000},
  {LPSS_I2C0_TMP_BAR0 + 0x90000,LPSS_I2C0_TMP_BAR0 + 0x91000},
  {LPSS_I2C0_TMP_BAR0 + 0x100000,LPSS_I2C0_TMP_BAR0 + 0x101000},
  {LPSS_I2C0_TMP_BAR0 + 0x110000,LPSS_I2C0_TMP_BAR0 + 0x111000},
  {LPSS_I2C0_TMP_BAR0 + 0x120000,LPSS_I2C0_TMP_BAR0 + 0x121000},
  {LPSS_I2C0_TMP_BAR0 + 0x130000,LPSS_I2C0_TMP_BAR0 + 0x131000},
  {LPSS_I2C0_TMP_BAR0 + 0x140000,LPSS_I2C0_TMP_BAR0 + 0x141000}
};

typedef struct {
  UINT16 PciCfgCtrAddr;
  UINT16 Psf3BaseAddress;
} SERIAL_IO_CONTROLLER_PSF3_OFFSETS;

GLOBAL_REMOVE_IF_UNREFERENCED SERIAL_IO_CONTROLLER_PSF3_OFFSETS mPchLpSerialIoPsf3Offsets [15] =
{
  { R_LPSS_SB_PCICFGCTRL_I2C0,  R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C0_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C1,  R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C1_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C2,  R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C2_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C3,  R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C3_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C4,  R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C4_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C5,  R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C5_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C6, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C6_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_I2C7, R_PCH_PCR_PSF3_T0_SHDW_LPSS_I2C7_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_UART0, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART0_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_UART1, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART1_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_UART2, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART2_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_UART3, R_PCH_PCR_PSF3_T0_SHDW_LPSS_UART3_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_SPI0, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI0_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_SPI1, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI1_REG_BASE},
  { R_LPSS_SB_PCICFGCTRL_SPI2, R_PCH_PCR_PSF3_T0_SHDW_LPSS_SPI2_REG_BASE},
};

/**
  Returns index of the last i2c controller

  @retval Value           Index of I2C controller
**/
PCH_SERIAL_IO_CONTROLLER
GetMaxI2cNumber (
  VOID
  )
{
  return PchSerialIoIndexI2C7;
}

/**
  Finds PCI Device Number of SerialIo devices.
  SerialIo devices' BDF is configurable

  @param[in] SerialIoDevice             0=I2C0, ..., 11=UART2

  @retval UINT8                         SerialIo device number
**/
UINT8
GetSerialIoDeviceNumber (
  IN PCH_SERIAL_IO_CONTROLLER  SerialIoNumber
  )
{
  return mSerialIoBdf[SerialIoNumber].DevNum;
}

/**
  Finds PCI Function Number of SerialIo devices.
  SerialIo devices' BDF is configurable

  @param[in] SerialIoDevice             0=I2C0, ..., 11=UART2

  @retval UINT8                         SerialIo funciton number
**/
UINT8
GetSerialIoFunctionNumber (
  IN PCH_SERIAL_IO_CONTROLLER  SerialIoNumber
  )
{
  return mSerialIoBdf[SerialIoNumber].FuncNum;
}

/**
  Finds BAR value of SerialIo devices.

  SerialIo devices can be configured to not appear on PCI so traditional method of reading BAR might not work.
  If the SerialIo device is in PCI mode, a request for BAR1 will return its PCI CFG space instead

  @param[in] SerialIoDevice             0=I2C0, ..., 11=UART2
  @param[in] BarNumber                  0=BAR0, 1=BAR1

  @retval UINTN                         SerialIo Bar value
**/
UINTN
FindSerialIoBar (
  IN PCH_SERIAL_IO_CONTROLLER           SerialIoDevice,
  IN UINT8                              BarNumber
  )
{
  UINT32 Bar;
  UINTN  PcieBase;
  UINT32 VenId;

  PcieBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, GetSerialIoDeviceNumber (SerialIoDevice), GetSerialIoFunctionNumber (SerialIoDevice));

  VenId = MmioRead32 (PcieBase + PCI_VENDOR_ID_OFFSET) & 0xFFFF;

  if (VenId == V_INTEL_VENDOR_ID) {
    if (BarNumber == 1) {
      return PcieBase;
    }
    Bar = (MmioRead32 (PcieBase + PCI_BASE_ADDRESSREG_OFFSET + 8*BarNumber) & 0xFFFFF000);
    return Bar;
  }

  return 0xFFFFFFFF;
}

/**
  Configures Serial IO Controller

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] SerialIoSafeRegister       D0i3 Max Power On Latency and Device PG config
**/
VOID
ConfigureSerialIoController (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode
#ifdef PCH_PO_FLAG
  , IN UINT32                 SerialIoSafeRegister
#endif
  )
{
  UINTN                               PciCfgBase;
  UINTN                               Bar;
  UINT32                              Data32;
  SERIAL_IO_CONTROLLER_PSF3_OFFSETS   *SerialIoPsf3Offsets;

  PciCfgBase = MmPciBase (0, GetSerialIoDeviceNumber (Controller), GetSerialIoFunctionNumber (Controller));

  //
  // Do not modify a device that has already been initialized
  //
  if(MmioRead16(PciCfgBase + PCI_VENDOR_ID_OFFSET) != V_INTEL_VENDOR_ID)
  {
    return;
  }

#ifdef PCH_PO_FLAG
  if (!SerialIoSafeRegister) {
#endif
  ///
  /// Step 1. Set Bit 16,17,18.
  ///
  MmioOr32 (PciCfgBase + R_LPSS_IO_D0I3MAXDEVPG, BIT18 | BIT17 | BIT16);
#ifdef PCH_PO_FLAG
  }
#endif

  SerialIoPsf3Offsets = mPchLpSerialIoPsf3Offsets;

  Bar = MmioRead32(PciCfgBase + R_LPSS_IO_BAR) & 0xFFFFF000;

  switch(DeviceMode) {
    case PchSerialIoDisabled:
      ///
      /// Step 1. Put device in D3
      /// Step 2. Function Disable in PSF
      ///
      MmioOr32 (PciCfgBase + R_LPSS_IO_PCS, BIT1 | BIT0);
      PchPcrAndThenOr32 (PID_PSF3, SerialIoPsf3Offsets[Controller].Psf3BaseAddress+R_PCH_PCR_PSFX_T0_SHDW_PCIEN, 0xFFFFFFFF, B_PCH_PCR_PSFX_T0_SHDW_PCIEN_FUNDIS);

      break;

    case PchSerialIoAcpi:
    case PchSerialIoAcpiHidden:
    case PchSerialIoLegacyUart:
      ///
      /// Assign BAR0
      /// Assign BAR1
      ///
      if (Bar == 0) {
        MmioWrite32(PciCfgBase + R_LPSS_IO_BAR, mSerialIoAcpiAddress[Controller].Bar0);
        MmioWrite32(PciCfgBase + R_LPSS_IO_BAR_HIGH, 0x0);
        MmioWrite32(PciCfgBase + R_LPSS_IO_BAR1, mSerialIoAcpiAddress[Controller].Bar1);
        MmioWrite32(PciCfgBase + R_LPSS_IO_BAR1_HIGH, 0x0);

        Bar = MmioRead32(PciCfgBase + R_LPSS_IO_BAR) & 0xFFFFF000;
      }

      MmioWrite32 (Bar + R_LPSS_IO_REMAP_ADDRESS_LOW, Bar);

      ///
      /// Set Memory space Enable
      ///
      MmioOr32 (PciCfgBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);

      ///
      /// Get controller out of reset
      ///
      MmioWrite32 (Bar + R_LPSS_IO_MEM_RESETS, 0);

      MmioWrite32 (Bar + R_LPSS_IO_MEM_RESETS, B_LPSS_IO_MEM_HC_RESET_REL | B_LPSS_IO_MEM_iDMA_RESET_REL);

      break;

    case PchSerialIoPci:

      if (Bar == 0) {
        ///
        /// Disable Bar1
        /// Disable Bar2 in PSF
        ///
        PchPcrAndThenOr32 (LPSS_EP_PORT_ID, SerialIoPsf3Offsets[Controller].PciCfgCtrAddr, 0xFFFFFFFF, B_LPSS_PCICFGCTRL_BAR1_DIS);
        PchPcrAndThenOr32 (PID_PSF3, SerialIoPsf3Offsets[Controller].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN, 0xFFFFFFFF, B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR2DIS);
        //Wait until SB write is complete before writing MMIO mem space enable/ bus master enable due to possible race condition
        do {
          PchPcrRead32(PID_PSF3, SerialIoPsf3Offsets[Controller].Psf3BaseAddress + R_PCH_PCR_PSFX_T0_SHDW_PCIEN, &Data32);
        } while (Data32 & B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR2DIS != B_PCH_PCR_PSFX_T0_SHDW_PCIEN_BAR2DIS);

        ///
        /// Assign BAR0 and Set Memory space Enable
        ///
        MmioWrite32 (PciCfgBase + R_LPSS_IO_BAR,  mSerialIoAcpiAddress[Controller].Bar0);
        MmioWrite32 (PciCfgBase + R_LPSS_IO_BAR_HIGH, 0x0);
        MmioOr32    (PciCfgBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);

        //
        // Read Newly Assigned BAR
        //
        Bar = MmioRead32(PciCfgBase + R_LPSS_IO_BAR) & 0xFFFFF000;
      }

      ///
      /// Update Address Remap Register with Current BAR
      ///
      MmioWrite32(Bar + R_LPSS_IO_REMAP_ADDRESS_LOW, Bar);

      ///
      /// Get controller out of reset
      ///
      MmioWrite32(Bar + R_LPSS_IO_MEM_RESETS, 0);

      MmioOr32 (Bar + R_LPSS_IO_MEM_RESETS, B_LPSS_IO_MEM_HC_RESET_REL | B_LPSS_IO_MEM_iDMA_RESET_REL);
      break;

    default:
      return;
  }

  ///
  /// Program clock dividers for UARTs in legacy mode
  /// Enable Byte addressing for UARTs in legacy mode
  ///
  if (Controller >= PchSerialIoIndexUart0 && Controller <= PchSerialIoIndexUart3) {
      Data32 = B_LPSS_IO_MEM_PCP_CLK_UPDATE | (V_LPSS_IO_PPR_CLK_N_DIV << 16) | (V_LPSS_IO_PPR_CLK_M_DIV << 1) | B_LPSS_IO_MEM_PCP_CLK_EN;
      MmioWrite32 (Bar + R_LPSS_IO_MEM_PCP, Data32);

      if (DeviceMode == PchSerialIoLegacyUart) {
        // Set UART Byte Address Control - Control bit for 16550 8-Bit Addressing Mode.
        PchPcrAndThenOr32(LPSS_EP_PORT_ID, R_LPSS_SB_GPPRVRW7, 0xFFFFFFFF, (B_LPSS_UART0_BYTE_ADDR_EN << (Controller - PchSerialIoIndexUart0)));

        // An MMIO Read Must Immediately Be Issued to UART2 BAR0 + 0xF8 for 8-bit Legacy Mode to Activate
        MmioRead32(Bar + R_LPSS_IO_MANID);
      }
  }

  ///
  /// Program I2C SDA hold registers
  ///
  if (Controller >= PchSerialIoIndexI2C0 && Controller <= GetMaxI2cNumber()) {
    if (DeviceMode != PchSerialIoDisabled) {
      MmioOr32 (mSerialIoAcpiAddress[Controller].Bar0 + R_LPSS_IO_I2C_SDA_HOLD, V_LPSS_IO_I2C_SDA_HOLD_VALUE);
    }
  }

}

#if 0
/**
  Initializes GPIO pins used by SerialIo I2C devices

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] I2cVoltage                 Select I2C voltage, 1.8V or 3.3V
**/
VOID
SerialIoI2cGpioInit (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode,
  IN UINT32                   I2cVoltage
  )
{
  if (DeviceMode == PchSerialIoDisabled) {
    return;
  }
  GpioSetSerialIoI2cPinsIntoNativeMode (Controller);
  if (I2cVoltage == PchSerialIoIs18V) {
    GpioSetSerialIoI2CPinsTolerance (Controller, TRUE);
  }
}

/**
  Initializes GPIO pins used by SerialIo SPI devices

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] SpiCsPolarity              SPI CS polarity
**/
VOID
SerialIoSpiGpioInit (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode,
  IN UINT32                   SpiCsPolarity
  )
{
  if (DeviceMode == PchSerialIoDisabled) {
    return;
  }

  //
  // set Invert Frame Signal before enabling pins to ensure correct initial ChipSelect polarity
  //
  if (SpiCsPolarity == PchSerialIoCsActiveLow) {
    MmioAnd32 (mSerialIoAcpiAddress[Controller].Bar0 + R_PCH_SERIAL_IO_SSCR1, (UINT32)~(B_PCH_SERIAL_IO_SSCR1_IFS));
    MmioOr32 (mSerialIoAcpiAddress[Controller].Bar0 + R_PCH_SERIAL_IO_SPI_CS_CONTROL, B_PCH_SERIAL_IO_SPI_CS_CONTROL_STATE);
  } else {
    MmioOr32 (mSerialIoAcpiAddress[Controller].Bar0 + R_PCH_SERIAL_IO_SSCR1, B_PCH_SERIAL_IO_SSCR1_IFS);
    MmioAnd32 (mSerialIoAcpiAddress[Controller].Bar0 + R_PCH_SERIAL_IO_SPI_CS_CONTROL, (UINT32)~B_PCH_SERIAL_IO_SPI_CS_CONTROL_STATE);
  }

  MmioOr32 (mSerialIoAcpiAddress[Controller].Bar0 + R_PCH_SERIAL_IO_SPI_CS_CONTROL, B_PCH_SERIAL_IO_SPI_CS_CONTROL_MODE);
  GpioSetSerialIoSpiPinsIntoNativeMode (Controller-PchSerialIoIndexSpi0);
}

/**
  Initializes GPIO pins used by SerialIo devices

  @param[in] Controller                 0=I2C0, ..., 11=UART2
  @param[in] DeviceMode                 Different type of serial io mode defined in PCH_SERIAL_IO_MODE
  @param[in] HardwareFlowControl        Hardware flow control method
**/
VOID
SerialIoUartGpioInit (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN PCH_SERIAL_IO_MODE       DeviceMode,
  IN BOOLEAN                  HardwareFlowControl
  )
{
  if (DeviceMode == PchSerialIoDisabled) {
    return;
  }
  GpioSetSerialIoUartPinsIntoNativeMode (Controller-PchSerialIoIndexUart0, HardwareFlowControl);
}

#endif
//[-start-160629-IB07400750-add]//
VOID
SaveRestoreSerialIoPci (
  IN PCH_SERIAL_IO_CONTROLLER SerialIoDevice,
  IN BOOLEAN                  SaveRestore   // TRUE: SAVE, FALSE: RESTORE
  )
{  
  UINT32 Bar;
  UINTN  PcieBase;
  UINT32 VenId;
  UINT16 MmioEnabled = (EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);
    
  STATIC UINT16 TempPcieCmd;
  STATIC UINT32 TempPcieBar0;
  STATIC UINT32 TempPcieBase;

  PcieBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, GetSerialIoDeviceNumber (SerialIoDevice), GetSerialIoFunctionNumber (SerialIoDevice));
  VenId = MmioRead32 (PcieBase + PCI_VENDOR_ID_OFFSET) & 0xFFFF;
  
  if (VenId == V_INTEL_VENDOR_ID) {
    if (SaveRestore) { // SAVE and enable PCI resource
      TempPcieBase = PcieBase;
      TempPcieBar0 = (MmioRead32 (PcieBase + PCI_BASE_ADDRESSREG_OFFSET) & 0xFFFFF000);
      TempPcieCmd  = MmioRead16 (PcieBase + PCI_COMMAND_OFFSET);

      if ((TempPcieBar0 & 0xFFFFFF00) == 0) { 
        // Asign New Bar
        MmioWrite32(PcieBase + PCI_BASE_ADDRESSREG_OFFSET, mSerialIoAcpiAddress[SerialIoDevice].Bar0);
      }
      if (!((TempPcieCmd & MmioEnabled) == MmioEnabled)) { 
        // Enable MMIO
        MmioOr16 (PcieBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);
      }
    } else { // Restore PCI resource
      if (PcieBase == TempPcieBase) {
        MmioWrite32 (PcieBase + PCI_BASE_ADDRESSREG_OFFSET, TempPcieBar0);
        MmioWrite16 (PcieBase + PCI_COMMAND_OFFSET, TempPcieCmd);
      }
    }
  }
}
//[-end-160629-IB07400750-add]//