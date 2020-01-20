/** @file
  Scs policy

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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
#ifndef _SCS_CONFIG_H_
#define _SCS_CONFIG_H_

#define SCS_CONFIG_REVISION 3
extern EFI_GUID gScsConfigGuid;

#pragma pack (push,1)

//
// Device Operating Mode
//
typedef enum {
  ScDisabled = 0,
  ScPciMode  = 1,
  ScAcpiMode = 2,
  ScDevModeMax
} SC_DEV_MODE;

//
// SCC eMMC Host Speed
//
enum SCC_EMMC_MODE {
  SccEmmcHs400 = 0,
  SccEmmcHs200 = 1,
  SccEmmcDdr50 = 2,
};

/**
  The SDIO_REG_CONFIG block is the structure defined in SC_SCS_CONFIG for SC.
  @note: the order defined below is for SDIO DLL registers settings, and MUST not change.
  Items defined will be accessed by its index in ConfigureSdioDll module
**/
typedef struct {
  UINT32 TxCmdCntl;
  UINT32 TxDataCntl1;
  UINT32 TxDataCntl2;
  UINT32 RxCmdDataCntl1;
  UINT32 RxCmdDataCntl2;
} SDIO_REG_CONFIG;

/**
  The SDCARD_REG_CONFIG block is the structure defined in SC_SCS_CONFIG for SC.
  @note: the order defined below is for SDCARD DLL registers settings, and MUST not change.
  Items defined will be accessed by its index in ConfigureSdioDll module
**/
typedef struct {
  UINT32 TxCmdCntl;
  UINT32 TxDataCntl1;
  UINT32 TxDataCntl2;
  UINT32 RxCmdDataCntl1;
  UINT32 RxStrobeCntl;
  UINT32 RxCmdDataCntl2;
} SDCARD_REG_CONFIG;

/**
  The EMMC_REG_CONFIG block is the structure defined in SC_SCS_CONFIG for SC.
  @note: the order defined below is for EMMC DLL registers settings, and MUST not change.
  Items defined will be accessed by its index in ConfigureSdioDll module
**/
typedef struct {
  UINT32 TxCmdCntl;
  UINT32 TxDataCntl1;
  UINT32 TxDataCntl2;
  UINT32 RxCmdDataCntl1;
  UINT32 RxStrobeCntl;
  UINT32 RxCmdDataCntl2;
  UINT32 MasterSwCntl;
} EMMC_REG_CONFIG;

//
// SCC eMMC Trace Length
//
#define SCC_EMMC_LONG_TRACE_LEN  0
#define SCC_EMMC_SHORT_TRACE_LEN 1

/**
  The SC_SCS_CONFIG block describes Storage and Communication Subsystem (SCS) settings for SC.
  @note: the order defined below is per the PCI BDF sequence, and MUST not change.
  Items defined will be accessed by its index in ScInit module
**/
typedef struct {
/**
    Revision 1: Init version
    Revision 2: Add SccEmmcTraceLength
    Revision 3: Add SDIO_REG_CONFIG and SDCARD_REG_CONFIG and EMMC_REG_CONFIG structures.
**/
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header //ChangeLog, Revision 1: Init version, Revision 2: Add SccEmmcTraceLength
  SC_DEV_MODE           SdcardEnable;             ///< Determine if SD Card is enabled - 0: Disabled, <b>1: Enabled</b>. Default is <b>ScPciMode</b>
  SC_DEV_MODE           EmmcEnable;               ///< Determine if eMMC is enabled - 0: Disabled, <b>1: Enabled</b>. Default is <b>ScPciMode</b>
  SC_DEV_MODE           UfsEnable;                ///< Determine if UFS is enabled - <b>0: Disabled</b>, 1: Enabled.
  SC_DEV_MODE           SdioEnable;               ///< Determine if SDIO is enabled - 0: Disabled, <b>1: Enabled</b>. Default is <b>ScPciMode</b>
  UINT32                EmmcHostMaxSpeed   : 2;   ///< Determine eMMC Mode. Default is <b>0: HS400</b>, 1: HS200, 2:DDR50
  UINT32                GppLock            : 1;   ///< Determine if GPP is locked <b>0: Disabled</b>; 1: Enabled
  UINT32                SccEmmcTraceLength : 2;   ///< Determine EMMC Trace length - 0: Longer Trace, Validated on Rvp <b>1: Shorter trace, Validated on FFD</b>. Default is <b>SCC_EMMC_LONG_TRACE_LEN</b>
  UINT32                RsvdBits           : 27;  ///< Reserved bits
  SDIO_REG_CONFIG       SdioRegDllConfig;         ///< SDIO DLL Configure structure
  SDCARD_REG_CONFIG     SdcardRegDllConfig;       ///< SDCARD DLL Configure structure
  EMMC_REG_CONFIG       EmmcRegDllConfig;         ///< eMMC DLL Configure structure
} SC_SCS_CONFIG;

#pragma pack (pop)

#endif // _SCS_CONFIG_H_
