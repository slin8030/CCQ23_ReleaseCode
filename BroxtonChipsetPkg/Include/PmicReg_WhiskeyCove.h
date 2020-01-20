/**@file
  Header file of WhiskeyCove PMIC registers.

@copyright
 Copyright (c) 2012 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _PMIC_REG_WHISKYCOVE_H_
#define _PMIC_REG_WHISKYCOVE_H_

#define    DEVICE1_BASE_ADDRESS           0x4E
#define    DEVICE2_BASE_ADDRESS           0x4F
#define    DEVICE3_BASE_ADDRESS           0x5E
#define    DEVICE6_BASE_ADDRESS           0x70
#define    SCRATCH_BASE_ADDRESS           0x6F
#define    VCC_BASE_ADDRESS               0x1A
#define    VNN_BASE_ADDRESS               0x12
#define    VNNAON_BASE_ADDRESS            0x14
#define    VSYS_BASE_ADDRESS              0x1C

//
// Device1
//
#define WKC_ID0_REG                       0x00
#define WKC_MASK_VENDID                   (BIT7|BIT6)
#define WKC_MASK_MAJREV                   (BIT5|BIT4|BIT3)
#define WKC_MASK_MINREV                   (BIT2|BIT1|BIT0)

#define WKC_ID1_REG                       0x01
#define WKC_IRQLVL1_REG                   0x02
#define WKC_MASK_CRIT                     BIT7
#define WKC_MASK_GPIO                     BIT6
#define WKC_MASK_CHGR                     BIT5
#define WKC_MASK_ADC                      BIT4
#define WKC_MASK_BCU                      BIT3
#define WKC_MASK_THRM                     BIT2
#define WKC_MASK_TMU                      BIT1
#define WKC_MASK_PWRBTN                   BIT0

#define WKC_PBIRQ_REG                     0x03
#define WKC_MASK_VEND                     BIT7
#define WKC_MASK_UBTN                     BIT1
#define WKC_MASK_PBTN                     BIT0

#define WKC_THRMIRQ0_REG                  0x04
#define WKC_MASK_GPALRT                   BIT7
#define WKC_MASK_SYS2ALRT1                BIT6
#define WKC_MASK_SYS1ALRT1                BIT5
#define WKC_MASK_SYS0ALRT1                BIT4
#define WKC_MASK_PMICALRT0                BIT3
#define WKC_MASK_SYS2ALRT0                BIT2
#define WKC_MASK_SYS1ALRT0                BIT1
#define WKC_MASK_SYS0ALRT0                BIT0

#define WKC_THRMIRQ1_REG                  0x05
#define WKC_MASK_BZIRQ                    BIT7
#define WKC_MASK_BATCRITHOTCOLD           BIT5
#define WKC_MASK_BATCRIT                  BIT4
#define WKC_MASK_PMICCRIT                 BIT3
#define WKC_MASK_SYS2CRIT                 BIT2
#define WKC_MASK_SYS1CRIT                 BIT1
#define WKC_MASK_SYS0CRIT                 BIT0

#define WKC_THRMIRQ2_REG                  0x06
#define WKC_MASK_BAT1ALRT3                BIT7
#define WKC_MASK_BAT0ALRT3                BIT6
#define WKC_MASK_BAT1ALRT0                BIT5
#define WKC_MASK_BAT0ALRT0                BIT4
#define WKC_MASK_PMICALRT3                BIT3
#define WKC_MASK_SYS2ALRT3                BIT2
#define WKC_MASK_SYS1ALRT3                BIT1
#define WKC_MASK_SYS0ALRT3                BIT0

#define WKC_BCUIRQ_REG                    0x07
#define WKC_MASK_TXPWRTH_LVL              BIT4
#define WKC_MASK_GSMPULSE_LVL             BIT3
#define WKC_MASK_VCRIT_LVL                BIT2
#define WKC_MASK_VWARN2_LVL               BIT1
#define WKC_MASK_VWARN1_LVL               BIT0

#define WKC_ADCIRQ_REG                    0x08
#define WKC_MASK_CCTICK                   BIT7
#define WKC_MASK_GPMEAS                   BIT6
#define WKC_MASK_VBATT                    BIT5
#define WKC_MASK_BATTID                   BIT4
#define WKC_MASK_SYSTEMP                  BIT3
#define WKC_MASK_BATTEMP                  BIT2
#define WKC_MASK_PEAK                     BIT1
#define WKC_MASK_USBID                    BIT0

#define WKC_CHGRIRQ0_REG                  0x09
#define WKC_MASK_CTYP                     BIT4
#define WKC_MASK_I2CERROR                 BIT3
#define WKC_MASK_I2CRDCMP                 BIT2
#define WKC_MASK_I2CWRCMP                 BIT1
#define WKC_MASK_CHGINTB                  BIT0

#define WKC_CHGRIRQ1_REG                  0x0A
#define WKC_MASK_USBIDGNDDET              BIT4
#define WKC_MASK_USBIDFLTDET              BIT3
#define WKC_MASK_BATTDET                  BIT2
#define WKC_MASK_DCDET                    BIT1
#define WKC_MASK_VBUSDET                  BIT0

#define WKC_GPIOIRQ0_REG                  0x0B
#define WKC_MASK_BATTIO                   BIT7
#define WKC_MASK_GPIO0P6                  BIT6
#define WKC_MASK_GPIO0P5                  BIT5
#define WKC_MASK_GPIO0P4                  BIT4
#define WKC_MASK_GPIO0P3                  BIT3
#define WKC_MASK_GPIO0P2                  BIT2
#define WKC_MASK_GPIO0P1                  BIT1
#define WKC_MASK_GPIO0P0                  BIT0

#define WKC_GPIOIRQ1_REG                  0x0C
#define WKC_MASK_GPIO2P1                  BIT5
#define WKC_MASK_GPIO2P0                  BIT4
#define WKC_MASK_GPIO1P3                  BIT3
#define WKC_MASK_GPIO1P2                  BIT2
#define WKC_MASK_GPIO1P1                  BIT1
#define WKC_MASK_GPIO1P0                  BIT0

#define WKC_CRITIRQ_REG                   0x0D
#define WKC_MASK_OSCSTP                   BIT1
#define WKC_MASK_VROCP                    BIT0

#define WKC_MIRQLVL1_REG                  0x0E
#define WKC_MASK_MCRIT                    BIT7
#define WKC_MASK_MGPIO                    BIT6
#define WKC_MASK_MCHGR                    BIT5
#define WKC_MASK_MADC                     BIT4
#define WKC_MASK_MBCU                     BIT3
#define WKC_MASK_MTHRM                    BIT2
#define WKC_MASK_MTMU                     BIT1
#define WKC_MASK_MPWRBTN                  BIT0

#define WKC_MPBIRQ_REG                    0x0F
#define WKC_MASK_MVEND                    BIT7
#define WKC_MASK_MUBTN                    BIT1
#define WKC_MASK_MPBTN                    BIT0

#define WKC_MTHRMIRQ0_REG                 0x12
#define WKC_MASK_MGPALERT                 BIT7
#define WKC_MASK_MSYS2ALRT1               BIT6
#define WKC_MASK_MSYS1ALRT1               BIT5
#define WKC_MASK_MSYS0ALRT1               BIT4
#define WKC_MASK_MPMICALRT0               BIT3
#define WKC_MASK_MSYS2ALRT0               BIT2
#define WKC_MASK_MSYS1ALRT0               BIT1
#define WKC_MASK_MSYS0ALRT0               BIT0

#define WKC_MTHRMIRQ1_REG                 0x13
#define WKC_MASK_MBZIRQ                   BIT7
#define WKC_MASK_MBATCRITHOTCOLD          BIT5
#define WKC_MASK_MBATCRIT                 BIT4
#define WKC_MASK_MPMICCRIT                BIT3
#define WKC_MASK_MSYS2CRIT                BIT2
#define WKC_MASK_MSYS1CRIT                BIT1
#define WKC_MASK_MSYS0CRIT                BIT0

#define WKC_MTHRMIRQ2_REG                 0x14
#define WKC_MASK_MBAT1ALRT3               BIT7
#define WKC_MASK_MBAT0ALRT3               BIT6
#define WKC_MASK_MBAT1ALRT0               BIT5
#define WKC_MASK_MBAT0ALRT0               BIT4
#define WKC_MASK_MPMICALRT3               BIT3
#define WKC_MASK_MSYS2ALRT3               BIT2
#define WKC_MASK_MSYS1ALRT3               BIT1
#define WKC_MASK_MSYS0ALRT3               BIT0

#define WKC_MBCUIRQ_REG                   0x15
#define WKC_MASK_MTXPWRTH                 BIT4
#define WKC_MASK_MGSMPULSE                BIT3
#define WKC_MASK_MVCRIT                   BIT2
#define WKC_MASK_MVWARN2                  BIT1
#define WKC_MASK_MVWARN1                  BIT0

#define WKC_MADCIRQ_REG                   0x16
#define WKC_MASK_MCCTICK                  BIT7
#define WKC_MASK_MGPMEAS                  BIT6
#define WKC_MASK_MVBATT                   BIT5
#define WKC_MASK_MBATTID                  BIT4
#define WKC_MASK_MSYSTEMP                 BIT3
#define WKC_MASK_MBATTEMP                 BIT2
#define WKC_MASK_MPEAK                    BIT1
#define WKC_MASK_MUSBID                   BIT0

#define WKC_MCHGRIRQ0_REG                 0x17
#define WKC_MASK_MCTYP                    BIT4
#define WKC_MASK_MI2CERROR                BIT3
#define WKC_MASK_MI2CRDCMP                BIT2
#define WKC_MASK_MI2CWRCMP                BIT1
#define WKC_MASK_MCHGINTB                 BIT0

#define WKC_MCHGRIRQ1_REG                 0x18
#define WKC_MASK_MUSBIDGNDDET             BIT4
#define WKC_MASK_MUSBIDFLTDET             BIT3
#define WKC_MASK_MBATTDET                 BIT2
#define WKC_MASK_MDCDET                   BIT1
#define WKC_MASK_MVBUSDET                 BIT0

#define WKC_MGPIOIRQ0_REG                 0x19
#define WKC_MASK_MBATTIO                  BIT7
#define WKC_MASK_MGPIO0P6                 BIT6
#define WKC_MASK_MGPIO0P5                 BIT5
#define WKC_MASK_MGPIO0P4                 BIT4
#define WKC_MASK_MGPIO0P3                 BIT3
#define WKC_MASK_MGPIO0P2                 BIT2
#define WKC_MASK_MGPIO0P1                 BIT1
#define WKC_MASK_MGPIO0P0                 BIT0

#define WKC_MGPIOIRQ1_REG                 0x1A
#define WKC_MASK_MGPIO2P1                 BIT5
#define WKC_MASK_MGPIO2P0                 BIT4
#define WKC_MASK_MGPIO1P3                 BIT3
#define WKC_MASK_MGPIO1P2                 BIT2
#define WKC_MASK_MGPIO1P1                 BIT1
#define WKC_MASK_MGPIO1P0                 BIT0

#define WKC_MCRITIRQ_REG                  0x1B
#define WKC_MASK_MOSCSTP                  BIT1
#define WKC_MASK_MVROCP                   BIT0

#define WKC_MSVIDALERT_REG                0x1D
#define WKC_MASK_MICCMAXVSYS              BIT2
#define WKC_MASK_MICCMAXVNN               BIT1
#define WKC_MASK_MICCMAXVCC               BIT0

#define WKC_SPWRSRC_REG                   0x20
#define WKC_MASK_SUSBIDDET                (BIT4|BIT3)
#define WKC_MASK_SBATTDET                 BIT2
#define WKC_MASK_SDCDET                   BIT1
#define WKC_MASK_SVBUSDET                 BIT0

#define WKC_LOWBATTDET0_REG               0x21
#define WKC_MASK_LOWBATT                  (BIT3|BIT2|BIT1|BIT0)

#define WKC_LOWBATTDET1_REG               0x22
#define WKC_MASK_LOWBATTDCP               (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_LOWBATTSDP               (BIT3|BIT2|BIT1|BIT0)

#define WKC_PSDETCTRL_REG                 0x23
#define WKC_MASK_BATTRMSRC                (BIT5|BIT4)
#define WKC_MASK_BATTRMPDEN               BIT3
#define WKC_MASK_VDCINDBEN                BIT1
#define WKC_MASK_VBUSDBEN                 BIT0

#define WKC_BATTDETCTRL0_REG              0x24
#define WKC_MASK_BATTYP                   (BIT7|BIT6|BIT5)
#define WKC_MASK_BATTDBEN                 BIT0

#define WKC_BATTDETCTRL1_REG              0x25
#define WKC_MASK_BTDSMART                 BIT7
#define WKC_MASK_BTDLC                    BIT6
#define WKC_MASK_BTDEN                    BIT5
#define WKC_MASK_BRMDBC                   (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_CHIPCNTRL_REG                 0x2A
#define WKC_MASK_PLATFORMRST              BIT3
#define WKC_MASK_WARMRESET                BIT2
#define WKC_MASK_COLDRESET                BIT1
#define WKC_MASK_COLDOFF                  BIT0

#define WKC_PLATFORMRST_DT_REG            0x2B
#define WKC_MASK_PRT                      (BIT3|BIT2|BIT1|BIT0)

#define WKC_RESETSRC0_REG                 0x2C
#define WKC_MASK_RPMICWDTEXP              BIT7
#define WKC_MASK_RBATTEMP                 BIT6
#define WKC_MASK_RVSYSUVP                 BIT5
#define WKC_MASK_RBATTRM                  BIT4
#define WKC_MASK_RVCRIT                   BIT3
#define WKC_MASK_RSYSTEMP                 BIT2
#define WKC_MASK_RPMICTEMP                BIT1
#define WKC_MASK_RTHERMTRIP               BIT0

#define WKC_RESETSRC1_REG                 0x2D
#define WKC_MASK_RPLATFORMRST             BIT5
#define WKC_MASK_RSOCWDT                  BIT4
#define WKC_MASK_RFCO                     BIT3
#define WKC_MASK_RWARMRESET               BIT2
#define WKC_MASK_RCOLDRESET               BIT1
#define WKC_MASK_RCOLDOFF                 BIT0

#define WKC_RESETRDYCTRL_REG              0x2E
#define WKC_MASK_WARMRSTRDY               BIT1
#define WKC_MASK_COLDRSTRDY               BIT0

#define WKC_SPLTIMER_REG                  0x28
#define WKC_SSPLTIMER_REG                 0x29
#define WKC_SRCWAKECFG_REG                0x30
#define WKC_MASK_SPLWAKEEN                BIT5
#define WKC_MASK_PWRONWAKEEN              BIT4
#define WKC_MASK_USBIDWAKEEN              BIT3
#define WKC_MASK_VDCINWAKEEN              BIT2
#define WKC_MASK_VBUSWAKEEN               BIT1
#define WKC_MASK_BATWAKEEN                BIT0

#define WKC_WAKESRC_REG                   0x31
#define WKC_MASK_WAKESPL                  BIT7
#define WKC_MASK_WAKEPWRON                BIT6
#define WKC_MASK_WAKEWA                   BIT5
#define WKC_MASK_WAKEUSBID                BIT4
#define WKC_MASK_WAKEVDCIN                BIT3
#define WKC_MASK_WAKEVBUS                 BIT2
#define WKC_MASK_WAKEBAT                  BIT1
#define WKC_MASK_WAKEPBTN                 BIT0

#define WKC_STDBYCNT_REG                  0x32
#define WKC_MASK_STDBYDEST                (BIT2|BIT1|BIT0)

#define WKC_MODEMCTRL_REG                 0x36
#define WKC_MASK_SDWNDRV                  BIT3
#define WKC_MASK_SDWN_N                   BIT2
#define WKC_MASK_MODEMRSTSEQ              BIT1
#define WKC_MASK_MODEMOFF                 BIT0

#define WKC_SOCCTRL_REG                   0x37
#define WKC_MASK_RESET_N                  BIT1
#define WKC_MASK_POWERGOOD                BIT0

#define WKC_BBCHGRCFG_REG                 0x38
#define WKC_MASK_BBCHGI                   (BIT4|BIT3)
#define WKC_MASK_BBCHGV                   (BIT2|BIT1)
#define WKC_MASK_BBCHGEN                  BIT0

#define WKC_FLTCFG_REG                    0x3D
#define WKC_MASK_FLTACT                   (BIT1|BIT0)

#define WKC_PBCONFIG1_REG                 0x3E
#define WKC_MASK_PBHOLD                   (BIT6|BIT5|BIT4)
#define WKC_MASK_FLT                      (BIT3|BIT2|BIT1|BIT0)

#define WKC_PBCONFIG2_REG                 0x3F
#define WKC_MASK_UIBTNDIS                 BIT2
#define WKC_MASK_PBDIS                    (BIT1|BIT0)

#define WKC_PBSTATUS_REG                  0x40
#define WKC_MASK_CLRHT                    BIT6
#define WKC_MASK_CLRFLT                   BIT5
#define WKC_MASK_PBLVL                    BIT4
#define WKC_MASK_PBHT                     (BIT3|BIT2|BIT1|BIT0)

#define WKC_UBSTATUS_REG                  0x41
#define WKC_MASK_UBLVL                    BIT4
#define WKC_MASK_UBHT                     (BIT3|BIT2|BIT1|BIT0)

#define WKC_GPIO0P0CTLO_REG               0x44
#define WKC_GPIO0P1CTLO_REG               0x45
#define WKC_GPIO0P2CTLO_REG               0x46
#define WKC_GPIO0P3CTLO_REG               0x47
#define WKC_GPIO0P4CTLO_REG               0x48
#define WKC_GPIO0P5CTLO_REG               0x49
#define WKC_GPIO0P6CTLO_REG               0x4A
#define WKC_GPIO1P0CTLO_REG               0x4B
#define WKC_GPIO1P1CTLO_REG               0x4C
#define WKC_GPIO1P2CTLO_REG               0x4D
#define WKC_GPIO1P3CTLO_REG               0x4E
#define WKC_GPIO2P0CTLO_REG               0x4F
#define WKC_GPIO2P1CTLO_REG               0x50
#define WKC_MASK_ALTFUNCEN                BIT6
#define WKC_MASK_DIR                      BIT5
#define WKC_MASK_DRV                      BIT4
#define WKC_MASK_REN                      BIT3
#define WKC_MASK_RVAL                     BIT1
#define WKC_MASK_DOUT                     BIT0

#define WKC_GPIO0P0CTLI_REG               0x51
#define WKC_GPIO0P1CTLI_REG               0x52
#define WKC_GPIO0P2CTLI_REG               0x53
#define WKC_GPIO0P3CTLI_REG               0x54
#define WKC_GPIO0P4CTLI_REG               0x55
#define WKC_GPIO0P5CTLI_REG               0x56
#define WKC_GPIO0P6CTLI_REG               0x57
#define WKC_GPIO1P0CTLI_REG               0x58
#define WKC_GPIO1P1CTLI_REG               0x59
#define WKC_GPIO1P2CTLI_REG               0x5A
#define WKC_GPIO1P3CTLI_REG               0x5B
#define WKC_GPIO2P0CTLI_REG               0x5C
#define WKC_GPIO2P1CTLI_REG               0x5D
#define WKC_MASK_GPIGLBYP                 BIT4
#define WKC_MASK_GPIDBNC                  BIT3
#define WKC_MASK_INTCNT                   (BIT2|BIT1)
#define WKC_MASK_DIN                      BIT0

#define WKC_VCCCNT_REG                    0x60
#define WKC_MASK_MODEVCC                  (BIT4|BIT3|BIT2|BIT1)

#define WKC_VNNCNT_REG                    0x61
#define WKC_MASK_MODEVNN                  (BIT4|BIT3|BIT2|BIT1)

#define WKC_VNNAONCNT_REG                 0x62
#define WKC_MASK_MODEVNNAON               (BIT4|BIT3|BIT2|BIT1)

#define WKC_VDD1CNT_REG                   0x63
#define WKC_MASK_MODEVDD1                 (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_VDD1EN                   BIT0

#define WKC_VDD1VSEL_REG                  0x64
#define WKC_VDD2CNT_REG                   0x65
#define WKC_MASK_MODEVDD2                 (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_VDD2EN                   BIT0

#define WKC_VDD2VSEL_REG                  0x66
#define WKC_VDD3CNT_REG                   0x67
#define WKC_MASK_MODEVDD3                 (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_VDD3EN                   BIT0

#define WKC_VDD3VSEL_REG                  0x68
#define WKC_MASK_VDD3VSEL                 (BIT2|BIT1|BIT0)

#define WKC_VCCRAMCNT_REG                 0x69
#define WKC_MASK_RAMPCTLVCCRAM            BIT5
#define WKC_MASK_MODEVCCRAM               (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_VCCRAMEN                 BIT0

#define WKC_VCCRAMVSEL_REG                0x6A
#define WKC_VMEMCNT_REG                   0x6B
#define WKC_MASK_MODEVMEM                 (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_VMEMEN                   BIT0

#define WKC_VMEMVSEL_REG                  0x6C
#define WKC_VFLEXCNT_REG                  0x6D
#define WKC_MASK_MODEVFLEX                (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_VFLEXEN                  BIT0

#define WKC_VFLEXVSEL_REG                 0x6E
#define WKC_MASK_VFLEXVSEL                (BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)
#define WKC_VPROG1ACNT_REG                0x6F
#define WKC_MASK_VPROG1ARMTCAP            BIT5
#define WKC_MASK_VPROG1AFASTSTUP          BIT4
#define WKC_MASK_VPROG1ADIS_PLD           BIT3
#define WKC_MASK_MODEVPROG1A              (BIT2|BIT1|BIT0)

#define WKC_VPROG1BCNT_REG                0x70
#define WKC_MASK_VPROG1BRMTCAP            BIT5
#define WKC_MASK_VPROG1BFASTSTUP          BIT4
#define WKC_MASK_VPROG1BDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG1B              (BIT2|BIT1|BIT0)

#define WKC_VPROG1CCNT_REG                0x71
#define WKC_MASK_VPROG1CRMTCAP            BIT5
#define WKC_MASK_VPROG1CFASTSTUP          BIT4
#define WKC_MASK_VPROG1CDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG1C              (BIT2|BIT1|BIT0)

#define WKC_VPROG1DCNT_REG                0x72
#define WKC_MASK_VPROG1DRMTCAP            BIT5
#define WKC_MASK_VPROG1DFASTSTUP          BIT4
#define WKC_MASK_VPROG1DDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG1D              (BIT2|BIT1|BIT0)

#define WKC_VPROG2ACNT_REG                0x73
#define WKC_MASK_VPROG2ARMTCAP            BIT5
#define WKC_MASK_VPROG2AFASTSTUP          BIT4
#define WKC_MASK_VPROG2ADIS_PLD           BIT3
#define WKC_MASK_MODEVPROG2A              (BIT2|BIT1|BIT0)

#define WKC_VPROG2BCNT_REG                0x74
#define WKC_MASK_VPROG2BRMTCAP            BIT5
#define WKC_MASK_VPROG2BFASTSTUP          BIT4
#define WKC_MASK_VPROG2BDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG2B              (BIT2|BIT1|BIT0)

#define WKC_VPROG2CCNT_REG                0x75
#define WKC_MASK_VPROG2CRMTCAP            BIT5
#define WKC_MASK_VPROG2CFASTSTUP          BIT4
#define WKC_MASK_VPROG2CDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG2C              (BIT2|BIT1|BIT0)

#define WKC_VPROG3ACNT_REG                0x76
#define WKC_MASK_VPROG3ARMTCAP            BIT5
#define WKC_MASK_VPROG3AFASTSTUP          BIT4
#define WKC_MASK_VPROG3ADIS_PLD           BIT3
#define WKC_MASK_MODEVPROG3A              (BIT2|BIT1|BIT0)

#define WKC_VPROG3BCNT_REG                0x77
#define WKC_MASK_VPROG3BRMTCAP            BIT5
#define WKC_MASK_VPROG3BFASTSTUP          BIT4
#define WKC_MASK_VPROG3BDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG3B              (BIT2|BIT1|BIT0)

#define WKC_VLDOCNT_REG                   0x78
#define WKC_MASK_MODEVPMIC                (BIT7|BIT6)
#define WKC_MASK_VSWITCH2EN               BIT5
#define WKC_MASK_VSWITCH1EN               BIT4
#define WKC_MASK_VPMIC_IN_SEL             (BIT3|BIT2)
#define WKC_MASK_VUSBPHYEN                BIT1

#define WKC_VSWITCHCNT0_REG               0x79
#define WKC_MASK_VSWITCH2DIS_PLD          BIT1
#define WKC_MASK_VSWITCH1DIS_PLD          BIT0

#define WKC_VSWITCHCNT1_REG               0x7A
#define WKC_MASK_VSWITCH2_DIV_SEL         (BIT6|BIT5|BIT4)
#define WKC_MASK_VSWITCH1_DIV_SEL         (BIT2|BIT1|BIT0)

#define WKC_VREFSOCCNT_REG                0x7B
#define WKC_MASK_VINSEL                   (BIT7|BIT6)
#define WKC_MASK_MODEVREFSOC              (BIT2|BIT1|BIT0)

#define WKC_VPROG1ECNT_REG                0xA0
#define WKC_MASK_VPROG1ERMTCAP            BIT5
#define WKC_MASK_VPROG1EFASTSTUP          BIT4
#define WKC_MASK_VPROG1EDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG1E              (BIT2|BIT1|BIT0)

#define WKC_VPROG1FCNT_REG                0xA1
#define WKC_MASK_VPROG1FRMTCAP            BIT5
#define WKC_MASK_VPROG1FFASTSTUP          BIT4
#define WKC_MASK_VPROG1FDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG1F              (BIT2|BIT1|BIT0)

#define WKC_VPROG2DCNT_REG                0xA2
#define WKC_MASK_VPROG2DRMTCAP            BIT5
#define WKC_MASK_VPROG2DFASTSTUP          BIT4
#define WKC_MASK_VPROG2DDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG2D              (BIT2|BIT1|BIT0)

#define WKC_VPROG4ACNT_REG                0xA3
#define WKC_MASK_VPROG4ARMTCAP            BIT5
#define WKC_MASK_VPROG4AFASTSTUP          BIT4
#define WKC_MASK_VPROG4ADIS_PLD           BIT3
#define WKC_MASK_MODEVPROG4A              (BIT2|BIT1|BIT0)

#define WKC_VPROG4BCNT_REG                0xA4
#define WKC_MASK_VPROG4BRMTCAP            BIT5
#define WKC_MASK_VPROG4BFASTSTUP          BIT4
#define WKC_MASK_VPROG4BDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG4B              (BIT2|BIT1|BIT0)

#define WKC_VPROG4CCNT_REG                0xA5
#define WKC_MASK_VPROG4CRMTCAP            BIT5
#define WKC_MASK_VPROG4CFASTSTUP          BIT4
#define WKC_MASK_VPROG4CDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG4C              (BIT2|BIT1|BIT0)

#define WKC_VPROG4DCNT_REG                0xA6
#define WKC_MASK_VPROG4DRMTCAP            BIT5
#define WKC_MASK_VPROG4DFASTSTUP          BIT4
#define WKC_MASK_VPROG4DDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG4D              (BIT2|BIT1|BIT0)

#define WKC_VPROG5ACNT_REG                0xA7
#define WKC_MASK_VPROG5ARMTCAP            BIT5
#define WKC_MASK_VPROG5AFASTSTUP          BIT4
#define WKC_MASK_VPROG5ADIS_PLD           BIT3
#define WKC_MASK_MODEVPROG5A              (BIT2|BIT1|BIT0)

#define WKC_VPROG5BCNT_REG                0xA8
#define WKC_MASK_VPROG5BRMTCAP            BIT5
#define WKC_MASK_VPROG5BFASTSTUP          BIT4
#define WKC_MASK_VPROG5BDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG5B              (BIT2|BIT1|BIT0)

#define WKC_VPROG6ACNT_REG                0xA9
#define WKC_MASK_VPROG6ARMTCAP            BIT5
#define WKC_MASK_VPROG6AFASTSTUP          BIT4
#define WKC_MASK_VPROG6ADIS_PLD           BIT3
#define WKC_MASK_MODEVPROG6A              (BIT2|BIT1|BIT0)

#define WKC_VPROG6BCNT_REG                0xAA
#define WKC_MASK_VPROG6BRMTCAP            BIT5
#define WKC_MASK_VPROG6BFASTSTUP          BIT4
#define WKC_MASK_VPROG6BDIS_PLD           BIT3
#define WKC_MASK_MODEVPROG6B              (BIT2|BIT1|BIT0)

#define WKC_VPROG1AVSEL_REG               0xB6
#define WKC_MASK_VPROG1AVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG1BVSEL_REG               0xB7
#define WKC_MASK_VPROG1BVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG1CVSEL_REG               0xB8
#define WKC_MASK_VPROG1CVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG1DVSEL_REG               0xB9
#define WKC_MASK_VPROG1DVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG1EVSEL_REG               0xBA
#define WKC_MASK_VPROG1EVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG1FVSEL_REG               0xBB
#define WKC_MASK_VPROG1FVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG2AVSEL_REG               0xBC
#define WKC_MASK_VPROG2AVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG2BVSEL_REG               0xBD
#define WKC_MASK_VPROG2BVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG2CVSEL_REG               0xBE
#define WKC_MASK_VPROG2CVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG2DVSEL_REG               0xBF
#define WKC_MASK_VPROG2DVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG3AVSEL_REG               0xC0
#define WKC_MASK_VPROG3AVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG3BVSEL_REG               0xC1
#define WKC_MASK_VPROG3BVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG4AVSEL_REG               0xC2
#define WKC_MASK_VPROG4AVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG4BVSEL_REG               0xC3
#define WKC_MASK_VPROG4BVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG4CVSEL_REG               0xC4
#define WKC_MASK_VPROG4CVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG4DVSEL_REG               0xC5
#define WKC_MASK_VPROG4DVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG5AVSEL_REG               0xC6
#define WKC_MASK_VPROG5AVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG5BVSEL_REG               0xC7
#define WKC_MASK_VPROG5BVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG6AVSEL_REG               0xC8
#define WKC_MASK_VPROG6AVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VPROG6BVSEL_REG               0xC9
#define WKC_MASK_VPROG6BVSEL              (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VWARN1_CFG_REG                0x80
#define WKC_MASK_VW1_ENTRY_DG             BIT7
#define WKC_MASK_VWARN1_DB                (BIT6|BIT5|BIT4)
#define WKC_MASK_VWARN1_EN                BIT3
#define WKC_MASK_VWARN1                   (BIT2|BIT1|BIT0)

#define WKC_VWARN2_CFG_REG                0x81
#define WKC_MASK_VW2_ENTRY_DG             BIT7
#define WKC_MASK_VWARN2_DB                (BIT6|BIT5|BIT4)
#define WKC_MASK_VWARN2_EN                BIT3
#define WKC_MASK_VWARN2                   (BIT2|BIT1|BIT0)

#define WKC_VCRIT_CFG_REG                 0x82
#define WKC_MASK_VCRIT_ENTRY_DG           BIT7
#define WKC_MASK_VCRIT_DB                 (BIT6|BIT5)
#define WKC_MASK_VCRIT_SDWN               BIT4
#define WKC_MASK_VCRIT_EN                 BIT3
#define WKC_MASK_VCRIT                    (BIT2|BIT1|BIT0)

#define WKC_BCUDISW2_BEH_REG              0x83
#define WKC_MASK_BCUDISW2_STICKY          BIT2
#define WKC_MASK_BCUDISW2_POL             BIT1
#define WKC_MASK_BCUDISW2_EN              BIT0

#define WKC_BCUDISCRIT_BEH_REG            0x84
#define WKC_MASK_BCUDISCRIT_STICKY        BIT2
#define WKC_MASK_BCUDISCRIT_POL           BIT1
#define WKC_MASK_BCUDISCRIT_EN            BIT0

#define WKC_S_BCUIRQ_REG                  0x8B
#define WKC_MASK_S_TXPWRTH                BIT4
#define WKC_MASK_S_GSMPULSE               BIT3
#define WKC_MASK_S_VCRIT                  BIT2
#define WKC_MASK_S_VWARN2                 BIT1
#define WKC_MASK_S_VWARN1                 BIT0

#define WKC_S_BCUCTRL_REG                 0x8C
#define WKC_MASK_S_BCUDISW2               BIT1
#define WKC_MASK_S_BCUDISCRIT             BIT0

#define WKC_ICCMAXVSYS_CFG_REG            0x8D
#define WKC_MASK_ICCMAXVSYS_2GVW1_EN      BIT2
#define WKC_MASK_ICCMAXVSYS_VW2_EN        BIT1
#define WKC_MASK_ICCMAXVSYS_VW1_EN        BIT0

#define WKC_ICCMAXVNN_CFG_REG             0x90
#define WKC_MASK_ICCMAXVNN_EN             BIT7
#define WKC_MASK_ICCMAXVNN_DB             (BIT6|BIT5|BIT4)
#define WKC_MASK_ICCMAXVNN                (BIT3|BIT2|BIT1|BIT0)

#define WKC_ICCMAXVCC_CFG_REG             0x91
#define WKC_MASK_ICCMAXVCC_EN             BIT7
#define WKC_MASK_ICCMAXVCC_DB             (BIT6|BIT5|BIT4)
#define WKC_MASK_ICCMAXVCC                (BIT3|BIT2|BIT1|BIT0)

#define WKC_SOCWDTCFG_REG                 0x92
#define WKC_MASK_SOCWDTACT                (BIT1|BIT0)

#define WKC_DEV1_STATUS_1_REG             0x10
#define WKC_MASK_DEV1_READ_STATUS2        BIT7
#define WKC_MASK_DEV1_RESERVED            (BIT2|BIT1|BIT0)

#define WKC_DEV1_STATUS_2_REG             0x11
#define WKC_MASK_DEV1_DATA_FRAME_ERROR    BIT1
#define WKC_MASK_DEV1_PARITY_ERROR        BIT0

#define WKC_DEV1_STATUS2_LASTREAD_REG     0x1C
#define WKC_MASK_DEV1_STATUS_2_ON_LAST_GETREG     (BIT1|BIT0)

#define WKC_DEV1_SETREGADR_REG            0x35

//
// Device2
//
#define WKC_TRMPGM_REG                    0x00
#define WKC_OTPVERSION_REG                0x01
#define WKC_PLTSEL_REG                    0xFE
#define WKC_MASK_PLTSEL                   BIT0

#define WKC_PROVERSION_REG                0xFF
#define WKC_GPADCREQ_REG                  0x02
#define WKC_MASK_ADC_PEAK                 BIT7
#define WKC_MASK_ADC_GPMEAS               BIT6
#define WKC_MASK_ADC_VBATT                BIT5
#define WKC_MASK_ADC_BATTID               BIT4
#define WKC_MASK_ADC_SYSTEMP              BIT3
#define WKC_MASK_ADC_BATTEMP              BIT2
#define WKC_MASK_ADC_USBID                BIT1
#define WKC_MASK_BUSY                     BIT0

#define WKC_VBATRSLTH_REG                 0x03
#define WKC_MASK_VBAT_H                   (BIT3|BIT2|BIT1|BIT0)

#define WKC_VBATRSLTL_REG                 0x04
#define WKC_GPADCCNTL_REG                 0x05
#define WKC_MASK_WDT_DIS                  BIT1
#define WKC_MASK_VREF_DIS                 BIT0

#define WKC_BATTIDRSLTH_REG               0x06
#define WKC_MASK_BATTID_CURSRC            (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BATTID_H                 (BIT3|BIT2|BIT1|BIT0)

#define WKC_BATTIDRSLTL_REG               0x07
#define WKC_MASK_USBID_CURSRC             (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_USBID_H                  (BIT3|BIT2|BIT1|BIT0)

#define WKC_USBIDRSLTH_REG                0x08
#define WKC_USBIDRSLTL_REG                0x09
#define WKC_GPMEASRSLTH_REG               0x0A
#define WKC_MASK_GPMEAS_CURSRC            (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_GPMEAS_H                 (BIT3|BIT2|BIT1|BIT0)

#define WKC_GPMEASRSLTL_REG               0x0B
#define WKC_Y0DATAH_REG                   0x0C
#define WKC_MASK_Y0_CURSRC                (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Y0_H                     (BIT3|BIT2|BIT1|BIT0)

#define WKC_Y0DATAL_REG                   0x0D
#define WKC_Y1DATAH_REG                   0x0E
#define WKC_MASK_Y1_CURSRC                (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Y1_H                     (BIT3|BIT2|BIT1|BIT0)

#define WKC_Y1DATAL_REG                   0x0F
#define WKC_PEAKREQ_REG                   0x12
#define WKC_MASK_PEAKTEST                 BIT4
#define WKC_MASK_RDOUTTEST                BIT3
#define WKC_MASK_IN2OUTTEST               BIT2
#define WKC_MASK_MAXDETECT                BIT1
#define WKC_MASK_MINDETECT                BIT0

#define WKC_PEAKRSLTH_REG                 0x13
#define WKC_MASK_PEAK_CURSRC              (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_PEAK_H                   (BIT3|BIT2|BIT1|BIT0)

#define WKC_PEAKRSLTL_REG                 0x14
#define WKC_BATTEMP0H_REG                 0x15
#define WKC_MASK_BATTEMP0_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_BATTEMP0L_REG                 0x16
#define WKC_BATTEMP1H_REG                 0x17
#define WKC_MASK_BATTEMP1_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_BATTEMP1L_REG                 0x18
#define WKC_STHRMIRQ0_REG                 0x19
#define WKC_MASK_GPMEASALRT               BIT7
#define WKC_MASK_SSYS2ALRT1               BIT6
#define WKC_MASK_SSYS1ALRT1               BIT5
#define WKC_MASK_SSYS0ALRT1               BIT4
#define WKC_MASK_SPMICALRT0               BIT3
#define WKC_MASK_SSYS2ALRT0               BIT2
#define WKC_MASK_SSYS1ALRT0               BIT1
#define WKC_MASK_SSYS0ALRT0               BIT0

#define WKC_STHRMIRQ1_REG                 0x1A
#define WKC_MASK_SBATCRITHOTCOLD          BIT5
#define WKC_MASK_SBATCRIT                 BIT4
#define WKC_MASK_SPMICCRIT                BIT3
#define WKC_MASK_SSYS2CRIT                BIT2
#define WKC_MASK_SSYS1CRIT                BIT1
#define WKC_MASK_SSYS0CRIT                BIT0

#define WKC_STHRMIRQ2_REG                 0x1B
#define WKC_MASK_SBAT1ALRT3               BIT7
#define WKC_MASK_SBAT0ALRT3               BIT6
#define WKC_MASK_SBAT1ALRT0               BIT5
#define WKC_MASK_SBAT0ALRT0               BIT4
#define WKC_MASK_SPMICALRT3               BIT3
#define WKC_MASK_SSYS2ALRT3               BIT2
#define WKC_MASK_SSYS1ALRT3               BIT1
#define WKC_MASK_SSYS0ALRT3               BIT0

#define WKC_THRMMONCFG_REG                0x1D
#define WKC_MASK_THRMSRC                  BIT1
#define WKC_MASK_NUMBAT                   BIT0

#define WKC_THRMMONCTL_REG                0x1E
#define WKC_MASK_SYSFRQS                  (BIT4|BIT3)
#define WKC_MASK_SYSFRQA                  (BIT1|BIT0)

#define WKC_BATTHERMMONCTL_REG            0x1F
#define WKC_MASK_BATFRQC                  (BIT6|BIT5)
#define WKC_MASK_BATFRQS                  (BIT4|BIT3)
#define WKC_MASK_BATFRQA                  (BIT1|BIT0)

#define WKC_VBATMONCTL_REG                0x20
#define WKC_MASK_VBATFRQS                 (BIT4|BIT3)
#define WKC_MASK_VBATFRQA                 (BIT1|BIT0)

#define WKC_GPMONCTL_REG                  0x21
#define WKC_MASK_GPFRQS                   (BIT3|BIT2)
#define WKC_MASK_GPFRQA                   (BIT1|BIT0)

#define WKC_THRMBATZONE_REG               0x22
#define WKC_MASK_BATZONE                  (BIT2|BIT1|BIT0)

#define WKC_SYS0ALERT0H_REG               0x23
#define WKC_MASK_SYS0ALERT0_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS0ALERT0_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS0ALERT0_H             BIT0

#define WKC_SYS0ALERT0L_REG               0x24
#define WKC_SYS0ALERT1H_REG               0x25
#define WKC_MASK_SYS0ALERT1_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS0ALERT1_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS0ALERT1_H             BIT0

#define WKC_SYS0ALERT1L_REG               0x26
#define WKC_SYS1ALERT0H_REG               0x27
#define WKC_MASK_SYS1ALERT0_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS1ALERT0_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS1ALERT0_H             BIT0

#define WKC_SYS1ALERT0L_REG               0x28
#define WKC_SYS1ALERT1H_REG               0x29
#define WKC_MASK_SYS1ALERT1_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS1ALERT1_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS1ALERT1_H             BIT0

#define WKC_SYS1ALERT1L_REG               0x2A
#define WKC_SYS2ALERT0H_REG               0x2B
#define WKC_MASK_SYS2ALERT0_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS2ALERT0_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS2ALERT0_H             BIT0

#define WKC_SYS2ALERT0L_REG               0x2C
#define WKC_SYS2ALERT1H_REG               0x2D
#define WKC_MASK_SYS2ALERT1_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS2ALERT1_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS2ALERT1_H             BIT0

#define WKC_SYS2ALERT1L_REG               0x2E
#define WKC_BAT0ALERT0H_REG               0x2F
#define WKC_MASK_BAT0ALERT0_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BAT0ALERT0_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_BAT0ALERT0_H             BIT0

#define WKC_BAT0ALERT0L_REG               0x30
#define WKC_BAT1ALERT0H_REG               0x31
#define WKC_MASK_BAT1ALERT0_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BAT1ALERT0_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_BAT1ALERT0_H             BIT0

#define WKC_BAT1ALERT0L_REG               0x32
#define WKC_PMICALERT0H_REG               0x33
#define WKC_MASK_PMICALERT0_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_PMICALERT0_H             (BIT3|BIT2|BIT1|BIT0)

#define WKC_PMICALERT0L_REG               0x34
#define WKC_GPMEASALERTH_REG              0x36
#define WKC_MASK_GPALERT_HYS              (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_GPALERT_CURHYS           (BIT3|BIT2|BIT1)
#define WKC_MASK_GPALERT_H                BIT0

#define WKC_GPMEASALERTL_REG              0x37
#define WKC_THRMRSLT0H_REG                0x38
#define WKC_MASK_SYS0CURSRC               (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS0TEMP_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_THRMRSLT0L_REG                0x39
#define WKC_THRMRSLT1H_REG                0x3A
#define WKC_MASK_SYS1CURSRC               (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS1TEMP_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_THRMRSLT1L_REG                0x3B
#define WKC_THRMRSLT2H_REG                0x3C
#define WKC_MASK_SYS2CURSRC               (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS2TEMP_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_THRMRSLT2L_REG                0x3D
#define WKC_THRMRSLT3H_REG                0x3E
#define WKC_MASK_BAT0TEMPCURSRC           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_THERM_BAT0TEMP_H         (BIT3|BIT2|BIT1|BIT0)

#define WKC_THRMRSLT3L_REG                0x3F
#define WKC_THRMRSLT4H_REG                0x40
#define WKC_MASK_BAT1TEMPCURSRC           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_THERM_BAT1TEMP_H         (BIT3|BIT2|BIT1|BIT0)

#define WKC_THRMRSLT4L_REG                0x41
#define WKC_THRMRSLT5H_REG                0x42
#define WKC_MASK_PMICTEMPCURSRC           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_PMICTEMP_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_THRMRSLT5L_REG                0x43
#define WKC_THRMZN0H_REG                  0x44
#define WKC_MASK_Z0HYS                    (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Z0CURHYS                 (BIT3|BIT2|BIT1)
#define WKC_MASK_TCOLD_H                  BIT0

#define WKC_THRMZN0L_REG                  0x45
#define WKC_THRMZN1H_REG                  0x46
#define WKC_MASK_Z1HYS                    (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Z1CURHYS                 (BIT3|BIT2|BIT1)
#define WKC_MASK_TCOOL_H                  BIT0

#define WKC_THRMZN1L_REG                  0x47
#define WKC_THRMZN2H_REG                  0x48
#define WKC_MASK_Z2HYS                    (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Z2CURHYS                 (BIT3|BIT2|BIT1)
#define WKC_MASK_TWARM_H                  BIT0

#define WKC_THRMZN2L_REG                  0x49
#define WKC_THRMZN3H_REG                  0x4A
#define WKC_MASK_Z3HYS                    (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Z3CURHYS                 (BIT3|BIT2|BIT1)
#define WKC_MASK_THOT_H                   BIT0

#define WKC_THRMZN3L_REG                  0x4B
#define WKC_THRMZN4H_REG                  0x4C
#define WKC_MASK_Z4HYS                    (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_Z4CURHYS                 (BIT3|BIT2|BIT1)
#define WKC_MASK_TEMERGHIGH_H             BIT0

#define WKC_THRMZN4L_REG                  0x4D
#define WKC_BATCRITCOLDH_REG              0x4E
#define WKC_MASK_BATCRITCHYS              (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BATCRITCCURHYS           (BIT3|BIT2|BIT1)
#define WKC_MASK_BATCRITCOLD_H            BIT0

#define WKC_BATCRITCOLDL_REG              0x4F
#define WKC_BATCRITHOTH_REG               0x50
#define WKC_MASK_BATCRITHHYS              (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BATCRITHCURHYS           (BIT3|BIT2|BIT1)
#define WKC_MASK_BATCRITHOT_H             BIT0

#define WKC_BATCRITHOTL_REG               0x51
#define WKC_BATCRITH_REG                  0x52
#define WKC_MASK_BATCRITCURHYS            (BIT3|BIT2|BIT1)
#define WKC_MASK_BATCRIT_H                BIT0

#define WKC_BATCRITL_REG                  0x53
#define WKC_PMICCRITH_REG                 0x54
#define WKC_MASK_PMICCRIT_H               (BIT3|BIT2|BIT1|BIT0)

#define WKC_PMICCRITL_REG                 0x55
#define WKC_SYS0CRITH_REG                 0x56
#define WKC_MASK_SYS0CRITCURHYS           (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS0CRIT_H               BIT0

#define WKC_SYS0CRITL_REG                 0x57
#define WKC_SYS1CRITH_REG                 0x58
#define WKC_MASK_SYS1CRITCURHYS           (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS1CRIT_H               BIT0

#define WKC_SYS1CRITL_REG                 0x59
#define WKC_SYS2CRITH_REG                 0x5A
#define WKC_MASK_SYS2CRITCURHYS           (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS2CRIT_H               BIT0

#define WKC_SYS2CRITL_REG                 0x5B
#define WKC_SYS0ALERT3H_REG               0x5C
#define WKC_MASK_SYS0ALERT3_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS0ALERT3_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS0ALERT3_H             BIT0

#define WKC_SYS0ALERT3L_REG               0x5D
#define WKC_SYS1ALERT3H_REG               0x5E
#define WKC_MASK_SYS1ALERT3_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS1ALERT3_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS1ALERT3_H             BIT0

#define WKC_SYS1ALERT3L_REG               0x5F
#define WKC_SYS2ALERT3H_REG               0x60
#define WKC_MASK_SYS2ALERT3_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_SYS2ALERT3_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_SYS2ALERT3_H             BIT0

#define WKC_SYS2ALERT3L_REG               0x61
#define WKC_PMICALERT3H_REG               0x62
#define WKC_MASK_PMICALERT3_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_PMICALERT3_H             (BIT3|BIT2|BIT1|BIT0)

#define WKC_PMICALERT3L_REG               0x63
#define WKC_BAT0ALERT3H_REG               0x64
#define WKC_MASK_BAT0ALERT3_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BAT0ALERT3_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_BAT0ALERT3_H             BIT0

#define WKC_BAT0ALERT3L_REG               0x65
#define WKC_BAT1ALERT3H_REG               0x66
#define WKC_MASK_BAT1ALERT3_HYS           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_BAT1ALERT3_CURHYS        (BIT3|BIT2|BIT1)
#define WKC_MASK_BAT1ALERT3_H             BIT0

#define WKC_BAT1ALERT3L_REG               0x67
#define WKC_THRMCRITEN_REG                0x68
#define WKC_MASK_BAT_CRIT_EN              BIT4
#define WKC_MASK_PMIC_CRIT_EN             BIT3
#define WKC_MASK_SYS2_CRIT_EN             BIT2
#define WKC_MASK_SYS1_CRIT_EN             BIT1
#define WKC_MASK_SYS0_CRIT_EN             BIT0

#define WKC_THRMALERT3EN_REG              0x69
#define WKC_MASK_BAT1_A3_EN               BIT5
#define WKC_MASK_BAT0_A3_EN               BIT4
#define WKC_MASK_PMIC_A3_EN               BIT3
#define WKC_MASK_SYS2_A3_EN               BIT2
#define WKC_MASK_SYS1_A3_EN               BIT1
#define WKC_MASK_SYS0_A3_EN               BIT0

#define WKC_THRMALERT3PAEN_REG            0x6A
#define WKC_MASK_BAT1_A3P_EN              BIT5
#define WKC_MASK_BAT0_A3P_EN              BIT4
#define WKC_MASK_PMIC_A3P_EN              BIT3
#define WKC_MASK_SYS2_A3P_EN              BIT2
#define WKC_MASK_SYS1_A3P_EN              BIT1
#define WKC_MASK_SYS0_A3P_EN              BIT0

#define WKC_CM_VCC0_MODE_CTRL_REG         0x6B
#define WKC_MASK_VCC0_CM_MODE             (BIT3|BIT2)
#define WKC_MASK_VCC0_CLR_DATA            BIT1
#define WKC_MASK_VCC0_CM_EN               BIT0

#define WKC_CM_VCC1_MODE_CTRL_REG         0x6C
#define WKC_MASK_VCC1_CM_MODE             (BIT3|BIT2)
#define WKC_MASK_VCC1_CLR_DATA            BIT1
#define WKC_MASK_VCC1_CM_EN               BIT0

#define WKC_CM_VCC_MODE_CTRL_REG          0x6D
#define WKC_MASK_VCC_CM_MODE              (BIT3|BIT2)
#define WKC_MASK_VCC_CLR_DATA             BIT1
#define WKC_MASK_VCC_CM_EN                BIT0

#define WKC_CM_VNN_MODE_CTRL_REG          0x6E
#define WKC_MASK_VNN_CM_MODE              (BIT3|BIT2)
#define WKC_MASK_VNN_CLR_DATA             BIT1
#define WKC_MASK_VNN_CM_EN                BIT0

#define WKC_CM_VNNAON_MODE_CTRL_REG       0x6F
#define WKC_MASK_VNNAON_CM_MODE           (BIT3|BIT2)
#define WKC_MASK_VNNAON_CLR_DATA          BIT1
#define WKC_MASK_VNNAON_CM_EN             BIT0

#define WKC_CM_VDD1_MODE_CTRL_REG         0x70
#define WKC_MASK_VDD1_CM_MODE             (BIT3|BIT2)
#define WKC_MASK_VDD1_CLR_DATA            BIT1
#define WKC_MASK_VDD1_CM_EN               BIT0

#define WKC_CM_VDD2_MODE_CTRL_REG         0x71
#define WKC_MASK_VDD2_CM_MODE             (BIT3|BIT2)
#define WKC_MASK_VDD2_CLR_DATA            BIT1
#define WKC_MASK_VDD2_CM_EN               BIT0

#define WKC_CM_VCCRAM_MODE_CTRL_REG       0x72
#define WKC_MASK_VCCRAM_CM_MODE           (BIT3|BIT2)
#define WKC_MASK_VCCRAM_CLR_DATA          BIT1
#define WKC_MASK_VCCRAM_CM_EN             BIT0

#define WKC_CM_VMEM_MODE_CTRL_REG         0x73
#define WKC_MASK_VMEM_CM_MODE             (BIT3|BIT2)
#define WKC_MASK_VMEM_CLR_DATA            BIT1
#define WKC_MASK_VMEM_CM_EN               BIT0

#define WKC_CM_VFLEX_MODE_CTRL_REG        0x74
#define WKC_MASK_VFLEX_CM_MODE_L          (BIT3|BIT2)
#define WKC_MASK_VFLEX_CLR_DATA           BIT1
#define WKC_MASK_VFLEX_CM_EN              BIT0

#define WKC_CM_VPROG1A_MODE_CTRL_REG      0x75
#define WKC_MASK_VPROG1A_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG1A_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG1A_CLR_DATA         BIT1
#define WKC_MASK_VPROG1A_CM_EN            BIT0

#define WKC_CM_VPROG1B_MODE_CTRL_REG      0x76
#define WKC_MASK_VPROG1B_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG1B_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG1B_CLR_DATA         BIT1
#define WKC_MASK_VPROG1B_CM_EN            BIT0

#define WKC_CM_VPROG1C_MODE_CTRL_REG      0x77
#define WKC_MASK_VPROG1C_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG1C_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG1C_CLR_DATA         BIT1
#define WKC_MASK_VPROG1C_CM_EN            BIT0

#define WKC_CM_VPROG1D_MODE_CTRL_REG      0x78
#define WKC_MASK_VPROG1D_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG1D_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG1D_CLR_DATA         BIT1
#define WKC_MASK_VPROG1D_CM_EN            BIT0

#define WKC_CM_VPROG2A_MODE_CTRL_REG      0x79
#define WKC_MASK_VPROG2A_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG2A_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG2A_CLR_DATA         BIT1
#define WKC_MASK_VPROG2A_CM_EN            BIT0

#define WKC_CM_VPROG2B_MODE_CTRL_REG      0x7A
#define WKC_MASK_VPROG2B_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG2B_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG2B_CLR_DATA         BIT1
#define WKC_MASK_VPROG2B_CM_EN            BIT0

#define WKC_CM_VPROG2C_MODE_CTRL_REG      0x7B
#define WKC_MASK_VPROG2C_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG2C_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG2C_CLR_DATA         BIT1
#define WKC_MASK_VPROG2C_CM_EN            BIT0

#define WKC_CM_VPROG3A_MODE_CTRL_REG      0x7C
#define WKC_MASK_VPROG3A_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG3A_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG3A_CLR_DATA         BIT1
#define WKC_MASK_VPROG3A_CM_EN            BIT0

#define WKC_CM_VPROG3B_MODE_CTRL_REG      0x7D
#define WKC_MASK_VPROG3B_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG3B_CM_MODE_L        (BIT3|BIT2)
#define WKC_MASK_VPROG3B_CLR_DATA         BIT1
#define WKC_MASK_VPROG3B_CM_EN            BIT0

#define WKC_CM_TSPAN_CTRL_REG             0x7E
#define WKC_MASK_CM_SUSPEND               BIT7
#define WKC_MASK_TSPAN_EN                 BIT4
#define WKC_MASK_TSPAN                    (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_LDO1_THRSHLD_REG           0x7F
#define WKC_MASK_VPROG1D_CM_THRSHLD       (BIT7|BIT6)
#define WKC_MASK_VPROG1C_CM_THRSHLD       (BIT5|BIT4)
#define WKC_MASK_VPROG1B_CM_THRSHLD       (BIT3|BIT2)
#define WKC_MASK_VPROG1A_CM_THRSHLD       (BIT1|BIT0)

#define WKC_CM_LDO2_THRSHLD_REG           0x80
#define WKC_MASK_VPROG2C_CM_THRSHLD       (BIT5|BIT4)
#define WKC_MASK_VPROG2B_CM_THRSHLD       (BIT3|BIT2)
#define WKC_MASK_VPROG2A_CM_THRSHLD       (BIT1|BIT0)

#define WKC_CM_LDO3_THRSHLD_REG           0x81
#define WKC_MASK_VPROG3B_CM_THRSHLD       (BIT3|BIT2)
#define WKC_MASK_VPROG3A_CM_THRSHLD       (BIT1|BIT0)

#define WKC_CM_SMPS0_THRSHLD_REG          0x82
#define WKC_MASK_VCC1_CM_THRSHLD          (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_VCC0_CM_THRSHLD          (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_SMPS1_THRSHLD_REG          0x83
#define WKC_MASK_VCC_CM_THRSHLD           (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_SMPS2_THRSHLD_REG          0x84
#define WKC_MASK_VNNAON_CM_THRSHLD        (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_VNN_CM_THRSHLD           (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_SMPS3_THRSHLD_REG          0x85
#define WKC_MASK_VDD2_CM_THRSHLD          (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_VDD1_CM_THRSHLD          (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_SMPS4_THRSHLD_REG          0x86
#define WKC_MASK_VMEM_CM_THRSHLD          (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_VCCRAM_CM_THRSHLD        (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_SMPS5_THRSHLD_REG          0x87
#define WKC_MASK_VFLEX_CM_THRSHLD         (BIT3|BIT2|BIT1|BIT0)

#define WKC_CM_VCC0_DATA_REG              0x88
#define WKC_CM_VCC1_DATA_REG              0x89
#define WKC_CM_VCC_DATA_REG               0x8A
#define WKC_CM_VNN_DATA_REG               0x8B
#define WKC_CM_VNNAON_DATA_REG            0x8C
#define WKC_CM_VDD1_DATA_REG              0x8D
#define WKC_CM_VDD2_DATA_REG              0x8E
#define WKC_CM_VCCRAM_DATA_REG            0x8F
#define WKC_CM_VMEM_DATA_REG              0x90
#define WKC_CM_VFLEX_DATA_REG             0x91
#define WKC_CM_PROG1A_DATA_REG            0x92
#define WKC_CM_PROG1B_DATA_REG            0x93
#define WKC_CM_PROG1C_DATA_REG            0x94
#define WKC_CM_PROG1D_DATA_REG            0x95
#define WKC_CM_PROG2A_DATA_REG            0x96
#define WKC_CM_PROG2B_DATA_REG            0x97
#define WKC_CM_PROG2C_DATA_REG            0x98
#define WKC_CM_PROG3A_DATA_REG            0x99
#define WKC_CM_PROG3B_DATA_REG            0x9A
#define WKC_CM_VCC0_INT_LVL_REG           0x9B
#define WKC_CM_VCC1_INT_LVL_REG           0x9C
#define WKC_CM_VCC_INT_LVL_REG            0x9D
#define WKC_CM_VNN_INT_LVL_REG            0x9E
#define WKC_CM_VNNAON_INT_LVL_REG         0x9F
#define WKC_CM_VDD1_INT_LVL_REG           0xA0
#define WKC_CM_VDD2_INT_LVL_REG           0xA1
#define WKC_CM_VCCRAM_INT_LVL_REG         0xA2
#define WKC_CM_VMEM_INT_LVL_REG           0xA3
#define WKC_CM_VFLEX_INT_LVL_REG          0xA4
#define WKC_CM_VPROG1A_INT_LVL_REG        0xA5
#define WKC_CM_VPROG1B_INT_LVL_REG        0xA6
#define WKC_CM_VPROG1C_INT_LVL_REG        0xA7
#define WKC_CM_VPROG1D_INT_LVL_REG        0xA8
#define WKC_CM_VPROG2A_INT_LVL_REG        0xA9
#define WKC_CM_VPROG2B_INT_LVL_REG        0xAA
#define WKC_CM_VPROG2C_INT_LVL_REG        0xAB
#define WKC_CM_VPROG3A_INT_LVL_REG        0xAC
#define WKC_CM_VPROG3B_INT_LVL_REG        0xAD
#define WKC_CM_VROCIRQSTAT0_REG           0xAE
#define WKC_MASK_SVDD2                    BIT6
#define WKC_MASK_SVDD1                    BIT5
#define WKC_MASK_SVNNAON                  BIT4
#define WKC_MASK_SVNN                     BIT3
#define WKC_MASK_SVCC                     BIT2
#define WKC_MASK_SVCC1                    BIT1
#define WKC_MASK_SVCC0                    BIT0

#define WKC_CM_VROCIRQSTAT1_REG           0xAF
#define WKC_MASK_SVFLEX                   BIT2
#define WKC_MASK_SVMEM                    BIT1
#define WKC_MASK_SVCCRAM                  BIT0

#define WKC_CM_VROCIRQSTAT2_REG           0xB0
#define WKC_MASK_SVPROG2C                 BIT6
#define WKC_MASK_SVPROG2B                 BIT5
#define WKC_MASK_SVPROG2A                 BIT4
#define WKC_MASK_SVPROG1D                 BIT3
#define WKC_MASK_SVPROG1C                 BIT2
#define WKC_MASK_SVPROG1B                 BIT1
#define WKC_MASK_SVPROG1A                 BIT0

#define WKC_CM_VROCIRQSTAT3_REG           0xB1
#define WKC_MASK_SVPROG3B                 BIT1
#define WKC_MASK_SVPROG3A                 BIT0

#define WKC_VRTCCTRL_REG                  0xB5
#define WKC_MASK_VRTCRLD                  BIT1
#define WKC_MASK_VRTCRST                  BIT0

#define WKC_TMUIRQ_REG                    0xB6
#define WKC_MASK_AF                       BIT2
#define WKC_MASK_WAF                      BIT1

#define WKC_MTMUIRQ_REG                   0xB7
#define WKC_MASK_MA                       BIT2
#define WKC_MASK_WMA                      BIT1

#define WKC_TMUSTATUS_REG                 0xB8
#define WKC_MASK_VRT                      BIT1

#define WKC_TMUCONFIG_REG                 0xB9
#define WKC_MASK_DSE                      BIT6
#define WKC_MASK_HF                       BIT4
#define WKC_MASK_RTCWAKEEN                BIT2
#define WKC_MASK_SETVRT                   BIT1
#define WKC_MASK_RSTTMU                   BIT0

#define WKC_SECONDS_REG                   0xBA
#define WKC_MASK_SECONDS                  (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_SECONDSW_REG                  0xBA
#define WKC_MASK_SECONDSW                 (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_SECONDSSA_REG                 0xBB
#define WKC_MASK_SECONDS_SA               (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_SECONDSWA_REG                 0xBC
#define WKC_MASK_SECONDS_WA               (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MINUTES_REG                   0xBD
#define WKC_MASK_MINUTES                  (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MINUTESW_REG                  0xBD
#define WKC_MASK_MINUTESW                 (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MINUTESSA_REG                 0xBE
#define WKC_MASK_MINUTES_SA               (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MINUTESWA_REG                 0xBF
#define WKC_MASK_MINUTES_WA               (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_HOURS_REG                     0xC0
#define WKC_MASK_HOURS_AM_PM              BIT7
#define WKC_MASK_HOURS                    (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_HOURSW_REG                    0xC0
#define WKC_MASK_HOURSW_AM_PM             BIT7
#define WKC_MASK_HOURSW                   (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_HOURSSA_REG                   0xC1
#define WKC_MASK_HOURSSA_AM_PM            BIT7
#define WKC_MASK_HOURS_SA                 (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_HOURSWA_REG                   0xC2
#define WKC_MASK_HOURSWA_AM_PM            BIT7
#define WKC_MASK_HOURS_WA                 (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_DWEEK_REG                     0xC3
#define WKC_MASK_DWEEK                    (BIT2|BIT1|BIT0)

#define WKC_DWEEKW_REG                    0xC3
#define WKC_MASK_DWEEKW                   (BIT2|BIT1|BIT0)

#define WKC_DMONTH_REG                    0xC4
#define WKC_MASK_DMONTH                   (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_DMONTHW_REG                   0xC4
#define WKC_MASK_DMONTHW                  (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MONTH_REG                     0xC5
#define WKC_MASK_MONTH                    (BIT3|BIT2|BIT1|BIT0)

#define WKC_MONTHW_REG                    0xC5
#define WKC_MASK_MONTHW                   (BIT3|BIT2|BIT1|BIT0)

#define WKC_YEAR_REG                      0xC6
#define WKC_MASK_YEAR                     (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_YEARW_REG                     0xC6
#define WKC_MASK_YEARW                    (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_STCB1_REG                     0xC7
#define WKC_STCB2_REG                     0xC8
#define WKC_STCB3_REG                     0xC9
#define WKC_STCB4_REG                     0xCA
#define WKC_DMONTHWA_REG                  0xCB
#define WKC_MASK_DMONTH_WA                (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_CLKCONFIG_REG                 0xCC
#define WKC_MASK_SLP2DLVL                 BIT5
#define WKC_MASK_SLP2D                    BIT4
#define WKC_MASK_SLP1DLVL                 BIT3
#define WKC_MASK_SLP1D                    BIT2
#define WKC_MASK_SLP0DLVL                 BIT1
#define WKC_MASK_SLP0D                    BIT0

#define WKC_FLLCTRL_REG                   0xCD
#define WKC_MASK_MULT_MINUS_ONE           (BIT7|BIT6|BIT5|BIT4)
#define WKC_MASK_REF_CLK_SEL              BIT3
#define WKC_MASK_DAC_SEL_CURRENT          (BIT2|BIT1|BIT0)

#define WKC_FLLDACSH_REG                  0xCE
#define WKC_FLLDACSL_REG                  0xCF
#define WKC_FLLDAC_STATUSH_REG            0xD0
#define WKC_MASK_DAC_H                    BIT0

#define WKC_FLLDAC_STATUSL_REG            0xD1
#define WKC_FLLFRQDIV_REG                 0xD2
#define WKC_CLK32CTRL_REG                 0xD3
#define WKC_MASK_EDGE_SEL                 BIT6
#define WKC_MASK_CLOAD_XTAL               (BIT5|BIT4|BIT3)
#define WKC_MASK_SHAPERDIS                BIT2
#define WKC_MASK_OSCPD                    BIT1
#define WKC_MASK_OSCBYP                   BIT0

#define WKC_GPLEDCTRL_REG                 0xDF
#define WKC_MASK_GPLEDF                   (BIT5|BIT4)   ///< GPLED Freq
#define WKC_MASK_GPLEDI                   (BIT3|BIT2)   ///< GPLED DC current
#define WKC_MASK_GPLEDON                  BIT0

#define WKC_GPLEDFSM_REG                  0xE0
#define WKC_MASK_GPLEDFF                  (BIT2|BIT1)

#define WKC_GPLEDPWM_REG                  0xE1
#define WKC_INTVREFCTRL_REG               0xE2
#define WKC_MASK_FAST_SETTLE              BIT3
#define WKC_MASK_FILTER_BYPASS            BIT2
#define WKC_MASK_MODEHPBG                 (BIT1|BIT0)

#define WKC_IREFTRIM_REG                  0xE3
#define WKC_MASK_IREFTRIM                 (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VREFTRIM0_REG                 0xE4
#define WKC_MASK_SPARE07                  BIT7
#define WKC_MASK_HPBGTRIM                 (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_VREFTRIM1_REG                 0xE5
#define WKC_MASK_SPARE07                  BIT7
#define WKC_MASK_LPBGTRIM                 (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_CC_PERSIST_DOWN_B3_REG        0xD5
#define WKC_CC_PERSIST_DOWN_B2_REG        0xD6
#define WKC_CC_PERSIST_DOWN_B1_REG        0xD7
#define WKC_CC_PERSIST_DOWN_B0_REG        0xD8
#define WKC_CC_PERSIST_UP_B3_REG          0xD9
#define WKC_CC_PERSIST_UP_B2_REG          0xDA
#define WKC_CC_PERSIST_UP_B1_REG          0xDB
#define WKC_CC_PERSIST_UP_B0_REG          0xDC
#define WKC_CC_THRH_REG                   0xE6
#define WKC_CC_THRL_REG                   0xE7
#define WKC_CC_CURR_SHRTH_REG             0xE8
#define WKC_MASK_CC_CURR_SHRTH            (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_CC_CURR_SHRTL_REG             0xE9
#define WKC_CC_CURR_LNGH_REG              0xEA
#define WKC_MASK_CC_CURR_LNGH             (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_CC_CURR_LNGL_REG              0xEB
#define WKC_CC_CTRL0_REG                  0xEC
#define WKC_MASK_CC_VREF_TST              BIT6
#define WKC_MASK_CC_CHOP_TST              BIT5
#define WKC_MASK_CC_LOW_PRECISION         BIT4
#define WKC_MASK_CC_PERSIST_CLR           BIT3
#define WKC_MASK_CC_ACLR                  BIT2
#define WKC_MASK_CC_CBT                   BIT1
#define WKC_MASK_CC_OFF                   BIT0

#define WKC_CC_CTRL1_REG                  0xED
#define WKC_MASK_CC_SPARE                 BIT7
#define WKC_MASK_CC_CHO_SCM               BIT6
#define WKC_MASK_CC_NO_CHO_CFG            BIT5
#define WKC_MASK_CC_CHO_PRD               (BIT4|BIT3|BIT2)
#define WKC_MASK_CC_CALI                  BIT1
#define WKC_MASK_CC_CHO_EN                BIT0

#define WKC_CC_DOWN_B3_REG                0xEE
#define WKC_CC_DOWN_B2_REG                0xEF
#define WKC_CC_DOWN_B1_REG                0xF0
#define WKC_CC_DOWN_B0_REG                0xF1
#define WKC_CC_UP_B3_REG                  0xF2
#define WKC_CC_UP_B2_REG                  0xF3
#define WKC_CC_UP_B1_REG                  0xF4
#define WKC_CC_UP_B0_REG                  0xF5
#define WKC_VBATMAXH_REG                  0xF6
#define WKC_MASK_VBATMAXH                 (BIT3|BIT2|BIT1|BIT0)

#define WKC_VBATMAXHW_REG                 0xF6
#define WKC_MASK_CLR_VBATMAX              BIT7
#define WKC_MASK_VBATMAXHW                (BIT3|BIT2|BIT1|BIT0)

#define WKC_VBATMAXL_REG                  0xF7
#define WKC_VBATMAXLW_REG                 0xF7
#define WKC_MAX_CURR_SHRTH_REG            0xF8
#define WKC_MASK_MAX_CURR_SHRTH           (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MAX_CURR_SHRTL_REG            0xF9
#define WKC_MAX_CURR_LNGH_REG             0xFA
#define WKC_MASK_MAX_CURR_LNGH            (BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_MAX_CURR_LNGL_REG             0xFB
#define WKC_DEV2_STATUS_1_REG             0x10
#define WKC_MASK_DEV2_READ_STATUS2        BIT7
#define WKC_MASK_DEV2_RESERVED            (BIT2|BIT1|BIT0)

#define WKC_DEV2_STATUS_2_REG             0x11
#define WKC_MASK_DEV2_DATA_FRAME_ERROR    BIT1
#define WKC_MASK_DEV2_PARITY_ERROR        BIT0

#define WKC_DEV2_STATUS2_LASTREAD_REG     0x1C
#define WKC_MASK_DEV2_STATUS_2_ON_LAST_GETREG     (BIT1|BIT0)

#define WKC_DEV2_SETREGADR_REG            0x35

///
/// Device3
///
#define WKC_PMICWDTCNT_REG                0x00
#define WKC_MASK_PMICWDTRST               BIT1
#define WKC_MASK_PMICWDTEN                BIT0

#define WKC_PMICWDTTC_REG                 0x01
#define WKC_SCRITIRQ_REG                  0x02
#define WKC_MASK_SOSCBAD                  BIT2
#define WKC_MASK_SOSCSTP                  BIT1
#define WKC_MASK_SOCP                     BIT0

#define WKC_USBIDCTRL_REG                 0x05
#define WKC_MASK_ACA_DETEN                BIT1
#define WKC_MASK_USB_IDEN                 BIT0

#define WKC_USBIDDETTYPE_REG              0x06
#define WKC_MASK_USBIDCONTI               BIT1
#define WKC_MASK_USBIDDETTYPE             BIT0

#define WKC_USBPHYCTRL_REG                0x07
#define WKC_MASK_CTYP_DIS                 BIT3
#define WKC_MASK_CTYP_START               BIT2
#define WKC_MASK_CHGDET_N_POL             BIT1
#define WKC_MASK_USBPHYRSTB               BIT0

#define WKC_USBRSTGPO_REG                 0x08
#define WKC_MASK_ALTFUNCEN                BIT6
#define WKC_MASK_DIR                      BIT5
#define WKC_MASK_DRV                      BIT4
#define WKC_MASK_REN                      BIT3
#define WKC_MASK_RVAL                     BIT1
#define WKC_MASK_DOUT                     BIT0

#define WKC_USBRSTGPI_REG                 0x09
#define WKC_MASK_GPIGLBYP                 BIT4
#define WKC_MASK_GPIDBNC                  BIT3
#define WKC_MASK_DIN                      BIT0

#define WKC_VSYSCTRL_REG                  0x0C
#define WKC_MASK_VSYSOFF                  (BIT6|BIT5|BIT4)
#define WKC_MASK_VSYSGOOD                 (BIT3|BIT2|BIT1|BIT0)

#define WKC_VXOCNT_REG                    0x0E
#define WKC_MASK_VXOEN                    BIT0

#define WKC_CHGDETGPO_REG                 0x12
#define WKC_MASK_ALTFUNCEN                BIT6
#define WKC_MASK_DIR                      BIT5
#define WKC_MASK_DRV                      BIT4
#define WKC_MASK_REN                      BIT3
#define WKC_MASK_RVAL                     BIT1
#define WKC_MASK_DOUT                     BIT0

#define WKC_CHGDETGPI_REG                 0x13
#define WKC_MASK_GPIGLBYP                 BIT4
#define WKC_MASK_GPIDBNC                  BIT3
#define WKC_MASK_DIN                      BIT0

#define WKC_DBPTIMEOUT_REG                0x14
#define WKC_DBPTIMER_REG                  0x15
#define WKC_CHGRCTRL0_REG                 0x16
#define WKC_MASK_CHR_WDT_NOKICK           BIT7
#define WKC_MASK_DBPOFF                   BIT6
#define WKC_MASK_CCSM_OFF                 BIT5
#define WKC_MASK_TTLCK                    BIT4
#define WKC_MASK_SWCONTROL                BIT3
#define WKC_MASK_EXTCHRDIS                BIT2
#define WKC_MASK_EMRGCHREN                BIT1
#define WKC_MASK_CHGRRESET                BIT0

#define WKC_CHGRCTRL1_REG                 0x17
#define WKC_MASK_DBPEN                    BIT7
#define WKC_MASK_OTGMODE                  BIT6
#define WKC_MASK_FTEMPEVENT               BIT5
#define WKC_MASK_FUSBINLMT1500            BIT4
#define WKC_MASK_FUSBINLMT900             BIT3
#define WKC_MASK_FUSBINLMT500             BIT2
#define WKC_MASK_FUSBINLMT150             BIT1
#define WKC_MASK_FUSBINLMT100             BIT0

#define WKC_CHGRCTRL2_REG                 0x18
#define WKC_MASK_PWROVREN                 BIT1
#define WKC_MASK_CHGDIS_ACT               BIT0

#define WKC_CHGRSTATUS_REG                0x19
#define WKC_MASK_SDCP                     BIT5
#define WKC_MASK_SDBP                     BIT4
#define WKC_MASK_USBSEL                   BIT3
#define WKC_MASK_CHGDISLVL                BIT2
#define WKC_MASK_CHGDETBLATCH             BIT1
#define WKC_MASK_CHGDETBLVL               BIT0

#define WKC_SCHGRIRQ_REG                  0x1A
#define WKC_MASK_SSTUCK                   BIT7
#define WKC_MASK_SNACK                    BIT3
#define WKC_MASK_SI2CRD                   BIT2
#define WKC_MASK_SI2CWR                   BIT1
#define WKC_MASK_CHGINTBLVL               BIT0

#define WKC_VBUSDETCTRL_REG               0x1D
#define WKC_MASK_DCDTIMEOUT               (BIT7|BIT6|BIT5)
#define WKC_MASK_SDP500_NC                BIT4
#define WKC_MASK_BOOT_100MA               BIT3
#define WKC_MASK_VBUSDETEN                BIT2
#define WKC_MASK_VBUSDETTYPE              BIT1

#define WKC_VDCINDETCTRL_REG              0x1E
#define WKC_MASK_VDCINDETEN               BIT2
#define WKC_MASK_VDCINDETTYPE             BIT1

#define WKC_CHRLEDCTRL_REG                0x1F
#define WKC_MASK_CHRLEDF                  (BIT5|BIT4)
#define WKC_MASK_CHRLEDI                  (BIT3|BIT2)
#define WKC_MASK_SWLEDON                  BIT1
#define WKC_MASK_CHRLEDFN                 BIT0

#define WKC_CHRLEDFSM_REG                 0x20
#define WKC_MASK_LEDEFF                   (BIT2|BIT1)
#define WKC_MASK_CHRLEDCIP                BIT0

#define WKC_CHRLEDPWM_REG                 0x21
#define WKC_CHRTTADDR_REG                 0x22
#define WKC_CHRTTDATA_REG                 0x23
#define WKC_I2COVRCTRL_REG                0x24
#define WKC_MASK_I2CCLRDIS                BIT2
#define WKC_MASK_I2CRD                    BIT1
#define WKC_MASK_I2CWR                    BIT0

#define WKC_I2COVRDADDR_REG               0x25
#define WKC_MASK_I2COVRDADDR              (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0)

#define WKC_I2COVROFFSET_REG              0x26
#define WKC_I2COVRWRDATA_REG              0x27
#define WKC_I2COVRRDDATA_REG              0x28
#define WKC_USBSRCDETSTATUS0_REG          0x29
#define WKC_MASK_SDCD                     (BIT7|BIT6)
#define WKC_MASK_USBSRCDETRSLT            (BIT5|BIT4|BIT3|BIT2)
#define WKC_MASK_SUSBHWDET                (BIT1|BIT0)
#define WKC_USBHWSTATUE_NOTSTARTED        0x00
#define WKC_USBHWSTATUE_ONGOING           0x01
#define WKC_USBHWSTATUE_COMPLETED         0x02
#define WKC_USBHWSTATUE_FAILED            0x03

#define WKC_USBSRCDETSTATUS1_REG          0x2A
#define WKC_MASK_SSEC                     BIT3
#define WKC_MASK_SPRIM                    BIT2
#define WKC_MASK_SACA                     (BIT1|BIT0)

#define WKC_CCSMCMDSKIP_REG               0x2B
#define WKC_CCSMSFTTIMER_REG              0x2C
#define WKC_SCCSMSFTTIMER_REG             0x2D
#define WKC_CHGRCTRL3_REG                 0x2E
#define WKC_MASK_DPDCPOFF                 BIT1
#define WKC_MASK_SFTTIMERSTOP             BIT0

#define WKC_CHGDISCTRL_REG                0x2F
#define WKC_MASK_CHGDISFN                 BIT6
#define WKC_MASK_CHGDISDRV                BIT4
#define WKC_MASK_CHGDISOUT                BIT0

#define WKC_TLP1CTRL_REG                  0x30
#define WKC_MASK_TLP1LCK                  BIT1
#define WKC_MASK_TLP1RST                  BIT0

#define WKC_TLP1EVSTATUS0_REG             0x31
#define WKC_MASK_TLP1EXTSTBY              BIT7
#define WKC_MASK_TLP1ENTS0I1              BIT6
#define WKC_MASK_TLP1ENTS0I2              BIT5
#define WKC_MASK_TLP1ENTS0I3              BIT4
#define WKC_MASK_TLP1WARMRST              BIT3
#define WKC_MASK_TLP1COLDOFF              BIT2
#define WKC_MASK_TLP1COLDBOOT             BIT1
#define WKC_MASK_TLP1COLDRST              BIT0

#define WKC_TLP1EVSTATUS1_REG             0x32
#define WKC_MASK_TLP1ENTS0I3LP            BIT4
#define WKC_MASK_TLP1MODEMRST             BIT3
#define WKC_MASK_TLP1ENTS0I1VNNLP         BIT2
#define WKC_MASK_TLP1ENTS0I1VNN           BIT1
#define WKC_MASK_TLP1PLATFORMRST          BIT0

#define WKC_TLP1TRACEINSTH_REG            0x36
#define WKC_MASK_TLP1TRACEINST_H          (BIT1|BIT0)

#define WKC_TLP1TRACEINSTL_REG            0x37
#define WKC_TLP1INSTMEMADDRH_REG          0x38
#define WKC_MASK_TLP1INSTMEMADDR_H        (BIT1|BIT0)

#define WKC_TLP1INSTMEMADDRL_REG          0x39
#define WKC_TLP1INSTMEMDATAH_REG          0x3A
#define WKC_MASK_TLP1INSTMEMDATA_H        (BIT2|BIT1|BIT0)

#define WKC_TLP1INSTMEMDATAHW_REG         0x3A
#define WKC_MASK_TLP1INSTMEMDATAW_H       (BIT2|BIT1|BIT0)

#define WKC_TLP1INSTMEMDATAL_REG          0x3B
#define WKC_TLP1COLDBOOTH_REG             0x3C
#define WKC_MASK_TLP1COLDBOOTBASE_H       (BIT1|BIT0)

#define WKC_TLP1COLDBOOTL_REG             0x3D
#define WKC_TLP1COLDOFFH_REG              0x3E
#define WKC_MASK_TLP1COLDOFFBASE_H        (BIT1|BIT0)

#define WKC_TLP1COLDOFFL_REG              0x3F
#define WKC_TLP1COLDRSTH_REG              0x40
#define WKC_MASK_TLP1COLDRSTBASE_H        (BIT1|BIT0)

#define WKC_TLP1COLDRSTL_REG              0x41
#define WKC_TLP1WARMRSTH_REG              0x42
#define WKC_MASK_TLP1WARMRSTBASE_H        (BIT1|BIT0)

#define WKC_TLP1WARMRSTL_REG              0x43
#define WKC_TLP1ESBS0I1H_REG              0x44
#define WKC_MASK_TLP1ESBS0I1BASE_H        (BIT1|BIT0)

#define WKC_TLP1ESBS0I1L_REG              0x45
#define WKC_TLP1ESBS0I2H_REG              0x4A
#define WKC_MASK_TLP1ESBS0I2BASE_H        (BIT1|BIT0)

#define WKC_TLP1ESBS0I2L_REG              0x4B
#define WKC_TLP1EXSBH_REG                 0x50
#define WKC_MASK_TLP1EXSBBASE_H           (BIT1|BIT0)

#define WKC_TLP1EXSBL_REG                 0x51
#define WKC_TLP1MODEMRSTH_REG             0x5A
#define WKC_MASK_TLP1MODEMRSTBASE_H       (BIT1|BIT0)

#define WKC_TLP1MODEMRSTL_REG             0x5B
#define WKC_TLP1VRSETTLED_REG             0x5C
#define WKC_MASK_VREFSOCDLY               (BIT2|BIT1|BIT0)

#define WKC_TLP2CTRL_REG                  0x61
#define WKC_MASK_TLP2LCK                  BIT1
#define WKC_MASK_TLP2RST                  BIT0

#define WKC_TLP2EVSTATUS_REG              0x62
#define WKC_MASK_TLP2GPMEAS               BIT6
#define WKC_MASK_TLP2PEAK                 BIT5
#define WKC_MASK_TLP2USBID                BIT4
#define WKC_MASK_TLP2BATTV                BIT3
#define WKC_MASK_TLP2BATTID               BIT2
#define WKC_MASK_TLP2SYSTEMP              BIT1
#define WKC_MASK_TLP2BATTEMP              BIT0

#define WKC_TLP2TRACEINST_REG             0x63
#define WKC_TLP2INSTMEMADDR_REG           0x64
#define WKC_TLP2INSTMEMDATA_REG           0x65
#define WKC_TLP2SYSTEMP_REG               0x66
#define WKC_TLP2BATTID_REG                0x67
#define WKC_TLP2BATTV_REG                 0x68
#define WKC_TLP2USBID_REG                 0x69
#define WKC_TLP2PEAK_REG                  0x6A
#define WKC_TLP2GPMEAS_REG                0x6B
#define WKC_DEV3_STATUS_1_REG             0x10
#define WKC_MASK_DEV3_READ_STATUS2        BIT7
#define WKC_MASK_DEV3_RESERVED            (BIT2|BIT1|BIT0)

#define WKC_DEV3_STATUS_2_REG             0x11
#define WKC_MASK_DEV3_DATA_FRAME_ERROR    BIT1
#define WKC_MASK_DEV3_PARITY_ERROR        BIT0

#define WKC_DEV3_STATUS2_LASTREAD_REG     0x1C
#define WKC_MASK_DEV3_STATUS_2_ON_LAST_GETREG     (BIT1|BIT0)

#define WKC_DEV3_SETREGADR_REG            0x35
#define WKC_CM_VPROG1E_MODE_CTRL_REG      0x70
#define WKC_MASK_VPROG1E_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG1E_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG1E_CLR_DATA         BIT1
#define WKC_MASK_VPROG1E_CM_EN            BIT0

#define WKC_CM_VPROG1F_MODE_CTRL_REG      0x71
#define WKC_MASK_VPROG1F_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG1F_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG1F_CLR_DATA         BIT1
#define WKC_MASK_VPROG1F_CM_EN            BIT0

#define WKC_CM_VPROG2D_MODE_CTRL_REG      0x72
#define WKC_MASK_VPROG2D_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG2D_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG2D_CLR_DATA         BIT1
#define WKC_MASK_VPROG2D_CM_EN            BIT0

#define WKC_CM_VPROG4A_MODE_CTRL_REG      0x73
#define WKC_MASK_VPROG4A_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG4A_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG4A_CLR_DATA         BIT1
#define WKC_MASK_VPROG4A_CM_EN            BIT0

#define WKC_CM_VPROG4B_MODE_CTRL_REG      0x74
#define WKC_MASK_VPROG4B_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG4B_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG4B_CLR_DATA         BIT1
#define WKC_MASK_VPROG4B_CM_EN            BIT0

#define WKC_CM_VPROG4C_MODE_CTRL_REG      0x75
#define WKC_MASK_VPROG4C_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG4C_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG4C_CLR_DATA         BIT1
#define WKC_MASK_VPROG4C_CM_EN            BIT0

#define WKC_CM_VPROG4D_MODE_CTRL_REG      0x76
#define WKC_MASK_VPROG4D_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG4D_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG4D_CLR_DATA         BIT1
#define WKC_MASK_VPROG4D_CM_EN            BIT0

#define WKC_CM_VPROG5A_MODE_CTRL_REG      0x77
#define WKC_MASK_VPROG5A_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG5A_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG5A_CLR_DATA         BIT1
#define WKC_MASK_VPROG5A_CM_EN            BIT0

#define WKC_CM_VPROG5B_MODE_CTRL_REG      0x78
#define WKC_MASK_VPROG5B_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG5B_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG5B_CLR_DATA         BIT1
#define WKC_MASK_VPROG5B_CM_EN            BIT0

#define WKC_CM_VPROG6A_MODE_CTRL_REG      0x79
#define WKC_MASK_VPROG6A_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG6A_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG6A_CLR_DATA         BIT1
#define WKC_MASK_VPROG6A_CM_EN            BIT0

#define WKC_CM_VPROG6B_MODE_CTRL_REG      0x7A
#define WKC_MASK_VPROG6B_CM_ACC           (BIT5|BIT4)
#define WKC_MASK_VPROG6B_CM_MODE          (BIT3|BIT2)
#define WKC_MASK_VPROG6B_CLR_DATA         BIT1
#define WKC_MASK_VPROG6B_CM_EN            BIT0

#define WKC_CM_LDO4_THRSHLD_REG           0x81
#define WKC_MASK_VPROG4A_CM_THRSHLD       (BIT7|BIT6)
#define WKC_MASK_VPROG2D_CM_THRSHLD       (BIT5|BIT4)
#define WKC_MASK_VPROG1F_CM_THRSHLD       (BIT3|BIT2)
#define WKC_MASK_VPROG1E_CM_THRSHLD       (BIT1|BIT0)

#define WKC_CM_LDO5_THRSHLD_REG           0x82
#define WKC_MASK_VPROG5A_CM_THRSHLD       (BIT7|BIT6)
#define WKC_MASK_VPROG4D_CM_THRSHLD       (BIT5|BIT4)
#define WKC_MASK_VPROG4C_CM_THRSHLD       (BIT3|BIT2)
#define WKC_MASK_VPROG4B_CM_THRSHLD       (BIT1|BIT0)

#define WKC_CM_LDO6_THRSHLD_REG           0x83
#define WKC_MASK_VPROG6B_CM_THRSHLD       (BIT5|BIT4)
#define WKC_MASK_VPROG6A_CM_THRSHLD       (BIT3|BIT2)
#define WKC_MASK_VPROG5B_CM_THRSHLD       (BIT1|BIT0)

#define WKC_CM_PROG1E_DATA_REG            0x86
#define WKC_CM_PROG1F_DATA_REG            0x87
#define WKC_CM_PROG2D_DATA_REG            0x88
#define WKC_CM_PROG4A_DATA_REG            0x89
#define WKC_CM_PROG4B_DATA_REG            0x8A
#define WKC_CM_PROG4C_DATA_REG            0x8B
#define WKC_CM_PROG4D_DATA_REG            0x8C
#define WKC_CM_PROG5A_DATA_REG            0x8D
#define WKC_CM_PROG5B_DATA_REG            0x8E
#define WKC_CM_PROG6A_DATA_REG            0x8F
#define WKC_CM_PROG6B_DATA_REG            0x90
#define WKC_CM_VPROG1E_INT_LVL_REG        0x97
#define WKC_CM_VPROG1F_INT_LVL_REG        0x98
#define WKC_CM_VPROG2D_INT_LVL_REG        0x99
#define WKC_CM_VPROG4A_INT_LVL_REG        0x9A
#define WKC_CM_VPROG4B_INT_LVL_REG        0x9B
#define WKC_CM_VPROG4C_INT_LVL_REG        0x9C
#define WKC_CM_VPROG4D_INT_LVL_REG        0x9D
#define WKC_CM_VPROG5A_INT_LVL_REG        0x9E
#define WKC_CM_VPROG5B_INT_LVL_REG        0x9F
#define WKC_CM_VPROG6A_INT_LVL_REG        0xA0
#define WKC_CM_VPROG6B_INT_LVL_REG        0xA1
#define WKC_CM_VROCIRQSTAT4_REG           0xA8
#define WKC_MASK_SVPROG5A                 BIT7
#define WKC_MASK_SVPROG4D                 BIT6
#define WKC_MASK_SVPROG4C                 BIT5
#define WKC_MASK_SVPROG4B                 BIT4
#define WKC_MASK_SVPROG4A                 BIT3
#define WKC_MASK_SVPROG2D                 BIT2
#define WKC_MASK_SVPROG1F                 BIT1
#define WKC_MASK_SVPROG1E                 BIT0

#define WKC_CM_VROCIRQSTAT5_REG           0xA9
#define WKC_MASK_SVPROG6B                 BIT2
#define WKC_MASK_SVPROG6A                 BIT1
#define WKC_MASK_SVPROG5B                 BIT0

//
// Scratch
//
#define WKC_SCRATCH1_REG      0x00
#define WKC_SCRATCH2_REG      0x01
#define WKC_SCRATCH3_REG      0x02
#define WKC_SCRATCH4_REG      0x03
#define WKC_SCRATCH5_REG      0x04
#define WKC_SCRATCH6_REG      0x05
#define WKC_SCRATCH7_REG      0x06
#define WKC_SCRATCH8_REG      0x07
#define WKC_SCRATCH9_REG      0x08
#define WKC_SCRATCH10_REG     0x09
#define WKC_SCRATCH11_REG     0x0A
#define WKC_SCRATCH12_REG     0x0B
#define WKC_SCRATCH13_REG     0x0C
#define WKC_SCRATCH14_REG     0x0D
#define WKC_SCRATCH15_REG     0x0E
#define WKC_SCRATCH16_REG     0x0F
#define WKC_SCRATCH17_REG     0x10
#define WKC_SCRATCH18_REG     0x11
#define WKC_SCRATCH19_REG     0x12
#define WKC_SCRATCH20_REG     0x13
#define WKC_SCRATCH21_REG     0x14
#define WKC_SCRATCH22_REG     0x15
#define WKC_SCRATCH23_REG     0x16
#define WKC_SCRATCH24_REG     0x17
#define WKC_SCRATCH25_REG     0x18
#define WKC_SCRATCH26_REG     0x19
#define WKC_SCRATCH27_REG     0x1A
#define WKC_SCRATCH28_REG     0x1B
#define WKC_SCRATCH29_REG     0x1C
#define WKC_SCRATCH30_REG     0x1D
#define WKC_SCRATCH31_REG     0x1E
#define WKC_SCRATCH32_REG     0x1F
#define WKC_SCRATCH33_REG     0x20
#define WKC_SCRATCH34_REG     0x21
#define WKC_SCRATCH35_REG     0x22
#define WKC_SCRATCH36_REG     0x23
#define WKC_SCRATCH37_REG     0x24
#define WKC_SCRATCH38_REG     0x25
#define WKC_SCRATCH39_REG     0x26
#define WKC_SCRATCH40_REG     0x27
#define WKC_SCRATCH41_REG     0x28
#define WKC_SCRATCH42_REG     0x29
#define WKC_SCRATCH43_REG     0x2A
#define WKC_SCRATCH44_REG     0x2B
#define WKC_SCRATCH45_REG     0x2C
#define WKC_SCRATCH46_REG     0x2D
#define WKC_SCRATCH47_REG     0x2E
#define WKC_SCRATCH48_REG     0x2F
#define WKC_SCRATCH49_REG     0x30
#define WKC_SCRATCH50_REG     0x31
#define WKC_SCRATCH51_REG     0x32
#define WKC_SCRATCH52_REG     0x33
#define WKC_SCRATCH53_REG     0x34
#define WKC_SCRATCH54_REG     0x35
#define WKC_SCRATCH55_REG     0x36
#define WKC_SCRATCH56_REG     0x37
#define WKC_SCRATCH57_REG     0x38
#define WKC_SCRATCH58_REG     0x39
#define WKC_SCRATCH59_REG     0x3A
#define WKC_SCRATCH60_REG     0x3B
#define WKC_SCRATCH61_REG     0x3C
#define WKC_SCRATCH62_REG     0x3D
#define WKC_SCRATCH63_REG     0x3E
#define WKC_SCRATCH64_REG     0x3F
#define WKC_SCRATCH65_REG     0x40
#define WKC_SCRATCH66_REG     0x41
#define WKC_SCRATCH67_REG     0x42
#define WKC_SCRATCH68_REG     0x43
#define WKC_SCRATCH69_REG     0x44
#define WKC_SCRATCH70_REG     0x45
#define WKC_SCRATCH71_REG     0x46
#define WKC_SCRATCH72_REG     0x47
#define WKC_SCRATCH73_REG     0x48
#define WKC_SCRATCH74_REG     0x49
#define WKC_SCRATCH75_REG     0x4A
#define WKC_SCRATCH76_REG     0x4B
#define WKC_SCRATCH77_REG     0x4C
#define WKC_SCRATCH78_REG     0x4D
#define WKC_SCRATCH79_REG     0x4E
#define WKC_SCRATCH80_REG     0x4F
#define WKC_SCRATCH81_REG     0x50
#define WKC_SCRATCH82_REG     0x51
#define WKC_SCRATCH83_REG     0x52
#define WKC_SCRATCH84_REG     0x53
#define WKC_SCRATCH85_REG     0x54
#define WKC_SCRATCH86_REG     0x55
#define WKC_SCRATCH87_REG     0x56
#define WKC_SCRATCH88_REG     0x57
#define WKC_SCRATCH89_REG     0x58
#define WKC_SCRATCH90_REG     0x59
#define WKC_SCRATCH91_REG     0x5A
#define WKC_SCRATCH92_REG     0x5B
#define WKC_SCRATCH93_REG     0x5C
#define WKC_SCRATCH94_REG     0x5D
#define WKC_SCRATCH95_REG     0x5E
#define WKC_SCRATCH96_REG     0x5F
#define WKC_SCRATCH97_REG     0x60
#define WKC_SCRATCH98_REG     0x61
#define WKC_SCRATCH99_REG     0x62
#define WKC_SCRATCH100_REG    0x63
#define WKC_SCRATCH101_REG    0x64
#define WKC_SCRATCH102_REG    0x65
#define WKC_SCRATCH103_REG    0x66
#define WKC_SCRATCH104_REG    0x67
#define WKC_SCRATCH105_REG    0x68
#define WKC_SCRATCH106_REG    0x69
#define WKC_SCRATCH107_REG    0x6A
#define WKC_SCRATCH108_REG    0x6B
#define WKC_SCRATCH109_REG    0x6C
#define WKC_SCRATCH110_REG    0x6D
#define WKC_SCRATCH111_REG    0x6E
#define WKC_SCRATCH112_REG    0x6F
#define WKC_SCRATCH113_REG    0x70
#define WKC_SCRATCH114_REG    0x71
#define WKC_SCRATCH115_REG    0x72
#define WKC_SCRATCH116_REG    0x73
#define WKC_SCRATCH117_REG    0x74
#define WKC_SCRATCH118_REG    0x75
#define WKC_SCRATCH119_REG    0x76
#define WKC_SCRATCH120_REG    0x77
#define WKC_SCRATCH121_REG    0x78
#define WKC_SCRATCH122_REG    0x79
#define WKC_SCRATCH123_REG    0x7A
#define WKC_SCRATCH124_REG    0x7B
#define WKC_SCRATCH125_REG    0x7C
#define WKC_SCRATCH126_REG    0x7D
#define WKC_SCRATCH127_REG    0x7E
#define WKC_SCRATCH128_REG    0x7F
#define WKC_SCRATCH129_REG    0x80
#define WKC_SCRATCH130_REG    0x81
#define WKC_SCRATCH131_REG    0x82
#define WKC_SCRATCH132_REG    0x83
#define WKC_SCRATCH133_REG    0x84
#define WKC_SCRATCH134_REG    0x85
#define WKC_SCRATCH135_REG    0x86
#define WKC_SCRATCH136_REG    0x87
#define WKC_SCRATCH137_REG    0x88
#define WKC_SCRATCH138_REG    0x89
#define WKC_SCRATCH139_REG    0x8A
#define WKC_SCRATCH140_REG    0x8B
#define WKC_SCRATCH141_REG    0x8C
#define WKC_SCRATCH142_REG    0x8D
#define WKC_SCRATCH143_REG    0x8E
#define WKC_SCRATCH144_REG    0x8F
#define WKC_SCRATCH145_REG    0x90
#define WKC_SCRATCH146_REG    0x91
#define WKC_SCRATCH147_REG    0x92
#define WKC_SCRATCH148_REG    0x93
#define WKC_SCRATCH149_REG    0x94
#define WKC_SCRATCH150_REG    0x95
#define WKC_SCRATCH151_REG    0x96
#define WKC_SCRATCH152_REG    0x97
#define WKC_SCRATCH153_REG    0x98
#define WKC_SCRATCH154_REG    0x99
#define WKC_SCRATCH155_REG    0x9A
#define WKC_SCRATCH156_REG    0x9B
#define WKC_SCRATCH157_REG    0x9C
#define WKC_SCRATCH158_REG    0x9D
#define WKC_SCRATCH159_REG    0x9E
#define WKC_SCRATCH160_REG    0x9F
#define WKC_SCRATCH161_REG    0xA0
#define WKC_SCRATCH162_REG    0xA1
#define WKC_SCRATCH163_REG    0xA2
#define WKC_SCRATCH164_REG    0xA3
#define WKC_SCRATCH165_REG    0xA4
#define WKC_SCRATCH166_REG    0xA5
#define WKC_SCRATCH167_REG    0xA6
#define WKC_SCRATCH168_REG    0xA7
#define WKC_SCRATCH169_REG    0xA8
#define WKC_SCRATCH170_REG    0xA9
#define WKC_SCRATCH171_REG    0xAA
#define WKC_SCRATCH172_REG    0xAB
#define WKC_SCRATCH173_REG    0xAC
#define WKC_SCRATCH174_REG    0xAD
#define WKC_SCRATCH175_REG    0xAE
#define WKC_SCRATCH176_REG    0xAF
#define WKC_SCRATCH177_REG    0xB0
#define WKC_SCRATCH178_REG    0xB1
#define WKC_SCRATCH179_REG    0xB2
#define WKC_SCRATCH180_REG    0xB3
#define WKC_SCRATCH181_REG    0xB4
#define WKC_SCRATCH182_REG    0xB5
#define WKC_SCRATCH183_REG    0xB6
#define WKC_SCRATCH184_REG    0xB7
#define WKC_SCRATCH185_REG    0xB8
#define WKC_SCRATCH186_REG    0xB9
#define WKC_SCRATCH187_REG    0xBA
#define WKC_SCRATCH188_REG    0xBB
#define WKC_SCRATCH189_REG    0xBC
#define WKC_SCRATCH190_REG    0xBD
#define WKC_SCRATCH191_REG    0xBE
#define WKC_SCRATCH192_REG    0xBF
#define WKC_SCRATCH193_REG    0xC0
#define WKC_SCRATCH194_REG    0xC1
#define WKC_SCRATCH195_REG    0xC2
#define WKC_SCRATCH196_REG    0xC3
#define WKC_SCRATCH197_REG    0xC4
#define WKC_SCRATCH198_REG    0xC5
#define WKC_SCRATCH199_REG    0xC6
#define WKC_SCRATCH200_REG    0xC7
#define WKC_SCRATCH201_REG    0xC8
#define WKC_SCRATCH202_REG    0xC9
#define WKC_SCRATCH203_REG    0xCA
#define WKC_SCRATCH204_REG    0xCB
#define WKC_SCRATCH205_REG    0xCC
#define WKC_SCRATCH206_REG    0xCD
#define WKC_SCRATCH207_REG    0xCE
#define WKC_SCRATCH208_REG    0xCF
#define WKC_SCRATCH209_REG    0xD0
#define WKC_SCRATCH210_REG    0xD1
#define WKC_SCRATCH211_REG    0xD2
#define WKC_SCRATCH212_REG    0xD3
#define WKC_SCRATCH213_REG    0xD4
#define WKC_SCRATCH214_REG    0xD5
#define WKC_SCRATCH215_REG    0xD6
#define WKC_SCRATCH216_REG    0xD7
#define WKC_SCRATCH217_REG    0xD8
#define WKC_SCRATCH218_REG    0xD9
#define WKC_SCRATCH219_REG    0xDA
#define WKC_SCRATCH220_REG    0xDB
#define WKC_SCRATCH221_REG    0xDC
#define WKC_SCRATCH222_REG    0xDD
#define WKC_SCRATCH223_REG    0xDE
#define WKC_SCRATCH224_REG    0xDF
#define WKC_SCRATCH225_REG    0xE0
#define WKC_SCRATCH226_REG    0xE1
#define WKC_SCRATCH227_REG    0xE2
#define WKC_SCRATCH228_REG    0xE3
#define WKC_SCRATCH229_REG    0xE4
#define WKC_SCRATCH230_REG    0xE5
#define WKC_SCRATCH231_REG    0xE6
#define WKC_SCRATCH232_REG    0xE7
#define WKC_SCRATCH233_REG    0xE8
#define WKC_SCRATCH234_REG    0xE9
#define WKC_SCRATCH235_REG    0xEA
#define WKC_SCRATCH236_REG    0xEB
#define WKC_SCRATCH237_REG    0xEC
#define WKC_SCRATCH238_REG    0xED
#define WKC_SCRATCH239_REG    0xEE
#define WKC_SCRATCH240_REG    0xEF
#define WKC_SCRATCH241_REG    0xF0
#define WKC_SCRATCH242_REG    0xF1
#define WKC_SCRATCH243_REG    0xF2
#define WKC_SCRATCH244_REG    0xF3
#define WKC_SCRATCH245_REG    0xF4
#define WKC_SCRATCH246_REG    0xF5
#define WKC_SCRATCH247_REG    0xF6
#define WKC_SCRATCH248_REG    0xF7
#define WKC_SCRATCH249_REG    0xF8
#define WKC_SCRATCH250_REG    0xF9
#define WKC_SCRATCH251_REG    0xFA
#define WKC_SCRATCH252_REG    0xFB
#define WKC_SCRATCH253_REG    0xFC
#define WKC_SCRATCH254_REG    0xFD
#define WKC_SCRATCH255_REG    0xFE
#define WKC_SCRATCH256_REG    0xFF

//
// VCC
//
#define WKC_VCC_SETVIDFAST_REG            0x07
#define WKC_VCC_SETVIDSLOW_REG            0x08
#define WKC_VCC_SETVIDDECAY_REG           0x09
#define WKC_VCC_STATUS_1_REG              0x10
#define WKC_MASK_VCC_READ_STATUS2         BIT7
#define WKC_MASK_VCC_ICC_MAX_ALERT        BIT2
#define WKC_MASK_VCC_VR_SETTLED           BIT0

#define WKC_VCC_STATUS_2_REG              0x11
#define WKC_MASK_VCC_DATA_FRAME_ERROR     BIT1
#define WKC_MASK_VCC_PARITY_ERROR         BIT0

#define WKC_VCC_TEMPERATURE_ZONE_REG      0x12
#define WKC_VCC_OUTPUT_CURRENT_REG        0x15
#define WKC_VCC_STATUS2_LASTREAD_REG      0x1C
#define WKC_MASK_VCC_STATUS_2_ON_LAST_GETREG      (BIT1|BIT0)

#define WKC_VCC_ICC_MAX_REG               0x21
#define WKC_VCC_TEMP_MAX_REG              0x22
#define WKC_VCC_SR_FAST_REG               0x24
#define WKC_VCC_SR_SLOW_REG               0x25
#define WKC_VCC_VBOOT_REG                 0x26
#define WKC_VCC_VID_STATUS_REG            0x2F
#define WKC_VCC_VOUT_MAX_REG              0x30
#define WKC_VCC_VID_SETTING_REG           0x31
#define WKC_VCC_POWER_STATE_REG           0x32
#define WKC_VCC_OFFSET_REG                0x33
#define WKC_VCC_MULTI_VR_CONFIG_REG       0x34
#define WKC_VCC_SETREGADR_REG             0x35
#define WKC_VCC_STDBY_EXITVID_REG         0x36

//
// VNN
//
#define WKC_VNN_SETVIDFAST_REG            0x07
#define WKC_VNN_SETVIDSLOW_REG            0x08
#define WKC_VNN_SETVIDDECAY_REG           0x09
#define WKC_VNN_CONTROL_REG               0x0A
#define WKC_MASK_VNN_FM                   BIT0

#define WKC_VNN_STATUS_1_REG              0x10
#define WKC_MASK_VNN_READ_STATUS2         BIT7
#define WKC_MASK_VNN_ICC_MAX_ALERT        BIT2
#define WKC_MASK_VNN_VR_SETTLED           BIT0

#define WKC_VNN_STATUS_2_REG              0x11
#define WKC_MASK_VNN_DATA_FRAME_ERROR     BIT1
#define WKC_MASK_VNN_PARITY_ERROR         BIT0

#define WKC_VNN_TEMPERATURE_ZONE_REG      0x12
#define WKC_VNN_OUTPUT_CURRENT_REG        0x15
#define WKC_VNN_STATUS2_LASTREAD_REG      0x1C
#define WKC_MASK_VNN_STATUS_2_ON_LAST_GETREG      (BIT1|BIT0)

#define WKC_VNN_ICC_MAX_REG               0x21
#define WKC_VNN_TEMP_MAX_REG              0x22
#define WKC_VNN_SR_FAST_REG               0x24
#define WKC_VNN_SR_SLOW_REG               0x25
#define WKC_VNN_VBOOT_REG                 0x26
#define WKC_VNN_VID_STATUS_REG            0x2F
#define WKC_VNN_VOUT_MAX_REG              0x30
#define WKC_VNN_VID_SETTING_REG           0x31
#define WKC_VNN_POWER_STATE_REG           0x32
#define WKC_VNN_OFFSET_REG                0x33
#define WKC_VNN_MULTI_VR_CONFIG_REG       0x34
#define WKC_VNN_SETREGADR_REG             0x35
#define WKC_VNN_STDBY_EXITVID_REG         0x36
#define WKC_VNN_STDBY_VID_SETTING_REG     0x39
#define WKC_VNN_COLDRST_REG               0x3A
#define WKC_MASK_COLDRSTEN                BIT1
#define WKC_MASK_CRT                      (BIT3|BIT2|BIT1|BIT0)

#define WKC_V1P05ACNT_REG                 0x3B
#define WKC_MASK_MODEV1P05A               (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_V1P05AEN                 BIT0

#define WKC_V1P15CNT_REG                  0x3C
#define WKC_MASK_RAMPCTLV1P15             BIT5
#define WKC_MASK_MODEV1P15                (BIT4|BIT3|BIT2|BIT1)
#define WKC_MASK_V1P15EN                  BIT0

#define WKC_V1P05AVSEL_REG                0x3D
#define WKC_V1P15VSEL_REG                 0x3E

//
// VNNAON
//
#define WKC_VNNAON_SETVIDFAST_REG         0x07
#define WKC_VNNAON_SETVIDSLOW_REG         0x08
#define WKC_VNNAON_SETVIDDECAY_REG        0x09
#define WKC_VNNAON_STATUS_1_REG           0x10
#define WKC_MASK_VNNAON_READ_STATUS2      BIT7
#define WKC_MASK_VNNAON_VR_SETTLED        BIT0

#define WKC_VNNAON_STATUS_2_REG           0x11
#define WKC_MASK_VNNAON_DATA_FRAME_ERROR  BIT1
#define WKC_MASK_VNNAON_PARITY_ERROR      BIT0

#define WKC_VNNAON_TEMPERATURE_ZONE_REG   0x12
#define WKC_VNNAON_OUTPUT_CURRENT_REG     0x15
#define WKC_VNNAON_STATUS2_LASTREAD_REG   0x1C
#define WKC_MASK_VNNAON_STATUS_2_ON_LAST_GETREG   (BIT1|BIT0)

#define WKC_VNNAON_ICC_MAX_REG            0x21
#define WKC_VNNAON_TEMP_MAX_REG           0x22
#define WKC_VNNAON_SR_FAST_REG            0x24
#define WKC_VNNAON_SR_SLOW_REG            0x25
#define WKC_VNNAON_VBOOT_REG              0x26
#define WKC_VNNAON_VID_STATUS_REG         0x2F
#define WKC_VNNAON_VOUT_MAX_REG           0x30
#define WKC_VNNAON_VID_SETTING_REG        0x31
#define WKC_VNNAON_POWER_STATE_REG        0x32
#define WKC_VNNAON_OFFSET_REG             0x33
#define WKC_VNNAON_MULTI_VR_CONFIG_REG    0x34
#define WKC_VNNAON_SETREGADR_REG          0x35

//
// VSS
//
#define WKC_VSYS_SETVIDFAST_REG           0x07
#define WKC_VSYS_SETVIDSLOW_REG           0x08
#define WKC_VSYS_SETVIDDECAY_REG          0x09
#define WKC_VSYS_STATUS_1_REG             0x10
#define WKC_MASK_VSYS_READ_STATUS2        BIT7
#define WKC_MASK_PMIC_INT                 BIT6
#define WKC_MASK_VEND_INT                 BIT5
#define WKC_MASK_VSYS_ICC_MAX_ALERT       BIT2
#define WKC_MASK_VSYS_THERM_ALERT         BIT1

#define WKC_VSYS_STATUS_2_REG             0x11
#define WKC_MASK_VSYS_DATA_FRAME_ERROR    BIT1
#define WKC_MASK_VSYS_PARITY_ERROR        BIT0

#define WKC_VSYS_TEMPERATURE_ZONE_REG     0x12
#define WKC_VSYS_OUTPUT_CURRENT_REG       0x15
#define WKC_VSYS_STATUS2_LASTREAD_REG     0x1C
#define WKC_VSYS_ICC_MAX_REG              0x21
#define WKC_VSYS_TEMP_MAX_REG             0x22
#define WKC_VSYS_SR_FAST_REG              0x24
#define WKC_VSYS_SR_SLOW_REG              0x25
#define WKC_VSYS_VBOOT_REG                0x26
#define WKC_VSYS_VOUT_MAX_REG             0x30
#define WKC_VSYS_VID_SETTING_REG          0x31
#define WKC_VSYS_POWER_STATE_REG          0x32
#define WKC_VSYS_OFFSET_REG               0x33
#define WKC_VSYS_MULTI_VR_CONFIG_REG      0x34
#define WKC_VSYS_SETREGADR_REG            0x35
#define WKC_VSYS_VIRTUAL_PINS_REG         0x36

//
// Charger
//
#define CHG_I2C_BQ24261_SLAVEADDRESS      0x6B


/// @todo: Merge all the WhiskeyCove registers above and below

#define WC_PMIC_I2C_CHANNEL_NUMBER           0x6
#define WC_PMIC_I2C_DEV_1_SLAVE_ADDRESS      0x4E ///< For BXT
#define WC_PMIC_I2C_DEV_2_SLAVE_ADDRESS      0x4F ///< Common
#define WC_PMIC_I2C_DEV_3_SLAVE_ADDRESS      0x5E ///< Common
#define WC_PMIC_I2C_DEV_4_SLAVE_ADDRESS      0x5F ///< Common
#define WC_PMIC_I2C_DEV_5_SLAVE_ADDRESS      0x6E ///< For CHT
#define WC_PMIC_I2C_SCRATCH_SLAVE_ADDRESS    0x6F

///
/// DEV1 definitions
///
#define DEV1_MCHGRIRQ0_REG   0x17  // Charger IRQ0
#define DEV1_GPIO0P4CTLO_REG 0x48  // PMIC Gpio
//
// DEV 2 definitions
// obsolete
//
#if 1
#define DEV2_ID_TRMPGM_REG 0x00
#define DEV2_ID_OTPVERSION_REG 0x01
#define DEV2_ADC_GPADCREQ_REG 0x02
#define DEV2_ADC_VBATRSLTH_REG 0x03
#define DEV2_ADC_VBATRSLTL_REG 0x04
#define DEV2_ADC_GPADCCNTL_REG 0x05
#define DEV2_ADC_BATTIDRSLTH_REG 0x06
#define DEV2_ADC_BATTIDRSLTL_REG 0x07
#define DEV2_ADC_USBIDRSLTH_REG 0x08
#define DEV2_ADC_USBIDRSLTL_REG 0x09
#define DEV2_ADC_GPMEASRSLTH_REG 0x0A
#define DEV2_ADC_GPMEASRSLTL_REG 0x0B
#define DEV2_ADC_Y0DATAH_REG 0x0C
#define DEV2_ADC_Y0DATAL_REG 0x0D
#define DEV2_ADC_Y1DATAH_REG 0x0E
#define DEV2_ADC_Y1DATAL_REG 0x0F
#define DEV2_SVID_DEV2_STATUS_1_REG 0x10
#define DEV2_SVID_DEV2_STATUS_2_REG 0x11
#define DEV2_ADC_PEAKREQ_REG 0x12
#define DEV2_ADC_PEAKRSLTH_REG 0x13
#define DEV2_ADC_PEAKRSLTL_REG 0x14
#define DEV2_THERM_BATTEMP0H_REG 0x15
#define DEV2_THERM_BATTEMP0L_REG 0x16
#define DEV2_THERM_BATTEMP1H_REG 0x17
#define DEV2_THERM_BATTEMP1L_REG 0x18
#define DEV2_THERM_STHRMIRQ0_REG 0x19
#define DEV2_THERM_STHRMIRQ1_REG 0x1A
#define DEV2_THERM_STHRMIRQ2_REG 0x1B
#define DEV2_SVID_DEV2_STATUS2_LASTREAD_REG 0x1C
#define DEV2_THERM_THRMMONCFG_REG 0x1D
#define DEV2_THERM_THRMMONCTL_REG 0x1E
#define DEV2_THERM_BATTHERMMONCTL_REG 0x1F
#define DEV2_THERM_VBATMONCTL_REG 0x20
#define DEV2_THERM_GPMONCTL_REG 0x21
#define DEV2_THERM_THRMBATZONE_REG 0x22
#define DEV2_THERM_SYS0ALERT0H_REG 0x23
#define DEV2_THERM_SYS0ALERT0L_REG 0x24
#define DEV2_THERM_SYS0ALERT1H_REG 0x25
#define DEV2_THERM_SYS0ALERT1L_REG 0x26
#define DEV2_THERM_SYS1ALERT0H_REG 0x27
#define DEV2_THERM_SYS1ALERT0L_REG 0x28
#define DEV2_THERM_SYS1ALERT1H_REG 0x29
#define DEV2_THERM_SYS1ALERT1L_REG 0x2A
#define DEV2_THERM_SYS2ALERT0H_REG 0x2B
#define DEV2_THERM_SYS2ALERT0L_REG 0x2C
#define DEV2_THERM_SYS2ALERT1H_REG 0x2D
#define DEV2_THERM_SYS2ALERT1L_REG 0x2E
#define DEV2_THERM_BAT0ALERT0H_REG 0x2F
#define DEV2_THERM_BAT0ALERT0L_REG 0x30
#define DEV2_THERM_BAT1ALERT0H_REG 0x31
#define DEV2_THERM_BAT1ALERT0L_REG 0x32
#define DEV2_THERM_PMICALERT0H_REG 0x33
#define DEV2_THERM_PMICALERT0L_REG 0x34
#define DEV2_SVID_DEV2_SETREGADR_REG 0x35
#define DEV2_THERM_GPMEASALERTH_REG 0x36
#define DEV2_THERM_GPMEASALERTL_REG 0x37
#define DEV2_THERM_THRMRSLT0H_REG 0x38
#define DEV2_THERM_THRMRSLT0L_REG 0x39
#define DEV2_THERM_THRMRSLT1H_REG 0x3A
#define DEV2_THERM_THRMRSLT1L_REG 0x3B
#define DEV2_THERM_THRMRSLT2H_REG 0x3C
#define DEV2_THERM_THRMRSLT2L_REG 0x3D
#define DEV2_THERM_THRMRSLT3H_REG 0x3E
#define DEV2_THERM_THRMRSLT3L_REG 0x3F
#define DEV2_THERM_THRMRSLT4H_REG 0x40
#define DEV2_THERM_THRMRSLT4L_REG 0x41
#define DEV2_THERM_THRMRSLT5H_REG 0x42
#define DEV2_THERM_THRMRSLT5L_REG 0x43
#define DEV2_THERM_THRMZN0H_REG 0x44
#define DEV2_THERM_THRMZN0L_REG 0x45
#define DEV2_THERM_THRMZN1H_REG 0x46
#define DEV2_THERM_THRMZN1L_REG 0x47
#define DEV2_THERM_THRMZN2H_REG 0x48
#define DEV2_THERM_THRMZN2L_REG 0x49
#define DEV2_THERM_THRMZN3H_REG 0x4A
#define DEV2_THERM_THRMZN3L_REG 0x4B
#define DEV2_THERM_THRMZN4H_REG 0x4C
#define DEV2_THERM_THRMZN4L_REG 0x4D
#define DEV2_THERM_BATCRITCOLDH_REG 0x4E
#define DEV2_THERM_BATCRITCOLDL_REG 0x4F
#define DEV2_THERM_BATCRITHOTH_REG 0x50
#define DEV2_THERM_BATCRITHOTL_REG 0x51
#define DEV2_THERM_BATCRITH_REG 0x52
#define DEV2_THERM_BATCRITL_REG 0x53
#define DEV2_THERM_PMICCRITH_REG 0x54
#define DEV2_THERM_PMICCRITL_REG 0x55
#define DEV2_THERM_SYS0CRITH_REG 0x56
#define DEV2_THERM_SYS0CRITL_REG 0x57
#define DEV2_THERM_SYS1CRITH_REG 0x58
#define DEV2_THERM_SYS1CRITL_REG 0x59
#define DEV2_THERM_SYS2CRITH_REG 0x5A
#define DEV2_THERM_SYS2CRITL_REG 0x5B
#define DEV2_THERM_SYS0ALERT3H_REG 0x5C
#define DEV2_THERM_SYS0ALERT3L_REG 0x5D
#define DEV2_THERM_SYS1ALERT3H_REG 0x5E
#define DEV2_THERM_SYS1ALERT3L_REG 0x5F
#define DEV2_THERM_SYS2ALERT3H_REG 0x60
#define DEV2_THERM_SYS2ALERT3L_REG 0x61
#define DEV2_THERM_PMICALERT3H_REG 0x62
#define DEV2_THERM_PMICALERT3L_REG 0x63
#define DEV2_THERM_BAT0ALERT3H_REG 0x64
#define DEV2_THERM_BAT0ALERT3L_REG 0x65
#define DEV2_THERM_BAT1ALERT3H_REG 0x66
#define DEV2_THERM_BAT1ALERT3L_REG 0x67
#define DEV2_THERM_THRMCRITEN_REG 0x68
#define DEV2_THERM_THRMALERT3EN_REG 0x69
#define DEV2_THERM_THRMALERT3PAEN_REG 0x6A
#define DEV2_IMON_CM_VCC0_MODE_CTRL_REG 0x6B
#define DEV2_IMON_CM_VCC1_MODE_CTRL_REG 0x6C
#define DEV2_IMON_CM_VCC_MODE_CTRL_REG 0x6D
#define DEV2_IMON_CM_VNN_MODE_CTRL_REG 0x6E
#define DEV2_IMON_CM_VNNAON_MODE_CTRL_REG 0x6F
#define DEV2_IMON_CM_VDD1_MODE_CTRL_REG 0x70
#define DEV2_IMON_CM_VDD2_MODE_CTRL_REG 0x71
#define DEV2_IMON_CM_VCCRAM_MODE_CTRL_REG 0x72
#define DEV2_IMON_CM_VMEM_MODE_CTRL_REG 0x73
#define DEV2_IMON_CM_VFLEX_MODE_CTRL_REG 0x74
#define DEV2_IMON_CM_VPROG1A_MODE_CTRL_REG 0x75
#define DEV2_IMON_CM_VPROG1B_MODE_CTRL_REG 0x76
#define DEV2_IMON_CM_VPROG1C_MODE_CTRL_REG 0x77
#define DEV2_IMON_CM_VPROG1D_MODE_CTRL_REG 0x78
#define DEV2_IMON_CM_VPROG2A_MODE_CTRL_REG 0x79
#define DEV2_IMON_CM_VPROG2B_MODE_CTRL_REG 0x7A
#define DEV2_IMON_CM_VPROG2C_MODE_CTRL_REG 0x7B
#define DEV2_IMON_CM_VPROG3A_MODE_CTRL_REG 0x7C
#define DEV2_IMON_CM_VPROG3B_MODE_CTRL_REG 0x7D
#define DEV2_IMON_CM_TSPAN_CTRL_REG 0x7E
#define DEV2_IMON_CM_LDO1_THRSHLD_REG 0x7F
#define DEV2_IMON_CM_LDO2_THRSHLD_REG 0x80
#define DEV2_IMON_CM_LDO3_THRSHLD_REG 0x81
#define DEV2_IMON_CM_SMPS0_THRSHLD_REG 0x82
#define DEV2_IMON_CM_SMPS1_THRSHLD_REG 0x83
#define DEV2_IMON_CM_SMPS2_THRSHLD_REG 0x84
#define DEV2_IMON_CM_SMPS3_THRSHLD_REG 0x85
#define DEV2_IMON_CM_SMPS4_THRSHLD_REG 0x86
#define DEV2_IMON_CM_SMPS5_THRSHLD_REG 0x87
#define DEV2_IMON_CM_VCC0_DATA_REG 0x88
#define DEV2_IMON_CM_VCC1_DATA_REG 0x89
#define DEV2_IMON_CM_VCC_DATA_REG 0x8A
#define DEV2_IMON_CM_VNN_DATA_REG 0x8B
#define DEV2_IMON_CM_VNNAON_DATA_REG 0x8C
#define DEV2_IMON_CM_VDD1_DATA_REG 0x8D
#define DEV2_IMON_CM_VDD2_DATA_REG 0x8E
#define DEV2_IMON_CM_VCCRAM_DATA_REG 0x8F
#define DEV2_IMON_CM_VMEM_DATA_REG 0x90
#define DEV2_IMON_CM_VFLEX_DATA_REG 0x91
#define DEV2_IMON_CM_PROG1A_DATA_REG 0x92
#define DEV2_IMON_CM_PROG1B_DATA_REG 0x93
#define DEV2_IMON_CM_PROG1C_DATA_REG 0x94
#define DEV2_IMON_CM_PROG1D_DATA_REG 0x95
#define DEV2_IMON_CM_PROG2A_DATA_REG 0x96
#define DEV2_IMON_CM_PROG2B_DATA_REG 0x97
#define DEV2_IMON_CM_PROG2C_DATA_REG 0x98
#define DEV2_IMON_CM_PROG3A_DATA_REG 0x99
#define DEV2_IMON_CM_PROG3B_DATA_REG 0x9A
#define DEV2_IMON_CM_VCC0_INT_LVL_REG 0x9B
#define DEV2_IMON_CM_VCC1_INT_LVL_REG 0x9C
#define DEV2_IMON_CM_VCC_INT_LVL_REG 0x9D
#define DEV2_IMON_CM_VNN_INT_LVL_REG 0x9E
#define DEV2_IMON_CM_VNNAON_INT_LVL_REG 0x9F
#define DEV2_IMON_CM_VDD1_INT_LVL_REG 0xA0
#define DEV2_IMON_CM_VDD2_INT_LVL_REG 0xA1
#define DEV2_IMON_CM_VCCRAM_INT_LVL_REG 0xA2
#define DEV2_IMON_CM_VMEM_INT_LVL_REG 0xA3
#define DEV2_IMON_CM_VFLEX_INT_LVL_REG 0xA4
#define DEV2_IMON_CM_VPROG1A_INT_LVL_REG 0xA5
#define DEV2_IMON_CM_VPROG1B_INT_LVL_REG 0xA6
#define DEV2_IMON_CM_VPROG1C_INT_LVL_REG 0xA7
#define DEV2_IMON_CM_VPROG1D_INT_LVL_REG 0xA8
#define DEV2_IMON_CM_VPROG2A_INT_LVL_REG 0xA9
#define DEV2_IMON_CM_VPROG2B_INT_LVL_REG 0xAA
#define DEV2_IMON_CM_VPROG2C_INT_LVL_REG 0xAB
#define DEV2_IMON_CM_VPROG3A_INT_LVL_REG 0xAC
#define DEV2_IMON_CM_VPROG3B_INT_LVL_REG 0xAD
#define DEV2_IMON_CM_VROCIRQSTAT0_REG 0xAE
#define DEV2_IMON_CM_VROCIRQSTAT1_REG 0xAF
#define DEV2_IMON_CM_VROCIRQSTAT2_REG 0xB0
#define DEV2_IMON_CM_VROCIRQSTAT3_REG 0xB1
#define DEV2_RTC_VRTCCTRL_REG 0xB5
#define DEV2_TMU_TMUIRQ_REG 0xB6
#define DEV2_TMU_MTMUIRQ_REG 0xB7
#define DEV2_TMU_TMUSTATUS_REG 0xB8
#define DEV2_TMU_TMUCONFIG_REG 0xB9
#define DEV2_TMU_SECONDS_REG 0xBA
#define DEV2_TMU_SECONDSW_REG 0xBA
#define DEV2_TMU_SECONDSSA_REG 0xBB
#define DEV2_TMU_SECONDSWA_REG 0xBC
#define DEV2_TMU_MINUTES_REG 0xBD
#define DEV2_TMU_MINUTESW_REG 0xBD
#define DEV2_TMU_MINUTESSA_REG 0xBE
#define DEV2_TMU_MINUTESWA_REG 0xBF
#define DEV2_TMU_HOURS_REG 0xC0
#define DEV2_TMU_HOURSW_REG 0xC0
#define DEV2_TMU_HOURSSA_REG 0xC1
#define DEV2_TMU_HOURSWA_REG 0xC2
#define DEV2_TMU_DWEEK_REG 0xC3
#define DEV2_TMU_DWEEKW_REG 0xC3
#define DEV2_TMU_DMONTH_REG 0xC4
#define DEV2_TMU_DMONTHW_REG 0xC4
#define DEV2_TMU_MONTH_REG 0xC5
#define DEV2_TMU_MONTHW_REG 0xC5
#define DEV2_TMU_YEAR_REG 0xC6
#define DEV2_TMU_YEARW_REG 0xC6
#define DEV2_TMU_STCB1_REG 0xC7
#define DEV2_TMU_STCB2_REG 0xC8
#define DEV2_TMU_STCB3_REG 0xC9
#define DEV2_TMU_STCB4_REG 0xCA
#define DEV2_TMU_DMONTHWA_REG 0xCB
#define DEV2_CLK_CLKCONFIG_REG 0xCC
#define DEV2_CLKGEN_FLLCTRL_REG 0xCD
#define DEV2_CLKGEN_FLLDACSH_REG 0xCE
#define DEV2_CLKGEN_FLLDACSL_REG 0xCF
#define DEV2_CLKGEN_FLLDAC_STATUSH_REG 0xD0
#define DEV2_CLKGEN_FLLDAC_STATUSL_REG 0xD1
#define DEV2_CLKGEN_FLLFRQDIV_REG 0xD2
#define DEV2_CLKGEN_CLK32CTRL_REG 0xD3
#define DEV2_GPLED_GPLEDCTRL_REG 0xDF
#define DEV2_GPLED_GPLEDFSM_REG 0xE0
#define DEV2_GPLED_GPLEDPWM_REG 0xE1
#define DEV2_INTVREF_INTVREFCTRL_REG 0xE2
#define DEV2_INTVREF_IREFTRIM_REG 0xE3
#define DEV2_INTVREF_VREFTRIM0_REG 0xE4
#define DEV2_INTVREF_VREFTRIM1_REG 0xE5
#define DEV2_COULCNT_CC_THRH_REG 0xE6
#define DEV2_COULCNT_CC_THRL_REG 0xE7
#define DEV2_COULCNT_CC_CURR_SHRTH_REG 0xE8
#define DEV2_COULCNT_CC_CURR_SHRTL_REG 0xE9
#define DEV2_COULCNT_CC_CURR_LNGH_REG 0xEA
#define DEV2_COULCNT_CC_CURR_LNGL_REG 0xEB
#define DEV2_COULCNT_CC_CTRL0_REG 0xEC
#define DEV2_COULCNT_CC_CTRL1_REG 0xED
#define DEV2_COULCNT_CC_DOWN_B3_REG 0xEE
#define DEV2_COULCNT_CC_DOWN_B2_REG 0xEF
#define DEV2_COULCNT_CC_DOWN_B1_REG 0xF0
#define DEV2_COULCNT_CC_DOWN_B0_REG 0xF1
#define DEV2_COULCNT_CC_UP_B3_REG 0xF2
#define DEV2_COULCNT_CC_UP_B2_REG 0xF3
#define DEV2_COULCNT_CC_UP_B1_REG 0xF4
#define DEV2_COULCNT_CC_UP_B0_REG 0xF5
#define DEV2_COULCNT_VBATMAXH_REG 0xF5
#define DEV2_COULCNT_VBATMAXHW_REG 0xF6
#define DEV2_COULCNT_VBATMAXL_REG 0xF6
#define DEV2_COULCNT_VBATMAXLW_REG 0xF7
#define DEV2_COULCNT_MAX_CURR_SHRTH_REG 0xF8
#define DEV2_COULCNT_MAX_CURR_SHRTL_REG 0xF9
#define DEV2_COULCNT_MAX_CURR_LNGH_REG 0xFA
#define DEV2_COULCNT_MAX_CURR_LNGL_REG 0xFB
#define DEV2_ID_PROVERSION_REG 0xFF

//
// DEV 3 definitions
//
#define DEV3_CRIT_PMICWDTCNT_REG 0x00
#define DEV3_CRIT_PMICWDTTC_REG 0x01
#define DEV3_CRIT_SCRITIRQ_REG 0x02
#define DEV3_USB_USBIDCTRL_REG 0x05
#define DEV3_USB_USBIDDETTYPE_REG 0x06
#define DEV3_USB_USBPHYCTRL_REG 0x07
#define DEV3_USB_USBRSTGPO_REG 0x08
#define DEV3_USB_USBRSTGPI_REG 0x09
#define DEV3_VCTRL_VSYSCTRL_REG 0x0C
#define DEV3_VCTRL_VOTPCTRL_REG 0x0D
#define DEV3_VCTRL_VXOCNT_REG 0x0E
#define DEV3_SVID_DEV3_STATUS_1_REG 0x10
#define DEV3_SVID_DEV3_STATUS_2_REG 0x11
#define DEV3_CHARGER_CHGDETGPO_REG 0x12
#define DEV3_CHARGER_CHGDETGPI_REG 0x13
#define DEV3_CHARGER_DBPTIMEOUT_REG 0x14
#define DEV3_CHARGER_DBPTIMER_REG 0x15
#define DEV3_CHARGER_CHGRCTRL0_REG 0x16
#define DEV3_CHARGER_CHGRCTRL1_REG 0x17
#define DEV3_CHARGER_CHGRCTRL2_REG 0x18
#define DEV3_CHARGER_CHGRSTATUS_REG 0x19
#define DEV3_CHARGER_SCHGRIRQ_REG 0x1A
#define DEV3_SVID_DEV3_STATUS2_LASTREAD_REG 0x1C
#define DEV3_CHARGER_VBUSDETCTRL_REG 0x1D
#define DEV3_CHARGER_VDCINDETCTRL_REG 0x1E
#define DEV3_CHARGER_CHRLEDCTRL_REG 0x1F
#define DEV3_CHARGER_CHRLEDFSM_REG 0x20
#define DEV3_CHARGER_CHRLEDPWM_REG 0x21
#define DEV3_CHARGER_CHRTTADDR_REG 0x22
#define DEV3_CHARGER_CHRTTDATA_REG 0x23
#define DEV3_CHARGER_I2COVRCTRL_REG 0x24
#define DEV3_CHARGER_I2COVRDADDR_REG 0x25
#define DEV3_CHARGER_I2COVROFFSET_REG 0x26
#define DEV3_CHARGER_I2COVRWRDATA_REG 0x27
#define DEV3_CHARGER_I2COVRRDDATA_REG 0x28
#define DEV3_CHARGER_USBSRCDETSTATUS0_REG 0x29
#define DEV3_CHARGER_USBSRCDETSTATUS1_REG 0x2A
#define DEV3_CHARGER_CCSMCMDSKIP_REG 0x2B
#define DEV3_CHARGER_CCSMSFTTIMER_REG 0x2C
#define DEV3_CHARGER_SCCSMSFTTIMER_REG 0x2D
#define DEV3_CHARGER_CHGRCTRL3_REG 0x2E
#define DEV3_CHARGER_CHGDISCTRL_REG 0x2F
#define DEV3_TLP_TLP1CTRL_REG 0x30
#define DEV3_TLP_TLP1EVSTATUS0_REG 0x31
#define DEV3_TLP_TLP1EVSTATUS1_REG 0x32
#define DEV3_TLP_TLP1EVSTATUS2_REG 0x33
#define DEV3_TLP_TLP1EVSTATUS3_REG 0x34
#define DEV3_SVID_DEV3_SETREGADR_REG 0x35
#define DEV3_TLP_TLP1TRACEINSTH_REG 0x36
#define DEV3_TLP_TLP1TRACEINSTL_REG 0x37
#define DEV3_TLP_TLP1INSTMEMADDRH_REG 0x38
#define DEV3_TLP_TLP1INSTMEMADDRL_REG 0x39
#define DEV3_TLP_TLP1INSTMEMDATAH_REG 0x3A
#define DEV3_TLP_TLP1INSTMEMDATAHW_REG 0x3A
#define DEV3_TLP_TLP1INSTMEMDATAL_REG 0x3B
#define DEV3_TLP_TLP1COLDBOOTH_REG 0x3C
#define DEV3_TLP_TLP1COLDBOOTL_REG 0x3D
#define DEV3_TLP_TLP1COLDOFFH_REG 0x3E
#define DEV3_TLP_TLP1COLDOFFL_REG 0x3F
#define DEV3_TLP_TLP1COLDRSTH_REG 0x40
#define DEV3_TLP_TLP1COLDRSTL_REG 0x41
#define DEV3_TLP_TLP1WARMRSTH_REG 0x42
#define DEV3_TLP_TLP1WARMRSTL_REG 0x43
#define DEV3_TLP_TLP1ESBS0I1H_REG 0x44
#define DEV3_TLP_TLP1ESBS0I1L_REG 0x45
#define DEV3_TLP_TLP1ESBS0I1VNNH_REG 0x46
#define DEV3_TLP_TLP1ESBS0I1VNNL_REG 0x47
#define DEV3_TLP_TLP1ESBS0I1VNNLPH_REG 0x48
#define DEV3_TLP_TLP1ESBS0I1VNNLPL_REG 0x49
#define DEV3_TLP_TLP1ESBS0I2H_REG 0x4A
#define DEV3_TLP_TLP1ESBS0I2L_REG 0x4B
#define DEV3_TLP_TLP1ESBS0I3H_REG 0x4C
#define DEV3_TLP_TLP1ESBS0I3L_REG 0x4D
#define DEV3_TLP_TLP1ESBS0I3LPH_REG 0x4E
#define DEV3_TLP_TLP1ESBS0I3LPL_REG 0x4F
#define DEV3_TLP_TLP1EXSBH_REG 0x50
#define DEV3_TLP_TLP1EXSBL_REG 0x51
#define DEV3_TLP_TLP1ESBS0IXH_REG 0x52
#define DEV3_TLP_TLP1ESBS0IXL_REG 0x53
#define DEV3_TLP_TLP1ESBS3H_REG 0x54
#define DEV3_TLP_TLP1ESBS3L_REG 0x55
#define DEV3_TLP_TLP1EXSBS0IXH_REG 0x56
#define DEV3_TLP_TLP1EXSBS0IXL_REG 0x57
#define DEV3_TLP_TLP1EXSBS3H_REG 0x58
#define DEV3_TLP_TLP1EXSBS3L_REG 0x59
#define DEV3_TLP_TLP1MODEMRSTH_REG 0x5A
#define DEV3_TLP_TLP1MODEMRSTL_REG 0x5B
#define DEV3_TLP_TLP1VRSETTLED_REG 0x5C
#define DEV3_TLP_TLP2CTRL_REG 0x61
#define DEV3_TLP_TLP2EVSTATUS_REG 0x62
#define DEV3_TLP_TLP2TRACEINST_REG 0x63
#define DEV3_TLP_TLP2INSTMEMADDR_REG 0x64
#define DEV3_TLP_TLP2INSTMEMDATA_REG 0x65
#define DEV3_TLP_TLP2SYSTEMP_REG 0x66
#define DEV3_TLP_TLP2BATTID_REG 0x67
#define DEV3_TLP_TLP2BATTV_REG 0x68
#define DEV3_TLP_TLP2USBID_REG 0x69
#define DEV3_TLP_TLP2PEAK_REG 0x6A
#define DEV3_TLP_TLP2GPMEAS_REG 0x6B
#define DEV3_IMON_CM_VPROG1E_MODE_CTRL_REG 0x70
#define DEV3_IMON_CM_VPROG1F_MODE_CTRL_REG 0x71
#define DEV3_IMON_CM_VPROG2D_MODE_CTRL_REG 0x72
#define DEV3_IMON_CM_VPROG4A_MODE_CTRL_REG 0x73
#define DEV3_IMON_CM_VPROG4B_MODE_CTRL_REG 0x74
#define DEV3_IMON_CM_VPROG4C_MODE_CTRL_REG 0x75
#define DEV3_IMON_CM_VPROG4D_MODE_CTRL_REG 0x76
#define DEV3_IMON_CM_VPROG5A_MODE_CTRL_REG 0x77
#define DEV3_IMON_CM_VPROG5B_MODE_CTRL_REG 0x78
#define DEV3_IMON_CM_VPROG6A_MODE_CTRL_REG 0x79
#define DEV3_IMON_CM_VPROG6B_MODE_CTRL_REG 0x7A
#define DEV3_IMON_CM_VPROG7A_MODE_CTRL_REG 0x7B
#define DEV3_IMON_CM_LDO4_THRSHLD_REG 0x81
#define DEV3_IMON_CM_LDO5_THRSHLD_REG 0x82
#define DEV3_IMON_CM_LDO6_THRSHLD_REG 0x83
#define DEV3_IMON_CM_PROG1E_DATA_REG 0x86
#define DEV3_IMON_CM_PROG1F_DATA_REG 0x87
#define DEV3_IMON_CM_PROG2D_DATA_REG 0x88
#define DEV3_IMON_CM_PROG4A_DATA_REG 0x89
#define DEV3_IMON_CM_PROG4B_DATA_REG 0x8A
#define DEV3_IMON_CM_PROG4C_DATA_REG 0x8B
#define DEV3_IMON_CM_PROG4D_DATA_REG 0x8C
#define DEV3_IMON_CM_PROG5A_DATA_REG 0x8D
#define DEV3_IMON_CM_PROG5B_DATA_REG 0x8E
#define DEV3_IMON_CM_PROG6A_DATA_REG 0x8F
#define DEV3_IMON_CM_PROG6B_DATA_REG 0x90
#define DEV3_IMON_CM_PROG7A_DATA_REG 0x91
#define DEV3_IMON_CM_VPROG1E_INT_LVL_REG 0x97
#define DEV3_IMON_CM_VPROG1F_INT_LVL_REG 0x98
#define DEV3_IMON_CM_VPROG2D_INT_LVL_REG 0x99
#define DEV3_IMON_CM_VPROG4A_INT_LVL_REG 0x9A
#define DEV3_IMON_CM_VPROG4B_INT_LVL_REG 0x9B
#define DEV3_IMON_CM_VPROG4C_INT_LVL_REG 0x9C
#define DEV3_IMON_CM_VPROG4D_INT_LVL_REG 0x9D
#define DEV3_IMON_CM_VPROG5A_INT_LVL_REG 0x9E
#define DEV3_IMON_CM_VPROG5B_INT_LVL_REG 0x9F
#define DEV3_IMON_CM_VPROG6A_INT_LVL_REG 0xA0
#define DEV3_IMON_CM_VPROG6B_INT_LVL_REG 0xA1
#define DEV3_IMON_CM_VPROG7A_INT_LVL_REG 0xA2

//
// DEV 4 definitions
//
#define DEV4_SVID_DEV4_STATUS_1_REG 0x10
#define DEV4_SVID_DEV4_STATUS_2_REG 0x11
#define DEV4_SVID_DEV4_STATUS2_LASTREAD_REG 0x1C
#define DEV4_SVID_DEV4_SETREGADR_REG 0x35
#define DEV4_VENDOR_VENDCTL0_REG 0x36
#define DEV4_VENDOR_VENDCTL1_REG 0x37
#define DEV4_VENDOR_VENDCTL2_REG 0x38
#define DEV4_VENDOR_VENDCTL3_REG 0x39
#define DEV4_VENDOR_NVM_MB_ADDRH_REG 0x3A
#define DEV4_VENDOR_NVM_MB_ADDRL_REG 0x3B
#define DEV4_VENDOR_NVM_MB_DATA_REG 0x3C
#define DEV4_VENDOR_NVMDBUF0_REG 0x3D
#define DEV4_VENDOR_NVMDBUF1_REG 0x3E
#define DEV4_VENDOR_NVMDBUF2_REG 0x3F
#define DEV4_VENDOR_NVMDBUF3_REG 0x40
#define DEV4_VENDOR_NVMDBUF4_REG 0x41
#define DEV4_VENDOR_NVMDBUF5_REG 0x42
#define DEV4_VENDOR_NVMDBUF6_REG 0x43
#define DEV4_VENDOR_NVMDBUF7_REG 0x44
#define DEV4_VENDOR_NVMDBUF8_REG 0x45
#define DEV4_VENDOR_NVMDBUF9_REG 0x46
#define DEV4_VENDOR_NVMDBUF10_REG 0x47
#define DEV4_VENDOR_NVMDBUF11_REG 0x48
#define DEV4_VENDOR_NVMDBUF12_REG 0x49
#define DEV4_VENDOR_NVMDBUF13_REG 0x4A
#define DEV4_VENDOR_NVMDBUF14_REG 0x4B
#define DEV4_VENDOR_NVMDBUF15_REG 0x4C
#define DEV4_VENDOR_NVMDBUF16_REG 0x4D
#define DEV4_VENDOR_NVMDBUF17_REG 0x4E
#define DEV4_VENDOR_NVMDBUF18_REG 0x4F
#define DEV4_VENDOR_NVMDBUF19_REG 0x50
#define DEV4_VENDOR_NVMDBUF20_REG 0x51
#define DEV4_VENDOR_NVMDBUF21_REG 0x52
#define DEV4_VENDOR_NVMDBUF22_REG 0x53
#define DEV4_VENDOR_NVMDBUF23_REG 0x54
#define DEV4_VENDOR_NVMDBUF24_REG 0x55
#define DEV4_VENDOR_NVMDBUF25_REG 0x56
#define DEV4_VENDOR_NVMDBUF26_REG 0x57
#define DEV4_VENDOR_NVMDBUF27_REG 0x58
#define DEV4_VENDOR_NVMDBUF28_REG 0x59
#define DEV4_VENDOR_NVMDBUF29_REG 0x5A
#define DEV4_VENDOR_NVMDBUF30_REG 0x5B
#define DEV4_VENDOR_NVMDBUF31_REG 0x5C
#define DEV4_VENDOR_NVMDBUF32_REG 0x5D
#define DEV4_VENDOR_NVMDBUF33_REG 0x5E
#define DEV4_VENDOR_NVMCTL0_REG 0x5F
#define DEV4_VENDOR_NVMCTL1_REG 0x60
#define DEV4_VENDOR_OTPMR_REG 0x61
#define DEV4_VENDOR_OTPMRA_RD1_REG 0x62
#define DEV4_VENDOR_OTPMRA_RD2_REG 0x63
#define DEV4_VENDOR_OTPMRB0_RD1_REG 0x64
#define DEV4_VENDOR_OTPMRB0_RD2_REG 0x65
#define DEV4_VENDOR_OTPMRB1_REG 0x66
#define DEV4_VENDOR_NVMSTAT0_REG 0x67
#define DEV4_VENDOR_SOAKING_STAT0_REG 0x68
#define DEV4_VENDOR_SOAKING_STAT1_REG 0x69
#define DEV4_VENDOR_NVM_USAGE_STATH_REG 0x6A
#define DEV4_VENDOR_NVM_USAGE_STATL_REG 0x6B
#define DEV4_VENDOR_CODSRC_REG 0x6C
#define DEV4_VENDOR_ISOCTRL_REG 0x6D
#define DEV4_VENDOR_OTPCONFIG_REG 0x6E
#define DEV4_VIRQ_VENDIRQLVL1_REG 0x77
#define DEV4_VIRQ_NVMVIRQ_REG 0x78
#define DEV4_VIRQ_ADCTLP2VIRQ_REG 0x79
#define DEV4_VIRQ_I2CSIFVIRQ_REG 0x7A
#define DEV4_VIRQ_REGBUSVIRQ_REG 0x7B
#define DEV4_VIRQ_CHGRVIRQ_REG 0x7C
#define DEV4_VIRQ_TLP1VIRQ_REG 0x7D
#define DEV4_VIRQ_DCDCVIRQ0_REG 0x7E
#define DEV4_VIRQ_DCDCVIRQ1_REG 0x7F
#define DEV4_VIRQ_VROCIRQ0_REG 0x80
#define DEV4_VIRQ_VROCIRQ1_REG 0x81
#define DEV4_VIRQ_VROCIRQ2_REG 0x82
#define DEV4_VIRQ_VROCIRQ3_REG 0x83
#define DEV4_VIRQ_MVENDIRQLVL1_REG 0x84
#define DEV4_VIRQ_MNVMVIRQ_REG 0x85
#define DEV4_VIRQ_MADCTLP2VIRQ_REG 0x86
#define DEV4_VIRQ_MI2CSIFVIRQ_REG 0x87
#define DEV4_VIRQ_MREGBUSVIRQ_REG 0x88
#define DEV4_VIRQ_MCHGRVIRQ_REG 0x89
#define DEV4_VIRQ_MTLP1VIRQ_REG 0x8A
#define DEV4_VIRQ_MDCDCVIRQ0_REG 0x8B
#define DEV4_VIRQ_MDCDCVIRQ1_REG 0x8C
#define DEV4_VIRQ_MVROCIRQ0_REG 0x8D
#define DEV4_VIRQ_MVROCIRQ1_REG 0x8E
#define DEV4_VIRQ_MVROCIRQ2_REG 0x8F
#define DEV4_VIRQ_MVROCIRQ3_REG 0x90
#define DEV4_VIRQ_M_VEND_INT_REG 0x91
#define DEV4_VIRQ_VROCIRQ4_REG 0x92
#define DEV4_VIRQ_MVROCIRQ4_REG 0x93
#define DEV4_TEST_TST_STARTUP_REG 0x9A
#define DEV4_TEST_TST_FREQ_DIV0_REG 0x9B
#define DEV4_TEST_TST_FREQ_DIV1_REG 0x9C
#define DEV4_TEST_TST_FREQ_SEL0_REG 0x9D
#define DEV4_TEST_TST_FREQ_SEL1_REG 0x9E
#define DEV4_TEST_TST_COUNT_CTRL0_REG 0x9F
#define DEV4_TEST_TST_COUNT_CTRL1_REG 0xA0
#define DEV4_TEST_TST_COUNT0_TIME_REG 0xA1
#define DEV4_TEST_TST_COUNT1_TIME_REG 0xA2
#define DEV4_TEST_TST_COUNT0_REG 0xA3
#define DEV4_TEST_TST_COUNT1_REG 0xA4
#define DEV4_TEST_TST_VINMON_REG 0xA5
#define DEV4_TEST_TST_VINMONRSLT_REG 0xA6
#define DEV4_TEST_TST_SRCDET0_REG 0xA7
#define DEV4_TEST_TST_SRCDET1_REG 0xA8
#define DEV4_TEST_TST_USB_REG 0xA9
#define DEV4_TEST_TST_VSWITCH0_REG 0xAA
#define DEV4_TEST_TST_VSWITCH1_REG 0xAB
#define DEV4_TEST_VUSBPHY_CP_REG 0xAC
#define DEV4_TEST_VUSBPHY_CP_VOUT_REG 0xAD
#define DEV4_TEST_TST_ATE_STRTUP_REG 0xAE
#define DEV4_TEST_GPADC_TEST0_REG 0xAF
#define DEV4_TEST_GPADC_TEST1_REG 0xB0
#define DEV4_TEST_GPADC_PKTST_REG 0xB1
#define DEV4_TEST_COMPTEST0_REG 0xB2
#define DEV4_TEST_COMPTEST1_REG 0xB3
#define DEV4_TEST_TST_ATE_CFG_REG 0xB4
#define DEV4_TEST_TST_ATE_REF0_REG 0xB5
#define DEV4_TEST_TST_ATE_REF1_REG 0xB6
#define DEV4_DEBUG_PMICSPARE01_AO_REG 0xC0
#define DEV4_DEBUG_PMICSPARE02_AI_REG 0xC1
#define DEV4_DEBUG_PMICSPARE03_AO_REG 0xC2
#define DEV4_DEBUG_PMICSPARE04_AI_REG 0xC3
#define DEV4_DEBUG_PMICSPARE05_REG 0xC4
#define DEV4_DEBUG_PMICSPARE06_REG 0xC5
#define DEV4_DEBUG_PMICSPARE07_REG 0xC6
#define DEV4_DEBUG_PMICSPARE08_REG 0xC7
#define DEV4_DEBUG_PMICSPARE09_REG 0xC8
#define DEV4_DEBUG_PMICSPARE10_REG 0xC9
#define DEV4_DEBUG_PMICSPARE11_REG 0xCA
#define DEV4_DEBUG_PMICSPARE12_REG 0xCB
#define DEV4_DEBUG_I2C_HS_TIMING_REG 0xCC
#define DEV4_DEBUG_DBGMON1_BLK_SEL_REG 0xCD
#define DEV4_DEBUG_DBGMON1_SIG_SEL_REG 0xCE
#define DEV4_DEBUG_DBGMON2_BLK_SEL_REG 0xCF
#define DEV4_DEBUG_DBGMON2_SIG_SEL_REG 0xD0
#define DEV4_DEBUG_DBGMON3_BLK_SEL_REG 0xD1
#define DEV4_DEBUG_DBGMON3_SIG_SEL_REG 0xD2
#define DEV4_DEBUG_DBGMON4_BLK_SEL_REG 0xD3
#define DEV4_DEBUG_DBGMON4_SIG_SEL_REG 0xD4
#define DEV4_DEBUG_PMICDBGCTRL1_REG 0xD5
#define DEV4_DEBUG_PMICDBGCTRL2_REG 0xD6
#define DEV4_DEBUG_REGBUS_ERR_CTL_REG 0xD7
#define DEV4_DEBUG_REGBUS_ERR_STATUS_REG 0xD8
#define DEV4_DEBUG_REGBUS_ERR_ADDRH_REG 0xD9
#define DEV4_DEBUG_REGBUS_ERR_ADDRL_REG 0xDA
#define DEV4_DEBUG_ANAMON0CTL0_REG 0xDB
#define DEV4_DEBUG_ANAMON0CTL1_REG 0xDC
#define DEV4_DEBUG_ANAMON1CTL0_REG 0xDD
#define DEV4_DEBUG_ANAMON1CTL1_REG 0xDE
#define DEV4_DEBUG_VCOMPTEST_REG 0xDF
#define DEV4_DEBUG_DBG_USBBC1_REG 0xE0
#define DEV4_DEBUG_DBG_USBBC2_REG 0xE1
#define DEV4_DEBUG_DBG_USBBCSTAT_REG 0xE2
#define DEV4_DEBUG_ANAMON2CTL0_REG 0xE3
#define DEV4_DEBUG_ANAMON2CTL1_REG 0xE4

//
// DEV5 definitions
//
#define DEV5_ID_ID0_REG 0x00
#define DEV5_ID_ID1_REG 0x01
#define DEV5_IRQ_IRQLVL1_REG 0x02
#define DEV5_IRQ_PWRSRCIRQ_REG 0x03
#define DEV5_IRQ_THRMIRQ0_REG 0x04
#define DEV5_IRQ_THRMIRQ1_REG 0x05
#define DEV5_IRQ_THRMIRQ2_REG 0x06
#define DEV5_IRQ_BCUIRQ_REG 0x07
#define DEV5_IRQ_ADCIRQ_REG 0x08
#define DEV5_IRQ_CHGRIRQ_REG 0x0A
#define DEV5_IRQ_GPIO0IRQ_REG 0x0B
#define DEV5_IRQ_GPIO1IRQ_REG 0x0C
#define DEV5_IRQ_MTHRMIRQ0_REG 0x0D
#define DEV5_IRQ_MIRQLVL1_REG 0x0E
#define DEV5_IRQ_MPWRSRCIRQ_REG 0x0F
#define DEV5_SVID_DEV5_STATUS_1_REG 0x10
#define DEV5_SVID_DEV5_STATUS_2_REG 0x11
#define DEV5_IRQ_MTHRMIRQ1_REG 0x12
#define DEV5_IRQ_MTHRMIRQ2_REG 0x13
#define DEV5_IRQ_MBCUIRQ_REG 0x14
#define DEV5_IRQ_MADCIRQ_REG 0x15
#define DEV5_IRQ_MCHGRIRQ_REG 0x17
#define DEV5_IRQ_MGPIO0IRQ_REG 0x19
#define DEV5_IRQ_MGPIO1IRQ_REG 0x1A
#define DEV5_SVID_DEV5_STATUS2_LASTREAD_REG 0x1C
#define DEV5_PWRSRC_SPWRSRC_REG 0x1E
#define DEV5_EXTCTRL_REGLOCK_REG 0x1F
#define DEV5_RESET_RESETSRC0_REG 0x20
#define DEV5_RESET_RESETSRC1_REG 0x21
#define DEV5_WAKE_WAKESRC_REG 0x22
#define DEV5_PWRSRC_LOWBATTDET0_REG 0x23
#define DEV5_PWRSRC_LOWBATTDET1_REG 0x24
#define DEV5_PWRSRC_PSDETCTRL_REG 0x25
#define DEV5_PB_PBCONFIG1_REG 0x26
#define DEV5_PB_PBSTATUS_REG 0x27
#define DEV5_PB_UBSTATUS_REG 0x28
#define DEV5_EXTCTRL_MODEMCTRL_REG 0x29
#define DEV5_EXTCTRL_BBCHGRCFG_REG 0x2A
#define DEV5_GPIO_GPIO0P0CTLO_REG 0x2B
#define DEV5_GPIO_GPIO0P2CTLO_REG 0x2D
#define DEV5_GPIO_GPIO0P3CTLO_REG 0x2E
#define DEV5_GPIO_GPIO0P5CTLO_REG 0x30
#define DEV5_GPIO_GPIO0P6CTLO_REG 0x31
#define DEV5_GPIO_GPIO0P0CTLI_REG 0x33
#define DEV5_GPIO_GPIO0P2CTLI_REG 0x34
#define DEV5_SVID_DEV5_SETREGADR_REG 0x35
#define DEV5_GPIO_GPIO0P3CTLI_REG 0x36
#define DEV5_IRQ_CRITIRQ_REG 0x37
#define DEV5_GPIO_GPIO0P5CTLI_REG 0x38
#define DEV5_GPIO_GPIO0P6CTLI_REG 0x39
#define DEV5_GPIO_GPIO1P0CTLO_REG 0x3B
#define DEV5_GPIO_GPIO1P1CTLO_REG 0x3C
#define DEV5_GPIO_GPIO1P0CTLI_REG 0x43
#define DEV5_GPIO_GPIO1P1CTLI_REG 0x44
#define DEV5_RESET_COLDRST_REG 0x53
#define DEV5_VREG_V1P8ACNT_REG 0x56
#define DEV5_VREG_V1P8SXCNT_REG 0x57
#define DEV5_VREG_VDDQCNT_REG 0x58
#define DEV5_VREG_V1P2ACNT_REG 0x59
#define DEV5_VREG_V1P2SXCNT_REG 0x5A
#define DEV5_VREG_V1P8AVSEL_REG 0x5B
#define DEV5_VREG_VDDQVSEL_REG 0x5C
#define DEV5_VREG_V2P8SXCNT_REG 0x5D
#define DEV5_VREG_V3P3ACNT_REG 0x5E
#define DEV5_VREG_V3P3SDCNT_REG 0x5F
#define DEV5_VREG_VNNCNT_REG 0x63
#define DEV5_VREG_VCC0CNT_REG 0x64
#define DEV5_VREG_VCC1CNT_REG 0x65
#define DEV5_VREG_VGGCNT_REG 0x66
#define DEV5_VREG_VSDIOCNT_REG 0x67
#define DEV5_VREG_V3P3AVSEL_REG 0x68
#define DEV5_VREG_VLDOCNT_REG 0x69
#define DEV5_VREG_VSWITCHCNT0_REG 0x6A
#define DEV5_VREG_VSWITCHCNT1_REG 0x6C
#define DEV5_ID_FWREV_REG 0x6D
#define DEV5_WAKE_SRCWAKECFG_REG 0x8B
#define DEV5_VREG_VPROG1ACNT_REG 0x90
#define DEV5_VREG_VPROG1BCNT_REG 0x91
#define DEV5_VREG_VPROG1FCNT_REG 0x95
#define DEV5_VREG_VPROG2DCNT_REG 0x99
#define DEV5_VREG_VPROG3ACNT_REG 0x9A
#define DEV5_VREG_VPROG3BCNT_REG 0x9B
#define DEV5_VREG_VPROG4ACNT_REG 0x9C
#define DEV5_VREG_VPROG4BCNT_REG 0x9D
#define DEV5_VREG_VPROG4CCNT_REG 0x9E
#define DEV5_VREG_VPROG4DCNT_REG 0x9F
#define DEV5_VREG_VPROG5ACNT_REG 0xA0
#define DEV5_VREG_VPROG5BCNT_REG 0xA1
#define DEV5_VREG_VPROG6ACNT_REG 0xA2
#define DEV5_VREG_VPROG6BCNT_REG 0xA3
#define DEV5_VREG_VPROG7ACNT_REG 0xA4
#define DEV5_BCU_VWARNA_CFG_REG 0xB4
#define DEV5_BCU_VWARNB_CFG_REG 0xB5
#define DEV5_BCU_VCRIT_CFG_REG 0xB6
#define DEV5_BCU_BCUDISB_BEH_REG 0xB8
#define DEV5_BCU_BCUDISCRIT_BEH_REG 0xB9
#define DEV5_BCU_BCUVSYS_DRP_BEH_REG 0xBA
#define DEV5_BCU_SBCUIRQ_REG 0xBB
#define DEV5_BCU_SBCUCTRL_REG 0xBC
#define DEV5_VREG_VPROG1AVSEL_REG 0xC0
#define DEV5_VREG_VPROG1BVSEL_REG 0xC1
#define DEV5_VREG_V1P8SXVSEL_REG 0xC2
#define DEV5_VREG_V1P2SXVSEL_REG 0xC3
#define DEV5_VREG_V1P2AVSEL_REG 0xC4
#define DEV5_VREG_VPROG1FVSEL_REG 0xC5
#define DEV5_VREG_VSDIOVSEL_REG 0xC6
#define DEV5_VREG_V2P8SXVSEL_REG 0xC7
#define DEV5_VREG_V3P3SDVSEL_REG 0xC8
#define DEV5_VREG_VPROG2DVSEL_REG 0xC9
#define DEV5_VREG_VPROG3AVSEL_REG 0xCA
#define DEV5_VREG_VPROG3BVSEL_REG 0xCB
#define DEV5_VREG_VPROG4AVSEL_REG 0xCC
#define DEV5_VREG_VPROG4BVSEL_REG 0xCD
#define DEV5_VREG_VPROG4CVSEL_REG 0xCE
#define DEV5_VREG_VPROG4DVSEL_REG 0xCF
#define DEV5_VREG_VPROG5AVSEL_REG 0xD0
#define DEV5_VREG_VPROG5BVSEL_REG 0xD1
#define DEV5_VREG_VPROG6AVSEL_REG 0xD2
#define DEV5_VREG_VPROG6BVSEL_REG 0xD3
#define DEV5_VREG_VPROG7AVSEL_REG 0xD4
#define DEV5_IRQ_THRMIRQ3_REG 0xD9
#define DEV5_IRQ_MTHRMIRQ3_REG 0xDA
#define DEV5_PB_PBCONFIG2_REG 0xDB
#define DEV5_PMON_PSOCMONCTL_REG 0xDC
#define DEV5_PMON_PSOCRSLTH_REG 0xDD
#define DEV5_PMON_PSOCRSLTL_REG 0xDE
#define DEV5_PMON_SOC_PALERTH_REG 0xE3
#define DEV5_PMON_SOC_PALERTL_REG 0xE4
#define DEV5_WAKE_WAKESRC2_REG 0xE5
#define DEV5_WAKE_SPLTIMER_REG 0xE6
#define DEV5_WAKE_SSPLTIMER_REG 0xE7
#define DEV5_PWRSRC_BATTDETCTRL0_REG 0xF0
#define DEV5_PWRSRC_BATTDETCTRL1_REG 0xF1
#define DEV5_EXTCTRL_SOCCTRL_REG 0xF8
#define DEV5_IRQ_MCRITIRQ_REG 0xFA
#define DEV5_BCU_ICCMAXVCC_CFG_REG 0xFB
#define DEV5_BCU_ICCMAXVNN_CFG_REG 0xFC
#define DEV5_BCU_ICCMAXVGG_CFG_REG 0xFD
#define DEV5_IRQ_MSVIDALERT_REG 0xFE
#endif

///
/// Dev6 Definitons
///
#define DEV6_ID_REG  0x00               //USB Type-C IP Revision Register
#define DEV6_USBCCTRL1_REG 0x01         //USB Type-C Control Register 1
#define DEV6_USBCCTRL2_REG 0x02         //USB Type-C Control Register 2
#define DEV6_USBCCTRL3_REG 0x03         //USB Type-C Control Register 3
#define DEV6_CC1CTRL_REG 0x04           //CC1 Pin Switch Control Register
#define DEV6_CC2CTRL_REG 0x05           //CC2 Pin Switch Control Register
#define DEV6_CCSEL_REG 0x06             //CC Select Register
#define DEV6_USBCSTATUS1_REG 0x07       //USB Type-C Status Register 1
#define DEV6_USBCSTATUS2_REG 0x08       //USB Type-C Status Register 2
#define DEV6_USBCSTATUS3_REG 0x09       //USB Type-C Status Register 3
#define DEV6_CC1CMP_REG 0x0A            //CC1 Pin Comparator Raw Status (no debounce) Register
#define DEV6_CC2CMP_REG 0x0B            //CC2 Pin Comparator Raw Status (no debounce) Register
#define DEV6_CC1STATUS_REG 0x0C         //CC1 Pin Status (after predebounce) Register
#define DEV6_CC2STATUS_REG 0x0D         //CC2 Pin Status (after predebounce) Register
#define DEV6_USBCIRQ1_REG 0x15          //USB Type-C Interrupt register 1
#define DEV6_USBCIRQ2_REG 0x16          //USB Type-C Interrupt register 2
#define DEV6_MUSBCIRQ1_REG 0x17         //IRQ Mask register for USBCIRQ1_REG
#define DEV6_MUSBCIRQ2_REG 0x18         //IRQ Mask register for USBCIRQ2_REG
#define DEV6_PDCFG1_REG 0x19            //USB PD Configuration Register 1
#define DEV6_PDCFG2_REG 0x1A            //USB PD Configuration Register 2
#define DEV6_PDCFG3_REG 0x1B            //USB PD Configuration Register 3
#define DEV6_PDSTATUS_REG 0x1C          //USB PD Status Register
#define DEV6_RXSTATUS_REG 0x1D          //USB PD Reception Status Register
#define DEV6_RXINFO_REG 0x1E            //USB PD Reception Information Register
#define DEV6_TXCMD_REG 0x1F             //USB PD Transmission Command Register
#define DEV6_TXINFO_REG 0x20            //USB PD Transmission Information Register
#define DEV6_RX_DATA1_REG 0x28          //Receive data
#define DEV6_RX_DATA2_REG 0x29          //Receive data

#define DEV6_RX_DATA30_REG 0x45         //Receive data
#define DEV6_TX_DATA1_REG 0x47          //Transmit data
#define DEV6_TX_DATA2_REG 0x48          //Transmit data

#define DEV6_TX_DATA30_REG 0x64         //Transmit data
#define DEV6_AFE_ADC1_REG 0x0E          //ADC1 Measurement Register
#define DEV6_AFE_TRIM1_REG 0x10         //Analog Front End Trim Register 1
#define DEV6_AFE_TRIM2_REG 0x11         //Analog Front End Trim Register 2
#define DEV6_AFE_PWRSTATE_REG 0x13      //AFE Power State Control Register
#define DEV6_AFE_SLICE_REG 0x14         //Slice Output Status Register
#define DEV6_AFE_TRIM3_REG 0x21         //Analog Front End Trim Register

#endif
