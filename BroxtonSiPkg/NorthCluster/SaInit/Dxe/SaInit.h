/** @file
Header file for SA Initialization Driver.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#ifndef _SA_INITIALIZATION_DRIVER_H_
#define _SA_INITIALIZATION_DRIVER_H_

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SaPolicy.h>
#include "LegacyRegion.h"
#include "IgdOpRegion.h"
//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
#include "HybridGraphicsInit.h"
#endif
#endif
//[-end-160628-IB07400748-modify]//
#include "PiDxe.h"
#include "SaRegs.h"
#include <SaAccess.h>
#include <Library/PeiDxeSmmMmPciLib.h>

//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//


typedef struct {
  UINT64  BaseAddr;
  UINT32  Offset;
  UINT32  AndMask;
  UINT32  OrMask;
} BOOT_SCRIPT_REGISTER_SETTING;


#if (ENBDT_PF_ENABLE == 1)
//BXTP
#define MIRROR_RANGE_MCHBAR                0x65c8 //0x6458 // 0x65C8
#define MOT_OUT_BASE_Bunit                 0x6AF0
#define MOT_OUT_MASK_Bunit                 0x6AF4
#define MOT_OUT_BASE_Aunit                 0x64C0
#define MOT_OUT_MASK_Aunit                 0x64C4
#define SPARE_BIOS_MCHBAR                  0x647C // BXTM 0x6474
#else
//BXTM
#define MIRROR_RANGE_MCHBAR                0x65C8 //0x6458 PO hang
#define MOT_OUT_BASE_Bunit                 0x6AF0
#define MOT_OUT_MASK_Bunit                 0x6AF4
#define MOT_OUT_BASE_Aunit                 0x64C0
#define MOT_OUT_MASK_Aunit                 0x64C4
#define SPARE_BIOS_MCHBAR                  0x647C //0x6474 // BXTM 0x6474
#endif

#define MSC0BAR                            0xA0108
#define MSC0DESTSZ                         0xA010C

#define A_CR_MIRROR_RANGE_1_DEFAULT_VALUE  0x00000049
#define A_CR_MIRROR_RANGE_2_DEFAULT_VALUE  0x00000008
#define B_CR_MOT_OUT_MASK_DEFAULT_VALUE  0x00074000

#define _1KB                 1024
#define _1MB                 1024*1024
#define _MB(x)               x * _1MB

#define NaturalAlignemnt(base,size)       (base & (~(size-1)))+size
#define Get_Aunit_mirror_size(x)          Aunit_mirror_1.r.MIRROR_BASE & 0x00000003
#define MCHBASE                           MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN)
#define MCHBASE_BAR                       MmioRead32(MCHBASE + 0x48) &~BIT0
#define MCHMmioRead32(offset)             MmioRead32((MCHBASE_BAR)+offset)
#define MCHMmioWrite32(offset, data)      MmioWrite32((MCHBASE_BAR)+offset, data)

#define NPKBASE                           MmPciBase(0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK)
#define NPKMmioRead32(offset)             MmioRead32(NPKBASE+offset)
#define NPKMmioWrite32(offset, data)      MmioWrite32(NPKBASE  + offset, data)

#define MTBWrite32(offset, data)          MmioWrite32((UINTN) (MTB_BASE_ADDRESS + offset), data)


typedef union {
  UINT32  ul;
  struct {
    UINT32  RESERVED_0   :14;    ///< 0:13 - Reserved
    /**
      14:28 - Specifies the size of the MOT region.
      If Request Address [38:24] ANDed with MOT_OUT_MASK[28:14] matches
      the MOT_OUT_BASE[28:14] then the request falls within the MOT_OUT region.
    **/
    UINT32  MOT_OUT_MASK :15;
    UINT32  RESERVED_1   : 1;    ///< 29 - Reserved
    UINT32  IA_IWB_EN    : 1;    ///< 30 - Enables implicit writebacks to protected region from IA caching agent.
    UINT32  GT_IWB_EN    : 1;    ///< 31 - Enables implicit writebacks to protected region from GT caching agent
  } r;
} MOT_OUT_MASK_0_0_0_MCHBAR;


typedef union {
  UINT32  ul;
  struct {
    UINT32  RESERVED_0   :14;    ///< 0:13 - Reserved
    /**
      14:28 - Specifies the size of the MOT region.
      If Request Address [38:24] ANDed with MOT_OUT_MASK[28:14] matches
      the MOT_OUT_BASE[28:14] then the request falls within the MOT_OUT region.
    **/
    UINT32  MOT_OUT_BASE :15;
    UINT32  RESERVED_1   : 1;    ///< 29 - Reserved
    UINT32  TR_EN        : 1;    ///< 30 - Enables implicit writebacks to protected region from IA caching agent.
    UINT32  IMR_EN       : 1;    ///< 31 - Enables implicit writebacks to protected region from GT caching agent
  } r;
} MOT_OUT_BASE_0_0_0_MCHBAR;


typedef union {
  UINT32  ul;
  struct {
    UINT32  MIRROR_BASE   :16;    ///< 0:15 - Mirror Base: specifies b38:b23 of HPA indicating the start of mirror packet buffer region
    UINT32  RESERVED_0    :16;    ///< 15:31 - Reserved.
  } r;
} MIRROR_RANGE_0_0_0_MCHBAR_1;

 typedef union {
   UINT32  ul;
   struct {
    UINT32  MIRROR_LIMIT  :16;   ///< 32:47 - Mirror Limit: specifies b38:b23 of HPA indicating the end of mirror packet buffer region.
    UINT32  RESERVED_0    :16;   ///< 48-63 - Reserved.
  } r;
}MIRROR_RANGE_0_0_0_MCHBAR_2;


typedef union {
  UINT32  ul;
  struct {
   UINT32  MSCBASE   :32;   // 32:0 - .
 } r;
} _MSC0BAR;

typedef union {
  UINT32  ul;
  struct {
   UINT32  MSCSIZE   :32;   // 32:0 - .
 } r;
} _MSC0DESTSZ;

typedef union {
  UINT32  ul;
  struct {
   UINT32  MIRROR_LIMIT  :16;   ///< 32:47 - Mirror Limit: specifies b38:b23 of HPA indicating the end of mirror packet buffer region.
   UINT32  RESERVED_0    :16;   ///< 48-63 - Reserved.
 } r;
}IMR_14_base;

typedef union {
  UINT32  ul;
  struct {
   UINT32  MIRROR_LIMIT  :16;   ///< 32:47 - Mirror Limit: specifies b38:b23 of HPA indicating the end of mirror packet buffer region.
   UINT32  RESERVED_0    :16;   ///< 48-63 - Reserved.
 } r;
}IMR_15_base;



/**
  [13:00] = PSMI Handler base address bits [31:18]
  [15:14] = PSMI Handler size (encoded)
  [17:16] = PSMI Expose Buffer size (encoded)
**/
typedef union {
  UINT32  ul;
  struct {
   UINT32  PSMI_Handler_base  :14;   ///< [13:00] = PSMI Handler base address bits [31:18]
   UINT32  PSMI_Handler_size  :2;    ///< [15:14] = PSMI Handler size (encoded)
   UINT32  PSMI_Expose_size   :2;    ///< [17:16] = PSMI Expose Buffer size (encoded)
   UINT32  Reserved           :14;   ///< PSMI Handler base address bits [31:18] TODO: need some info
 } r;
}A_CR_SPARE_BIOS_MCHBAR;

EFI_STATUS
PSMI_Mem_init(void);

#endif


