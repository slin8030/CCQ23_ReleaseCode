/** @file

Init SIO in PEI stage.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#include "SioInitPei.h"

UINT16  mSioCfgPortList[]   = {0x2E, 0x4E, 0x162E, 0x164E};
UINT16  mSioCfgPortListSize = sizeof (mSioCfgPortList) / sizeof (UINT16);
UINT8   EnterConfig         = ENTER_CONFIG;
UINT8   ExitConfig          = EXIT_CONFIG;

EFI_PEI_NOTIFY_DESCRIPTOR PeiSioInitNotifyList[] = {
  {
    0,
    NULL,
    NULL
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
  IoWrite8 (ConfigPort, ExitConfig);
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
      IDW8 (SIO_LDN, SIO_KEYBOARD, SioResourceFunction);  
      if (PcdPointer->DeviceEnable) {
        IDW8 (SIO_BASE_IO_ADDR1_MSB, ((PS2_KB_MS >> 8) & 0xff), SioResourceFunction);
        IDW8 (SIO_BASE_IO_ADDR1_LSB, (PS2_KB_MS & 0xff), SioResourceFunction);
        IDW8 (SIO_BASE_IO_ADDR2_MSB, ((PS2_KB_MS >> 8) & 0xff), SioResourceFunction);
        IDW8 (SIO_BASE_IO_ADDR2_LSB, ((PS2_KB_MS & 0xff) + 4), SioResourceFunction);  
        IDW8 (SIO_IRQ_SET, 0x01, SioResourceFunction);
        IDW8 (SIO_DEV_ACTIVE, TRUE, SioResourceFunction);
      } else {
        IDW8 (SIO_DEV_ACTIVE, FALSE, SioResourceFunction);
      }
    }

    if ((PcdPointer->Device == MOUSE) && (PcdPointer->TypeInstance == SioResourceFunction->Instance)) {
      IDW8 (SIO_LDN, SIO_MOUSE, SioResourceFunction);  
      if (PcdPointer->DeviceEnable) {
        IDW8 (SIO_IRQ_SET, 0x0C, SioResourceFunction);
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
SioDebugPortInit (
  IN UINT16 IndexPort
  )
{
  
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SioSpecificInit (
  IN UINT16 IndexPort
  )
{

  return EFI_UNSUPPORTED;
}
