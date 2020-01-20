/** @file
  Register names for Smbus Device.

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, SC registers are denoted by "_SC_" in register names
  - Registers / bits that are different between SC generations are denoted by
    "_SC_<generation_name>_" in register/bit names.
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SC generation will be just named
    as "_SC_" without <generation_name> inserted.

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

@par Specification Reference:
**/
#ifndef _REGS_SMBUS_H_
#define _REGS_SMBUS_H_

///
/// SMBus Controller Registers (D31:F1)
///
#define PCI_DEVICE_NUMBER_SMBUS        31
#define PCI_FUNCTION_NUMBER_SMBUS      1

#define R_SMBUS_VENDOR_ID              0x00  ///< Vendor ID
#define V_SMBUS_VENDOR_ID              V_INTEL_VENDOR_ID ///< Intel Vendor ID

#define R_SMBUS_DEVICE_ID              0x02  ///< Device ID
#define V_SMBUS_DEVICE_ID              0x5AD4

#define R_SMBUS_PCICMD                 0x04  ///< CMD register enables/disables, Memory/IO space access and interrupt
#define B_SMBUS_PCICMD_INTR_DIS        BIT10 ///< Interrupt Disable
#define B_SMBUS_PCICMD_FBE             BIT9  ///< FBE - reserved as '0'
#define B_SMBUS_PCICMD_SERR_EN         BIT8  ///< SERR Enable - reserved as '0'
#define B_SMBUS_PCICMD_WCC             BIT7  ///< Wait Cycle Control - reserved as '0'
#define B_SMBUS_PCICMD_PER             BIT6  ///< Parity Error - reserved as '0'
#define B_SMBUS_PCICMD_VPS             BIT5  ///< VGA Palette Snoop - reserved as '0'
#define B_SMBUS_PCICMD_PMWE            BIT4  ///< Postable Memory Write Enable - reserved as '0'
#define B_SMBUS_PCICMD_SCE             BIT3  ///< Special Cycle Enable - reserved as '0'
#define B_SMBUS_PCICMD_BME             BIT2  ///< Bus Master Enable - reserved as '0'
#define B_SMBUS_PCICMD_MSE             BIT1  ///< Memory Space Enable
#define B_SMBUS_PCICMD_IOSE            BIT0  ///< I/O Space Enable

#define R_SMBUS_PCISTS                 0x06  ///< Configuration status register
#define B_SMBUS_PCISTS_DPE             BIT15 ///< Detect Parity Error - reserved as '0'
#define B_SMBUS_PCISTS_SSE             BIT14 ///< Signaled System Error - reserved as '0'
#define B_SMBUS_PCISTS_RMA             BIT13 ///< Received Master Abort - reserved as '0'
#define B_SMBUS_PCISTS_RTA             BIT12 ///< Received Target Abort - reserved as '0'
#define B_SMBUS_PCISTS_STA             BIT11 ///< Signaled Target Abort - reserved as '0'
#define B_SMBUS_PCISTS_DEVT            (BIT10 | BIT9) ///< Devsel Timing Status
#define B_SMBUS_PCISTS_DPED            BIT8  ///< Data Parity Error Detected - reserved as '0'
#define B_SMBUS_PCISTS_FB2BC           BIT7  ///< Fast Back To Back Capable - reserved as '1'
#define B_SMBUS_PCISTS_UDF             BIT6  ///< User Defined Features - reserved as '0'
#define B_SMBUS_PCISTS_66MHZ_CAP       BIT5  ///< 66 MHz Capable - reserved as '0'
#define B_SMBUS_PCISTS_CAP_LIST        BIT4  ///< Capabilities List Indicator - reserved as '0'
#define B_SMBUS_PCISTS_INTS            BIT3  ///< Interrupt Status

#define R_SMBUS_RID                    0x08  ///< Revision ID
#define B_SMBUS_RID                    0xFF  ///< Revision ID

#define R_SMBUS_PRGIF                  0x09  ///< Programming Interface
#define B_SMBUS_PRGIF                  0xFF  ///< Programming Interface

#define R_SMBUS_SCC                    0x0A  ///< Sub Class Code
#define V_SMBUS_SCC                    0x05  ///< A value of 05h indicates that this device is a SM Bus serial controller

#define R_SMBUS_BCC                    0x0B  ///< Base Class Code
#define V_SMBUS_BCC                    0x0C  ///< A value of 0Ch indicates that this device is a serial controller

#define R_SMBUS_BAR0                   0x10  ///< The memory bar low
#define B_SMBUS_BAR0_BAR               0xFFFFFFE0 ///< Base Address
#define B_SMBUS_BAR0_PREF              BIT3  ///< Hardwired to 0. Indicated that SMBMBAR is not prefetchable
#define B_SMBUS_BAR0_ADDRNG            (BIT2 | BIT1)
#define B_SMBUS_BAR0_MSI               BIT0  ///< Memory Space Indicator

#define R_SMBUS_BAR1                   0x14  ///< The memory bar high
#define B_SMBUS_BAR1_BAR               0xFFFFFFFF ///< Base Address

#define R_SMBUS_BASE                   0x20  ///< The I/O memory bar
#define B_SMBUS_BASE_BAR               0x0000FFE0 ///< Base Address
#define B_SMBUS_BASE_IOSI              BIT0  ///< IO Space Indicator

#define R_SMBUS_SVID                   0x2C  ///< Subsystem Vendor ID
#define B_SMBUS_SVID                   0xFFFF ///< Subsystem Vendor ID

#define R_SMBUS_SID                    0x2E  ///< Subsystem ID
#define B_SMBUS_SID                    0xFFFF ///< Subsystem ID

#define R_SMBUS_INT_LN                 0x3C  ///< Interrupt Line
#define B_SMBUS_INT_LN                 0xFF  ///< Interrupt Line

#define R_SMBUS_INT_PN                 0x3D  ///< Interrupt Pin
#define B_SMBUS_INT_PN                 0xFF  ///< Interrupt Pin


#define R_PCH_SMBUS_HSTS               0x00  // Host Status Register R/W
#define B_PCH_SMBUS_HBSY               0x01
#define R_PCH_SMBUS_HCTL               0x02  // Host Control Register R/W
#define B_PCH_SMBUS_START              BIT6  // Start
#define B_PCH_SMBUS_DERR               0x04
#define B_PCH_SMBUS_BERR               0x08
#define B_PCH_SMBUS_IUS                0x40
#define B_PCH_SMBUS_BYTE_DONE_STS      0x80
#define B_PCH_SMBUS_HSTS_ALL           0xFF
#define V_PCH_SMBUS_SMB_CMD_BYTE_DATA  0x08  // Byte Data
#define V_PCH_SMBUS_SMB_CMD_BLOCK      0x14  // Block

#define R_SMBUS_HOSTC                  0x40  ///< Host Configuration Register
#define B_SMBUS_HOSTC_SPD_WD           BIT4  ///< SPD Write Disable
#define B_SMBUS_HOSTC_SSRESET          BIT3  ///< Soft SMBus Reset
#define B_SMBUS_HOSTC_I2C_EN           BIT2  ///< I2C Enable Bit
#define B_SMBUS_HOSTC_SMI_EN           BIT1  ///< SMI Enable Bit
#define B_SMBUS_HOSTC_HST_EN           BIT0  ///< Host Controller Enable Bit

#define R_SMBUS_TCOBASE                0x50 ///< TCO Base Address
#define B_SMBUS_TCOBASE_BAR            0x0000FFE0

#define R_SMBUS_TCOCTL                 0x54 ///< TCO Control
#define B_SMBUS_TCOCTL_TCO_BASE_EN     BIT8  ///< TCO Base Enable
#define R_SMBUS_TCOCTL_TCO_BASE_LOCK   BIT0  ///< TCO Base Lock

#define R_SMBUS_GTIM                   0x60  ///< Global Timing
#define B_SMBUS_GTIM_THDDAT            0xF0000 ///< THDDAT Timing Parameter
#define V_SMBUS_GTIM_THDDAT_0NS        0x00000 ///< 0 Clock (0 ns)
#define V_SMBUS_GTIM_THDDAT_100NS      0x10000 ///< 1 Clock (100 ns)
#define V_SMBUS_GTIM_THDDAT_1500NS     0xF0000 ///< 15 Clock (1500 ns)
#define B_SMBUS_GTIM_TSUDAT            0xF00  ///< TSUDAT Timing Parameter
#define V_SMBUS_GTIM_TSUDAT_0NS        0x000 ///< 0 Clock (0 ns)
#define V_SMBUS_GTIM_TSUDAT_100NS      0x100 ///< 1 Clock (100 ns)
#define V_SMBUS_GTIM_TSUDAT_1500NS     0xF00 ///< 15 Clock (1500 ns)
#define B_SMBUS_GTIM_DEGLITCH          0xF00  ///< DEGLITCH Timing Parameter
#define V_SMBUS_GTIM_DEGLITCH_0NS      0x0  ///< 0 Clock (0 ns)
#define V_SMBUS_GTIM_DEGLITCH_100NS    0x1  ///< 1 Clock (100 ns)
#define V_SMBUS_GTIM_DEGLITCH_1500NS   0xF  ///< 15 Clock (1500 ns)

#define R_SMBUS_HTIM                   0x64  ///< Host Timing
#define B_SMBUS_HTIM_THIGH             0xFF000000 ///< THIGH Timing Parameter
#define V_SMBUS_HTIM_THIGH_N700NS      0x00000000 ///< -7 Clocks (-700 ns)
#define V_SMBUS_HTIM_THIGH_N600NS      0x01000000 ///< -6 Clocks (-600 ns)
#define V_SMBUS_HTIM_THIGH_N100NS      0x07000000 ///< -1 Clocks (-100 ns)
#define V_SMBUS_HTIM_THIGH_0NS         0x08000000 ///< 0 Clocks (+0 ns)
#define V_SMBUS_HTIM_THIGH_P100NS      0x09000000 ///< +1 Clocks (+100 ns)
#define V_SMBUS_HTIM_THIGH_P24700NS    0xFF000000 ///< +247 Clocks (+24700 ns)
#define B_SMBUS_HTIM_TLOW              0xFF0000 ///< TLOW Timing Parameter
#define V_SMBUS_HTIM_TLOW_N700NS       0x000000 ///< -7 Clocks (-700 ns)
#define V_SMBUS_HTIM_TLOW_N600NS       0x010000 ///< -6 Clocks (-600 ns)
#define V_SMBUS_HTIM_TLOW_N100NS       0x070000 ///< -1 Clocks (-100 ns)
#define V_SMBUS_HTIM_TLOW_0NS          0x080000 ///< 0 Clocks (+0 ns)
#define V_SMBUS_HTIM_TLOW_P100NS       0x090000 ///< +1 Clocks (+100 ns)
#define V_SMBUS_HTIM_TLOW_P24700NS     0xFF0000 ///< +247 Clocks (+24700 ns)
#define B_SMBUS_HTIM_THDSTA            0xF000 ///< THDSTA Timing Parameter
#define V_SMBUS_HTIM_THDSTA_N700NS     0xF000 ///< -7 Clocks (-700 ns)
#define V_SMBUS_HTIM_THDSTA_N600NS     0xE000 ///< -6 Clocks (-600 ns)
#define V_SMBUS_HTIM_THDSTA_0NS        0x0000 ///< 0 Clocks (+0 ns)
#define V_SMBUS_HTIM_THDSTA_P100NS     0x1000 ///< +1 Clocks (+100 ns)
#define V_SMBUS_HTIM_THDSTA_P700NS     0x7000 ///< +7 Clocks (+700 ns)
#define B_SMBUS_HTIM_TSUDAT            0xF00 ///< TSUDAT Timing Parameter
#define V_SMBUS_HTIM_TSUDAT_N700NS     0xF00 ///< -7 Clocks (-700 ns)
#define V_SMBUS_HTIM_TSUDAT_N600NS     0xE00 ///< -6 Clocks (-600 ns)
#define V_SMBUS_HTIM_TSUDAT_0NS        0x000 ///< 0 Clocks (+0 ns)
#define V_SMBUS_HTIM_TSUDAT_P100NS     0x100 ///< +1 Clocks (+100 ns)
#define V_SMBUS_HTIM_TSUDAT_P700NS     0x700 ///< +7 Clocks (+700 ns)
#define B_SMBUS_HTIM_TBUF              0xF0  ///< TBUF Timing Parameter
#define V_SMBUS_HTIM_TBUF_N700NS       0xF0  ///< -7 Clocks (-700 ns)
#define V_SMBUS_HTIM_TBUF_N600NS       0xE0  ///< -6 Clocks (-600 ns)
#define V_SMBUS_HTIM_TBUF_0NS          0x00  ///< 0 Clocks (+0 ns)
#define V_SMBUS_HTIM_TBUF_P100NS       0x10  ///< +1 Clocks (+100 ns)
#define V_SMBUS_HTIM_TBUF_P700NS       0x70  ///< +7 Clocks (+700 ns)
#define B_SMBUS_HTIM_TSUSTA            0xF   ///< TSUSTA Timing Parameter
#define V_SMBUS_HTIM_TSUSTA_N700NS     0xF   ///< -7 Clocks (-700 ns)
#define V_SMBUS_HTIM_TSUSTA_N600NS     0xE   ///< -6 Clocks (-600 ns)
#define V_SMBUS_HTIM_TSUSTA_0NS        0x0   ///< 0 Clocks (+0 ns)
#define V_SMBUS_HTIM_TSUSTA_P100NS     0x1   ///< +1 Clocks (+100 ns)
#define V_SMBUS_HTIM_TSUSTA_P700NS     0x7   ///< +7 Clocks (+700 ns)

#define B_SMBUS_STIM                   0x68  ///< Slave Timing
#define B_SMBUS_STIM_STLOEXT           0xFF000000 ///< SMBCLK Tlow Extension Timing Parameter
#define V_SMBUS_STIM_STLOEXT_0NS       0x00000000 ///< 0 Clocks (+0 ns)
#define V_SMBUS_STIM_STLOEXT_100NS     0x01000000 ///< 1 Clocks (100 ns)
#define V_SMBUS_STIM_STLOEXT_25500NS   0xFF000000 ///< 255 Clocks (25500 ns)
#define B_SMBUS_STIM_STSUSTA           0xF00 ///< TSUSTA Timing Parameter
#define V_SMBUS_STIM_STSUSTA_N700NS    0xF00 ///< -7 Clocks (-700 ns)
#define V_SMBUS_STIM_STSUSTA_N600NS    0xE00 ///< -6 Clocks (-600 ns)
#define V_SMBUS_STIM_STSUSTA_0NS       0x000 ///< 0 Clocks (+0 ns)
#define V_SMBUS_STIM_STSUSTA_P100NS    0x100 ///< +1 Clocks (+100 ns)
#define V_SMBUS_STIM_STSUSTA_P700NS    0x700 ///< +7 Clocks (+700 ns)
#define B_SMBUS_STIM_STHDSTA           0xF0  ///< THDSTA Timing Parameter
#define V_SMBUS_STIM_STHDSTA_N700NS    0xF0  ///< -7 Clocks (-700 ns)
#define V_SMBUS_STIM_STHDSTA_N600NS    0xE0  ///< -6 Clocks (-600 ns)
#define V_SMBUS_STIM_STHDSTA_0NS       0x00  ///< 0 Clocks (+0 ns)
#define V_SMBUS_STIM_STHDSTA_P100NS    0x10  ///< +1 Clocks (+100 ns)
#define V_SMBUS_STIM_STHDSTA_P700NS    0x70  ///< +7 Clocks (+700 ns)
#define B_SMBUS_STIM_STSUSTO           0xF   ///< TSUSTO Timing Parameter
#define V_SMBUS_STIM_STSUSTO_N700NS    0xF   ///< -7 Clocks (-700 ns)
#define V_SMBUS_STIM_STSUSTO_N600NS    0xE   ///< -6 Clocks (-600 ns)
#define V_SMBUS_STIM_STSUSTO_0NS       0x0   ///< 0 Clocks (+0 ns)
#define V_SMBUS_STIM_STSUSTO_P100NS    0x1   ///< +1 Clocks (+100 ns)
#define V_SMBUS_STIM_STSUSTO_P700NS    0x7   ///< +7 Clocks (+700 ns)

#define R_SMBUS_SMBSM                  0x80  ///< SMB Safe Mode
#define B_SMBUS_SMBSM_PCDCGDIS         BIT18 ///< Power Gate Control Block Clock Gating Disable
#define B_SMBUS_SMBSM_ISDCGDIS         BIT16 ///< IOSF Sideband Interface Dynamic Clock Gating Disable
#define B_SMBUS_SMBSM_BBDCGDIS         BIT14 ///< IOSF Primary Interface Dynamic Clock Gating Disable
#define B_SMBUS_SMBSM_TRDCGDIS         BIT12 ///< Transaction Layer Dynamic Clock Gating Disable
#define B_SMBUS_SMBSM_LNDCGDIS         BIT10 ///< Link Layer Dynamic Clock Gating Disable
#define B_SMBUS_SMBSM_PHDCGDIS         BIT8  ///< Physical Layer Dynamic Clock Gating Disable
#define B_SMBUS_SMBSM_SACSE            BIT5  ///< Slave Auto Clock Stretch Enable
#define B_SMBUS_SMBSM_BLKWR_POL        BIT2  ///< Block Write Interrupt Policy
#define B_SMBUS_SMBSM_BLKRD_POL        BIT1  ///< Block Read Interrupt Policy
#define B_SMBUS_SMBSM_RSTART_RWBIT_POL BIT0  ///< Repeated Start Read/Write Bit Policy

#define R_SMBUS_MANID                  0xF8  ///< Manufacturer's ID Register
#define B_SMBUS_MANID_DOTID            0x0F000000 ///< DOT ID
#define B_SMBUS_MANID_SID              0x00FF0000 ///< Stepping ID
#define B_SMBUS_MANID_MID              0x0000FF00 ///< Manufacturer ID
#define B_SMBUS_MANID_PPID             0x000000FF ///< Process ID

///
/// SMBus I/O Registers
///
#define R_SMBUS_HSTS                   0x00  ///< Host Status Register R/W
#define B_SMBUS_HSTS_ALL               0xFF
#define B_SMBUS_BYTE_DONE_STS          BIT7  ///< Byte Done Status
#define B_SMBUS_IUS                    BIT6  ///< In Use Status
#define B_SMBUS_SMBALERT_STS           BIT5  ///< SMBUS Alert
#define B_SMBUS_FAIL                   BIT4  ///< Failed
#define B_SMBUS_BERR                   BIT3  ///< Bus Error
#define B_SMBUS_DERR                   BIT2  ///< Device Error
#define B_SMBUS_ERRORS                 (B_SMBUS_FAIL | B_SMBUS_BERR | B_SMBUS_DERR)
#define B_SMBUS_INTR                   BIT1  ///< Interrupt
#define B_SMBUS_HBSY                   BIT0  ///< Host Busy

#define R_SMBUS_HCTL                   0x02  ///< Host Control Register R/W
#define B_SMBUS_PEC_EN                 BIT7  ///< Packet Error Checking Enable
#define B_SMBUS_START                  BIT6  ///< Start
#define B_SMBUS_LAST_BYTE              BIT5  ///< Last Byte
#define B_SMBUS_SMB_CMD                0x1C  ///< SMB Command
#define V_SMBUS_SMB_CMD_BLOCK_PROCESS  0x1C  ///< Block Process
#define V_SMBUS_SMB_CMD_IIC_READ       0x18  ///< I2C Read
#define V_SMBUS_SMB_CMD_BLOCK          0x14  ///< Block
#define V_SMBUS_SMB_CMD_PROCESS_CALL   0x10  ///< Process Call
#define V_SMBUS_SMB_CMD_WORD_DATA      0x0C  ///< Word Data
#define V_SMBUS_SMB_CMD_BYTE_DATA      0x08  ///< Byte Data
#define V_SMBUS_SMB_CMD_BYTE           0x04  ///< Byte
#define V_SMBUS_SMB_CMD_QUICK          0x00  ///< Quick
#define B_SMBUS_KILL                   BIT1  ///< Kill
#define B_SMBUS_INTREN                 BIT0  ///< Interrupt Enable

#define R_SMBUS_HCMD                   0x03  ///< Host Command Register R/W
#define B_SMBUS_HCMD                   0xFF  ///< Command to be transmitted

#define R_SMBUS_TSA                    0x04  ///< Transmit Slave Address Register R/W
#define B_SMBUS_ADDRESS                0xFE  ///< 7-bit address of the targeted slave
#define B_SMBUS_RW_SEL                 BIT0  ///< Direction of the host transfer, 1 = read, 0 = write
#define B_SMBUS_RW_SEL_READ            0x01  ///< Read
#define B_SMBUS_RW_SEL_WRITE           0x00  ///< Write

#define R_SMBUS_HD0                    0x05  ///< Data 0 Register R/W
#define R_SMBUS_HD1                    0x06  ///< Data 1 Register R/W
#define R_SMBUS_HBD                    0x07  ///< Host Block Data Register R/W
#define R_SMBUS_PEC                    0x08  ///< Packet Error Check Data Register R/W

#define R_SMBUS_RSA                    0x09  ///< Receive Slave Address Register R/W
#define B_SMBUS_SLAVE_ADDR             0x7F  ///< TCO slave address (Not used, reserved)

#define R_SMBUS_SD                     0x0A  ///< Receive Slave Data Register R/W

#define R_SMBUS_AUXS                   0x0C  ///< Auxiliary Status Register R/WC
#define B_SMBUS_CRCE                   BIT0  ///< CRC Error

#define R_SMBUS_AUXC                   0x0D  ///< Auxiliary Control Register R/W
#define B_SMBUS_E32B                   BIT1  ///< Enable 32-byte Buffer
#define B_SMBUS_AAC                    BIT0  ///< Automatically Append CRC

#define R_SMBUS_SMLC                   0x0E  ///< SMLINK Pin Control Register R/W
#define B_SMBUS_SMLINK_CLK_CTL         BIT2  ///< Not supported
#define B_SMBUS_SMLINK1_CUR_STS        BIT1  ///< Not supported
#define B_SMBUS_SMLINK0_CUR_STS        BIT0  ///< Not supported


#define R_SMBUS_SMBC                   0x0F  ///< SMBus Pin Control Register R/W
#define B_SMBUS_SMBCLK_CTL             BIT2  ///< SMBCLK Control
#define B_SMBUS_SMBDATA_CUR_STS        BIT1  ///< SMBDATA Current Status
#define B_SMBUS_SMBCLK_CUR_STS         BIT0  ///< SMBCLK Current Status

#define R_SMBUS_SSTS                   0x10  ///< Slave Status Register R/WC
#define B_SMBUS_HOST_NOTIFY_STS        BIT0  ///< Host Notify Status

#define R_SMBUS_SCMD                   0x11  ///< Slave Command Register R/W
#define B_SMBUS_SMBALERT_DIS           BIT2  ///< Not supported
#define B_SMBUS_HOST_NOTIFY_WKEN       BIT1  ///< Host Notify Wake Enable
#define B_SMBUS_HOST_NOTIFY_INTREN     BIT0  ///< Host Notify Interrupt Enable

#define R_SMBUS_NDA                    0x14  ///< Notify Device Address Register RO
#define B_SMBUS_DEVICE_ADDRESS         0xFE  ///< Device Address

#define R_SMBUS_NDLB                   0x16  ///< Notify Data Low Byte Register RO
#define R_SMBUS_NDHB                   0x17  ///< Notify Data High Byte Register RO

///
/// Private Control Registers
///
#define R_PCR_SMBUS_PCE                0x10  ///< Power Control Enable
#define B_PCR_SMBUS_PCE_PMCRE          BIT0  ///< PMC Request Enable
#define B_PCR_SMBUS_PCE_I3E            BIT1  ///< I3 enable not supported RO
#define B_PCR_SMBUS_PCE_D3HE           BIT2  ///< D3-Hot enable not sopported RO
#define B_PCR_SMBUS_PCE_HAE            BIT5  ///< Hardware Autonomous Enable

//
// TCO register I/O map
//
#define R_TCO_TRLD                     0x0
#define R_TCO_TDI                      0x2
#define R_TCO_TDO                      0x3
#define R_TCO_TSTS1                    0x4
#define S_TCO_TSTS1                    2
#define B_TCO_TSTS1_CPUSERR            BIT12
#define B_TCO_TSTS1_CPUSMI             BIT10
#define B_TCO_TSTS1_CPUSCI             BIT9
#define B_TCO_TSTS1_BIOSWR             BIT8
#define B_TCO_TSTS1_NEWCENTURY         BIT7
#define B_TCO_TSTS1_TIMEOUT            BIT3
#define B_TCO_TSTS1_TCO_INT            BIT2
#define B_TCO_TSTS1_OS_TCO_SMI         BIT1
#define B_TCO_TSTS1_NMI2SMI            BIT0
#define N_TCO_TSTS1_CPUSMI             10
#define N_TCO_TSTS1_BIOSWR             8
#define N_TCO_TSTS1_NEWCENTURY         7
#define N_TCO_TSTS1_TIMEOUT            3
#define N_TCO_TSTS1_OS_TCO_SMI         1
#define N_TCO_TSTS1_NMI2SMI            0
#define R_TCO_TSTS2                    0x06
#define S_TCO_TSTS2                    2
#define B_TCO_TSTS2_SMLINK_SLV_SMI     BIT4
#define B_TCO_TSTS2_SECOND_TO          BIT1
#define B_TCO_TSTS2_INTRD_DET          BIT0
#define N_TCO_TSTS2_INTRD_DET          0
#define R_TCO_TCTL1                    0x08
#define S_TCO_TCTL1                    2
#define B_TCO_TCTL1_LOCK               BIT12
#define B_TCO_TCTL1_TMR_HLT            BIT11
#define B_TCO_TCTL1_NMI2SMI_EN         BIT9
#define B_TCO_TCTL1_NMI_NOW            BIT8
#define N_TCO_TCTL1_NMI2SMI_EN         9
#define R_TCO_TCTL2                    0x0A
#define S_TCO_TCTL2                    2
#define B_TCO_TCTL2_OS_POLICY          (BIT5|BIT4)
#define B_TCO_TCTL2_SMB_ALERT_DISABLE  BIT3
#define B_TCO_TCTL2_INTRD_SEL          (BIT2|BIT1)
#define N_TCO_TCTL2_INTRD_SEL          2
#define R_TCO_TMSG                     0x0C
#define R_TCO_WDS                      0x0E
#define R_TCO_SW_IRQ_GEN               0x10
#define R_TCO_TTMR                     0x12

#endif
