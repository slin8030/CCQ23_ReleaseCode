/** @file
  This file defines Bbs type table.

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "BbsType.h"

//
// the sequence of items is default boot order
//
BBS_TYPE_TABLE mBbsTypeTable[BBS_TYPE_TABLE_NUM] = {
     {
      BBS_FLOPPY,
      STRING_TOKEN(STR_FLOPPY_BOOT),
      0,
      0,
      KEY_FDD_BOOT_DEVICE_BASE,
      FDD_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_FDD_FORM_ID,
      0
    },
    {
      BBS_HARDDISK,
      STRING_TOKEN(STR_HDD_BOOT),
      0,
      0,
      KEY_HDD_BOOT_DEVICE_BASE,
      HDD_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_HDD_FORM_ID,
      0
    },
    {
      BBS_CDROM,
      STRING_TOKEN(STR_CD_BOOT),
      0,
      0,
      KEY_CD_BOOT_DEVICE_BASE,
      CD_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_CD_FORM_ID,
      0
    },
     {
      BBS_PCMCIA,
      STRING_TOKEN(STR_PCMCIA_BOOT),
      0,
      0,
      KEY_PCMCIA_BOOT_DEVICE_BASE,
      PCMCIA_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_PCMCIA_FORM_ID,
      0
    },
     {
      BBS_USB,
      STRING_TOKEN(STR_USB_BOOT),
      0,
      0,
      KEY_USB_BOOT_DEVICE_BASE,
      USB_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_USB_FORM_ID,
      0
    },
      {
      BBS_EMBED_NETWORK,
      STRING_TOKEN(STR_EMBED_NETWORK_BOOT),
      0,
      0,
      KEY_EMBED_NETWORK_BASE,
      EMBED_NETWORK_BOOT_DEVICE_LABEL,
      BOOT_EMBED_NETWORK_FORM_ID,
      0
    },
      {
      BBS_BEV_DEVICE,
      STRING_TOKEN(STR_BEV_BOOT),
      0,
      0,
      KEY_BEV_BOOT_DEVICE_BASE,
      BEV_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_BEV_FORM_ID,
      0
    },
    {
      OTHER_DRIVER,
      STRING_TOKEN(STR_OTHER_DRIVE_BOOT),
      0,
      0,
      KEY_OTHER_BOOT_DEVICE_BASE,
      OTHER_BOOT_DEVICE_LABEL,
      BOOT_DEVICE_OTHER_FORM_ID,
      0
    }
  };

UINT32 mBbsTypeTableCount = sizeof (mBbsTypeTable) / sizeof(BBS_TYPE_TABLE);