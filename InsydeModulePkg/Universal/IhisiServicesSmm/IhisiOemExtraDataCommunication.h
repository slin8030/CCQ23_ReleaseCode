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

#ifndef _IHISI_OEM_EXTRA_DATA_COMMUNICATION_H_
#define _IHISI_OEM_EXTRA_DATA_COMMUNICATION_H_

#include "IhisiRegistration.h"

typedef
EFI_STATUS
(EFIAPI *OEM_EXT_FUNCTION) (
  IN  AP_COMMUNICATION_DATA_TABLE          *ApCommDataBuffer,
  OUT BIOS_COMMUNICATION_DATA_TABLE        *BiosCommDataBuffer
  );

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
  );

/**
  AH=41h, OEM Extra Data Communication type.
  51h = Oa30Erase Ec Idle true
**/
EFI_STATUS
EFIAPI
KernelT51EcIdelTrue (
  VOID
  );

/**
  AH=41h, OEM Extra Data Communication type.
  51h = Oa30Erase Ec Idle False
**/
EFI_STATUS
EFIAPI
KernelT51EcIdelFalse (
  VOID
  );

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
  );

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
  );

/**
  AH=41h, OEM Extra Data Communication type.

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
  );

/**
  AH=42h, OEM Extra Data Write.
  50h = Oa30ReadWrite Ec Idle true
**/
EFI_STATUS
EFIAPI
KernelT50EcIdelTrue (
  VOID
  );

/**
  AH=42h, OEM Extra Data Write.
  50h = Oa30ReadWrite Ec Idle false
**/
EFI_STATUS
EFIAPI
KernelT50EcIdelFalse (
  VOID
  );

/**
  AH=42h, OEM Extra Data Write type.

  50h = Oa30ReadWrite

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
Oa30DataWrite (
  UINT8                  *WriteDataBuffer,
  UINTN                   WriteSize,
  UINTN                   RomBaseAddress
  );

/**
  AH=47h, OEM Extra Data Read type.

  50h = Oa30ReadWrite

  @retval EFI_SUCCESS   Successfully returns.
**/
EFI_STATUS
EFIAPI
Oa30DataRead (
  UINT8                  *ReadDataBuffer,
  UINTN                  *ReadSize,
  UINTN                   RomBaseAddress
  );

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
  );

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
  );

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
  );

/**
  AH=42h, OEM Extra Data Write.

  Hook chipset service.SmmCsSvcIhisiFbtsShutDown/SmmCsSvcIhisiFbtsReboot

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
ChipsetOemExtraDataDosShutdownMode (
  VOID
  );

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
  );

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
  );

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
  );

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
  );

/**
  AH=41h, Oem OEM Extra Data Communication

  Reserve;
  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
OemIhisiS41ReservedFunction (
  VOID
  );

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
  );

/**
  AH=41h, OEM Extra Data Communication

  @retval EFI_SUCCESS    Process OEM extra data communication successful.
  @return Other          Process OEM extra data communication failed.
**/
EFI_STATUS
EFIAPI
KernelOemExtraDataCommunication (
  VOID
  );

/**
  AH=42h, OEM Extra Data Write.

  @retval EFI_SUCCESS            OEM Extra Data Write successful.
  @return Other                  OEM Extra Data Write failed.
**/
EFI_STATUS
EFIAPI
KernelOemExtraDataWrite (
  VOID
  );

/**
  AH=47h, OEM Extra Data Read.

  @retval EFI_SUCCESS            OEM Extra Data Read successful.
  @return Other                  OEM Extra Data Read failed.
**/
EFI_STATUS
EFIAPI
KernelOemExtraDataRead (
  VOID
  );
#endif
