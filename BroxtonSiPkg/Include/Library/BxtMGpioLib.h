/** @file
  This library provides the BxtM GPIO library definitions.

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

#ifndef _BXTP_GPIOLIB_H_
#define _BXTP_GPIOLIB_H_

#define GPIO_PADBAR                     0x400

#define GPIO_NORTH_COMMUNITY_LENGTH     0x698
#define GPIO_NORTHWEST_COMMUNITY_LENGTH 0x640
#define GPIO_WEST_COMMUNITY_LENGTH      0x550
#define GPIO_SOUTHWEST_COMMUNITY_LENGTH 0x4F8
#define GPIO_SOUTH_COMMUNITY_LENGTH     0x4A0

// GPIO pad and offset definition as GPIO HAS
// North community GPIO pad definition
#define N_GPIO_0              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0400
#define N_GPIO_1              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0408
#define N_GPIO_2              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0410
#define N_GPIO_3              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0418
#define N_GPIO_4              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0420
#define N_GPIO_5              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0428
#define N_GPIO_6              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0430
#define N_GPIO_7              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0438
#define N_GPIO_8              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0440
#define N_GPIO_9              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0448
#define N_GPIO_10             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0450
#define N_GPIO_11             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0458
#define N_GPIO_12             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0460
#define N_GPIO_13             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0468
#define N_GPIO_14             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0470
#define N_GPIO_15             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0478
#define N_GPIO_16             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0480
#define N_GPIO_17             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0488
#define N_GPIO_18             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0490
#define N_GPIO_19             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0498
#define N_GPIO_20             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04A0
#define N_GPIO_21             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04A8
#define N_GPIO_22             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04B0
#define N_GPIO_23             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04B8
#define N_GPIO_24             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04C0
#define N_GPIO_25             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04C8
#define N_GPIO_26             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04D0
#define N_GPIO_27             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04D8
#define N_GPIO_28             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04E0
#define N_GPIO_29             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04E8
#define N_GPIO_30             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04F0
#define N_GPIO_31             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x04F8
#define N_GPIO_32             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0500
#define N_GPIO_33             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0508
#define N_GPIO_34             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0510
#define N_GPIO_35             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0518
#define N_GPIO_36             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0520
#define N_GPIO_37             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0528
#define N_GPIO_38             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0530
#define N_GPIO_39             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0538
#define N_GPIO_40             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0540
#define N_GPIO_41             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0548
#define N_GPIO_42             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0550
#define N_GPIO_43             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0558
#define N_GPIO_44             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0560
#define N_GPIO_45             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0568
#define N_GPIO_46             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0570
#define N_GPIO_47             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0578
#define N_GPIO_48             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0580
#define N_GPIO_49             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0588
#define N_GPIO_50             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0590
#define N_GPIO_51             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0598
#define N_GPIO_52             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05A0
#define N_GPIO_53             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05A8
#define N_GPIO_54             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05B0
#define N_GPIO_55             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05B8
#define N_GPIO_56             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05C0
#define N_GPIO_57             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05C8
#define N_GPIO_58             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05D0
#define N_GPIO_59             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05D8
#define N_GPIO_60             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05E0
#define N_GPIO_61             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05E8
#define N_GPIO_62             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05F0
#define N_GPIO_63             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x05F8
#define N_GPIO_64             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0600
#define N_GPIO_65             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0608
#define N_GPIO_66             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0610
#define N_GPIO_67             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0618
#define N_GPIO_68             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0620
#define N_GPIO_69             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0628
#define N_GPIO_70             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0630
#define N_GPIO_71             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0638
#define N_GPIO_72             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0640
#define N_GPIO_73             (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0648
#define N_TCK                 (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0650
#define N_TRST_B              (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0658
#define N_TMS                 (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0660
#define N_TDI                 (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0668
#define N_CX_PMODE            (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0670
#define N_CX_PREQ_B           (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0678
#define N_JTAGX               (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0680
#define N_CX_PRDY_B           (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0688
#define N_TDO                 (((UINT32)GPIO_NORTH_COMMUNITY)<<16)+0x0690

// Northwest community GPIO pad definition
#define NW_PMC_SPI_FS0        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0400
#define NW_PMC_SPI_FS1        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0408
#define NW_PMC_SPI_FS2        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0410
#define NW_PMC_SPI_RXD        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0418
#define NW_PMC_SPI_TXD        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0420
#define NW_PMC_SPI_CLK        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0428
#define NW_PMC_UART_RXD       (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0430
#define NW_PMC_UART_TXD       (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0438
#define NW_PMIC_PWRGOOD       (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0440
#define NW_PMIC_RESET_B       (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0448
#define NW_RTC_CLK            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0450
#define NW_PMIC_SDWN_B        (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0458
#define NW_PMIC_BCUDISW2      (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0460
#define NW_PMIC_BCUDISCRIT    (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0468
#define NW_PMIC_THERMTRIP_B   (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0470
#define NW_PMIC_STDBY         (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0478
#define NW_SVID0_ALERT_B      (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0480
#define NW_SVID0_DATA         (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0488
#define NW_SVID0_CLK          (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0490
#define NW_PMIC_I2C_SCL       (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0498
#define NW_PMIC_I2C_SDA       (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04a0
#define NW_GPIO_74            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04a8
#define NW_GPIO_75            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04b0
#define NW_GPIO_76            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04b8
#define NW_GPIO_77            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04c0
#define NW_GPIO_78            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04c8
#define NW_GPIO_79            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04d0
#define NW_GPIO_80            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04d8
#define NW_GPIO_81            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04e0
#define NW_GPIO_82            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04e8
#define NW_GPIO_83            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04f0
#define NW_GPIO_84            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x04f8
#define NW_GPIO_85            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0500
#define NW_GPIO_86            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0508
#define NW_GPIO_87            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0510
#define NW_GPIO_88            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0518
#define NW_GPIO_89            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0520
#define NW_GPIO_90            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0528
#define NW_GPIO_91            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0530
#define NW_GPIO_92            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0538
#define NW_GPIO_93            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0540
#define NW_GPIO_94            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0548
#define NW_GPIO_95            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0550
#define NW_GPIO_96            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0558
#define NW_PROCHOT_B          (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0560
#define NW_GPIO_97            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0568
#define NW_GPIO_98            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0570
#define NW_GPIO_99            (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0578
#define NW_GPIO_100           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0580
#define NW_GPIO_101           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0588
#define NW_GPIO_102           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0590
#define NW_GPIO_103           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0598
#define NW_FST_SPI_CLK_FB     (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05a0
#define NW_GPIO_104           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05a8
#define NW_GPIO_105           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05b0
#define NW_GPIO_106           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05b8
#define NW_GPIO_107           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05c0
#define NW_GPIO_109           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05c8
#define NW_GPIO_110           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05d0
#define NW_GPIO_111           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05d8
#define NW_GPIO_112           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05e0
#define NW_GPIO_113           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05e8
#define NW_GPIO_114           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05f0
#define NW_GPIO_115           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x05f8
#define NW_GPIO_116           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0600
#define NW_GPIO_117           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0608
#define NW_GPIO_118           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0610
#define NW_GPIO_119           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0618
#define NW_GPIO_120           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0620
#define NW_GPIO_121           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0628
#define NW_GPIO_122           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0630
#define NW_GPIO_123           (((UINT32)GPIO_NORTHWEST_COMMUNITY)<<16)+0x0638

// West community GPIO pad definition
#define W_GPIO_124            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0400
#define W_GPIO_125            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0408
#define W_GPIO_126            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0410
#define W_GPIO_127            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0418
#define W_GPIO_128            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0420
#define W_GPIO_129            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0428
#define W_GPIO_130            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0430
#define W_GPIO_131            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0438
#define W_GPIO_132            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0440
#define W_GPIO_133            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0448
#define W_GPIO_134            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0450
#define W_GPIO_135            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0458
#define W_GPIO_136            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0460
#define W_GPIO_137            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0468
#define W_GPIO_138            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0470
#define W_GPIO_139            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0478
#define W_GPIO_140            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0480
#define W_GPIO_141            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0488
#define W_GPIO_142            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0490
#define W_GPIO_143            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0498
#define W_GPIO_144            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04a0
#define W_GPIO_145            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04a8
#define W_GPIO_146            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04b0
#define W_GPIO_147            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04b8
#define W_GPIO_148            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04c0
#define W_GPIO_149            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04c8
#define W_GPIO_150            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04d0
#define W_GPIO_151            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04d8
#define W_GPIO_152            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04e0
#define W_GPIO_153            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04e8
#define W_GPIO_154            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04f0
#define W_GPIO_155            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x04f8
#define W_MODEM_CLKREQ        (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0500
#define W_DGCLKDBG_PMC_0      (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0508
#define W_DGCLKDBG_PMC_1      (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0510
#define W_DGCLKDBG_PMC_2      (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0518
#define W_DGCLKDBG_ICLK_0     (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0520
#define W_DGCLKDBG_ICLK_1     (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0528
#define W_OSC_CLK_OUT_0       (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0530
#define W_OSC_CLK_OUT_1       (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0538
#define W_OSC_CLK_OUT_2       (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0540
#define W_OSC_CLK_OUT_3       (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0548
#define W_GPIO_209            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0550
#define W_GPIO_210            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0558
#define W_GPIO_211            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0560
#define W_GPIO_212            (((UINT32)GPIO_WEST_COMMUNITY)<<16)+0x0568

// Southwest community GPIO pad definition
#define SW_GPIO_156           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0400
#define SW_GPIO_157           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0408
#define SW_GPIO_158           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0410
#define SW_GPIO_159           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0418
#define SW_GPIO_160           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0420
#define SW_GPIO_161           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0428
#define SW_GPIO_162           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0430
#define SW_GPIO_163           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0438
#define SW_GPIO_164           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0440
#define SW_GPIO_165           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0448
#define SW_GPIO_166           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0450
#define SW_GPIO_167           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0458
#define SW_GPIO_168           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0460
#define SW_GPIO_169           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0468
#define SW_GPIO_170           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0470
#define SW_GPIO_171           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0478
#define SW_GPIO_172           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0480
#define SW_GPIO_173           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0488
#define SW_GPIO_174           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0490
#define SW_GPIO_175           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x0498
#define SW_GPIO_176           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04a0
#define SW_GPIO_177           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04a8
#define SW_GPIO_178           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04b0
#define SW_GPIO_179           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04b8
#define SW_GPIO_180           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04c0
#define SW_GPIO_181           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04c8
#define SW_GPIO_182           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04d0
#define SW_GPIO_183           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04d8
#define SW_GPIO_184           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04e0
#define SW_GPIO_185           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04e8
#define SW_GPIO_186           (((UINT32)GPIO_SOUTHWEST_COMMUNITY)<<16)+0x04f0

// South community GPIO pad definition
#define S_GPIO_187            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0400
#define S_GPIO_188            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0408
#define S_GPIO_189            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0410
#define S_GPIO_190            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0418
#define S_GPIO_191            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0420
#define S_GPIO_192            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0428
#define S_GPIO_193            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0430
#define S_GPIO_194            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0438
#define S_GPIO_195            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0440
#define S_GPIO_196            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0448
#define S_GPIO_197            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0450
#define S_GPIO_198            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0458
#define S_GPIO_199            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0460
#define S_GPIO_200            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0468
#define S_GPIO_201            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0470
#define S_GPIO_202            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0478
#define S_GPIO_203            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0480
#define S_GPIO_204            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0488
#define S_GPIO_205            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0490
#define S_GPIO_206            (((UINT32)GPIO_SOUTH_COMMUNITY)<<16)+0x0498

#endif
