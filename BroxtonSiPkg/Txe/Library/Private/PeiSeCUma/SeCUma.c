/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2017 Intel Corporation.

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
/*++
Module Name:

  SeCUma.c

Abstract:

  Framework PEIM to SeCUma

--*/


#include <SeCUma.h>
#include <Library/PreSiliconLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SideBandLib.h>
#include <Private/Library/HeciInitLib.h>
#include <Library/HobLib.h>
#ifndef FSP_FLAG
#include <Library/PeiServicesLib.h>
#include <Library/PerformanceLib.h>
#include <Ppi/EndOfPeiPhase.h>
#endif
#include <Library/PcdLib.h>
#include <Library/MmPciLib.h>
#ifdef FSP_FLAG
#include <Library/FspCommonLib.h>
#include <FspmUpd.h>
#endif
//[-start-160216-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160216-IB07400702-add]//
//[-start-160804-IB07400769-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160804-IB07400769-add]//

extern EFI_GUID gEfiBootMediaHobGuid;
extern EFI_GUID gEfiIfwiDnxRequestHobGuid;
//[-start-160810-IB03090430-add]//
extern EFI_GUID gFdoModeEnabledHobGuid;
//[-end-160810-IB03090430-add]//

extern EFI_PEI_STALL_PPI  mStallPpi;
extern BOOLEAN ImageInMemory;

#define S3          0x20
#ifndef FSP_FLAG
/**
  Txe End of PEI callback function. This is the last event before entering DXE and OS in S3 resume.

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
EFIAPI
TxeOnEndOfPei (
  IN EFI_PEI_SERVICES                   **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR          *NotifyDesc,
  IN VOID                               *Ppi
  );

static EFI_PEI_NOTIFY_DESCRIPTOR  mTxeNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiEndOfPeiSignalPpiGuid,
    TxeOnEndOfPei
  }
};
#endif

  //
// Function Declarations
//
static SEC_UMA_PPI         mSeCUmaPpi = {
  SeCSendUmaSize,
  SeCConfigDidReg,
  SeCTakeOwnerShip
};

static EFI_PEI_PPI_DESCRIPTOR mSeCUmaPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gSeCUmaPpiGuid,
    &mSeCUmaPpi
  }
};
EFI_PEI_PPI_DESCRIPTOR mCseEmmcSelectPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gCseEmmcSelectPpiGuid,
    NULL
  }
};
EFI_PEI_PPI_DESCRIPTOR mCseUfsSelectPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gCseUfsSelectPpiGuid,
    NULL
  }
};
EFI_PEI_PPI_DESCRIPTOR mCseSpiSelectPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gCseSpiSelectPpiGuid,
    NULL
  }
};
/**

  This procedure will read and return the amount of SeC UMA requested
  by SeC ROM from the HECI device.


  @param PeiServices     - General purpose services available to every PEIM.

  @return Return SeC UMA Size

**/
UINT32
SeCSendUmaSize (
  IN EFI_PEI_SERVICES **PeiServices
  )
{
  UINT32                  Timeout;
//  UINTN                   HeciBaseAddress;
  HECI_MISC_SHDW_REGISTER SeCMiscShdw;
  HECI_FWS_REGISTER       SeCHfs;
  UINT32                  tempBuffer;
  UINTN                   SpiBase;
  UINTN                   SpiBAR0;
  UINT16                  Hsfs;

  EFI_HOB_GUID_TYPE       *FdoEnabledGuidHob = NULL;
//[-start-160804-IB07400769-add]//
  UINT8                   CmosData;
//[-end-160804-IB07400769-add]//
  
  SeCMiscShdw.ul = HeciPciRead32(R_SEC_MEM_REQ);
  SeCHfs.ul      = HeciPciRead32(R_SEC_FW_STS0);

  Timeout       = 0x0;
  //
  // ISH
  // BIOS will enable the PMCTL register.
  // Write to ISH MMIO offset 0x1D0 to set bits[5:0] to 0x3F
  //
  tempBuffer = SideBandRead32 (0x98, 0x1d0);
  tempBuffer = tempBuffer | 0x3F;
  SideBandWrite32 (0x98, 0x1d0, tempBuffer );

  DEBUG ((DEBUG_INFO, "ISH MMIO offset 0x1D0 to set bits[5:0] to 0x3F after write: %x\n", tempBuffer));
 
//[-start-160714-IB11270157-modify]//
  //
  // Enable memory mapping IO
  //
  MmioOr32 (
          MmPciAddress (
              0,
              DEFAULT_PCI_BUS_NUMBER_SC,
              PCI_DEVICE_NUMBER_SPI,
              PCI_FUNCTION_NUMBER_SPI,
              PCI_COMMAND_OFFSET
              ),
          EFI_PCI_COMMAND_MEMORY_SPACE
          );

  SpiBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI);
  SpiBAR0 = MmioRead32 (SpiBase + R_SPI_BASE) & B_SPI_BASE_BAR;

  DEBUG ((DEBUG_INFO, "Checking if the SPI flash descriptor security override pin-strap is set.\n"));

  Hsfs = MmioRead16 (SpiBAR0 + R_SPI_HSFS);
//[-start-160804-IB07400769-modify]//
  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2); 
  if (((Hsfs & B_SPI_HSFS_FDOPSS) == 0) ||
	  (SeCHfs.r.SeCOperationMode == SEC_OPERATION_MODE_SECOVR_JMPR) ||
	  (SeCHfs.r.SeCOperationMode == SEC_OPERATION_MODE_SECOVR_HECI_MSG) ||
	  (SeCHfs.r.CurrentState == SEC_STATE_RECOVERY)) {
    DEBUG ((DEBUG_INFO, "SPI FDO is set. This is an FDO boot.\n"));

    FdoEnabledGuidHob = BuildGuidHob (&gFdoModeEnabledHobGuid, 0);
    ASSERT (FdoEnabledGuidHob != NULL);
    PcdSetBool(PcdFDOState,1);
//[-start-161020-IB07400800-add]//
    PcdSetBool(PcdForceVolatileVariable,1);
//[-end-161020-IB07400800-add]//
    CmosData |= B_CMOS_TXE_DATA_UNAVAILABLE;
  } else {
    DEBUG ((DEBUG_INFO, "SPI FDO is not set.\n"));
    PcdSetBool(PcdFDOState,0);
    CmosData &= (~B_CMOS_TXE_DATA_UNAVAILABLE);
  }
  WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature2, CmosData); 
//[-end-160804-IB07400769-modify]//
  //
  // Disable memory mapping IO
  //
  MmioAnd32 (SpiBase + PCI_COMMAND_OFFSET, (UINT32)~EFI_PCI_COMMAND_MEMORY_SPACE);
//[-end-160714-IB11270157-modify]//

  DEBUG ((DEBUG_INFO, "HeciPciWrite32\n"));
  DEBUG ((DEBUG_INFO, "HeciPciRead32(R_SEC_DevID_VID)=%x\n", HeciPciRead32(R_SEC_DevID_VID)));
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR0)=%x\n", HeciPciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR1)=%x\n", HeciPciRead32(HECI_BAR1)));

  HeciPciWrite32 (HECI_BAR0, 0xFFFFFFFF);         // BAR0  Dev 15 Func 0    10h bits 31:0
  HeciPciWrite32 (HECI_BAR1, 0xFFFFFFFF);         // BAR1  Dev 15 Func 0    14h bits 31:0
  HeciPciWrite32 (HECI_BAR0, HECI_BASE_ADDRESS);  // BAR0  Dev 15 Func 0    10h bits 31:0
  HeciPciWrite32 (HECI_BAR1, 0); // BAR1  Dev 15 Func 0    14h bits 31:0
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR0)=%x\n", HeciPciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR1)=%x\n", HeciPciRead32(HECI_BAR1)));
  //
  // Memory space enable: Set to enable SeC memory mapped register space.
  // Bus Master Enable: Set to enable SeC bus master functionality.
  // SERR Enable: Set to enable SERR capability..
  // bit 8 for SERR enable
  DEBUG ((DEBUG_INFO, "Heci2PciWrite32\n"));
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(R_SEC_DevID_VID)=%x\n", Heci2PciRead32(R_SEC_DevID_VID)));
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR0)=%x\n", Heci2PciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR1)=%x\n", Heci2PciRead32(HECI_BAR1)));

  Heci2PciWrite32 (HECI_BAR0, 0xFFFFFFFF);         // BAR0  Dev 15 Func 1    10h bits 31:0
  Heci2PciWrite32 (HECI_BAR1, 0xFFFFFFFF);         // BAR1  Dev 15 Func 1    14h bits 31:0
  Heci2PciWrite32 (HECI_BAR0, HECI2_BASE_ADDRESS);  // BAR0  Dev 15 Func 1    10h bits 31:0
  Heci2PciWrite32 (HECI_BAR1, 0);                   // BAR1  Dev 15 Func 1    14h bits 31:0
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR0)=%x\n", Heci2PciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR1)=%x\n", Heci2PciRead32(HECI_BAR1)));


  Heci3PciWrite32 (HECI_BAR0, 0xFFFFFFFF);         // BAR0  Dev 15 Func 2    10h bits 31:0
  Heci3PciWrite32 (HECI_BAR1, 0xFFFFFFFF);         // BAR1  Dev 15 Func 2    14h bits 31:0
  Heci3PciWrite32 (HECI_BAR0, HECI3_BASE_ADDRESS); // BAR0  Dev 15 Func 2    10h bits 31:0
  Heci3PciWrite32 (HECI_BAR1, 0);                  // BAR1  Dev 15 Func 2    14h bits 31:0
  HeciPciWrite32 (R_COMMAND, 0x106);             // Enable BAR Dev 15 Func 0    04h bits 8, 2:1
  Heci2PciWrite32 (R_COMMAND, 0x106);             // Enable BAR Dev 15 Func 1    04h bits 8, 2:1
  Heci3PciWrite32 (R_COMMAND, 0x106);             // Enable BAR Dev 15 Func 2    04h bits 8, 2:1

  HeciPciWrite32 (HECI_BAR0, 0xFFFFFFFF);         // BAR0  Dev 15 Func 0    10h bits 31:0
  HeciPciWrite32 (HECI_BAR1, 0xFFFFFFFF);         // BAR1  Dev 15 Func 0    14h bits 31:0
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR0)=%x\n", HeciPciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR1)=%x\n", HeciPciRead32(HECI_BAR1)));

  HeciPciWrite32 (HECI_BAR0, HECI_BASE_ADDRESS);  // BAR0  Dev 15 Func 0    10h bits 31:0
  HeciPciWrite32 (HECI_BAR1, 0); // BAR1  Dev 15 Func 0    14h bits 31:0
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR0)=%x\n", HeciPciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "HeciPciRead32(HECI_BAR1)=%x\n", HeciPciRead32(HECI_BAR1)));

  Heci2PciWrite32 (HECI_BAR0, 0xFFFFFFFF);         // BAR0  Dev 15 Func 1    10h bits 31:0
  Heci2PciWrite32 (HECI_BAR1, 0xFFFFFFFF);         // BAR1  Dev 15 Func 1    14h bits 31:0
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR0)=%x\n", Heci2PciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR1)=%x\n", Heci2PciRead32(HECI_BAR1)));

  Heci2PciWrite32 (HECI_BAR0, HECI2_BASE_ADDRESS);  // BAR0  Dev 15 Func 1    10h bits 31:0
  Heci2PciWrite32 (HECI_BAR1, 0);                   // BAR1  Dev 15 Func 1    14h bits 31:0
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR0)=%x\n", Heci2PciRead32(HECI_BAR0)));
  DEBUG ((DEBUG_INFO, "Heci2PciRead32(HECI_BAR1)=%x\n", Heci2PciRead32(HECI_BAR1)));
  Heci3PciWrite32 (HECI_BAR0, 0xFFFFFFFF);       // BAR0  Dev 15 Func 2   10h bits 31:0
  Heci3PciWrite32 (HECI_BAR1, 0xFFFFFFFF);     // BAR1  Dev 15 Func 2    14h bits 31:0
  Heci3PciWrite32 (HECI_BAR0, HECI3_BASE_ADDRESS);  // BAR0  Dev 15 Func 2    10h bits 31:0
  Heci3PciWrite32 (HECI_BAR1, 0);                   // BAR1  Dev 15 Func 2   14h bits 31:0

  if (SeCHfs.r.SeCOperationMode == SEC_OPERATION_MODE_ALT_DISABLED) {
    DEBUG ((DEBUG_INFO, "SeC debug mode, do not check for SeC UMA. \n"));
    return EFI_SUCCESS;
  }

  if (SeCHfs.r.ErrorCode != 0) {
    DEBUG ((DEBUG_INFO, "SeC error, do not check for SeC UMA. \n"));
    return EFI_SUCCESS;
  }
  //
  // Poll on MUSZV until it indicates a valid size is present or 5s timeout expires.
  //
  while ((SeCMiscShdw.r.MUSZV == 0) && (Timeout < MUSZV_TIMEOUT_MULTIPLIER)) {
    if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
      MicroSecondDelay (STALL_1_MILLISECOND);
    }
    SeCMiscShdw.ul = HeciPciRead32(R_SEC_MEM_REQ);
    Timeout++;
  }

  //
  // Return MeUmaSize value
  //
  DEBUG ((DEBUG_INFO, "SEC UMA Size Requested: %x\n", SeCMiscShdw.r.MUSZ));

  return SeCMiscShdw.r.MUSZ;
}

/**
This procedure will enforce the BIOS Action that was requested by SEC FW
as part of the DRAM Init Done message.

@param[in] BiosAction           Me requests BIOS to act

@retval EFI_SUCCESS             Always return EFI_SUCCESS
**/
EFI_STATUS
HandleSecBiosAction (
  IN UINT8                        BiosAction
  )
{
  EFI_STATUS              Status;
  HECI_FWS_REGISTER       SeCFirmwareStatus;

  ///
  /// Read SEC FWSTS
  ///
  SeCFirmwareStatus.ul = HeciPciRead32(R_SEC_FW_STS0);
  DEBUG((DEBUG_INFO, "SecFwsts = %x.\n", SeCFirmwareStatus.ul));

  switch (BiosAction) {
  case 0:
    ///
    /// Case: DID ACK was not received
    ///
    DEBUG((DEBUG_ERROR, "DID Ack was not received, no BIOS Action to process.\n"));
    break;

  case CBM_DIR_NON_PCR:
    ///
    /// Case: Perform Non-Power Cycle Reset
    ///
    DEBUG((DEBUG_ERROR, "SEC FW has requested a Non-PCR.\n"));
    Status = PerformReset(CBM_DIR_NON_PCR);
    break;

  case CBM_DIR_PCR:
    ///
    /// Case: Perform Power Cycle Reset
    ///
    DEBUG((DEBUG_ERROR, "SEC FW has requested a PCR.\n"));
    Status = PerformReset(CBM_DIR_PCR);
    break;

  case 3:
    ///
    /// Case: Go To S3
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID ACK has requested entry to S3.  Not defined, continuing to POST.\n"));
    break;

  case 4:
    ///
    /// Case: Go To S4
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID ACK has requested entry to S4.  Not defined, continuing to POST.\n"));
    break;

  case 5:
    ///
    /// Case: Go To S5
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID ACK has requested entry to S5.  Not defined, continuing to POST.\n"));
    break;

  case CBM_DIR_GLOBAL_RESET:
    ///
    /// Case: Perform Global Reset
    ///
    DEBUG((DEBUG_ERROR, "SEC FW has requested a Global Reset.\n"));
    Status = PerformReset(CBM_DIR_GLOBAL_RESET);
    break;

  case CBM_DIR_CONTINUE_POST:
    ///
    /// Case: Continue to POST
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID Ack requested to continue to POST.\n"));
    break;
  }

  return EFI_SUCCESS;
}

/**
  Send Get proxy State message through HEC1 but not waiting for response.

  @param[in] Heci2Protocol The HECI protocol to send the message to HECI2 device.

  @return EFI_SUCCESS   Send get proxy state message success.
  @return Others              Send get proxy state message failed.
**/
EFI_STATUS
HeciGetProxyState(
void  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength;
  HECI2_BIOS_MESSAGE              *ReadFileMessage;
  UINT32                          SeCMode;
  UINT32                           DataBuffer[0x40];
  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  SetMem(DataBuffer, sizeof(DataBuffer), 0);
  ReadFileMessage = (HECI2_BIOS_MESSAGE*)DataBuffer;
  ReadFileMessage->header.cmd_id = HECI2_GET_PROXY_STATE_CMD_ID;
  HeciSendLength                        = sizeof(HECI2_BIOS_MESSAGE);
  HeciRecvLength                        = sizeof(DataBuffer);
  Status = HeciSendwACK(
                  HECI1_DEVICE,
                  DataBuffer,
                  HeciSendLength,
                  &HeciRecvLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI2_BIOS_MCA_FIXED_ADDR
             );
  return Status;
}
/**

  This procedure will configure the SEC Host General Status register,
  indicating that DRAM Initialization is complete and SeC FW may
  begin using the allocated SeC UMA space.


  @param PeiServices     - General purpose services available to every PEIM.
  @param MrcBootMode     - MRC BootMode
  @param InitStat        - H_GS[27:24] Status
  @param SeCUmaBase      - LSB of base address
                         SeCUmaBaseEx - MSB of base address
                         SeCUmaSIze -  Allocated size of UMA
  @param SeCUmaSize      EDES_TODO: Add parameter description

  @return EFI_SUCCESS

**/

EFI_STATUS
SeCConfigDidReg (
  IN CONST EFI_PEI_SERVICES **PeiServices,
  MRC_BOOT_MODE_T           MrcBootMode,
  UINT8                     InitStat,
  UINT32                    SeCUmaBase,
  UINT32                    *SeCUmaSize
  )
{
  EFI_STATUS     Status;
  UINT8          BiosAction;
  UINT8          IsS3 = 0;
#ifdef FSP_FLAG
  FSPM_UPD      *FspmUpd;
#endif

  BiosAction = 0;

  DEBUG ((DEBUG_INFO, "SeCConfigDidReg.\n"));

#ifdef FSP_FLAG
  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
  if (FspmUpd->FspmConfig.SkipCseRbp) {
    DEBUG ((DEBUG_INFO, "Skip CSE RBP by setting IRRBP & UCSTS in HOST_TO_CSE register...\n"));
    HeciPciWrite32(R_HOST_TO_CSE, B_IRRBP|B_UCSTS);
  }
#endif

  //Add SecureBoot flag around this MSR
  // if BXTM not have Securboot fuse enabled, then system will GP fault
  AsmWriteMsr64(0x139,1);
  if (S3 == MrcBootMode) {
    IsS3=TRUE;
  }
  Status = HeciSendDIDMessage (SeCUmaBase, IsS3, SeCUmaSize, &BiosAction);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Sending DID failed! Error: %r\n", Status));
  }
  if (Status == EFI_TIMEOUT) {
    DEBUG((DEBUG_ERROR, "Timeout occurred waiting for DID ACK.\n"));
  } else {
    DEBUG((DEBUG_INFO, "SEC DRAM Init Done ACK received.\n"));
  }
  Status = HandleSecBiosAction(BiosAction);

  return Status;
}

#ifdef FSP_FLAG
VOID FspCheckBootDevice(
  IN OUT INTN   *DeviceIndex,
  IN CONST EFI_PEI_SERVICES **PeiServices
  )
{
  MBP_IFWI_DNX_REQUEST    IfwiDnxRequest;
  MBP_CURRENT_BOOT_MEDIA  BootMediaData;
  EFI_STATUS Status;

  Status = HeciGetIfwiDnxRequest(&IfwiDnxRequest);
  if (EFI_ERROR(Status)) {
    SetMem(&IfwiDnxRequest, sizeof(MBP_IFWI_DNX_REQUEST), 0x0);
  }

  Status = HeciGetBootDevice(&BootMediaData);
  if (EFI_ERROR(Status)) {
    SetMem(&BootMediaData, sizeof(MBP_CURRENT_BOOT_MEDIA), 0xFF);
  }
  *DeviceIndex = BootMediaData.PhysicalData;

  //
  // Build HOB for BootMediaData
  //
  BuildGuidDataHob (
    &gEfiBootMediaHobGuid,
    &BootMediaData,
    sizeof (MBP_CURRENT_BOOT_MEDIA)
    );

  if ((*DeviceIndex) == 0) {
    HeciTakeOwnerShip();
  } else {
    Status = (*PeiServices)->InstallPpi(PeiServices, mCseSpiSelectPpiList);
  }
}
#endif

VOID CheckBootDevice(IN CONST EFI_PEI_SERVICES **PeiServices)
{

  INTN   DeviceIndex;
  EFI_STATUS Status;
  MBP_CURRENT_BOOT_MEDIA  BootMediaData;
  MBP_IFWI_DNX_REQUEST    IfwiDnxRequest;
//[-start-160614-IB07400744-add]//
  EFI_BOOT_MODE           BootMode;
//[-end-160614-IB07400744-add]//

//[-start-160216-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_CHECK_BOOT_DEVICE_START);
//[-end-160216-IB07400702-add]//

//[-start-160614-IB07400744-add]//
  Status = (*PeiServices)->GetBootMode (
    PeiServices,
    &BootMode
    );
  ASSERT_EFI_ERROR (Status);
//[-end-160614-IB07400744-add]//
  
  SetMem (&BootMediaData, sizeof (MBP_CURRENT_BOOT_MEDIA), 0xFF);


  Status = HeciGetIfwiDnxRequest (&IfwiDnxRequest);
  if (EFI_ERROR (Status)) {
    SetMem (&IfwiDnxRequest, sizeof (MBP_IFWI_DNX_REQUEST), 0x0);
  }

  Status = HeciGetBootDevice (&BootMediaData);
//[-start-160726-IB07400762-add]//
  if (EFI_ERROR(Status)) {
    SetMem(&BootMediaData, sizeof(MBP_CURRENT_BOOT_MEDIA), 0xFF);
  }
//[-end-160726-IB07400762-add]//
  DeviceIndex = BootMediaData.PhysicalData;

  if (DeviceIndex == 0) {
    DEBUG ((DEBUG_INFO, "CSE Boot Device is EMMC.\n"));
    Status = (*PeiServices)->InstallPpi(PeiServices, mCseEmmcSelectPpiList);
  } else if (DeviceIndex == 1) {
    DEBUG ((DEBUG_INFO, "CSE Boot Device is UFS.\n"));
    Status = (*PeiServices)->InstallPpi(PeiServices, mCseUfsSelectPpiList);
  } else if (DeviceIndex == 2) {
    DEBUG ((DEBUG_INFO, "CSE Boot Device is SPI.\n"));
    Status = (*PeiServices)->InstallPpi(PeiServices, mCseSpiSelectPpiList);
  }  else {
    DEBUG ((DEBUG_INFO, "CSE Boot Device is Unknown, Try EMMC and UFS.\n"));
//[-start-160311-IB07400709-modify]//
//[-start-160614-IB07400744-modify]//
    if (BootMode == BOOT_IN_RECOVERY_MODE) {
      Status = (*PeiServices)->InstallPpi(PeiServices, mCseSpiSelectPpiList);
      DEBUG ((DEBUG_INFO, "Recovery mode, force CSE Boot Device is SPI.\n"));
    } else {
      CHIPSET_POST_CODE (PEI_UNKNOWN_BOOT_SOURCE);
      DEBUG ((EFI_D_INFO, "If you're stuck here and boot source is eMMC, Please check that your eMMC has GPP4.\n"));
      CpuDeadLoop();
    }
//[-end-160614-IB07400744-modify]//
//[-end-160311-IB07400709-modify]//
  }

  //
  // Build HOB for BootMediaData
  //
  BuildGuidDataHob (
    &gEfiBootMediaHobGuid,
    &BootMediaData,
    sizeof (MBP_CURRENT_BOOT_MEDIA)
    );

  //
  // Build HOB for IfwiDnxRequest
  //
  BuildGuidDataHob (
    &gEfiIfwiDnxRequestHobGuid,
    &IfwiDnxRequest,
    sizeof (MBP_IFWI_DNX_REQUEST)
    );
  if (DeviceIndex != 2) {
//[-start-160218-IB07400702-modify]//
//[-start-160614-IB07400744-modify]//
    Status = HeciTakeOwnerShip();
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "HeciTakeOwnerShip %r, do the cold reboot\n", Status));
      CHIPSET_POST_CODE (PEI_FRC_HECI_TAKE_OWNER_SHIP_FAIL);
      if (BootMode != BOOT_IN_RECOVERY_MODE) {
        IoWrite8 (0xCf9, 0xE); // do the cold reboot
      }
    }
//[-end-160614-IB07400744-modify]//
//[-end-160218-IB07400702-modify]//
  }
//[-start-160216-IB07400702-add]//
  CHIPSET_POST_CODE (PEI_FRC_CHECK_BOOT_DEVICE_END);
//[-end-160216-IB07400702-add]//
}

/**
  EDES_TODO: Add function description

  @param PeiServices     EDES_TODO: Add parameter description
  @param BiosAction      EDES_TODO: Add parameter description

  @return Return EFI_SUCCESS

**/



EFI_STATUS
SeCTakeOwnerShip (
  )
{
  return EFI_SUCCESS;
}

/**

  This procedure will issue a Non-Power Cycle, Power Cycle, or Global Rest.


  @param PeiServices     - General purpose services available to every PEIM.
  @param ResetType       -  Type of reset to be issued.

  @return Return EFI_SUCCESS

**/
EFI_STATUS
PerformReset (
  UINT8       ResetType
  )
{
  EFI_STATUS  Status;
  UINT32      Data32;
  UINT32      GpioBase;
  UINT8       Reset;
  UINT32      PmcPciMmioBase;
  UINT32      PmcBase;

  Reset     = 0;
  GpioBase  = 0;
  //
  // Clear the DISB bit in SC (DRAM Initialization Scratchpad Bit - GEN_PMCON2[7]),
  // since S3 Data will not be saved to NVRAM.
  //
  Status = ClearDISB ();

  //
  // Clear CF9GR of SC (PMCBASE 0x1048[20]) to  indicate Host reset
  // Make sure CWORWRE (CF9 Without Resume Well Reset Enable) is cleared
  //
  PmcPciMmioBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC,PCI_DEVICE_NUMBER_PMC,PCI_FUNCTION_NUMBER_PMC);
  PmcBase = MmioRead32 (PmcPciMmioBase + R_PMC_BASE);

  MmioAnd32(
    (UINTN)(PmcBase + R_PMC_ETR),
    (UINT32)~(B_PMC_ETR_CF9GR)
    );

  Reset = IoRead8 (R_RST_CNT);
  Reset &= 0xF1;


  switch (ResetType) {
  case 0:
    ///
    /// Case: DID ACK was not received
    ///
    DEBUG((DEBUG_ERROR, "DID Ack was not received, no BIOS Action to process.\n"));
    break;

  case CBM_DIR_NON_PCR:
    DEBUG ((DEBUG_INFO, "SEC FW DID ACK has requested a Non Power Cycle Reset.\n"));
    Reset |= 0x06;
    break;

  case CBM_DIR_PCR:
    //
    // Power Cycle Reset requested
    //
    DEBUG ((DEBUG_INFO, "SEC FW DID ACK has requested a Power Cycle Reset.\n"));
    Reset |= 0x0E;
    break;

  case 3:
    ///
    /// Case: Go To S3
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID ACK has requested entry to S3.  Not defined, continuing to POST.\n"));
    break;

  case 4:
    ///
    /// Case: Go To S4
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID ACK has requested entry to S4.  Not defined, continuing to POST.\n"));
    break;

  case 5:
    ///
    /// Case: Go To S5
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID ACK has requested entry to S5.  Not defined, continuing to POST.\n"));
    break;

  case CBM_DIR_GLOBAL_RESET:
    //
    // Global Reset
    //
    DEBUG ((DEBUG_INFO, "SEC FW DID Ack requested a global reset.\n"));

    //
    // Drive GPIO[30] (SPDNA#) low prior to 0xCF9 write
    //
    //
    // Only reset GPIO[30] by resume reset
    //
    Data32 = IoRead32 (GpioBase + 0x60);
    Data32 |= BIT30;
    IoWrite32 (GpioBase + 0x60, Data32);

    //
    // Set CF9GR of SC (PMCBASE offset 0x1048[20] = 1b) to  indicate Global reset
    //
    MmioOr32(
      (UINTN)(PmcBase + R_PMC_ETR),
      (UINT32)(B_PMC_ETR_CF9GR)
      );

    //
    // Issue global reset CF9 = 0x0E
    //
    DEBUG ((DEBUG_INFO, "Issuing global reset.\n"));
    Reset |= 0x0E;
    break;

  case CBM_DIR_CONTINUE_POST:
    ///
    /// Case: Continue to POST
    ///
    DEBUG((DEBUG_INFO, "SEC FW DID Ack requested to continue to POST.\n"));
    break;
  }
  //
  // Write SC RST CNT, Issue Reset
  //
  IoWrite8 (R_RST_CNT, Reset);

  return EFI_SUCCESS;
}


/**

  This procedure will clear the DISB.


  @param VOID            EDES_TODO: Add parameter description

  @return Return EFI_SUCCESS

**/
EFI_STATUS
ClearDISB (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**

  This function is the entry point for this PEI.


  @param FileHandle      EDES_TODO: Add parameter description
  @param FileHandle      EDES_TODO: Add parameter description
  @param endif           EDES_TODO: Add parameter description

  @return Return Status based on errors that occurred while waiting for time to expire.

**/

EFI_STATUS
EFIAPI
SeCUmaEntry (
  IN EFI_PEI_FILE_HANDLE     *FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS                        Status;
  EFI_PEI_PPI_DESCRIPTOR            *PeiPpiDescriptor;
  EFI_BOOT_MODE                     BootMode;
#ifdef FSP_FLAG
  FSPM_UPD                          *FspmUpd;
#endif

  if (!ImageInMemory) {

    Status = (*PeiServices)->InstallPpi (PeiServices, mSeCUmaPpiList);
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // locate the SecUma PPI
    //
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gSeCUmaPpiGuid, // GUID
                               0,                                  // INSTANCE
                               &PeiPpiDescriptor,          // EFI_PEI_PPI_DESCRIPTOR
                               NULL // PPI
                               );
    if (Status == EFI_SUCCESS) {
      //
      // Reinstall the SecUma PPI
      //
      Status = (**PeiServices).ReInstallPpi (
                                 PeiServices,
                                 PeiPpiDescriptor,
                                 mSeCUmaPpiList
                                 );
    }

    Status = (*PeiServices)->GetBootMode (
                           PeiServices,
                           &BootMode
                           );

    if (EFI_ERROR (Status) || BootMode != BOOT_ON_S3_RESUME) {
#ifndef FSP_FLAG
      DEBUG((DEBUG_INFO, "SeCUmaEntry() CheckBootDevice, %r\n", Status));
      CheckBootDevice (PeiServices);
#endif
    } else {
#ifndef FSP_FLAG
      Status = PeiServicesNotifyPpi (mTxeNotifyList);
#endif

#ifdef FSP_FLAG
    FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
    if (FspmUpd->FspmConfig.EnableS3Heci2) {
#endif
      HeciReset (HECI2_DEVICE);
#ifdef FSP_FLAG
    }
#endif
    }
  }
  return Status;
}


/**

  This procedure will check the exposure of SeC device.


  @param PeiServices     - Pointer to the PEI services table

  @return Return EFI_SUCCESS

**/
EFI_STATUS
isSeCExpose(
  IN EFI_PEI_SERVICES **PeiServices
  )
{
EFI_STATUS Status = 0x1;

  // Device ID read here
  UINT32 DeviceID;
  DeviceID = (HeciPciRead32 (R_SEC_DevID_VID) & S_SEC_DevID_MASK) >> 16;

 // if(DeviceID >= S_SEC_DevID_RANGE_LO && DeviceID <= S_SEC_DevID_RANGE_HI) {
 //   Status = 0x1;
    DEBUG ((DEBUG_INFO, "SeC Device ID: %x\n", DeviceID));
 //  }
  return Status;
}

#ifndef FSP_FLAG
VOID
HideHeci23 (
  VOID
  )
{
  UINT32 HeciBar;

  DEBUG ((EFI_D_INFO, "Put all HECI devices into D0i3 and hide HECI 2 and HECI 3.\n"));

  //
  // 1. Read the DEVIDLEC register value
  // 2. If DEVIDLE = 1, already in D0i3 state - nothing to do, exit
  // 3. If CIP = 1, wait until cleared (poll on the bit value until cleared)
  // 4. Set DEVIDLE = 1 and write the DEVIDLEC register value
  //
  HeciBar = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0 ;
  if ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    // Poll until CIP == 0
    while ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_CIP) == B_HECI_DEVIDLEC_CIP);

    Mmio32Or (HeciBar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  HeciBar = Heci2PciRead32 (R_HECIMBAR0) & 0xFFFFFFF0 ;
  if ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    // Poll until CIP == 0
    while ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_CIP) == B_HECI_DEVIDLEC_CIP);

    Mmio32Or (HeciBar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  HeciBar = Heci3PciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
  if ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    // Poll until CIP == 0
    while ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_CIP) == B_HECI_DEVIDLEC_CIP);

    Mmio32Or (HeciBar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  //
  // To hide the HECI 2 in PCI configuration space
  //
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F1_FUN_DIS,
    0xFFFFFFFF,
    (UINT32) BIT0
    );

  //
  // To hide the HECI 3 in PCI configuration space
  //
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F2_FUN_DIS,
    0xFFFFFFFF,
    (UINT32) BIT0
    );
 }

/**
  Check it's SPI boot path or not.

  @retval TRUE                       SPI Boot path
  @retval FALSE                      Not SPI boot path
**/
BOOLEAN
IsSpiBoot(
  VOID
  )
{
  VOID                                  *HobList;
  MBP_CURRENT_BOOT_MEDIA                *BootMediaData;
  DEBUG ((EFI_D_INFO, "IsSpiBoot Start!\n"));
  HobList = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  if (HobList != NULL) {
    DEBUG ((EFI_D_INFO, "IsSpiBoot HobList != NULL\n"));
    BootMediaData = GET_GUID_HOB_DATA (HobList);
    if (BootMediaData->PhysicalData == BOOT_FROM_SPI) {
      DEBUG ((EFI_D_INFO, "BootMediaData->PhysicalData ==  IsSpiBoot\n"));
      return TRUE;
    } else {
      DEBUG ((EFI_D_INFO, "Not boot from SPI\n"));
      return FALSE;
    }
  }
  return FALSE;
}
/**
  Txe End of PEI callback function. This is the last event before entering DXE and OS in S3 resume.

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
EFIAPI
TxeOnEndOfPei (
  IN EFI_PEI_SERVICES                   **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR          *NotifyDesc,
  IN VOID                               *Ppi
  )
{
  DEBUG ((EFI_D_INFO, "TxeOnEndOfPei Start!\n"));

  PERF_START_EX (NULL, NULL, NULL, 0, 0x8100);
  HideHeci23();
  PERF_END_EX (NULL, NULL, NULL, 0, 0x8101);

  DEBUG ((EFI_D_INFO, "TxeOnEndOfPei Exit!\n"));
  return EFI_SUCCESS;
}
#endif
