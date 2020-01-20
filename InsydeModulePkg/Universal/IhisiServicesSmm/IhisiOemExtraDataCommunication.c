/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "IhisiOemExtraDataCommunication.h"
#include <IndustryStandard/Oa3_0.h>
#include <Library/SmmChipsetSvcLib.h>
#include <Library/BaseOemSvcKernelLib.h>
#include <Library/SmmOemSvcKernelLib.h>
#include <Library/FlashRegionLib.h>

STATIC UINT8     mOemExtraDataType;
STATIC UINTN     mRomBaseAddress;
STATIC BOOLEAN   mEcIdle = FALSE;
STATIC AP_COMMUNICATION_DATA_TABLE      mApCommDataBuffer;

STATIC
IHISI_REGISTER_TABLE
OEM_EXT_COMMON_REGISTER_TABLE[] = {
  //
  // AH=41h
  //
  { OEMSFOEMExCommunication, "S41Kn_CommuSaveRegs", KernelCommunicationSaveRegs             }, \
  { OEMSFOEMExCommunication, "S41Cs_ExtDataCommun", ChipsetOemExtraDataCommunication        }, \
  { OEMSFOEMExCommunication, "S41OemT01Vbios00000", OemIhisiS41T1Vbios                      }, \
  { OEMSFOEMExCommunication, "S41OemT54LogoUpdate", OemIhisiS41T54LogoUpdate                }, \
  { OEMSFOEMExCommunication, "S41OemT55CheckSignB", OemIhisiS41T55CheckBiosSignBySystemBios }, \
  { OEMSFOEMExCommunication, "S41OemReservedFun00", OemIhisiS41ReservedFunction             }, \
  { OEMSFOEMExCommunication, "S41Kn_T51EcIdelTrue", KernelT51EcIdelTrue                     }, \
  { OEMSFOEMExCommunication, "S41Kn_ExtDataCommun", KernelOemExtraDataCommunication         }, \
  { OEMSFOEMExCommunication, "S41Kn_T51EcIdelFals", KernelT51EcIdelFalse                    }, \
  { OEMSFOEMExCommunication, "S41OemT50Oa30RWFun0", OemIhisiS41T50a30ReadWrite              }, \

  //
  // AH=42h
  //
  { OEMSFOEMExDataWrite, "S42Cs_ExtDataWrite0", ChipsetOemExtraDataWrite,         }, \
  { OEMSFOEMExDataWrite, "S42Kn_T50EcIdelTrue", KernelT50EcIdelTrue,              }, \
  { OEMSFOEMExDataWrite, "S42Kn_ExtDataWrite0", KernelOemExtraDataWrite,          }, \
  { OEMSFOEMExDataWrite, "S42Kn_T50EcIdelFals", KernelT50EcIdelFalse,             }, \
  { OEMSFOEMExDataWrite, "S42Cs_DShutdownMode", ChipsetOemExtraDataDosShutdownMode}, \

  //
  // AH=47h
  //
  { OEMSFOEMExDataRead, "S47Cs_ExtDataRead00", ChipsetOemExtraDataRead}, \
  { OEMSFOEMExDataRead, "S47Kn_ExtDataRead00", KernelOemExtraDataRead }
};

STATIC
BOOLEAN
MsdmExist (
  )
{
  EFI_STATUS                       Status;
  UINTN                            DataSize;
  BOOLEAN                          MsdmExist;
  UINTN                            RomBaseAddress;
  EFI_ACPI_MSDM_DATA_STRUCTURE     *MsdmData;

  MsdmData = NULL;
  MsdmExist = FALSE;

  DataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  RomBaseAddress = (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionMsdmGuid, 1);

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    DataSize,
                    (VOID **)&MsdmData
                    );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 RomBaseAddress,
                                 0,
                                 &DataSize,
                                 (UINT8*) MsdmData
                                 );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if ((MsdmData->MsdmVersion == 0xFFFFFFFF) &&
      (MsdmData->MdsmDataType == 0xFFFFFFFF) &&
      (MsdmData->MsdmDataLength == 0xFFFFFFFF)) {
    goto Done;
  }

  MsdmExist = TRUE;

Done:
  if (MsdmData != NULL) {
    gSmst->SmmFreePool (MsdmData);
  }
  return MsdmExist;
}

EFI_STATUS
EFIAPI
SetApandBiosCommDataBuffer (
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer,
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer
  )
{

  if ((ApCommDataBuffer->Signature != AP_COMMUNICATION_SIGNATURE) && (ApCommDataBuffer->Signature != BIOS_COMMUNICATION_SIGNATURE)) {
    return EFI_UNSUPPORTED;
  }

  if (BufferOverlapSmram (ApCommDataBuffer, ApCommDataBuffer->StructureSize)){
    return EFI_UNSUPPORTED;
  }

  if (ApCommDataBuffer->Signature == BIOS_COMMUNICATION_SIGNATURE) {
    CopyMem(BiosCommDataBuffer, (VOID *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), sizeof (BIOS_COMMUNICATION_DATA_TABLE));
    CopyMem(ApCommDataBuffer, &mApCommDataBuffer, sizeof (AP_COMMUNICATION_DATA_TABLE));
  } else {
    ZeroMem (BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraDataSize64k;
    BiosCommDataBuffer->PhysicalDataSize = 0;
    BiosCommDataBuffer->ErrorReturn      = 0;
  }
  return EFI_SUCCESS;
}


/**
  AH=41h, OEM Extra Data Communication type.

  50h = Oa30ReadWrite

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS  Read or write OA3.0 successful.
**/
EFI_STATUS
EFIAPI
Oa30ReadWriteFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{
  UINT32            MsdmDataSize;

  MsdmDataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  switch (ApCommDataBuffer->DataSize) {

  case OemExtraReportReadSize:
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraReportReadSize;
    BiosCommDataBuffer->PhysicalDataSize = MsdmDataSize;
    break;

  case OemExtraSkipSizeCheck:
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraSkipSizeCheck;   //Don't care
    BiosCommDataBuffer->PhysicalDataSize = 0x00;                   //Don't care
    break;

  case OemExtraReportWriteSize:
    BiosCommDataBuffer->Signature        = BIOS_COMMUNICATION_SIGNATURE;
    BiosCommDataBuffer->StructureSize    = sizeof (BIOS_COMMUNICATION_DATA_TABLE);
    BiosCommDataBuffer->BlockSize        = OemExtraBlockSize4k;
    BiosCommDataBuffer->DataSize         = OemExtraReportWriteSize;
    BiosCommDataBuffer->PhysicalDataSize = MsdmDataSize;   //bin size
    break;

  default:
    break;
  }

  if (!MsdmExist ()) {
    BiosCommDataBuffer->ErrorReturn    = (BiosCommDataBuffer->ErrorReturn | ERROR_RETURE_OA30_NOT_EXIST);
  }

  return EFI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication type.
  51h = Oa30Erase Ec Idle true
**/
EFI_STATUS
EFIAPI
KernelT51EcIdelTrue (
  VOID
  )
{
  mEcIdle = FALSE;
  if (MsdmExist () && mOemExtraDataType == Oa30Erase) {
    OemSvcEcIdle (TRUE);
    mEcIdle = TRUE;
  }

  return EFI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication type.
  51h = Oa30Erase Ec Idle False
**/
EFI_STATUS
EFIAPI
KernelT51EcIdelFalse (
  VOID
  )
{
  if (mEcIdle) {
    OemSvcEcIdle (FALSE);
    mEcIdle = FALSE;
  }

  return EFI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication type.

  51h = Oa30Erase

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS  Erase OA3.0 successful.
**/
EFI_STATUS
EFIAPI
Oa30EraseFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{
  EFI_STATUS                   Status;
  UINT8                        LoopCount;
  UINTN                        RomBaseAddress;
  UINTN                        EraseSize = 0x1000;

  RomBaseAddress = (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionMsdmGuid, 1);
  LoopCount = 0;
  Status = EFI_SUCCESS;

  if (!MsdmExist ()) {
    BiosCommDataBuffer->ErrorReturn = (BiosCommDataBuffer->ErrorReturn | ERROR_RETURE_OA30_NOT_EXIST);
    goto Done;
  }
  Status = EFI_UNSUPPORTED;
  while ((EFI_ERROR (Status)) && (LoopCount < 100)) {
    Status = mSmmFwBlockService->EraseBlocks (
                                   mSmmFwBlockService,
                                   RomBaseAddress,
                                   &EraseSize
                                   );
    LoopCount++;
  }

Done:
  BiosCommDataBuffer->DataSize = ApCommDataBuffer->DataSize;
  BiosCommDataBuffer->PhysicalDataSize = ApCommDataBuffer->PhysicalDataSize;

  return Status;
}


/**
  AH=41h, OEM Extra Data Communication type.

  52h = Oa30PopulateHeader

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS   populate header successful.
**/
EFI_STATUS
EFIAPI
Oa30PopulateHeaderFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{
  return EFI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication type 53h to de-populate header.

  53h = Oa30DePopulateHeader

  @param[in]  ApCommDataBuffer   Pointer to AP communication data buffer.
  @param[out] BiosCommDataBuffer Pointer to BIOS communication data buffer.

  @retval EFI_SUCCESS   populate header successful.
**/
EFI_STATUS
EFIAPI
Oa30DePopulateHeaderFunction (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  )
{
  return EFI_SUCCESS;
}

/**
  AH=42h, OEM Extra Data Write.
  50h = Oa30ReadWrite Ec Idle true
**/
EFI_STATUS
EFIAPI
KernelT50EcIdelTrue (
  VOID
  )
{
  UINTN           WriteSize;
  UINT32          MsdmDataSize;

  WriteSize    = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  MsdmDataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  mEcIdle      = FALSE;

  if ((WriteSize == MsdmDataSize) && (mOemExtraDataType == Oa30ReadWrite)) {
    OemSvcEcIdle (TRUE);
    mEcIdle = TRUE;
  }

  return EFI_SUCCESS;
}

/**
  AH=42h, OEM Extra Data Write.
  50h = Oa30ReadWrite Ec Idle false
**/
EFI_STATUS
EFIAPI
KernelT50EcIdelFalse (
  VOID
  )
{
  UINTN           WriteSize;
  UINT32          MsdmDataSize;

  WriteSize       = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  MsdmDataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);

  if (mEcIdle) {
    OemSvcEcIdle (FALSE);
    mEcIdle = FALSE;
  }
  return EFI_SUCCESS;
}

/**
  AH=42h, OEM Extra Data Write.
  50h = Oa30ReadWrite

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
Oa30DataWrite (
  UINT8                   *WriteDataBuffer,
  UINTN                   WriteSize,
  UINTN                   RomBaseAddress
  )
{
  EFI_STATUS              Status;
  UINT8                   LoopCount;
  UINT32                  MsdmDataSize;

  MsdmDataSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);

  if (WriteSize == MsdmDataSize) {
    UINT8                          *ReturnDataBuffer = NULL;
    UINTN                          Index2 = 0;
    UINTN                          EraseSize = 0x1000;
    UINT8                          *TEMP;

    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      0x1000,
                      (VOID **)&ReturnDataBuffer
                      );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = mSmmFwBlockService->Read (
                                   mSmmFwBlockService,
                                   RomBaseAddress,
                                   0,
                                   &EraseSize,
                                   ReturnDataBuffer
                                   );
    //
    // Only modify the first 0x31 bytes
    //
    TEMP = ReturnDataBuffer;
    for (Index2 = 0; Index2 < MsdmDataSize; Index2++) {
      TEMP[Index2] = WriteDataBuffer[Index2];
    }

    LoopCount = 0;
    Status = EFI_UNSUPPORTED;
    *(mSmmFwBlockService->FlashMode) = SMM_FW_FLASH_MODE;

    while ((EFI_ERROR (Status)) && (LoopCount < 100)) {
      Status = mSmmFwBlockService->EraseBlocks (
                                     mSmmFwBlockService,
                                     RomBaseAddress,
                                     &EraseSize
                                     );
      Status = mSmmFwBlockService->Write (
                                     mSmmFwBlockService,
                                     RomBaseAddress,
                                     &EraseSize,
                                     ReturnDataBuffer
                                     );
      LoopCount++;
    }
    gSmst->SmmFreePool (ReturnDataBuffer);
  } else {
    return EFI_INVALID_PARAMETER;
  }

  return IHISI_SUCCESS;
}

/**
  AH=47h, OEM Extra Data Read.

  50h = Oa30ReadWrite

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
Oa30DataRead (
  UINT8                   *ReadDataBuffer,
  UINTN                   *ReadSize,
  UINTN                   RomBaseAddress
  )
{
  EFI_STATUS                       Status;
  UINT8                            *ReturnDataBuffer = NULL;
  UINTN                            DataSize = 0x1000;

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    DataSize,
                    (VOID **)&ReturnDataBuffer
                    );
  if (EFI_ERROR(Status)) {
    return IHISI_FBTS_READ_FAILED;
  }

  Status = mSmmFwBlockService->Read (
                                 mSmmFwBlockService,
                                 RomBaseAddress,
                                 0,
                                 &DataSize,
                                 ReturnDataBuffer
                                 );

  *ReadSize = sizeof (EFI_ACPI_MSDM_DATA_STRUCTURE);
  CopyMem (ReadDataBuffer, ReturnDataBuffer, *ReadSize);

  return Status;
}

/**
  AH=41h, OEM Extra Data Communication type.

  Hook chipset service SmmCsSvcIhisiOemExtCommunication.

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
ChipsetOemExtraDataCommunication (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = SmmCsSvcIhisiOemExtCommunication(ApCommDataBuffer, BiosCommDataBuffer);
  switch (Status) {
    case EFI_SUCCESS:
      Status = IHISI_END_FUNCTION_CHAIN;
      break;

    case EFI_UNSUPPORTED:
    case EFI_MEDIA_CHANGED:
      if ((ApCommDataBuffer->DataType == Oa30ReadWrite) ||
          (ApCommDataBuffer->DataType == Oa30Erase) ||
          (ApCommDataBuffer->DataType == Oa30PopulateHeader) ||
          (ApCommDataBuffer->DataType == Oa30DePopulateHeader)) {
        Status = IHISI_SUCCESS;
      }
      break;
  }

  CopyMem ((VOID *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));

  gSmst->SmmFreePool (BiosCommDataBuffer);

  return Status;

}

/**
  AH=42h, OEM Extra Data Write.

  Hook chipset service SmmCsSvcIhisiOemExtDataWrite.

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
ChipsetOemExtraDataWrite (
  VOID
  )
{
  EFI_STATUS          Status;
  UINTN               WriteSize;
  UINT8               ShutdownMode;
  UINTN               RomBaseAddress;
  UINT8               *WriteDataBuffer;
  UINT32              Ecx;

  WriteDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  WriteSize       = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  RomBaseAddress  = (UINTN) (IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) >> 8);
  ShutdownMode    = (UINT8) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);

  if (BufferOverlapSmram ((VOID *) WriteDataBuffer, WriteSize)){
    return IHISI_INVALID_PARAMETER;
  }

  Status = SmmCsSvcIhisiOemExtDataWrite(mOemExtraDataType, WriteDataBuffer, &WriteSize, &RomBaseAddress, ShutdownMode);
  switch (Status) {
    case EFI_SUCCESS:
      Status = IHISI_END_FUNCTION_CHAIN;
      break;

    case EFI_UNSUPPORTED:
      if (mOemExtraDataType == Oa30ReadWrite) {
        mRomBaseAddress = (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionMsdmGuid, 1);
      }
    case EFI_MEDIA_CHANGED:
      IhisiWriteCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RDI, (UINT32)WriteSize);
      Ecx = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
      Ecx &=0xFFFF00FF;
      Ecx |= (RomBaseAddress << 8);
      IhisiWriteCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RCX, Ecx);
      Status = IHISI_SUCCESS;
      break;

    default:
      break;
  }
  return Status;
}

/**
  AH=47h, OEM Extra Data Read.

  Hook chipset service SmmCsSvcIhisiOemExtDataRead.

  @retval EFI_SUCCESS            OEM Extra Data Read successful.
  @return Other                  OEM Extra Data Read failed.
**/
EFI_STATUS
EFIAPI
ChipsetOemExtraDataRead (
  VOID
  )
{
  EFI_STATUS                       Status;
  UINTN                            ReadSize;
  UINTN                            RomBaseAddress;
  UINT8                            *ReadDataBuffer;

  ReadSize = 0;
  RomBaseAddress = 0;
  ReadDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  ReadSize = *(UINT32 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  if (BufferOverlapSmram ((VOID *) ReadDataBuffer, ReadSize)){
    return IHISI_INVALID_PARAMETER;
  }

  Status = SmmCsSvcIhisiOemExtDataRead(mOemExtraDataType, ReadDataBuffer, &ReadSize, &RomBaseAddress);

  //
  // Pass Chipset update parameter to kernel
  //
  switch (Status) {
    case EFI_SUCCESS:
      Status = IHISI_END_FUNCTION_CHAIN;
      break;

    case EFI_MEDIA_CHANGED:
      mRomBaseAddress = RomBaseAddress;
      CopyMem ((VOID *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI), &ReadSize, sizeof(UINT32));
      Status = IHISI_SUCCESS;
      break;

    case EFI_UNSUPPORTED:
      if (mOemExtraDataType == Oa30ReadWrite) {
        mRomBaseAddress = (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionMsdmGuid, 1);
      }
      Status = IHISI_SUCCESS;
      break;

    default:
      Status = IHISI_SUCCESS;
      break;
  }

  return Status;

}

/**
  AH=42h, OEM Extra Data Write.

  Hook chipset service.SmmCsSvcIhisiFbtsShutDown

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
ChipsetOemExtraDataDosShutdownMode (
  VOID
  )
{
  UINT8           ShutdownMode;
  EFI_STATUS      Status;

  Status = EFI_SUCCESS;
  ShutdownMode    = (UINT8) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);

  switch (ShutdownMode) {
    case DosReboot:
      Status = SmmCsSvcIhisiFbtsReboot ();
      break;

    case DosShutdown:
      Status = SmmCsSvcIhisiFbtsShutDown ();
      break;

    case DoNothing:
    case WindowsReboot:
    case WindowsShutdown:
    default:
      Status = EFI_SUCCESS;
      break;
  }

  return Status;
}

/**
  AH=41h, Hook OemService OEM Extra Data Communication

  01h = VBIOS
  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemIhisiS41T1Vbios (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = IHISI_SUCCESS;
  if (ApCommDataBuffer->DataType == Vbios) {
    Status = OemSvcIhisiS41T1VbiosFunction (ApCommDataBuffer, BiosCommDataBuffer);
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcIhisiS41T1VbiosFunction, Status : %r\n", Status));
    CopyMem ((VOID *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
    Status = IHISI_END_FUNCTION_CHAIN;
  }

  gSmst->SmmFreePool (BiosCommDataBuffer);

  return Status;
}

/**
  AH=41h, Oem OEM Extra Data Communication

  54h = LogoUpdate
  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemIhisiS41T54LogoUpdate (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = IHISI_SUCCESS;
  if (ApCommDataBuffer->DataType == LogoUpdate) {
    Status = OemSvcIhisiS41T54LogoUpdateFunction (ApCommDataBuffer, BiosCommDataBuffer);
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcIhisiS41T54LogoUpdateFunction, Status : %r\n", Status));
    CopyMem ((VOID *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
    Status = IHISI_END_FUNCTION_CHAIN;
  }

  gSmst->SmmFreePool (BiosCommDataBuffer);

  return Status;
}

/**
  AH=41h, Oem OEM Extra Data Communication

  55h = CheckBiosSignBySystemBios
  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemIhisiS41T55CheckBiosSignBySystemBios (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = EFI_SUCCESS;
  if (ApCommDataBuffer->DataType == CheckBiosSignBySystemBios) {
    Status = OemSvcIhisiS41T55CheckBiosSignBySystemBiosFunction (ApCommDataBuffer, BiosCommDataBuffer);
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcIhisiS41T55CheckBiosSignBySystemBiosFunction, Status : %r\n", Status));
    CopyMem ((VOID *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
    Status = IHISI_END_FUNCTION_CHAIN;
  }

  gSmst->SmmFreePool (BiosCommDataBuffer);

  return Status;
}

/**
  AH=41h, Oem OEM Extra Data Communication

  50h = Oa30ReadWrite
  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemIhisiS41T50a30ReadWrite (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ApCommDataBuffer->DataType == Oa30ReadWrite) {
    Status = OemSvcIhisiS41T50HookOa30ReadWriteFunction (ApCommDataBuffer, BiosCommDataBuffer);
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemIhisiS41T50a30ReadWrite, Status : %r\n", Status));
  }

  if (Status == EFI_SUCCESS) {
    Status = IHISI_END_FUNCTION_CHAIN;
  } else {
    Status = IHISI_SUCCESS;
  }

  CopyMem ((VOID *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
  gSmst->SmmFreePool (BiosCommDataBuffer);

  return Status;
}

/**
  AH=41h, Oem OEM Extra Data Communication

  Resserved
  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemIhisiS41ReservedFunction (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = OemSvcIhisiS41ReservedFunction (ApCommDataBuffer, BiosCommDataBuffer);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemIhisiS41ReservedFunction, Status : %r\n", Status));

  CopyMem ((VOID *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));
  gSmst->SmmFreePool (BiosCommDataBuffer);

  return EFI_SUCCESS;
}

/**
  AH=41h,OEM Extra Data Communication

  Save input registers;

  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
KernelCommunicationSaveRegs (
  VOID
  )
{
  AP_COMMUNICATION_DATA_TABLE      *ApCommDataBuffer;

  mRomBaseAddress = 0;
  ApCommDataBuffer = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  if (ApCommDataBuffer->Signature != AP_COMMUNICATION_SIGNATURE) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  if (BufferOverlapSmram ((VOID *) ApCommDataBuffer, ApCommDataBuffer->StructureSize)){
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  mOemExtraDataType = ApCommDataBuffer->DataType;

  CopyMem(&mApCommDataBuffer, ApCommDataBuffer, sizeof(AP_COMMUNICATION_DATA_TABLE));

  IhisiErrorCodeHandler((UINT32)IHISI_SUCCESS);

  return IHISI_SUCCESS;
}

/**
  AH=41h, OEM Extra Data Communication

  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
KernelOemExtraDataCommunication (
  VOID
  )
{
  EFI_STATUS                        Status;
  BIOS_COMMUNICATION_DATA_TABLE     *BiosCommDataBuffer;
  AP_COMMUNICATION_DATA_TABLE       *ApCommDataBuffer;

  ApCommDataBuffer   = (AP_COMMUNICATION_DATA_TABLE*) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (BIOS_COMMUNICATION_DATA_TABLE),
                    (VOID **)&BiosCommDataBuffer
                    );

  Status = SetApandBiosCommDataBuffer(ApCommDataBuffer, BiosCommDataBuffer);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  switch (ApCommDataBuffer->DataType) {
    case Oa30ReadWrite:
      Status = Oa30ReadWriteFunction (ApCommDataBuffer, BiosCommDataBuffer);
      break;

    case Oa30Erase:
      Status = Oa30EraseFunction (ApCommDataBuffer, BiosCommDataBuffer);
     break;

    case Oa30PopulateHeader:
      Status = Oa30PopulateHeaderFunction (ApCommDataBuffer, BiosCommDataBuffer);
      break;

    case Oa30DePopulateHeader:
      Status = Oa30DePopulateHeaderFunction (ApCommDataBuffer, BiosCommDataBuffer);
      break;

    default:
      Status = EFI_SUCCESS;
      break;
  }

  if (EFI_ERROR (Status)) {
    mOemExtraDataType = 0;
    return Status;
  }

  CopyMem ((VOID *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX), BiosCommDataBuffer, sizeof (BIOS_COMMUNICATION_DATA_TABLE));

  gSmst->SmmFreePool (BiosCommDataBuffer);
  return IHISI_SUCCESS;
}

/**
  AH=42h, OEM Extra Data Write.

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
KernelOemExtraDataWrite (
  VOID
  )
{
  EFI_STATUS          Status;
  UINTN               WriteSize;
  UINTN               RomBaseAddress;
  UINT8               *WriteDataBuffer;

  WriteDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  WriteSize       = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  RomBaseAddress  = (UINTN) (IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX) >> 8);

  if (BufferOverlapSmram ((VOID *) WriteDataBuffer, WriteSize)){
    return IHISI_INVALID_PARAMETER;
  }

  switch (mOemExtraDataType) {
    case Oa30ReadWrite:
      RomBaseAddress = mRomBaseAddress;
      Status = Oa30DataWrite(WriteDataBuffer, WriteSize, RomBaseAddress);
      break;

    default:
      Status = EFI_SUCCESS;
      break;
  }

  return Status;
}

/**
  AH=47h, OEM Extra Data Read.

  @retval EFI_SUCCESS            OEM Extra Data Read successful.
  @return Other                  OEM Extra Data Read failed.
**/
EFI_STATUS
EFIAPI
KernelOemExtraDataRead (
  VOID
  )
{
  EFI_STATUS                       Status;
  UINTN                            ReadSize;
  UINTN                            RomBaseAddress;
  UINT8                            *ReadDataBuffer;

  ReadSize = 0;
  RomBaseAddress = mRomBaseAddress;
  ReadDataBuffer = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  ReadSize = *(UINT32 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  if (BufferOverlapSmram ((VOID *) ReadDataBuffer, ReadSize)){
    return IHISI_INVALID_PARAMETER;
  }

  switch (mOemExtraDataType) {
    case Oa30ReadWrite:
      Status = Oa30DataRead(ReadDataBuffer, &ReadSize, RomBaseAddress);
      break;

    default:
      Status = EFI_SUCCESS;
      break;
  }

  CopyMem ((VOID *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI), &ReadSize, sizeof(UINT32));

  return Status;
}

EFI_STATUS
InstallOemExtraDataCommunicationServices (
  VOID
  )
{
  EFI_STATUS              Status;
  IHISI_REGISTER_TABLE    *SubFuncTable;
  UINT16                  TableCount;

  SubFuncTable = OEM_EXT_COMMON_REGISTER_TABLE;
  TableCount = sizeof(OEM_EXT_COMMON_REGISTER_TABLE)/sizeof(OEM_EXT_COMMON_REGISTER_TABLE[0]);
  Status = RegisterIhisiSubFunction (SubFuncTable, TableCount);
  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

