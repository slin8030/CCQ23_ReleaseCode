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

extern EFI_STATUS InstallCom1Protocol (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallCom2Protocol (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallCom3Protocol (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallCom4Protocol (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallWdtProtocol (SIO_DEVICE_LIST_TABLE* Resource);
//[-start-150414-IB12691000-add]//
//extern EFI_STATUS InstallHwmProtocol  (SIO_DEVICE_LIST_TABLE* Resource);
//[-end-150414-IB12691000-add]//
//
// Provide DeviceProtocolFunction Table
//
//[-start-150414-IB12691000-modify]//
EFI_INSTALL_DEVICE_FUNCTION mDeviceFunction[] = {
  {COM,     DEVICE_INSTANCE0, SIO_COM1,       InstallCom1Protocol},
  {COM,     DEVICE_INSTANCE1, SIO_COM2,       InstallCom2Protocol},
  {COM,     DEVICE_INSTANCE2, SIO_COM3,       InstallCom3Protocol},
  {COM,     DEVICE_INSTANCE3, SIO_COM4,       InstallCom4Protocol},
  {WDT,     DEVICE_INSTANCE0, SIO_WDT,        InstallWdtProtocol},
//  {HWM,     DEVICE_INSTANCE0, SIO_HWM,        InstallHwmProtocol},
  {NULL_ID, 0,                0,              NULL},
};
//[-end-150414-IB12691000-modify]//

EFI_SIO_TABLE mSioTable[] = {
  //======Default Start======//
  {0x1C    ,0x1C    },
  {0x26    ,0x50    },  //Enable access permitionn 
  {0x07    ,0x02    },  //SELECT COM A 
  {0xF8    ,0x01    },  //Enable 128 BYTE FFIO 
  {0x07    ,0x03    },  //SELECT COM B
  {0xF8    ,0x01    },
  {0x07    ,0x10    },  //SELECT COM C 
  {0xF8    ,0x01    },
  {0x07    ,0x11    },  //SELECT COM D
  {0xF8    ,0x01    },
  {0x14    ,0x00    },  //set COM A, B to Edge trigger
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

