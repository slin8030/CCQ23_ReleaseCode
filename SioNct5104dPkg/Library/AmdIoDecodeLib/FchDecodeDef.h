/** @file

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _FCH_DECODE_DEF_H_
#define _FCH_DECODE_DEF_H_

#include <Base.h>

#define FCH_LPC_BUS              0
#define FCH_LPC_DEV              0x14
#define FCH_LPC_FUN              0x3
#define FCH_PCI_LPC_REG_ADDR(a)  ((1 << 31) + (FCH_LPC_BUS << 16) + (FCH_LPC_DEV << 11) + (FCH_LPC_FUN << 8) + (a))

#define R_FCH_IO_DECODE1         0x44
#define  B_IO_378_37F            (1 << 0) //BIT0
#define  B_IO_778_77F            (1 << 1) //BIT1
#define  B_IO_278_27F            (1 << 2) //BIT2
#define  B_IO_678_67F            (1 << 3) //BIT3
#define  B_IO_3BC_3BF            (1 << 4) //BIT4
#define  B_IO_7BC_7BF            (1 << 5) //BIT5
#define  B_IO_3F8_3FF            (1 << 6) //BIT6
#define  B_IO_2F8_2FF            (1 << 7) //BIT7
#define  B_IO_220_227            (1 << 8) //BIT8
#define  B_IO_228_22F            (1 << 9) //BIT9
#define  B_IO_238_23F            (1 << 10) //BIT10
#define  B_IO_2E8_2EF            (1 << 11) //BIT11
#define  B_IO_338_33F            (1 << 12) //BIT12
#define  B_IO_3E8_3EF            (1 << 13) //BIT13
#define  B_IO_230_233            (1 << 14) //BIT14
#define  B_IO_240_253            (1 << 15) //BIT15
#define  B_IO_260_273            (1 << 16) //BIT16
#define  B_IO_280_293            (1 << 17) //BIT17
#define  B_IO_300_301            (1 << 18) //BIT18
#define  B_IO_310_311            (1 << 19) //BIT19
#define  B_IO_320_321            (1 << 20) //BIT20
#define  B_IO_330_331            (1 << 21) //BIT21
#define  B_IO_530_537            (1 << 22) //BIT22
#define  B_IO_604_60B            (1 << 23) //BIT23
#define  B_IO_E80_E87            (1 << 24) //BIT24
#define  B_IO_F40_F47            (1 << 25) //BIT25
#define  B_IO_3F0_3F7            (1 << 26) //BIT26
#define  B_IO_370_377            (1 << 27) //BIT27
#define  B_IO_200_20F            (1 << 28) //BIT28
#define  B_IO_60_64              (1 << 29) //BIT29
#define  B_IO_62_66              (1 << 30) //BIT30
#define  B_IO_388_389            (1 << 21) //BIT31

#define R_FCH_IO_DECODE2         0x48
#define  B_IO_2E_2F              (1 << 0) //BIT0
#define  B_IO_4E_4F              (1 << 1) //BIT1
#define  B_WIDE_IO_0_EN          (1 << 2) //BIT2
#define  B_MEM_RANGE_PORT_EN     (1 << 5) //BIT5
#define  B_IO_70_73              (1 << 6) //BIT6
#define  B_IO_400_43F            (1 << 16) //BIT16
#define  B_IO_480_2BF            (1 << 17) //BIT17
#define  B_IO_500_53F            (1 << 18) //BIT18
#define  B_IO_580_5BF            (1 << 19) //BIT19
#define  B_MEM_PORT_EN           (1 << 20) //BIT20
#define  B_IO_80                 (1 << 21) //BIT21
#define  B_IO_4700_470B          (1 << 22) //BIT22
#define  B_IO_FD60_FD6F          (1 << 23) //BIT23
#define  B_WIDE_IO_1_EN          (1 << 24) //BIT24
#define  B_WIDE_IO_2_EN          (1 << 25) //BIT25

#define R_WIDE_IO_0_PORT         0x64
#define R_WIDE_IO_1_PORT         0x66
#define R_WIDE_IO_2_PORT         0x90

#define R_WIDE_IO_RANGE          0x74
#define  B_WIDE_IO_0_16BYTE      BIT0
#define  B_WIDE_IO_1_16BYTE      BIT2
#define  B_WIDE_IO_2_16BYTE      BIT3

#endif
