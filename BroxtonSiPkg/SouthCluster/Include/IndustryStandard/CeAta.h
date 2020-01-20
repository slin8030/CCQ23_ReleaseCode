/** @file
  Header file for chipset CE-AT spec.

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

@par Specification Reference:
**/

#ifndef _CE_ATA_H
#define _CE_ATA_H

#pragma pack(1)


#define  DATA_UNIT_SIZE       512


#define  CMD60                60
#define  CMD61                61


#define RW_MULTIPLE_REGISTER  CMD60
#define RW_MULTIPLE_BLOCK     CMD61


#define CE_ATA_SIG_CE         0xCE
#define CE_ATA_SIG_AA         0xAA


#define Reg_Features_Exp      01
#define Reg_SectorCount_Exp   02
#define Reg_LBALow_Exp        03
#define Reg_LBAMid_Exp        04
#define Reg_LBAHigh_Exp       05
#define Reg_Control           06
#define Reg_Features_Error    09
#define Reg_SectorCount       10
#define Reg_LBALow            11
#define Reg_LBAMid            12
#define Reg_LBAHigh           13
#define Reg_Device_Head       14
#define Reg_Command_Status    15

#define Reg_scrTempC          0x80
#define Reg_scrTempMaxP       0x84
#define Reg_scrTempMinP       0x88
#define Reg_scrStatus         0x8C
#define Reg_scrReallocsA      0x90
#define Reg_scrERetractsA     0x94
#define Reg_scrCapabilities   0x98
#define Reg_scrControl        0xC0



typedef struct {
  UINT8  Reserved0;
  UINT8  Features_Exp;
  UINT8  SectorCount_Exp;
  UINT8  LBALow_Exp;
  UINT8  LBAMid_Exp;
  UINT8  LBAHigh_Exp;
  UINT8  Control;
  UINT8  Reserved1[2];
  UINT8  Features_Error;
  UINT8  SectorCount;
  UINT8  LBALow;
  UINT8  LBAMid;
  UINT8  LBAHigh;
  UINT8  Device_Head;
  UINT8  Command_Status;
}TASK_FILE;


//
//Reduced ATA command set
//
#define IDENTIFY_DEVICE       0xEC
#define READ_DMA_EXT          0x25
#define WRITE_DMA_EXT         0x35
#define STANDBY_IMMEDIATE     0xE0
#define FLUSH_CACHE_EXT       0xEA



typedef struct {
  UINT16  Reserved0[10];
  UINT16  SerialNumber[10];
  UINT16  Reserved1[3];
  UINT16  FirmwareRevision[4];
  UINT16  ModelNumber[20];
  UINT16  Reserved2[33];
  UINT16  MajorVersion;
  UINT16  Reserved3[19];
  UINT16  MaximumLBA[4];
  UINT16  Reserved4[2];
  UINT16  Sectorsize;
  UINT16  Reserved5;
  UINT16  DeviceGUID[4];
  UINT16  Reserved6[94];
  UINT16  Features;
  UINT16  MaxWritesPerAddress;
  UINT16  Reserved7[47];
  UINT16  IntegrityWord;
}IDENTIFY_DEVICE_DATA;





#pragma pack()

#endif
