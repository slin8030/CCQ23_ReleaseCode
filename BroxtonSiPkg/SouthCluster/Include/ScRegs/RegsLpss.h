/** @file
  Register names for Low Power Sub System (LPSS) module.

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, SC registers are denoted by "_PCH_" in register names
  - Registers / bits that are different between SC generations are denoted by
    "_PCH_<generation_name>_" in register/bit names. e.g., "_PCH_CHV_"
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SC generation will be just named
    as "_PCH_" without <generation_name> inserted.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

@par Specification
**/
#ifndef _REGS_LPSS_H_
#define _REGS_LPSS_H_


///
///Low Power Input Output (LPSS) Module Registers
///
///Define LPSS IO Devices Generic PCI Registers
///

#define R_LPSS_IO_DEVVENDID                  0x00 ///< Device ID & Vendor ID
#define B_LPSS_IO_DEVVENDID_DID              0xFFFF0000 ///< Device ID
#define B_LPSS_IO_DEVVENDID_VID              0x0000FFFF ///< Vendor ID

#define R_LPSS_IO_STSCMD                     0x04  ///< Status & Command
#define B_LPSS_IO_STSCMD_SSE                 BIT30 ///< Signaled System Error
#define B_LPSS_IO_STSCMD_RMA                 BIT29 ///< Received Master Abort
#define B_LPSS_IO_STSCMD_RTA                 BIT28 ///< Received Target Abort
#define B_LPSS_IO_STSCMD_STA                 BIT27 ///< Signaled Target Abort
#define B_LPSS_IO_STSCMD_CAPLIST             BIT20 ///< Capability List
#define B_LPSS_IO_STSCMD_INTRSTS             BIT19 ///< Interrupt Status
#define B_LPSS_IO_STSCMD_INTRDIS             BIT10 ///< Interrupt Disable
#define B_LPSS_IO_STSCMD_SERREN              BIT8  ///< SERR# Enable
#define B_LPSS_IO_STSCMD_BME                 BIT2  ///< Bus Master Enable
#define B_LPSS_IO_STSCMD_MSE                 BIT1  ///< Memory Space Enable

#define R_LPSS_IO_REVCC                      0x08  ///< Revision ID & Class Code
#define B_LPSS_IO_REVCC_CC                   0xFFFFFF00 ///< Class Code
#define B_LPSS_IO_REVCC_RID                  0x000000FF ///< Revision ID

#define R_LPSS_IO_CLHB                       0x0C
#define B_LPSS_IO_CLHB_MULFNDEV              BIT23 ///< Multi Function Device
#define B_LPSS_IO_CLHB_HT                    0x007F0000 ///< Header Type
#define B_LPSS_IO_CLHB_LT                    0x0000FF00 ///< Latency Timer
#define B_LPSS_IO_CLHB_CLS                   0x000000FF ///< Cache Line Size

#define R_LPSS_IO_BAR                        0x10  ///< BAR0 Low
#define R_LPSS_IO_BAR_HIGH                   0x14  ///< BAR0 High
#define B_LPSS_IO_BAR_BA                     0xFFFFF000 ///< Base Address
#define V_LPSS_IO_BAR_SIZE                   0x1000
#define N_LPSS_IO_BAR_ALIGNMENT              12
#define B_LPSS_IO_BAR_SI                     0x00000FF0 ///< Size Indicator
#define B_LPSS_IO_BAR_PF                     BIT3  ///< Prefetchable
#define B_LPSS_IO_BAR_TYPE                   (BIT2 | BIT1) ///< Type
#define B_LPSS_IO_BAR_MS                     BIT0  ///< Message Space

#define R_LPSS_IO_BAR1                       0x18  ///< BAR1 Low
#define R_LPSS_IO_BAR1_HIGH                  0x1C  ///< BAR1 High
#define B_LPSS_IO_BAR1_BA                    0xFFFFF000 ///< Base Address
#define V_LPSS_IO_BAR1_SIZE                  0x1000
#define B_LPSS_IO_BAR1_SI                    0x00000FF0 ///< Size Indicator
#define B_LPSS_IO_BAR1_PF                    BIT3  ///< Prefetchable
#define B_LPSS_IO_BAR1_TYPE                  (BIT2 | BIT1) ///< Type
#define B_LPSS_IO_BAR1_MS                    BIT0  ///< Message Space

#define R_LPSS_IO_SSID                       0x2C  ///< Sub System ID
#define B_LPSS_IO_SSID_SID                   0xFFFF0000 ///< Sub System ID
#define B_LPSS_IO_SSID_SVID                  0x0000FFFF ///< Sub System Vendor ID

#define R_LPSS_IO_ERBAR                      0x30  ///< Expansion ROM BAR
#define B_LPSS_IO_ERBAR_BA                   0xFFFFFFFF ///< Expansion ROM Base Address

#define R_LPSS_IO_CAPPTR                     0x34  ///< Capability Pointer
#define B_LPSS_IO_CAPPTR_CPPWR               0xFF  ///< Capability Pointer Power

#define R_LPSS_IO_INTR                       0x3C  ///< Interrupt
#define B_LPSS_IO_INTR_ML                    0xFF000000 ///< Max Latency
#define B_LPSS_IO_INTR_MG                    0x00FF0000
#define B_LPSS_IO_INTR_IP                    0x00000F00 ///< Interrupt Pin
#define B_LPSS_IO_INTR_IL                    0x000000FF ///< Interrupt Line

#define R_LPSS_IO_PCAPID                     0x80  ///< Power Capability ID
#define B_LPSS_IO_PCAPID_PS                  0xF8000000 ///< PME Support
#define B_LPSS_IO_PCAPID_VS                  0x00070000 ///< Version
#define B_LPSS_IO_PCAPID_NC                  0x0000FF00 ///< Next Capability
#define B_LPSS_IO_PCAPID_PC                  0x000000FF ///< Power Capability

#define R_LPSS_IO_PCS                        0x84  ///< PME Control Status
#define B_LPSS_IO_PCS_PMESTS                 BIT15 ///< PME Status
#define B_LPSS_IO_PCS_PMEEN                  BIT8  ///< PME Enable
#define B_LPSS_IO_PCS_NSS                    BIT3  ///< No Soft Reset
#define B_LPSS_IO_PCS_PS                     (BIT1 | BIT0) ///< Power State

#define R_LPSS_IO_MANID                      0xF8  ///< Manufacturer ID
#define B_LPSS_IO_MANID_MANID                0xFFFFFFFF ///< Manufacturer ID

#define R_LPSS_IO_D0I3MAXDEVPG               0x0A0 ///< D0i3 Max Power On Latency and Device PG config


///
///LPSS IO Device Generic MMIO Register
///MMIO Registers (BAR0 + Offset)
///
#define R_LPSS_IO_MEM_PCP                 0x200        ///< Private Clock Parameters
#define B_LPSS_IO_MEM_PCP_CLK_UPDATE      BIT31        ///< Clock Divider Update
#define B_LPSS_IO_MEM_PCP_N_VAL           0x7FFF0000   ///< N value for the M over N divider
#define B_LPSS_IO_MEM_PCP_M_VAL           0x0000FFFE   ///< M value for the M over N divider
#define B_LPSS_IO_MEM_PCP_CLK_EN          BIT0         ///< Clock Enable
#define V_LPSS_IO_PPR_CLK_M_DIV           0x120
#define V_LPSS_IO_PPR_CLK_N_DIV           0x3D09

#define R_LPSS_IO_MEM_RESETS              0x204        ///< Software Reset
#define B_LPSS_IO_MEM_HC_RESET_REL        (BIT0|BIT1)  ///< LPSS IO Host Controller Reset Release
#define B_LPSS_IO_MEM_iDMA_RESET_REL      BIT2         ///< iDMA Reset Release

#define R_LPSS_IO_ACTIVELTR               0x210
 #define B_LPSS_IO_ACTIVELTR_LATENCY      BIT11        ///< When the LTR registers are under Platform/HW default control, the value need set to 0x800

#define R_LPSS_IO_IDLELTR         0x214
 #define B_LPSS_IO_IDLELTR_LATENCY        BIT11        ///< When the LTR registers are under Platform/HW default control, the value need set to 0x800

#define R_LPSS_IO_REMAP_ADDRESS_LOW       0x240        ///< Low 32 bits of BAR address read by SW from BAR Low CFG Offset 0x10
#define R_LPSS_IO_REMAP_ADDRESS_HI        0x244        ///< High 32 bits of BAR address read by SW from BAR High CFG Offset 0x14

#define R_LPSS_IO_DEV_IDLE_CTRL           0x24C        ///< DevIdle Control per LPSS slice


///
/// LPSS I2C Module
/// PCI Config Space Registers
///
#define PCI_DEVICE_NUMBER_LPSS_I2C0             22
  #define PCI_FUNCTION_NUMBER_LPSS_I2C0         0
  #define PCI_FUNCTION_NUMBER_LPSS_I2C1         1
  #define PCI_FUNCTION_NUMBER_LPSS_I2C2         2
  #define PCI_FUNCTION_NUMBER_LPSS_I2C3         3

#define PCI_DEVICE_NUMBER_LPSS_I2C1             23
  #define PCI_FUNCTION_NUMBER_LPSS_I2C4         0
  #define PCI_FUNCTION_NUMBER_LPSS_I2C5         1
  #define PCI_FUNCTION_NUMBER_LPSS_I2C6         2
  #define PCI_FUNCTION_NUMBER_LPSS_I2C7         3

#define LPSS_I2C_TMP_BAR0_DELTA         0x10000   ///< Increasement for each I2C device BAR0
#define LPSS_I2C_TMP_BAR1_OFFSET        0x8000    ///< Offset from I2C BAR0 to BAR1
/*  #define LPSS_TMP_BAR_I2C1       0xFE920000
  #define LPSS_TMP_BAR_I2C2         0xFE930000
  #define LPSS_TMP_BAR_I2C3         0xFE940000
  #define LPSS_TMP_BAR_I2C4         0xFE950000
  #define LPSS_TMP_BAR_I2C5         0xFE960000
  #define LPSS_TMP_BAR_I2C6         0xFE970000
  #define LPSS_TMP_BAR_I2C7         0xFE980000
*/

#define R_LPSS_I2C_IC_ENABLE            0x6C
 #define B_LPSS_I2C_IC_ENABLE_ENABLE    BIT0

///
/// LPSS HSUART Modules
/// PCI Config Space Registers
///
#define PCI_DEVICE_NUMBER_LPSS_HSUART         24
#define PCI_FUNCTION_NUMBER_LPSS_HSUART0      0
#define PCI_FUNCTION_NUMBER_LPSS_HSUART1      1
#define PCI_FUNCTION_NUMBER_LPSS_HSUART2      2
#define PCI_FUNCTION_NUMBER_LPSS_HSUART3      3

///
/// LPSS UART MMIO Registers
///
#define R_LPSS_UART_MEM_DLL         0x000 ///< DLAB Divisor Latch (Low) Register.  DLL mode is only available when LCR register [7] (DLAB bit) = 1.
#define R_LPSS_UART_MEM_DLH         0x004 ///< DLAB Divisor Latch (High) Register.  DLH mode is only available when LCR register [7] (DLAB bit) = 1.

#define R_LPSS_UART_MEM_LCR         0x00C
 #define B_LPSS_UART_MEM_LCR_DLAB   BIT7  /// DLAB (Divisor Latch Access Bit) is used to enable reading and writing of the Divisor Latch register
                                          /// (DLL and DLH) to set the baud rate of the UART. This bit must be cleared after initialbaud rate setup
                                          /// in order to access other registers.

///
/// LPSS SPI Module
/// PCI Config Space Registers
///
#define PCI_DEVICE_NUMBER_LPSS_SPI            25
#define PCI_FUNCTION_NUMBER_LPSS_SPI0         0
#define PCI_FUNCTION_NUMBER_LPSS_SPI1         1
#define PCI_FUNCTION_NUMBER_LPSS_SPI2         2

#define R_LPSS_SPI_MEM_SSP_CONTROL0             0x000
 #define B_LPSS_SPI_MEM_SSP_CONTROL0_SSE        BIT7

#define R_LPSS_SPI_MEM_CS_CONTROL               0x224
 #define B_LPSS_SPI_MEM_CS_CTRL_CS0_POLARITY    BIT12
 #define B_LPSS_SPI_MEM_CS_CTRL_CS1_POLARITY    BIT13
 #define B_LPSS_SPI_MEM_CS_CTRL_CS2_POLARITY    BIT14
 #define B_LPSS_SPI_MEM_CS_CTRL_CS3_POLARITY    BIT15

///
/// LPSS Bridge Private Space (Access by Sideband Msg)
///
#define LPSS_EP_PORT_ID                    0x90  ///< LPSS Private Space Port ID

#define R_LPSS_PCICFGCTRLx                 0x200
#define R_LPSS_PCICFGCTRL_N_OFFS           0x04

#define R_LPSS_SB_PMCTL                    0x1D0  ///< PM Control Register for bridge
 #define B_IOSFPRIMCLK_GATE_EN             BIT0
 #define B_OCPCLK_GATE_EN                  BIT1
 #define B_OCPCLK_TRUNK_GATE_EN            BIT2
 #define B_IOSFSBCLK_GATE_EN               BIT3
 #define B_IOSFPRIM_TRUNK_GATE_EN          BIT4
 #define B_IOSFSB_TRUNK_GATE_EN            BIT5

#define R_LPSS_SB_PCICFGCTRL_I2C0           0x200 ///<I2C0
#define R_LPSS_SB_PCICFGCTRL_I2C1           0x204 ///<I2C1
#define R_LPSS_SB_PCICFGCTRL_I2C2           0x208 ///<I2C2
#define R_LPSS_SB_PCICFGCTRL_I2C3           0x20C ///<I2C3
#define R_LPSS_SB_PCICFGCTRL_I2C4           0x210 ///<I2C4
#define R_LPSS_SB_PCICFGCTRL_I2C5           0x214 ///<I2C5
#define R_LPSS_SB_PCICFGCTRL_I2C6           0x218 ///<I2C6
#define R_LPSS_SB_PCICFGCTRL_I2C7           0x21C ///<I2C7

#define R_LPSS_SB_PCICFGCTRL_UART0          0x220 ///<UART0
#define R_LPSS_SB_PCICFGCTRL_UART1          0x224 ///<UART1
#define R_LPSS_SB_PCICFGCTRL_UART2          0x228 ///<UART2
#define R_LPSS_SB_PCICFGCTRL_UART3          0x22C ///<UART3

#define R_LPSS_SB_PCICFGCTRL_SPI0           0x230 ///<SPI0
#define R_LPSS_SB_PCICFGCTRL_SPI1           0x234 ///<SPI1
#define R_LPSS_SB_PCICFGCTRL_SPI2           0x238 ///<SPI2

 #define B_LPSS_PCICFGCTRL_PCI_CFG_DIS      BIT0
 #define B_LPSS_ACPI_INTR_EN                BIT1
 #define B_LPSS_PCICFGCTRL_BAR1_DIS         BIT7

 #define R_LPSS_IO_I2C_SDA_HOLD              0x7C
 #define V_LPSS_IO_I2C_SDA_HOLD_VALUE        0x002C002C
 #define B_LPSS_PCICFGCTRL_INT_PIN_MASK      (BIT8 | BIT9 | BIT10 | BIT11) ///< Interrupt Pin
 #define V_LPSS_PCICFGCTRL_INT_PIN_NO_INT    0 ///< No Interrupt
 #define V_LPSS_PCICFGCTRL_INT_PIN_A         BIT8 ///< INTA
 #define V_LPSS_PCICFGCTRL_INT_PIN_B         BIT9 ///< INTB
 #define V_LPSS_PCICFGCTRL_INT_PIN_C         (BIT8|BIT9) ///< INTC
 #define V_LPSS_PCICFGCTRL_INT_PIN_D         BIT10 ///< INTD

#define R_LPSS_SB_GPPRVRW2                   0x604 ///< PGCB_CDC_CFG0 - PGCB and CDC Configuration 0
 #define B_LPSS_PGCB_FRC_CLK_CP_EN           BIT1
 #define B_LPSS_CDC_SIDE_CFG_CG_EN           BIT5
 #define B_LPSS_CDC_SIDE_CFG_CLKREQ_CTL_EN   BIT11

#define R_LPSS_SB_GPPRVRW4                   0x60C ///<I2C Clock Gate Control
#define B_LPSS_I2C_CG                        0xFF

#define R_LPSS_SB_GPPRVRW5                   0x610 ///<UART Clock Gate Control
#define B_LPSS_UART_CG                       0xF

#define R_LPSS_SB_GPPRVRW6                   0x614 ///<SPI Clock Gate Control
#define B_LPSS_SPI_CG                        0x7

#define R_LPSS_SB_GPPRVRW7                   0x618 ///<UART Byte Address Control - Control bit for 16550 8-Bit Addressing Mode.
#define B_LPSS_UART0_BYTE_ADDR_EN            BIT0
#define B_LPSS_UART2_BYTE_ADDR_EN            BIT2

#endif

