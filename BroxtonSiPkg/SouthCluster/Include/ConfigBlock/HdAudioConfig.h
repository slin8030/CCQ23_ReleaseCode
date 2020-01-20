/** @file
  HD-Audio policy

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
#ifndef _HDAUDIO_CONFIG_H_
#define _HDAUDIO_CONFIG_H_

#define HDAUDIO_CONFIG_REVISION 6
extern EFI_GUID gHdAudioConfigGuid;

#pragma pack (push,1)

/**
  Azalia verb table header
  Every verb table should contain this defined header and followed by azalia verb commands.
  Then update the verb table in SC_HDAUDIO_CONFIG.
**/
typedef struct {
  UINT32  VendorDeviceId;       ///< Codec Vendor/Device ID
  UINT8   RevisionId;           ///< Revision ID of the codec. 0xFF matches any revision.
  UINT8   SdiNo;                ///< SDI number, 0xFF matches any SDI.
  UINT16  DataDwords;           ///< Number of data DWORDs following the header.
} HDA_VERB_TABLE_HEADER;

/**
  SC_HDAUDIO_CONFIG produced by a platform driver specifying Azalia Codec verb table.
  This SC_HDAUDIO_CONFIG is consumed by the PEI modules.
**/
typedef struct {
  HDA_VERB_TABLE_HEADER         VerbTableHeader;
  UINT32                        VerbTableData[];
} HDAUDIO_VERB_TABLE;

///
/// The SC_HDAUDIO_CONFIG block describes the expected configuration of the Intel HD Audio feature.
///
enum SC_HDAUDIO_IO_BUFFER_OWNERSHIP {
  ScHdaIoBufOwnerHdaLink        = 0,   ///< HD-Audio link owns all the I/O buffers.
  ScHdaIoBufOwnerHdaLinkI2sPort = 1,   ///< HD-Audio link owns 4 and I2S port owns 4 of the I/O buffers.
  ScHdaIoBufOwnerI2sPort        = 3    ///< I2S0 and I2S1 ports own all the I/O buffers.
};

enum SC_HDAUDIO_IO_BUFFER_VOLTAGE {
  ScHdaIoBuf33V = 0,
  ScHdaIoBuf18V = 1
};

enum SC_HDAUDIO_DMIC_TYPE {
  ScHdaDmicDisabled = 0,
  ScHdaDmic2chArray = 1,
  ScHdaDmic4chArray = 2
};

enum SC_HDAUDIO_VC_TYPE {
  ScHdaVc0 = 0,
  ScHdaVc1 = 1,
  ScHdaVc2 = 2
};

typedef enum {
  ScHdaLinkFreq6MHz  = 0,
  ScHdaLinkFreq12MHz = 1,
  ScHdaLinkFreq24MHz = 2,
  ScHdaLinkFreq48MHz = 3,
  ScHdaLinkFreq96MHz = 4,
  ScHdaLinkFreqInvalid
} SC_HDAUDIO_LINK_FREQUENCY;

typedef enum  {
  ScHdaIDispMode2T = 0,
  ScHdaIDispMode1T = 1
} SC_HDAUDIO_IDISP_TMODE;

typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Add VerbTableEntryNum and VerbTablePtr.
    Revision 3: Deprecate AudioCtlPwrGate and AudioDspPwrGate.
    Revision 4: Add I2sConfiguration
    Revision 5: Add DspUaaCompliance
    Revision 6: Deprecate BiosCfgLockDOwn
  **/
  CONFIG_BLOCK_HEADER   Header;       ///< Config Block Header
  /**
    This member describes whether or not Intel HD Audio (Azalia) should be enabled.
    0: Disable, <b>1: Enable</b>
  **/
  UINT32 Enable               : 2;
  UINT32 DspEnable            : 1;    ///< DSP enablement: 0: Disable; <b>1: Enable</b>
  UINT32 Pme                  : 1;    ///< Azalia wake-on-ring, <b>0: Disable</b>; 1: Enable
  UINT32 IoBufferOwnership    : 2;    ///< I/O Buffer Ownership Select: <b>0: HD-A Link</b>; 1: Shared, HD-A Link and I2S Port; 3: I2S Ports
  UINT32 IoBufferVoltage      : 1;    ///< I/O Buffer Voltage Mode Select: <b>0: 3.3V</b>; 1: 1.8V
  UINT32 VcType               : 1;    ///< Virtual Channel Type Select: <b>0: VC0</b>, 1: VC1
  UINT32 HdAudioLinkFrequency : 4;    ///< HDA-Link frequency (SC_HDAUDIO_LINK_FREQUENCY enum): <b>2: 24MHz</b>, 1: 12MHz, 0: 6MHz
  UINT32 IDispLinkFrequency   : 4;    ///< iDisp-Link frequency (SC_HDAUDIO_LINK_FREQUENCY enum): <b>4: 96MHz</b>, 3: 48MHz
  UINT32 IDispLinkTmode       : 1;    ///< iDisp-Link T-Mode (SC_HDAUDIO_IDISP_TMODE enum): <b>0: 2T</b>, 1: 1T
  UINT32 DspUaaCompliance     : 1;    ///< Universal Audio Architecture compliance for DSP enabled system:<b>0: Not-UAA Compliant (Intel SST driver supported only)</b>,1: UAA Compliant (HDA Inbox driver or SST driver supported)
  UINT32 RsvdBits0            : 14;   ///< Reserved bits 1
  /**
    Bitmask of supported DSP endpoint configuration exposed via NHLT ACPI table:
  **/
  UINT32 DspEndpointDmic      : 2;    ///< DMIC Select (SC_HDAUDIO_DMIC_TYPE enum): 0: Disable; 1: 2ch array; <b>2: 4ch array</b>
  UINT32 DspEndpointBluetooth : 1;    ///< Bluetooth enablement: <b>0: Disable</b>; 1: Enable
  UINT32 DspEndpointI2sSkp    : 1;    ///< I2S SHK enablement: <b>0: Disable</b>; 1: Enable
  UINT32 DspEndpointI2sHp     : 1;    ///< I2S HP enablement: <b>0: Disable</b>; 1: Enable
  UINT32 AudioCtlPwrGate      : 1;    ///< @deprecated
  UINT32 AudioDspPwrGate      : 1;    ///< @deprecated
  UINT32 Mmt                  : 1;    ///< CSME Memory Transfers : 0: VC0, 1: VC2
  UINT32 Hmt                  : 1;    ///< Host Memory Transfers : 0: VC0, 1: VC2
  UINT32 BiosCfgLockDown      : 1;    ///< @deprecated
  UINT32 PwrGate              : 1;    ///< Power Gating : 0: Disable, 1: Enable
  UINT32 ClkGate              : 1;    ///< Clock Gating : 0: Disable, 1: Enable
  UINT32 I2sConfiguration     : 3;    ///< 0: Disable, 1: Realtek ALC298, 2: Realtek ALC286S
  UINT32 RsvdBits1            : 17;   ///< Reserved bits
  /**
    Bitmask of supported DSP features:
    [BIT0] - WoV; [BIT1] - BT Sideband; [BIT2] - Codec VAD; [BIT5] - BT Intel HFP; [BIT6] - BT Intel A2DP
    [BIT7] - DSP based speech pre-processing disabled; [BIT8] - 0: Intel WoV, 1: Windows Voice Activation
    Default is <b>zero</b>.
  **/
  UINT32 DspFeatureMask;
  /**
    Bitmask of supported DSP endpoint configuration exposed via NHLT ACPI table:
    Bitmask of supported DSP Post-Processing Modules.
    Specific post-processing module bit position must be coherent with the ACPI implementation:
    \_SB.PCI0.HDAS._DSM Function 3: Query Post Processing Module Support.
    DspPpModuleMask is passed to ACPI as 'ADPM' NVS variable
    Default is <b>zero</b>.
  **/
  UINT32 DspPpModuleMask;
  UINT16 ResetWaitTimer;              ///< < <b>(Test)</b> The delay timer after Azalia reset, the value is number of microseconds. Default is <b>300</b>.
  UINT8  Rsvd0[2];                    ///< Reserved bytes, align to multiple 4
  /**
    Number of the verb table entry defined in VerbTablePtr.
    Each entry points to a verb table which contains HDAUDIO_VERB_TABLE structure and verb command blocks.
  **/
  UINT8  VerbTableEntryNum;
  /**
    Pointer to a verb table array.
    This pointer points to 32bits address, and is only eligible and consumed in post mem phase.
    Each entry points to a verb table which contains HDAUDIO_VERB_TABLE structure and verb command blocks.
    The prototype of this is:
    HDAUDIO_VERB_TABLE **VerbTablePtr;
  **/
  UINT32 VerbTablePtr;

} SC_HDAUDIO_CONFIG;

#pragma pack (pop)

#endif // _HDAUDIO_CONFIG_H_
