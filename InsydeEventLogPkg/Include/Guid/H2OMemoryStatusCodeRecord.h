/** @file

   The definition of H2O Memory Status Code Record.

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
  GUID used to identify status code records HOB that originate from the PEI status code.    
  
Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
(C) Copyright 2016 Hewlett Packard Enterprise Development LP<BR>
This program and the accompanying materials are licensed and made available under
the terms and conditions of the BSD License that accompanies this distribution.
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __MEMORY_STATUS_CODE_RECORD_H__
#define __MEMORY_STATUS_CODE_RECORD_H__

///
/// Global ID used to identify GUIDed HOBs that start with a structure of type 
/// MEMORY_STATUSCODE_PACKET_HEADER, followed by an array of structures of type 
/// MEMORY_STATUSCODE_RECORD.  These GUIDed HOBs record all the information 
/// passed into the ReportStatusCode() service of PEI Services Table.
///
///  <pre>
///  Memory status code records packet structure :
///  +---------------+----------+----------+-----+----------+-----+----------+
///  | Packet Header | Record 1 | Record 2 | ... + Record n | ... | Record m |
///  +---------------+----------+----------+-----+----------+-----+----------+
///                  ^                                 ^                     ^
///                  +--------- RecordIndex -----------+                     |
///                  +---------------- MaxRecordsNumber----------------------+
///  </pre>
///
#define MEMORY_STATUS_CODE_RECORD_GUID \
  { \
    0x60cc026, 0x4c0d, 0x4dda, {0x8f, 0x41, 0x59, 0x5f, 0xef, 0x0, 0xa5, 0x2} \
  }

///
/// A header structure that is followed by an array of records that contain the 
/// parameters passed into the ReportStatusCode() service in the PEI Services Table.
///
typedef struct {
  ///
  /// Index of the packet.
  ///
  UINT16  PacketIndex;
  ///
  /// The number of active records in the packet.
  ///
  UINT16  RecordIndex;
  ///
  /// The maximum number of records that the packet can store.
  ///
  UINT32  MaxRecordsNumber;
} MEMORY_STATUSCODE_PACKET_HEADER;

///
/// A header structure that is followed by an array of records that contain the 
/// parameters passed into the ReportStatusCode() service in the DXE Services Table.
///
typedef struct {
  ///
  /// The index pointing to the last recored being stored.
  ///
  UINT32   RecordIndex;
  ///
  /// The number of records being stored.
  ///
  UINT32   NumberOfRecords;
  ///
  /// The maximum number of records that can be stored.
  ///
  UINT32   MaxRecordsNumber;
} RUNTIME_MEMORY_STATUSCODE_HEADER;

///
/// A structure that contains the parameters passed into the ReportStatusCode() 
/// service in the PEI Services Table.
///
typedef struct {
  ///
  /// Status Code type to be reported.
  ///
  EFI_STATUS_CODE_TYPE   CodeType;

  ///
  /// An operation, plus value information about the class and subclass, used to
  /// classify the hardware and software entity.
  ///
  EFI_STATUS_CODE_VALUE  Value;

  ///
  /// The enumeration of a hardware or software entity within
  /// the system. Valid instance numbers start with the number 1.
  ///
  UINT32                 Instance;
} MEMORY_STATUSCODE_RECORD;

extern EFI_GUID gMemoryStatusCodeRecordGuid;

#endif
