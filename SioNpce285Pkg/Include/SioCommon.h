/** @file

  Define Super I/O config register and common structure

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SIO_COMMON_H_
#define _SIO_COMMON_H_

#pragma pack (1)
typedef struct {
  UINT8 Register;
  UINT8 Value;
} EFI_SIO_TABLE;

typedef struct {
  UINT8  Device;
  UINT16 DeviceBaseIo;
  UINT8  DeviceLdn;
} EFI_ASL_RESOURCE;

typedef struct _SIO_DEVICE_LIST_TABLE{
  UINT8   TypeH;
  UINT8   TypeL;
  UINT8   TypeInstance;
  UINT8   Device;
  UINT8   DeviceInstance;
  UINT8   DeviceEnable;
  UINT16  DeviceBaseAdr;
  UINT8   DeviceSize;
  UINT8   DeviceLdn;
  UINT8   DeviceIrq;
  UINT8   DeviceDma;
} SIO_DEVICE_LIST_TABLE;

typedef struct _SIO_EXTENSIVE_TABLE_ENTRY_POINT {
  UINT8  EntryPoint;
  UINT8  Length;
  UINT8  SioInstance;
  UINT8  Reserve;
} SIO_EXTENSIVE_TABLE_ENTRY_POINT;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE0 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Device;
  UINT8  DeviceInstance;
} SIO_IDENTIFY_DEVICE;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE1 {
  UINT8  Type;
  UINT8  Length;
  UINT8  OwnedMode;
  UINT8  Enable;
  UINT8  Setting;
} SIO_DEVICE_MODE_SELECTION;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE2 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  Ddn[10];
} SIO_DEVICE_DDN_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE3 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  IrqInfo;
} SIO_ACPI_IRQ_INFORMATION;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE4 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  SetupDefault;
  UINT8  LevelSupported;
  UINT8  Level;
} SIO_DEVICE_FIFO_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE5 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  MaxSupportPowerState;
  UINT16 MaxSupportFreq;
  UINT8  MaxSupportDuty;
  UINT8  NumericDutySetting;
  UINT8  AddModeFreq;
  UINT8  AddModeDuty;
  UINT8  ExclusiveModeFreq;
  UINT8  ExclusiveModeDuty;
  UINT8  SupportSetting;
  UINT8  S0DefaultSetting;
  UINT8  S3DefaultSetting;
  UINT8  S5DefaultSetting;
  UINT8  DeepS3DefaultSetting;
  UINT8  DeepS5DefaultSetting;
  UINT8  DeepSleepWellDefaultSetting;
  UINT8  ActiveState;
  UINT8  BlinkingPin;
} SIO_LED_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE6 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  SupportDisplayMode;
  UINT8  DisplayModeSelect;
  UINT8  SupportDutyCyclePhase;
  UINT8  MaximumDutyCycle;
  UINT8  MiddleDutyCycle;
  UINT8  MinimumDutyCycle;
  UINT8  IncreaseDutyCycle;
  UINT8  DecreaseDutyCycle;
  UINT8  RepeatDutyCycle;
  UINT8  DutyCycleExtendSelect;
  UINT8  LightExtendTime;
  UINT8  DarkExtendTime;
  UINT8  SupportFadingControl;
  UINT8  BlinkModeFadingFreq;
  UINT8  FadingFreqDivide;
  UINT8  DutyCycleChangeFreq;
} SIO_FADING_LED_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE7 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  VoltageName;
  UINT8  Instance;
  UINT8  InputType;
  UINT8  Resistor1;
  UINT8  Resistor2;
} SIO_HWM_VOLTAGE_CONFIGURATION;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE8 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  Instance;
  UINT8  SourceSupported;
  UINT8  SourceSelection;
  UINT8  SensorType;
} SIO_HWM_TEMPERATURE_CONFIGURATION;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE9 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  FanName;
  UINT8  Instance;
  UINT8  OuputType;
  UINT8  FanDivisor;
  UINT8  SmartFanSupported;
  UINT8  SmartFanSelection;
} SIO_HWM_FAN_CONFIGURATION;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE10 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Output;
} SIO_SMART_FAN_MANUAL;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE11 {
  UINT8  Type;
  UINT8  Length;
  UINT8  OwnedSource;
  UINT8  TemperatureSource;
  UINT8  MaximumTarget;
  UINT8  TargetTemperature;
  UINT8  MaximumTolerance;
  UINT8  Tolerance;
} SIO_SMART_FAN_THERMAL_CRUISE;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE12 {
  UINT8  Type;
  UINT8  Length;
  UINT16 MaximumTarget;
  UINT16 TargetSpeedCount;
  UINT8  MaximumTolerance;
  UINT8  Tolerance;
} SIO_SMART_FAN_SPEED_CRUISE;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE13 {
  UINT8  Type;
  UINT8  Length;
  UINT8  OwnedSoucre;
  UINT8  TemperatureSource;
  UINT8  MaximumTarget;
  UINT8  TargetTemperature;
  UINT8  MaximumTolerance;
  UINT8  Tolerance;
  UINT8  InitialOutput;
  UINT8  OutputStep;
} SIO_SMART_FAN_III;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE14 {
  UINT8  Type;
  UINT8  Length;
  UINT8  OwnedSoucre;
  UINT8  TemperatureSource;
  UINT8  BoundaryNumber;
  UINT8  MaximumTemperature;
  UINT8  Boundary[6];
  UINT8  Output[6];
} SIO_SMART_FAN_IV;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE15 {
  UINT8   Type;
  UINT8   Length;
  UINT8   Enable;
  UINT8   SourceSupported;
  UINT8   SourceSelection;
  UINT32  BaudRateSetting;
} SIO_PORT80_TO_UART;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE16 {
  UINT8   Type;
  UINT8   Length;
  UINT8   UnitSupported;
  UINT8   UnitSelection;
  UINT16  TimeOut;
  UINT16  MaxTimeOut;
} SIO_WATCH_DOG_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE18 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  MaxSupportFreq;
  UINT8  PowerStateLedSetting;
} SIO_POWER_STATE_LED_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE19 {
  UINT8  Type;
  UINT8  Length;
  UINT8  SupportEnable;
  UINT8  EnableSetting;
} SIO_LED_ENABLE_SETTING;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE20 {
  UINT8  Type;
  UINT8  Length;
  UINT8  AdditionalFeatureSupport;
  UINT8  OscillatePhase;
  UINT8  AutoModeEnable;
  UINT8  BlinkPattern;
  UINT8  GrnYlwSwitch;
  UINT8  Reserved[4];
} SIO_LED_ADDITIONAL_FEATURE_SETTING;

typedef struct {
  UINT8  Type;
  UINT8  Length;
  UINT8  OwnedSoucre;
  UINT8  TemperatureSource;
  UINT8  BoundaryNumber;
  UINT8  MaximumTemperature;
  UINT8  Boundary[6];
  UINT8  Output[6];
} SIO_SMART_FAN_LINEAR;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE21 {
  UINT8  Type;
  UINT8  Length;
  UINT8  OwnedSoucre;
  UINT8  TemperatureSource;
  UINT8  BoundaryNumber;
  UINT8  MaximumTemperature;
  UINT8  Boundary[4];
  UINT8  Output[5];
} SIO_SMART_FAN_STAGE;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE22 {
  UINT8  Type;
  UINT8  Length;
  UINT8  TemperatureSource;
  UINT8  StartTemperature;
  UINT8  OffTemperature;
  UINT8  StartPwm;
  UINT8  MaximumSlopePwm;
  UINT8  SlopePwm;
} SIO_SMART_FAN_GUARDIAN;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE23 {
  UINT8  Type;
  UINT8  Length;
  UINT8  TemperatureSource;
  UINT8  LowTemperatureLimit;
  UINT8  PwmMinimum;
  UINT8  TemperatureRange;
} SIO_SMART_FAN_AUTO;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE24 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Capability;
  UINT8  Setting;
} SIO_PCD_POWER_LOSS;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE25 {
  UINT8  Type;
  UINT8  Length;
  UINT8  Enable;
  UINT8  Device;
  UINT8  DeviceInstance;
  UINT8  Setting;
} SIO_PCD_PME;

#pragma pack ()

//
// Define Type
//
#define EXTENSIVE_TABLE_ENTRY       0xF0
#define EXTENSIVE_TABLE_ENDING      0x4F495324 //EFI_SIGNATURE_32('$', 'S', 'I', 'O')
#define DEVICE_EXTENSIVE_RESOURCE   0x00
#define MODE_SELECTION_TYPE         0x01
#define DDN_SETTING_TYPE            0x02
#define ACPI_IRQ_INFO_TYPE          0x03
#define FIFO_SETTING_TYPE           0x04
#define FRONT_PANEL_LED_TYPE        0x05
#define FADING_LED_TYPE             0x06
#define VOLTAGE_CONFIG_TYPE            0x07
#define TEMPERATURE_CONFIG_TYPE        0x08
#define FAN_CONFIG_TYPE                0x09
#define SMART_FAN_MANUAL_TYPE          0x0A
#define SMART_FAN_THERMAL_CRUISE_TYPE  0x0B
#define SMART_FAN_SPEED_CRUISE_TYPE    0x0C
#define SMART_FAN_III_TYPE             0x0D
#define SMART_FAN_IV_TYPE              0x0E
#define PORT80_TO_UART_TYPE            0x0F
#define WATCH_DOG_SETTING_TYPE         0x10
#define TYPE17                         0x11
#define POWER_STATE_LED_TYPE           0x12
#define LED_ENABLE_TYPE                0x13
#define LED_ADDITIONAL_FEATURE_TYPE    0x14
#define SMART_FAN_LINEAR_TYPE          SMART_FAN_IV_TYPE
#define SMART_FAN_STAGE_TYPE           0x15
#define SMART_FAN_GUARDIAN_TYPE        0x16
#define SMART_FAN_AUTO_TYPE            0x17
#define POWER_LOSS_TYPE                0x18
#define PME_TYPE                       0x19

//
// Setup Page
//
#define SELECT_DISABLE              0x00
#define SELECT_ENABLE               0x01
#define SELECT_AUTO                 0x02

//
// Define Device Register
//
#define CFG_ID                      0x7F
#define NULL_ID                     0xFF
#define NONE_ID                     0x00
//
// PNP device
//
#define COM                         0x01
#define FLOPPY                      0x02
#define LPT                         0x03
#define KYBD                        0x04
#define MOUSE                       0x05
#define IR                          0x07
#define CIR                         0x07
#define GAME                        0x08
//
// Non PNP device
//
#define HWM                         0x10
#define SACPI                       0x11
#define WDT                         0x12
#define SGPIO                       0x13
#define PORT80                      0x15
#define LED                         0x20
#define POWER_LOSS                  0x21
#define PME                         0x22

#define DEVICE_INSTANCE0            0x00
#define DEVICE_INSTANCE1            0x01
#define DEVICE_INSTANCE2            0x02
#define DEVICE_INSTANCE3            0x03
#define DEVICE_INSTANCE4            0x04
#define DEVICE_INSTANCE5            0x05
#define DEVICE_INSTANCE6            0x06
#define DEVICE_INSTANCE7            0x07
#define DEVICE_INSTANCE8            0x08
#define DEVICE_INSTANCE9            0x09
#define DEVICE_INSTANCE10           0x0A
#define DEVICE_INSTANCE11           0x0B
#define DEVICE_INSTANCE12           0x0C
#define DEVICE_INSTANCE13           0x0D
#define DEVICE_INSTANCE14           0x0E
#define DEVICE_INSTANCE15           0x0F
#define DEVICE_INSTANCE16           0x10
#define DEVICE_INSTANCE17           0x11
#define DEVICE_INSTANCE18           0x12
#define DEVICE_INSTANCE19           0x13
#define DEVICE_INSTANCE20           0x14

//
// Configuration Register
//
#define SIO_CR01    0x01
#define SIO_CR02    0x02
#define SIO_CR03    0x03
#define SIO_CR04    0x04
#define SIO_CR05    0x05
#define SIO_CR06    0x06
#define SIO_CR07    0x07
#define SIO_CR08    0x08
#define SIO_CR09    0x09
#define SIO_CR0A    0x0A
#define SIO_CR0B    0x0B
#define SIO_CR0C    0x0C
#define SIO_CR0D    0x0D
#define SIO_CR0E    0x0E
#define SIO_CR0F    0x0F
#define SIO_CR10    0x10
#define SIO_CR11    0x11
#define SIO_CR12    0x12
#define SIO_CR13    0x13
#define SIO_CR14    0x14
#define SIO_CR15    0x15
#define SIO_CR16    0x16
#define SIO_CR17    0x17
#define SIO_CR18    0x18
#define SIO_CR19    0x19
#define SIO_CR1A    0x1A
#define SIO_CR1B    0x1B
#define SIO_CR1C    0x1C
#define SIO_CR1D    0x1D
#define SIO_CR1E    0x1E
#define SIO_CR1F    0x1F
#define SIO_CR20    0x20
#define SIO_CR21    0x21
#define SIO_CR22    0x22
#define SIO_CR23    0x23
#define SIO_CR24    0x24
#define SIO_CR25    0x25
#define SIO_CR26    0x26
#define SIO_CR27    0x27
#define SIO_CR28    0x28
#define SIO_CR29    0x29
#define SIO_CR2A    0x2A
#define SIO_CR2B    0x2B
#define SIO_CR2C    0x2C
#define SIO_CR2D    0x2D
#define SIO_CR2E    0x2E
#define SIO_CR2F    0x2F
#define SIO_CR30    0x30
#define SIO_CR31    0x31
#define SIO_CR32    0x32
#define SIO_CR33    0x33
#define SIO_CR34    0x34
#define SIO_CR35    0x35
#define SIO_CR36    0x36
#define SIO_CR37    0x37
#define SIO_CR38    0x38
#define SIO_CR39    0x39
#define SIO_CR3A    0x3A
#define SIO_CR3B    0x3B
#define SIO_CR3C    0x3C
#define SIO_CR3D    0x3D
#define SIO_CR3E    0x3E
#define SIO_CR3F    0x3F
#define SIO_CR40    0x40
#define SIO_CR41    0x41
#define SIO_CR42    0x42
#define SIO_CR43    0x43
#define SIO_CR44    0x44
#define SIO_CR45    0x45
#define SIO_CR46    0x46
#define SIO_CR47    0x47
#define SIO_CR48    0x48
#define SIO_CR49    0x49
#define SIO_CR4A    0x4A
#define SIO_CR4B    0x4B
#define SIO_CR4C    0x4C
#define SIO_CR4D    0x4D
#define SIO_CR4E    0x4E
#define SIO_CR4F    0x4F
#define SIO_CR50    0x50
#define SIO_CR51    0x51
#define SIO_CR52    0x52
#define SIO_CR53    0x53
#define SIO_CR54    0x54
#define SIO_CR55    0x55
#define SIO_CR56    0x56
#define SIO_CR57    0x57
#define SIO_CR58    0x58
#define SIO_CR59    0x59
#define SIO_CR5A    0x5A
#define SIO_CR5B    0x5B
#define SIO_CR5C    0x5C
#define SIO_CR5D    0x5D
#define SIO_CR5E    0x5E
#define SIO_CR5F    0x5F
#define SIO_CR60    0x60
#define SIO_CR61    0x61
#define SIO_CR62    0x62
#define SIO_CR63    0x63
#define SIO_CR64    0x64
#define SIO_CR65    0x65
#define SIO_CR66    0x66
#define SIO_CR67    0x67
#define SIO_CR68    0x68
#define SIO_CR69    0x69
#define SIO_CR6A    0x6A
#define SIO_CR6B    0x6B
#define SIO_CR6C    0x6C
#define SIO_CR6D    0x6D
#define SIO_CR6E    0x6E
#define SIO_CR6F    0x6F
#define SIO_CR70    0x70
#define SIO_CR71    0x71
#define SIO_CR72    0x72
#define SIO_CR73    0x73
#define SIO_CR74    0x74
#define SIO_CR75    0x75
#define SIO_CR76    0x76
#define SIO_CR77    0x77
#define SIO_CR78    0x78
#define SIO_CR79    0x79
#define SIO_CR7A    0x7A
#define SIO_CR7B    0x7B
#define SIO_CR7C    0x7C
#define SIO_CR7D    0x7D
#define SIO_CR7E    0x7E
#define SIO_CR7F    0x7F
#define SIO_CR80    0x80
#define SIO_CR81    0x81
#define SIO_CR82    0x82
#define SIO_CR83    0x83
#define SIO_CR84    0x84
#define SIO_CR85    0x85
#define SIO_CR86    0x86
#define SIO_CR87    0x87
#define SIO_CR88    0x88
#define SIO_CR89    0x89
#define SIO_CR8A    0x8A
#define SIO_CR8B    0x8B
#define SIO_CR8C    0x8C
#define SIO_CR8D    0x8D
#define SIO_CR8E    0x8E
#define SIO_CR8F    0x8F
#define SIO_CR90    0x90
#define SIO_CR91    0x91
#define SIO_CR92    0x92
#define SIO_CR93    0x93
#define SIO_CR94    0x94
#define SIO_CR95    0x95
#define SIO_CR96    0x96
#define SIO_CR97    0x97
#define SIO_CR98    0x98
#define SIO_CR99    0x99
#define SIO_CR9A    0x9A
#define SIO_CR9B    0x9B
#define SIO_CR9C    0x9C
#define SIO_CR9D    0x9D
#define SIO_CR9E    0x9E
#define SIO_CR9F    0x9F
#define SIO_CRA0    0xA0
#define SIO_CRA1    0xA1
#define SIO_CRA2    0xA2
#define SIO_CRA3    0xA3
#define SIO_CRA4    0xA4
#define SIO_CRA5    0xA5
#define SIO_CRA6    0xA6
#define SIO_CRA7    0xA7
#define SIO_CRA8    0xA8
#define SIO_CRA9    0xA9
#define SIO_CRAA    0xAA
#define SIO_CRAB    0xAB
#define SIO_CRAC    0xAC
#define SIO_CRAD    0xAD
#define SIO_CRAE    0xAE
#define SIO_CRAF    0xAF
#define SIO_CRB0    0xB0
#define SIO_CRB1    0xB1
#define SIO_CRB2    0xB2
#define SIO_CRB3    0xB3
#define SIO_CRB4    0xB4
#define SIO_CRB5    0xB5
#define SIO_CRB6    0xB6
#define SIO_CRB7    0xB7
#define SIO_CRB8    0xB8
#define SIO_CRB9    0xB9
#define SIO_CRBA    0xBA
#define SIO_CRBB    0xBB
#define SIO_CRBC    0xBC
#define SIO_CRBD    0xBD
#define SIO_CRBE    0xBE
#define SIO_CRBF    0xBF
#define SIO_CRC0    0xC0
#define SIO_CRC1    0xC1
#define SIO_CRC2    0xC2
#define SIO_CRC3    0xC3
#define SIO_CRC4    0xC4
#define SIO_CRC5    0xC5
#define SIO_CRC6    0xC6
#define SIO_CRC7    0xC7
#define SIO_CRC8    0xC8
#define SIO_CRC9    0xC9
#define SIO_CRCA    0xCA
#define SIO_CRCB    0xCB
#define SIO_CRCC    0xCC
#define SIO_CRCD    0xCD
#define SIO_CRCE    0xCE
#define SIO_CRCF    0xCF
#define SIO_CRD0    0xD0
#define SIO_CRD1    0xD1
#define SIO_CRD2    0xD2
#define SIO_CRD3    0xD3
#define SIO_CRD4    0xD4
#define SIO_CRD5    0xD5
#define SIO_CRD6    0xD6
#define SIO_CRD7    0xD7
#define SIO_CRD8    0xD8
#define SIO_CRD9    0xD9
#define SIO_CRDA    0xDA
#define SIO_CRDB    0xDB
#define SIO_CRDC    0xDC
#define SIO_CRDD    0xDD
#define SIO_CRDE    0xDE
#define SIO_CRDF    0xDF
#define SIO_CRE0    0xE0
#define SIO_CRE1    0xE1
#define SIO_CRE2    0xE2
#define SIO_CRE3    0xE3
#define SIO_CRE4    0xE4
#define SIO_CRE5    0xE5
#define SIO_CRE6    0xE6
#define SIO_CRE7    0xE7
#define SIO_CRE8    0xE8
#define SIO_CRE9    0xE9
#define SIO_CREA    0xEA
#define SIO_CREB    0xEB
#define SIO_CREC    0xEC
#define SIO_CRED    0xED
#define SIO_CREE    0xEE
#define SIO_CREF    0xEF
#define SIO_CRF0    0xF0
#define SIO_CRF1    0xF1
#define SIO_CRF2    0xF2
#define SIO_CRF3    0xF3
#define SIO_CRF4    0xF4
#define SIO_CRF5    0xF5
#define SIO_CRF6    0xF6
#define SIO_CRF7    0xF7
#define SIO_CRF8    0xF8
#define SIO_CRF9    0xF9
#define SIO_CRFA    0xFA
#define SIO_CRFB    0xFB
#define SIO_CRFC    0xFC
#define SIO_CRFD    0xFD
#define SIO_CRFE    0xFE
#define SIO_CRFF    0xFF

#endif
