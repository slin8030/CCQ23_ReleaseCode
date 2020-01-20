/** @file
  Header file with all common HSIO information

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
#ifndef _SC_HSIO_H_
#define _SC_HSIO_H_

#define SC_LANE_OWN_MIPI_A             0x08
#define SC_LANE_OWN_MIPI_B             0x09
#define SC_LANE_OWN_COMMON             0x10
#define SC_LANE_MCAST                  0x11

#define SC_HSIO_LANE_GROUP_NO          0x09
#define SC_HSIO_LANE_GROUP_COMMON_LANE 0x00
#define SC_HSIO_LANE_GROUP_PCIE        0x01
#define SC_HSIO_LANE_GROUP_USB3        0x04
#define SC_HSIO_LANE_GROUP_SATA        0x05
#define SC_HSIO_LANE_GROUP_SSIC        0x06
#define SC_MODPHY0_LOS1_LANE_START     0x00
#define SC_MODPHY0_LOS1_LANE_END       0x07
#define SC_MODPHY0_LOS2_LANE_START     0x00
#define SC_MODPHY0_LOS2_LANE_END       0x01

/**
  SC SBI HSIO table data structure
**/
typedef struct {
  UINT32 PortId;
  UINT32 AndMask;
  UINT32 OrMask;
  UINT16 Offset;
  UINT8  LanePhyMode;
} SC_SBI_HSIO_TABLE_STRUCT;

typedef struct {
  UINT16 Address;
  UINT32 AndMask;
  UINT32 OrMask;
} SC_SIDEBAND_TABLE_STRUCT;

#include <Private/ScBxtHsioBx.h>
//[-start-160828-IB07400775-add]//
#include <Private/ScBxtpHsioAx.h>
//[-end-160828-IB07400775-add]//
#include <Private/ScBxtpHsioBx.h>

#endif //_SC_HSIO_H_

