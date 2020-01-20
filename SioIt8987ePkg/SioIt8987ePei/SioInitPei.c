/** @file

Init SIO in PEI stage.

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

#include "SioInitPei.h"

CHAR16  *mSioVariableName   = (CHAR16*) PCD_SIO_VARIABLE_NAME;
UINT16  mSioCfgPortList[]   = {0x2E, 0x4E, 0x162E, 0x164E};
UINT16  mSioCfgPortListSize = sizeof (mSioCfgPortList) / sizeof (UINT16);

EFI_PEI_NOTIFY_DESCRIPTOR PeiSioInitNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiReadOnlyVariable2PpiGuid,
    ProgramExtensiveDevicePei
  }
};

/**
 Enter SIO mode.

 @param[in]   ConfigPort             SIO config port.
**/
VOID
EnterConfigMode (
  IN UINT16  ConfigPort
  )
{
  IoWrite8 (ConfigPort, ENTER_CONFIG);
  IoWrite8 (ConfigPort, ENTER_CONFIG_2);
  IoWrite8 (ConfigPort, ENTER_CONFIG_3);

  if (ConfigPort == 0x2E) {
    IoWrite8 (ConfigPort, ENTER_CONFIG_3);
  } else {
    IoWrite8 (ConfigPort, ENTER_CONFIG_4);
  }
}

/**
 Exit SIO mode.

 @param[in]   ConfigPort             SIO config port.
**/
VOID
ExitConfigMode (
  IN UINT16  ConfigPort
  )
{
  IoWrite8 (ConfigPort    , EXIT_CONFIG);
  IoWrite8 (ConfigPort + 1, EXIT_CONFIG);
}

EFI_STATUS
SioInitPs2KBMS (
  IN UINT16 IndexPort,
  IN UINT8  SioInstance
  )
{
  SIO_DEVICE_LIST_TABLE            *PcdPointer;
  EFI_SIO_RESOURCE_FUNCTION        SioResourceFunction[] = { NULL_ID,
                                                             SIO_ID1,
                                                             SIO_ID2,
                                                             NULL_ID,
                                                             FALSE
                                                           };
  SioResourceFunction->SioCfgPort = IndexPort;
  SioResourceFunction->Instance   = SioInstance;
  PcdPointer                      = (SIO_DEVICE_LIST_TABLE *)PCD_SIO_CONFIG_TABLE;

  while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
    if ((PcdPointer->Device == KYBD) && (PcdPointer->TypeInstance == SioResourceFunction->Instance)) {
      IDW8 (SIO_LDN, SIO_KYBD, SioResourceFunction);

      if (PcdPointer->DeviceEnable) {
        IDW8 (SIO_BASE_IO_ADDR1_MSB, ((PS2_KB_MS >> 8) & 0xff), SioResourceFunction);
        IDW8 (SIO_BASE_IO_ADDR1_LSB, (PS2_KB_MS & 0xff), SioResourceFunction);
        IDW8 (SIO_BASE_IO_ADDR2_MSB, ((PS2_KB_MS >> 8) & 0xff), SioResourceFunction);
        IDW8 (SIO_BASE_IO_ADDR2_LSB, ((PS2_KB_MS & 0xff) + 4), SioResourceFunction);
        IDW8 (SIO_IRQ_SET, PS2_KB_IRQ, SioResourceFunction);
        IDW8 (SIO_DEV_ACTIVE, TRUE, SioResourceFunction);

      } else {
        IDW8 (SIO_DEV_ACTIVE, FALSE, SioResourceFunction);

      }
    }

    if ((PcdPointer->Device == MOUSE) && (PcdPointer->TypeInstance == SioResourceFunction->Instance)) {

      IDW8 (SIO_LDN, SIO_MOUSE, SioResourceFunction);
      if (PcdPointer->DeviceEnable) {
        IDW8 (SIO_IRQ_SET, PS2_MS_IRQ, SioResourceFunction);
        IDW8 (SIO_DEV_ACTIVE, TRUE, SioResourceFunction);
      } else {
        IDW8 (SIO_DEV_ACTIVE, FALSE, SioResourceFunction);
      }
    }
    PcdPointer++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SioInitCom1 (
  IN EFI_SIO_RESOURCE_FUNCTION *SioResourceFunction
  )
{
  IDW8 (SIO_LDN, SIO_COM1, SioResourceFunction);
  IDW8 (SIO_BASE_IO_ADDR1_MSB, (COM_PORT >> 8), SioResourceFunction);
  IDW8 (SIO_BASE_IO_ADDR1_LSB, (COM_PORT & 0xFF), SioResourceFunction);
  IDW8 (SIO_IRQ_SET, 0x04, SioResourceFunction);
  IDW8 (SIO_DEV_ACTIVE, TRUE, SioResourceFunction);
  return EFI_SUCCESS;
}

EFI_STATUS
SioDebugPortInit (
  IN UINT16 IndexPort
  )
{
  EFI_SIO_RESOURCE_FUNCTION        SioResourceFunction[] = { NULL_ID,
                                                             SIO_ID1,
                                                             SIO_ID2,
                                                             NULL_ID,
                                                             FALSE
                                                           };
  SioResourceFunction->SioCfgPort = IndexPort;

  SioInitCom1 (SioResourceFunction);
  return EFI_SUCCESS;
}

EFI_STATUS
SioSpecificInit (
  IN UINT16 IndexPort
  )
{
  //
  // not sure if these are necessary... I am commenting these out.
  //

  //UINT16 DataPort;

  //DataPort = IndexPort + 1;

  //
  // Set Interrupt Level Select for Environment Controller
  // Default value from 09h to 00h by ITE recommendation
  //
  //IoWrite8 (IndexPort, SIO_LDN);
  //IoWrite8 (DataPort, SIO_SWUC);
  //IoWrite8 (IndexPort, SIO_IRQ_SET);
  //IoWrite8 (DataPort, 0x00);

  return EFI_SUCCESS;
}
