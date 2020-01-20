/** @file
  GUID used for Platform Emmc Hs400 Tuning Info NVRAM Data

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _PLATFORM_EMMC_HS400_TUNING_INFO_GUID_H_
#define _PLATFORM_EMMC_HS400_TUNING_INFO_GUID_H_

#define HS400_TUNING_DATA_VAR         L"Hs400TuningData"

//
// This GUID is used to identify 'Hs400TuningData' variable
//
extern EFI_GUID gPlatformEmmcHs400TuningInfoGuid;

/**
 This structure describes the variable to be set with the returned EMMC_TUNING_DATA
 from EMMC_TUNE protocol.
**/
typedef struct _PLATFORM_EMMC_TUNING_DATA {
  /**
    eMMC HS400 Data Validation Status
    0: HS400 Data is invalid, 1: HS400 Data is valid
  **/
  UINT8    Hs400DataValid;
  UINT8    Hs400RxStrobe1Dll;  ///< Rx Strobe Delay Control - Rx Strobe Delay DLL 1 (HS400 Mode)
  UINT8    Hs400TxDataDll;     ///< Tx Data Delay Control 1 - Tx Data Delay (HS400 Mode)
} PLATFORM_EMMC_TUNING_DATA;


#endif
