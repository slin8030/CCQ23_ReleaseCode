/** @file

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/*--
Module Name:

  ChipsetFota.h

Abstract:


--*/

#ifndef _CHIPSET_FOTA_H_
#define _CHIPSET_FOTA_H_


#define FILE_LIST             2
#define BIOS_BACKUP           0
#define CAP_BINARY            1

#define FLASH_SIZE            0x00800000
#define FLASH_SIZE_2M_SPI     0x00200000
#define SPI_DEFAULT           0xFF

//
// FOTA Process Step
//
#define FOTA_START            0x10
#define UPDATE_TXE            0x21
#define UPDATE_BIOS           0x22
#define UPDATE_BACKUP         0x23

//
// Insyde Capsule Process Step
//
#define CAPSULE_NOT_FOUND    0
#define CAPSULE_FOUND        1
#define CAPSULE_UPDATE       2
#define CAPSULE_TXE_CHECK    3      

typedef struct {
  EFI_GUID                              VendorGuid;
  CHAR16                                *VariableName;
} FOTA_PRESERVED_VARIABLE_TABLE;

#endif
