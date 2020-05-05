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
UINT8   EnterConfig         = ENTER_CONFIG;
UINT8   ExitConfig          = EXIT_CONFIG;

EFI_PEI_NOTIFY_DESCRIPTOR PeiSioInitNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
    &gEfiPeiReadOnlyVariable2PpiGuid,
    ProgramExtensiveDevicePei
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiReadOnlyVariable2PpiGuid,
    SioWdt
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
  IoWrite8 (ConfigPort, EnterConfig);
  IoWrite8 (ConfigPort, EnterConfig);
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
SioInitPs2KBMS (
  IN UINT16 IndexPort,
  IN UINT8  SioInstance
  )
{

  return EFI_UNSUPPORTED;
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

  SioPort80ToUart (IndexPort);
  
  return EFI_SUCCESS;
}

EFI_STATUS
SioSpecificInit (
  IN UINT16 IndexPort
  )
{

  return EFI_UNSUPPORTED;
}
