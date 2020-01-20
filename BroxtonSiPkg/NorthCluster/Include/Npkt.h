/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1996 - 2016 Intel Corporation.

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

#ifndef _NPKT_H
#define _NPKT_H

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>
#include "PlatformBaseAddresses.h"

#include <PiPei.h>
#include <IndustryStandard/Pci22.h>


#ifndef MmPciAddress
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCIEX_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#endif

typedef union{
    PHYSICAL_ADDRESS Bar64;
    struct {
         UINT32 BarLow;
         UINT32 BarHigh;
       }r;

} MMIOBAR64;
//
// Definitions
//
#define EFI_ACPI_NORTH_PEAK_TABLE_REVISION        0x01
#define EFI_ACPI_NORTH_PEAK_TABLE_SIGNATURE       SIGNATURE_32('N', 'P', 'K', 'T')

#define PCI_DEVICE_NUMBER_NPK                     0
#define PCI_FUNCTION_NUMBER_NPK                   2

#define V_PCH_NPK_VENDOR_ID                       0x8086
#define V_BXT_P_PCH_NPK_DEVICE_ID                 0x5A8E
#define V_BXT_M_PCH_NPK_DEVICE_ID_A1              0x0A80

// As per the PCI Device ID spreadsheet, NPK Device ID is 0x1A80
// But on the real BXT B1 hardware, we have 0x1A8E as Device ID for NPK
// So commenting out 0x1A80 & enabling 0x1A8E
//#define V_BXT_M_PCH_NPK_DEVICE_ID_B0            0x1A80
#define V_BXT_M_PCH_NPK_DEVICE_ID_B0              0x1A8E

//
//  NPK Device ID macros
//
#define IS_BXT_P_NPK_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_BXT_P_PCH_NPK_DEVICE_ID) \
    )

#define IS_BXT_M_NPK_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_BXT_M_PCH_NPK_DEVICE_ID_A1) || \
      (DeviceId == V_BXT_M_PCH_NPK_DEVICE_ID_B0) \
    )

#define R_PCH_NPK_CMD                             0x04
#define B_PCH_NPK_CMD_MSE                         BIT1  // Memory Space Enable
#define R_PCH_NPK_CSR_MTB_LBAR                    0x10
#define B_PCH_NPK_CSR_MTB_RBAL                    0xFFF00000
#define R_PCH_NPK_CSR_MTB_UBAR                    0x14
#define B_PCH_NPK_CSR_MTB_RBAU                    0xFFFFFFFF
#define R_PCH_NPK_SW_LBAR                         0x18
#define B_PCH_NPK_SW_RBAL                         0xFFE00000
#define R_PCH_NPK_SW_UBAR                         0x1C
#define B_PCH_NPK_SW_RBAU                         0xFFFFFFFF
#define R_PCH_NPK_RTIT_LBAR                       0x20
#define B_PCH_NPK_RTIT_RBAL                       0xFFFFFF00
#define R_PCH_NPK_RTIT_UBAR                       0x24
#define B_PCH_NPK_RTIT_RBAU                       0xFFFFFFFF
#define R_PCH_NPK_MSICID                          0x40
#define R_PCH_NPK_MSINCP                          0x41
#define R_PCH_NPK_MSIMC                           0x42
#define R_PCH_NPK_MSILMA                          0x44
#define R_PCH_NPK_MSIUMA                          0x48
#define R_PCH_NPK_MSIMD                           0x4C
#define R_PCH_NPK_FW_LBAR                         0x70
#define B_PCH_NPK_FW_RBAL                         0xFFFC0000
#define R_PCH_NPK_FW_UBAR                         0x74
#define B_PCH_NPK_FW_RBAU                         0xFFFFFFFF
#define V_PCH_NPK_FW_BARL                         0xFE240000  //Range : 0xFE240000 - 0xFE27FFFF
#define V_PCH_NPK_FW_BARU                         0x00000000
#define R_PCH_NPK_DSC                             0x80
#define B_PCH_NPK_BYP                             BIT0
#define R_PCH_NPK_DSS                             0x81
#define R_PCH_NPK_ISTOT                           0x84
#define R_PCH_NPK_ICTOT                           0x88
#define R_PCH_NPK_IPAD                            0x8C
#define R_PCH_NPK_DSD                             0x90

//
// Offsets from CSR_MTB_BAR
//

#define R_PCH_NPK_MTB_SWDEST_0                    0x08



#define R_PCH_NPK_MTB_GTHOPT0                     0x00
#define B_PCH_NPK_MTB_GTHOPT0_P0FLUSH             BIT7
#define B_PCH_NPK_MTB_GTHOPT0_P1FLUSH             BIT15
#define B_PCH_NPK_MTB_GTHOPT0_P2NULL              BIT19
#define V_PCH_NPK_MTB_SWDEST_PTI                  0x0A
#define V_PCH_NPK_MTB_SWDEST_MEMEXI               0x08
#define V_PCH_NPK_MTB_SWDEST_DISABLE              0x00
#define R_PCH_NPK_MTB_SWDEST_1                    0x0C
#define B_PCH_NPK_MTB_SWDEST_CSE_1                0x0000000F
#define B_PCH_NPK_MTB_SWDEST_CSE_2                0x000000F0
#define B_PCH_NPK_MTB_SWDEST_CSE_3                0x00000F00
#define B_PCH_NPK_MTB_SWDEST_ISH_1                0x0000F000
#define B_PCH_NPK_MTB_SWDEST_ISH_2                0x000F0000
#define B_PCH_NPK_MTB_SWDEST_ISH_3                0x00F00000
#define B_PCH_NPK_MTB_SWDEST_AUDIO                0x0F000000
#define B_PCH_NPK_MTB_SWDEST_PMC                  0xF0000000
#define R_PCH_NPK_MTB_SWDEST_2                    0x10
#define B_PCH_NPK_MTB_SWDEST_FTH                  0x0000000F
#define R_PCH_NPK_MTB_SWDEST_3                    0x14
#define B_PCH_NPK_MTB_SWDEST_MAESTRO              0x00000F00
#define B_PCH_NPK_MTB_SWDEST_IPU                  0x0F000000
#define B_PCH_NPK_MTB_SWDEST_AET                  0xF0000000
#define R_PCH_NPK_MTB_SWDEST_4                    0x18
#define R_PCH_NPK_MTB_SWDEST_15                   0x44
#define R_PCH_NPK_MTB_MSC0CTL                     0xA0100
#define R_PCH_NPK_MTB_MSC0STS                     0xA0104
#define R_PCH_NPK_MTB_MSC1CTL                     0xA0200
#define V_PCH_NPK_MTB_MSCNMODE_SINGLE             0x0
#define V_PCH_NPK_MTB_MSCNMODE_MULTI              0x1
#define V_PCH_NPK_MTB_MSCNMODE_EXI                0x2
#define V_PCH_NPK_MTB_MSCNMODE_DEBUG              0x3
#define B_PCH_NPK_MTB_MSCNLEN                     (BIT10 | BIT9 | BIT8)
#define B_PCH_NPK_MTB_MSCNMODE                    (BIT5 | BIT4)
#define N_PCH_NPK_MTB_MSCNMODE                    0x4
#define B_PCH_NPK_MTB_MSCN_RD_HDR_OVRD            BIT2
#define B_PCH_NPK_MTB_MSCnSTS_WRAPSTAT            BIT1
#define B_PCH_NPK_MTB_WRAPENN                     BIT1
#define B_PCH_NPK_MTB_MSCNEN                      BIT0
#define R_PCH_NPK_MTB_GTHSTAT                     0xD4
#define B_PCH_NPK_MTB_GTHSTAT_PLE0                BIT0
#define R_PCH_NPK_MTB_SCR2                        0xD8
#define B_PCH_NPK_MTB_SCR2_FCD                    BIT0
#define B_PCH_NPK_MTB_SCR2_FSEOFF2                BIT2
#define B_PCH_NPK_MTB_SCR2_FSEOFF3                BIT3
#define B_PCH_NPK_MTB_SCR2_FSEOFF4                BIT4
#define B_PCH_NPK_MTB_SCR2_FSEOFF5                BIT5
#define B_PCH_NPK_MTB_SCR2_FSEOFF6                BIT6
#define B_PCH_NPK_MTB_SCR2_FSEOFF7                BIT7
#define B_PCH_NPK_MTB_SCRPD_DEBUGGER_IN_USE       BIT24
#define R_PCH_NPK_MTB_MSC0BAR                     0xA0108
#define R_PCH_NPK_MTB_MSC0SIZE                    0xA010C
#define R_PCH_NPK_MTB_MSC1STS                     0xA0204
#define R_PCH_NPK_MTB_MSC1BAR                     0xA0208
#define R_PCH_NPK_MTB_MSC1SIZE                    0xA020C
#define R_PCH_NPK_MTB_STREAMCFG1                  0xA1000
#define B_PCH_NPK_MTB_STREAMCFG1_SETSTRMMODE      BIT0  //0: Dbc, 1:BSSB
#define B_PCH_NPK_MTB_STREAMCFG1_ENABLE           BIT28
#define R_PCH_NPK_MTB_STREAMCFG2                  0xA1004
#define R_PCH_NPK_MTB_DBCSTSCMD                   0xA1008
#define R_PCH_NPK_MTB_DBCSTSDATA                  0xA100C
#define R_PCH_NPK_MTB_EXISTSCMD                   0xA1010
#define R_PCH_NPK_MTB_EXISTSDATA                  0xA1014
#define R_PCH_NPK_MTB_TIMEOUT                     0xA1018
#define R_PCH_NPK_MTB_EXIBASEHI                   0xA101C
#define R_PCH_NPK_MTB_EXIBASELO                   0xA1020
#define R_PCH_NPK_MTB_DBCBASEHI                   0xA1024
#define R_PCH_NPK_MTB_DBCBASELO                   0xA1028
#define R_PCH_NPK_MTB_NPKHSTS                     0xA102C
#define R_PCH_NPK_MTB_NPKHRETRY                   0xA1030
#define R_PCH_NPK_MTB_PTI_CTL                     0x1C00
#define B_PCH_NPK_MTB_PTIMODESEL                  0xF0
#define N_PCH_NPK_MTB_PTIMODESEL                  0x4
#define B_PCH_NPK_MTB_PTICLKDIV                   (BIT17 | BIT16)
#define N_PCH_NPK_MTB_PTICLKDIV                   0x10
#define B_PCH_NPK_MTB_PATGENMOD                   (BIT22 | BIT21 | BIT20)
#define N_PCH_NPK_MTB_PATGENMOD                   0x14
#define B_PCH_NPK_MTB_PTI_EN                      BIT0
#define R_PCH_NPK_MTB_SCR                         0xC8
#define V_PCH_NPK_MTB_SCR                         0x00130000
#define R_PCH_NPK_CSR_MTB_SCRATCHPAD1             0xE4
#define R_PCH_NPK_CSR_MTB_SCRATCHPAD2             0xE8
#define R_PCH_NPK_MTB_CTPGCS                      0x1C14
#define B_PCH_NPK_MTB_CTPEN                       BIT0
#define B_PCH_NPK_MTB_DESTOVR                     0xDC
#define B_PCH_NPK_MTB_GSWDEST                     0x88
#define B_PCH_NPK_MTB_SCRPD                       0xE0

#define B_PCH_NPK_MTB_SCRPD_MEMISPRIMDEST         BIT0
#define B_PCH_NPK_MTB_SCRPD_DBCISPRIMDEST         BIT1
#define B_PCH_NPK_MTB_SCRPD_PTIISPRIMDEST         BIT2
#define B_PCH_NPK_MTB_SCRPD_BSSBISPRIMDEST        BIT3
#define B_PCH_NPK_MTB_SCRPD_PTIISALTDEST          BIT4
#define B_PCH_NPK_MTB_SCRPD_BSSBISALTDEST         BIT5
#define B_PCH_NPK_MTB_SCRPD_MSC0ISENABLED         BIT9
#define B_PCH_NPK_MTB_SCRPD_MSC1ISENABLED         BIT10
#define B_PCH_NPK_MTB_SCRPD_TRIGGERISENABLED      BIT12
#define B_PCH_NPK_MTB_SCRPD_ODLAISENABLED         BIT13
#define B_PCH_NPK_MTB_SCRPD_SOCHAPISENABLED       BIT14
#define B_PCH_NPK_MTB_SCRPD_STHISENABLED          BIT15
#define B_PCH_NPK_MTB_SCRPD_NPKHISENABLED         BIT16
#define B_PCH_NPK_MTB_SCRPD_VERISENABLED          BIT17

#define R_PCH_NPK_MTB_MSC0MWP                     0xA0110
#define R_PCH_NPK_MTB_MSC0TBRP                    0xA0114
#define R_PCH_NPK_MTB_MSC0TBWP                    0xA0118
#define R_PCH_NPK_MTB_MSC1MWP                     0xA0210
#define R_PCH_NPK_MTB_MSC1TBRP                    0xA0214
#define R_PCH_NPK_MTB_MSC1TBWP                    0xA0218

//
// SCRPD[2]: ScratchPad[2] register use to record BIOS setup Option and other info to sync to TraceLib
//
#define TRACE_TYPE_MEM                            BIT0
#define TRACE_TYPE_PTI                            BIT1
#define TRACE_TYPE_DCI                            BIT2
#define TRACE_TYPE_BSSB                           BIT3
#define MEMORY_INSTALLED                          BIT4
#define NPK_INIT_DONE                             BIT5


#define LIBRARY_INIT_DONE_BEFORE_MEM              BIT5
#define LIBRARY_INIT_DONE_AFTER_MEM               BIT6
#define LIBRARY_INIT_DONE_READY_BOOT              BIT7


//
//
//
#define MTB_WRITE_WRAPPED                         BIT0
#define MTB_RECOVERY_WRITE_WRAPPED                BIT1
#define CSR_WRITE_WRAPPED                         BIT2
#define CSR_RECOVERY_WRITE_WRAPPED                BIT3
#define CSR_IN_USE                                BIT4

#define ENABLE                                    0

//
// Define Northpeak destination
//
#define NPK_TRACE_TO_MEMORY                       1
#define NPK_TRACE_TO_DCI                          2
#define NPK_TRACE_TO_BSSB                         3
#define NPK_TRACE_TO_PTI                          4


//
// Define GTH port type
//
#define NPK_GTH_PORT_NONE                   0
#define NPK_GTH_PORT_MEM_USB_BSSB           1
#define NPK_GTH_PORT_CTP                    2
#define NPK_GTH_PORT_LPP                    3
#define NPK_GTH_PORT_PTI                    4

#define NPK_MSC_SINGLE                      0
#define NPK_MSC_MUTI                        1
#define NPK_GTH_EXI                         2
#define NPK_GTH_DEBUG                       3

#define NPK_BEFORE_MEM_INIT_DONE            BIT0
#define NPK_AFTER_MEM_INIT_DONE             BIT1

#define PUNIT_DEBUG_MESSAGE_VERBOSITY_LEVEL_NONE     0
#define PUNIT_DEBUG_MESSAGE_VERBOSITY_LEVEL_MIN      1
#define PUNIT_DEBUG_MESSAGE_VERBOSITY_LEVEL_MEDIUM   2
#define PUNIT_DEBUG_MESSAGE_VERBOSITY_LEVEL_MAX      3
#define PUNIT_DEBUG_MESSAGE_VERBOSITY_LEVEL_DEFAULT  4

#define PMC_DEBUG_MESSAGE_VERBOSITY_LEVEL_NONE     0
#define PMC_DEBUG_MESSAGE_VERBOSITY_LEVEL_MIN      1
#define PMC_DEBUG_MESSAGE_VERBOSITY_LEVEL_MEDIUM   2
#define PMC_DEBUG_MESSAGE_VERBOSITY_LEVEL_MAX      3
#define PMC_DEBUG_MESSAGE_VERBOSITY_LEVEL_DEFAULT  4


//
// Define BIOS trace master and channle
//
#define SW_BAR_BIOS_TRACE_MASTER 256
#define SW_BAR_BIOS_TRACE_CHANNEL 13

//
// Define Register STATUS_CSR_LOWER_MAP and field Dfx Secure Policy[31:16]
//
#define R_DFX_AGG_STATUS_CSR_LOWER_MAP                           0x8408
#define B_DFX_AGG_STATUS_CSR_LOWER_MAP_DFX_SECURE_POLICY_MASK    0xffff0000
#define N_DFX_AGG_STATUS_CSR_LOWER_MAP_DFX_SECURE_POLICY_OFFSET  0x10
#define V_POLICY_RED2                                            0x2
#define V_POLICY_RED4                                            0x4
#define V_POLICY_ORANGE                                          0x5

//
// NPK ACPI define
//
#pragma pack(1)

typedef struct {
  UINT64 MtbAddress;
  UINT32 MtbSize;
  UINT32 MtbWriteOffset;
  UINT64 MtbRecoveryAddress;
  UINT32 MtbRecoverySize;
  UINT32 MtbRecoveryWriteOffset;
  UINT64 CsrAddress;
  UINT32 CsrSize;
  UINT32 CsrWriteOffset;
  UINT64 CsrRecoveryAddress;
  UINT32 CsrRecoverySize;
  UINT32 CsrRecoveryWriteOffset;
} SPECIFIC_TABLE_CONTENT;


typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  SPECIFIC_TABLE_CONTENT      SpecificTable;
  UINT8                       Flags;
} EFI_ACPI_NORTH_PEAK_TABLE_HEADER;

#pragma pack()

#endif

