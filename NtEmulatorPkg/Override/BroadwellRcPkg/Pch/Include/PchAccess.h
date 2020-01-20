/** @file
  Macros that simplify accessing PCH devices's PCI registers.

@copyright
  Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
#ifndef _PCH_ACCESS_H_
#define _PCH_ACCESS_H_

#include "PchLimits.h"

#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND 1
#endif
#ifndef STALL_ONE_SECOND
#define STALL_ONE_SECOND 1000000
#endif

//
// Bit Difinitions.
//
#ifndef BIT0
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000
#define BIT32 0x100000000
#define BIT33 0x200000000
#define BIT34 0x400000000
#define BIT35 0x800000000
#define BIT36 0x1000000000
#define BIT37 0x2000000000
#define BIT38 0x4000000000
#define BIT39 0x8000000000
#define BIT40 0x10000000000
#define BIT41 0x20000000000
#define BIT42 0x40000000000
#define BIT43 0x80000000000
#define BIT44 0x100000000000
#define BIT45 0x200000000000
#define BIT46 0x400000000000
#define BIT47 0x800000000000
#define BIT48 0x1000000000000
#define BIT49 0x2000000000000
#define BIT50 0x4000000000000
#define BIT51 0x8000000000000
#define BIT52 0x10000000000000
#define BIT53 0x20000000000000
#define BIT54 0x40000000000000
#define BIT55 0x80000000000000
#define BIT56 0x100000000000000
#define BIT57 0x200000000000000
#define BIT58 0x400000000000000
#define BIT59 0x800000000000000
#define BIT60 0x1000000000000000
#define BIT61 0x2000000000000000
#define BIT62 0x4000000000000000
#define BIT63 0x8000000000000000
#endif
///
/// The default PCH PCI bus number
///
#define DEFAULT_PCI_BUS_NUMBER_PCH  0

//
// Default Vendor ID and Subsystem ID
//
#define V_PCH_INTEL_VENDOR_ID   0x8086      ///< Default Intel PCH Vendor ID
#define V_PCH_DEFAULT_SID       0x7270      ///< Default Intel PCH Subsystem ID
#define V_PCH_DEFAULT_SVID_SID  (V_INTEL_VENDOR_ID + (V_PCH_DEFAULT_SID << 16))   ///< Default INTEL PCH Vendor ID and Subsystem ID

//
// Include device register definitions
//
#include "PchRegs/PchRegsHda.h"
#include "PchRegs/PchRegsLan.h"
#include "PchRegs/PchRegsLpc.h"
#include "PchRegs/PchRegsPcie.h"
#include "PchRegs/PchRegsRcrb.h"
#include "PchRegs/PchRegsSata.h"
#include "PchRegs/PchRegsSmbus.h"
#include "PchRegs/PchRegsSpi.h"
#include "PchRegs/PchRegsThermal.h"
#include "PchRegs/PchRegsUsb.h"
#include "PchRegs/PchRegsSerialIo.h"
#include "PchRegs/PchRegsAdsp.h"

//
//  LPC Device ID macros
//
//
// Device IDs that are PCH LPT Desktop specific
//
#define IS_PCH_9S_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    (  \
      (DeviceId == V_PCH_9S_LPC_DEVICE_ID_DT_1) || \
      (DeviceId == V_PCH_9S_LPC_DEVICE_ID_DT_2) || \
      (DeviceId == V_PCH_9S_LPC_DEVICE_ID_DT_SUPER_SKU) \
    )

#define IS_PCH_LPTH_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    (  \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_0) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_1) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_2) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_3) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_4) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_5) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_6) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_DT_SUPER_SKU) || \
      (DeviceId == 0x0) \
    )

#define IS_PCH_LPT_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    ( \
      IS_PCH_LPTH_LPC_DEVICE_ID_DESKTOP(DeviceId) || \
      IS_PCH_9S_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    )
#define IS_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    ( \
      IS_PCH_LPT_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    )
//
// Device IDs that are PCH LPT Mobile specific
//
#define IS_PCH_9S_LPC_DEVICE_ID_MOBILE(DeviceId) \
    (  \
      (DeviceId == V_PCH_9S_LPC_DEVICE_ID_MB_1) || \
      (DeviceId == V_PCH_9S_LPC_DEVICE_ID_MB_2) || \
      (DeviceId == V_PCH_9S_LPC_DEVICE_ID_MB_SUPER_SKU) \
    )

#define IS_PCH_LPTH_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_MB_0) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_MB_2) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_MB_1) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_MB_SUPER_SKU) \
    )

#define IS_PCH_LPTLP_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_SUPER_SKU) || \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_0) || \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_1) || \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_2) || \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_3) || \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_4) || \
      (DeviceId == V_PCH_LPTLP_LPC_DEVICE_ID_MB_5) \
    )

#define IS_PCH_WPTLP_LPC_DEVICE_ID_ULX(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULX_SSKU) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULX_0) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULX_1) \
    )

#define IS_PCH_WPTLP_LPC_DEVICE_ID_ULT(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULT_SSKU_HSW) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULT_SSKU) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULT_0) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_ULT_1) \
    )

#define IS_PCH_WPTLP_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      IS_PCH_WPTLP_LPC_DEVICE_ID_ULT(DeviceId) || \
      IS_PCH_WPTLP_LPC_DEVICE_ID_ULX(DeviceId) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_MB_0) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_MB_1) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_MB_2) || \
      (DeviceId == V_PCH_WPTLP_LPC_DEVICE_ID_MB_3) \
    )
    
#define IS_PCH_LPT_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      IS_PCH_LPTH_LPC_DEVICE_ID_MOBILE(DeviceId)  || \
      IS_PCH_LPTLP_LPC_DEVICE_ID_MOBILE(DeviceId) || \
      IS_PCH_9S_LPC_DEVICE_ID_MOBILE(DeviceId) \
    )
#define IS_PCH_WPT_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      IS_PCH_WPTLP_LPC_DEVICE_ID_MOBILE(DeviceId) \
    )
#define IS_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      IS_PCH_LPT_LPC_DEVICE_ID_MOBILE(DeviceId) || \
      IS_PCH_WPT_LPC_DEVICE_ID_MOBILE(DeviceId) \
    )
//
// Device IDS that are PCH LPT WorkStation specific
//
#define IS_PCH_LPTH_LPC_DEVICE_ID_WS(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_SVR_3) || \
      FALSE \
    )

#define IS_PCH_LPTLP_LPC_DEVICE_ID_WS(DeviceId) (FALSE)
#define IS_PCH_WPTLP_LPC_DEVICE_ID_WS(DeviceId) (FALSE)

#define IS_PCH_LPT_LPC_DEVICE_ID_WS(DeviceId) \
    ( \
      IS_PCH_LPTH_LPC_DEVICE_ID_WS(DeviceId) || \
      IS_PCH_LPTLP_LPC_DEVICE_ID_WS(DeviceId) \
    )

//
// Device IDS that are PCH LPT Server specific
//
#define IS_PCH_LPTH_LPC_DEVICE_ID_SERVER(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_SVR_0) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_SVR_1) || \
      (DeviceId == V_PCH_LPTH_LPC_DEVICE_ID_SVR_2) || \
      FALSE \
    )

#define IS_PCH_LPTLP_LPC_DEVICE_ID_SERVER(DeviceId) (FALSE)
#define IS_PCH_WPTLP_LPC_DEVICE_ID_SERVER(DeviceId) (FALSE)

#define IS_PCH_LPT_LPC_DEVICE_ID_SERVER(DeviceId) \
    ( \
      IS_PCH_LPTH_LPC_DEVICE_ID_SERVER(DeviceId) || \
      IS_PCH_LPTLP_LPC_DEVICE_ID_SERVER(DeviceId) \
    )

#define IS_PCH_9S_LPC_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_9S_LPC_DEVICE_ID_DESKTOP (DeviceId) || \
      IS_PCH_9S_LPC_DEVICE_ID_MOBILE (DeviceId) \
    )

#define IS_PCH_LPTH_LPC_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_LPC_DEVICE_ID_DESKTOP (DeviceId) || \
      IS_PCH_LPTH_LPC_DEVICE_ID_MOBILE (DeviceId) || \
      IS_PCH_LPTH_LPC_DEVICE_ID_WS (DeviceId) || \
      IS_PCH_LPTH_LPC_DEVICE_ID_SERVER (DeviceId) \
    )

#define IS_PCH_LPTLP_LPC_DEVICE_ID(DeviceId) \
    ( \
     IS_PCH_LPTLP_LPC_DEVICE_ID_MOBILE (DeviceId) || \
     IS_PCH_LPTLP_LPC_DEVICE_ID_WS (DeviceId) || \
     IS_PCH_LPTLP_LPC_DEVICE_ID_SERVER (DeviceId) \
    )

#define IS_PCH_WPTLP_LPC_DEVICE_ID(DeviceId) \
    ( \
     IS_PCH_WPTLP_LPC_DEVICE_ID_MOBILE (DeviceId) || \
     IS_PCH_WPTLP_LPC_DEVICE_ID_WS (DeviceId) || \
     IS_PCH_WPTLP_LPC_DEVICE_ID_SERVER (DeviceId) \
    )
#define IS_PCH_LPT_LPC_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_LPC_DEVICE_ID(DeviceId)  || \
      IS_PCH_LPTLP_LPC_DEVICE_ID(DeviceId) || \
      IS_PCH_9S_LPC_DEVICE_ID (DeviceId) \
    )
#define IS_PCH_WPT_LPC_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_LPC_DEVICE_ID(DeviceId) \
    )
//
//  SATA AHCI Device ID macros
//
#define IS_PCH_LPTH_SATA_AHCI_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_D_AHCI) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_D_AHCI) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_M_AHCI) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_M_AHCI) \
    )

#define IS_PCH_LPTLP_SATA_AHCI_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_AHCI0) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_AHCI1) \
    )
#define IS_PCH_WPTLP_SATA_AHCI_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_AHCI0) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_AHCI1) \
    )

#define IS_PCH_LPT_SATA_AHCI_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_SATA_AHCI_DEVICE_ID (DeviceId) || \
      IS_PCH_LPTLP_SATA_AHCI_DEVICE_ID (DeviceId) \
    )

#define IS_PCH_WPT_SATA_AHCI_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_SATA_AHCI_DEVICE_ID(DeviceId) \
    )

#define IS_SATA_AHCI_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPT_SATA_AHCI_DEVICE_ID (DeviceId) || \
      IS_PCH_WPT_SATA_AHCI_DEVICE_ID (DeviceId) \
    )
//
//  SATA IDE Device ID macros
//
#define IS_PCH_LPTH_SATA_IDE_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_D_IDE) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_D_IDE) || \
      (DeviceId == V_PCH_LPTH_SATA2_DEVICE_ID_D_IDE) || \
      (DeviceId == V_PCH_9S_SATA2_DEVICE_ID_D_IDE) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_M_IDE) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_M_IDE) || \
      (DeviceId == V_PCH_LPTH_SATA2_DEVICE_ID_M_IDE) || \
      (DeviceId == V_PCH_9S_SATA2_DEVICE_ID_M_IDE) \
    )

#define IS_PCH_LPT_SATA_IDE_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_SATA_IDE_DEVICE_ID(DeviceId) \
    )

#define IS_SATA_IDE_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPT_SATA_IDE_DEVICE_ID(DeviceId) \
    )

//
//  SATA RAID Device ID macros
//
#define IS_PCH_LPTH_SATA_RAID_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_D_RAID) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_D_RAID) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_D_RAID_PREM) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_D_RAID_PREM) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_D_RAID_ALTDIS) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_M_RAID) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_M_RAID) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_M_RAID_PREM) || \
      (DeviceId == V_PCH_9S_SATA_DEVICE_ID_M_RAID_PREM) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_D_RAID_SERVER) || \
      (DeviceId == V_PCH_LPTH_SATA_DEVICE_ID_M_RAID_ALTDIS) \
    )

#define IS_PCH_LPTLP_SATA_RAID_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID0) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID1) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID2) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID3) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID_ALTDIS0) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID_ALTDIS1) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID_PREM0) || \
      (DeviceId == V_PCH_LPTLP_SATA_DEVICE_ID_M_RAID_PREM1) \
    )

#define IS_PCH_WPTLP_SATA_RAID_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID0) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID1) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID2) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID3) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID_ALTDIS0) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID_ALTDIS1) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID_PREM0) || \
      (DeviceId == V_PCH_WPTLP_SATA_DEVICE_ID_M_RAID_PREM1) \
    )
#define IS_PCH_LPT_SATA_RAID_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_SATA_RAID_DEVICE_ID(DeviceId) || \
      IS_PCH_LPTLP_SATA_RAID_DEVICE_ID(DeviceId) \
    )
#define IS_PCH_WPT_SATA_RAID_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_SATA_RAID_DEVICE_ID(DeviceId) \
    )
#define IS_SATA_RAID_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPT_SATA_RAID_DEVICE_ID(DeviceId) || \
      IS_PCH_WPT_SATA_RAID_DEVICE_ID(DeviceId) \
    )

//
//  Combined SATA IDE/AHCI/RAID Device ID macros
//
#define IS_PCH_LPTH_SATA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_SATA_IDE_DEVICE_ID (DeviceId) || \
      IS_PCH_LPTH_SATA_AHCI_DEVICE_ID (DeviceId) || \
      IS_PCH_LPTH_SATA_RAID_DEVICE_ID (DeviceId) \
    )

#define IS_PCH_LPTLP_SATA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTLP_SATA_AHCI_DEVICE_ID (DeviceId) || \
      IS_PCH_LPTLP_SATA_RAID_DEVICE_ID (DeviceId) \
    )

#define IS_PCH_WPTLP_SATA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_SATA_AHCI_DEVICE_ID (DeviceId) || \
      IS_PCH_WPTLP_SATA_RAID_DEVICE_ID (DeviceId) \
    )
#define IS_PCH_LPT_SATA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_SATA_DEVICE_ID (DeviceId) || \
      IS_PCH_LPTLP_SATA_DEVICE_ID (DeviceId) \
    )
#define IS_PCH_WPT_SATA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_SATA_DEVICE_ID (DeviceId) \
    )
#define IS_SATA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPT_SATA_DEVICE_ID (DeviceId) || \
      IS_PCH_WPT_SATA_DEVICE_ID (DeviceId) \
    )

#define IS_PCH_LPTH_RAID_AVAILABLE(DeviceId) (TRUE)
#define IS_PCH_LPTLP_RAID_AVAILABLE(DeviceId) (TRUE)
#define IS_PCH_WPTLP_RAID_AVAILABLE(DeviceId) (TRUE)
#define IS_PCH_LPT_RAID_AVAILABLE(DeviceId) \
    ( \
      IS_PCH_LPTH_RAID_AVAILABLE(DeviceId) || \
      IS_PCH_LPTLP_RAID_AVAILABLE(DeviceId) \
    )

//
//  USB Device ID macros
//
#define IS_PCH_LPTH_USB_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_USB_DEVICE_ID_EHCI_1) || \
      (DeviceId == V_PCH_9S_USB_DEVICE_ID_EHCI_1) || \
      (DeviceId == V_PCH_LPTH_USB_DEVICE_ID_EHCI_2) || \
      (DeviceId == V_PCH_9S_USB_DEVICE_ID_EHCI_2) || \
      (DeviceId == V_PCH_LPTH_USB_DEVICE_ID_XHCI_1) || \
      (DeviceId == V_PCH_9S_USB_DEVICE_ID_XHCI_1) \
    )

#define IS_PCH_LPTLP_USB_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_USB_DEVICE_ID_EHCI_1) || \
      (DeviceId == V_PCH_LPTLP_USB_DEVICE_ID_XHCI_1) \
    )
#define IS_PCH_WPTLP_USB_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_USB_DEVICE_ID_EHCI_1) || \
      (DeviceId == V_PCH_WPTLP_USB_DEVICE_ID_XHCI_1) \
    )

#define IS_PCH_LPT_USB_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_USB_DEVICE_ID(DeviceId) || \
      IS_PCH_LPTLP_USB_DEVICE_ID(DeviceId) \
    )
#define IS_PCH_WPT_USB_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_USB_DEVICE_ID(DeviceId) \
    )
#define IS_PCH_USB_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPT_USB_DEVICE_ID(DeviceId) || \
      IS_PCH_WPT_USB_DEVICE_ID(DeviceId) \
    )
//
//  PCIE Device ID macros
//
#define IS_PCH_LPTH_PCIE_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT1) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT2) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT3) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT4) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT5) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT6) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT7) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_PORT8) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_MB_SUBD) || \
      (DeviceId == V_PCH_LPTH_PCIE_DEVICE_ID_DT_SUBD) ||\
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT1) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT2) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT3) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT4) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT5) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT6) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT7) || \
      (DeviceId == V_PCH_9S_PCIE_DEVICE_ID_PORT8) \
    )

#define IS_PCH_LPTLP_PCIE_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT1) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT1_ALT) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT2) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT2_ALT) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT3) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT3_ALT) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT4) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT4_ALT) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT5) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT5_ALT) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT6) || \
      (DeviceId == V_PCH_LPTLP_PCIE_DEVICE_ID_PORT6_ALT) \
    )

#define IS_PCH_WPTLP_PCIE_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT1) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT1_ALT) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT2) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT2_ALT) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT3) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT3_ALT) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT4) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT4_ALT) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT5) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT5_ALT) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT6) || \
      (DeviceId == V_PCH_WPTLP_PCIE_DEVICE_ID_PORT6_ALT) \
    )
#define IS_PCH_LPT_PCIE_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTH_PCIE_DEVICE_ID(DeviceId) || \
      IS_PCH_LPTLP_PCIE_DEVICE_ID(DeviceId) \
    )
#define IS_PCH_WPT_PCIE_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_WPTLP_PCIE_DEVICE_ID(DeviceId) \
    )
#define IS_PCH_PCIE_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPT_PCIE_DEVICE_ID(DeviceId) || \
      IS_PCH_WPT_PCIE_DEVICE_ID(DeviceId) \
    )

//
//  HD Azalia Device ID macros
//
#define IS_PCH_LPTLP_HDA_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_HDA_DEVICE_ID) || \
      (DeviceId == V_PCH_LPTLP_HDA_DEVICE_ID_ALT) \
    )
#define IS_PCH_WPTLP_HDA_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_HDA_DEVICE_ID) || \
      (DeviceId == V_PCH_WPTLP_HDA_DEVICE_ID_ALT) \
    )
#define IS_PCH_HDA_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_LPTLP_HDA_DEVICE_ID(DeviceId) || \
      IS_PCH_WPTLP_HDA_DEVICE_ID(DeviceId) \
    )

//
//  ADSP Device ID macros
//
#define IS_PCH_LPTLP_ADSP_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_LPTLP_ADSP_DEVICE_ID) \
    )
#define IS_PCH_WPTLP_ADSP_DEVICE_ID(DeviceId) \
    ( \
      (DeviceId == V_PCH_WPTLP_ADSP_DEVICE_ID) \
    )

#endif

