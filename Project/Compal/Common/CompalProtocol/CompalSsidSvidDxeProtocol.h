/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _COMPAL_SSID_SVID_DXE_PROTOCOL_H_
#define _COMPAL_SSID_SVID_DXE_PROTOCOL_H_


#define COMPAL_SSID_SVID_DXE_PROTOCOL_GUID  \
  { \
    0xe7273837, 0xb16a, 0x4512, 0xaa, 0xa9, 0x70, 0x95, 0xf0, 0x1b, 0x24, 0xa0 \
  }

typedef struct _COMPAL_SSID_SVID_DXE_PROTOCOL COMPAL_SSID_SVID_DXE_PROTOCOL;

#define PLATFORM_SSID                       0x00000000
#define DONT_WRITE_SSID                     0xFFFFFFFF

#define END_OF_TABLE                        0xFFFF
#define DEVICE_ID_DONT_CARE                 0xFFFF

#define WIDTH8                              EfiBootScriptWidthUint8
#define WIDTH16                             EfiBootScriptWidthUint16
#define WIDTH32                             EfiBootScriptWidthUint32

#define ATI_VID                             0x1002
#define AMD_VID                             0x1022
#define NV_VID                              0x10DE
#define INTEL_VID                           0x8086

#define NULL_ID                             0xFFFF
#define ARRANDALE_DID                       0x0044
#define CLARKSFIELD_DID                     0xD132
#define EFI_PCI_CAPABILITY_ID_SSID          0x0D

#define PEG_BUS                             0x0101
#define GENERIC_BUS                         0x7F

#define AMD_NB_SSID_OFFSET                  0x50
#define ATI_VGA_SSID_OFFSET                 0x4C
#define NV_VGA_SSID_OFFSET                  0x40
#define INTEL_VGA_CONTROLLER_OFFSET         0x8C
#define CARDBUS_SSID_OFFSET                 0x40
#define SECONDARY_BUS_OFFSET                0x19

#define BASE_CLASS_MASK                     0xFF00
#define VGA_CLASSCODE                       0x0300
#define AUDIO_CLASSCODE                     0x0403
#define P2P_BRIDGE_CLASSCODE                0x0604
#define INTEL_EHCI_CLASSCODE                0x0C03
#define INTEL_EHCI_INTERFACE                0x20
#define INTEL_XHCI_CLASSCODE                0x0C03
#define INTEL_XHCI_INTERFACE                0x30

#pragma pack(1)

typedef
EFI_STATUS
( EFIAPI *COMPAL_SSID_SVID_FUNCTION ) (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
);

typedef struct {
    UINT16                     VendorId;
    UINT16                     DeviceId;
    UINT8                      Offset;
    UINT32                     SsidSvid;
    COMPAL_SSID_SVID_FUNCTION  CompalSsidSvidSpecialFunction;
} COMPAL_SSID_SVID_SPECIAL_TABLE;

#pragma pack()

//
// Member functions
//
typedef
EFI_STATUS
(EFIAPI *COMPAL_SSID_SVID_DXE_WRITE_PROTOCOL) (
    IN  UINT32                           SsidSvid,
    IN  COMPAL_SSID_SVID_SPECIAL_TABLE   *CompalSsidSvidSpecialTable
);

typedef
EFI_STATUS
(EFIAPI *COMPAL_SSID_SVID_DXE_PCI_READ_PROTOCOL) (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT8                            Offset,
    IN  EFI_BOOT_SCRIPT_WIDTH            DataWidth,
    IN  VOID                             *DataPtr
);

typedef
EFI_STATUS
(EFIAPI *COMPAL_SSID_SVID_DXE_PCI_WRITE_PROTOCOL) (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT8                            Offset,
    IN  EFI_BOOT_SCRIPT_WIDTH            DataWidth,
    IN  VOID                             *DataPtr
);

//
// Extern Guid
//
extern EFI_GUID gCompalSsidSvidDxeProtocolGuid;

//
// Protocol Definition
//
typedef struct _COMPAL_SSID_SVID_DXE_PROTOCOL {
  COMPAL_SSID_SVID_DXE_WRITE_PROTOCOL       CompalSsidSvidDxeWrite;
  COMPAL_SSID_SVID_DXE_PCI_READ_PROTOCOL    CompalSsidSvidDxePciRead;
  COMPAL_SSID_SVID_DXE_PCI_WRITE_PROTOCOL   CompalSsidSvidDxePciWrite;
} COMPAL_SSID_SVID_DXE_PROTOCOL;

#endif
