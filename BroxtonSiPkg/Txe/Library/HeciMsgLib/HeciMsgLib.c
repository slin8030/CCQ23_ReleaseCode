/** @file
  Implementation file for common HECI Message functionality

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2018 Intel Corporation.

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

@par Specification Reference

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/PerformanceLib.h>
#include <Library/TimerLib.h>
#include <SeCState.h>
#include <CoreBiosMsg.h>
#include <HeciRegs.h>
#include <SeCAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Protocol/Heci.h>
#include <Private/Library/HeciInitLib.h>
#include <Library/PeiDxeSmmMmPciLib.h>
#include <Library/HobLib.h>
//[-start-161205-IB11270171-add]//
#include <Library/PcdLib.h>
//[-end-161205-IB11270171-add]//
//[-start-190215-IB16990019-add]//
extern EFI_GUID gMeBiosPayloadSensitiveDataHobGuid;
extern EFI_GUID gMeBiosPayloadHobGuid;
//[-end-190215-IB16990019-add]//

//[-start-160629-IB07400749-add]//
#ifdef EFI_DEBUG
#ifndef EFI_DEBUG_HECI_MESSAGE
#undef DEBUG
#define DEBUG(Expression)
#endif
#endif
//[-end-160629-IB07400749-add]//

#define MIRROR_RANGE_MCHBAR                0x65c8 //0x6458 // 0x65C8
#define MOT_OUT_BASE_Bunit                 0x6AF0
#define MOT_OUT_MASK_Bunit                 0x6AF4
#define MOT_OUT_BASE_Aunit                 0x64C0
#define MOT_OUT_MASK_Aunit                 0x64C4
#define SPARE_BIOS_MCHBAR                  0x647C // BXTM 0x6474

#define MCHBASE                            MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN)
#define MCHBASE_BAR                        MmioRead32(MCHBASE + 0x48) &~BIT0
#define MCHMmioRead32(offset)              MmioRead32((MCHBASE_BAR)+offset)
#define MCHMmioWrite32(offset, data)       MmioWrite32((MCHBASE_BAR)+offset, data)

VOID* WriteCacheBuffer = NULL;
VOID* ReadCacheBuffer  = NULL;

//[-start-160808-IB07220123-add]//
STATIC BOOLEAN mReadyToBootEventSignaled = FALSE;
//[-end-160808-IB07220123-add]//

/**
  Get the HECI2 read buffer size.

  @param       None

  @retval      UINTN   The HECI2 read buffer size in bytes.
**/
UINTN
EFIAPI
HeciGetHeci2ReadBufferSize (
  VOID
  )
{
  return MAX_HECI2_READ_DATA_SIZE;
}

/**
  Get the HECI2 write buffer size.

  @param       None

  @retval      UINTN   The HECI2 write buffer size in bytes.
**/
UINTN
EFIAPI
HeciGetHeci2WriteBufferSize (
  VOID
  )
{
  return MAX_HECI2_WRITE_DATA_SIZE;
}

/**
  Send DID Request Message through HECI.

  @param[in]  UMABase      The IMR Base address
  @param[out] UMASize      The IMR region size.
  @param[out] BiosAction   ME response to DID

  @return EFI_SUCCESS      Send DID success.
  @return Others           Send DID failed.
**/
EFI_STATUS
HeciSendDIDMessage (
  IN  UINT32 UMABase,
  IN  UINT8  IsS3,
  OUT UINT32 *UMASize,
  OUT UINT8  *BiosAction
  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength;
  DRAM_INIT_DONE_CMD_REQ          *SendDID;
  DRAM_INIT_DONE_CMD_RESP_DATA    *DIDResp;
  UINT32                          SeCMode;
  UINT32                          DataBuffer[0x70];
  UINT32                          temp32;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciSendDIDMessage\n"));
  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || ((SeCMode != SEC_MODE_NORMAL)&&(SEC_MODE_RECOVER !=SeCMode)
  )) {
    DEBUG ((EFI_D_INFO, "CSE Firmware not in normal and SV mode\n"));
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));
  temp32 = MCHMmioRead32(MOT_OUT_MASK_Bunit);
  SetMem(DataBuffer, sizeof(DataBuffer), 0);
  SendDID = (DRAM_INIT_DONE_CMD_REQ*)DataBuffer;
  SendDID->MKHIHeader.Data              = 0;
  SendDID->MKHIHeader.Fields.Command    = DRAM_INIT_DONE_CMD; ///<DRAM INIT DONE =0x01
  SendDID->MKHIHeader.Fields.IsResponse = 0;
  SendDID->MKHIHeader.Fields.GroupId    = COMMON_GROUP_ID;//Group ID = 0xF0
  SendDID->ImrData.BiosMinImrsBa    = UMABase;

  if (TRUE == IsS3) {
    DEBUG ((EFI_D_INFO, "Setting  NonDestructiveAliasCheck in DID command successful\n"));
    SendDID->Flags.NonDestructiveAliasCheck=1;
  }

  HeciSendLength                        = sizeof(DRAM_INIT_DONE_CMD_REQ);
  HeciRecvLength                        = sizeof(DataBuffer);


  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  DIDResp = (DRAM_INIT_DONE_CMD_RESP_DATA*)DataBuffer;

  if (DIDResp->MKHIHeader.Fields.Result == 0) {
    DEBUG ((EFI_D_INFO, "HeciSend DRAM Init Done successful\n"));
  }
  MCHMmioWrite32(MOT_OUT_MASK_Bunit,temp32);
  DEBUG ((EFI_D_INFO, "ImrsSortedRegionBa =%x\n", DIDResp->ImrsData.ImrsSortedRegionBa));
  DEBUG ((EFI_D_INFO, "ImrsSortedRegionLen=%x\n", DIDResp->ImrsData.ImrsSortedRegionLen));
  DEBUG ((EFI_D_INFO, "OemSettingsRejected=%x\n", DIDResp->ImrsData.OemSettingsRejected));
  DEBUG ((EFI_D_INFO, "BiosAction         =%x\n", DIDResp->BiosAction));
  ASSERT(UMABase == DIDResp->ImrsData.ImrsSortedRegionBa);
  *UMASize = DIDResp->ImrsData.ImrsSortedRegionLen;

  *BiosAction = DIDResp->BiosAction;

  return Status;
}

/**
  Get NVM file's size through HECI1.

  @param[in]  FileName       The file name.
  @param[out] FileSize       The file's size.

  @return     EFI_SUCCESS    Get NVM file size success.
  @return     Others         Get NVM file size failed.
**/
EFI_STATUS
HeciGetNVMFileSize (
  IN  CONST  CHAR8    *FileName,
  OUT        UINTN    *FileSize
  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength;
  GET_FILESIZE_CMD_REQ_DATA       *SendNVMGet;
  GET_FILESIZE_CMD_RESP_DATA      *NVMGetResp;
  UINT32                          SeCMode;
  UINT32                          DataBuffer[0x70];

//[-start-170613-IB07400874-modify]//
  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: GetNVMFileSize (FileName:%a)\n", FileName));
//[-end-170613-IB07400874-modify]//
  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendNVMGet = (GET_FILESIZE_CMD_REQ_DATA*)DataBuffer;
  SendNVMGet->MKHIHeader.Fields.GroupId = 0xA;
  SendNVMGet->MKHIHeader.Fields.Command = HECI1_FILE_SIZE;
  AsciiStrCpyS ((CHAR8 *)SendNVMGet->FileName, MCA_MAX_FILE_NAME, FileName);

  DEBUG ((EFI_D_INFO, "WRITE_TO_BIOS_DATA_CMD_REQ_DATA size if %x\n", sizeof(GET_FILESIZE_CMD_REQ_DATA)));
  HeciSendLength                        = sizeof(GET_FILESIZE_CMD_REQ_DATA);
  HeciRecvLength                        = sizeof(DataBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  NVMGetResp = (GET_FILESIZE_CMD_RESP_DATA*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group    =%x\n", NVMGetResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%x\n", NVMGetResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%x\n", NVMGetResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%x\n", NVMGetResp->MKHIHeader.Fields.Result));
  switch(NVMGetResp->MKHIHeader.Fields.Result) {
  case BIOS_HECI_STATUS_OK:
    Status = EFI_SUCCESS;
    break;
  case BIOS_HECI_STATUS_INVALID_PARAM:
    Status = EFI_INVALID_PARAMETER;
    break;
  case BIOS_HECI_STATUS_FILE_NOT_FOUND:
    Status = EFI_NOT_FOUND;
    break;
  case BIOS_HECI_STATUS_AFTER_EOP:
    Status = EFI_DEVICE_ERROR;
    break;
  case BIOS_HECI_STATUS_ERROR:
    Status = EFI_DEVICE_ERROR;
    break;
  default:
    Status = EFI_DEVICE_ERROR;
    break;
  }
  if (!EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "FileSize =%x\n", NVMGetResp->FileSize));
    *FileSize = NVMGetResp->FileSize;
  }
  return Status;
}

/**
  Set Read Write Temp memory.

  @param[in] Address   Temp memory.
**/
VOID
HeciSetReadWriteCache(
  VOID*  Address
)
{
  WriteCacheBuffer = Address;
  ReadCacheBuffer  = Address;
}

/**
  Write Data to NVM file through HECI1.

  @param[in] FileName     The file name.
  @param[in] Offset       The offset of data.
  @param[in] Data         The data content.
  @param[in] DataSize     Number of bytes to write.
  @param[in] Truncate     Truncate the file.

  @return    EFI_SUCCESS  Write NVM file success.
  @return    Others       Write NVM file failed.
**/
EFI_STATUS
HeciWriteNVMFile (
  IN CONST CHAR8    *FileName,
  IN       UINT32   Offset,
  IN CONST UINT8    *Data,
  IN       UINTN    DataSize,
  IN       BOOLEAN  Truncate
  )
{
  EFI_STATUS                           Status;
  UINT32                               HeciSendLength;
  UINT32                               HeciRecvLength;
  WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA   *SendNVMWrite;
  WRITE_TO_RPMB_STORAGE_CMD_RESP_DATA  *NVMWriteResp;
  UINT32                               SeCMode;
  UINT32                               WriteMessageBuffer[0x70];

  VOID                                 *DataSrcBufferPtr   = NULL;
  VOID                                 *AllocatedBufferPtr = NULL;

//[-start-170613-IB07400874-modify]//
  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: WriteNVMFile (FileName:%a), FileOffset: 0x%x, WriteMessageBuffer: 0x%x, DataSize: 0x%x\n", FileName, Offset, Data, DataSize));
//[-end-170613-IB07400874-modify]//
  Status = HeciGetSeCMode (HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  //
  // Allocate the number of 4K pages buffer to support CSE DMA requirement.
  // Workaround: Since the address range provided to CSE can't be in SMRAM, must use the pre-allocated buffer
  //             when needing to read/write NVM file(s). The pre-allocated 64KB buffer must be initialized
  //             by HeciSetReadWriteCache() before the buffer can be used by the SMM variable service.
  //             For non SMM variables in PEI/DXE phase, the pre-allocated buffer is not need because the NVM file
  //             service allocates the temporary pages with EfiBootServiceData memory type automatically.
  //
  if (WriteCacheBuffer != NULL) {
    DataSrcBufferPtr = WriteCacheBuffer;
  } else {
    //
    // The pre-allocated buffer has not yet initialized, then allocate pages from BS data area.
    //
//[-start-161003-IB07220139-modify]//
//[-start-161202-IB11270171-modify]//
//[-start-170330-IB07400855-modify]//
    if (PcdGet32(PcdAccessNVMFileBase) == 0) {
      AllocatedBufferPtr = AllocatePages (EFI_SIZE_TO_PAGES (DataSize == 0 ? 1 : DataSize));
    } else {
      AllocatedBufferPtr = (VOID*)(UINTN)PcdGet32(PcdAccessNVMFileBase);
      ZeroMem (AllocatedBufferPtr, PcdGet32(PcdCseNvmDataFileMaximumSize));
    }
//[-end-170330-IB07400855-modify]//
//[-end-161202-IB11270171-modify]//
//[-end-161003-IB07220139-modify]//
    DataSrcBufferPtr   = AllocatedBufferPtr;
  }
  if (DataSrcBufferPtr == NULL) {
    ASSERT (DataSrcBufferPtr != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (DataSrcBufferPtr, Data, DataSize);
  ZeroMem (WriteMessageBuffer, sizeof (WriteMessageBuffer));

  SendNVMWrite                            = (WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA *) WriteMessageBuffer;
  SendNVMWrite->MKHIHeader.Data           = 0;
  SendNVMWrite->MKHIHeader.Fields.GroupId = GROUP_ID_MCA;
  SendNVMWrite->MKHIHeader.Fields.Command = HECI1_WRITE_DATA;

  AsciiStrCpyS ((CHAR8 *) SendNVMWrite->FileName, sizeof (SendNVMWrite->FileName), FileName);

  SendNVMWrite->Offset          = Offset;
  SendNVMWrite->Size            = (UINT32) DataSize;
  SendNVMWrite->Truncate        = (Truncate) ? 1 : 0;
  SendNVMWrite->SrcAddressLower = (UINT32) (UINTN) DataSrcBufferPtr;
  SendNVMWrite->SrcAddressUpper = (UINT32) ((PHYSICAL_ADDRESS) (UINTN) DataSrcBufferPtr >> 32);
  DEBUG ((EFI_D_INFO, "HECI1 Write Source Data Buffer: 0x%x\n", DataSrcBufferPtr));

  DEBUG ((EFI_D_INFO, "WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA size is %d bytes.\n", sizeof (WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA)));
  HeciSendLength = sizeof (WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA);
  HeciRecvLength = sizeof (WriteMessageBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             WriteMessageBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  NVMWriteResp = (WRITE_TO_RPMB_STORAGE_CMD_RESP_DATA *) WriteMessageBuffer;

  DEBUG ((EFI_D_INFO, "Group    =%08x\n", NVMWriteResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", NVMWriteResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", NVMWriteResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", NVMWriteResp->MKHIHeader.Fields.Result));

  if (NVMWriteResp->MKHIHeader.Fields.Result != 0) {
    DEBUG ((EFI_D_ERROR, "NVM Write Failed. Update is lost. If repeatedly encountered, clear GPP4 and try again\n"));
    Status = EFI_DEVICE_ERROR;
  }

  if (AllocatedBufferPtr != NULL) {
//[-start-161003-IB07220139-modify]//
//[-start-161202-IB11270171-modify]//
//[-start-170330-IB07400855-modify]//
    if (PcdGet32(PcdAccessNVMFileBase) == 0) {
      FreePages (AllocatedBufferPtr, EFI_SIZE_TO_PAGES (DataSize == 0 ? 1 : DataSize));
    } else {
      ZeroMem (AllocatedBufferPtr, PcdGet32(PcdCseNvmDataFileMaximumSize));
    }
//[-end-170330-IB07400855-modify]//
//[-end-161202-IB11270171-modify]//
//[-end-161003-IB07220139-modify]//
  }
  return Status;
}

/**
  Read NVM file data from HECI1.

  @param[in]     FileName     The file name.
  @param[in]     Offset       The offset of data.
  @param[out]    Data         The data buffer.
  @param[in,out] DataSize     Number of bytes to read.

  @return        EFI_SUCCESS  Read NVM file success.
  @return        Others       Read NVM file failed.
**/
EFI_STATUS
HeciReadNVMFile (
  IN      CONST  UINT8      *FileName,
  IN             UINT32     Offset,
  OUT            UINT8      *Data,
  IN OUT         UINTN      *DataSize
  )
{
  EFI_STATUS                            Status;
  UINT32                                HeciSendLength;
  UINT32                                HeciRecvLength;
  READ_FROM_RPMB_STORAGE_CMD_REQ_DATA   *SendNVMRead;
  READ_FROM_RPMB_STORAGE_CMD_RESP_DATA  *NVMReadResp;
  UINTN                                 RequestedDataSize;
  UINT32                                SeCMode;
  UINT32                                ReadMessageBuffer[0x70];

  VOID                                  *DataDestBufferPtr  = NULL;
  VOID                                  *AllocatedBufferPtr = NULL;

  RequestedDataSize = *DataSize;

//[-start-170613-IB07400874-modify]//
  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: ReadNVMFile (FileName:%a) FileOffset : 0x%x, DataBuffer: 0x%x, DataSize : 0x%x \n", FileName, Offset, Data, *DataSize));
//[-end-170613-IB07400874-modify]//
  Status = HeciGetSeCMode (HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  //
  // Allocate the number of 4K pages buffer to support CSE DMA requirement.
  // Workaround: Since the address range provided to CSE can't be in SMRAM, must use the pre-allocated buffer
  //             when needing to read/write NVM file(s). The pre-allocated 64KB buffer must be initialized
  //             by HeciSetReadWriteCache() before the buffer can be used by the SMM variable service.
  //             For non SMM variables in PEI/DXE phase, the pre-allocated buffer is not need because the NVM file
  //             service allocates the temporary pages with EfiBootServiceData memory type automatically.
  //
  if (ReadCacheBuffer != NULL) {
    DataDestBufferPtr = ReadCacheBuffer;
  } else {
    //
    // The pre-allocated buffer has not yet initialized, then allocate pages from BS data area.
    //
//[-start-161202-IB11270171-modify]//
//[-start-170330-IB07400855-modify]//
    if (PcdGet32(PcdAccessNVMFileBase) == 0) {
      AllocatedBufferPtr = AllocatePages (EFI_SIZE_TO_PAGES (RequestedDataSize));
    } else {
      AllocatedBufferPtr = (VOID*)(UINTN)PcdGet32(PcdAccessNVMFileBase);
      ZeroMem (AllocatedBufferPtr, PcdGet32(PcdCseNvmDataFileMaximumSize));
    }  
//[-end-170330-IB07400855-modify]//
//[-end-161202-IB11270171-modify]//    
    DataDestBufferPtr  = AllocatedBufferPtr;
  }
  if (DataDestBufferPtr == NULL) {
    ASSERT (DataDestBufferPtr != NULL);
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (ReadMessageBuffer, sizeof (ReadMessageBuffer));

  SendNVMRead                            = (READ_FROM_RPMB_STORAGE_CMD_REQ_DATA*) ReadMessageBuffer;
  SendNVMRead->MKHIHeader.Fields.GroupId = GROUP_ID_MCA;
  SendNVMRead->MKHIHeader.Fields.Command = HECI1_READ_DATA;

  AsciiStrCpyS (
    (CHAR8 *) SendNVMRead->FileName,
    sizeof (SendNVMRead->FileName),
    (CHAR8 *) FileName
    );

  SendNVMRead->Offset          = Offset;
  SendNVMRead->Size            = (UINT16) RequestedDataSize;
  SendNVMRead->DstAddressLower = (UINT32) (UINTN) DataDestBufferPtr;
  SendNVMRead->DstAddressUpper = (UINT32) ((PHYSICAL_ADDRESS) (UINTN) DataDestBufferPtr >> 32);
  DEBUG ((EFI_D_INFO, "DataDestBufferPtr: 0x%x \n", DataDestBufferPtr));

  DEBUG ((EFI_D_INFO, "READ_FROM_RPMB_STORAGE_CMD_REQ_DATA size if %x\n", sizeof(READ_FROM_RPMB_STORAGE_CMD_REQ_DATA)));
  HeciSendLength = sizeof (READ_FROM_RPMB_STORAGE_CMD_REQ_DATA);
  HeciRecvLength = sizeof (ReadMessageBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             ReadMessageBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  NVMReadResp = (READ_FROM_RPMB_STORAGE_CMD_RESP_DATA *) ReadMessageBuffer;

  DEBUG ((EFI_D_INFO, "Group    =%08x\n", NVMReadResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", NVMReadResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", NVMReadResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", NVMReadResp->MKHIHeader.Fields.Result));
  DEBUG ((EFI_D_INFO, "DataSize =%08x\n", NVMReadResp->DataSize));

  if (NVMReadResp->MKHIHeader.Fields.Result == 0) {
    *DataSize = NVMReadResp->DataSize;
    if (NVMReadResp->DataSize > RequestedDataSize) {
        Status = EFI_BUFFER_TOO_SMALL;
    } else {
        CopyMem(Data, DataDestBufferPtr, NVMReadResp->DataSize);
    }
  } else {
    Status = EFI_DEVICE_ERROR;
  }

  if (AllocatedBufferPtr != NULL) {
//[-start-161202-IB11270171-modify]//
//[-start-170330-IB07400855-modify]//
    if (PcdGet32(PcdAccessNVMFileBase) == 0) {
      FreePages (AllocatedBufferPtr, EFI_SIZE_TO_PAGES (RequestedDataSize));
    } else {
      ZeroMem (AllocatedBufferPtr, PcdGet32(PcdCseNvmDataFileMaximumSize));
    }  
//[-end-170330-IB07400855-modify]//
//[-end-161202-IB11270171-modify]//
  }
  return Status;
}

/**
  Clears all sensitive MBP data from memory.

  @retval EFI_SUCCESS             Sensitive MBP data was cleared successfully.
  @retval EFI_NOT_FOUND           Sensitive MBP data was not found.

**/
EFI_STATUS
EFIAPI
MbpClearSensitiveData (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS    GuidHob;
  UINTN                   MbpSensitiveDataHobInstances;

  DEBUG ((EFI_D_INFO, "MbpClearSensitiveData\n"));
  GuidHob.Raw = GetHobList ();
  MbpSensitiveDataHobInstances = 0;
  while ((GuidHob.Raw = GetNextGuidHob (&gMeBiosPayloadSensitiveDataHobGuid, GuidHob.Raw)) != NULL) {
    ZeroMem (GET_GUID_HOB_DATA (GuidHob.Guid), (UINTN) GET_GUID_HOB_DATA_SIZE (GuidHob.Guid));
    GuidHob.Raw = GET_NEXT_HOB (GuidHob);
    MbpSensitiveDataHobInstances++;
  }
  if (MbpSensitiveDataHobInstances == 0) {
    return EFI_NOT_FOUND;
  }
//
// Only one instance of the sensitive data MBP HOB should exist
//
  ASSERT (MbpSensitiveDataHobInstances == 1);
  DEBUG ((EFI_D_INFO, "Successfully cleared the all Sensitive Data HOB\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
HeciDataClearLock (
  VOID
  )
{
  EFI_STATUS                Status;
  UINT32                    HeciSendLength;
  UINT32                    HeciRecvLength;
  DATA_CLEAR_LOCK_REQ       *SendDCL;
  DATA_CLEAR_LOCK_RES       *DCLResp;
  UINT32                    SeCMode;
  UINT32                    DataBuffer[0x40];
  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || ((SeCMode != SEC_MODE_NORMAL) && (SeCMode != SEC_MODE_RECOVER))) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));
  SetMem(DataBuffer, sizeof(DataBuffer), 0);
  SendDCL = (DATA_CLEAR_LOCK_REQ*)DataBuffer;
  SendDCL->MKHIHeader.Fields.GroupId     = MKHI_IFWI_UPDATE_GROUP_ID;
  SendDCL->MKHIHeader.Fields.Command     = DATA_CLEAR_LOCK_CMD_ID;
  SendDCL->MKHIHeader.Fields.IsResponse  = 0;
  DEBUG ((EFI_D_INFO, "HeciDataClearLock size is %x\n", sizeof(DATA_CLEAR_LOCK_RES)));
  HeciSendLength              = sizeof(DATA_CLEAR_LOCK_REQ);
  HeciRecvLength              = sizeof(DataBuffer);
  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );
  DCLResp = (DATA_CLEAR_LOCK_RES*)DataBuffer;
  DEBUG ((EFI_D_INFO, "Group    =%08x\n", DCLResp->Header.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", DCLResp->Header.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", DCLResp->Header.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", DCLResp->Header.Fields.Result));
  return DCLResp->Header.Fields.Result;
}
/**
  Send EOP message through HECI1.

  @return EFI_SUCCESS   Send EOP message success.
  @return Others              Send EOP message failed.
**/
EFI_STATUS
HeciEndOfPost(
  VOID
  )
{
  EFI_STATUS                Status;
  UINT32                    HeciSendLength;
  UINT32                    HeciRecvLength;
  GEN_END_OF_POST           *SendEOP;
  GEN_END_OF_POST_ACK       *EOPResp;
  UINT32                    SeCMode;
  UINT32                    DataBuffer[0x70];

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: EndOfPost\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
 if (EFI_ERROR (Status) || ((SeCMode != SEC_MODE_NORMAL)&&(SeCMode !=SEC_MODE_RECOVER))) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendEOP = (GEN_END_OF_POST*)DataBuffer;
  SendEOP->MKHIHeader.Fields.GroupId = EOP_GROUP_ID;
  SendEOP->MKHIHeader.Fields.Command = EOP_CMD_ID;

  DEBUG ((EFI_D_INFO, "GEN_END_OF_POST size is %x\n", sizeof(GEN_END_OF_POST)));
  HeciSendLength              = sizeof(GEN_END_OF_POST);
  HeciRecvLength              = sizeof(DataBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  EOPResp = (GEN_END_OF_POST_ACK*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group    =%08x\n", EOPResp->Header.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", EOPResp->Header.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", EOPResp->Header.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", EOPResp->Header.Fields.Result));
  DEBUG ((EFI_D_INFO, "RequestedActions   =%08x\n", EOPResp->Data.RequestedActions));

  return Status;
}

/**
  Send EOS message through HECI1.

  @return EFI_SUCCESS   Send EOS message success.
  @return Others              Send EOS message failed.
**/
EFI_STATUS
HeciEndOfServices(
  VOID
  )
{
  EFI_STATUS                Status;
  UINT32                    HeciSendLength;
  UINT32                    HeciRecvLength;
  GEN_END_OF_SERVICES       *SendEOS;
  GEN_END_OF_SERVICES_ACK   *EOSResp;
  UINT32                    SeCMode;
  UINT32                    DataBuffer[0x70];

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: EndOfServices\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendEOS = (GEN_END_OF_SERVICES*)DataBuffer;
  SendEOS->MKHIHeader.Fields.GroupId = GROUP_ID_MCA;
  SendEOS->MKHIHeader.Fields.Command = HECI1_EOS;

  DEBUG ((EFI_D_INFO, "GEN_END_OF_SERVICES size is %x\n", sizeof(GEN_END_OF_SERVICES)));
  HeciSendLength              = sizeof(GEN_END_OF_SERVICES);
  HeciRecvLength              = sizeof(DataBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  EOSResp = (GEN_END_OF_SERVICES_ACK*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group    =%08x\n", EOSResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", EOSResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", EOSResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", EOSResp->MKHIHeader.Fields.Result));

  return Status;
}


/**
  Get IFWI DNX request.

  @param[out] IfwiDnxRequestData       Dnx request data buffer.

  @return EFI_SUCCESS   Get IFWI DNX success.
  @return Others              Get IFWI DNX failed.
**/

EFI_STATUS
HeciGetIfwiDnxRequest (
  OUT MBP_IFWI_DNX_REQUEST    *IfwiDnxRequestData
  )
{
  EFI_STATUS                Status;
  UINT32                    DataBuffer [MAX_MBP_ITEM_COUNT];
  MBP_CMD_RESP_DATA         *MBPHeader;
  MBP_ITEM_HEADER           *MBPItem;
  MBP_IFWI_DNX_REQUEST      *IfwiDnxRequest;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetIfwiDnxRequest\n"));
  SetMem(DataBuffer, sizeof(DataBuffer), 0);
  Status = NonSensitiveHeciMBP (DataBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  MBPHeader = (MBP_CMD_RESP_DATA*)DataBuffer;
  DumpBuffer_HECI(MBPHeader,sizeof(MBP_CMD_RESP_DATA));
  MBPItem = (MBP_ITEM_HEADER*)(MBPHeader + 1);
  DumpBuffer_HECI(MBPItem,MBPHeader->Length);
  while ((UINT32*)MBPItem < (UINT32*)DataBuffer + MBPHeader->Length) {
    if (MBPItem->AppID == MBP_APP_ID_DNX && MBPItem->ItemID == MBP_ITEM_IAFW_DNX_REQ) {
      DumpBuffer_HECI(MBPItem,sizeof(MBP_ITEM_HEADER)+MBPItem->Length);
      IfwiDnxRequest = (MBP_IFWI_DNX_REQUEST*)(MBPItem);
      CopyMem((VOID*)IfwiDnxRequestData, (VOID*)IfwiDnxRequest, sizeof(MBP_IFWI_DNX_REQUEST));

      return EFI_SUCCESS;
    }
    MBPItem = (MBP_ITEM_HEADER*)((UINT32*)MBPItem + MBPItem->Length);
    DumpBuffer_HECI(MBPItem,sizeof(MBP_ITEM_HEADER)+MBPItem->Length);
  }
  return EFI_DEVICE_ERROR;
}

/**
  Get Image FW Versions

  @param[out] MsgGetFwVersionRespData      Output FW version response data

  @return EFI_SUCCESS   Get version success.
  @return Others              Get version failed.
**/
EFI_STATUS
HeciGetImageFwVerMsg (
  OUT FW_VERSION_CMD_RESP_DATA    *MsgGetFwVersionRespData
  )
{
  EFI_STATUS                Status;
  UINT32                    HeciSendLength;
  UINT32                    HeciRecvLength;
  FW_VER_CMD_REQ            *MsgGetFwVersion;
  FW_VERSION_CMD_RESP_DATA  *MsgGetFwVersionResp;
  UINT32                    SeCMode;
  UINT32                    DataBuffer[0x20];

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetImageFwVerMsg\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  //
  // Allocate MsgGenGetFwVersion data structure
  //
  SetMem(DataBuffer, sizeof(DataBuffer), 0x0);

  MsgGetFwVersion = (FW_VER_CMD_REQ*)DataBuffer;
  MsgGetFwVersion->MKHIHeader.Fields.GroupId = MKHI_GEN_GROUP_ID;
  MsgGetFwVersion->MKHIHeader.Fields.Command = FW_VER_CMD_REQ_ID;
  MsgGetFwVersion->MKHIHeader.Fields.IsResponse = 0;

  DEBUG ((EFI_D_INFO, "FW_VER_CMD_REQ size is 0x%x, FW_VERSION_CMD_RESP_DATA size is: 0x%x\n", sizeof(FW_VER_CMD_REQ), sizeof(FW_VERSION_CMD_RESP_DATA)));
  HeciSendLength              = sizeof(FW_VER_CMD_REQ);
  HeciRecvLength              = sizeof(DataBuffer);

  //
  // Send Get image Firmware Version Request to CSE
  //
  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  DumpBuffer_HECI(DataBuffer,sizeof(DataBuffer));
  MsgGetFwVersionResp = (FW_VERSION_CMD_RESP_DATA*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group      =%08x\n", MsgGetFwVersionResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command    =%08x\n", MsgGetFwVersionResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone  =%08x\n", MsgGetFwVersionResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result     =%08x\n", MsgGetFwVersionResp->MKHIHeader.Fields.Result));
  DEBUG ((EFI_D_INFO, "NumModules =%08x\n", MsgGetFwVersionResp->NumModules));

  CopyMem(MsgGetFwVersionRespData, DataBuffer, sizeof(FW_VERSION_CMD_RESP_DATA));

  return Status;
}

/**
  Get NFC Device Type through HECI1.

  @param[out] Mbp_Nfc_Device_Type  Output data buffer for NFC device type.

  @return EFI_SUCCESS   Get device success.
  @return Others              Get device failed.
**/

EFI_STATUS
HeciGetNfcDeviceType (
  OUT MBP_NFC_DEVICE_TYPE    *Mbp_Nfc_Device_Type
  )
{
  EFI_STATUS                Status;
  UINT32                    DataBuffer [MAX_MBP_ITEM_COUNT];
  MBP_CMD_RESP_DATA         *MBPHeader;
  MBP_ITEM_HEADER           *MBPItem;
  MBP_NFC_DEVICE_TYPE       *Nfc_Device_Type;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetNfcDeviceType\n"));
  SetMem(DataBuffer, sizeof(DataBuffer), 0);
  Status = NonSensitiveHeciMBP (DataBuffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "HeciGetNfcDeviceType HeciMBP: %r\n", Status));
    return Status;
  }

  MBPHeader = (MBP_CMD_RESP_DATA*)DataBuffer;
  DEBUG ((EFI_D_INFO, "HeciGetNfcDeviceType 1\n"));
  DumpBuffer_HECI(MBPHeader,sizeof(MBP_CMD_RESP_DATA));
  MBPItem = (MBP_ITEM_HEADER*)(MBPHeader + 1);
  DEBUG ((EFI_D_INFO, "HeciGetNfcDeviceType 2\n"));
  DumpBuffer_HECI(MBPItem,sizeof(MBP_ITEM_HEADER));

  while ((UINT32*)MBPItem < (UINT32*)DataBuffer + MBPHeader->Length) {
    if (MBPItem->AppID == 6 && MBPItem->ItemID == 1) {
      DEBUG ((EFI_D_INFO, "HeciGetNfcDeviceType 3\n"));
      DumpBuffer_HECI(MBPItem,sizeof(MBP_ITEM_HEADER)+MBPItem->Length);
      Nfc_Device_Type = (MBP_NFC_DEVICE_TYPE*)(MBPItem);
      CopyMem((VOID*)Mbp_Nfc_Device_Type, (VOID*)Nfc_Device_Type, sizeof(MBP_NFC_DEVICE_TYPE));

      DEBUG ((EFI_D_INFO, "Mbp_Nfc_Device_Type= %x\n",Mbp_Nfc_Device_Type->NfcDeviceData));
      return EFI_SUCCESS;
    }
    MBPItem = (MBP_ITEM_HEADER*)((UINT32*)MBPItem + MBPItem->Length);
    DEBUG ((EFI_D_INFO, "HeciGetNfcDeviceType 4\n"));
    DumpBuffer_HECI(MBPItem,sizeof(MBP_ITEM_HEADER)+MBPItem->Length);
  }
  return EFI_DEVICE_ERROR;
}

/**
Get ME FW Capability from MBP.

@param[out] MBP_ME_FW_CAPS  Output data buffer for ME FW Capability.

@return EFI_SUCCESS         Get ME FW Capability success.
@return Others              Get ME FW Capability failed.
**/

EFI_STATUS
HeciGetMeFwCapability(
  OUT MBP_ME_FW_CAPS    *Mbp_Me_Fw_Caps
  )
{
  EFI_STATUS                Status;
  UINT32                    DataBuffer [MAX_MBP_ITEM_COUNT];
  MBP_CMD_RESP_DATA         *MBPHeader;
  MBP_ITEM_HEADER           *MBPItem;
  MBP_ME_FW_CAPS            *Me_Fw_Caps;
  UINT32                    MBPItemCount;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetMeFwCapability\n"));
  SetMem (DataBuffer, sizeof (DataBuffer), 0);
  Status = NonSensitiveHeciMBP (DataBuffer);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "HeciGetMeFwCapability HeciMBP: %r\n", Status));
    return Status;
  }

  MBPHeader = (MBP_CMD_RESP_DATA*)DataBuffer;
  DEBUG ((EFI_D_INFO, "HeciGetMeFwCapability 1\n"));
  DumpBuffer_HECI(MBPHeader, sizeof(MBP_CMD_RESP_DATA));
  MBPItem = (MBP_ITEM_HEADER*)(MBPHeader + 1);
  DEBUG ((EFI_D_INFO, "HeciGetMeFwCapability 2\n"));
  DumpBuffer_HECI (MBPItem, sizeof (MBP_ITEM_HEADER));
  for (MBPItemCount = 1; MBPItemCount <= MBPHeader->ItemsNum; MBPItemCount++) {
    if (MBPItem->AppID == MBP_APP_ID_KERNEL && MBPItem->ItemID == MBP_ITEM_ID_SMM_TRUSTED_CHANNEL) {
      DEBUG ((EFI_D_INFO, "HeciGetMeFwCapability 3\n"));
      DumpBuffer_HECI(MBPItem, sizeof(MBP_ITEM_HEADER) + MBPItem->Length);
      Me_Fw_Caps = (MBP_ME_FW_CAPS*)(MBPItem);
      CopyMem((VOID*)Mbp_Me_Fw_Caps, (VOID*)Me_Fw_Caps, sizeof(MBP_ME_FW_CAPS));
      DEBUG ((EFI_D_INFO, "Mbp_Me_Fw_Caps = %x\n", Mbp_Me_Fw_Caps->CurrentFeatures));
      return EFI_SUCCESS;
    }
    MBPItem = (MBP_ITEM_HEADER*)((UINT32*)MBPItem + MBPItem->Length);
    DEBUG ((EFI_D_INFO, "HeciGetMeFwCapability 4\n"));
    DumpBuffer_HECI(MBPItem, sizeof(MBP_ITEM_HEADER) + MBPItem->Length);
  }
  return EFI_DEVICE_ERROR;
}

/**
  Send IAFW DNX request set message throught HECI1.

  @param[in] Resp       Buffer to receive the CSE response data..

  @return EFI_SUCCESS   Set request success.
  @return Others        Set request failed.
**/
EFI_STATUS
HeciIafwDnxReqSet (
  IN OUT IAFW_DNX_REQ_SET_RESP_DATA                *Resp
  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength;
  IAFW_DNX_REQ_SET_REQ_DATA       *Req;
  IAFW_DNX_REQ_SET_RESP_DATA      *RespIn;
  UINT32                          SeCMode;
  UINT32                          DataBuffer[0x70];

  DEBUG ((EFI_D_INFO, "BIOS Starts to send HECI Message: HeciIafwDnxReqSet\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || ((SeCMode != SEC_MODE_NORMAL)&&(SEC_MODE_RECOVER !=SeCMode))) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  Req = (IAFW_DNX_REQ_SET_REQ_DATA*)DataBuffer;
  Req->MKHIHeader.Fields.GroupId = MKHI_GEN_DNX_GROUP_ID;
  Req->MKHIHeader.Fields.Command = CSE_DNX_REQ_SET;

  DEBUG ((EFI_D_INFO, "IAFW_DNX_REQ_Set_REQ_DATA size is %x\n", sizeof(IAFW_DNX_REQ_CLEAR_REQ_DATA)));
  HeciSendLength              = sizeof(IAFW_DNX_REQ_SET_REQ_DATA);
  HeciRecvLength              = sizeof(DataBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  RespIn = (IAFW_DNX_REQ_SET_RESP_DATA*)DataBuffer;
  CopyMem((VOID*)Resp, (VOID*)RespIn, sizeof(IAFW_DNX_REQ_SET_RESP_DATA));

  DEBUG ((EFI_D_INFO, "Group    =%08x\n", Resp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", Resp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", Resp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", Resp->MKHIHeader.Fields.Result));
  DEBUG ((EFI_D_INFO, "Response ReqBiosAction   =%08x\n", Resp->ReqBiosAction));

  return Status;
}

/**
  Send IAFW DNX request clear message throught HECI1.

  @param[in] Flag    Flag to decide which type clear operation need be done.

  @return EFI_SUCCESS   Clear request success.
  @return Others              Clear request failed.
**/
EFI_STATUS
HeciIafwDnxReqClear(
  IN UINT32         Flag
  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength;
  IAFW_DNX_REQ_CLEAR_REQ_DATA     *Req;
  IAFW_DNX_REQ_CLEAR_RESP_DATA    *Resp;
  UINT32                          SeCMode;
  UINT32                          DataBuffer[0x70];

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciIafwDnxReqClear\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || ((SeCMode != SEC_MODE_NORMAL)&&(SEC_MODE_RECOVER !=SeCMode))) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  Req = (IAFW_DNX_REQ_CLEAR_REQ_DATA*)DataBuffer;
  Req->MKHIHeader.Fields.GroupId = MKHI_GEN_DNX_GROUP_ID;
  Req->MKHIHeader.Fields.Command = IAFW_DNX_REQ_CLEAR;
  Req->Flag = Flag;

  DEBUG ((EFI_D_INFO, "IAFW_DNX_REQ_CLEAR_REQ_DATA size is %x\n", sizeof(IAFW_DNX_REQ_CLEAR_REQ_DATA)));
  HeciSendLength              = sizeof(IAFW_DNX_REQ_CLEAR_REQ_DATA);
  HeciRecvLength              = sizeof(DataBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  Resp = (IAFW_DNX_REQ_CLEAR_RESP_DATA*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group    =%08x\n", Resp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command  =%08x\n", Resp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone=%08x\n", Resp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result   =%08x\n", Resp->MKHIHeader.Fields.Result));

  return Status;
}

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.
  FWInitComplete value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] RuleData            MEFWCAPS_SKU message

  @exception EFI_UNSUPPORTED      Current ME mode doesn't support this function
**/
EFI_STATUS
HeciGetFwFeatureStateMsgII (
  OUT MEFWCAPS_SKU                *RuleData
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  GEN_GET_FW_FEATURE_STATUS_ACK   GetFwFeatureStatus;
  UINT32                          SeCMode;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetFwFeatureStateMsgII\n"));

  Status = HeciGetSeCMode (HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }


  GetFwFeatureStatus.MKHIHeader.Data              = 0;
  GetFwFeatureStatus.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  GetFwFeatureStatus.MKHIHeader.Fields.Command    = FWCAPS_GET_RULE_CMD;
  GetFwFeatureStatus.MKHIHeader.Fields.IsResponse = 0;
  GetFwFeatureStatus.RuleId                       = 0x20;
  Length                                          = sizeof (GEN_GET_FW_FEATURE_STATUS);
  RecvLength                                      = sizeof (GEN_GET_FW_FEATURE_STATUS_ACK);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             (UINT32*)&GetFwFeatureStatus,
             Length,
             &RecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );


  if (!EFI_ERROR (Status)) {
    RuleData->Data = GetFwFeatureStatus.RuleData.Data;
    DEBUG ((EFI_D_INFO, "BIOS Get Feature Message: %08x\n", RuleData->Data = GetFwFeatureStatus.RuleData.Data));
  } else {
    DEBUG ((EFI_D_ERROR, "BIOS Get Feature Message ERROR: %08x\n", Status));
  }

  return Status;
}


/**
  Sends the MKHI Enable/Disable manageability message.
  The message will only work if bit 2 in the bitmasks is toggled.
  To enable manageability:
    EnableState = 0x00000004, and
    DisableState = 0x00000000.
  To disable manageability:
    EnableState = 0x00000000, and
    DisableState = 0x00000004

  @param[in] EnableState          Enable Bit Mask
  @param[in] DisableState         Disable Bit Mask

  @exception EFI_UNSUPPORTED      Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciFwFeatureStateOverride (
  IN UINT32                       EnableState,
  IN UINT32                       DisableState
  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciLength;
  UINT32                          HeciRecvLength;
  UINT32                          SeCMode;
  FIRMWARE_CAPABILITY_OVERRIDE    MngStateCmd;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciFwFeatureStateOverride\n"));

  Status = HeciGetSeCMode (HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }



  MngStateCmd.MKHIHeader.Data               = 0;
  MngStateCmd.MKHIHeader.Fields.Command     = FIRMWARE_CAPABILITY_OVERRIDE_CMD;
  MngStateCmd.MKHIHeader.Fields.IsResponse  = 0;
  MngStateCmd.MKHIHeader.Fields.GroupId     = MKHI_GEN_GROUP_ID;
  MngStateCmd.MKHIHeader.Fields.Reserved    = 0;
  MngStateCmd.MKHIHeader.Fields.Result      = 0;
  MngStateCmd.FeatureState.EnableFeature    = EnableState;
  MngStateCmd.FeatureState.DisableFeature   = DisableState;
  HeciLength                                = sizeof (FIRMWARE_CAPABILITY_OVERRIDE);
  HeciRecvLength                            = sizeof (FIRMWARE_CAPABILITY_OVERRIDE_ACK);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             (UINT32*)&MngStateCmd,
             HeciLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  DEBUG ((EFI_D_INFO, "BIOS HeciFwFeatureStateOverride Message: %x\n", Status));
  return Status;
}

/**
Send IFWI PREPARE FOR UPDATE(IPFU) Command through HECI1.
This command provide necessary synchronization between HOST & CSE when
BIOS Performance IFWI Update process.

@return EFI_SUCCESS         Send IFWI Prepare For Update command succeeded
@return Others              Send IFWI Prepare For Update command failed.
**/
EFI_STATUS
HeciIfwiPrepareForUpdate(
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT32                      HeciSendLength;
  UINT32                      HeciRecvLength;
  IFWI_PREPARE_FOR_UPDATE     *SendDC;
  IFWI_PREPARE_FOR_UPDATE_ACK *DCResp;
  UINT32                      SeCMode;
  UINT32                      DataBuffer[0x2];
  UINT8                       Flags;
  UINT8                       Result;
  UINT8                       TimeOut;
  UINT8                       Count;
  BOOLEAN                     Reset_Status;


  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciIfwiPrepareForUpdate\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));
  DEBUG ((EFI_D_ERROR, "IFWI Prepare For Update: In Progress...\n"));

  Flags = IPFU_TRY_AGAIN;
  Result  = 0;
  TimeOut = 0;

  //
  // Keep in while loop until Successful or Error or timeout of 1 min duration
  //
  while ((Flags == IPFU_TRY_AGAIN) && (TimeOut <= IPFU_TIMEOUT)) {

    SetMem(DataBuffer, sizeof(DataBuffer), 0);
    SendDC = (IFWI_PREPARE_FOR_UPDATE*)DataBuffer;
    SendDC->MKHIHeader.Fields.GroupId = MKHI_IFWI_UPDATE_GROUP_ID;
    SendDC->MKHIHeader.Fields.Command = IFWI_PREPARE_FOR_UPDATE_CMD_ID;
    SendDC->ResetType = 0x01;

    HeciSendLength = sizeof(IFWI_PREPARE_FOR_UPDATE);
    HeciRecvLength = sizeof(DataBuffer);

    Status = HeciSendwACK(
               HECI1_DEVICE,
               DataBuffer,
               HeciSendLength,
               &HeciRecvLength,
               BIOS_FIXED_HOST_ADDR,
               HECI_CORE_MESSAGE_ADDR
               );
    // When CSE is in reset, Status may fail, so ignore checking of Status
    DCResp = (IFWI_PREPARE_FOR_UPDATE_ACK*)DataBuffer;

    DEBUG ((EFI_D_INFO, "Group            = %08x\n", DCResp->MKHIHeader.Fields.GroupId));
    DEBUG ((EFI_D_INFO, "Command          = %08x\n", DCResp->MKHIHeader.Fields.Command));
    DEBUG ((EFI_D_INFO, "IsRespone        = %08x\n", DCResp->MKHIHeader.Fields.IsResponse));
    DEBUG ((EFI_D_INFO, "Result           = %08x\n", DCResp->MKHIHeader.Fields.Result));
    DEBUG ((EFI_D_INFO, "Flags value = %08x\n", DCResp->Flags));

    Result = (UINT8)DCResp->MKHIHeader.Fields.Result;
    Flags = (UINT8)DCResp->Flags;

    //
    // Result = 0x00 - OK
    //          0x05 - Size error
    //          0x89 - STATUS_NOT_SUPPORTED
    //          0x8D - STATUS_INVALID_COMMAND
    //          0xFC - STATUS_PROHIBITED
    //          0xFE - STATUS_AUDIT_LOG_ERROR
    //          0xFF - STATUS_UNDEFINED
    // If Result is OK then check for Flags
    //
    if (Result == EFI_SUCCESS) {
      if (Flags != IPFU_SUCCESS) {
        Count = 0;
        Reset_Status = FALSE;
        //
        // Check for CSE Reset and then issue HECI Reset.
        // Normally reset occurs immediately after giving cmmand to CSE, so this time can be ignored.
        // The 1 min time out is mainly after reset and CSE to get ready
        //
        while ((Reset_Status == FALSE) && (Count <= 60)) {   // Try checking for 1 min and exit
          Status = CheckCseResetAndIssueHeciReset(&Reset_Status);
          if (Reset_Status == FALSE) {
            MicroSecondDelay(1000 * 1000);
            Count++;
          }
        }
        if (Count > 60){
          DEBUG ((EFI_D_ERROR, "CSE Reset or HECI reset not occurred to send Command again \n"));
          return EFI_TIMEOUT;
        }

        TimeOut += Count;   // Add time delay counted for reset of CSE too.
        MicroSecondDelay(1000 * 1000);
        TimeOut++;
      }
    } else {
      DEBUG ((EFI_D_ERROR, "IFWI Prepare For Update - Failed! Result: 0x%x \n", Result));
      return Result;
    }

  }  // End of While Loop

  if (Flags == IPFU_SUCCESS) {
    DEBUG ((EFI_D_INFO, "IFWI Prepare For Update - SUCCESS! Result = 0x%x, Flags = 0x%x \n", Result, Flags));
    return EFI_SUCCESS;
  } else {
    if (TimeOut > IPFU_TIMEOUT) {
      DEBUG ((EFI_D_ERROR, "IFWI Prepare For Update - TIME OUT! \n"));
      return EFI_TIMEOUT;
    }
    DEBUG ((EFI_D_ERROR, "IFWI Prepare For Update - Flag ERROR! Result = 0x%x, Flags = 0x%x \n", Result, Flags));
    return EFI_PROTOCOL_ERROR;
  }
}

/**
  Send RPMB/Device Extention region Data Clear message through HECI1.
  Note: This command should be send only after successful execution of
  IFWI_PREPARE_FOR_UPDATE Command

  @return EFI_SUCCESS         Send DataClear message success.
  @return Others              Send DataClear message failed.
**/
EFI_STATUS
HeciDataClear (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT32                      HeciSendLength;
  UINT32                      HeciRecvLength;
  IFWI_UPDATE_DATA_CLEAR      *SendDC;
  IFWI_UPDATE_DATA_CLEAR_ACK  *DCResp;
  UINT32                      SeCMode;
  UINT32                      DataBuffer[0x4];
  UINT8                       Result = 0xFF; // Status Undefined

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: DataClear\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || ((SeCMode != SEC_MODE_NORMAL) && (SeCMode != SEC_MODE_RECOVER))) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful \n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendDC = (IFWI_UPDATE_DATA_CLEAR*)DataBuffer;
  SendDC->MKHIHeader.Fields.GroupId = MKHI_IFWI_UPDATE_GROUP_ID;
  SendDC->MKHIHeader.Fields.Command = DATA_CLEAR_CMD_ID;

  DEBUG ((EFI_D_INFO, "IFWI_UPDATE_DATA_CLEAR size is %x\n", sizeof(IFWI_UPDATE_DATA_CLEAR)));
  HeciSendLength = sizeof(IFWI_UPDATE_DATA_CLEAR);
  HeciRecvLength              = sizeof(DataBuffer);

  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  DCResp = (IFWI_UPDATE_DATA_CLEAR_ACK*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group            = %08x\n", DCResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command          = %08x\n", DCResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone        = %08x\n", DCResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result           = %08x\n", DCResp->MKHIHeader.Fields.Result));
  Result = (UINT8)DCResp->MKHIHeader.Fields.Result;

  if (Result == 0) {
    DEBUG ((EFI_D_INFO, "Data Clear SUCCESS! \n"));
    Status = EFI_SUCCESS;
  } else {
    DEBUG ((EFI_D_INFO, "Data Clear FAILED! Result = %08x\n", Result));
    Status = Result;
  }
  return Status;
}

/**
Send 'Update Image Check' command to CSE to verify IFWI Image and confirm
that appropriate for FW Update. (This command will be sending as part of Capsule Update)

@param[in] ImageBaseAddr  FW Image Base address. 64 Bit Wide
@param[in] ImageSize      FW Image Size
@param[in] *HeciResponse  To capture Response Info (Optional)
@param[in] ResponseSize   Size of Response (Optional)

@return EFI_SUCCESS       Image was checked (best effort) and verified to be appropriate for FW update.
@return 0x01               IMAGE_FAILED.
@return 0x02               IMG_SIZE_INVALID.
@return 0x05               SIZE_ERROR
@return 0x89               STATUS_NOT_SUPPORTED.
@return 0x8D               STATUS_INVALID_COMMAND.
@return 0xFF               STATUS_UNDEFINED.
**/
EFI_STATUS
HeciUpdateImageCheck(
  IN UINT64        ImageBaseAddr,
  IN UINT32        ImageSize,
  IN OUT UINT32    *HeciResponse,
  IN UINT32        ResponseSize
  )
{
  EFI_STATUS                  Status;
  UINT32                      HeciSendLength;
  UINT32                      HeciRecvLength;
  IFWI_UPDATE_IMAGE_CHECK     *SendRequest;
  IFWI_UPDATE_IMAGE_CHECK_ACK *ReceiveResp;
  UINT32                      SeCMode;
  UINT32                      DataBuffer[0x7];
  UINT8                       Result = 0xFF;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciUpdateImageCheck\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || ((SeCMode != SEC_MODE_NORMAL) && (SeCMode != SEC_MODE_RECOVER))) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful \n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendRequest = (IFWI_UPDATE_IMAGE_CHECK*)DataBuffer;
  SendRequest->MKHIHeader.Fields.GroupId = MKHI_IFWI_UPDATE_GROUP_ID;
  SendRequest->MKHIHeader.Fields.Command = UPDATE_IMAGE_CHECK_CMD_ID;
  SendRequest->ImageBaseAddrLower32b = (UINT32)ImageBaseAddr;
  SendRequest->ImageBaseAddrUpper32b = (UINT32)(ImageBaseAddr >> 32);
  SendRequest->ImageSize = ImageSize;

  DEBUG ((EFI_D_INFO, "IFWI_UPDATE_IMAGE_CHECK size is %x\n", sizeof(IFWI_UPDATE_IMAGE_CHECK)));
  HeciSendLength = sizeof(IFWI_UPDATE_IMAGE_CHECK);
  HeciRecvLength = sizeof(DataBuffer);

  Status = HeciSendwACK(
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  ReceiveResp = (IFWI_UPDATE_IMAGE_CHECK_ACK*)DataBuffer;
  DEBUG ((EFI_D_INFO, "Group            = %08x\n", ReceiveResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command          = %08x\n", ReceiveResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone        = %08x\n", ReceiveResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result           = %08x\n", ReceiveResp->MKHIHeader.Fields.Result));

  //
  // Copy Response information if response pointer is available
  //
  if ((HeciResponse != NULL) && (ResponseSize != 0)) {
    if (HeciRecvLength < ResponseSize) {
      ResponseSize = HeciRecvLength;
    }
    CopyMem(HeciResponse, DataBuffer, (UINTN) ResponseSize);
  }

  Result = (UINT8)ReceiveResp->MKHIHeader.Fields.Result;
  if (Result == 0) {
    DEBUG ((EFI_D_INFO, "Update Image Check SUCCESS! \n"));
    Status = EFI_SUCCESS;
  } else {
    DEBUG ((EFI_D_INFO, "Update Image Check FAILED! Result = %08x\n", Result));
    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  Checks Platform Trust Technology enablement state.

  @param[out] IsPttEnabledState  TRUE if PTT is enabled, FALSE othewrwise.

  @retval EFI_SUCCESS            Command succeeded
  @retval EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
**/
EFI_STATUS
EFIAPI
PttHeciGetState (
  OUT BOOLEAN   *IsPttEnabledState
  )
{
  EFI_STATUS      Status;
  MEFWCAPS_SKU    CurrentFeatures;

  *IsPttEnabledState = FALSE;

  Status = HeciGetFwFeatureStateMsgII (&CurrentFeatures);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((EFI_D_INFO, "SkuMgr Data = 0x%X\n", CurrentFeatures.Data));

  if (CurrentFeatures.Fields.PTT) {
    *IsPttEnabledState = TRUE;
  }

  DEBUG ((EFI_D_INFO, "PTT SkuMgr: PttState = %d\n", *IsPttEnabledState));

  return EFI_SUCCESS;
}

/**
  Changes current Platform Trust Technology state.

  @param[in] PttEnabledState     TRUE to enable, FALSE to disable.

  @retval EFI_SUCCESS            Command succeeded
  @retval EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
**/
EFI_STATUS
EFIAPI
PttHeciSetState (
  IN  BOOLEAN   PttEnabledState
  )
{
  EFI_STATUS      Status;
  UINT32          EnableBitmap;
  UINT32          DisableBitmap;

  if (PttEnabledState) {
    //
    // Enable PTT
    //
    DEBUG ((EFI_D_INFO, "PTT SkuMgr: Enable PTT\n"));
    EnableBitmap  = PTT_BITMASK;
    DisableBitmap = CLEAR_PTT_BIT;
  } else {
    //
    // Disable PTT
    //
    DEBUG ((EFI_D_INFO, "PTT SkuMgr: Disable PTT\n"));
    EnableBitmap  = CLEAR_PTT_BIT;
    DisableBitmap = PTT_BITMASK;
  }

  Status = HeciFwFeatureStateOverride (EnableBitmap, DisableBitmap);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Send Reset Request Message through HECI.

  @param[in] ResetOrigin          Reset source
  @param[in] ResetType            Global or Host reset

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendResetRequest (
  IN  UINT8                             ResetOrigin,
  IN  UINT8                             ResetType
  )
{
  EFI_STATUS                Status;
  UINT32                    HeciLength;
  CBM_RESET_REQ             CbmResetRequest;
  UINT32                    HeciRecvLength;
  UINT32                    SeCMode;

  CbmResetRequest.MKHIHeader.Data               = 0;
  CbmResetRequest.MKHIHeader.Fields.Command     = CBM_RESET_REQ_CMD;
  CbmResetRequest.MKHIHeader.Fields.IsResponse  = 0;
  CbmResetRequest.MKHIHeader.Fields.GroupId     = MKHI_CBM_GROUP_ID;
  CbmResetRequest.MKHIHeader.Fields.Reserved    = 0;
  CbmResetRequest.MKHIHeader.Fields.Result      = 0;
  CbmResetRequest.Data.RequestOrigin            = ResetOrigin;
  CbmResetRequest.Data.ResetType                = ResetType;

  HeciLength = sizeof (CBM_RESET_REQ);
  HeciRecvLength = sizeof(CBM_RESET_REQ);

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  Status = HeciSendwoACK(
             (UINT32 *) &CbmResetRequest,
             HeciLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Send Reset Request - %r\n", Status));
  }

  return Status;
}

/**
Wait for CSE reset bit set and then Reset HECI interface.

@param[out] Reset_Status        Reset Status value

@retval EFI_SUCCESS             Command succeeded
@retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/

EFI_STATUS
CheckCseResetAndIssueHeciReset (
  OUT BOOLEAN *Reset_Status
  )
{
  EFI_STATUS                           Status;
  volatile HECI_SEC_CONTROL_REGISTER   *SecControlReg;
  UINTN                                HeciMbar;

  HeciMbar = CheckAndFixHeciForAccess(HECI1_DEVICE);
  if (HeciMbar == 0) {
    return EFI_DEVICE_ERROR;
  }

  *Reset_Status = FALSE;
  SecControlReg = (volatile HECI_SEC_CONTROL_REGISTER  *)(UINTN)(HeciMbar + SEC_CSR_HA);
  if (SecControlReg->r.SEC_RST_HRA == 1) {
    DEBUG ((EFI_D_INFO, "CheckCseResetAndIssueHeciReset, SecControlReg: %08x\n", SecControlReg->ul));
    *Reset_Status = TRUE;
    // Reset HECI Interface
    Status = ResetHeciInterface(HECI1_DEVICE);
    if (EFI_ERROR(Status)) {
      *Reset_Status = FALSE;
    } else {
      DEBUG ((EFI_D_INFO, "CheckCseResetAndIssueHeciReset, HECI Reset success\n"));
    }
  }

  DEBUG ((EFI_D_INFO, "CheckCseResetAndIssueHeciReset, SecControlReg: %08x\n", SecControlReg->ul));

  return EFI_SUCCESS;
}


VOID
EFIAPI
HeciCoreBiosDoneMsg (
  VOID
  )
{
  EFI_STATUS                                Status;
  UINT32                                    Length;
  UINT32                                    RespLength;
  CORE_BIOS_DONE                            CoreBiosDone;
  UINT32                                    SeCMode;
  Length = sizeof (CORE_BIOS_DONE);
  RespLength = sizeof (CORE_BIOS_DONE_ACK);
  Status = HeciGetSeCMode (HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return;
  }

  DEBUG ((EFI_D_INFO, "Sending the HeciCoreBiosDoneMsg - Start \n"));
  CoreBiosDone.MkhiHeader.Data              = 0;
  CoreBiosDone.MkhiHeader.Fields.GroupId    = HECI_MKHI_MCA_GROUP_ID;
  CoreBiosDone.MkhiHeader.Fields.Command    = HECI_MCA_CORE_BIOS_DONE_CMD;
  CoreBiosDone.MkhiHeader.Fields.IsResponse = 0;
  Status = HeciSendwACK(
             HECI1_DEVICE,
             (UINT32 *) &CoreBiosDone,
             Length,
             &RespLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  DEBUG ((EFI_D_INFO, "Sending the HeciCoreBiosDoneMsg - done \n"));
  return;
}

VOID
EFIAPI
HeciDXECallback (
  IN EFI_EVENT     Event,
  IN VOID          *ParentImageHandle
  )
{
//[-start-160808-IB07220123-add]//
    if (mReadyToBootEventSignaled) {
      return;
    }
    mReadyToBootEventSignaled = TRUE;
//[-end-160808-IB07220123-add]//
  HeciDataClearLock();
  HeciCoreBiosDoneMsg();
//[-start-170422-IB07400874-add]//
  PcdSet8 (PcdHeci1NvInterfaceLocked, 1);
//[-end-170422-IB07400874-add]//
}


/**
  Get SMM Trusted Key.

  @param[out] SmmTrustedKeyData       Smm trusted data buffer.

  @return EFI_SUCCESS   Get SMM Trusted success.
  @return Others              Get SMM Trusted failed.
**/

EFI_STATUS
HeciGetSMMTrustedKey (
  OUT MBP_SMM_TRUSTED_KEY    *SmmTrustedKeyData
  )
{
  MBP_CMD_RESP_DATA          *MBPHeader;
  MBP_ITEM_HEADER            *MBPItem;
  MBP_SMM_TRUSTED_KEY        *SmmTrustedKey = NULL;
  UINT32                     MBPItemCount;
  EFI_HOB_GUID_TYPE          *MbpSensitiveDataHob;
  BOOLEAN                    MbpSensitiveDataHobInstances;

  if (SmmTrustedKeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  DEBUG ((EFI_D_INFO, "Get SMMTrustedKey from HOB\n"));
  MbpSensitiveDataHob = GetFirstGuidHob (&gMeBiosPayloadSensitiveDataHobGuid);
  if (MbpSensitiveDataHob == NULL) {
    DEBUG ((DEBUG_ERROR, "Sensitive HeciGetMBPData HOB not found\n"));
    return EFI_NOT_FOUND;
  }
  MbpSensitiveDataHobInstances = FALSE;
  while (MbpSensitiveDataHob != NULL) {
    MBPHeader = (MBP_CMD_RESP_DATA *) ((UINT32*) GET_GUID_HOB_DATA (MbpSensitiveDataHob));
    MBPItem = (MBP_ITEM_HEADER *) (MBPHeader + 1);
    for (MBPItemCount = 0; MBPItemCount <= MBPHeader->ItemsNum; MBPItemCount++) {
      if (MBPItem->AppID == MBP_APP_ID_NVM && MBPItem->ItemID == MBP_ITEM_ID_SMM_TRUSTED_CHANNEL) {
        SmmTrustedKey = (MBP_SMM_TRUSTED_KEY *) (MBPItem + 1);
        if (MbpSensitiveDataHobInstances == FALSE) {
          CopyMem ((VOID *) SmmTrustedKeyData, (VOID *) SmmTrustedKey, sizeof (MBP_SMM_TRUSTED_KEY));
          MbpSensitiveDataHobInstances = TRUE;
        }
        ZeroMem ((VOID *) (SmmTrustedKey), sizeof (MBP_SMM_TRUSTED_KEY));
      }
    MBPItem = (MBP_ITEM_HEADER *) ((UINT32 *) MBPItem + MBPItem->Length);
    }
    MbpSensitiveDataHob = GET_NEXT_HOB (MbpSensitiveDataHob);
    MbpSensitiveDataHob = GetNextGuidHob (&gMeBiosPayloadSensitiveDataHobGuid, MbpSensitiveDataHob);
  }
  ASSERT (MbpSensitiveDataHobInstances == TRUE);
  return EFI_SUCCESS;
}

/**
Get ARB Status
Send 'Get ARB Status' command to CSE to get ARB Status information in the received buffer.
Host Calls this API on boot after ifwi update.

@param[out] GetArbStatus  Structure filled with ARB status info

@return EFI_SUCCESS       SVN info populated correctly. Refer ARB Out put structure for info
@return other values      Refer BIOS CSE Interface document
**/
EFI_STATUS
HeciGetArbStatus(
  IN OUT GET_ARB_STATUS_ACK   *GetArbStatus
  )
{
  EFI_STATUS                  Status;
  UINT32                      HeciSendLength;
  UINT32                      HeciRecvLength;
  GET_ARB_STATUS              *SendRequest;
  GET_ARB_STATUS_ACK          *ReceiveResp;
  UINT32                      SeCMode;
  UINT32                      DataBuffer[sizeof(GET_ARB_STATUS_ACK)];
  UINT8                       Result = 0xFF;

  DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetArbStatus\n"));
  if (GetArbStatus == NULL) {
    DEBUG ((EFI_D_INFO, "Invalid Parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (SeCMode != SEC_MODE_NORMAL)) {
      DEBUG ((EFI_D_INFO, "HeciGetArbStatus-EFI_NOT_READY\n"));
      return EFI_NOT_READY;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful \n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendRequest = (GET_ARB_STATUS*)DataBuffer;
  SendRequest->MKHIHeader.Fields.GroupId = MKHI_SECURE_BOOT_GROUP_ID;
  SendRequest->MKHIHeader.Fields.Command = GET_ARB_STATUS_CMD_ID;

  HeciSendLength = sizeof(GET_ARB_STATUS);
  HeciRecvLength = sizeof(GET_ARB_STATUS_ACK);

  Status = HeciSendwACK(
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  ReceiveResp = (GET_ARB_STATUS_ACK*)DataBuffer;
  DEBUG ((EFI_D_INFO, "Group            = %08x\n", ReceiveResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command          = %08x\n", ReceiveResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone        = %08x\n", ReceiveResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result           = %08x\n", ReceiveResp->MKHIHeader.Fields.Result));

  //
  // Copy Response information
  //
  CopyMem(GetArbStatus, DataBuffer, (UINTN)HeciRecvLength);

  Result = (UINT8)ReceiveResp->MKHIHeader.Fields.Result;
  if (Result == 0) {
    DEBUG ((EFI_D_INFO, "HeciGetArbStatus SUCCESS! \n"));
    Status = EFI_SUCCESS;
  } else {
    DEBUG ((EFI_D_INFO, "HeciGetArbStatus FAILED! Result = %08x\n", Result));
    Status = Result;
  }

  return Status;
}

/**
Commit ARB SVN Updates
Send 'Commit ARB SVN Updates' command to CSE to commit ARB SVN information.
Host Calls this API on boot after ifwi update.

@param[in] CommitSvns    Structure filled with ARB SVN Commit info

@return EFI_SUCCESS       SVN info populated correctly. Refer ARB Out put structure for info
@return other values      Refer BIOS CSE Interface document
**/
EFI_STATUS
HeciCommitArbSvnUpdates(
  IN UINT8     *CommitArbSvns
  )
{
  EFI_STATUS                  Status;
  UINT32                      HeciSendLength;
  UINT32                      HeciRecvLength;
  COMMIT_ARB_SVN_UPDATES      *SendRequest;
  COMMIT_ARB_SVN_UPDATES_ACK  *ReceiveResp;
  UINT32                      SeCMode;
  UINT8                       DataBuffer[sizeof(COMMIT_ARB_SVN_UPDATES)];

  DEBUG ((EFI_D_INFO, "\nBIOS Start Send HECI Message: HeciCommitArbSvnUpdates\n"));
  if (CommitArbSvns == NULL) {
    DEBUG ((EFI_D_INFO, "Invalid Parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (SeCMode != SEC_MODE_NORMAL)) {
    if(SeCMode == SEC_OPERATION_MODE_IN_FWUPDATE_PROGRESS){
      return EFI_NOT_READY;
    } else {
      return EFI_UNSUPPORTED;
    }
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful \n"));

  SetMem(DataBuffer, sizeof(DataBuffer), 0);

  SendRequest = (COMMIT_ARB_SVN_UPDATES*)DataBuffer;
  SendRequest->MKHIHeader.Fields.GroupId = MKHI_SECURE_BOOT_GROUP_ID;
  SendRequest->MKHIHeader.Fields.Command = COMMIT_ARB_SVN_UPDATES_CMD_ID;
  CopyMem(SendRequest->CommitSvns, CommitArbSvns, sizeof( SendRequest->CommitSvns));  // 16 Bytes = Size of SVN to commit as per BIOS - CSE Ref doc

  HeciSendLength = sizeof(COMMIT_ARB_SVN_UPDATES);
  HeciRecvLength = sizeof(COMMIT_ARB_SVN_UPDATES_ACK);

  Status = HeciSendwACK(
             HECI1_DEVICE,
             (UINT32*)DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );

  ReceiveResp = (COMMIT_ARB_SVN_UPDATES_ACK*)DataBuffer;
  DEBUG ((EFI_D_INFO, "Group            = %08x\n", ReceiveResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command          = %08x\n", ReceiveResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone        = %08x\n", ReceiveResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result           = %08x\n", ReceiveResp->MKHIHeader.Fields.Result));

  Status =(UINT8)ReceiveResp->MKHIHeader.Fields.Result;;
  if (Status != EFI_SUCCESS) {
    DEBUG ((EFI_D_INFO, "HeciCommitArbSvnUpdates FAILED! Result = %08x\n", Status));
  } else {
    DEBUG ((EFI_D_INFO, "HeciCommitArbSvnUpdates SUCCESS -- Result = %08x\n", Status));
  }

  return Status;
}

/**
Write Data to NVM file for ISH through HECI1.

@param[in] FileName   The file name.
@param[in] Offset        The offset of data.
@param[in] Data         The data content.
@param[in] DataSize   Data's size.

@return EFI_SUCCESS   Write NVM file success.
@return Others        Write NVM file failed.
**/
EFI_STATUS
HeciWriteIshNVMFile(
  IN  ISH_SRV_HECI_SET_FILE_REQUEST *Ish2CseData
  )
{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength = 0;
  ISH_SRV_HECI_STATUS_REPLY       ReceiveData;
  UINT32                          SeCMode;
  UINT32                          Index = 0;
  UINT32                          *TempPtr = NULL;

  DEBUG ((EFI_D_INFO, "HeciWriteIshNVMFile Entry\n"));
  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  if (Ish2CseData == NULL) {
    DEBUG ((EFI_D_ERROR, "Invalid Parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((EFI_D_INFO, "Dump Bios2ish buffer\n"));
  TempPtr = (UINT32 *)Ish2CseData;
  for (Index = 0; Index < 10; Index++ ){
    DEBUG ((EFI_D_INFO, "%x\n", *TempPtr++));
  }

  HeciSendLength = Ish2CseData->Header.Length + 4; // Including command and msglength

  Status = HeciIshSendwAck(
             HECI1_DEVICE,
             (VOID *) Ish2CseData,
             (VOID *) &ReceiveData,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x03
             );

  DEBUG ((EFI_D_INFO, "Command    =%08x\n", ReceiveData.Header.Command));
  DEBUG ((EFI_D_INFO, "Length    =%08x\n", ReceiveData.Header.Length));
  DEBUG ((EFI_D_INFO, "Status    =%08x\n", ReceiveData.Status));

  if (ReceiveData.Status != 0) {
    DEBUG ((EFI_D_ERROR, "HeciWriteIshNVMFile Failed.\n"));
    Status = EFI_DEVICE_ERROR;
  } else {
    Status = EFI_SUCCESS;
  }

  return Status;
}

//[-start-190321-IB07401094-add]//
/**
  Get Bootloader Seed List

  @param[out] BLSList      Output CSE Seed List data

  @return EFI_SUCCESS   Get seed success.
  @return Others        Get seed failed.
**/
EFI_STATUS
HeciGetBootloaderSeedList (
  IN  BUP_BOOTLOADER_SEED_LIST_INFO *BLSList
  )
{
  EFI_STATUS                Status;
  UINT32                    HeciSendLength;
  UINT32                    HeciRecvLength;
  BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Request             *MsgGetSeedListReq;
  BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Response            *MsgGetSeedListResp;
  UINT32                    SeCMode;
  UINT32                    DataBuffer[sizeof(BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Response)];
//[-start-190323-IB07401095-add]//
  UINT32                    result;
//[-end-190323-IB07401095-add]//

  //DEBUG ((EFI_D_INFO, "BIOS Start Send HECI Message: HeciGetBootloaderSeedList\n"));

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  DEBUG ((EFI_D_INFO, "GetSeCMode successful\n"));

  //
  // Allocate HeciGetBootloaderSeedList data structure
  //
  SetMem(DataBuffer, sizeof(DataBuffer), 0x0);

  MsgGetSeedListReq = (BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Request*)DataBuffer;
  MsgGetSeedListReq->MKHIHeader.Fields.GroupId = 0xF0;
  MsgGetSeedListReq->MKHIHeader.Fields.Command = 0x17;
  MsgGetSeedListReq->MKHIHeader.Fields.IsResponse = 0;

  HeciSendLength              = sizeof(BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Request);
  HeciRecvLength              = sizeof(DataBuffer);

  //
  // Send Get Boot Loader Seed List Request to CSE
  //
  Status = HeciSendwACK (
             HECI1_DEVICE,
             DataBuffer,
             HeciSendLength,
             &HeciRecvLength,
             BIOS_FIXED_HOST_ADDR,
             0x7
             );

  MsgGetSeedListResp = (BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Response*)DataBuffer;

  DEBUG ((EFI_D_INFO, "Group      =%08x\n", MsgGetSeedListResp->MKHIHeader.Fields.GroupId));
  DEBUG ((EFI_D_INFO, "Command    =%08x\n", MsgGetSeedListResp->MKHIHeader.Fields.Command));
  DEBUG ((EFI_D_INFO, "IsRespone  =%08x\n", MsgGetSeedListResp->MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_INFO, "Result     =%08x\n", MsgGetSeedListResp->MKHIHeader.Fields.Result));

//[-start-190323-IB07401095-modify]//
  result = MsgGetSeedListResp->MKHIHeader.Fields.Result;
//[-end-190323-IB07401095-modify]//

  if (result==0){
    CopyMem(BLSList, &(MsgGetSeedListResp->NumOfSeeds), sizeof(BUP_BOOTLOADER_SEED_LIST_INFO));

    // Do cleanup of memory which contains sensitive data
    ZeroMem(DataBuffer, sizeof(DataBuffer));
  }
  else {
    //DEBUG ((EFI_D_ERROR, "SeedList Retrieval FAILED! result     =%08x\n", result));
  }
  return Status;
}
//[-end-190321-IB07401094-add]//

