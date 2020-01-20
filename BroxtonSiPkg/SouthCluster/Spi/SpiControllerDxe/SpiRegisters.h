/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#ifndef _SPI_REGISTERS_H_
#define _SPI_REGISTERS_H_
#include <Uefi.h>

#pragma pack(1)
typedef struct INTELSPI_REGISTERS
{
  ///
  /// Register mapping of the SPI controller hardware.
  ///
  volatile UINT32  SSCR0;         ///< 0x00
  volatile UINT32  SSCR1;         ///< 0x04
  volatile UINT32  SSSR;          ///< 0x08
  volatile UINT32  SSITR;         ///< 0x0C
  volatile UINT32  SSDR;          ///< 0x10
  volatile UINT32  RSVD1[5];      ///< 0x14-0x24
  volatile UINT32  SSTO;          ///< 0x28
  volatile UINT32  SSPSP;         ///< 0x2C
  volatile UINT32  SSTSA;         ///< 0x30
  volatile UINT32  SSRSA;         ///< 0x34
  volatile UINT32  SSTSS;         ///< 0x38
  volatile UINT32  SSACD;         ///< 0x3C
  volatile UINT32  RSVD2;         ///< 0x40
  volatile UINT32  SITF;          ///< 0x44
  volatile UINT32  SIRF;          ///< 0x48
  //UINT32  RSVD3[493];           ///< 0x4C-0x7FC
  volatile UINT32  RSVD3[237];    ///< 0x4C-0x3FC
  volatile UINT32  PRV_CLOCKS;    ///< 0x400
  volatile UINT32  PRV_RESETS;    ///< 0x404
  volatile UINT32  PRV_GENERAL;   ///< 0x408
  volatile UINT32  PRV_SSP_REG;   ///< 0x40C
  volatile UINT32  PRV_HWLTR_VAL; ///< 0x410
  volatile UINT32  PRV_SWLTR_VAL; ///< 0x414
  volatile UINT32  PRV_CS_CTRL;   ///< 0x418
} SPI_REGISTERS;

#pragma pack()
//
// General defines
//
#define BIT0          0x00000001
#define BIT1          0x00000002
#define BIT2          0x00000004
#define BIT3          0x00000008
#define BIT4          0x00000010
#define BIT5          0x00000020
#define BIT6          0x00000040
#define BIT7          0x00000080
#define BIT8          0x00000100
#define BIT9          0x00000200
#define BIT10         0x00000400
#define BIT11         0x00000800
#define BIT12         0x00001000
#define BIT13         0x00002000
#define BIT14         0x00004000
#define BIT15         0x00008000
#define BIT16         0x00010000
#define BIT17         0x00020000
#define BIT18         0x00040000
#define BIT19         0x00080000
#define BIT20         0x00100000
#define BIT21         0x00200000
#define BIT22         0x00400000
#define BIT23         0x00800000
#define BIT24         0x01000000
#define BIT25         0x02000000
#define BIT26         0x04000000
#define BIT27         0x08000000
#define BIT28         0x10000000
#define BIT29         0x20000000
#define BIT30         0x40000000
#define BIT31         0x80000000

//
// Intel SPI controller registers.
//



//
// SSCR0 register bits.
//

#define SSCR0_MOD         BIT31
#define SSCR0_ACS         BIT30           ///< ESS==0
#define SSCR0_LNW_CHBIT   BIT29
#define SSCR0_FDRC_MASK   (BIT26 | BIT25 | BIT24)
#define SSCR0_TUR_NOINT   BIT23
#define SSCR0_ROR_NOINT   BIT22
#define SSCR0_NCS         BIT21           ///< ESS==0
#define SSCR0_EDSS_SMASK  BIT20           ///< ESS==0
#define SSCR0_EDDS_SHIFT  20
#define SSCR0_SCR_SHIFT   8
#define SSCR0_SCR_MASK    0xFFF
#define SSCR0_SCR_SMASK   (SSCR0_SCR_MASK << SSCR0_SCR_SHIFT)
#define SSCR0_SSE_EN      BIT7
#define SSCR0_ECS_EN      BIT6            ///< ESS==0
#define SSCR0_FRF_SPI     (0x00 << 4)     ///< ESS==0
#define SSCR0_FRF_SSP     (0x01 << 4)     ///< ESS==0
#define SSCR0_FRF_MW      (0x02 << 4)     ///< ESS==0
#define SSCR0_FRF_PSP     (0x03 << 4)     ///< ESS==0
#define SSCR0_DSS_MASK    0xF             ///< ESS==0
#define SSCR0_DSS_SHIFT   0
#define SSCR0_DSS_SMASK   (SSCR0_DSS_MASK << SSCR0_DSS_SHIFT)

#define SSCR1_TTELP     BIT31
#define SSCR1_TTE       BIT30
#define SSCR1_EBCEI     BIT29
#define SSCR1_SCFR      BIT28
#define SSCR1_ECRA      BIT27
#define SSCR1_ECRB      BIT26
#define SSCR1_SCLKDIR   BIT25
#define SSCR1_SFRMDIR   BIT24
#define SSCR1_RWOT      BIT23
#define SSCR1_TRAIL     BIT22
#define SSCR1_TSRE      BIT21
#define SSCR1_RSRE      BIT20
#define SSCR1_TINTE     BIT19
#define SSCR1_PINTE     BIT18
#define SSCR1_IFS       BIT16
#define SSCR1_STRF      BIT15
#define SSCR1_EFWR      BIT14
#define SSCR1_MWDS      BIT5
#define SSCR1_SPH       BIT4              ///< ESS==0
#define SSCR1_SPO       BIT3              ///< ESS==0
#define SSCR1_LBM       BIT2
#define SSCR1_TIE       BIT1
#define SSCR1_RIE       BIT0

#define SSSR_BCE        BIT23
#define SSSR_CSS        BIT22
#define SSSR_TUR        BIT21
#define SSSR_EOC        BIT20
#define SSSR_TINT       BIT19
#define SSSR_PINT       BIT18
#define SSSR_ROR        BIT7
#define SSSR_RFS        BIT6
#define SSSR_TFS        BIT5
#define SSSR_BSY        BIT4
#define SSSR_RNE        BIT3
#define SSSR_TNF        BIT2

#define SSTO_MASK     0x00FFFFFF

#define SSPSP_FSRT      BIT25
#define SSPSP_DMYSTOP_SHIFT 24
#define SSPSP_DMYSTOP_MASK  (BIT24 | BIT23)
#define SSPSP_SFRMWDTH_SHIFT  16
#define SSPSP_SFRMWDTH_MASK (0x3F << SSPSP_SFRMWDTH_SHIFT)
#define SSPSP_SFRMDLY_SHIFT 9
#define SSPSP_SFRMDLY_MASK  (0x7F << SSPSP_SFRMDLY_SHIFT)
#define SSPSP_DMYSTRT_SHIFT 7
#define SSPSP_DMYSTRT_MASK  (BIT8 | BIT7)
#define SSPSP_STRTDLY_SHIFT 4
#define SSPSP_STRTDLY_MASK  (BIT6 | BIT5 | BIT4)
#define SSPSP_ETDS      BIT3
#define SSPSP_SFRMP     BIT2
#define SSPSP_SCMODE_SHIFT  0
#define SSPSP_SCMODE_FRL  0
#define SSPSP_SCMODE_RFL  1
#define SSPSP_SCMODE_RFH  2
#define SSPSP_SCMODE_FRH  3

#define SSTSA_MASK      0x000000FF
#define SSRSA_MASK      0x000000FF

#define SSTSS_NMBSY     BIT31
#define SSTSS_TSS_MASK    (BIT2 | BIT1 | BIT0)
#define SSTSS_TSS_SHIFT   0

#define SSACD_ACPS_MASK   (BIT6 | BIT5 | BIT4)
#define SSACD_ACPS_SHIFT  4
#define SSACD_SCDB      BIT3
#define SSACD_ACDS_MASK   (BIT2 | BIT1 | BIT0)
#define SSACD_ACDS_SHIFT  0

#define SITF_SITFL_SHIFT  16
#define SITF_SITFL_MASK   0xFF
#define SITF_SITFL_SMASK  (SITF_SITFL_MASK << SITF_SITFL_SHIFT)
#define SITF_LWMTF_SHIFT  8
#define SITF_LWMTF_MASK   0xFF
#define SITF_LWMTF_SMASK  (SITF_LWMTF_MASK << SITF_LWMTF_SHIFT)
#define SITF_HWMTF_SHIFT  0
#define SITF_HWMTF_MASK   0xFF
#define SITF_HWMTF_SMASK  (SITF_HWMTF_MASK << SITF_HWMTF_SHIFT)

#define SIRF_SIRFL_SHIFT  8
#define SIRF_SIRFL_MASK   0xFF
#define SIRF_SIRFL_SMASK  (SIRF_SIRFL_MASK << SITF_LWMTF_SHIFT)
#define SIRF_WMRF_SHIFT   0
#define SIRF_WMRF_MASK    0xFF
#define SIRF_WMRF_SMASK   (SIRF_WMRF_MASK << SITF_HWMTF_SHIFT)


#define SPI_RX_FIFO_SIZE    256
#define SPI_TX_FIFO_SIZE    256

#define SPI_TX_ENTRIES     ((pRegisters->SITF >> SITF_SITFL_SHIFT) & SITF_SITFL_MASK)
#define SPI_TX_FIFO_SPACE   (SPI_TX_FIFO_SIZE - SPI_TX_ENTRIES)
#define SPI_RX_ENTRIES      ((pRegisters->SIRF >> SIRF_SIRFL_SHIFT) & SIRF_SIRFL_MASK)

#define SPI_TX_FIFO_NOT_FULL  SSSR_TNF  ///< TX FIFO not full
#define SPI_RX_FIFO_NOT_EMPTY SSSR_RNE  ///< RX FIFO not empty

#define SET_TRANSMIT_THRESHOLD(pDevice, low, high)  pRegisters->SITF = ((low & SITF_LWMTF_MASK) << SITF_LWMTF_SHIFT) | (high & SITF_HWMTF_MASK)
#define SET_RECEIVE_THRESHOLD(pDevice, thr)     pRegisters->SIRF = ((thr & SIRF_WMRF_MASK) << SIRF_WMRF_SHIFT)

#define GET_TRANSMIT_FIFO_LEVEL(pDevice)      ((pRegisters->SITF >> SITF_SITFL_SHIFT) & SITF_SITFL_MASK)
#define GET_RECEIVE_FIFO_LEVEL(pDevice)       ((pRegisters->SIRF >> SIRF_SIRFL_SHIFT) & SIRF_SIRFL_MASK)

#define SPI_TXRX_DMA_DISABLE(pDevice)       pRegisters->SSCR1 &= ~(ULONG)(SSCR1_TSRE | SSCR1_RSRE | SSCR1_TRAIL);
#define SPI_TX_DMA_DISABLE(pDevice)         pRegisters->SSCR1 &= ~(ULONG)(SSCR1_TSRE);
#define SPI_RX_DMA_DISABLE(pDevice)         pRegisters->SSCR1 &= ~(ULONG)(SSCR1_RSRE);
#define SPI_TXRX_DMA_ENABLE(pDevice)        pRegisters->SSCR1 |= (SSCR1_TSRE | SSCR1_RSRE | SSCR1_TRAIL);
#define SPI_TX_DMA_ENABLE(pDevice)          pRegisters->SSCR1 |= (SSCR1_TSRE | SSCR1_TRAIL);
#define SPI_RX_DMA_ENABLE(pDevice)          pRegisters->SSCR1 |= (SSCR1_RSRE | SSCR1_TRAIL);
#define SPI_TX_DMA_ONLY_ENABLE(pDevice)     pRegisters->SSCR1 = (pRegisters->SSCR1 & ~(ULONG)SSCR1_RSRE) | SSCR1_TSRE | SSCR1_TRAIL;
#define SPI_RX_DMA_ONLY_ENABLE(pDevice)     pRegisters->SSCR1 = (pRegisters->SSCR1 & ~(ULONG)SSCR1_TSRE) | SSCR1_RSRE | SSCR1_TRAIL;

//
// Private space bits
//
#define PRV_RESETS_NOT_FUNC_RST     0x00000001
#define PRV_RESETS_NOT_APB_RST      0x00000002

#define PRV_CLOCKS_CLK_EN           0x00000001
#define PRV_CLOCKS_UPDATE           0x80000000
#define PRV_CLOCKS_M_VAL_SHIFT      1
#define PRV_CLOCKS_M_VAL_MASK       0x7FFF
#define PRV_CLOCKS_M_VAL_DEFAULT    9154
#define PRV_CLOCKS_N_VAL_SHIFT      16
#define PRV_CLOCKS_N_VAL_MASK       0x7FFF
#define PRV_CLOCKS_N_VAL_DEFAULT    9154

#define PRV_GENERAL_ISOLATION       0x00000001
#define PRV_GENERAL_LTR_EN          0x00000002
#define PRV_GENERAL_LTR_MODE        0x00000004

#define PRV_GENERAL_DMA_RXTO_HOLDOFF_DISABLE          BIT24
#define PRV_GENERAL_TERMINATE_TX_ON_RX_FULL_DISABLE   BIT25

#define PRV_CS_CTRL_MODE_SW         BIT0
#define PRV_CS_CTRL_STATE_HI_BIT    BIT1
#define PRV_CS_CTRL_STATE_SHIFT     1

///
/// Interrupt Enable Bits on SSCR0
///
#define SSCR0_TIM   BIT23       ///< Transmit FIFO Under Run Interrupt disable
#define SSCR0_RIM   BIT22       ///< Receive FIFO Over Run Interrupt disable
#define SPI_INTERRUPT_SSCR0_MASK  (SSCR0_TIM | SSCR0_RIM)
///
/// Interrupt Enable Bits on SSCR1
///
#define SSCR1_RIE   BIT0         ///< Receive FIFO level interrupt is enabled
#define SSCR1_TIE   BIT1         ///< Transmit FIFO level interrupt is enabled
#define SSCR1_PINTE   BIT18      ///< Peripheral Trailing Byte Interrupts are enabled
#define SSCR1_TINTE   BIT19      ///< Receiver Time-out interrupts are enabled
//#define SSCR1_RSRE    BIT20    ///< DMA Receive Service Request Enable
//#define SSCR1_TSRE    BIT21    ///< DMA Transmit Service Request Enable
#define SSCR1_EBCEI   BIT29      ///< Enable Bit Count Error Interrupt (slave mode)
#define SPI_INTERRUPT_SSCR1_MASK  (SSCR1_RIE | SSCR1_TIE | SSCR1_PINTE | SSCR1_TINTE | SSCR1_EBCEI)
//#define SPI_INTERRUPT_SSCR1_MASK  (SSCR1_RIE | SSCR1_TIE | SSCR1_PINTE | SSCR1_TINTE | SSCR1_RSRE | SSCR1_TSRE | SSCR1_EBCEI)

///
/// Interrupt Status bits in SSSR
///
#define SSSR_TFS    BIT5    ///< Transmit FIFO Service Request - SSCR1_TIE
#define SSSR_RFS    BIT6    ///< Receive FIFO Service Request - SSCR1_RIE
#define SSSR_ROR    BIT7    ///< Receive FIFO Overrun - SSCR0_RIM
#define SSSR_PINT   BIT18   ///< Peripheral Trailing Byte Interrupt - SSCR0_PINTE
#define SSSR_TINT   BIT19   ///< Receiver Time-out Interrupt - SSCR1_TINTE
#define SSSR_EOC    BIT20   ///< DMA has signaled an end of chain condition
#define SSSR_TUR    BIT21   ///< Transmit FIFO Under Run - SSCR0_TIM
#define SSSR_BCE    BIT23   ///< Bit Count Error - SSCR1_EBCEI
#define SPI_INTERRUPT_STATUS_MASK (SSSR_TFS | SSSR_RFS | SSSR_ROR | SSSR_PINT | SSSR_TINT | SSSR_EOC | SSSR_TUR | SSSR_BCE)


typedef struct {
  //
  // Register structure to match the mapping of MBAR1
  //
  UINT32  DEVID_VENID;        ///< 0x00
  UINT32  IC_RESERVED4[31];   ///< 0x04-0x7F
  UINT32  PWRCAP;             ///< 0x80
  UINT32  PWRCON;             ///< 0x84
} PCI_REGISTERS;

#define PCI_PWR_SET_Dx_MASK   0x00000003
#define PCI_PWR_SET_D0_BITS   0x00000000
#define PCI_PWR_SET_D3_BITS   0x00000003
//
//emulated DMA Interrupt status
//
#define IC_INTR_DMA_TX          0x00010000
#define IC_INTR_DMA_RX          0x00020000
#define I2C_ALL_DMA_INTR        (IC_INTR_DMA_TX | IC_INTR_DMA_RX)

#define DMA_STATUS_NONE           0x0
#define DMA_STATUS_COMPLETE       0x1
#define DMA_STATUS_ABORTED        0x2
#define DMA_STATUS_ERROR          0x4
#define DMA_STATUS_CANCELLED      0x8

#endif

