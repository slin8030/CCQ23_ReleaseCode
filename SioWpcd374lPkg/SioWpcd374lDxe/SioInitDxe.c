/** @file

Lock unlock sio and structure of install device protocol

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SioInitDxe.h"

//
// this SIO support Cfg Port List
//
UINT16  mSioCfgPortList[]    = {0x2E, 0x4E, 0x162E, 0x164E};

//[-start-140116-IB12970054-modify]//
UINT16  mSioCfgPortListSize  = sizeof (mSioCfgPortList) / sizeof (UINT16);
//[-end-140116-IB12970054-modify]//

UINT8  EnterConfig = ENTER_CONFIG;
UINT8  ExitConfig  = EXIT_CONFIG;

//
// Provide mSioResourceFunction Protocol Interface 
//
EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[] = {
  NULL_ID,
  SIO_ID1,
  SIO_ID2,
  NULL_ID,
  FALSE,
};

extern EFI_STATUS InstallCom1Protocol   (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallCom2Protocol   (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallFloppyProtocol (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallLptProtocol    (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallPs2KProtocol   (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallPs2MProtocol   (SIO_DEVICE_LIST_TABLE* Resource);

//
// Provide DeviceProtocolFunction Table
//
EFI_INSTALL_DEVICE_FUNCTION mDeviceFunction[] = {
  {COM,     DEVICE_INSTANCE0, SIO_COM1,   InstallCom1Protocol},
  {COM,     DEVICE_INSTANCE1, SIO_COM2,   InstallCom2Protocol},
  {FLOPPY,  DEVICE_INSTANCE0, SIO_FLOPPY, InstallFloppyProtocol},
  {LPT,     DEVICE_INSTANCE0, SIO_LPT1,   InstallLptProtocol},
  {KYBD,    DEVICE_INSTANCE0, SIO_KYBD,   InstallPs2KProtocol},
  {MOUSE,   DEVICE_INSTANCE1, SIO_MOUSE,  InstallPs2MProtocol},
  {NULL_ID, 0,                0,          NULL},
};

EFI_SIO_TABLE mSioTable[] = {
  //======Default Start======//
  //====== Default End ======//
  {0    ,0    }
};

/**

 Enter SIO mode

**/
VOID
EnterConfigMode (
  )
{
  SioWrite8 (
    mSioResourceFunction->SioCfgPort, 
    ENTER_CONFIG,
    mSioResourceFunction->ScriptFlag
    );

  SioWrite8 (
    mSioResourceFunction->SioCfgPort, 
    ENTER_CONFIG,
    mSioResourceFunction->ScriptFlag
    );  
}

/**

 Exit SIO mode

**/
VOID
ExitConfigMode (
  )
{
  SioWrite8 (
    mSioResourceFunction->SioCfgPort, 
    EXIT_CONFIG,
    mSioResourceFunction->ScriptFlag
    );
}

