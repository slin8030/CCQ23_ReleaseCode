//;******************************************************************************
//;* Copyright (c) 1983-2012, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//;
//; Module Name:
//;
//;   XhcStatusCode.h
//;
//; Abstract:
//;
//;   XhcStatusCode header file
//;

#ifndef _XHC_STATUS_CODE_H_
#define _XHC_STATUS_CODE_H_

#include <Uefi.h>
#include <Pi/PiHob.h>
#include <Guid/StatusCodeDataTypeDebug.h>

#define INSYDE_USB3D_VID        0x1656
#define INSYDE_USB3D_PID        0x8500
#define INSYDE_USB3D_REV        0x0001
#define XHCI                    0x01
#define BRIDGE                  0x0f
#define XHCI_BRIDGE             0x03
#define PLATFORM_USB_BAR        0xFC000000
#define BASE_TICKS              3333333LL
#define TICKS_PER_SECOND        (1000LL * BASE_TICKS)
#define EVENT_RING_SIZE         8
//
// The definition of XHCI Token
//
#define B_TOKEN_XHCI_PCI_DEV    0xf8000000
#define N_TOKEN_XHCI_PCI_DEV    27
#define B_TOKEN_XHCI_PCI_FUN    0x07000000
#define N_TOKEN_XHCI_PCI_FUN    24
#define B_TOKEN_XHCI_PCI_BRIDGE 0x00ff0000
#define N_TOKEN_XHCI_PCI_BRIDGE 16
#define B_TOKEN_DEBUG_CAP_INDEX 0x0000fffc
#define B_TOKEN_PORT_ROUTING    0x00000002
#define B_TOKEN_LMC             0x00000001

//
// XHCI Debug Capability Registers
//
#define XHCI_DCID               0x00          /* Debug Capability ID Register Offset */
#define   B_DCID_ID               0x000000ff  /* Capability ID */
#define   B_DCID_NEXTP            0x0000ff00  /* Next Capability Pointer */
#define   N_DCID_NEXTP            8           /* Next Capability Pointer (position) */
#define   B_DCID_DCERST_MAX       0x001f0000  /* Debug Capability Event Ring Segment Table Max */
#define   N_DCID_DCERST_MAX       16          /* Debug Capability Event Ring Segment Table Max (position) */
#define XHCI_DCDB               0x04          /* Debug Capability Doorbell Register Offset */
#define   B_DCDB_DB_TARGET        0x0000ff00  /* DB Target */
#define   N_DCDB_DB_TARGET        8           /* DB Target (position) */
#define XHCI_DCERSTSZ           0x08          /* Debug Capability Event Ring Segment Table Size Register Offset */
#define XHCI_DCERSTBA           0x10          /* Debug Capability Event Ring Segment Table Base Address Register Offset */
#define XHCI_DCERDP             0x18          /* Debug Capability Event Ring Dequeue Pointer Register Offset */
#define XHCI_DCCTRL             0x20          /* Debug Capability Control Register Offset */
#define   B_DCCTRL_DCR            0x00000001  /* DbC Run */
#define   B_DCCTRL_LES            0x00000002  /* Link Status Event Enable */
#define   B_DCCTRL_HOT            0x00000004  /* Halt OUT TR */
#define   B_DCCTRL_HIT            0x00000008  /* Halt IN TR */
#define   B_DCCTRL_DRC            0x00000010  /* DbC Run Change */
#define   B_DCCTRL_MBSZ           0x00ff0000  /* Debug Max Burst Size */
#define   N_DCCTRL_MBSZ           16          /* Debug Max Burst Size (position) */
#define   B_DCCTRL_DEVICE_ADDR    0x3f000000  /* Device Address */
#define   N_DCCTRL_DEVICE_ADDR    24          /* Device Address (position) */
#define   B_DCCTRL_DCE            0x80000000  /* Debug Capability Enable */
#define XHCI_DCST               0x24          /* Debug Capability Status Register Offset */
#define   B_DCST_ER               0x00000001  /* Event Ring Not Empty */
#define   B_DCST_DBG_PORT_NUM     0xff000000  /* Debug Port Number */
#define   N_DCST_DBG_PORT_NUM     24          /* Debug Port Number (position) */
#define XHCI_DCPORTSC           0x28          /* Debug Capability Port Status and Control Register Offset */
#define   B_DCPORTSC_CCS          0x00000001  /* Current Connect Status */
#define   B_DCPORTSC_PED          0x00000002  /* Port Enabled/Disabled */
#define   B_DCPORTSC_PR           0x00000010  /* Port Reset */
#define   B_DCPORTSC_PLS          0x000001e0  /* Port Link State */
#define   N_DCPORTSC_PLS          5           /* Port Link State (position) */
#define   B_DCPORTSC_PS           0x00003c00  /* Port Speed */
#define   N_DCPORTSC_PS           10          /* Port Speed (position) */
#define   B_DCPORTSC_CSC          0x00020000  /* Connect Status Change */
#define   B_DCPORTSC_PRC          0x00200000  /* Port Reset Change */
#define   B_DCPORTSC_PLC          0x00400000  /* Port Link Status Change */
#define   B_DCPORTSC_CEC          0x00800000  /* Port Config Error Change */
#define XHCI_DCCP               0x30          /* Debug Capability Context Pointer Register Offset */
#define XHCI_DCDDI1             0x38          /* Debug Capability Device Descriptor Info Register 1 Offset */
#define   B_DCDDI1_PROTOCOL       0x000000ff  /* DbC Protocol */
#define   B_DCDDI1_VID            0xffff0000  /* Vendor ID */
#define   N_DCDDI1_VID            16          /* Vendor ID (position) */
#define XHCI_DCDDI2             0x3c          /* Debug Capability Device Descriptor Info Register 2 Offset */
#define   B_DCDDI2_PID            0x0000ffff  /* Product ID */
#define   B_DCDDI2_REV            0xffff0000  /* Device Revision */
#define   N_DCDDI2_REV            16          /* Device Revision (position) */
//
// XHCI Local Memory Capability Registers
//
#define XHCI_LMC                0x00          /* Local Mamory Capability ID Register Offset */
#define   B_LMC_ID                0x000000ff  /* Capability ID */
#define   B_LMC_NEXTP             0x0000ff00  /* Next Capability Pointer */
#define   N_LMC_NEXTP             8           /* Next Capability Pointer (position) */
#define   B_LMC_LME               0x00010000  /* Local Mamory Enable */
#define   N_LMC_LME               16          /* Local Mamory Enable (position) */
#define XHCI_LMSZ               0x04          /* Local Mamory Size */
//
// XHCI TRB type ID
//
#define TRB_TYPE_NORMAL         0x01
#define TRB_TYPE_LINK           0x06
//
// Endpoint Type
//
#define EP_TYPE_BULK_OUT        0x02
#define EP_TYPE_BULK_IN         0x06

#pragma pack(1)

typedef struct {
  UINT64                        RingSegmentBaseAddress;
  UINT64                        RingSegmentSize : 16;
  UINT64                        RsvdZ           : 48;
} XHCI_EVENT_RING_SEGMENT_TABLE;

typedef struct {
  UINT64                        Str0DescAddr;
  UINT64                        ManufacturerDescAddr;
  UINT64                        ProductDescAddr;
  UINT64                        SerialNumDescAddr;
  UINT32                        Str0Len          : 8;
  UINT32                        ManufacturerLen  : 8;
  UINT32                        ProductLen       : 8;
  UINT32                        SerialNumLen     : 8;
} XHCI_DBG_INFO_CONTEXT;

typedef struct {
  UINT32                        EPState          : 3;
  UINT32                        RsvdZ0           : 5;
  UINT32                        Mult             : 2;
  UINT32                        MaxPStreams      : 5;
  UINT32                        LSA              : 1;
  UINT32                        Interval         : 8;
  UINT32                        RsvdZ1           : 8;
  UINT32                        ForceEvent       : 1;
  UINT32                        ErrorCount       : 2;
  UINT32                        EPType           : 3;
  UINT32                        RsvdZ2           : 2;
  UINT32                        MaxBurstSize     : 8;
  UINT32                        MaxPacketSize    : 16;
  UINT64                        TRDequeuePointer;
  UINT32                        AvgTRBLength     : 16;
  UINT32                        MaxESITPayload   : 16;
  UINT32                        RsvdZ3;
  UINT64                        RsvdZ4;
} XHCI_ENDPOINT_CONTEXT;

typedef struct {
  UINT64                        DataBufferPointer;
  UINT32                        TransferLength   : 17;
  UINT32                        TDSize           : 5;
  UINT32                        InterruptTarget  : 10;
  UINT32                        CycleBit         : 1;
  UINT32                        EvalNextTRB      : 1;
  UINT32                        ISP              : 1;
  UINT32                        NoSnoop          : 1;
  UINT32                        ChainBit         : 1;
  UINT32                        IOC              : 1;
  UINT32                        IDT              : 1;
  UINT32                        RsvdZ1           : 3;
  UINT32                        TRBType          : 6;
  UINT32                        RsvdZ2           : 16;
} XHCI_NORMAL_TRB;

typedef struct {
  UINT64                        RingSegmentPointer;
  UINT32                        RsvdZ1           : 22;
  UINT32                        InterruptTarget  : 10;
  UINT32                        CycleBit         : 1;
  UINT32                        ToggleCycle      : 1;
  UINT32                        RsvdZ2           : 2;
  UINT32                        ChainBit         : 1;
  UINT32                        IOC              : 1;
  UINT32                        RsvdZ3           : 4;
  UINT32                        TRBType          : 6;
  UINT32                        RsvdZ4           : 16;
} XHCI_LINK_TRB;

typedef struct {
  UINT64                        TRBPointer;
  UINT32                        TransferLength   : 24;
  UINT32                        CompletionCode   : 8;
  UINT32                        CycleBit         : 1;
  UINT32                        RsvdZ0           : 1;
  UINT32                        EventData        : 1;
  UINT32                        RsvdZ1           : 7;
  UINT32                        TRBType          : 6;
  UINT32                        EPID             : 5;
  UINT32                        RsvdZ2           : 3;
  UINT32                        SlotID           : 8;
} XHCI_TRANSFER_EVENT;
//
// Debug Context Signature
//
#define USB_XHCID_SIGNATURE     SIGNATURE_32('x','h','c','d')

#define DEBUG_

typedef struct {
  UINT32                        LmcCapID;
  UINT32                        LmcSize;
  UINT32                        Signature;
  UINT32                        ContextSize;
  CHAR16                        Str0[8];
  CHAR16                        ManufacturerStr[8];
  CHAR16                        SerialNum[8];
  XHCI_EVENT_RING_SEGMENT_TABLE EventRingSegmentTable;
  CHAR16                        ProductStr[24];
  XHCI_TRANSFER_EVENT           Event[EVENT_RING_SIZE];
  XHCI_DBG_INFO_CONTEXT         DebugInfoContext;
  UINT32                        Rsvd0[7];
  XHCI_ENDPOINT_CONTEXT         EndpointOut;
  UINT32                        Rsvd1[8];
  XHCI_ENDPOINT_CONTEXT         EndpointIn;
  UINT32                        Rsvd2[8];
  XHCI_NORMAL_TRB               BulkOutTRB;
  XHCI_LINK_TRB                 BulkOutLinkTRB;
  XHCI_NORMAL_TRB               BulkInTRB;
  XHCI_LINK_TRB                 BulkInLinkTRB;
  CHAR8                         StringBuffer[EFI_STATUS_CODE_DATA_MAX_SIZE];
} DBG_CONTEXT;

typedef struct _DEBUGIO_CONFIG {
  UINT32 RPR           :  1;
  UINT32 Bypass        :  1;
  UINT32 DriverType    :  2;
  UINT32 UsbHcPort     :  4;
  UINT32 UsbDebugPort  :  8;
  UINT32 DebugPort     : 16;
  UINT32 BypassTimeout;
} DEBUGIO_CONFIG;

typedef struct _DEBUG_IO_PRIVATE_DATA {
  EFI_HOB_GUID_TYPE             EfiHobGuidType;
  UINT32                        HcToken;
  UINT32                        DebugContextAddr;
} DEBUG_IO_PRIVATE_DATA;

#pragma pack()

#endif
