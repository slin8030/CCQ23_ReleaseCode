/** @file
    BXT-P Ax HSIO Header File

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

#ifndef _SC_BXTP_HSIO_AX_H_
#define _SC_BXTP_HSIO_AX_H_

#define SC_BXTP_HSIO_VER_AX 0x6

extern SC_SBI_HSIO_TABLE_STRUCT  ScBxtpHsioTable_Ax[80];
extern UINT16 ScBxtpUsb3HsioAddress[6];
extern UINT16 ScBxtpPcieHsioAddress[6];
extern UINT16 ScBxtpSataHsioAddress[2];
extern SC_SIDEBAND_TABLE_STRUCT ScBxtpUsb2PhyCommonTable_A0[7];
extern SC_SIDEBAND_TABLE_STRUCT ScBxtpUsb2PhyPerPortTable_A0[3];
extern SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_Ax_RateA[61];
extern SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_Ax_RateB[61];
extern SC_SIDEBAND_TABLE_STRUCT ScBxtpSsicModphy_Ax_RateB_Dlane_PG[67];

#endif //_SC_BXTP_HSIO_AX_H_
