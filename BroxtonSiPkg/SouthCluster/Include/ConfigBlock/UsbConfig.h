/** @file
  USB policy

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _USB_CONFIG_H_
#define _USB_CONFIG_H_

#define USB_CONFIG_REVISION 3
extern EFI_GUID gUsbConfigGuid;

#pragma pack (push,1)

//
// Overcurrent pins, the values match the setting of SC EDS, please refer to SC EDS for more details
//
typedef enum {
  ScUsbOverCurrentPin0 = 0,
  ScUsbOverCurrentPin1,
  ScUsbOverCurrentPinSkip,
  ScUsbOverCurrentPinMax
} SC_USB_OVERCURRENT_PIN;

#define XHCI_MODE_OFF 0
#define XHCI_MODE_ON  1

typedef struct {
  UINT8 Mode     : 2;    /// 0: Disable; 1: Enable, 2: Auto, 3: Smart Auto
  UINT8 Rsvdbits : 4;
} SC_USB30_CONTROLLER_SETTINGS;

/**
  This structure configures per USB2 AFE settings.
  It allows to setup the port parameters.
**/
typedef struct {

  /**
    PerPort Half Bit Pre-emphasis (PERPORTTXPEHALF)
    1b - select half-bit pre-emphasis
    <b>0b - select full-bit pre-emphasis</b>

  **/
  UINT32 PerPortTxPeHalf : 1;

  /**
    Per Port HS Preemphasis Bias (PERPORTPETXISET)
    000b - 0mV
    001b - 11.25mV
    010b - 16.9mV
    011b - 28.15mV
    100b - 28.15mV
    <b>101b - 39.35mV</b>
    110b - 45mV
    111b - 56.3mV
  **/
  UINT32 PerPortPeTxiSet : 3;

  /**
    Per Port HS Transmitter Bias (PERPORTTXISET)
    <b>000b - 0mV</b>
    001b - 11.25mV
    010b - 16.9mV
    011b - 28.15mV
    100b - 28.15mV
    101b - 39.35mV
    110b - 45mV
    111b - 56.3mV
  **/
  UINT32 PerPortTxiSet : 3;

  /**
    Select the skew direction for HS transition (HSSKEWSEL)
    1b - Delay HS Driver 1)0 Transition
    <b>0b - Delay HS Driver 0)1 Transition</b>

  **/
  UINT32 HsSkewSel : 1;

  /**
    Per Port HS Transmitter Emphasis (IUSBTXEMPHASISEN)
    00b - Emphasis OFF
    01b - De-emphasis ON (strength = TXISET+PETXISET)
    10b - Pre-emphasis ON (strength = TXISET+PETXISET)
    <b>11b - Pre-emphasis & De-emphasis ON (strength = TXISET+PETXISET</b>
  **/
  UINT32 IUsbTxEmphasisEn : 2;

  /**
    PerPort HS Receiver Bias (PERPORTRXISET)
    000b - 80uA
    001b - 90uA
    010b - 100uA
    011b - 110uA
    <b>100b - 120uA</b>
    101b - 130uA
    110b - 140uA
    111b - 150uA
  **/
  UINT32 PerPortRXISet : 3;

  /**
    Delay/skew's strength control for HS driver (HSNPREDRVSEL)
    <b>00b - No Skew</b>
    01b - 30mV
    10b - 40mV
    11b - 50mV
  **/
  UINT32 HsNpreDrvSel : 2;
  UINT32 RsvdBits0 : 17;     ///< Reserved bits
} SC_USB20_AFE;

/**
  This structure configures per USB2 port physical settings.
  It allows to setup the port location and port length, and configures the port strength accordingly.
**/
typedef struct {
  UINT32 Enable    :  1;   ///< 0: Disable; <b>1: Enable</b>.
  UINT32 RsvdBits0 : 31;   ///< Reserved bits
  /**
    These members describe the specific over current pin number of USB 2.0 Port N.
    It is SW's responsibility to ensure that a given port's bit map is set only for
    one OC pin Description. USB2 and USB3 on the same combo Port must use the same
    OC pin (see: SC_USB_OVERCURRENT_PIN).
  **/
  UINT8  OverCurrentPin;
  UINT8  Rsvd0[3];         ///< Reserved bytes, align to multiple 4.
  /**
  This structure configures per USB2 AFE settings. It allows to setup the port parameters.
**/
  SC_USB20_AFE AfeConfig;
} SC_USB20_PORT_CONFIG;

/**
  This structure describes whether the USB3 Port is enabled by platform modules.
**/
typedef struct {
  UINT32 Enable    :  1;   ///< 0: Disable; <b>1: Enable</b>.
  UINT32 RsvdBits0 : 31;   ///< Reserved bits
  /**
    These members describe the specific over current pin number of USB 3.0 Port N.
    It is SW's responsibility to ensure that a given port's bit map is set only for
    one OC pin Description. USB2 and USB3 on the same combo Port must use the same
    OC pin (see: SC_USB_OVERCURRENT_PIN).
  **/
  UINT8  OverCurrentPin;
  UINT8  Rsvd0[3];         ///< Reserved bytes, align to multiple 4
  UINT32 Rsvd1[2];         ///< Reserved bytes
} SC_USB30_PORT_CONFIG;

typedef struct {
  /**
    This member describes whether or not the xDCI controller should be enabled.
    0: Disabled; <b>1: PCI Mode</b> 2: ACPI Mode
  **/
  UINT32 Enable   : 2;
  UINT32 RsvdBits : 30;  ///< Reserved bits
} SC_XDCI_CONFIG;

/**
  These members describe some settings which are related to the HSIC ports.
**/
typedef struct {
  /**
    <b>0: Disable</b>; 1: Enable HSIC support.
  **/
  UINT32  Enable          : 1;
  UINT32  RsvdBits        : 31;
} SC_XHCI_HSIC_PORT;

/**
  These members describe some settings which are related to the SSIC ports.
**/
typedef struct {
  SC_XHCI_HSIC_PORT  HsicPort[XHCI_MAX_HSIC_PORTS];
} SC_HSIC_CONFIG;

typedef enum {
  XhciSsicRateA = 1,
  XhciSsicRateB
} SC_XHCI_SSIC_PORT_RATE;

/**
  These members describe some settings which are related to the SSIC ports.
**/
typedef struct {
  /**
    <b>0: Disable</b>; 1: Enable SSIC support.
  **/
  UINT32  Enable          : 1;
  /**
    <b>(Test)</b> Default is <b>XhciSsicRateA</b>
  **/
  UINT32  Rate            : 2;
  UINT32  RsvdBits        : 29;
} SC_XHCI_SSIC_PORT;
/**
  These members describe some settings which are related to the SSIC ports.
**/
typedef struct {
  SC_XHCI_SSIC_PORT  SsicPort[XHCI_MAX_SSIC_PORTS];
  /**
    0: Disable <b>1: Enable</b> SSIC Data lane Power Gating
  **/
  UINT32  DlanePwrGating : 1;
  UINT32  RsvdBits       : 31;
} SC_SSIC_CONFIG;

/**
  This member describes the expected configuration of the SC USB controllers,
  Platform modules may need to refer Setup options, schematic, BIOS specification
  to update this field.
  The Usb20OverCurrentPins and Usb30OverCurrentPins field must be updated by referring
  the schematic.
**/
typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Deprecate UsbPerPortCtl
    Revision 3: Add SC_USB20_AFE structure
  **/
  CONFIG_BLOCK_HEADER   Header;     ///< Config Block Header
  /**
    This policy will disable XHCI compliance mode on all ports. Complicance Mode should be default enabled.
    For the platform that support USB Type-C, it can disable Compliance Mode, and enable Compliance Mode when testing.
    <b>0:Disable</b> , 1: Enable
  **/
  UINT32 DisableComplianceMode : 1;
  UINT32 UsbPerPortCtl         : 1; ///< @deprecated since revision 2
  UINT32 RsvdBits              : 30;
  SC_USB30_CONTROLLER_SETTINGS  Usb30Settings;
  SC_USB20_PORT_CONFIG          PortUsb20[SC_MAX_USB2_PORTS];
  /**
    These members describe whether the USB3 Port N of SC is enabled by platform modules.
  **/
  SC_USB30_PORT_CONFIG          PortUsb30[SC_MAX_USB3_PORTS];
  /**
    This member describes whether or not the xDCI controller should be enabled.
  **/
  SC_XDCI_CONFIG                XdciConfig;
  /**
    These members describe some settings which are related to the HSIC ports.
  **/
  SC_HSIC_CONFIG                HsicConfig;
  /**
    These members describe some settings which are related to the SSIC ports.
  **/
  SC_SSIC_CONFIG                SsicConfig;
} SC_USB_CONFIG;

#pragma pack (pop)

#endif // _USB_CONFIG_H_
