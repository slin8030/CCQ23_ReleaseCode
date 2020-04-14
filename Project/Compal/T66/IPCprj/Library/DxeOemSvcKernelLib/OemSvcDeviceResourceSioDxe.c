/** @file
  Modify resources befer installing protocol.

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Protocol/CpuIo2.h>
#include <Library/SioLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/GpioLib.h>
#include <Library/ScPlatformLib.h>

#include <SetupConfig.h>

SYSTEM_CONFIGURATION                  mSetupConfig;

//
// UART mode: RS232/422/485
//
typedef enum {
  RS232     = 0x00,
  RS422     = 0x01,
  RS485     = 0x02,
  MAX_MODE  = 0x03
};

typedef enum {
  COM1   ,
  COM2   ,
  COM3   ,
  MAX_COM_DEV
} SCU_DEVICE;

//
// LOGICAL DEVICE NUMBER LIST:
//
typedef enum {
  UARTA     = 0x02,
  UARTB     = 0x03,
  GPIO_0    = 0x07,
  WDT1      = 0x08,
  GPIO_1    = 0x0F,
  UARTC     = 0x10,
  UARTD     = 0x11,
  PORT80    = 0x14,
  NOT_FUND  = 0xFF
} LOGICAL_DEVICE_NUM;

#define CONFIG_PIN_NUMBER  2
BXT_GPIO_PAD_INIT  mCOM_MODE_GPIO[MAX_COM_DEV][MAX_MODE][CONFIG_PIN_NUMBER] =
{
  /*                  Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled ,Term_H_L, Inverted,GPI_ROUT, IOSstae,IOSTerm,     MMIO_Offset   ,Community */
  {//COM1_MODE:
    {// RS232 {0,1}
      BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0058,  NORTH),//COM1_UARTA_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0060,  NORTH),//COM1_UARTA_M1#
    },
    {// RS422 {0,0}
      BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0058,  NORTH),//COM1_UARTA_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0060,  NORTH),//COM1_UARTA_M1#
    },
    {// RS485 {1,0}
      BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0058,  NORTH),//COM1_UARTA_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0060,  NORTH),//COM1_UARTA_M1#
    },
  },
  {//COM2_MODE:
    {// RS232 {0,1}
      BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPu, GPIO_PADBAR+0x0068,  NORTH),//COM2_UARTB_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_29",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPu, GPIO_PADBAR+0x00E8,  NORTH),//COM2_UARTB_M1#
    },
    {// RS422 {0,0}
      BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPu, GPIO_PADBAR+0x0068,  NORTH),//COM2_UARTB_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_29",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPu, GPIO_PADBAR+0x00E8,  NORTH),//COM2_UARTB_M1#
    },
    {// RS485 {1,0}
      BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPu, GPIO_PADBAR+0x0068,  NORTH),//COM2_UARTB_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_29",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPu, GPIO_PADBAR+0x00E8,  NORTH),//COM2_UARTB_M1#
    },
  },
  {//COM3_MODE:
    {// RS232 {0,1}
      BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPd, GPIO_PADBAR+0x0078,  NORTH),//COM3_UARTC_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPd, GPIO_PADBAR+0x0080,  NORTH),//COM3_UARTC_M1#
    },
    {// RS422 {0,0}
      BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPd, GPIO_PADBAR+0x0078,  NORTH),//COM3_UARTC_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPd, GPIO_PADBAR+0x0080,  NORTH),//COM3_UARTC_M1#
    },
    {// RS485 {1,0}
      BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPd, GPIO_PADBAR+0x0078,  NORTH),//COM3_UARTC_M0#
      BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     EnPd, GPIO_PADBAR+0x0080,  NORTH),//COM3_UARTC_M1#
    },
  }
};
/*
STATIC BXT_GPIO_PAD_INIT  mCOM_PULL_GPIO[3][2] =
{
  {// COM1 PULL Resistor Disable(High)/ Enable(Low)
    BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0088,  NORTH),//COM1_422_485_SW#
    BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0088,  NORTH),//COM1_422_485_SW#
  },
  {// COM2 PULL Resistor Disable(High)/ Enable(Low)
    BXT_GPIO_PAD_CONF(L"GPIO_18",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0090,  NORTH),//COM2_422_485_SW#
    BXT_GPIO_PAD_CONF(L"GPIO_18",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0090,  NORTH),//COM2_422_485_SW#
  },
  {// COM3 PULL Resistor Disable(High)/ Enable(Low)
    BXT_GPIO_PAD_CONF(L"GPIO_19",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0098,  NORTH),//COM3_422_485_SW#
    BXT_GPIO_PAD_CONF(L"GPIO_19",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0098,  NORTH),//COM3_422_485_SW#
  }
};*/

STATIC BXT_GPIO_PAD_INIT  mCOM_TERMINATION_GPIO[3][2] =
{
  {// COM1 Termination Resistor :Disable(High)/ Enable(Low)
    BXT_GPIO_PAD_CONF(L"GPIO_32",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0100,  NORTH),//COM1_TERMA#
    BXT_GPIO_PAD_CONF(L"GPIO_32",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x0100,  NORTH),//COM1_TERMA#
  },
  {// COM2 Termination Resistor :Disable(High)/ Enable(Low)
    BXT_GPIO_PAD_CONF(L"GPIO_21",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x00A8,  NORTH),//COM2_TERMB#
    BXT_GPIO_PAD_CONF(L"GPIO_21",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x00A8,  NORTH),//COM2_TERMB#
  },
  {// COM3 Termination Resistor :Disable(High)/ Enable(Low)
    BXT_GPIO_PAD_CONF(L"GPIO_27",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x00D8,  NORTH),//COM3_TERMC#
    BXT_GPIO_PAD_CONF(L"GPIO_27",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H ,   NA    ,    NA,     NA   ,     SAME, GPIO_PADBAR+0x00D8,  NORTH),//COM3_TERMC#
  }
};



#define   DEVICE_INSTANCE(a, b)  a<<8|b

#define   UPDATE_PCD_POINTER(COMPORT_NUM, PCDPTR, DMI_PCD_PTR)        \
            do {                                                      \
              if (mSetupConfig.##COMPORT_NUM != SELECT_AUTO) {        \
                PCDPTR->DeviceEnable  = mSetupConfig.##COMPORT_NUM;   \
                PCDPTR->DeviceBaseAdr = GetBaseAddress(mSetupConfig.##COMPORT_NUM##BaseIo); \
                PCDPTR->DeviceIrq = mSetupConfig.##COMPORT_NUM##Interrupt;\
              } else if (mSetupConfig.##COMPORT_NUM == SELECT_DISABLE) {  \
                PCDPTR->DeviceDma = NULL_ID;                            \
              }                                                         \
              if (DMI_PCD_PTR != NULL) {                                \
                DMI_PCD_PTR += *(DMI_PCD_PTR + 1);                      \
                if (*DMI_PCD_PTR == SUBTYPE2) {                         \
                  *(DMI_PCD_PTR+2) = mSetupConfig.##COMPORT_NUM;        \
                  if (mSetupConfig.##COMPORT_NUM##Mode == RS232) {      \
                    *(DMI_PCD_PTR+3) = 0;                               \
                  }else {                                               \
                    *(DMI_PCD_PTR+3) = 1;                               \
                  }                                                     \
                }                                                       \
              }                                                         \
            }while (0)

UINT16
GetBaseAddress (
  IN UINT8  BaseAddressSelect
  )
{
  UINT16   AddressTable[] = {0x2E8, 0x2F8, 0x3E8, 0x3F8, 0x338, 0x228, 0x240, 0x248, 0x250, 0x258, 0x378, 0x388};
  UINT16  BaseAddress;

  if (BaseAddressSelect >= sizeof(AddressTable)/sizeof(AddressTable[0])) {
    BaseAddress = AddressTable[0];
  }

  BaseAddress = AddressTable[BaseAddressSelect];
  return BaseAddress;
}

/**
  Config UART setting: RS232/422/485
**/
LOGICAL_DEVICE_NUM
EFIAPI
ChangeToLogical(
  IN UINT8          Device,
  IN UINT8          DeviceInstance
  )
{
  LOGICAL_DEVICE_NUM         LogicalNum;

  switch (DEVICE_INSTANCE(Device, DeviceInstance)) {
    case DEVICE_INSTANCE(COM, DEVICE_INSTANCE0):
      LogicalNum = UARTA;
      break;

    case DEVICE_INSTANCE(COM, DEVICE_INSTANCE1):
      LogicalNum = UARTB;
      break;

    case DEVICE_INSTANCE(COM, DEVICE_INSTANCE2):
      LogicalNum = UARTC;
      break;

    case DEVICE_INSTANCE(COM, DEVICE_INSTANCE3):
      LogicalNum = UARTD;
      break;

    case DEVICE_INSTANCE(SGPIO, DEVICE_INSTANCE0):
      LogicalNum = GPIO_0;
      break;

    case DEVICE_INSTANCE(SGPIO, DEVICE_INSTANCE1):
      LogicalNum = GPIO_1;
      break;

    case DEVICE_INSTANCE(WDT, DEVICE_INSTANCE0):
      LogicalNum = WDT1;
      break;

    default:
      LogicalNum = NOT_FUND;
      break;
  }
  return LogicalNum;
}

/**
  Config UART setting: RS232/422/485
**/
VOID
EFIAPI
UpdatePcdData(
  IN OUT SIO_DEVICE_LIST_TABLE        *PcdPointer,
  IN OUT UINT8                        *PcdDmiPointer
  )
{
  UINT8   *DmiTablePtr;

  //
  // Update PcdPointer
  //
  if ((PcdPointer->TypeH == 0xC4) && (PcdPointer->TypeL == 0x52)) {
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      DmiTablePtr = PcdDmiPointer;
      FindExtensiveDevice (
        &DmiTablePtr,
        PcdPointer->TypeInstance,
        PcdPointer->Device,
        PcdPointer->DeviceInstance
      );
      switch (DEVICE_INSTANCE(PcdPointer->Device, PcdPointer->DeviceInstance)) {
        case DEVICE_INSTANCE(COM, DEVICE_INSTANCE0):
          UPDATE_PCD_POINTER(ComPortA, PcdPointer, DmiTablePtr);
          break;

        case DEVICE_INSTANCE(COM, DEVICE_INSTANCE1):
          UPDATE_PCD_POINTER(ComPortB, PcdPointer, DmiTablePtr);
          break;

        case DEVICE_INSTANCE(COM, DEVICE_INSTANCE2):
          UPDATE_PCD_POINTER(ComPortC, PcdPointer, DmiTablePtr);
          break;

        case DEVICE_INSTANCE(COM, DEVICE_INSTANCE3):
          UPDATE_PCD_POINTER(ComPortD, PcdPointer, DmiTablePtr);
          break;
      }
      PcdPointer++;
    }
  }
}

/**
  Config UART setting: RS232/422/485
**/
EFI_STATUS
EFIAPI
ConfigCOM_GPIO(
  )
{
  GpioPadConfigTable(CONFIG_PIN_NUMBER, &mCOM_MODE_GPIO[COM1][mSetupConfig.ComPortAMode][0]);
//  GpioPadConfigTable(1, &mCOM_PULL_GPIO[COM1][mSetupConfig.ComPortA_PULL]);
  GpioPadConfigTable(1, &mCOM_TERMINATION_GPIO[COM1][mSetupConfig.ComPortA_Termination]);

  GpioPadConfigTable(CONFIG_PIN_NUMBER, &mCOM_MODE_GPIO[COM2][mSetupConfig.ComPortBMode][0]);
//  GpioPadConfigTable(1, &mCOM_PULL_GPIO[COM2][mSetupConfig.ComPortB_PULL]);
  GpioPadConfigTable(1, &mCOM_TERMINATION_GPIO[COM2][mSetupConfig.ComPortB_Termination]);

  GpioPadConfigTable(CONFIG_PIN_NUMBER, &mCOM_MODE_GPIO[COM3][mSetupConfig.ComPortCMode][0]);
//  GpioPadConfigTable(1, &mCOM_PULL_GPIO[COM3][mSetupConfig.ComPortC_PULL]);
  GpioPadConfigTable(1, &mCOM_TERMINATION_GPIO[COM3][mSetupConfig.ComPortC_Termination]);

  return EFI_SUCCESS;
}

/**
  Before install device protocol, it can change value to PCD data, per device info in PCD

  @param[in]       SioInstance      Which SIO pass in.
  @param[IN, OUT]  *PcdPointer      PCD data
  @param[IN]       SioConfig        The index port of the configuration of the SIO

  @retval EFI_UNSUPPORTED           Returns unsupported by default.
  @retval EFI_SUCCESS               The service is customized in the project.
  @retval EFI_MEDIA_CHANGED         The value of IN OUT parameter is changed.
  @retval Others                    Depends on customization.
**/
EFI_STATUS
OemSvcDeviceResourceSioDxe (
  IN EFI_SIO_RESOURCE_FUNCTION        *SioResourceFunction,
  IN OUT SIO_DEVICE_LIST_TABLE        *PcdPointer,
  IN OUT UINT8                        *DmiTablePtr
  )
{
 /*++
    Todo:
      Add project specific code in here.
  --*/
  EFI_STATUS                            Status;
  UINTN                                 SetupSize;

  SetupSize = sizeof (SYSTEM_CONFIGURATION);
  Status = gRT->GetVariable(
    L"Setup",
    &gSystemConfigurationGuid,
    NULL,
    &SetupSize,
    &mSetupConfig
    );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  if ((SioResourceFunction->TypeIdHigh == 0xC4) && (SioResourceFunction->TypeIdLow == 0x52)) {
    mSetupConfig.NCT5104 = 1;
  }

  PchLpcGenIoRangeSet(0x3E8, 0x08);       // Add IO decode resourec

  UpdatePcdData(PcdPointer,DmiTablePtr);
  ConfigCOM_GPIO();

  Status =  gRT->SetVariable (
                  L"Setup",
                  &gSystemConfigurationGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  SetupSize,
                  &mSetupConfig
                  );
  return EFI_MEDIA_CHANGED;
}
