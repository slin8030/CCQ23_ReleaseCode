/** @file
  File name:    Jedec_spec.h

  Implements JEDEC register definitions for LPDDR3 and LPDDR4. Incomplete, evolves as need.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2016 Intel Corporation.

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
#ifndef _JEDEC_SPEC_H_
#define _JEDEC_SPEC_H_

#define JEDEC_MR8 8
//
//MR8 register definitions
//
typedef enum {
  //JEDEC Spec: JESD209-3 pp 22
  RSVD00B_3T = 0,
  RSVD01B_3T = 1,
  RSVD10B_3T = 2,
  S8_SDRAM = 3 //11b
} JEDEC_LPDDR3_Type;

typedef enum {
  //JEDEC Spec: JESD209-3 pp 22
  RSVD0000B_3D = 0,
  RSVD0001B_3D = 1,
  RSVD0010B_3D = 2,
  RSVD0011B_3D = 3,
  RSVD0100B_3D = 4,
  RSVD0101B_3D = 5,
  Die4Gb_LPDDR3  = 6, //0110b
  Die8Gb_LPDDR3  = 7, //0111b
  Die16Gb_LPDDR3 = 8, //1000b
  Die32Gb_LPDDR3 = 9, //1001b
  RSVD1010B_3D = 10,
  RSVD1011B_3D = 11,
  RSVD1100B_3D = 12,
  Die12Gb_LPDDR3 = 13, //1101b
  Die6Gb_LPDDR3 = 14, //1110b
  RSVD1111B_3D = 15,
} JEDEC_LPDDR3_Density;

typedef enum {
  //JEDEC Spec: JESD209-3 pp 22
  x32 = 0,
  x16 = 1,
  RSVD10B_3I = 2,
  RSVD11B_3I = 3
} JEDEC_LPDDR3_IOW;

typedef enum {
  //JEDEC Spec: JESD209-4 pp 24
  S16_SDRAM = 0, //00b = 16n pre-fetch
  RSVD01B_4T = 1,
  RSVD10B_4T = 2,
  RSVD11B_4T = 3
} JEDEC_LPDDR4_Type;

typedef enum {
  //JEDEC Spec: JESD209-4 pp 24
  Die4Gb_2GbCh_LPDDR4   = 0, //0000b
  Die6Gb_3GbCh_LPDDR4   = 1, //0001b
  Die8Gb_4GbCh_LPDDR4   = 2, //0010b
  Die12Gb_6GbCh_LPDDR4  = 3, //0011b
  Die16Gb_8GbCh_LPDDR4  = 4, //0100b
  Die24Gb_12GbCh_LPDDR4 = 5, //0101b
  Die32Gb_16GbCh_LPDDR4 = 6, //0110b
  RSVD0111B_4D = 7,
  RSVD1000B_4D = 8,
  RSVD1001B_4D = 9,
  RSVD1010B_4D = 10,
  RSVD1011B_4D = 11,
  RSVD1100B_4D = 12,
  RSVD1101B_4D = 13,
  RSVD1110B_4D = 14,
  RSVD1111B_4D = 15,
} JEDEC_LPDDR4_Density;

typedef enum {
  //JEDEC Spec: JESD209-3 pp 22
  x16_4I = 0,
  RSVD01B_4I = 1,
  RSVD10B_4I = 2,
  RSVD11B_4I = 3
} JEDEC_LPDDR4_IOW;

typedef union {
  //JEDEC Spec: JESD209-3 pp 22 && JESD209-4 pp 24
  struct {
    //
    // Bits[1:0], RO
    //
    UINT8 type : 2;
    //
    // Bits[5:2], RO
    //
    UINT8 density : 4;
    //
    // Bits[7:6], R0
    //
    UINT8 io_width : 2;
  } Bits;
  UINT8 Raw;
} JEDEC_MR8_LPDDR3N4_STRUCT;

#endif // _JEDEC_SPEC_H_
