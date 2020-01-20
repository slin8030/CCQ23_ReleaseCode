/** @file
  XhcDebug.c

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/UsbIo.h>
#include <XhcStatusCode.h>
#include "Pci.h"


STATIC DEBUG_IO_PRIVATE_DATA    *mPrivate = NULL;
STATIC EFI_GUID                 mXhcDebugHobGuid    = { 0x850fae6a, 0x16da, 0x4b42, { 0x8d, 0x84, 0x3c, 0x36, 0x56, 0xb1, 0xe0, 0x1f } };
STATIC UINT8                    mStr0[0x10]         = { 0x04, 0x03, 0x09, 0x04 };
STATIC UINT8                    mManufacturer[0x10] = { 0x0e, 0x03, 'I', 0x00, 'n', 0x00, 's', 0x00, 'y', 0x00, 'd', 0x00, 'e', 0x00 };
STATIC UINT8                    mProduct[0x30]      = { 0x2a, 0x03, 'I', 0x00, 'n', 0x00, 's', 0x00, 'y', 0x00, 'd', 0x00, 'e', 0x00, ' ', 0x00,
                                                        'U',  0x00, 'S', 0x00, 'B', 0x00, '3', 0x00, ' ', 0x00,
                                                        'D',  0x00, 'e', 0x00, 'b', 0x00, 'u', 0x00, 'g', 0x00, 'g', 0x00, 'e', 0x00, 'r', 0x00 };
STATIC UINT8                    mSerialNum[0x10]    = { 0x08, 0x03, '1', 0x00, '.', 0x00, '0', 0x00 };

struct
{
  CHAR8             Signature[4];
  DEBUGIO_CONFIG    Config;
  UINT32            XhciBar;
} XhcDebugIoConfig = {
  { 'D', 'B', 'G', 'C' },
  { 0,        // 0:LPC, 1:PCI
    0,        // 0:Bypass connection off, 1:Bypass connection on
    0,        // 0:USB, 1:LPT, 2:COM
    FixedPcdGet8 (PcdUsbDebugXhciControl), // USB HC port
    0,        // USB debug port
    0x0080, // Debug port
    10       // Bypass timeout
  },
  FixedPcdGet32 (PcdUsbDebugXhciBaseAddress)
};

UINT8
EFIAPI
ByteRotate (
  IN  UINT8                     Data
  )
{
  return (Data << 4) | (Data >> 4);
}

//
// Internal Functions
//
STATIC
EFI_STATUS
InitXhciDebugPort (
  IN  UINT32                    HcToken,
  OUT UINT32                    *DbgBar,
  IN OUT UINT32                 *DebugContextAddr,
  IN  BOOLEAN                   RuntimeState
  );


/**
 Is it USB Device

 @param [in]   PciAddress

 @return 0x00 : No
 @return 0x01 : XHCI
 @return 0x0f : Pcie bridge

**/
STATIC
UINTN
IsUsbDevice (
  IN UINT32                     PciAddress
  )
{
  UINT32 ClassCode;

  PciRead (EfiWidthUint32, PciAddress + 0x08, &ClassCode);
  ClassCode &= 0xFFFFFF00;
  switch (ClassCode) {
  case 0x0C033000: return XHCI;
  case 0x06040000: return BRIDGE;
  }
  return 0;
}

/**
 Memory copy common routine

 @param [in]   DestAddr         Destination Address
 @param [in]   SourAddr         Source Address
 @param [in]   Width            Data width

 @retval EFI_SUCCESS            Success
 @retval EFI_UNSUPPORTED        Upsupported

**/
STATIC
EFI_STATUS
MemoryCopy (
  IN UINT8                      *DestAddr,
  IN UINT8                      *SourAddr,
  IN EFI_WIDTH                  Width
  )
{
  switch (Width) {
  case EfiWidthUint32:
    *(UINT32*)DestAddr = *(UINT32*)SourAddr;
    break;
  case EfiWidthUint64:
    //
    // break down into two 32 bits operation
    //
    *(UINT32*)(DestAddr + 0) = *(UINT32*)(SourAddr + 0);
    *(UINT32*)(DestAddr + 4) = *(UINT32*)(SourAddr + 4);
    break;
  default:
    return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;
}

/**
 Read Xhc debug register

 @param [in]   Bar              Debug capability base address
 @param [in]   Width            Data width
 @param [in]   RegAddr
 @param [in, out] Data          A pointer to data read from register

 @retval EFI_SUCCESS            Success
 @retval EFI_UNSUPPORTED        Upsupported

**/
EFI_STATUS
ReadXhcDebugReg (
  IN UINT32                     Bar,
  IN EFI_WIDTH                  Width,
  IN UINT32                     RegAddr,
  IN OUT VOID                   *Data
  )
{
  return MemoryCopy(Data, (UINT8*)(UINTN)(Bar + RegAddr), Width);
}

/**
 Write Xhc debug register

 @param [in]   Bar              Debug capability base address
 @param [in]   Width            Data width
 @param [in]   RegAddr
 @param [in]   Data             A pointer to data write to register

 @retval EFI_SUCCESS            Success
 @retval EFI_UNSUPPORTED        Upsupported

**/
EFI_STATUS
WriteXhcDebugReg (
  IN UINT32                     Bar,
  IN EFI_WIDTH                  Width,
  IN UINT32                     RegAddr,
  IN VOID                       *Data
  )
{
  return MemoryCopy((UINT8*)(UINTN)(Bar + RegAddr), Data, Width);
}

/**
 Update the event dequeue pointer

 @param [in]   DbgBar
 @param [in]   DebugContext     A pointer to debug context
 @param [in]   TargetAddr


**/
STATIC
BOOLEAN
UpdateEventDequeuePointer (
  IN UINT32                     DbgBar,
  IN DBG_CONTEXT                *DebugContext,
  IN UINT64                     TargetAddr
  )
{
  UINT64              Data;
  UINT32              CycleBit;
  XHCI_TRANSFER_EVENT *Event;
  BOOLEAN             Result = FALSE;

  ReadXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCERDP, &Data);
  Event = (XHCI_TRANSFER_EVENT*)(UINTN)Data;
  CycleBit = Event->CycleBit;
  while (CycleBit == Event->CycleBit) {
    if (TargetAddr && TargetAddr == Event->TRBPointer) Result = TRUE;
    Event ++;
    if ((UINT64)(UINTN)Event >= DebugContext->EventRingSegmentTable.RingSegmentBaseAddress + (MultU64x32 (DebugContext->EventRingSegmentTable.RingSegmentSize, sizeof (XHCI_TRANSFER_EVENT)))) {
      Event = (XHCI_TRANSFER_EVENT*)(UINTN)DebugContext->EventRingSegmentTable.RingSegmentBaseAddress;
      CycleBit ^= 1;
    }
  }
  Data = (UINT64)(UINTN)Event;
  WriteXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCERDP, &Data);
  do {
    ReadXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCST, &Data);
  } while (Data & B_DCST_ER);
  return Result;
}


/**
 Get USB Bar

 @param [in]   HcToken

 @return USB BAR

**/
STATIC
UINT32
GetUsbBar (
  IN UINT32                     HcToken
  )
{
  UINT32  PciAddress;
  UINT32  BarAddress;

  PciAddress = EFI_PCI_ADDRESS(0, ((HcToken & B_TOKEN_XHCI_PCI_DEV) >> N_TOKEN_XHCI_PCI_DEV), ((HcToken & B_TOKEN_XHCI_PCI_FUN) >> N_TOKEN_XHCI_PCI_FUN), 0);
  if (IsUsbDevice (PciAddress) == BRIDGE) {
    PciRead (EfiWidthUint32, PciAddress + 0x18, &BarAddress);
    PciAddress = EFI_PCI_ADDRESS(((BarAddress >> 8) & 0xff), 0, 0, 0);
  }
  PciRead (EfiWidthUint32, PciAddress + 0x10, &BarAddress);
  BarAddress &= ~0x0f;
  if (BarAddress != 0) {
    if (BarAddress != XhcDebugIoConfig.XhciBar) {
      if ((*(UINT32*)(UINTN)BarAddress & 0xff000000) == 0x01000000 && (*(UINT32*)(UINTN)BarAddress & 0x000000ff) > 0) return BarAddress;
    } else return BarAddress;
  }
  PciWrite (EfiWidthUint32, PciAddress + 0x10, &XhcDebugIoConfig.XhciBar);
  return XhcDebugIoConfig.XhciBar;
}


/**
 Turn power state on

 @param [in]   PciAddress


**/
STATIC
VOID
PowerStateOn (
  IN UINT32                     PciAddress
  )
{
  UINT32 Data;
  UINT8  CapPtr;
  //
  // Searching for Power Capabilities
  //
  PciRead (EfiWidthUint8, PciAddress + 0x34, &CapPtr);
  while (CapPtr != 0) {
    PciRead (EfiWidthUint32, PciAddress + CapPtr, &Data);
    if ((Data & 0xff) == 0x01) {
      PciRead (EfiWidthUint8, PciAddress + CapPtr + 4, &Data);
      if ((Data & 0x03) != 0) {
        Data = 0;
        PciWrite (EfiWidthUint8, PciAddress + CapPtr + 4, &Data);
        MicroSecondDelay (100000);
      }
      break;
    }
    CapPtr = (UINT8)((Data >> 8) & 0xff);
  }
  if (IsUsbDevice (PciAddress) == BRIDGE) {
    //
    // Set the power state for XHC device after bridge
    //
    PciRead (EfiWidthUint32, PciAddress + 0x18, &Data);
    PowerStateOn ((UINT32)EFI_PCI_ADDRESS(((Data >> 8) & 0xff), 0, 0, 0));
  }
}

/**
 Get Xhci PCI location

 @param [in]   HcToken

 @return PCI address

**/
STATIC
UINT32
GetXhciLocation (
  IN UINT32                     HcToken
  )
{
  UINT32  PciAddress;
  UINT32  Bus;
  UINT32  BusEnd;
  UINT32  Data;
  UINT32  XhciBridge;
  BOOLEAN BusMasterResetNeeded;

  PciAddress = EFI_PCI_ADDRESS(0, ((HcToken & B_TOKEN_XHCI_PCI_DEV) >> N_TOKEN_XHCI_PCI_DEV), ((HcToken & B_TOKEN_XHCI_PCI_FUN) >> N_TOKEN_XHCI_PCI_FUN), 0);
  switch (IsUsbDevice (PciAddress)) {
  case XHCI:
    Data = 0;
    PciRead (EfiWidthUint16, PciAddress + 4, &Data);
    if ((Data & 0x06) != 0x06) {
      Data |= 0x06;
      PciWrite (EfiWidthUint16, PciAddress + 4, &Data);
      MicroSecondDelay (100000);
    }
    PowerStateOn (PciAddress);
    return PciAddress;
  case BRIDGE:
    BusMasterResetNeeded = FALSE;
    PciRead (EfiWidthUint16, PciAddress + 4, &Data);
    if ((Data & 0x06) != 0x06) {
      Data |= 0x06;
      PciWrite (EfiWidthUint16, PciAddress + 4, &Data);
      MicroSecondDelay (100000);
      BusMasterResetNeeded = TRUE;
    }
    PciRead (EfiWidthUint32, PciAddress + 0x18, &Data);
    if (Data == 0) {
      XhciBridge = (HcToken & B_TOKEN_XHCI_PCI_BRIDGE) >> N_TOKEN_XHCI_PCI_BRIDGE;
      Data = (XhciBridge << 16) + (XhciBridge << 8);
      PciWrite (EfiWidthUint32, PciAddress + 0x18, &Data);
      MicroSecondDelay (100000);
      BusMasterResetNeeded = TRUE;
    }
    PowerStateOn (PciAddress);
    Bus    = (Data >> 8) & 0xff;
    BusEnd = (Data >> 16) & 0xff;
    for (; Bus <= BusEnd; Bus ++) {
      PciAddress = EFI_PCI_ADDRESS(Bus, 0, 0, 0);
      if ((IsUsbDevice (PciAddress)) == XHCI) {
        PciRead (EfiWidthUint16, PciAddress + 4, &Data);
        if (BusMasterResetNeeded || ((Data & 0x06) != 0x06)) {
          Data |= 0x06;
          PciWrite (EfiWidthUint16, PciAddress + 4, &Data);
          MicroSecondDelay (100000);
        }
        return PciAddress;
      }
    }
  }
  return 0;
}

/**
 Route the debug port from ordinary USB3 port

 @param [in]   Bar


**/
STATIC
VOID
RouteDebugPort (
  IN  UINT32                    Bar
  )
{
  UINTN  Index;
  UINT32 Data;
  UINT32 *Port;
  UINTN  Count;

  Index = ((*(UINT32*)(UINTN)(Bar + 0x10) & 0xffff0000) >> 14);
  do {
    Data = *(UINT32*)(UINTN)(Bar + Index);
    if ((Data & 0xff0000ff) == 0x3000002) {
      //
      // USB3 supported protocol
      //
      Data = *(UINT32*)(UINTN)(Bar + Index + 8);
      Port = (UINT32*)(UINTN)(Bar + 0x400 + (*(UINT32*)(UINTN)Bar & 0xff) + (((Data & 0xff) - 1) * 0x10));
      Count = (UINTN)((Data >> 8) & 0xff);
      for (Index = 0; Index < Count; Index ++, Port += 4) {
        Data = *Port;
        if ((Data & 0x1e0) == 0xc0) {
          //
          // The debug port found, toggle the port power to make the signal route to debug port
          //
          Data &= ~0x200;
          *Port = Data;
          MicroSecondDelay (500000);
          Data = 0x200;
          *Port = Data;
          MicroSecondDelay (500000);
        }
      }
      break;
    }
    Index += (UINTN)((Data & 0xff00) >> (8 - 2));
  } while ((Data & 0xff00) != 0);
}

/**
 Detect Xhci

 @param [in]   Token
 @param [out]  DbgBar
 @param [in, out] DebugContextAddr

 @retval EFI_SUCCESS
 @retval EFI_NOT_FOUND

**/
STATIC
EFI_STATUS
DetectXhci (
  IN  UINT32                    *Token,
  OUT UINT32                    *DbgBar,
  IN OUT UINT32                 *DebugContextAddr
  )
{
  UINT32  PciAddress;
  UINT32  Dev;
  UINT32  Fun;
  UINT32  Data;
  UINT32  HcToken;
  UINTN   HcPort;
  BOOLEAN Found;
  UINT32  Bridge;
  UINT32  Device;
  UINT32  Bar;
  UINT32  Index;
  UINT32  LmcIndex;
  UINT32  XhciBridge;
  if (*Token != 0) {
    //
    // Runtime checking
    //
    return InitXhciDebugPort(*Token, DbgBar, DebugContextAddr, TRUE);
  }
  //
  // XHCI detection
  //
  HcPort = XhcDebugIoConfig.Config.UsbHcPort;
  Bridge = 0;
  Device = 0;
  Found  = FALSE;
  for (Dev = 0; Dev < 0x20 && !Found; Dev ++) {
    for (Fun = 0; Fun < 8 && !Found; Fun ++) {
      PciAddress = EFI_PCI_ADDRESS(0, Dev, Fun, 0);
      if (IsUsbDevice (PciAddress) == XHCI && HcPort -- == 0) {
        Device = PciAddress;
        Found  = TRUE;
      }
    }
  }
  if (!Found) {
    if (!Bridge) {
      for (Dev = 0; Dev < 0x20 && !Found; Dev ++) {
        for (Fun = 0; Fun < 8 && !Found; Fun ++) {
          PciAddress = EFI_PCI_ADDRESS(0, Dev, Fun, 0);
          if ((IsUsbDevice (PciAddress)) == BRIDGE) {
            PciRead (EfiWidthUint32, PciAddress + 0x18, &Data);
            if (Data == 0) {
              XhciBridge = (XHCI_BRIDGE << 16) + (XHCI_BRIDGE << 8);
              PciWrite (EfiWidthUint32, PciAddress + 0x18, &XhciBridge);
              MicroSecondDelay (1000);
              XhciBridge = XHCI_BRIDGE;
            } else {
              XhciBridge = (Data >> 8) & 0xff;
            }
            Device = EFI_PCI_ADDRESS(XhciBridge, 0, 0, 0);
            if ((IsUsbDevice (Device)) == XHCI && HcPort -- == 0) {
              Bridge = EFI_PCI_ADDRESS(0, Dev, Fun, 0);
              Found  = TRUE;
            } else if (Data == 0) {
              PciWrite (EfiWidthUint32, PciAddress + 0x18, &Data);
              MicroSecondDelay (10000);
            }
          }
        }
      }
    }
    if (Bridge) {
      PowerStateOn (Bridge);
      if (IsUsbDevice (Device) != XHCI) return EFI_NOT_FOUND; else Found = TRUE;
      Bar = GetUsbBar ((Bridge << 16) & 0xff000000);
      Data = (Bar & 0xfff00000) + ((Bar & 0xfff00000) >> 16);
      PciWrite (EfiWidthUint32, Bridge + 0x20, &Data);
      MicroSecondDelay (1000);
    }
  }
  if (Found) {
    if (!Bridge) {
      PciAddress = Device;
      HcToken = (PciAddress << 16) & (B_TOKEN_XHCI_PCI_DEV | B_TOKEN_XHCI_PCI_FUN);
    } else {
      PciRead (EfiWidthUint32, Bridge + 0x18, &Data);
      XhciBridge = (Data >> 8) & 0xff;
      PciAddress = EFI_PCI_ADDRESS(XhciBridge, 0, 0, 0);
      HcToken = ((Bridge << 16) & (B_TOKEN_XHCI_PCI_DEV | B_TOKEN_XHCI_PCI_FUN)) + ((XhciBridge << 16) & B_TOKEN_XHCI_PCI_BRIDGE);
      PciRead (EfiWidthUint16, Bridge + 4, &Data);
      Data |= 0x06;
      PciWrite (EfiWidthUint16, Bridge + 4, &Data);
      MicroSecondDelay (100000);


    }
    Bar = GetUsbBar (HcToken);
    PowerStateOn (PciAddress);
    PciRead (EfiWidthUint16, PciAddress + 4, &Data);
    Data |= 0x06;
    PciWrite (EfiWidthUint16, PciAddress + 4, &Data);
    MicroSecondDelay (100000);
    //
    // Port routing detection for Intel XHCI
    //
    Data = 0;
    PciRead (EfiWidthUint16, PciAddress, &Data);
    if (Data == 0x8086) {
      PciRead (EfiWidthUint32, PciAddress + 0xd8, &Dev);
      Data = Dev ^ 0xffffffff;
      PciWrite (EfiWidthUint32, PciAddress + 0xd8, &Data);
      PciRead (EfiWidthUint32, PciAddress + 0xd8, &Data);
      if (Data != Dev) {
        HcToken |= B_TOKEN_PORT_ROUTING;
      }
    }
    Index = ((*(UINT32*)(UINTN)(Bar + 0x10) & 0xffff0000) >> 14);
    LmcIndex = 0;
    do {
      Data = *(UINT32*)(UINTN)(Bar + Index);
      if ((Data & 0xff) == 0x0a) {
        //
        // Debug Capability
        //
        HcToken |= (Index & B_TOKEN_DEBUG_CAP_INDEX);
      }
      Index += ((Data & 0xff00) >> (8 - 2));
    } while ((Data & 0xff00) != 0);
    if ((HcToken & B_TOKEN_DEBUG_CAP_INDEX) != 0) {
      *Token = HcToken;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

/**
 Debug Software Startup with Non-Initialized XHCI

 @param [in]   HcToken
 @param [out]  DebugBar
 @param [in, out] DebugContextAddr
 @param [in]   RuntimeState

 @retval EFI_SUCCESS

**/
STATIC
EFI_STATUS
InitXhciDebugPort(
  IN  UINT32                    HcToken,
  OUT UINT32                    *DebugBar,
  IN OUT UINT32                 *DebugContextAddr,
  IN  BOOLEAN                   RuntimeState
  )
{
  EFI_STATUS            Status;
  UINT32                PciAddress;
  UINT32                Bar;
  UINT32                DbgBar;
  UINT32                Data;
  UINT64                Pointer;
  DBG_CONTEXT           *DebugContext;
  UINT8                 PostCode;
  UINTN                 Index;
  UINTN                 Timeout;
  BOOLEAN               UpdateContext;

  UpdateContext = TRUE;
  if ((PciAddress = GetXhciLocation (HcToken)) == 0) return EFI_DEVICE_ERROR;
  Bar = GetUsbBar (HcToken);
  DbgBar = Bar + (HcToken & B_TOKEN_DEBUG_CAP_INDEX);
  if (DebugBar) *DebugBar = DbgBar;
  //
  // Set upstream non-posted request size to 1 if it is add-on PCIe XHCI card on Intel BayTrail chipset
  //
  if (((HcToken & B_TOKEN_XHCI_PCI_BRIDGE) >> N_TOKEN_XHCI_PCI_BRIDGE) != 0) {
    //
    // Set upstream non-posted request size in PCIe root bridge Channel Configuration(0xd0) bit 15 to 1
    //
    PciAddress = EFI_PCI_ADDRESS(0x00, ((HcToken & B_TOKEN_XHCI_PCI_DEV) >> N_TOKEN_XHCI_PCI_DEV), 0, 0);
    Data = 0;
    PciRead (EfiWidthUint16, PciAddress, &Data);
    if (Data == 0x8086) {
      PciRead (EfiWidthUint32, PciAddress + 0xd0, &Data);
      if ((Data & 0x00008000) == 0) {
        Data |= 0x00008000;
        PciWrite (EfiWidthUint32, PciAddress + 0xd0, &Data);
      }
    }
  }
  //
  // First pass for XHCI debug capability initial
  //
  while (RuntimeState) {
    //
    // HCRST detection during runtime for XHC debug capability recovery
    //
    Data = *(UINT32*)(UINTN)(Bar + (*(UINT32*)(UINTN)Bar & 0xff));
    if (Data & 0x02) {
      do {
        Data = *(UINT32*)(UINTN)(Bar + (*(UINT32*)(UINTN)Bar & 0xff));
        MicroSecondDelay (1000);
      } while ((Data & 0x02) != 0);
      RuntimeState = FALSE;
      break;
    }
    //
    // Debug capability changed detection
    //
    ReadXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCCTRL, &Data);
    if (!(Data & B_DCCTRL_DCE)) {
      RuntimeState = FALSE;
      break;
    }

    //
    // Intel port routing changed detection
    //
    if (HcToken & B_TOKEN_PORT_ROUTING) {
      PciRead (EfiWidthUint32, PciAddress + 0xd0, &Data);
      if (Data == 0) {
        RuntimeState = FALSE;
        break;
      }
      PciRead (EfiWidthUint32, PciAddress + 0xd8, &Data);
      if (Data == 0) {
        RuntimeState = FALSE;
      }
    }
    break;
  }
  if (!RuntimeState) {
    //
    // Set XHC/EHC routing for Intel XHCI to make sure the signal routed to XHC
    //
    if (HcToken & B_TOKEN_PORT_ROUTING) {
      Data = 0xffffffff;
      PciWrite (EfiWidthUint32, PciAddress + 0xd0, &Data);
      PciWrite (EfiWidthUint32, PciAddress + 0xd8, &Data);
      MicroSecondDelay (100000);
    }
    Data = 0;
    PciRead (EfiWidthUint16, PciAddress, &Data);
    if (Data == 0x8086) {

      //
      // Host Controller Capability - 8070 h
      // Xhci Capability Vendor Defined (ID: C0 h)
      //
      // Program xHCIBAR + 0x80B8 [5:0] - SSPE_REG: according to the number of enabled SuperSpeed port.
      // The lower four bits represent the first four USB SuperSpeed ports
      // and the upper two bits represent the two SSIC ports.
      // [3:0]:Port 0 - Port 3
      // [5:4]:SSIC Port 0 - Port 1
      //
      if ((*(UINT32*)(UINTN)(Bar + 0x8070) & 0xffff) == 0xffc0) {
        *(UINT32*)(UINTN)(Bar + 0x80b8) |= 0x0000000f;
      }

      //
      //  SSIC Policy and Implementation Specific Registers - 0x8800
      //  PORT1_SSIC_CONFIG_REG2 - 880C h
      //  PORT2_SSIC_CONFIG_REG2 - 883c h
      //  Set PROG_DONE bit,
      //  xHCIBAR + 0x880C [30] = 1b and xHCIBAR + 0x883C [30] = 1b.
      //
      //
      if ((*(UINT32*)(UINTN)(Bar + 0x8800) & 0xffff) == 0x40c4) {
        *(UINT32*)(UINTN)(Bar + 0x880c) |= 0x40000000;
        *(UINT32*)(UINTN)(Bar + 0x883c) |= 0x40000000;
      }
      //
      // Set 64 byte DMA boundary(bit 22~25) and Read Request Size control(bit 0~3) on Intel BayTrail chipset
      //
      Data = 0x83cfc68f;
      PciWrite (EfiWidthUint32, PciAddress + 0x44, &Data);
    }
    //
    // Issue a HCRESET before initialization
    //
    *(UINT32*)(UINTN)(Bar + (*(UINT32*)(UINTN)Bar & 0xff)) = 0x02;
    do {
      Data = *(UINT32*)(UINTN)(Bar + (*(UINT32*)(UINTN)Bar & 0xff));
      MicroSecondDelay (1000);
    } while ((Data & 0x02) != 0);

    //
    // Allocate memory for the debug context
    //
    if (DebugContextAddr == NULL || *DebugContextAddr == 0) {
      Pointer = 0x100000000;
      Status = gBS->AllocatePages (
                      AllocateMaxAddress,
                      EfiACPIMemoryNVS,
                      1,
                      (EFI_PHYSICAL_ADDRESS*)&Pointer
                      );
      if (Status != EFI_SUCCESS) {
        return EFI_OUT_OF_RESOURCES;
      }
      DebugContext = (DBG_CONTEXT*)(UINTN)Pointer;
      if (DebugContextAddr != NULL) *DebugContextAddr = (UINT32)(UINTN)DebugContext;
    } else {
      DebugContext = (DBG_CONTEXT*)(UINTN)*DebugContextAddr;
    }
    //
    // Set LMC enable after HCRESET
    //
    ZeroMem (DebugContext, sizeof (DBG_CONTEXT));
    DebugContext->LmcCapID |= B_LMC_LME;
  } else {
    //
    // Get the debug context back from DCERSTBA
    //
    ReadXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCERSTBA, &Pointer);
    DebugContext = (DBG_CONTEXT*)(UINTN)(Pointer - (UINTN)&((DBG_CONTEXT*)0)->EventRingSegmentTable);

    //
    // Inspect the DCPORTSC to reinitial the debug port if port status changed
    //
    Data = 0;
    ReadXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCPORTSC, &Data);
    if (Data & (B_DCPORTSC_CSC | B_DCPORTSC_PRC | B_DCPORTSC_PLC | B_DCPORTSC_CEC)) {
      //
      // Port status changed, we need to recheck the port connection status without context update
      //
      UpdateContext = FALSE;
      //
      // Enable the port if it under disable state
      //
      if ((Data & B_DCPORTSC_CCS) && ((Data & B_DCPORTSC_PLS) >> N_DCPORTSC_PLS) == 0x04) {
        Data |= B_DCPORTSC_PED;
        WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCPORTSC, &Data);
        MicroSecondDelay (100000);
      }
    } else {
      return EFI_SUCCESS;
    }
  }
  if (!RuntimeState) {
    //
    // Setup Debug Context
    //
    DebugContext->Signature                             = USB_XHCID_SIGNATURE;
    DebugContext->ContextSize                           = sizeof (DBG_CONTEXT);
    DebugContext->EventRingSegmentTable.RingSegmentSize = EVENT_RING_SIZE;
    DebugContext->DebugInfoContext.Str0Len              = mStr0[0];
    DebugContext->DebugInfoContext.ManufacturerLen      = mManufacturer[0];
    DebugContext->DebugInfoContext.ProductLen           = mProduct[0];
    DebugContext->DebugInfoContext.SerialNumLen         = mSerialNum[0];
    DebugContext->EndpointOut.MaxPacketSize             = 0x400;
    DebugContext->EndpointOut.ErrorCount                = 3;
    DebugContext->EndpointOut.EPType                    = EP_TYPE_BULK_OUT;
    DebugContext->EndpointOut.AvgTRBLength              = 0x400;
    DebugContext->EndpointOut.MaxESITPayload            = 0x400;
    DebugContext->BulkOutTRB.TRBType                    = TRB_TYPE_NORMAL;
    DebugContext->BulkOutTRB.IOC                        = 1;
    DebugContext->BulkOutLinkTRB.ToggleCycle            = 1;
    DebugContext->BulkOutLinkTRB.TRBType                = TRB_TYPE_LINK;
    DebugContext->EndpointIn.MaxPacketSize              = 0x400;
    DebugContext->EndpointIn.ErrorCount                 = 3;
    DebugContext->EndpointIn.EPType                     = EP_TYPE_BULK_IN;
    DebugContext->EndpointIn.AvgTRBLength               = 0x400;
    DebugContext->EndpointIn.MaxESITPayload             = 0x400;
    DebugContext->BulkInTRB.TRBType                     = TRB_TYPE_NORMAL;
    DebugContext->BulkInTRB.IOC                         = 1;
    DebugContext->BulkInTRB.ISP                         = 1;
    DebugContext->BulkInLinkTRB.ToggleCycle             = 1;
    DebugContext->BulkInLinkTRB.TRBType                 = TRB_TYPE_LINK;
    CopyMem (DebugContext->Str0, mStr0, 0x10);
    CopyMem (DebugContext->ManufacturerStr, mManufacturer, 0x10);
    CopyMem (DebugContext->ProductStr, mProduct, 0x30);
    CopyMem (DebugContext->SerialNum, mSerialNum, 0x10);
    //
    // Setup Debug Capability Registers
    //
    Data = 1;
    WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCERSTSZ, &Data);
    Data = INSYDE_USB3D_VID << N_DCDDI1_VID;
    WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCDDI1, &Data);
    Data = (INSYDE_USB3D_REV << N_DCDDI2_REV) + INSYDE_USB3D_PID;
    WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCDDI2, &Data);
  }
  if (UpdateContext) {
    //
    // Setup internal data pointers
    //
    DebugContext->EventRingSegmentTable.RingSegmentBaseAddress = (UINT64)(UINTN)DebugContext->Event;
    DebugContext->DebugInfoContext.Str0DescAddr                = (UINT64)(UINTN)DebugContext->Str0;
    DebugContext->DebugInfoContext.ManufacturerDescAddr        = (UINT64)(UINTN)DebugContext->ManufacturerStr;
    DebugContext->DebugInfoContext.ProductDescAddr             = (UINT64)(UINTN)DebugContext->ProductStr;
    DebugContext->DebugInfoContext.SerialNumDescAddr           = (UINT64)(UINTN)DebugContext->SerialNum;
    DebugContext->EndpointOut.TRDequeuePointer                 = (UINT64)(UINTN)&DebugContext->BulkOutTRB | 0x01;
    DebugContext->EndpointIn.TRDequeuePointer                  = (UINT64)(UINTN)&DebugContext->BulkInTRB | 0x01;
    DebugContext->BulkOutLinkTRB.RingSegmentPointer            = (UINT64)(UINTN)&DebugContext->BulkOutTRB;
    DebugContext->BulkInLinkTRB.RingSegmentPointer             = (UINT64)(UINTN)&DebugContext->BulkInTRB;
    //
    // Setup Debug Capability pointer registers
    //
    Pointer = (UINT64)(UINTN)&DebugContext->EventRingSegmentTable;
    WriteXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCERSTBA, &Pointer);
    Pointer = (UINT64)(UINTN)DebugContext->Event;
    WriteXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCERDP, &Pointer);
    Pointer = (UINT64)(UINTN)&DebugContext->DebugInfoContext;
    WriteXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCCP, &Pointer);
  }
  //
  // Debug Capability Enable
  //
  Data = B_DCCTRL_DCE | B_DCCTRL_DRC;
  WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCCTRL, &Data);
  //
  // Route the port signal from ordinary USB3 port to debug port
  //
  RouteDebugPort (Bar);
  //
  // Waiting for USB3 debug port linked
  //
  PostCode = 0xd0;
  Index = 0;
  Timeout = XhcDebugIoConfig.Config.BypassTimeout * 2;
  while (TRUE) {
    ReadXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCCTRL, &Data);
    if (Data & B_DCCTRL_DCR) break;
    MicroSecondDelay (1000);
    if (!Index) {
      IoWrite8((UINT16)XhcDebugIoConfig.Config.DebugPort, PostCode);
      PostCode = ByteRotate(PostCode);
      Index = 500;
      if (XhcDebugIoConfig.Config.Bypass) {
        if (!Timeout) return EFI_TIMEOUT;
        Timeout --;
      }
    }
    Index --;
  }
  //
  // Sets debug port for 0xd1 to indicate Xhci debug port ready
  //
  IoWrite8((UINT16)XhcDebugIoConfig.Config.DebugPort, 0xd1);
  //
  // Stall 3 seconds to waiting for host/device communication completed
  //
  MicroSecondDelay (3000000);

  //
  // Clear status registers
  //
  ReadXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCPORTSC, &Data);
  WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCPORTSC, &Data);
  UpdateEventDequeuePointer(DbgBar, DebugContext, 0);
  return EFI_SUCCESS;
}


/**
 Calculate the TD size based on xHCI interface version

 @param [in]   RemainingSize    Remaining Byte Size

 @return TD size

**/
STATIC
UINT32
CalcTDSize (
  IN  UINTN                     RemainingSize
  )
{
  UINT32 TDSize = (UINT32)(RemainingSize + 0x400 - 1) / 0x400;
  return (TDSize < 0x1f) ? TDSize : 0x1f;
}

/**
 UsbIoWrite.

 @param [in]   BufferSize
 @param [in]   Buffer

 @retval EFI_SUCCESS

**/
EFI_STATUS
DebugXhcWrite(
  IN DEBUG_IO_PRIVATE_DATA      *Private,
  IN UINTN                      BufferSize,
  IN VOID                       *Buffer
  )
{
  EFI_STATUS                Status;
  UINT32                    DbgBar;
  UINT32                    Data;
  UINT64                    Pointer;
  DBG_CONTEXT               *DebugContext;
  UINT32                    CycleBit;
  UINT64                    Tsc;

  if (DetectXhci(&Private->HcToken, &DbgBar, &Private->DebugContextAddr) != EFI_SUCCESS) return EFI_DEVICE_ERROR;
  //
  // Get the debug context back from DCERSTBA
  //
  ReadXhcDebugReg (DbgBar, EfiWidthUint64, XHCI_DCERSTBA, &Pointer);
  DebugContext = (DBG_CONTEXT*)(UINTN)(Pointer - (UINTN)&((DBG_CONTEXT*)0)->EventRingSegmentTable);
  CycleBit = DebugContext->BulkOutTRB.CycleBit ^ 1;
  DebugContext->BulkOutTRB.DataBufferPointer = (UINT64)(UINTN)Buffer;
  DebugContext->BulkOutTRB.TransferLength = (UINT32)(BufferSize);
  DebugContext->BulkOutTRB.CycleBit       = CycleBit;
  DebugContext->BulkOutTRB.TDSize         = CalcTDSize(BufferSize);
  DebugContext->BulkOutLinkTRB.CycleBit   = CycleBit ^ 1;
  //
  // Ring the door bell
  //
  Data = 0x00000000;
  WriteXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCDB, &Data);
  Status = EFI_SUCCESS;
  Tsc = 0;
  while (TRUE) {
    ReadXhcDebugReg (DbgBar, EfiWidthUint32, XHCI_DCST, &Data);
    if (Data & B_DCST_ER) {
      if (UpdateEventDequeuePointer(DbgBar, DebugContext, (UINT64)(UINTN)&DebugContext->BulkOutTRB)) break;
    }
  }

  return Status;
}


/**
 Prints a string to the USB port

 @param [in]   OutputString     Ascii string to print to serial port.

 @retval None

**/
VOID
EFIAPI
XhcDebugPrint (
  IN CHAR8    *OutputString
  )
{
  UINTN                         StringLength;
  CHAR8                         *StringPtr;
  UINT32                        PciIndexSave;
  DBG_CONTEXT                   *DbgContext;
  CHAR8                         *TempBuffer;
  UINTN                         BufferLen;
  DEBUG_IO_PRIVATE_DATA         *Private;
  EFI_STATUS                    Status;

  Private    = mPrivate;
  if (Private == NULL) {
    Status = gBS->LocateProtocol (
                    &mXhcDebugHobGuid,
                    NULL,
                    &Private
                    );
    if (EFI_ERROR (Status)) {
      return ;
    }
    mPrivate = Private;
  }

  DbgContext = (DBG_CONTEXT *) (UINTN) Private->DebugContextAddr;
  TempBuffer = DbgContext->StringBuffer;
  BufferLen  = sizeof (DbgContext->StringBuffer);

  //
  // Save PCI index
  //
  PciIndexSave = IoRead32(0xcf8);

  StringLength = AsciiStrLen (OutputString);
  for (StringPtr = OutputString; StringLength >= BufferLen; StringLength -= BufferLen, StringPtr += BufferLen) {
    CopyMem (TempBuffer, StringPtr, BufferLen);
    DebugXhcWrite (Private, BufferLen, TempBuffer);
  }

  if (StringLength > 0) {
    CopyMem (TempBuffer, StringPtr, StringLength);
    DebugXhcWrite (Private, StringLength, TempBuffer);
  }

  //
  // Restore PCI index
  //
  IoWrite32(0xcf8, PciIndexSave);
}

EFI_STATUS
EFIAPI
XhcInitializeStatusCode (
  VOID
  )
{
  DEBUG_IO_PRIVATE_DATA         *Private;
  UINT32                        PciIndexSave;
  EFI_STATUS                    Status;
  EFI_HANDLE                    Handle;

  Status = gBS->LocateProtocol (
                  &mXhcDebugHobGuid,
                  NULL,
                  &Private
                  );
  if (!EFI_ERROR (Status)) {
    mPrivate = Private;
    return EFI_SUCCESS;
  }

  Status = gBS->AllocatePool (
                  EfiACPIMemoryNVS,
                  sizeof (DEBUG_IO_PRIVATE_DATA),
                  (VOID**) &mPrivate
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem (mPrivate, sizeof (DEBUG_IO_PRIVATE_DATA));
  Private = GetFirstGuidHob (&mXhcDebugHobGuid);
  if (Private != NULL) {
    CopyMem (mPrivate, Private, sizeof (DEBUG_IO_PRIVATE_DATA));
    return EFI_SUCCESS;
  }

  Private = mPrivate;

  PciIndexSave = IoRead32(0xcf8);
  Status = DetectXhci(&Private->HcToken, NULL, NULL);
  if (EFI_ERROR (Status)) {
    IoWrite32(0xcf8, PciIndexSave);
    return Status;
  }

  Status = InitXhciDebugPort(Private->HcToken, NULL, &Private->DebugContextAddr, FALSE);
  if (EFI_ERROR (Status) && Status != EFI_TIMEOUT) {
    IoWrite32(0xcf8, PciIndexSave);
    return Status;
  }
  IoWrite32(0xcf8, PciIndexSave);

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &mXhcDebugHobGuid,
                  EFI_NATIVE_INTERFACE,
                  Private
                  );

  return Status;
}

