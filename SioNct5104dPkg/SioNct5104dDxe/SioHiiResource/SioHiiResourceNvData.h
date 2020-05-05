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
*/

#ifndef _SIO_NTC5104D_HII_RESOURCE_NV_DATA_H_
#define _SIO_NTC5104D_HII_RESOURCE_NV_DATA_H_

#include <SioCommon.h>

//
// Used by VFR for form or button identification
//
#define SIO_NCT5104D_CONFIGURATION_VARSTORE_ID     0x4000
#define SIO_NCT5104D_CONFIGURATION_FORM_ID         0x4001
#define SIO_GPIO0X_FORM_ID                0x4005
#define SIO_GPIO1X_FORM_ID                0x4006
#define SIO_GPIO2X_FORM_ID                0x4007
#define SIO_GPIO3X_FORM_ID                0x4008
#define SIO_GPIO4X_FORM_ID                0x4009
#define SIO_GPIO5X_FORM_ID                0x400A
#define SIO_GPIO6X_FORM_ID                0x400B
#define SIO_GPIO7X_FORM_ID                0x400C

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

#define SIO_GPIO0X_LABEL                  0x5000
#define SIO_GPIO1X_LABEL                  0x5001
#define SIO_GPIO2X_LABEL                  0x5002
#define SIO_GPIO3X_LABEL                  0x5003
#define SIO_GPIO4X_LABEL                  0x5004
#define SIO_GPIO5X_LABEL                  0x5005
#define SIO_GPIO6X_LABEL                  0x5006
#define SIO_GPIO7X_LABEL                  0x5007
#define SIO_GPIO_RESISTANCE_QUESTION_ID   0x5008
#define SIO_GPIO_RESISTANCE2_QUESTION_ID  0x5158
#define SIO_GPIO_POLARITY_QUESTION_ID     0x52A8
#define SIO_GPIO_IN_OUT_QUESTION_ID       0x53F8
#define SIO_GPIO_OUT_VAL_QUESTION_ID      0x5548
#define SIO_GPIO_OUT2_VAL_QUESTION_ID     0x5698

#define MAX_SIO_GPIO                      0x18

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
#define VAR_OFFSET(Field)          ((UINT16) ((UINTN) &(((SIO_NCT5104D_CONFIGURATION*) 0)->Field)))
#define SIO_HWM_OFFSET             VAR_OFFSET (HwmConfig)

#define FIFO_SETTING_OFFSET(Field) ((UINT16) ((UINTN) &(((SIO_NCT5104D_FIFO_SETTING*) 0)->Field)))
#define IRQ_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SIO_NCT5104D_ACPI_IRQ_INFO*) 0)->Field)))
#define WDT_SETTING_OFFSET(Field)  ((UINT16) ((UINTN) &(((SIO_NCT5104D_WDT_CONFIGURATION*) 0)->Field)))

#define VAR_GPIO_OFFSET(Field)     ((UINT16) ((UINTN) &(((SIO_NCT5104D_CONFIGURATION *) 0)->Field)))
#define SIO_GPIO00_OFFSET          VAR_GPIO_OFFSET (GpioVariable.GpVariable[0])

//
// Nv Data structure referenced by IFR
//
#pragma pack(1)

typedef struct {
  UINT8  Enable;
  UINT8  Level;
} SIO_NCT5104D_FIFO_SETTING;

typedef struct {
  UINT8  IrqNum;
  UINT8  WakeCapability;
  UINT8  IntShare;
  UINT8  InterruptPolarity;
  UINT8  IntMode;
} SIO_NCT5104D_ACPI_IRQ_INFO;

typedef struct {
  UINT8   TimerCountMode;
  UINT16  TimeOutValue;
} SIO_NCT5104D_WDT_CONFIGURATION;

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
} SIO_NCT5104D_GPIO_CONFIGURATION;

typedef struct {
  SIO_DEVICE_LIST_TABLE DeviceA; ///< Com1
  SIO_DEVICE_LIST_TABLE DeviceB; ///< Com2
  SIO_DEVICE_LIST_TABLE DeviceC; ///< Com3
  SIO_DEVICE_LIST_TABLE DeviceD; ///< Com4
  SIO_DEVICE_LIST_TABLE DeviceM; ///< WDT
  SIO_DEVICE_LIST_TABLE DeviceO; ///< GPIO
  UINT8                             AutoUpdateNum; ///< Count the number of the auto update SIO_DEVICE_LIST_TABLE
  UINT8                             Com1Mode;
  UINT8                             Com2Mode;
  UINT8                             Com3Mode;
  UINT8                             Com4Mode;
  SIO_NCT5104D_FIFO_SETTING         Com1Fifo;
  SIO_NCT5104D_FIFO_SETTING         Com2Fifo;
  SIO_NCT5104D_FIFO_SETTING         Com3Fifo;
  SIO_NCT5104D_FIFO_SETTING         Com4Fifo;
  SIO_NCT5104D_ACPI_IRQ_INFO        ComIrq[4];
  SIO_NCT5104D_WDT_CONFIGURATION    WatchDogConfig;
  UINT8                             Gpio[MAX_SIO_GPIO];
  SIO_NCT5104D_GPIO_CONFIGURATION   GpioVariable;
  BOOLEAN                           NotFirstBoot;
} SIO_NCT5104D_CONFIGURATION;
#pragma pack()

#endif
