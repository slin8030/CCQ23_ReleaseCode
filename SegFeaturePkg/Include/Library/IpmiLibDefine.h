/** @file
 H2O IPMI SEL Info library header file.

 This file contains functions prototype that can easily retrieve SEL Info via
 using H2O IPMI SEL Data protocol.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
//
// Server Feature - Insyde IPMI Package V1.0
//

#ifndef _IPMI_LIB_DEFINE_H_
#define _IPMI_LIB_DEFINE_H_

#include <Library/H2OIpmiSdr.h>

#define SEL_INFO_STRING_LEN               64
#define SEL_INFO_DESC_STRING_LEN          256
#define SDR_STRING_LENGTH                 0x20


#pragma pack(1)
//
// Standard System Event Log Structure
//
typedef struct {
  UINT16  RecordId;
  UINT8   RecordType;
  UINT32  TimeStamp;
  UINT16  GeneratorId;
  UINT8   EvMRev;
  UINT8   SensorType;
  UINT8   SensorNum;
  UINT8   EventType :7;
  UINT8   EventDir  :1;
  UINT8   EventData1;
  UINT8   EventData2;
  UINT8   EventData3;
} H2O_IPMI_SEL_DATA_INPUT;

typedef struct {
  CHAR16    SensorName[SEL_INFO_STRING_LEN];
  CHAR16    Type[SEL_INFO_STRING_LEN];
  CHAR16    Desc[SEL_INFO_DESC_STRING_LEN];
  CHAR16    Generator[SEL_INFO_STRING_LEN];
  CHAR16    IdType[SEL_INFO_STRING_LEN];
  CHAR16    Lun[SEL_INFO_STRING_LEN];
} SEL_INFO_STRUCT;

//
// Structure to store System Event Log entry
//
typedef struct {
  UINT16                    NextRecordId;
  H2O_IPMI_SEL_DATA_INPUT   Data;
} H2O_IPMI_SEL_ENTRY_INPUT;

typedef struct {
  UINT16  RecordId;
  CHAR16  Name[SDR_STRING_LENGTH];
  UINT16  DataStatus;
  UINT64  Value;
  CHAR16  ValueStr[SDR_STRING_LENGTH];
  CHAR16  Unit[SDR_STRING_LENGTH * 2];
  UINT64  LowNonCriticalThres;
  CHAR16  LowNonCriticalThresStr[SDR_STRING_LENGTH];
  UINT64  LowCriticalThres;
  CHAR16  LowCriticalThresStr[SDR_STRING_LENGTH];
  UINT64  LowNonRecoverThres;
  CHAR16  LowNonRecoverThresStr[SDR_STRING_LENGTH];
  UINT64  UpNonCriticalThres;
  CHAR16  UpNonCriticalThresStr[SDR_STRING_LENGTH];
  UINT64  UpCriticalThres;
  CHAR16  UpCriticalThresStr[SDR_STRING_LENGTH];
  UINT64  UpNonRecoverThres;
  CHAR16  UpNonRecoverThresStr[SDR_STRING_LENGTH];
  UINT8   SensorNumber;
  UINT8   SensorOwnerId;
} SDR_DATA_STURCT;

//
// Standard System Event Log Structure
//
typedef struct {
  UINT16  RecordId;
  UINT8   RecordType;
  UINT32  TimeStamp;
  UINT16  GeneratorId;
  UINT8   EvMRev;
  UINT8   SensorType;
  UINT8   SensorNum;
  UINT8   EventType :7;
  UINT8   EventDir  :1;
  UINT8   EventData1;
  UINT8   EventData2;
  UINT8   EventData3;
} H2O_IPMI_SEL_DATA;

//
// Structure to store System Event Log entry
//
typedef struct {
  UINT16             NextRecordId;
  H2O_IPMI_SEL_DATA  Data;
} H2O_IPMI_SEL_ENTRY;

#pragma pack()

#endif

