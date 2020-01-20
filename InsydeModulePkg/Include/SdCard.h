/** @file
  SD Card spec definition.

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

#ifndef _SD_CARD_H
#define _SD_CARD_H

#include "Mmc.h"

#define CHECK_PATTERN          0xAA  ///< Physical Layer Simplified Specification Version 3.01 recommended 0xAA

#define ACMD6                  6
#define ACMD13                 13
#define ACMD23                 23 
#define ACMD41                 41
#define ACMD42                 42
#define ACMD51                 51

#define FREQUENCY_SD_PP        (25 * 1000 * 1000)
#define FREQUENCY_SD_PP_HIGH   (50 * 1000 * 1000)

#define SD_BUS_WIDTH_1         0
#define SD_BUS_WIDTH_4         2

#define SD_SPEC_10             0
#define SD_SPEC_11             1
#define SD_SPEC_20             2


#pragma pack(1)
typedef struct {
  UINT32  Reserved1         :15;// 0
  UINT32  V27               :1; // 2.70V
  UINT32  V28               :1; // 2.80V
  UINT32  V29               :1; // 2.90V
  UINT32  V30               :1; // 3.00V
  UINT32  V31               :1; // 3.10V
  UINT32  V32               :1; // 3.20V
  UINT32  V33               :1; // 3.30V
  UINT32  V34               :1; // 3.40V
  UINT32  V35               :1; // 3.50V
  UINT32  S18A              :1; // Switching to 1.8V Accepted (S18A)
  UINT32  Reserved2         :5; // Reserved
  UINT32  CardCapacityStatus:1; // Card Capacity Status (CCS)
  UINT32  CardBusy          :1; // This bit is set to LOW if the card has not finished the power up routine
} SD_OCR;

typedef struct {
  UINT8   NotUsed                 :1; // Not used, always 1 [0:0]
  UINT8   CRC                     :7; // CRC [7:1]
  UINT8   Reserved1               :2; // Reserved [9:8]
  UINT8   FileFormat              :2; // File format [11:10]
  UINT8   TemporaryWriteProtect   :1; // Temporary write protection [12:12]
  UINT8   PermanentWriteProtect   :1; // Permanent write protection [13:13]
  UINT8   CopyFlag                :1; // Copy flag (OTP) [14:14]
  UINT8   FileFormatGroup         :1; // File format group [15:15]
  UINT16  Reserved2               :5; // Reserved [20:16]
  UINT16  WriteBlkPartial         :1; // Partial blocks for write allowed [21:21]
  UINT16  MaxWriteBlkLen          :4; // Max. write data block length [25:22]
  UINT16  WriteSpeedFactor        :3; // Write speed factor [28:26]
  UINT16  Reserved3               :2; // Reserved [30:29]
  UINT16  WriteProtectGroupEnable :1; // Write protect group enable [31:31]
  UINT16  WriteProtectGroupSize   :7; // Write protect group size [38:32]
  UINT16  EraseSectorSize         :7; // Erase sector size [45:39]
  UINT16  EraseSingleBlkEnable    :1; // Erase single block enable [46:46]
  UINT16  Reserved4               :1; // Reserved [47:47]
  UINT32  DeviceSizeLow16         :16;// Device size [69:48]
  UINT32  DeviceSizeHigh6         :6; // Device size [69:48]
  UINT32  Reserved5               :6; // Reserved [75:70]
  UINT32  DSRImplemented          :1; // DSR implemented [76:76]
  UINT32  ReadBlkMisalign         :1; // Read block misalignment [77:77]
  UINT32  WriteBlkMisalign        :1; // Write block misalignment [78:78]
  UINT32  ReadBlkPartial          :1; // Partial blocks for read allowed [79:79]
  UINT16  MaxReadBlkLen           :4; // Max. read data block length [83:80]
  UINT16  CardCommandClasses      :12;// Card command classes [95:84]
  UINT8   MaxTransferRate;            // Max. bus clock frequency [103:96]
  UINT8   Nsac;                       // Data read access-time 2 in CLK cycles (NSAC*100) [111:104]
  UINT8   Taac;                       // Data read access-time 1 [119:112]
  UINT8   Reserved6               :6; // 0 [125:120]
  UINT8   CsdStructure            :2; // CSD structure [127:126]
} SD_CSD2;

typedef struct {
  UINT8   NotUsed                 :1; // Not used, always 1 [0:0]
  UINT8   CRC                     :7; // CRC [7:1]
  UINT8   Reserved1               :2; // Reserved [9:8]
  UINT8   FileFormat              :2; // File format [11:10]
  UINT8   TemporaryWriteProtect   :1; // Temporary write protection [12:12]
  UINT8   PermanentWriteProtect   :1; // Permanent write protection [13:13]
  UINT8   CopyFlag                :1; // Copy flag (OTP) [14:14]
  UINT8   FileFormatGroup         :1; // File format group [15:15]
  UINT16  Reserved2               :5; // Reserved [20:16]
  UINT16  WriteBlkPartial         :1; // Partial blocks for write allowed [21:21]
  UINT16  MaxWriteBlkLen          :4; // Max. write data block length [25:22]
  UINT16  WriteSpeedFactor        :3; // Write speed factor [28:26]
  UINT16  Reserved3               :2; // Reserved [30:29]
  UINT16  WriteProtectGroupEnable :1; // Write protect group enable [31:31]
  UINT32  WriteProtectGroupSize   :7; // Write protect group size [38:32]
  UINT32  EraseSectorSize         :7; // Erase sector size [45:39]
  UINT32  EraseSingleBlkEnable    :1; // Erase single block enable [46:46]
  UINT32  DeviceSizeMult          :3; // Device size multiplier [49:47]
  UINT32  VddMaxWriteCurrent      :3; // Max. write current @ VDD max [52:50]
  UINT32  VddMinWriteCurrent      :3; // Max. write current @ VDD min [55:53]
  UINT32  VddMaxReadCurrent       :3; // Max. read current @ VDD max [58:56]
  UINT32  VddMinReadCurrent       :3; // Max. read current @ VDD min [61:59]
  UINT32  DeviceSizeLow2          :2; // Device size [63:62]
  UINT32  DeviceSizeHigh10        :10;// Device size [73:64]
  UINT32  Reserved4               :2; // Reserved [75:74]
  UINT32  DSRImplemented          :1; // DSR implemented [76:76]
  UINT32  ReadBlkMisalign         :1; // Read block misalignment [77:77]
  UINT32  WriteBlkMisalign        :1; // Write block misalignment [78:78]
  UINT32  ReadBlkPartial          :1; // Partial blocks for read allowed [79:79]
  UINT32  MaxReadBlkLen           :4; // Max. read data block length [83:80]
  UINT32  CardCommandClasses      :12;// Card command classes [95:84]
  UINT8   MaxTransferRate;            // Max. bus clock frequency [103:96]
  UINT8   Nsac;                       // Data read access-time 2 in CLK cycles (NSAC*100) [111:104]
  UINT8   Taac;                       // Data read access-time 1 [119:112]
  UINT8   Reserved5               :6; // Reserved [125:120]
  UINT8   CsdStructure            :2; // CSD structure [127:126]
} SD_CSD1;

typedef struct {
  UINT8   NotUsed           :1; // 1 [0:0]
  UINT8   Checksum          :7; // CRC7 checksum [7:1]
  UINT16  ManufacturingDate :12;// Manufacturing date [19:8]
  UINT16  Reserved1         :4; // Reserved [23:20]
  UINT32  ProductSerialNumber;  // Product serial number [55:24]
  UINT8   ProductRevision;      // Product revision [63:56]
  UINT8   ProductName[5];       // Product name [64:103]
  UINT16  OemApplicationId;     // OEM/Application ID [119:104]
  UINT8   ManufacturerId;       // Manufacturer ID [127:120]
} SD_CID;

//
// TODO: Swtich Status data structure is different from the spec
// Switch Status data structure here is in the reverse order of the spec definition since all the actual SD cards we tested are so
//
typedef struct {
  UINT16  MaxCurrent;
  UINT16  Group6Function;
  UINT16  Group5Function;
  UINT16  Group4Function;
  UINT16  Group3Function;
  UINT16  Group2Function;
  UINT16  Group1Function;
  UINT8   Group65Status;
  UINT8   Group43Status;
  UINT8   Group21Status;
  UINT8   DataStructureVersion;
  UINT16  Group6BusyStatus;
  UINT16  Group5BusyStatus;
  UINT16  Group4BusyStatus;
  UINT16  Group3BusyStatus;
  UINT16  Group2BusyStatus;
  UINT16  Group1BusyStatus;
  UINT8   Reserved0[34];
} SWITCH_STATUS;

#pragma pack()
#endif

