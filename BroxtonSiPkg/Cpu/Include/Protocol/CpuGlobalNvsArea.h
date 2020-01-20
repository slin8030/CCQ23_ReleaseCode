/** @file
  Definition of the CPU global NVS area protocol. This protocol
  publishes the address and format of a global ACPI NVS buffer used as a communications
  buffer between SMM/DXE/PEI code and ASL code.

  @note Data structures defined in this protocol are not naturally aligned.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
#ifndef _CPU_GLOBAL_NVS_AREA_H_
#define _CPU_GLOBAL_NVS_AREA_H_

typedef struct _CPU_GLOBAL_NVS_AREA_PROTOCOL CPU_GLOBAL_NVS_AREA_PROTOCOL;

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gCpuGlobalNvsAreaProtocolGuid;

//
// Processor Power Management GlobalNvs Revisions
//
#define CPU_GLOBAL_NVS_AREA_REVISION_1  1 ///< Initial Version

#pragma pack(1)
///
/// CPU Global NVS Area definition
///
typedef struct {
  UINT8  Revision;                             ///< (0) CPU GlobalNvs Revision
  //
  // PPM Flag Values
  //
  UINT32 PpmFlags;                             ///< (1-4) PPM Flags
  //
  // Number of Logical Processors Values
  //
  UINT8  LogicalProcessorCount;                ///< (5) Number of Logical Processors
  //
  // Thermal Configuration Values
  //
  UINT8  AutoCriticalTripPoint;                ///< (6) Auto Critical Trip Point
  UINT8  AutoPassiveTripPoint;                 ///< (7) Auto Passive Trip Point
  UINT8  AutoActiveTripPoint;                  ///< (8) Auto Active Trip Point
  UINT32 Cpuid;                                ///< (9-12) CPUID

  UINT8  Reserved[28];                         ///< (13-40) Reserved
  //
  // Mwait Hints and Latency values for C3/C6/C7/C7S
  //
  UINT8  C3MwaitValue;                         ///< (41) Mwait Hint value for C3
  UINT8  C6MwaitValue;                         ///< (42) Mwait Hint value for C6
  UINT8  C7MwaitValue;                         ///< (43) Mwait Hint value for C6
  UINT8  CDMwaitValue;                         ///< (44) Mwait Hint value for C7/C8/C9/C10
  UINT16 C3Latency;                            ///< (45-46) Latency value for C3
  UINT16 C6Latency;                            ///< (47-48) Latency Value for C6
  UINT16 C7Latency;                            ///< (49-50) Latency Value for C6
  UINT16 CDLatency;                            ///< (51-52) Latency Value for C7/C8/C9/C10
  UINT16 CDIOLevel;                            ///< (53-54) IO Level Value for C7/C8/C9/C10
  UINT16 CDPowerValue;                         ///< (55-56) Power Value for C7/C8/C9/C10
  UINT8  MiscPowerManagementFlags;             ///< (57) MiscPowerManagementFlags
  //
  // DTS
  //
  UINT8  EnableDigitalThermalSensor;           ///< (58) DTS Function enable
  UINT8  BspDigitalThermalSensorTemperature;   ///< (59) Temperature of BSP
  UINT8  ApDigitalThermalSensorTemperature;    ///< (60) Temperature of AP
  UINT8  DigitalThermalSensorSmiFunction;      ///< (61) SMI function call via DTS IO Trap
  UINT8  PackageDTSTemperature;                ///< (62) Package temperature
  UINT8  IsPackageTempMSRAvailable;            ///< (63) Package Temperature MSR available
  UINT8  Ap2DigitalThermalSensorTemperature;   ///< (64) Temperature of the second AP
  UINT8  Ap3DigitalThermalSensorTemperature;   ///< (65) Temperature of the third AP

  UINT8  Reserved1[16];                        ///< (66-81) Reserved
  UINT8  DtsAcpiEnable;                        ///< (82) DTS is in ACPI Mode Enabled
  //
  // Software Guard Extension
  //
  UINT8  SgxStatus;                            ///< (83)    SE Status
  UINT64 EpcBaseAddress;                       ///< (84-91) EPC Base Address
  UINT64 EpcLength;                            ///< (92-99) EPC Length

  //
  //  HWP
  //
  UINT8  HwpVersion;                           ///< (100) HWP Status
  UINT16 HwpIoTrapAddress;                     ///< (101-102) IoTrap Address for HWP
  UINT16 HwpIoTrapLength;                      ///< (103-104) IoTrap Length for HWP
  UINT8  PowerState;                           ///< (105) Power State
  UINT8  EnableHdcPolicy;                      ///< (106) Hardware Duty Cycling Policy
  UINT8  HwpInterruptStatus;                   ///< (107) HWP Interrupt Status

  UINT8  MonitorMwaitEnable;                   ///< (108) Mwait Enable
} CPU_GLOBAL_NVS_AREA;
#pragma pack()
///
/// CPU Global NVS Area Protocol
///
struct _CPU_GLOBAL_NVS_AREA_PROTOCOL {
  CPU_GLOBAL_NVS_AREA *Area;
};

#endif
