/** @file
  Lock down policy

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
#ifndef _LOCK_DOWN_CONFIG_H_
#define _LOCK_DOWN_CONFIG_H_

#define LOCK_DOWN_CONFIG_REVISION 2
extern EFI_GUID gLockDownConfigGuid;

#pragma pack (push,1)

/**
  The SC_LOCK_DOWN_CONFIG block describes the expected configuration
  for security requirement.
**/
typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Add TcoLock
  **/
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header
  /**
    <b>(Test)</b> Enable SMI_LOCK bit to prevent writes to the Global SMI Enable bit. 0: Disable; <b>1: Enable</b>.
  **/
  UINT32  GlobalSmi          :  1;
  /**
    <b>(Test)</b> Enable BIOS Interface Lock Down bit to prevent writes to the Backup Control Register
    Top Swap bit and the General Control and Status Registers Boot BIOS Straps. 0: Disable; <b>1: Enable</b>.
  **/
  UINT32  BiosInterface      :  1;
  /**
    <b>(Test)</b> Enable RTC lower and upper 128 byte Lock bits to lock Bytes 38h-3Fh in the upper
    and lower 128-byte bank of RTC RAM. 0: Disable; <b>1: Enable</b>.
  **/
  UINT32  RtcLock            :  1;
  /**
    Enable the BIOS Lock Enable (BLE) feature and set EISS bit (D31:F5:RegDCh[5])
    for the BIOS region protection. When it is enabled, the BIOS Region can only be
    modified from SMM after EndOfDxe protocol is installed.
    Note: When BiosLock is enabled, platform code also needs to update to take care
    of BIOS modification (including SetVariable) in DXE or runtime phase after
    EndOfDxe protocol is installed. <b>0: Disable</b>; 1: Enable.
  **/
  UINT32  BiosLock           :  1;
  /**
    Enable InSMM.STS (EISS) in SPI
    If this bit is set, then WPD must be a '1' and InSMM.STS must be '1' also
    in order to write to BIOS regions of SPI Flash. If this bit is clear,
    then the InSMM.STS is a don't care.
    The BIOS must set the EISS bit while BIOS Guard support is enabled.
    In recovery path, platform can temporary disable EISS for SPI programming in
    PEI phase or early DXE phase.
    0: Clear EISS bit; <b>1: Set EISS bit</b>.
  **/
  UINT32  SpiEiss            : 1;
  /**
    This member describes the SwSmi value for Bios Lock. Default is <b>0xA9</b>
  **/
  UINT32  BiosLockSwSmiNumber: 8;
  /**
    This member describes the option to Halt TcoTimer. Default is <b>0x01</b>.
  **/
  UINT32  TcoLock            : 1;
  UINT32  RsvdBits0          : 18;          ///< Reserved bits
} SC_LOCK_DOWN_CONFIG;

#pragma pack (pop)

#endif // _LOCK_DOWN_CONFIG_H_
