/** @file
 IPMI Syn Bmc library header file.

 This file contains functions prototype that can easily access FUR data via
 using H2O IPMI FRU protocol.

;******************************************************************************
;* Copyright (c) 2016 - 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
//
// Server Feature - Insyde IPMI Package V1.0
//

#ifndef _IPMI_SYNC_BMC_LIB_H_
#define _IPMI_SYNC_BMC_LIB_H_

EFI_STATUS
IpmiSyncBmcPowerPolicy2 (
  UINT8             *PowerPolicy,
  BOOLEAN           SetToBmc,
//[-start-161121-IB09330314-add]//
  IN  BOOLEAN       IsHeci
//[-end-161121-IB09330314-add]//
  );

#endif

