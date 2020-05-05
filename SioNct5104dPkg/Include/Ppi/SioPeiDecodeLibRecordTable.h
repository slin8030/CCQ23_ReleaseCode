/** @file
  IoDecodeLib F75113 Code, need to be modified for different chipset support

;******************************************************************************
;* Copyright (c) 2018, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _SIO_PEI_DECODE_LIB_RECORD_TABLE_H_
#define _SIO_PEI_DECODE_LIB_RECORD_TABLE_H_

#define SIO_PEI_DECODE_LIB_RECORD_TABLE_PPI_GUID \
  { 0xd55a226b, 0x90b8, 0x4320, {0x8f, 0x65, 0xc1, 0xd5, 0xa2, 0x55, 0x77, 0x01} }

#define IO_DECODE_RECORD_TABLE_SIZE 32

typedef enum _IO_DECODE_ACTION{
  DeleteDataAction = 0x01,
  ModifyDataAction = 0x02,
  NoAction         = 0x03,
  AddDataAction    = 0x04,
  ActionMaximum
} IO_DECODE_ACTION;

typedef struct _IO_DECODE_RECORD_TABLE {
  UINT16         IoBaseAddress;
  UINT16         Length;
} IO_DECODE_RECORD_TABLE;

typedef struct _IO_DECODE_PRIVATE_DATA_PPI {
  EFI_PEI_PPI_DESCRIPTOR      PpiDescriptor;
  IO_DECODE_RECORD_TABLE      Table[IO_DECODE_RECORD_TABLE_SIZE];
  UINTN                       Count; // It used to record how many data was added into Table
} IO_DECODE_PRIVATE_DATA_PPI;

extern EFI_GUID gH2OSioPeiDecodeLibRecordTablePpiGuid;

#endif
