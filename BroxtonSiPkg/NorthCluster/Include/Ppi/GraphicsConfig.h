/** @file
  Policy definition for Internal Graphics Config Block

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
#ifndef _GRAPHICS_CONFIG_H_
#define _GRAPHICS_CONFIG_H_
#pragma pack(1)

#define GRAPHICS_CONFIG_REVISION 1

extern EFI_GUID gGraphicsConfigGuid;

typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Offset 0-23 Config Block Header
  UINT32     CdClock            : 3;
  UINT32     PeiGraphicsPeimInit: 1;   ///< 0- Disable 1- Enable
  UINT32     RsvdBits0          : 4;   ///< Reserved for future use
  UINT8      Rsvd1[2];                 ///< Offset 14
  UINT32     GttMmAdr;                 ///< Offset 16 Address of System Agent GTTMMADR: Default is <b>0xDF000000</b>
  VOID*      LogoPtr;                  ///< Offset 20 Address of Logo to be displayed in PEI
  UINT32     LogoSize;                 ///< Offset 24 Logo Size
  VOID*      GraphicsConfigPtr;        ///< Offset 28 Address of the Graphics Configuration Table
  UINT32     GmAdr;                    ///< Offset 32 Address of System Agent GMADR: Default is <b>0xC0000000</b>
  UINT8      PmSupport;
  UINT8      PavpEnable;
  UINT8      EnableRenderStandby;
  UINT8      PavpPr3;
} GRAPHICS_CONFIG;

#pragma pack()
#endif // _GRAPHICS_CONFIG_H_
