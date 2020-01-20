/** @file

Declaration file for NV Data

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#ifndef _SIO_IT8987E_HII_RESOURCE_NV_DATA_H_
#define _SIO_IT8987E_HII_RESOURCE_NV_DATA_H_

#include <SioCommon.h>

//
// Used by VFR for form or button identification
//
#define SIO_CONFIGURATION_VARSTORE_ID     0x4000
#define SIO_CONFIGURATION_FORM_ID         0x4001
#define SIO_HARDWARE_MONITOR_FORM_ID      0x4002
#define SIO_LED_CONFIG_FORM_ID            0x4003
#define SIO_GPIO0X_FORM_ID                0x4005
#define SIO_GPIO1X_FORM_ID                0x4006
#define SIO_GPIO2X_FORM_ID                0x4007
#define SIO_GPIO3X_FORM_ID                0x4008
#define SIO_GPIO4X_FORM_ID                0x4009
#define SIO_GPIO5X_FORM_ID                0x400A
#define SIO_GPIO6X_FORM_ID                0x400B
#define SIO_GPIO7X_FORM_ID                0x400C
#define SIO_GPIO8X_FORM_ID                0x400D
#define SIO_PME_CONFIG_FORM_ID            0x4010

#define SIO_HWM_QUESTION_ID               0x4350

#define SIO_VOL_LABEL                     0x4400
#define SIO_UPDATE_VOL_QUESTION_ID        0x4401
#define SIO_TEMP_LABEL                    0x4500
#define SIO_UPDATE_TEMP_QUESTION_ID       0x4501
#define SIO_FAN_LABEL                     0x4600
#define SIO_UPDATE_FAN_QUESTION_ID        0x4601
#define SIO_LABEL_END                     0x4700

#define COM1_MODE_LABEL                   0x4100
#define COM2_MODE_LABEL                   0x4101
#define COM3_MODE_LABEL                   0x4102
#define COM4_MODE_LABEL                   0x4103
#define COM5_MODE_LABEL                   0x4104
#define COM6_MODE_LABEL                   0x4105
#define COM_MODE_QUESTION_ID              0x4106

#define COM1_FIFO_LABEL                   0x4110
#define COM2_FIFO_LABEL                   0x4111
#define COM3_FIFO_LABEL                   0x4112
#define COM4_FIFO_LABEL                   0x4113
#define COM5_FIFO_LABEL                   0x4114
#define COM6_FIFO_LABEL                   0x4115
#define COM_FIFO_QUESTION_ID              0x4116

//
// From 0x4130 to 0x4147
//
#define COM1_ACPI_IRQ_LABEL               0x4130
#define COM2_ACPI_IRQ_LABEL               0x4131
#define COM3_ACPI_IRQ_LABEL               0x4132
#define COM4_ACPI_IRQ_LABEL               0x4133
#define COM5_ACPI_IRQ_LABEL               0x4134
#define COM6_ACPI_IRQ_LABEL               0x4135
#define COM_ACPI_IRQ_QUESTION_ID          0x4136

//
// From 0x4200 to 0x4203
//
#define LPT1_MODE_LABEL                   0x4200
#define LPT2_MODE_LABEL                   0x4201
#define LPT1_MODE_QUESTION_ID             0x4202

//
// From 0x4210 to 0x4212
//
#define SIO_WDT_LABEL                     0x4210
#define SIO_WDT_QUESTION_ID               0x4211

#define SIO_LED0_LABEL                    0x4900
#define SIO_LED1_LABEL                    0x4901
#define SIO_LED_QUESTION_ID               0x4902

#define SIO_LED_ENABLE_QUESTION_ID             0x4920
#define SIO_ADDITIONAL_FEATURE_LABEL           0x4928
#define SIO_LED_ADDITIONAL_FEATURE_QUESTION_ID 0x4929

#define SIO_FADING_LED_LABEL              0x4930
#define SIO_FADING_LED_QUESTION_ID        0x4931

#define SIO_POWER_STATE_LED_LABEL         0x4940
#define SIO_POWER_STATE_LED_QUESTION_ID   0x4941

#define SIO_GPIO0X_LABEL                  0x5000
#define SIO_GPIO1X_LABEL                  0x5001
#define SIO_GPIO2X_LABEL                  0x5002
#define SIO_GPIO3X_LABEL                  0x5003
#define SIO_GPIO4X_LABEL                  0x5004
#define SIO_GPIO5X_LABEL                  0x5005
#define SIO_GPIO6X_LABEL                  0x5006
#define SIO_GPIO7X_LABEL                  0x5007
#define SIO_GPIO8X_LABEL                  0x5008
#define SIO_GPIO_RESISTANCE_QUESTION_ID   0x5009
#define SIO_GPIO_RESISTANCE2_QUESTION_ID  0x5048
#define SIO_GPIO_POLARITY_QUESTION_ID     0x5088
#define SIO_GPIO_IN_OUT_QUESTION_ID       0x5108
#define SIO_GPIO_OUT_VAL_QUESTION_ID      0x5148
#define SIO_GPIO_OUT2_VAL_QUESTION_ID     0x5188

#define MAX_SIO_GPIO                      0x40

//
// Power Loss
//
#define SIO_POWER_LOSS_LABEL                0x6000
#define SIO_POWER_LOSS_QUESTION_ID          0x6001
#define SIO_POWER_LOSS_USER_DEF_QUESTION_ID 0x6002

//
// PME
//
#define SIO_PME_LABEL                       0x6100
#define SIO_PME_QUESTION_ID                 0x6101

#define MAX_SIO_PME                         0x03

//
// HWM 0x6200~0x6A2F
//
#define SIO_HWM_LABEL                                         0x6200
#define SIO_HWM_FAN_MODE_QUESTION_ID                          0x6210
#define SIO_HWM_MANUAL_OUTPUT_QUESTION_ID                     0x6220
#define SIO_HWM_THERMAL_CRUISE_TARGET_TEMPERATURE_QUESTION_ID 0x6300
#define SIO_HWM_THERMAL_CRUISE_TOLERANCE_QUESTION_ID          0x6310
#define SIO_HWM_SPEED_CRUISE_TARGET_SPEED_COUNT_QUESTION_ID   0x6400
#define SIO_HWM_SPEED_CRUISE_TOLERANCE_QUESTION_ID            0x6410
#define SIO_HWM_FAN_III_TARGET_TEMPERATURE_QUESTION_ID        0x6500
#define SIO_HWM_FAN_III_TOLERANCE_QUESTION_ID                 0x6510
#define SIO_HWM_FAN_III_INIT_OUTPUT_QUESTION_ID               0x6520
#define SIO_HWM_FAN_III_OUTPUT_STEP_QUESTION_ID               0x6530
#define SIO_HWM_FAN_IV_BOUNDARY_QUESTION_ID                   0x6600
#define SIO_HWM_FAN_IV_OUTPUT_VALUE_QUESTION_ID               0x6680
#define SIO_HWM_FAN_LINEAR_BOUNDARY_QUESTION_ID               0x6700
#define SIO_HWM_FAN_LINEAR_OUTPUT_VALUE_QUESTION_ID           0x6780
#define SIO_HWM_FAN_STAGE_BOUNDARY_QUESTION_ID                0x6800
#define SIO_HWM_FAN_STAGE_OUTPUT_VALUE_QUESTION_ID            0x6880
#define SIO_HWM_GUARDIAN_START_TEMPERATURE_QUESTION_ID        0x6900
#define SIO_HWM_GUARDIAN_OFF_TEMPERATURE_QUESTION_ID          0x6910
#define SIO_HWM_GUARDIAN_START_PWM_QUESTION_ID                0x6920
#define SIO_HWM_GUARDIAN_SLOPE_PWM_QUESTION_ID                0x6930
#define SIO_HWM_FAN_AUTO_LOW_TEMPERATURE_LIMIT_QUESTION_ID    0x6A00
#define SIO_HWM_FAN_AUTO_PWM_MINIMUM_QUESTION_ID              0x6A10
#define SIO_HWM_FAN_AUTO_TEMPERATURE_RANGE_QUESTION_ID        0x6A20

//
// EFI Variable attributes
//
#define EFI_VARIABLE_NON_VOLATILE         0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS   0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS       0x00000004
#define EFI_VARIABLE_READ_ONLY            0x00000008

//
// VarOffset that will be used to create question
// all these values are computed from the structure
// defined below
//
#define VAR_OFFSET(Field)          ((UINT16) ((UINTN) &(((SIO_CONFIGURATION*) 0)->Field)))
#define SIO_HWM_OFFSET             VAR_OFFSET (HwmConfig)

#define FIFO_SETTING_OFFSET(Field) ((UINT16) ((UINTN) &(((SIO_FIFO_SETTING*) 0)->Field)))
#define IRQ_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SIO_ACPI_IRQ_INFO*) 0)->Field)))
#define WDT_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SIO_WDT_CONFIGURATION*) 0)->Field)))
#define LED_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SIO_LED_CONFIGURATION*) 0)->Field)))
#define FADING_LED_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SIO_FADING_LED_CONFIGURATION*) 0)->Field)))
#define POWER_LOSS_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((POWER_LOSS_CONFIGURATION*) 0)->Field)))
#define PME_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((PME_CONFIGURATION*) 0)[Field])))
#define HWM_SETTING_OFFSET(Field)              ((UINT16) ((UINTN) &(((SIO_SMART_FAN*) 0)->Field)))
#define MANUAL_SETTING_OFFSET(Field)           ((UINT16) ((UINTN) &(((MANUAL_MODE*) 0)->Field)))
#define THERMAL_CRUISE_SETTING_OFFSET(Field)   ((UINT16) ((UINTN) &(((THERMAL_CRUISE*) 0)->Field)))
#define SPEED_CRUISE_SETTING_OFFSET(Field)     ((UINT16) ((UINTN) &(((SPEED_CRUISE*) 0)->Field)))
#define SMART_FAN_III_SETTING_OFFSET(Field)    ((UINT16) ((UINTN) &(((SMART_FAN_III*) 0)->Field)))
#define SMART_FAN_IV_SETTING_OFFSET(Field)     ((UINT16) ((UINTN) &(((SMART_FAN_IV*) 0)->Field)))
#define SMART_FAN_LINEAR_SETTING_OFFSET(Field) ((UINT16) ((UINTN) &(((SMART_FAN_LINEAR*) 0)->Field)))
#define SMART_FAN_STAGE_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SMART_FAN_STAGE*) 0)->Field)))
#define GUARDIAN_SETTING_OFFSET(Field)         ((UINT16) ((UINTN) &(((SMART_FAN_GUARDIAN*) 0)->Field)))
#define SMART_FAN_AUTO_SETTING_OFFSET(Field)   ((UINT16) ((UINTN) &(((SMART_FAN_AUTO*) 0)->Field)))

#define VAR_GPIO_OFFSET(Field)     ((UINT16) ((UINTN) &(((SIO_CONFIGURATION *) 0)->Field)))
#define SIO_GPIO10_OFFSET          VAR_GPIO_OFFSET (GpioVariable.GpVariable[0])

#define HWM_OFFSET(Field)          ((UINT16) ((UINTN) &(((SIO_HWM_CONFIGURATION*) 0)->Field)))
#define SIO_VOL1_HWM_OFFSET        HWM_OFFSET (VolData1)
#define SIO_VOL2_HWM_OFFSET        HWM_OFFSET (VolData2)
#define SIO_VOL3_HWM_OFFSET        HWM_OFFSET (VolData3)
#define SIO_VOL4_HWM_OFFSET        HWM_OFFSET (VolData4)
#define SIO_VOL5_HWM_OFFSET        HWM_OFFSET (VolData5)
#define SIO_VOL6_HWM_OFFSET        HWM_OFFSET (VolData6)
#define SIO_VOL7_HWM_OFFSET        HWM_OFFSET (VolData7)
#define SIO_TEMP1_HWM_OFFSET       HWM_OFFSET (TempData1)
#define SIO_TEMP2_HWM_OFFSET       HWM_OFFSET (TempData2)
#define SIO_TEMP3_HWM_OFFSET       HWM_OFFSET (TempData3)
#define SIO_FAN1_HWM_OFFSET        HWM_OFFSET (FanData1)
#define SIO_FAN2_HWM_OFFSET        HWM_OFFSET (FanData2)
#define SIO_FAN3_HWM_OFFSET        HWM_OFFSET (FanData3)

//
// Nv Data structure referenced by IFR
//
#pragma pack(1)

typedef struct {
  UINT8  Enable;
  UINT8  Level;
} SIO_FIFO_SETTING;

typedef struct {
  UINT8  IrqNum;
  UINT8  WakeCapability;
  UINT8  IntShare;
  UINT8  InterruptPolarity;
  UINT8  IntMode;
} SIO_ACPI_IRQ_INFO;

typedef struct {
  UINT16  VolData1[11];
  UINT16  VolData2[11];
  UINT16  VolData3[11];
  UINT16  VolData4[11];
  UINT16  VolData5[11];
  UINT16  VolData6[11];
  UINT16  VolData7[11];
  UINT16  TempData1[18];
  UINT16  TempData2[18];
  UINT16  TempData3[18];
  UINT16  FanData1[10];
  UINT16  FanData2[10];
  UINT16  FanData3[10];
  UINT8   Enable;
} SIO_HWM_CONFIGURATION;

typedef struct {
  UINT8  OutputValue;
} MANUAL_MODE;

typedef struct {
  UINT8  TargetTemperature;
  UINT8  Tolerance;
} THERMAL_CRUISE;

typedef struct {
  UINT16 TargetSpeedCount;
  UINT8  Tolerance;
} SPEED_CRUISE;

typedef struct {
  UINT8  TargetTemperature;
  UINT8  Tolerance;
  UINT8  InitOutput;
  UINT8  OutputStep;
} SMART_FAN_III;

typedef struct {
  UINT8  Boundary[6];
  UINT8  OutputValue[6];
} SMART_FAN_IV;

typedef struct {
  UINT8  Boundary[6];
  UINT8  OutputValue[6];
} SMART_FAN_LINEAR;

typedef struct {
  UINT8  Boundary[4];
  UINT8  OutputValue[5];
} SMART_FAN_STAGE;

typedef struct {
  UINT8  StartTemperature;
  UINT8  OffTemperature;
  UINT8  StartPwm;
  UINT8  SlopePwm;
} SMART_FAN_GUARDIAN;

typedef struct {
  UINT8  LowTemperatureLimit;
  UINT8  PwmMinimum;
  UINT8  TemperatureRange;
} SMART_FAN_AUTO;

typedef struct {
  UINT8               SmartFanMode;
  MANUAL_MODE         ManualMode;
  THERMAL_CRUISE      ThermalCruise;
  SPEED_CRUISE        SpeedCruise;
  SMART_FAN_III       SmartFanIII;
  SMART_FAN_IV        SmartFanIV;
  SMART_FAN_GUARDIAN  SmartGuardian;
} SIO_SMART_FAN;

typedef struct {
  UINT8   TimerCountMode;
  UINT16  TimeOutValue;
} SIO_WDT_CONFIGURATION;

typedef struct _SIO_LED_DEFAULT_SETTING {
  UINT8 S0DefaultSetting;
  UINT8 S3DefaultSetting;
  UINT8 S5DefaultSetting;
  UINT8 DeepS3DefaultSetting;
  UINT8 DeepS5DefaultSetting;
} SIO_LED_DEFAULT_SETTING;

typedef struct _SIO_LED_ENABLE_SETTING {
  UINT8 EnableUnderS3ToS5;
  UINT8 EnableDeepS3;
  UINT8 EnableDeepS5;
} SIO_LED_ENABLE_DEFAULT_SETTING;

typedef struct {
  SIO_LED_DEFAULT_SETTING        LedDefaultSetting[2];
  SIO_LED_ENABLE_DEFAULT_SETTING LedEnable[2];
  UINT8                          AdditionalFeatureSetting[5];
} SIO_LED_CONFIGURATION;

typedef struct {
  UINT8 FadingMaxVal;
  UINT8 FadingMidVal;
  UINT8 FadingMinVal;
  UINT8 FadingIncVal;
  UINT8 FadingDecVal;
  UINT8 FadingRepVal;
} SIO_FADING_LED_DUTY_CYCLE;

typedef struct {
  UINT8 BlinkModeFadingClk;
  UINT8 FadingClkDivide;
  UINT8 FadingFreq;
} SIO_FADING_LED_CONTROL;

typedef struct {
  UINT8                     DisplayModeSelect;
  SIO_FADING_LED_DUTY_CYCLE DutyCycle;
  UINT8                     ExtendSelect;
  UINT8                     LightExtendTime;
  UINT8                     DarkExtendTime;
  SIO_FADING_LED_CONTROL    FadingControl;
} SIO_FADING_LED_CONFIGURATION;

typedef struct {
  UINT8  Resistance;
  UINT8  TriState;
  UINT8  Invert;
  UINT8  InOut;
  UINT8  Output;
  UINT8  Output2;
} GPIO_VARIABLE;

typedef struct {
  GPIO_VARIABLE     GpVariable[MAX_SIO_GPIO];
} SIO_GPIO_CONFIGURATION;

typedef struct {
  UINT8 Setting;
  UINT8 UserDefineOnOff;
} POWER_LOSS_CONFIGURATION;

typedef struct {
  UINT8 Setting;
} PME_CONFIGURATION;

typedef struct {
  SIO_DEVICE_LIST_TABLE DeviceA; ///< Com1
  SIO_DEVICE_LIST_TABLE DeviceB; ///< Com2
  SIO_DEVICE_LIST_TABLE DeviceL; ///< CIR
  UINT8                  AutoUpdateNum; ///< Count the number of the auto update SIO_DEVICE_LIST_TABLE
  UINT8                  Com1Mode;
  UINT8                  Com2Mode;
  SIO_FIFO_SETTING       Com1Fifo;
  SIO_FIFO_SETTING       Com2Fifo;
  SIO_ACPI_IRQ_INFO      ComIrq[2];
  BOOLEAN                  NotFirstBoot;
} SIO_CONFIGURATION;
#pragma pack()

#endif
