/** @file
  CPU policy PPI produced by a platform driver specifying various
  expected CPU settings. This PPI is consumed by CPU PEI modules.

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

@par Specification
**/
#ifndef _TXT_CONFIG_H_
#define _TXT_CONFIG_H_

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gTxtConfigGuid;

#pragma pack(push, 1)

#define TXT_CONFIG_REVISION  1


/**
  This structure describes various settings required for TXT configuration.
**/
typedef struct {
  CONFIG_BLOCK_HEADER Header;                ///< Offset 0 GUID number for main entry of config block
  UINT8                Reserved[3];           ///< Offset 21-23 Reserved for future use
  UINT64               SinitMemorySize;       ///< Size of SINIT module if installed in flash part. Zero otherwise.
  UINT64               TxtHeapMemorySize;     ///< Size of memory reserved for TXT Heap. This memory is used by MLE.
  EFI_PHYSICAL_ADDRESS TxtDprMemoryBase;      ///< Base address of DPR protected memory reserved for Intel TXT component.
  UINT64               TxtDprMemorySize;      ///< Size of DPR protected memory reserved for Intel TXT component.
  ///
  /// Base address of BIOS ACM in flash part. It can be passed through platform code for customization;
  /// Intel TXT reference code would skip searching the BIOS ACM in PEI firmware volume if the field is not zero.
  ///
  EFI_PHYSICAL_ADDRESS BiosAcmBase;
  UINT64               BiosAcmSize;           ///< Size of ACM Binary.
  ///
  /// Base address of CPU micro code patch loaded into BSP. It can be passed through platform code for customization;
  /// Intel TXT reference code would skip searching the micro code path in PEI firmware volume if the field is not zero.
  ///
  EFI_PHYSICAL_ADDRESS McuUpdateDataAddr;
  UINT64               TgaSize;               ///< Size of Trusted Graphics Aperture if supported by chipset.
  EFI_PHYSICAL_ADDRESS TxtLcpPdBase;          ///< Base address of Platform Default Launch Control Policy data if installed in flash part. Zero otherwise.
  UINT64               TxtLcpPdSize;          ///< Size of Platform Default Launch Control Policy data if installed in flash part. Zero otherwise.

  UINT32               ResetAux         :  1; ///< <b>(Test)</b> Reset Auxiliary content, <b>0: Disabled</b>, 1: Enabled
  UINT32               TxtAcheckRequest :  1; ///< <b>(Test)</b> AcheckRequest <b>0: Disabled</b>, 1: Enabled. When Enabled, it will call Acheck regardless of crashcode value
  UINT32               RsvdBits         : 30; ///< Bits reserved for DWORD alignment.

} TXT_CONFIG;

#pragma pack(pop)
#endif
