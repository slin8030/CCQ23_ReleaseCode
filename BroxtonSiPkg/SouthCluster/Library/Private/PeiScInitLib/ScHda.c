/** @file
  Initializes the HD-Audio Controller and Codec.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#include "ScInitPei.h"
#include <SaRegs.h>
#include <ScAccess.h>
#include <Library/PcdLib.h>
#include <Library/SteppingLib.h>
#include <Ppi/SaPolicy.h>
#include <Library/ConfigBlockLib.h>

#define HDA_MAX_LOOP_TIME    10
#define HDA_WAIT_PERIOD      100
#define HDA_MAX_SID_NUMBER   4
#define HDA_MAX_SID_MASK     ((1 << HDA_MAX_SID_NUMBER) - 1)
#define HDA_SDI_0_HDALINK    0
#define HDA_SDI_1_HDALINK    1
#define HDA_SDI_2_IDISPLINK  2


/**
  Polling the Status bit

  @param[in] StatusReg            The register address to read the status
  @param[in] PollingBitMap        The bit mapping for polling
  @param[in] PollingData          The Data for polling

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             Polling the bit map time out
**/
EFI_STATUS
StatusPolling (
  IN      UINT32          StatusReg,
  IN      UINT32          PollingBitMap,
  IN      UINT32          PollingData
  )
{
  UINT32  LoopTime;

  for (LoopTime = 0; LoopTime < HDA_MAX_LOOP_TIME; LoopTime++) {
    if ((MmioRead16 (StatusReg) & PollingBitMap) == PollingData) {
      break;
    } else {
      ScPmTimerStall (HDA_WAIT_PERIOD);
    }
  }

  if (LoopTime >= HDA_MAX_LOOP_TIME) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Send the command to the codec via the Immediate Command mechanism is written
  to the IC register

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in, out] CodecCommandData      The Codec Command to be sent to the codec
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
    DEBUG ((DEBUG_ERROR, "ICB bit is not zero before SendCodecCommand! \n"));
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
      DEBUG ((DEBUG_ERROR, "SendCodecCommand: ReadBack fail! \n"));
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}

/**
  Checks if connected codec supports statically switchable BCLK clock frequency.

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecSDINo                 SDI number to which codec is connected
  @param[out] BclkFrequencySupportMask  Bit mask of supported frequencies

  @retval EFI_NO_RESPONSE               Verb response has not been received from codec
  @retval EFI_UNSUPPORTED               Codec does not support frequency switching
  @retval EFI_SUCCESS                   Codec supports frequency switching, the function completed successfully
**/
EFI_STATUS
GetCodecFrequencySwitchingSupport (
  IN      UINT32                             HdaBar,
  IN      UINT8                              CodecSDINo,
  OUT     UINT32                             *BclkFrequencySupportMask
  )
{
  UINT32          BclkFreqSupport;
  EFI_STATUS      Status;

  *BclkFrequencySupportMask = 0;

  ///
  /// Read Statically Switchable BCLK Frequency capabilities from the attached codec (VerbId = F00h, ParameterId = 16h)
  ///
  BclkFreqSupport = 0x000F0016 | (CodecSDINo << 28);
  Status   = SendCodecCommand (HdaBar, &BclkFreqSupport, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Reading the Codec BCLK Frequency Switching support fail!\n"));
    return EFI_NO_RESPONSE;

  }
  DEBUG ((DEBUG_INFO, "SDI:%d Verb F00h ParamtererId = 16h response = 0x%08x\n", CodecSDINo, BclkFreqSupport));

  ///
  /// If the codec returns value of all 0s, it implies that it does not support static frequency switching.
  /// HDAudio-Link (SDIN 0/1) : BIT2 (24MHz) must be always set if supported
  /// iDisp-Link:  (SDIN 2)   : BIT4 (96MHz) must be always set if supported
  ///
  if (BclkFreqSupport == 0 ||
      ((CodecSDINo == HDA_SDI_0_HDALINK || CodecSDINo == HDA_SDI_1_HDALINK) && ((BclkFreqSupport & BIT2) == 0)) ||
      ((CodecSDINo == HDA_SDI_2_IDISPLINK) && ((BclkFreqSupport & BIT4) == 0)))
  {
    DEBUG ((DEBUG_INFO, "HDA: Codec Static Frequency Switching not supported!\n"));
    return EFI_UNSUPPORTED;
  }

  *BclkFrequencySupportMask = (BclkFreqSupport & 0x1F); // 1Fh = BITS[4:0] - 96/48/24/12/6 MHz
  DEBUG ((DEBUG_INFO, "HDA: Codec Static Frequency Switching supported - FrequencySupportMask = 0x%08x\n", *BclkFrequencySupportMask));
  return EFI_SUCCESS;
}

/**
  Get codec's current BCLK clock frequency.

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecSDINo                 SDI number to which codec is connected
  @param[out] CurrentBclkFrequency      Current BCLK frequency of codec

  @retval EFI_NO_RESPONSE               Verb response has not been received from codec
  @retval EFI_NOT_FOUND                 Could not match current codec frequency with any supported frequency
  @retval EFI_SUCCESS                   Supported frequency found, the function completed successfully
**/
EFI_STATUS
GetCodecCurrentBclkFrequency (
  IN      UINT32                              HdaBar,
  IN      UINT8                               CodecSDINo,
  OUT     SC_HDAUDIO_LINK_FREQUENCY          *CurrentBclkFrequency
  )
{
  UINT32          BclkFreq;
  UINT32          FreqIndex;
  EFI_STATUS      Status;

  DEBUG ((DEBUG_INFO, "GetCodecCurrentBclkFrequency() Start\n"));

  *CurrentBclkFrequency = ScHdaLinkFreqInvalid;
  ///
  /// Read BCLK Freq from the attached codec
  ///
  BclkFreq = 0x000F3700 | (CodecSDINo << 28);
  Status   = SendCodecCommand (HdaBar, &BclkFreq, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Reading the Codec current BCLK Frequency fail!\n"));
    return EFI_NO_RESPONSE;
  }
  DEBUG ((DEBUG_INFO, "SDI:%d Verb F37h (GET_CCF) response = 0x%08x\n", CodecSDINo, BclkFreq));

  for (FreqIndex = 0, Status = EFI_NOT_FOUND; FreqIndex < ScHdaLinkFreqInvalid; FreqIndex++) {
    if ((BclkFreq >> FreqIndex) & BIT0) {
      *CurrentBclkFrequency = FreqIndex;
      DEBUG ((DEBUG_INFO, "SDI:%d Codec BCLK frequency = %d\n", CodecSDINo, *CurrentBclkFrequency));
      Status = EFI_SUCCESS;
      break;
    }
  }

  DEBUG ((DEBUG_INFO, "GetCodecCurrentBclkFrequency() End, Status = %r\n", Status));
  return Status;
}

/**
  ConfigureIDispAudioFrequency: Configures iDisplay Audio BCLK frequency and T-Mode

  @param[in] RequestedBclkFrequency     IDisplay Link clock frequency to be set
  @param[in] RequestedTmode             IDisplay Link T-Mode to be set

  @retval EFI_NOT_FOUND                 SA Policy PPI or GT config block not found, cannot initialize GttMmAdr
  @retval EFI_UNSUPPORTED               iDisp link unsupported frequency
  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
ConfigureIDispAudioFrequency (
  IN       SC_HDAUDIO_LINK_FREQUENCY   RequestedBclkFrequency,
  IN       SC_HDAUDIO_IDISP_TMODE      RequestedTmode
  )
{
  UINTN               McD2BaseAddress;
  SI_SA_POLICY_PPI    *SiSaPolicyPpi;
  GRAPHICS_CONFIG     *GtConfig;
  UINTN               GttMmAdr;
  UINT32              Data32And;
  UINT32              Data32Or;
  BOOLEAN             IsGttBarInitialyAssigned;
  BOOLEAN             IsGttBarInitialyEnabled;
  EFI_STATUS          Status;

  DEBUG ((DEBUG_INFO, "ConfigureIDispAudioFrequency() Start\n"));
  McD2BaseAddress = MmPciBase (SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0);
  IsGttBarInitialyAssigned = FALSE;
  IsGttBarInitialyEnabled  = FALSE;

  if (MmioRead16 (McD2BaseAddress + R_SA_IGD_VID) == 0xFFFF) {
    DEBUG ((DEBUG_INFO, "iGFX not enabled - frequency switching for iDisplay link not supported - Exit!\n"));
    return EFI_UNSUPPORTED;
  }

  // Check if GttMmAdr has been already assigned, initialize if not
  GttMmAdr = (MmioRead32 (McD2BaseAddress + R_SA_IGD_GTTMMADR)) & 0xFFFFFFF0;
  if (GttMmAdr == 0) {
    ///
    /// Get SA Policy settings through the SaInitConfigBlock PPI
    ///
    Status = PeiServicesLocatePpi (
               &gSiSaPolicyPpiGuid,
               0,
               NULL,
               (VOID **) &SiSaPolicyPpi
               );
    if (EFI_ERROR (Status) || (SiSaPolicyPpi == NULL)) {
      DEBUG ((DEBUG_ERROR, "SaPolicy PPI not found - Exit!\n"));
      return EFI_NOT_FOUND;
    }

    Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gGraphicsConfigGuid, (VOID *)&GtConfig);
    ASSERT_EFI_ERROR (Status);

    GttMmAdr = GtConfig->GttMmAdr;
    if (GttMmAdr == 0) {
      DEBUG ((DEBUG_ERROR, "Temporary GttMmAdr Bar is not initialized - Exit!\n"));
      return EFI_NOT_FOUND;
    }

    ///
    /// Program and read back GTT Memory Mapped BAR
    ///
    MmioWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, (UINT32)(GttMmAdr & 0xFF000000));
    GttMmAdr = (MmioRead32 (McD2BaseAddress + R_SA_IGD_GTTMMADR)) & 0xFFFFFFF0;
  } else {
    DEBUG ((DEBUG_INFO, "GttMmAdr Bar already initialized\n"));
    IsGttBarInitialyAssigned = TRUE;
    IsGttBarInitialyEnabled  = (MmioRead16 (McD2BaseAddress + R_SA_IGD_CMD) & (BIT2 | BIT1)) ? TRUE : FALSE;
  }

  switch (RequestedBclkFrequency) {
    case ScHdaLinkFreq96MHz:
      // SA IGD: GttMmAdr + 0x65900[15] =  0b, [4:3] = 10b (2T Mode, 96MHz)
      Data32And = (UINT32)~(B_SA_IGD_AUD_FREQ_CNTRL_TMODE | B_SA_IGD_AUD_FREQ_CNTRL_48MHZ);
      Data32Or  = (UINT32) B_SA_IGD_AUD_FREQ_CNTRL_96MHZ;
      break;
    case ScHdaLinkFreq48MHz:
      // SA IGD: GttMmAdr + 0x65900[4:3] = 01b (48MHz)
      Data32And = (UINT32)~(B_SA_IGD_AUD_FREQ_CNTRL_96MHZ);
      Data32Or  = (UINT32) B_SA_IGD_AUD_FREQ_CNTRL_48MHZ;

      if (RequestedTmode == ScHdaIDispMode2T) {
        // SA IGD: 2T Mode [15] = 0b
        Data32And &= (UINT32)~(B_SA_IGD_AUD_FREQ_CNTRL_TMODE);
      } else {
        // SA IGD: 1T Mode [15] = 1b
        Data32Or |= (UINT32) B_SA_IGD_AUD_FREQ_CNTRL_TMODE;
      }
      break;
    default:
      DEBUG ((DEBUG_ERROR, "SA iGFX: Unsupported iDisplay Audio link frequency - Exit!\n"));
      return EFI_UNSUPPORTED;
  }

  if (!IsGttBarInitialyEnabled) {
    ///
    /// Enable Bus Master and Memory access on 0:2:0
    ///
    MmioOr16 (McD2BaseAddress + R_SA_IGD_CMD, (BIT2 | BIT1));
  }

  ///
  /// Program iDisplay Audio link frequency
  ///
  MmioAndThenOr32 ((UINTN)(GttMmAdr + R_SA_IGD_AUD_FREQ_CNTRL_OFFSET), Data32And, Data32Or);

  if (!IsGttBarInitialyEnabled) {
    ///
    /// Disable Bus Master and Memory access on 0:2:0 and clear GTTMMADR
    ///
    MmioAnd16 (McD2BaseAddress + R_SA_IGD_CMD, (UINT16) ~(BIT2 | BIT1));
    if (!IsGttBarInitialyAssigned) {
      MmioAnd32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, 0x0);
    }
  }

  DEBUG ((DEBUG_INFO, "ConfigureIDispAudioFrequency() End\n"));
  return EFI_SUCCESS;
}

/**
  Sets link clock frequency.
  Controller must be out of reset to set Output/Input payload registers.
  Link must be turned off before setting Link Control register, then turned on again.

  @param[in] HdaPciBase                 PCI Configuration Space Base Address
  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] LinkTypeIndex              Link index: 0 = HDA link, 1 = iDisp link
  @param[in] RequestedBclkFrequency     Clock frequency to be set

  @retval EFI_NOT_READY                 Link reset cannot be asserted
  @retval EFI_DEVICE_ERROR              iDisplay link frequency switching failed
  @retval EFI_INVALID_PARAMETER         Incorrect link index given
  @retval EFI_SUCCESS                   Requested frequency set, the function completed successfully
**/
EFI_STATUS
SetCodecBclkFrequency (
  IN      UINTN                               HdaPciBase,
  IN      UINT32                              HdaBar,
  IN      UINT8                               LinkTypeIndex,
  IN      SC_HDAUDIO_LINK_FREQUENCY           RequestedBclkFrequency,
  IN      SC_HDAUDIO_IDISP_TMODE              RequestedIDispTmode
  )
{
  UINT16      OutputPayloadWords;
  UINT16      InputPayloadWords;
  UINT32      Data32;
  EFI_STATUS  Status;

  // Assert link RESET# before frequency switching
  MmioAnd32 ((UINTN)(HdaBar + R_HDABA_LCTLX(LinkTypeIndex)), (UINT32)~(B_HDABA_LCTLX_SPA));
  Status = StatusPolling (HdaBar + R_HDABA_LCTLX(LinkTypeIndex), (UINT32) B_HDABA_LCTLX_CPA, (UINT32) 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HDA: Turn off the link (SPA = 0) failed! CPA polling Status = %r\n", Status));
    return EFI_NOT_READY;
  }

  if (LinkTypeIndex == V_HDA_HDALINK_INDEX) {
     DEBUG ((DEBUG_INFO, "HDA: HDA-Link Static BCLK Frequency Switch ID = %d - ", RequestedBclkFrequency));

     // For HDA-Link change OUTPAY and INPAY
    switch (RequestedBclkFrequency) {
      case ScHdaLinkFreq24MHz:
        DEBUG ((DEBUG_INFO, "24MHz\n"));
        // Output: 24MHz -> 1000 bits/frame - 40 bits (command & control) = 960 bits -> 60 words of payload
        // Input : 24MHz -> 500 bits/frame - 36 bits (response) = 464 bits -> 29 words of payload (limit to 28)
        OutputPayloadWords = 0x3C; // 60 words of payload
        InputPayloadWords  = V_HDA_INPAY_DEFAULT; // 28 words of payload
        break;
      case ScHdaLinkFreq12MHz:
        DEBUG ((DEBUG_INFO, "12MHz\n"));
        // Output: 12MHz -> 500 bits/frame - 40 bits (command & control) = 460 bits -> 28 words of payload
        // Input : 12MHz -> 250 bits/frame - 36 bits (response) = 214 bits -> 13 words of payload
        OutputPayloadWords = 0x1C; // 28 words of payload
        InputPayloadWords  = 0x0D; // 13 words of payload
        break;
      case ScHdaLinkFreq6MHz:
        DEBUG ((DEBUG_INFO, "6MHz\n"));
        // Output: 6MHz -> 250 bits/frame - 40 bits (command & control) = 210 bits -> 13 words of payload
        // Input : 6MHz -> 125 bits/frame - 36 bits (response) = 89 bits -> 5 words of payload
        OutputPayloadWords = 0x0D; // 13 words of payload
        InputPayloadWords  = 0x05; // 5 words of payload
        break;
      default:
        DEBUG ((DEBUG_INFO, "Default 24MHz\n"));
        // Default for 24MHz
        OutputPayloadWords = 0x3C; // 60 words of payload
        InputPayloadWords  = V_HDA_INPAY_DEFAULT; // 28 words of payload
        break;
    }

    // Controller must be out of reset (CRSTB = 1) to set the following registers
    MmioWrite16 ((UINTN) (HdaBar + R_HDA_OUTPAY), OutputPayloadWords);
    MmioWrite16 ((UINTN) (HdaBar + R_HDA_INPAY), InputPayloadWords);

    Data32 = MmioRead16 ((UINTN) (HdaBar + R_HDA_OUTPAY));
    DEBUG ((DEBUG_INFO, "HDA: OUTPAY (Output Payload Capability) = 0x%04x\n", Data32));
    Data32 = MmioRead16 ((UINTN) (HdaBar + R_HDA_INPAY));
    DEBUG ((DEBUG_INFO, "HDA: INPAY (Input Payload Capability) = 0x%04x\n", Data32));

  } else if (LinkTypeIndex == V_HDA_IDISPLINK_INDEX) {
    DEBUG ((DEBUG_INFO, "HDA: iDisplay-Link Static BCLK Frequency Switch ID = %d - ", RequestedBclkFrequency));
    DEBUG ((DEBUG_INFO, (RequestedBclkFrequency == ScHdaLinkFreq96MHz) ? "96MHz " : "48MHz "));
    DEBUG ((DEBUG_INFO, (RequestedIDispTmode    == ScHdaIDispMode2T)   ? "(2T Mode)\n" : "(1T Mode)\n"));

    // Configure iDisplay link frequency on System Agent counterpart (iGFX)
    Status = ConfigureIDispAudioFrequency (RequestedBclkFrequency, RequestedIDispTmode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "HDA SA: iDisplay-Link frequency switching failed! Status = %r\n", Status));
      return EFI_DEVICE_ERROR;
    }

    // For iDisp-Link set T-Mode
    switch (RequestedBclkFrequency) {
      case ScHdaLinkFreq96MHz:
        // PCH HDA: PCI Config Space + C0h[12] = 0b.
        MmioAnd32 ((UINTN) (HdaPciBase + R_HDA_SEM1), (UINT32)~(B_HDA_SEM1_TMODE));
        break;
      case ScHdaLinkFreq48MHz:
        if (RequestedIDispTmode == ScHdaIDispMode2T) {
          // PCH HDA: PCI Config Space + C0h[12] = 0b.
          MmioAnd32 ((UINTN) (HdaPciBase + R_HDA_SEM1), (UINT32)~(B_HDA_SEM1_TMODE));
        } else {
          // PCH HDA: PCI Config Space + C0h[12] = 1b.
          MmioOr32 ((UINTN) (HdaPciBase + R_HDA_SEM1), (UINT32) B_HDA_SEM1_TMODE);
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "HDA: Unsupported iDisplay Audio link frequency - Exit!\n"));
        return EFI_UNSUPPORTED;
    }
  } else {
    DEBUG ((DEBUG_ERROR, "HDA: Incorrect Link Type!\n"));
    return EFI_INVALID_PARAMETER;
  }

  MmioAndThenOr32 ((UINTN)(HdaBar + R_HDABA_LCTLX(LinkTypeIndex)),
                      (UINT32)~(BIT3 | BIT2 | BIT1 | BIT0),
                      (UINT32)(RequestedBclkFrequency << N_HDABA_LCTLX_SCF));

  Data32 = MmioRead32 ((UINTN)(HdaBar + R_HDABA_LCTLX(LinkTypeIndex)));
  DEBUG ((DEBUG_INFO, "HDA: LCTLx (Link %d Control) = 0x%08x\n", LinkTypeIndex, Data32));

  // De-assert link RESET# after frequency switching
  MmioOr32 ((UINTN)(HdaBar + R_HDABA_LCTLX(LinkTypeIndex)), (UINT32) B_HDABA_LCTLX_SPA);
  Status = StatusPolling (HdaBar + R_HDABA_LCTLX(LinkTypeIndex), (UINT32) B_HDABA_LCTLX_CPA, (UINT32) B_HDABA_LCTLX_CPA);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HDA: Turn on the link (SPA = 1) failed! CPA polling Status = %r\n", Status));
  }

  return EFI_SUCCESS;
}

/**
  Configures static link frequency for HD-Audio link and iDisplay link.

  @param[in] HdaConfig                  HD-A Configuration
  @param[in] HdaPciBase                 PCI Configuration Space Base Address
  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecSDINo                 SDI number to which codec is connected

  @retval EFI_ABORTED                   S3 boot - frequency cannot be switched
  @retval EFI_NOT_STARTED               iDisplay link frequency switch not supported (iGfx disabled)
  @retval EFI_INVALID_PARAMETER         Incorrect SDI number given
  @retval EFI_UNSUPPORTED               Codec does not support frequency switching
  @retval EFI_NOT_FOUND                 Codec current frequency does not match with any supported frequency
  @retval EFI_DEVICE_ERROR              Codec supports requested frequency, but the switching failed (default frequency set)
  @retval EFI_SUCCESS                   Requested frequency successfully set (or correct frequency is already configured)
**/
EFI_STATUS
ConfigureLinkFrequency (
  IN      SC_HDAUDIO_CONFIG                  *HdaConfig,
  IN      UINTN                               HdaPciBase,
  IN      UINT32                              HdaBar,
  IN      UINT8                               CodecSDINo
  )
{
  EFI_BOOT_MODE              BootMode;
  SC_HDAUDIO_LINK_FREQUENCY  RequestedFreq;
  SC_HDAUDIO_LINK_FREQUENCY  CurrentFreq;
  UINT32                     FreqSupportMask;
  UINT8                      LinkTypeIndex;
  EFI_STATUS                 Status;

  DEBUG ((DEBUG_INFO, "ConfigureLinkFrequency() Start [SDI:%d]\n", CodecSDINo));

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  if (BootMode == BOOT_ON_S3_RESUME) {
    DEBUG ((DEBUG_INFO, "HDA: S3 Resume Boot - do not switch codec's current BCLK frequency!\n"));
    return EFI_ABORTED;
  }

  switch (CodecSDINo) {
    case HDA_SDI_0_HDALINK:
    case HDA_SDI_1_HDALINK:
      // HDAudio link frequency change
      DEBUG ((DEBUG_INFO, "HDA: HDAudio-Link frequency configuration\n"));
      LinkTypeIndex = V_HDA_HDALINK_INDEX;
      RequestedFreq = HdaConfig->HdAudioLinkFrequency;
      // Program INPAY register with value 1Ch instead of the default value of 1Dh (controller must be out of reset, CRSTB = 1)
      MmioWrite16 ((UINTN) (HdaBar + R_HDA_INPAY), V_HDA_INPAY_DEFAULT);
      break;
    case HDA_SDI_2_IDISPLINK:
      // iDisplay link frequency change
      if (MmioRead16 (MmPciBase (SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0) + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
        DEBUG ((DEBUG_INFO, "HDA: iGFX not enabled - frequency switch for iDisplay link not supported!\n"));
        return EFI_NOT_STARTED;
      }

      DEBUG ((DEBUG_INFO, "HDA iDisplay-Link frequency configuration\n", CodecSDINo));
      LinkTypeIndex = V_HDA_IDISPLINK_INDEX;
      RequestedFreq = HdaConfig->IDispLinkFrequency;
      break;
    default:
      DEBUG ((DEBUG_ERROR, "HDA: Invalid SDI number for frequency configuration!\n"));
      return EFI_INVALID_PARAMETER;
  }

  Status = GetCodecFrequencySwitchingSupport (HdaBar, CodecSDINo, &FreqSupportMask);
  if (EFI_ERROR (Status)){
    return Status;
  }

  Status = GetCodecCurrentBclkFrequency (HdaBar, CodecSDINo, &CurrentFreq);
  if (EFI_ERROR (Status)){
    return Status;
  }

  if (CurrentFreq == RequestedFreq) {
    DEBUG ((DEBUG_INFO, "HDA: Requested BCLK clock frequency matches current frequency (ID = %d) - Exit\n", CurrentFreq));
    return EFI_SUCCESS;
  }

  if ((FreqSupportMask >> RequestedFreq) & BIT0) {
    DEBUG ((DEBUG_INFO, "HDA: Requested Frequency ID = %d - Supported!\n", RequestedFreq));
    Status = SetCodecBclkFrequency (HdaPciBase, HdaBar, LinkTypeIndex, RequestedFreq, HdaConfig->IDispLinkTmode);
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }

    // Delay needed after switching frequency (link reset), before sending verb to codec.
    MicroSecondDelay (HdaConfig->ResetWaitTimer);

    Status = GetCodecCurrentBclkFrequency (HdaBar, CodecSDINo, &CurrentFreq);
    if ((Status == EFI_NOT_FOUND) || (CurrentFreq != RequestedFreq)) {
      DEBUG ((DEBUG_ERROR, "HDA: Frequency switching failed - return to default!\n"));
      RequestedFreq = (LinkTypeIndex == V_HDA_IDISPLINK_INDEX) ? ScHdaLinkFreq96MHz : ScHdaLinkFreq24MHz;
      SetCodecBclkFrequency (HdaPciBase, HdaBar, LinkTypeIndex, RequestedFreq, ScHdaIDispMode2T);
      return EFI_DEVICE_ERROR;
    }
  }

  DEBUG ((DEBUG_INFO, "ConfigureLinkFrequency() End\n"));
  return EFI_SUCCESS;
}

/**
  Locates HD Audio codec verb table.

  @param[in] HdaConfig        HD-A Configuration
  @param[in] VendorDeviceId   Codec vendor ID  and device ID
  @param[in] RevisionId       Codec revision ID
  @param[in] SdiNo            SDI channel number

  @return HDA_VERB_TABLE_PPI pointer
  @retval NULL                Table not found.
**/
HDAUDIO_VERB_TABLE*
LocateVerbTable (
  IN      SC_HDAUDIO_CONFIG       *HdaConfig,
  IN      UINT32                  VendorDeviceId,
  IN      UINT8                   RevisionId,
  IN      UINT8                   SdiNo
  )
{
  HDAUDIO_VERB_TABLE         *VerbTable;
  UINTN                      Num;

  ASSERT (VendorDeviceId != 0);

  VerbTable = NULL;

  for (Num = 0; Num < HdaConfig->VerbTableEntryNum; ++Num) {
    VerbTable = ((HDAUDIO_VERB_TABLE **) (HdaConfig->VerbTablePtr)) [Num];
    if ((VerbTable != NULL) &&
        (VerbTable->VerbTableHeader.VendorDeviceId == VendorDeviceId) &&
        ((VerbTable->VerbTableHeader.RevisionId == 0xFF) || (VerbTable->VerbTableHeader.RevisionId == RevisionId)) &&
        ((VerbTable->VerbTableHeader.SdiNo == 0xFF) || VerbTable->VerbTableHeader.SdiNo == SdiNo)) {
      //
      // DEBUG print verb table
      //
//[-start-160111-IB08450335-modify]//
       UINT16        Index;
//[-end-160111-IB08450335-modify]//
      DEBUG ((DEBUG_INFO, "Num = %d\n", Num));

      DEBUG ((DEBUG_INFO,
           "HD-Audio installing verb table for VendorId = 0x%04X DevId = 0x%04X, "
           "revision = 0x%02X\n",
           VerbTable->VerbTableHeader.VendorDeviceId >> 16,
           (UINT16) (VerbTable->VerbTableHeader.VendorDeviceId),
           VerbTable->VerbTableHeader.RevisionId
           ));
      for (Index = 0; Index < VerbTable->VerbTableHeader.DataDwords; Index++) {
        DEBUG ((DEBUG_INFO, "VerbData[%d] = 0x%x\n", Index, VerbTable->VerbTableData[Index]));
      }

      break;
    }
  }
  if (Num >= HdaConfig->VerbTableEntryNum) {
    VerbTable = NULL;
  }

  return VerbTable;
}

/**
  Initialize the Intel High Definition Audio Codec(s) present in the system.
  For each codec, a predefined codec verb table should be programmed.
  The list contains 32-bit verbs to be sent to the corresponding codec.
  If it is not programmed, the codec uses the default verb table, which may or may not
  correspond to the platform jack information.

  @param[in] ScPolicy             The SC Policy protocol instance
  @param[in] HdaPciBase           PCI Configuration Space Base Address

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_INVALID_PARAMETER   Provided VerbTableData is null
**/
EFI_STATUS
DetectAndInitializeCodec (
  IN      SC_HDAUDIO_CONFIG     *HdaConfig,
  IN      UINTN                 HdaPciBase,
  IN      UINT32                HdaBar
  )
{
  EFI_STATUS                    Status;
  UINT32                        Index;
  UINT32                        VendorDeviceId;
  UINT32                        RevisionId;
  UINT8                         ByteReg;
  UINT8                         HdaSDINo;
  UINT32                        LoopTime;
  UINT32                        Data32And;
  UINT32                        Data32Or;
  UINT32                        CodecCmdData;
  UINT16                        Data16;
  UINT16                        BitMask;
  UINT16                        BitValue;
  HDAUDIO_VERB_TABLE            *VerbTable;

  DEBUG ((DEBUG_INFO, "DetectAndInitializeCodec() Start\n"));

  ///
  /// CHV BIOS Spec Rev 0.3.0 Section 14.1.3 Codec Initialization Programming Sequence
  /// System BIOS should also ensure that the Controller Reset# bit of Global Control register
  /// in memory-mapped space (HDBAR+08h[0]) is set to 1 and read back as 1.
  /// Deassert the HDA controller RESET# to start up the link
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = (UINT32) (B_HDA_GCTL_CRST);
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32Or);

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = (UINT16) B_HDA_GCTL_CRST;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);

  ///
  /// CHV BIOS Spec Rev 0.3.0 Section 14.1.3 Codec Initialization Programming Sequence
  /// Read GCAP and write the same value back to the register once after Controller Reset# bit is set
  ///
  Data16  = MmioRead16 (HdaBar + R_HDA_GCAP);
  MmioWrite16 (HdaBar + R_HDA_GCAP, Data16);

  ///
  /// Clear the "State Change Status Register" STATESTS bits for
  /// each of the "SDIN Stat Change Status Flag"
  ///
  MmioOr8 ((UINTN) (HdaBar + R_HDA_STATESTS), (UINT8) (HDA_MAX_SID_MASK));

  ///
  /// Turn off the link and poll RESET# bit until it reads back as 0 to get hardware reset report
  ///
  Data32And = (UINT32) (~B_HDA_GCTL_CRST);
  Data32Or  = (UINT32) 0;
  MmioAnd32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32And);

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = 0;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Reset HD-Audio Codec Time Out - 1! \n"));
    goto ExitInitCodec;
  }
  ///
  /// Turn on the link and poll RESET# bit until it reads back as 1
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = (UINT32) (B_HDA_GCTL_CRST);
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32Or);

  ///
  /// For some combo card that will need this delay because each codec has different latency to come out from RESET.
  /// This delay can make sure all codecs be recognized by BIOS after RESET sequence.
  /// Additional delay might be required to allow codec coming out of reset prior to subsequent operations,
  /// please contact your codec vendor for detail. When clearing this bit and setting it afterward,
  /// BIOS must ensure that minimum link timing requirements (minimum RESET# assertion time, etc.) are met..
  ///
  ScPmTimerStall (HdaConfig->ResetWaitTimer);

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = (UINT16) B_HDA_GCTL_CRST;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Reset HD-Audio Codec Time Out - 2! \n"));
    goto ExitInitCodec;
  }
  ///
  /// Read the "State Change Status Register" STATESTS bits twice to find out if any SDIN is connected
  /// to a codec.
  ///
  for (LoopTime = 0, ByteReg = 0, HdaSDINo = 0; LoopTime < HDA_MAX_LOOP_TIME; LoopTime++) {
    ByteReg = (UINT8) (MmioRead8 (HdaBar + R_HDA_STATESTS) & HDA_MAX_SID_MASK);
    if (ByteReg != 0 && (ByteReg == HdaSDINo)) {
      break;
    } else {
      HdaSDINo = ByteReg;
    }
    ScPmTimerStall (HDA_WAIT_PERIOD);
  }
  ///
  /// BIT3(1000) -- SDI3
  /// BIT2(0100) -- SDI2
  /// BIT1(0010) -- SDI1
  /// BIT0(0001) -- SDI0
  ///
  if (ByteReg == 0) {
    ///
    /// No codec Detected
    ///
    ///
    /// Turn off the link
    ///
    DEBUG ((DEBUG_ERROR, "HD-Audio Codec not detected (SDIN not connected to a codec)\n"));
    Data32And = (UINT32) (~B_HDA_GCTL_CRST);
    Data32Or  = (UINT32) 0;
    MmioAnd32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32And);
    Status = EFI_DEVICE_ERROR;
    goto ExitInitCodec;
  }
  ///
  /// PME Enable for Audio controller, this bit is in the resume well
  ///
  if (HdaConfig->Pme == TRUE) {
    MmioOr32 ((UINTN) (HdaPciBase + R_HDA_PCS), (UINT32) (B_HDA_PCS_PMEE));
  }

  for (HdaSDINo = 0; HdaSDINo < HDA_MAX_SID_NUMBER; HdaSDINo++, ByteReg >>= 1) {
    if ((ByteReg & 0x1) == 0) {
      ///
      /// SDIx has no HD-Audio Device
      ///
      DEBUG ((DEBUG_ERROR, "SDI%d has no HD-Audio device.\n", HdaSDINo));
      continue;
    }
    ///
    /// PME Enable for each existing codec, these bits are in the resume well
    ///
    if (HdaConfig->Pme == TRUE) {
      MmioOr16 (
        (UINTN) (HdaBar + R_HDA_WAKEEN),
        (UINT16) ((B_HDA_WAKEEN_SDI_0 << HdaSDINo))
        );
    }
    ///
    /// Verb:  31~28   27  26~20                   19~0
    ///         CAd    1    NID   Verb Command and data
    ///       0/1/2
    ///
    /// Read the Vendor ID/Device ID pair from the attached codec
    ///
    VendorDeviceId  = 0x000F0000 | (HdaSDINo << 28);
    DEBUG ((DEBUG_ERROR, "VendorDeviceId = %x\n", VendorDeviceId));

    Status          = SendCodecCommand (HdaBar, &VendorDeviceId, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Read the Codec Vendor ID/Device ID fail! \n"));
      goto ExitInitCodec;
    }
    ///
    /// Read the Revision ID from the attached codec
    ///
    RevisionId  = 0x000F0002 | (HdaSDINo << 28);
    DEBUG ((DEBUG_ERROR, "RevisionId = %x\n", RevisionId));
    Status      = SendCodecCommand (HdaBar, &RevisionId, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Read the Codec Revision ID fail! \n"));
      goto ExitInitCodec;
    }

    RevisionId = (RevisionId >> 8) & 0xFF;

    DEBUG ((DEBUG_INFO, "SDI:%d Detected HD-Audio Codec 0x%08X rev 0x%02X\n", HdaSDINo, VendorDeviceId, RevisionId));

    ///
    /// Locate Verb Table and initialize detected codec
    ///
    VerbTable = LocateVerbTable (HdaConfig, VendorDeviceId, (UINT8) RevisionId, HdaSDINo);
    if (VerbTable == NULL) {
        DEBUG ((DEBUG_ERROR | DEBUG_INFO, "VerbTableData of VendorID:0x%X is null.\n", VendorDeviceId));
        Status = EFI_INVALID_PARAMETER;
        goto ExitInitCodec;
    }
    DEBUG ((DEBUG_INFO, "Detected HD-Audio Codec with verb table, VendorID = 0x%X", VendorDeviceId));
    DEBUG ((DEBUG_INFO, " on SDI%d, revision = 0x%0x.\n", HdaSDINo, RevisionId));
    ///
    /// Send the entire list of verbs in the matching verb table one by one to the codec
    ///
    for (Index = 0; Index < VerbTable->VerbTableHeader.DataDwords; Index++) {
      ///
      /// Clear CAd Field
      ///
      CodecCmdData  = VerbTable->VerbTableData[Index] & (UINT32) ~(BIT31 | BIT30 | BIT29 | BIT28);
      ///
      /// Program CAd Field per the SDI number got during codec detection
      ///
      CodecCmdData  |= (UINT32) (HdaSDINo << 28);
      Status        = SendCodecCommand (HdaBar, &CodecCmdData, FALSE);
      if (EFI_ERROR (Status)) {
        ///
        /// Skip the HD-Audio verb table loading when find the verb table content is not
        /// properly matched with the HDA hardware, though IDs match.
        ///
        DEBUG (
          (DEBUG_ERROR | DEBUG_INFO,
          "Detected HD-Audio Codec of VendorID:0x%X, error occurs during loading verb table.\n",
          VendorDeviceId)
          );
        goto ExitInitCodec;
      }
    }
  }
  ///
  /// end of for
  ///
  Status = EFI_SUCCESS;

ExitInitCodec:
  DEBUG ((DEBUG_INFO, "DetectAndInitializeCodec() Exit, Status = %r\n", Status));
  return Status;
}

/**
  Initialize the Intel High Definition Audio Controller

  @param[in] SiPolicy             The SI Policy PPI instance
  @param[in] ScPolicy             The SC Policy PPI instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS             Codec is detected and initialized.
  @retval EFI_OUT_OF_RESOURCES    Failed to allocate resources to initialize the codec.
**/
EFI_STATUS
ConfigureHda (
  IN     SI_POLICY_PPI  *SiPolicy,
  IN     SC_POLICY_PPI  *ScPolicy,
  IN OUT UINT32         *FuncDisableReg
  )
{
  EFI_STATUS        Status;
  UINT32            HdaFncfg;
  UINTN             HdaPciBase;
  UINT32            HdaBar;
  SC_HDAUDIO_CONFIG *HdaConfig;
//[-start-170410-IB07400857-add]//
  BXT_STEPPING      Stepping;
//[-end-170410-IB07400857-add]//
  BXT_SERIES        Series;

  HdaBar = SiPolicy->TempMemBaseAddr;
  ///
  /// Detect and initialize the type of codec present in the system
  ///
  HdaPciBase = MmPciBase (
                 DEFAULT_PCI_BUS_NUMBER_SC,
                 PCI_DEVICE_NUMBER_HDA,
                 PCI_FUNCTION_NUMBER_HDA
                 );

  DEBUG ((DEBUG_INFO, "ConfigureHda() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gHdAudioConfigGuid, (VOID *) &HdaConfig);
  ASSERT_EFI_ERROR (Status);

  HdaFncfg = 0;
//[-start-170410-IB07400857-add]//
  Stepping = BxtStepping ();
//[-end-170410-IB07400857-add]//
  Series   = GetBxtSeries ();
  ///
  /// Skip HD-Audio initialization and codec detection if
  /// HD-Audio is disabled by fuse or soft strap
  ///
  if (MmioRead16 (HdaPciBase) == 0xFFFF) {
    DEBUG ((DEBUG_ERROR, "HD-Audio not detected, skipped!\n"));
    if (HdaConfig->Enable == FALSE) {
      *FuncDisableReg |= B_PMC_FUNC_DIS_AVS;
    }
    return EFI_SUCCESS;
  }


    SideBandAndThenOr32(
      HDA_PORT_ID,
      R_HDA_PTDC,
      (UINT32) (~B_HDS_PTDC_SRMIW),
      (UINT32) (V_HDA_PTDC_256XTAL_OSC_CLOCKS)
      );

    HdaFncfg = SideBandRead32(HDA_PORT_ID, R_HDA_FNCFG);


  ///
  /// Program HD-Audio Lower Base Address
  ///
  DEBUG ((DEBUG_INFO, "Program HD-Audio Lower Base Address.\n"));
  MmioWrite32 (HdaPciBase + R_HDA_HDBARL, HdaBar);

  ///
  /// Program HD-Audio Upper Base Address
  ///
  DEBUG ((DEBUG_INFO, "Program HD-Audio Upper Base Address.\n"));
  MmioWrite32 (HdaPciBase + R_HDA_HDBARU, 0);

  ///
  /// Program Status Command - Space Memory Enable bit
  ///
  MmioOr16 ((UINTN) (HdaPciBase + R_HDA_STSCMD), (UINT16) (B_HDA_STSCMD_MSE));
  DEBUG ((DEBUG_INFO, "HD-Audio Base Address: 0x%08X.\n", HdaBar));

  ///
  /// If HD-Audio is to be disabled, skip the codec detection code
  ///
  if (HdaConfig->Enable == FALSE) {
    ///
    /// Step1. Put device in D0i3 state.
    ///
    /// Set HDABA + 104Ah [2] = 1b
    ///
    DEBUG ((DEBUG_INFO, "Putting HD-Audio into D0i3 State.\n"));
    MmioOr8 ((UINTN) (HdaBar + R_HDA_D0I3C), (UINT8) B_HDA_D0I3C_D0I3);
    DEBUG ((DEBUG_INFO, "D0I3C register is: 0x%02X\n", MmioRead8((UINTN)(HdaBar + R_HDA_D0I3C))));

    ///
    /// Step2. Put device in D3hot state.
    ///
    /// Set PCI[HDA] + 54h [1,0] = 3b
    ///
    DEBUG ((DEBUG_INFO, "Putting HD-Audio into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (HdaPciBase + R_HDA_PCS), (UINT32) V_HDA_PCS_PS3);
    DEBUG ((DEBUG_INFO, "PCS register is: 0x%08X\n", MmioRead32((UINTN)(HdaPciBase + R_HDA_PCS))));

    /// Step3. Configure FNCFG register
    ///
    /// Set PCR[HDA] + 530h [5] = 0b
    ///
    DEBUG ((DEBUG_INFO, "Power Gating disabled.\n"));
    HdaFncfg &= (UINT32) ~(B_HDA_FNCFG_PGD);
    ///
    /// Set PCR[HDA] + 530h [2] = 1b
    ///
    DEBUG ((DEBUG_INFO, "ADSP Disabled.\n"));
    HdaFncfg |= (UINT32) (B_HDA_FNCFG_ADSPD);
    ///
    /// Set PCR[HDA] + 530h [0] = 1b
    ///
    DEBUG ((DEBUG_INFO, "HDA Disabled.\n"));
    HdaFncfg |= (UINT32) (B_HDA_FNCFG_HDASD);
    ///
    /// Set PCR[HDA] + 530h [4] = 1b
    ///
    DEBUG ((DEBUG_INFO, "BIOS Configuration Lock Down.\n"));
      SideBandAndThenOr32(
        HDA_PORT_ID,
        R_HDA_FNCFG,
        (UINT32) ~(B_HDA_FNCFG_MASK),
        (UINT32) (B_HDA_FNCFG_BCLD | HdaFncfg)
       );

      DEBUG ((DEBUG_INFO, "FNCFG register is: 0x%08X\n", SideBandRead32(HDA_PORT_ID, R_HDA_FNCFG)));
    ///
    /// Step4. Disable PCI function by PSF FunDis bit.
    /// Set PCR[PSF3] + PSF_3_AGNT_T0_SHDW_PCIEN_AUDIO_RS0_D14_F0[8] = 1b
    ///
    DEBUG ((DEBUG_INFO, "Disable PCI function.\n"));
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      R_PCH_PCR_PSF3_T0_SHDW_AUDIO_PCIEN,
      0xFFFFFFFF,
      (UINT32) B_PCH_PCR_PSF3_T0_SHDW_AUDIO_PCIEN_FUNDIS
      );
    DEBUG ((DEBUG_INFO, "PCIEN register is: 0x%08X\n", SideBandRead32(SB_PORTID_PSF3, R_PCH_PCR_PSF3_T0_SHDW_AUDIO_PCIEN)));

    ///
    /// Step5. Set function disable register
    /// Set PMC + 34h [31] = 1b
    *FuncDisableReg |= B_PMC_FUNC_DIS_AVS;
  } else {
    ///
    /// Program HD-Audio as PCIe device
    ///
    DEBUG ((DEBUG_INFO, "Program HD-Audio as PCIe device.\n"));
    HdaFncfg &= (UINT32) ~(B_HDA_FNCFG_HDASPCID);

    ///
    /// GPROCEN Programming
    /// Setting GPROCEN is the Audio OS driver responsibility (to enable ADSP for operation)
    ///
    /// GPROCEN = 0, ADSPxBA (BAR2) is mapped to the Intel HD Audio memory mapped configuration registers (Default),
    /// GPROCEN = 1, ADSPxBA (BAR2) is mapped to the actual Audio DSP memory mapped configuration registers.
    ///

    if(HdaConfig->DspEnable == TRUE) {
      if (HdaConfig->DspUaaCompliance == FALSE) {
        DEBUG ((DEBUG_INFO, "HDA: Audio DSP Enabled, SST mode - set Sub Class Code to 0x%02x\n", V_SC_HDA_SCC_ADSP));
        //
        // Set Sub Class Code to 01 for Audio DSP enabled (SST only support)
        //
        MmioWrite8 ((UINTN) (HdaPciBase + R_SC_HDA_SCC), (UINT8) V_SC_HDA_SCC_ADSP);
      } else {
        DEBUG ((DEBUG_INFO, "HDA: Audio DSP Enabled, UAA mode - set Programming Interface to 0x%02x\n", V_SC_HDA_PI_ADSP_UAA));
        //
        // Set Programming Interface to 01 for Audio DSP enabled (HDA Inbox and SST support)
        //
        MmioWrite8 ((UINTN) (HdaPciBase + R_SC_HDA_PI), (UINT8) V_SC_HDA_PI_ADSP_UAA);
      }
    } else {
      DEBUG ((DEBUG_INFO, "HDA: Audio DSP Disabled\n"));
      ///
      /// Set PCR[HDA] + 530h [2] = 1b.
      ///
      HdaFncfg |= (UINT32) (B_HDA_FNCFG_ADSPD);
    }

    DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM Before.\n"));
    DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM1 = 0x%x\n",MmioRead32(HdaPciBase + R_HDA_SEM1)));
    DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM2 = 0x%x\n",MmioRead32(HdaPciBase + R_HDA_SEM2)));
    DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM3L = 0x%x\n",MmioRead32(HdaPciBase + R_HDA_SEM3L)));
    DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM4L = 0x%x\n",MmioRead32(HdaPciBase + R_HDA_SEM4L)));


    ///
    /// Set EM1/SEM1 register (HDABAR + 1000h / HDA PCI Config Space + C0h):
    /// 1) Set LFLCS BIT[24] = 0b to support 6MHz HDA links on BXT SOCs
    /// 2) Set FIFORDYSEL BIT[10:9] = 11b
    ///
    MmioAndThenOr32 ((UINTN) (HdaPciBase + R_HDA_SEM1),
                      (UINT32)~(B_HDA_SEM1_LFLCS | B_HDA_SEM1_FIFORDYSEL),
                      (UINT32) B_HDA_SEM1_FIFORDYSEL);

     ///
     /// Set Input Stream L1 Exit Threshold 2 to 1/8 of FIFO size
     /// Set EM3L/SEM3L.ISL1EXT2 to 10b
     /// Set Output Stream L1 Exit Threshold 2 to 1/4 of FIFO size
     /// Set EM4L/SEM4L.OSL1EXT2 to 11b
     ///
     MmioAndThenOr32 ((UINTN) (HdaPciBase + R_HDA_SEM3L),
                      (UINT32)~B_HDA_SEM3L_ISL1EXT2,
                      (UINT32)(V_HDA_SEM3L_ISL1EXT2 << N_HDA_SEM3L_ISL1EXT2));

     MmioAndThenOr32 ((UINTN) (HdaPciBase + R_HDA_SEM4L),
                      (UINT32)~B_HDA_SEM4L_OSL1EXT2,
                      (UINT32)(V_HDA_SEM4L_OSL1EXT2 << N_HDA_SEM4L_OSL1EXT2));

     if ((Series != BxtSeriesMax) && (Series != BxtP)) {
       if (SiPolicy->OsSelection == SiAndroid) {
         //for AOS Audio D0i3
         MmioOr32 ((UINTN) (HdaPciBase + R_HDA_SEM4L), BIT20);
       }
     }

    switch (HdaConfig->IoBufferOwnership) {
      ///
      /// Set I/O Buffers Ownership to HDA-Link (HW default)
      ///
      case ScHdaIoBufOwnerHdaLink:
        DEBUG ((DEBUG_INFO, "HD-Audio link owns all the I/O buffers.\n"));
          SideBandAndThenOr32(
            HDA_PORT_ID,
            R_HDA_IOBCTL,
            (UINT32)~B_HDA_IOBCTL_OSEL,
            (UINT32) (V_HDA_IOBCTL_OSEL_HDALINK << N_HDA_IOBCTL_OSEL)
          );
        Status = DetectAndInitializeCodec (HdaConfig, HdaPciBase, HdaBar);
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_ERROR | DEBUG_INFO, "Codec detection / initialization failure!\n"));
        }
        break;
      ///
      ///  Set I/O Buffers Ownership to shared HDA-Link and I2S Port
      ///
      case ScHdaIoBufOwnerHdaLinkI2sPort:
        DEBUG ((DEBUG_INFO, "HD-Audio link owns 4 of the I/O buffers, I2S port owns 4 of the I/O buffers.\n"));
          SideBandAndThenOr32(
            HDA_PORT_ID,
            R_HDA_IOBCTL,
            (UINT32)~B_HDA_IOBCTL_OSEL,
            (UINT32) (V_HDA_IOBCTL_OSEL_HDALINK_I2S << N_HDA_IOBCTL_OSEL)
           );

        Status = DetectAndInitializeCodec (HdaConfig, HdaPciBase, HdaBar);
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_ERROR | DEBUG_INFO, "Codec detection / initialization failure!\n"));
        }
        break;
      ///
      /// Set I/O Buffers Ownership to I2S
      ///
      case ScHdaIoBufOwnerI2sPort:
        DEBUG ((DEBUG_INFO, "I2S0 and I2S1 ports own all the I/O buffers.\n"));
          SideBandAndThenOr32(
            HDA_PORT_ID,
            R_HDA_IOBCTL,
            (UINT32)~B_HDA_IOBCTL_OSEL,
            (UINT32) (V_HDA_IOBCTL_OSEL_I2S << N_HDA_IOBCTL_OSEL)
           );
        break;

      default:
        DEBUG ((DEBUG_ERROR, "HDA: Invalid I/O buffer ownership selection!\n"));
        Status = EFI_INVALID_PARAMETER;
    }

    if (HdaConfig->VcType == ScHdaVc1) {
      ///
      /// Enable Virtual Channel VC1
      /// Permit non snoop on VC0, PCI Config Space + C4h[24] = 1b.
      /// Enable VC1 traffic, PCR[HDA] + E4h[1] = 1b
      ///
      DEBUG ((DEBUG_INFO, "HDA: Isochronous Support on VC1\n"));
      MmioOr32 ((UINTN) (HdaPciBase + R_HDA_SEM2), (UINT32)B_HDA_SEM2_VC0PSNR);
      SideBandAndThenOr32(
        HDA_PORT_ID,
        R_HDA_TTCCFG,
        0xFFFFFFFF,
        (UINT32)(B_HDA_TTCCFG_MMT)
        );
    }

    if ((Series != BxtSeriesMax) && (Series != BxtP)) {
      if (SiPolicy->OsSelection == SiAndroid) {
        //for AOS Audio D0i3
        MmioOr32 ((UINTN) (HdaPciBase + R_HDA_SEM2), (BIT26|BIT24));
      }
    }

    if (SiPolicy->OsSelection != SiAndroid) {
      //
      // 128 bytes data buffer size (padded) for each DMA engine.
      //
      MmioAndThenOr32 (
        (UINTN) (HdaPciBase + R_HDA_SEM2),
        (UINT32)(~B_HDA_SEM2_BSMT),
        (UINT32)(V_HDA_SEM2_BSMT_128B << N_HDA_SEM2_BSMT)
        );
    }

//[-start-160828-IB07400775-add]//
    if ((Stepping == BxtPA0)) {
      DEBUG((DEBUG_INFO, "HDA: HP/LP LDOCFG A0 Workaround - Modify New Values for CFG1/2 HP/LP SRAMS.\n"));

        SideBandWrite32(HDA_PORT_ID, R_HDA_HPLDOCFG1, V_HDA_HPLDOCFG1);
        SideBandWrite32(HDA_PORT_ID, R_HDA_HPLDOCFG2, V_HDA_HPLDOCFG2);
        SideBandWrite32(HDA_PORT_ID, R_HDA_LPLDOCFG1, V_HDA_LPLDOCFG1);
        SideBandWrite32(HDA_PORT_ID, R_HDA_LPLDOCFG2, V_HDA_LPLDOCFG2);

        DEBUG((DEBUG_INFO, "HDA: HP/LP LDOCFG A0 Workaround - R_HDA_HPLDOCFG1 = 0x%x.\n", SideBandRead32(HDA_PORT_ID, R_HDA_HPLDOCFG1)));
        DEBUG((DEBUG_INFO, "HDA: HP/LP LDOCFG A0 Workaround - R_HDA_HPLDOCFG2 = 0x%x.\n", SideBandRead32(HDA_PORT_ID, R_HDA_HPLDOCFG2)));
        DEBUG((DEBUG_INFO, "HDA: HP/LP LDOCFG A0 Workaround - R_HDA_LPLDOCFG1 = 0x%x.\n", SideBandRead32(HDA_PORT_ID, R_HDA_LPLDOCFG1)));
        DEBUG((DEBUG_INFO, "HDA: HP/LP LDOCFG A0 Workaround - R_HDA_LPLDOCFG2 = 0x%x.\n\n", SideBandRead32(HDA_PORT_ID, R_HDA_LPLDOCFG2)));
    }
    if (Stepping == BxtPA0) {
      MmioOr32 ((UINTN)(HdaPciBase + R_HDA_PGCTL), (UINT32)(B_HDA_PGC_LSRMD));
    }
//[-end-160828-IB07400775-add]//

    //
    // Clear SROTCGE bit in PGCTL For Audio PG1 to Go Down In D3 Transition (RTL Issue)
    MmioAnd32((UINTN)(HdaPciBase + R_HDA_CGCTL), (~(UINT32)(B_HDA_CGCTL_SROTCGE)));

    ///
    /// Expose D0i3 Capability Structure
    ///
    DEBUG ((DEBUG_INFO, "Expose D0i3 Capability Structure.\n"));
    MmioAndThenOr16 ((UINTN) (HdaPciBase + R_HDA_PID), (UINT16) B_HDA_PID_CAP, (UINT16) (R_HDA_VSCID << N_HDA_PID_CAP));

    ///
    /// Set PCR[HDA] + E4h [4] = 1b
    ///
    if (HdaConfig->Mmt == ScHdaVc2) {
      DEBUG ((DEBUG_INFO, "Configure MMT as VC2.\n"));
        SideBandAndThenOr32(
          HDA_PORT_ID,
          R_HDA_TTCCFG,
          0xFFFFFFFF,
          (UINT32)(B_HDA_TTCCFG_MMT)
         );
    }

    ///
    /// Set PCR[HDA] + E4h [2] = 1b
    ///
    if (HdaConfig->Hmt == ScHdaVc2) {
      DEBUG ((DEBUG_INFO, "Configure HMT as VC2.\n"));
        SideBandAndThenOr32(
          HDA_PORT_ID,
          R_HDA_TTCCFG,
          0xFFFFFFFF,
          (UINT32)(B_HDA_TTCCFG_HMT)
          );
    }
//[-start-160828-IB07400775-add]//
    if ((Stepping == BxtPA0)) {
      ///
      /// Set PGCTL + 44h [1] = 1b
      ///
      DEBUG ((DEBUG_INFO, "Disable Audio Ctrl power gating.\n"));
      MmioOr32 ((UINTN) (HdaPciBase + R_HDA_PGCTL), (UINT32) B_HDA_PGC_CTLPGD);

      ///
      /// Set PGCTL + 44h [2] = 1b
      ///
      DEBUG ((DEBUG_INFO, "Disable Audio DSP power gating.\n"));
      MmioOr32 ((UINTN) (HdaPciBase + R_HDA_PGCTL), (UINT32) B_HDA_PGC_ADSPPGD);
    }

    //
    // Clear ADSPPGD bit in PGCTL for RTD3 and S0ix Enabling
    //
    // Setting the ADSPPGD bit (PGCTL + 44h) is needed as a W/A for WoV. However, this prevents Audio
    // from de-asserting its vnn_req which prevents RTD3 and S0ix entry.
    if ((Stepping == BxtPA0)) {
      ///
      /// Set PGCTL + 44h [2] = 0b
      ///
      DEBUG((DEBUG_INFO, "Disable Audio DSP power gating.\n"));
      MmioAnd32((UINTN)(HdaPciBase + R_HDA_PGCTL), (UINT32)~(B_HDA_PGC_ADSPPGD));
    }
//[-end-160828-IB07400775-add]//

    ///
    /// Enable HAE and Disable D3-Hot and D0i3
    /// Set 'HAE' and Clear 'D3HE' and 'I3E' in PCE, HDABAR + 104Bh[5] = 1b (104Bh[2][1] = 0b)
    ///
    MmioAndThenOr32 (
      (UINTN)(HdaBar + R_HDA_LTRC),
      (UINT32)~(B_HDA_PCE_D3HE | B_HDA_PCE_I3E),
      (UINT32)(B_HDA_PCE_HAE)
    );

    if ((Series != BxtSeriesMax) && (Series != BxtP)) {
      DEBUG ((DEBUG_INFO, "ConfigureHda() BXTM \n"));
      if (SiPolicy->OsSelection == SiAndroid) {
        //for AOS Audio D0i3
        MmioOr32 ((UINTN) (HdaBar + R_HDA_LTRC), (BIT7|BIT5|BIT3|BIT0));
        DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_LTRC = 0x%x\n", MmioRead32((UINTN)(HdaBar + R_HDA_LTRC))));
      }
    }

    if (SiPolicy->OsSelection != SiAndroid) {
      //
      // 0x29 ms that has to be decremented from the L1 exit threshold
      //
      MmioAndThenOr32 (
        (UINTN) (HdaBar + R_HDA_LTRC),
        (UINT32)(~B_HDA_PCE_GB),
        (UINT32)((BIT5|BIT3|BIT0) << N_HDA_PCE_GB)
        );
    }

    ///
    /// Set PCR[HDA] + 530h [3] = 1b
    ///
    if (HdaConfig->ClkGate == TRUE) {
      DEBUG ((DEBUG_INFO, "Enable HD-Audio Clock Gating.\n"));
      HdaFncfg &= (UINT32) ~(B_HDA_FNCFG_CGD);
    }

    ///
    /// Set PCR[HDA] + 530h [5] = 1b
    ///
    if (HdaConfig->PwrGate == TRUE) {
      DEBUG ((DEBUG_INFO, "Enable HD-Audio Power Gating.\n"));
      HdaFncfg &= (UINT32) ~(B_HDA_FNCFG_PGD);
    }

    DEBUG ((DEBUG_INFO, "BIOS configurations lock down.\n"));
    SideBandAndThenOr32(
      HDA_PORT_ID,
      R_HDA_FNCFG,
      (UINT32)~(B_HDA_FNCFG_MASK),
      (UINT32)(B_HDA_FNCFG_BCLD | HdaFncfg)
    );
  }

  ///
  /// Clear HDBAR and disable memory map access
  ///
  MmioAnd16 ((UINTN) (HdaPciBase + R_HDA_STSCMD), (UINT16) (~B_HDA_STSCMD_MSE));
  MmioWrite32 (HdaPciBase + R_HDA_HDBARL, 0);
  MmioWrite32 (HdaPciBase + R_HDA_HDBARU, 0);

  DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM After.\n"));
  DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM1 = 0x%x\n", MmioRead32(HdaPciBase + R_HDA_SEM1)));
  DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM2 = 0x%x\n", MmioRead32(HdaPciBase + R_HDA_SEM2)));
  DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM3L = 0x%x\n", MmioRead32(HdaPciBase + R_HDA_SEM3L)));
  DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM4L = 0x%x\n", MmioRead32(HdaPciBase + R_HDA_SEM4L)));
  DEBUG ((DEBUG_INFO, "ConfigureHda() R_HDA_SEM3L = 0x%x\n", MmioRead32(HdaPciBase + R_HDA_SEM3L)));

  DEBUG ((DEBUG_INFO, "ConfigureHda() End\n"));
  return Status;
}
