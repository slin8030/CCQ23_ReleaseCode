/** @file

   The definition of IPMI definitions for Event Log.

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

#ifndef _EVENT_LOG_IPMI_DEFINITION_H_
#define _EVENT_LOG_IPMI_DEFINITION_H_

#pragma pack(1)

typedef struct {
  UINT8   SelVer;
  UINT16  SelEntries;
  UINT16  FreeSpace;
  UINT32  RecentAdditionTimeStamp;
  UINT32  RecentEraseTimeStamp;
  UINT8   OperationSupport;
} H2O_IPMI_SEL_INFO;

//
// Structure to store information read back when executing GetDeviceId command
//
typedef struct {
  UINT8 DeviceId;
  UINT8 DeviceRevision      :4;
  UINT8 Reserved            :3;
  UINT8 ProvideSDRsDevice   :1;
  UINT8 MajorFmRevision   :7;
  UINT8 DeviceAvailable   :1;
  UINT8 MinorFmRevision;
  UINT8 IpmiVersionMostSigBits  :4;
  UINT8 IpmiVersionLeastSigBits :4;
  UINT8 SensorDevice        :1;
  UINT8 SDRRepositoryDevice :1;
  UINT8 SELDevice           :1;
  UINT8 FRUInventoryDevice  :1;
  UINT8 IPMBEventReceiver   :1;
  UINT8 IPMBEventGenerator  :1;
  UINT8 Bridge              :1;
  UINT8 ChassisDevice       :1;
  UINT8 ManufacturerId[3];
  UINT8 ProductId[2];
  UINT8 AuxInfo[4];
} H2O_IPMI_BMC_INFO;

//
// IPMI Device Global Command
//
#define H2O_IPMI_CMD_GET_DEVICE_ID          0x01
#define H2O_IPMI_CMD_COLD_RESET             0x02
#define H2O_IPMI_CMD_WARM_RESET             0x03
#define H2O_IPMI_CMD_GET_SELF_TEST_RESULTS  0x04
#define H2O_IPMI_CMD_TEST_ON                0x05
#define H2O_IPMI_CMD_SET_ACPI_POWER_STATE   0x06
#define H2O_IPMI_CMD_GET_ACPI_POWER_STATE   0x07
#define H2O_IPMI_CMD_GET_DEVICE_GUID        0x08

//
// Net Function Definition
//
#define H2O_IPMI_NETFN_CHASSIS      0x00
#define H2O_IPMI_NETFN_BRIDGE       0x02
#define H2O_IPMI_NETFN_SENSOR_EVENT 0x04
#define H2O_IPMI_NETFN_APPLICATION  0x06
#define H2O_IPMI_NETFN_FIRMWARE     0x08
#define H2O_IPMI_NETFN_STORAGE      0x0A
#define H2O_IPMI_NETFN_TRANSPORT    0x0C

//
// Event Command
//
#define H2O_IPMI_CMD_SET_EVENT_RECEIVER     0x00
#define H2O_IPMI_CMD_GET_EVENT_RECEIVER     0x01
#define H2O_IPMI_CMD_EVENT_MESSAGE          0x02

//
// SEL Device Command
//
#define H2O_IPMI_CMD_GET_SEL_INFO                0x40
#define H2O_IPMI_CMD_GET_SEL_ALLOCTION_INFO      0x41
#define H2O_IPMI_CMD_RESERVE_SEL_ENTRY           0x42
#define H2O_IPMI_CMD_GET_SEL_ENTRY               0x43
#define H2O_IPMI_CMD_ADD_SEL_ENTRY               0x44
#define H2O_IPMI_CMD_PARTIAL_ADD_SEL_ENTRY       0x45
#define H2O_IPMI_CMD_DELETE_SEL_ENTRY            0x46
#define H2O_IPMI_CMD_CLEAR_SEL_ENTRY             0x47
#define H2O_IPMI_CMD_GET_SEL_TIME                0x48
#define H2O_IPMI_CMD_SET_SEL_TIME                0x49
#define H2O_IPMI_CMD_GET_SEL_TIME_UTC_OFFSET     0x5C
#define H2O_IPMI_CMD_SET_SEL_TIME_UTC_OFFSET     0x5D

#pragma pack()

#endif
