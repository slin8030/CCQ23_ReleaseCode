/** @file
  Azalia controller initialize in PEI phase.

;******************************************************************************
;* Copyright (c) 2013-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**
 @file PchAzalia.c
 Initializes the PCH Azalia codec.

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification
**/

//#include "PchInit.h"
#include <ChipsetSetupConfig.h>
#include <ScAccess.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/VerbTable.h>
#include <Ppi/AzaliaPolicy.h>
#include <PlatformBaseAddresses.h>
#include "SaRegs.h"
#include "SaAccess.h"
//
// This file is modified from Intel reference code at DXE phase
// Since it is now in PEI phase, no need to use (EDKII)S3BootScriptSaveMemWrite... any more.
// We attach S3BootScriptLib|MdePkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf on [LibraryClasses.common.PEIM] platformpkg.dsc
// The variable "RootComplexBar" must to be replaced with  RCBA definition value.
//

//
// Macro Definition
//
#define RootComplexBar      RCBA_BASE_ADDRESS 

#define PCH_DEVICE_ENABLE   1
#define PCH_DEVICE_DISABLE  0
//
// Module global variables
//
//extern EFI_PCH_S3_SUPPORT_PROTOCOL           *mPchS3Support;

#if 0
/**
  Polling the Status bit

  @param[in] StatusReg            The regsiter address to read the status
  @param[in] PollingBitMap        The bit mapping for polling
  @param[in] PollingData          The Data for polling

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             Polling the bit map time out
**/
EFI_STATUS
StatusPolling (
  IN      UINT32          StatusReg,
  IN      UINT16          PollingBitMap,
  IN      UINT16          PollingData
  )
{
  UINT32  LoopTime;

  for (LoopTime = 0; LoopTime < AZALIA_MAX_LOOP_TIME; LoopTime++) {
    if ((MmioRead16 (StatusReg) & PollingBitMap) == PollingData) {
      break;
    } else {
      PchPmTimerStall (AZALIA_WAIT_PERIOD);
    }
  }

  if (LoopTime >= AZALIA_MAX_LOOP_TIME) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Send the command to the codec via the Immediate Command mechanism is written
  to the IC register

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in,out] CodecCommandData      The Codec Command to be sent to the codec
  @param[in] ReadBack                   Whether to get the response received from the codec

  @retval EFI_DEVICE_ERROR              Device status error, operation failed
  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
SendCodecCommand (
  IN      UINT32          HdaBar,
  IN OUT  UINT32          *CodecCommandData,
  IN      BOOLEAN         ReadBack
  )
{
  EFI_STATUS  Status;

  Status = StatusPolling (HdaBar + R_HDA_ICS, (UINT16) B_HDA_ICS_ICB, (UINT16) 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ICB bit is not zero before SendCodecCommand! \n"));
    return EFI_DEVICE_ERROR;
  }

  MmioWrite32 (HdaBar + R_HDA_IC, *CodecCommandData);
  MmioOr16 ((UINTN) (HdaBar + R_HDA_ICS), (UINT16) ((B_HDA_ICS_IRV | B_HDA_ICS_ICB)));

  Status = StatusPolling (HdaBar + R_HDA_ICS, (UINT16) B_HDA_ICS_ICB, (UINT16) 0);
  if (EFI_ERROR (Status)) {
    MmioAnd16 ((UINTN) (HdaBar + R_HDA_ICS), (UINT16)~(B_HDA_ICS_ICB));
    return Status;
  }

  if (ReadBack == TRUE) {
    if ((MmioRead16 (HdaBar + R_HDA_ICS) & B_HDA_ICS_IRV) != 0) {
      *CodecCommandData = MmioRead32 (HdaBar + R_HDA_IR);
    } else {
      DEBUG ((EFI_D_ERROR, "SendCodecCommand: ReadBack fail! \n"));
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}

/**
  Set a "Send Codec Command" S3 dispatch item

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in,out] CodecCommandData      The Codec Command to be sent to the codec

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
SendCodecCommandS3Item (
  IN      UINT32          HdaBar,
  IN OUT  UINT32          CodecCommandData
  )
{
#ifdef EFI_S3_RESUME
  UINT16                        BitMask;
  UINT16                        BitValue;

  BitMask  = (UINT16) B_HDA_ICS_ICB;
  BitValue = (UINT16) 0;
  S3BootScriptSaveMemPoll (
    S3BootScriptWidthUint16,
    HdaBar + R_HDA_ICS,
    &BitMask,
    &BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_IC),
    1,
    &CodecCommandData
    );

  BitMask  = (UINT16) 0xFFFF;
  BitValue = (UINT16) ((B_HDA_ICS_IRV | B_HDA_ICS_ICB));
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (HdaBar + R_HDA_ICS),
    &BitValue,  // Data to be ORed
    &BitMask  // Data to be ANDed
    );

  BitMask  = (UINT16) B_HDA_ICS_ICB;
  BitValue = (UINT16) 0;
  S3BootScriptSaveMemPoll (
    S3BootScriptWidthUint16,
    HdaBar + R_HDA_ICS,
    &BitMask,
    &BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );
#endif
  return EFI_SUCCESS;
}
#endif

/**
 Initialize the Intel High Definition Audio Codec(s) present in the system.
 For each codec, a predefined codec verb table should be programmed.
 The list contains 32-bit verbs to be sent to the corresponding codec.
 If it is not programmed, the codec uses the default verb table, which may or may not
 correspond to the platform jack information.

 @param [in]   PeiServices     Pointer's pointer to EFI_PEI_SERVICES
 @param [in]   AzaliaConfig    The Azalia Policy information.

 @retval EFI_SUCCESS             The function completed successfully
 @retval EFI_INVALID_PARAMETER   Provided VerbTableData is null
**/
EFI_STATUS
DetectAndInitializeAzalia (
  IN CONST EFI_PEI_SERVICES                   **PeiServices,
  IN AZALIA_POLICY                            *AzaliaConfig
  )
{
  EFI_STATUS                    Status;
//  UINT32                        Index;
//  UINT32                        VendorDeviceId;
//  UINT32                        RevisionId;
//  UINT8                         ByteReg;
  UINTN                         AzaliaBase;
//  UINT8                         AzaliaSDINo;
  UINT32                        HdaBar;
//  UINT32                        *VerbTable;
//  UINT32                        LoopTime;
//  PCH_AZALIA_VERB_TABLE_HEADER  *VerbHeaderTable;
//  EFI_PHYSICAL_ADDRESS          BaseAddressBarMem;
//  UINT8                         VerbTableNum;
//  PCH_AZALIA_CONFIG             *AzaliaConfig;
  UINT8                         Data8;
  UINT32                        Data32And;
  UINT32                        Data32Or;
//  UINT32                        CodecCmdData;
//  UINT16                        Data16;
  UINT16                        Data16And;
  UINT16                        Data16Or;
//  UINT16                        BitMask;
//  UINT16                        BitValue;
  UINT16                        SdinWake;
  UINT16                        WakeEnable;
  PEI_VERB_TABLE_PPI            *VerbTablePpi;

  VerbTablePpi = NULL;
  SdinWake     = 0;
  WakeEnable   = 0;

//  AzaliaConfig = PchPlatformPolicy->AzaliaConfig;
  AzaliaBase = MmPciAddress (0,
                 0,
                 PCI_DEVICE_NUMBER_PCH_AZALIA,
                 PCI_FUNCTION_NUMBER_PCH_AZALIA,
                 0
                 );

  ///
  /// CHV BIOS Spec, Section 14.4.1 High Definition Audio VC1 Configuration
  ///
  /// Step 1
  /// Set the VT to VC mapping for VC0
  /// RCBA + 0x24 (HDA V0CTL - HDAudio Virtual Channel 0 Resource Control) = 0x80000019
  ///
  MmioWrite32 (RootComplexBar + R_PCH_RCRB_HDA_V0CTL, 0x80000019);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V0CTL),
    1,
    (VOID *) (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V0CTL)
    );
  ///
  /// Step 2
  /// Assign a TC ID for VC1
  /// RCBA + 0x28 (HDA V1CTL - HDAudio Virtual Channel 1 Resource Control) = 0x81000022
  ///
  MmioWrite32 (RootComplexBar + R_PCH_RCRB_HDA_V1CTL,  0x81000022);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V1CTL),
    1,
    (VOID *) (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V1CTL)
    );

  if (AzaliaConfig->AzaliaVCi == TRUE) {
    ///
    /// Step 3
    /// Set VCi Enable bit (VCIEN) of VCi Resource Control register
    /// D27:F0:Reg 0x120 [31]
    ///
    MmioWrite32 (AzaliaBase + R_PCH_HDA_VCICTL,  0x81000022);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL)
      );
    ///
    /// Step 4
    /// Enable No Snoop traffic
    /// D27:F0:Reg 0x78[11] (DEVC.NSNPEN - Enable No Snoop) = 1
    ///
    Data16And = (UINT16) (~B_PCH_HDA_DEVC_NSNPEN);
    Data16Or  = (UINT16) (B_PCH_HDA_DEVC_NSNPEN);
    MmioAndThenOr16 (
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      Data16And,
      Data16Or
      );
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      &Data16Or,  // Data to be ORed
      &Data16And  // Data to be ANDed
      );

  } else {
    ///
    /// Step 3
    /// Clear VCi Enable bit (VCIEN) of VCi Resource Control register
    /// D27:F0:Reg 0x120 (VCi Resource Control) = 0x1000022
    ///
    MmioWrite32 (AzaliaBase + R_PCH_HDA_VCICTL,  0x1000022);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL)
      );
    ///
    /// Step 4
    /// Disable No Snoop traffic
    /// D27:F0:Reg 0x78[11] (DEVC.NSNPEN - Enable No Snoop) = 0
    ///
    Data16And = (UINT16) (~B_PCH_HDA_DEVC_NSNPEN);
    Data16Or  = (UINT16) 0x00;
    MmioAndThenOr16 (
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      Data16And,
      Data16Or
      );
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      &Data16Or,  // Data to be ORed
      &Data16And  // Data to be ANDed
      );
  }
  ///
  /// CHV BIOS Spec, Section 14.5
  /// HDMI Codec Enabling
  /// System BIOS is required to perform the steps listed
  /// below in order to detect Codec on SoC HDMI channel
  /// 1. Set D27:F0:C4h[1] = 1b
  /// 2. Set D27:F0:43h[6] = 1b
  ///
  Data8 = MmioRead8 (AzaliaBase + R_PCH_HDA_TM1);
  if (AzaliaConfig->HdmiCodec == PCH_DEVICE_ENABLE) {
    MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM2), (UINT32) B_PCH_HDA_SEM2_IACE);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_SEM2),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_SEM2)
      );
    Data8 |= (UINT8) (B_PCH_HDA_TM1_ACCD);
  } else {
    MmioAnd32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM2), (UINT32) ~B_PCH_HDA_SEM2_IACE);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_SEM2),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_SEM2)
      );
  }
  ///
  /// CHV BIOS Spec, Section 14.5
  /// System BIOS is required set D27:F0:43h[3] = 1b
  ///
  Data8 |= (UINT8) (B_PCH_HDA_TM1_HAPD);
  MmioWrite8 ((UINTN) (AzaliaBase + R_PCH_HDA_TM1), Data8);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint8,
    (UINTN) (AzaliaBase + R_PCH_HDA_TM1),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_TM1)
    );

#if 0
  ///
  /// Firstly Initialize Azalia to be not started.
  ///
  *AzaliaStarted = FALSE;

  ///
  /// Allocate resource for HDBAR
  ///
#ifndef FSP_FLAG
  BaseAddressBarMem = 0x0FFFFFFFF;
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  14,
                  V_PCH_HDA_HDBAR_SIZE,
                  &BaseAddressBarMem,
                  mImageHandle,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
#else
  BaseAddressBarMem = MmioRead32 (AzaliaBase + R_PCH_HDA_HDBARL) & 0xFFFFFFF0; 
#endif
#endif
  ///
  /// System BIOS should ensure that the High Definition Audio HDBAR D27:F0:Reg 10-17h contains a valid address value
  /// and is enabled by setting D27:F0:Reg 04h[1].
  ///
  HdaBar = PcdGet32 (PcdPchHdaBaseAddress);
  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARL, HdaBar);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL)
    );

  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARU, 0);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU)
    );

  MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_STSCMD), (UINT32) B_PCH_HDA_STSCMD_MSE);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD)
    );

  ///
  /// CHV BIOS Spec, Section 14.6
  /// Additional High Definition Audio Programming Steps
  /// BIOS is required to perform the additional steps listed below before controller is out of reset.
  /// Step 1 and 2
  /// Set D27:F0:C4h[13] (B_PCH_HDA_SEM2_LSE) = 1b
  /// Set D27:F0:C4h[10] (B_PCH_HDA_SEM2_MQRDAD) = 1b
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = B_PCH_HDA_SEM2_LSE | B_PCH_HDA_SEM2_MQRDAD;
  MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM2), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_SEM2),
    &Data32Or,  // Data to be ORed
    &Data32And  // Data to be ANDed
    );
  ///
  /// B0:D27:F0 + 0xC4 [27:26] = 2'b10
  /// B0:D27:F0 + 0xC8 [12:7] = 0x00
  ///
  Data32And = (UINT32) ~B_PCH_HDA_SEM2_BSMT;
  Data32Or  = (UINT32) BIT27;
  MmioAndThenOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM2), Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_SEM2),
    &Data32Or,  // Data to be ORed
    &Data32And  // Data to be ANDed
    );
  Data32And = (UINT32) ~B_PCH_HDA_SEM3_LOW_ISRWS;
  Data32Or  = (UINT32) 0x00;
  MmioAndThenOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM3_LOW), Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_SEM3_LOW),
    &Data32Or,  // Data to be ORed
    &Data32And  // Data to be ANDed
    );

  if (AzaliaConfig->DS == PCH_DEVICE_DISABLE) {
    MmioAnd8 ((UINTN) (AzaliaBase + R_PCH_HDA_DCKSTS), (UINT8) (~B_PCH_HDA_DCKSTS_DS));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (AzaliaBase + R_PCH_HDA_DCKSTS),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_DCKSTS)
      );
  } else if (AzaliaConfig->DA != PCH_DEVICE_DISABLE) {
    if ((MmioRead8 (AzaliaBase + R_PCH_HDA_DCKSTS) & B_PCH_HDA_DCKSTS_DM) == 0) {
      MmioOr8 ((UINTN) (AzaliaBase + R_PCH_HDA_DCKCTL), (UINT8) (B_PCH_HDA_DCKCTL_DA));
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (AzaliaBase + R_PCH_HDA_DCKCTL),
        1,
        (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_DCKCTL)
        );
    }
  }

#if 0
  ///
  /// CHV BIOS Spec, Section 14.1.3 Codec Initialization Programming Sequence
  /// System BIOS should also ensure that the Controller Reset# bit of Global Control register
  /// in memory-mapped space (HDBAR+08h[0]) is set to 1 and read back as 1.
  /// Deassert the HDA controller RESET# to start up the link
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = (UINT32) (B_HDA_GCTL_CRST);
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    &Data32Or,  // Data to be ORed
    &Data32And  // Data to be ANDed
    );

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = (UINT16) B_HDA_GCTL_CRST;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  S3BootScriptSaveMemPoll (
    S3BootScriptWidthUint16,
    HdaBar + R_HDA_GCTL,
    &BitMask,
    &BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );
  ///
  /// CHV BIOS Spec, Section 14.1.3 Codec Initialization Programming Sequence
  /// Read GCAP and write the same value back to the register once after Controller Reset# bit is set
  ///
  Data16  = MmioRead16 (HdaBar + R_HDA_GCAP);
  MmioWrite16 (HdaBar + R_HDA_GCAP, Data16);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (HdaBar + R_HDA_GCAP),
    1,
    (VOID *) (UINTN) (HdaBar + R_HDA_GCAP)
    );

  ///
  /// Clear the "State Change Status Register" STATESTS bits for
  /// each of the "SDIN Stat Change Status Flag"
  ///
  MmioOr8 ((UINTN) (HdaBar + R_HDA_STATESTS), (UINT8) (AZALIA_MAX_SID_MASK));
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint8,
    (UINTN) (HdaBar + R_HDA_STATESTS),
    1,
    (VOID *) (UINTN) (HdaBar + R_HDA_STATESTS)
    );

  ///
  /// Turn off the link and poll RESET# bit until it reads back as 0 to get hardware reset report
  ///
  Data32And = (UINT32) (~B_HDA_GCTL_CRST);
  Data32Or  = (UINT32) 0;
  MmioAnd32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32And);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    &Data32Or,  // Data to be ORed
    &Data32And  // Data to be ANDed
    );

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = 0;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  S3BootScriptSaveMemPoll (
    S3BootScriptWidthUint16,
    HdaBar + R_HDA_GCTL,
    &BitMask,
    &BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Reset High Definition Audio (Azalia) Codec Time Out - 1! \n"));
    goto ExitInitAzalia;
  }
  ///
  /// Turn on the link and poll RESET# bit until it reads back as 1
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = (UINT32) (B_HDA_GCTL_CRST);
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    &Data32Or,  // Data to be ORed
    &Data32And  // Data to be ANDed
    );
  ///
  /// For some combo card that will need this delay because each codec has different latency to come out from RESET.
  /// This delay can make sure all codecs be recognized by BIOS after RESET sequence.
  /// Additional delay might be required to allow codec coming out of reset prior to subsequent operations,
  /// please contact your codec vendor for detail. When clearing this bit and setting it afterward,
  /// BIOS must ensure that minimum link timing requirements (minimum RESET# assertion time, etc.) are met..
  ///
  PchPmTimerStall (AzaliaConfig->ResetWaitTimer);
  S3BootScriptSaveStall (AzaliaConfig->ResetWaitTimer);

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = (UINT16) B_HDA_GCTL_CRST;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  S3BootScriptSaveMemPoll (
    S3BootScriptWidthUint16,
    HdaBar + R_HDA_GCTL,
    &BitMask,
    &BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Reset High Definition Audio (Azalia) Codec Time Out - 2! \n"));
    goto ExitInitAzalia;
  }
  ///
  /// Read the "State Change Status Register" STATESTS bits twice to find out if any SDIN is connected
  /// to a codec.
  ///
  for (LoopTime = 0, ByteReg = 0, AzaliaSDINo = 0; LoopTime < AZALIA_MAX_LOOP_TIME; LoopTime++) {
    ByteReg = (UINT8) (MmioRead8 (HdaBar + R_HDA_STATESTS) & AZALIA_MAX_SID_MASK);
    if (ByteReg != 0 && (ByteReg == AzaliaSDINo)) {
      break;
    } else {
      AzaliaSDINo = ByteReg;
    }

    PchPmTimerStall (AZALIA_WAIT_PERIOD);
  }
  ///
  /// BIT3(1000) -- SDI3
  /// BIT2(0100) -- SDI2
  /// BIT1(0010) -- SDI1
  /// BIT0(0001) -- SDI0
  ///
  if (ByteReg == 0) {
    ///
    /// No Azalia Detected
    ///
    ///
    /// Turn off the link
    ///
    DEBUG ((EFI_D_ERROR, "No Azalia device is detected.\n"));
    Data32And = (UINT32) (~B_HDA_GCTL_CRST);
    Data32Or  = (UINT32) 0;
    MmioAnd32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32And);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (HdaBar + R_HDA_GCTL),
      &Data32Or,  // Data to be ORed
      &Data32And  // Data to be ANDed
      );
    Status = EFI_DEVICE_ERROR;
    goto ExitInitAzalia;
  }
  ///
  /// PME Enable for Audio controller, this bit is in the resume well
  ///
  if (AzaliaConfig->Pme == PCH_DEVICE_ENABLE) {
    MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_PCS), (UINT32) (B_PCH_HDA_PCS_PMEE));
  }

  for (AzaliaSDINo = 0; AzaliaSDINo < AZALIA_MAX_SID_NUMBER; AzaliaSDINo++, ByteReg >>= 1) {
    if ((ByteReg & 0x1) == 0) {
      ///
      /// SDIx has no Azalia Device
      ///
      DEBUG ((EFI_D_ERROR, "SDI%d has no Azalia device.\n", AzaliaSDINo));
      continue;
    }
    ///
    /// PME Enable for each existing codec, these bits are in the resume well
    ///
    if (AzaliaConfig->Pme != PCH_DEVICE_DISABLE) {
      MmioOr16 (
        (UINTN) (HdaBar + R_HDA_WAKEEN),
        (UINT16) ((B_HDA_WAKEEN_SDI_0 << AzaliaSDINo))
        );
    }
    ///
    /// Verb:  31~28   27  26~20                   19~0
    ///         CAd    1    NID   Verb Command and data
    ///       0/1/2
    ///
    /// Read the Vendor ID/Device ID pair from the attached codec
    ///
    VendorDeviceId  = 0x000F0000 | (AzaliaSDINo << 28);
    Status          = SendCodecCommand (HdaBar, &VendorDeviceId, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Read the Codec Vendor ID/Device ID fail! \n"));
      goto ExitInitAzalia;
    }
    ///
    /// Read the Revision ID from the attached codec
    ///
    RevisionId  = 0x000F0002 | (AzaliaSDINo << 28);
    Status      = SendCodecCommand (HdaBar, &RevisionId, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Read the Codec Revision ID fail! \n"));
      goto ExitInitAzalia;
    }

    RevisionId = (RevisionId >> 8) & 0xFF;

    ///
    /// Get the match codec verb table, RevID of 0xFF applies to all steppings.
    ///
    for (VerbTableNum = 0, VerbHeaderTable = NULL, VerbTable = NULL;
         VerbTableNum < AzaliaConfig->AzaliaVerbTableNum;
         VerbTableNum++) {
      if ((VendorDeviceId == AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader.VendorDeviceId) &&
          ((AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader.RevisionId == 0xFF) ||
          ( RevisionId == AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader.RevisionId))) {
        VerbHeaderTable = &(AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader);
        VerbTable       = AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableData;
        if (VerbTable == 0) {
            DEBUG ((EFI_D_ERROR | EFI_D_INFO, "VerbTableData of VendorID:0x%X is null.\n", VendorDeviceId));
            Status = EFI_INVALID_PARAMETER;
            goto ExitInitAzalia;
        }
        DEBUG ((EFI_D_INFO, "Detected Azalia Codec with verb table, VendorID = 0x%X", VendorDeviceId));
        DEBUG ((EFI_D_INFO, " on SDI%d, revision = 0x%0x.\n", AzaliaSDINo, RevisionId));
        ///
        /// Send the entire list of verbs in the matching verb table one by one to the codec
        ///
        for (Index = 0;
             Index < (UINT32) ((VerbHeaderTable->NumberOfFrontJacks + VerbHeaderTable->NumberOfRearJacks) * 4);
             Index++) {
          ///
          /// Clear CAd Field
          ///
          CodecCmdData  = VerbTable[Index] & (UINT32) ~(BIT31 | BIT30 | BIT29 | BIT28);
          ///
          /// Program CAd Field per the SDI number got during codec detection
          ///
          CodecCmdData  |= (UINT32) (AzaliaSDINo << 28);
          Status        = SendCodecCommand (HdaBar, &CodecCmdData, FALSE);
          if (EFI_ERROR (Status)) {
            ///
            /// Skip the Azalia verb table loading when find the verb table content is not
            /// properly matched with the HDA hardware, though IDs match.
            ///
            DEBUG (
              (EFI_D_ERROR | EFI_D_INFO,
              "Detected Azalia Codec of VendorID:0x%X, error occurs during loading verb table.\n",
              VendorDeviceId)
              );
            goto ExitInitAzalia;
          }
          SendCodecCommandS3Item (HdaBar, CodecCmdData);
        }
        break;
      }
    }

    if (VerbTableNum >= AzaliaConfig->AzaliaVerbTableNum) {
      DEBUG (
        (EFI_D_ERROR,
        "Detected High Definition Audio (Azalia) Codec, VendorID = 0x%08x on SDI%d,",
        VendorDeviceId,
        AzaliaSDINo)
        );
      DEBUG ((EFI_D_ERROR, " but no matching verb table found.\n"));
    }
  }
  ///
  /// end of for
  ///
  *AzaliaStarted  = TRUE;
  Status          = EFI_SUCCESS;
#endif

  //
  // To program codec by verb table installation.
  //
  Status = (**PeiServices).LocatePpi (
                              PeiServices, 
                              &gPeiInstallVerbTablePpiGuid,
                              0,
                              NULL,
                              (VOID **)&VerbTablePpi
                              );

  if (EFI_ERROR (Status)) {
    goto ExitInitAzalia;
  }
  //
  // Force HDA controller Initialization
  //
  MmioAnd32 (HdaBar + R_HDA_GCTL, (UINT32)(~B_HDA_GCTL_CRST));

  Status = VerbTablePpi->InstallVerbTable ((EFI_PEI_SERVICES **)PeiServices, HdaBar, NULL);  
  if (EFI_ERROR (Status)) {
    goto ExitInitAzalia;
  }

  SdinWake = MmioRead16(HdaBar + R_HDA_STATESTS) & 0x7FFF;

  if ((SdinWake != 0) && (AzaliaConfig->Pme == PCH_DEVICE_ENABLE)){
    //
    // To set SDIN Wake Enable Flags when a codec be found.
    //
    MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_PCS), (UINT32) (B_PCH_HDA_PCS_PMEE));
    WakeEnable = SdinWake;
    MmioOr16(HdaBar + R_HDA_WAKEEN, WakeEnable);
  }

ExitInitAzalia:
  ///
  /// Clear AZBAR and disable memory map access
  ///
  MmioAnd32 ((UINTN) (AzaliaBase + R_PCH_HDA_STSCMD), (UINT32) (~B_PCH_HDA_STSCMD_MSE));
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD)
    );

  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARL, 0);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL)
    );

  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARU, 0);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU)
    );

#if 0
  gDS->FreeMemorySpace (
        BaseAddressBarMem,
        V_PCH_HDA_HDBAR_SIZE
        );
#endif

  return Status;
}

/**
  Initialize the Intel High Definition Audio Codec(s) in PEI phase.

  @param[in] FfsHeader            Pointer to EFI_FFS_FILE_HEADER
  @param[in] PeiServices          Pointer's pointer to EFI_PEI_SERVICES

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
AzaliaInitPeimEntry (
  IN EFI_PEI_FILE_HANDLE                   FileHandle,
  IN CONST EFI_PEI_SERVICES                **PeiServices
  )
{
  UINTN                       VariableSize;
  EFI_STATUS                  Status;
  AZALIA_POLICY               *AzaliaPolicyPpi;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *Variable;
  CHIPSET_CONFIGURATION        *SystemConfiguration;

  AzaliaPolicyPpi = NULL;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **) &Variable
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Read variable PPI can not be located !\n"));
    return EFI_ABORTED;
  }

  VariableSize = PcdGet32 (PcdSetupConfigSize);
  SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VariableSize);
  ASSERT (SystemConfiguration != NULL);
  
  Status = Variable->GetVariable (
                     Variable,
                     L"Setup",
                     &gSystemConfigurationGuid,
                     NULL,
                     &VariableSize,
                     SystemConfiguration
                     );
  ASSERT_EFI_ERROR (Status);
  if (SystemConfiguration->ScHdAudio == PCH_DEVICE_DISABLE) {
    FreePool (SystemConfiguration);
    return EFI_SUCCESS;
  }
  
  FreePool (SystemConfiguration);
  Status = PeiServicesLocatePpi (
                   &gAzaliaPolicyPpiGuid,
                   0,
                   NULL,
                   (VOID **)&AzaliaPolicyPpi
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Azalia policy PPI can not be located !\n"));
    return EFI_ABORTED;
  }

  Status = DetectAndInitializeAzalia (PeiServices, AzaliaPolicyPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Azalia detection / initialization failure!\n"));
    return EFI_ABORTED;
  }

  DEBUG ((EFI_D_INFO, "Azalia Initialize End.\n"));

  return EFI_SUCCESS;
}

#if 0
/**
  Detect and initialize the type of codec (AC'97 and HDA) present in the system.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in,out] AzaliaEnable    Returned with TRUE if Azalia High Definition Audio codec
                                  is detected and initialized.

  @retval EFI_SUCCESS            Codec is detected and initialized.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources to initialize the codec.
**/
EFI_STATUS
ConfigureAzalia (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  BOOLEAN                             *AzaliaEnable
  )
{
  EFI_STATUS  Status;
  UINTN       AzaliaBase;

  DEBUG ((EFI_D_INFO, "ConfigureAzalia() Start\n"));

  *AzaliaEnable = FALSE;
  AzaliaBase    = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_AZALIA,
                    PCI_FUNCTION_NUMBER_PCH_AZALIA,
                    0
                  );
  ///
  /// Skip Azalia initialization and codec detection if
  /// Azalia is disabled by fuse or soft strap
  ///
  if (MmioRead32 ((UINTN) AzaliaBase) == 0xFFFFFFFF) {
    PchPlatformPolicy->DeviceEnabling->Azalia = PCH_DEVICE_DISABLE;
    return EFI_SUCCESS;
  }
  ///
  /// CHV BIOS Spec, Section 14.8 Disabling High Definition Audio
  /// If HDAudio is enabled by fuse and soft-strap but still need to be
  /// function disabled, System BIOS must do the following:
  ///  1. Move HDAudio to RTD3hot by program PCS.PS (D27:F0:0x54 [1:0]) to 2'b11.
  ///  2. Set function disable bit for HDAudio at PBASE + 0x34 [12].
  ///
  if (PchPlatformPolicy->DeviceEnabling->Azalia == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Skip Azalia Codec detection.\n"));
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting Azalia Controller into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_PCS), V_PCH_HDA_PCS_PS3);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_PCS),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_PCS)
      );
    return EFI_SUCCESS;
  }

  Status = DetectAndInitializeAzalia (PchPlatformPolicy, RootComplexBar, AzaliaEnable);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Azalia detection / initialization failure!\n"));

    if (PchPlatformPolicy->DeviceEnabling->Azalia == PCH_DEVICE_ENABLE) {
      *AzaliaEnable = TRUE;
    }
  }

  DEBUG ((EFI_D_INFO, "ConfigureAzalia() End\n"));
  return EFI_SUCCESS;
}

#endif
