/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/


Name(SP3O, 0x2e)        // Super I/O (Winbond 3xx) Index/Data configuration port for ASL.
Name(IO4B, 0xa20)       // Super I/O (Winbond 3xx) GPIO base address
Name(IO4L, 0x20)        // Super I/O (Winbond 3xx) GPIO base address length
Name(SP1O, 0x4e)        // Super IO Index/Data configuration port for ASL.
Name(PMBS, 0x400)       // ASL alias for ACPI I/O base address.
Name(SMIP, 0xb2)        // I/O port to trigger SMI
Name(APCB, 0xfec00000)  // Default I/O APIC(s) memory start address, 0x0FEC00000 - default, 0 - I/O APIC's disabled
Name(APCL, 0x1000)      // I/O APIC(s) memory decoded range, 0x1000 - default, 0 - I/O APIC's not decoded
Name(PMCB, 0xfed03000)  // PMC Base Address
Name(IBAS, 0xfed08000)  // ILB Base Address
Name(SRCB, 0xfed1c000)  // RCBA (Root Complex Base Address)
Name(HPTB, 0xfed00000)  // Same as HPET_BASE_ADDRESS for ASL use
Name(PEBS, 0xe0000000)  //
Name(PELN, 0x10000000)  //
Name(FMBL, 0x1)         // Platform Flavor - Mobile flavor for ASL code.
Name(FDTP, 0x2)         // Platform Flavor - Desktop flavor for ASL code.
Name(GCDD, 0x1)         // GET_CURRENT_DISPLAY_DEVICE_SMI
Name(DSTA, 0xa)         // DISPLAY_SWITCH_TOGGLE_ACPI_SMI
Name(DSLO, 0x2)         // DISPLAY_SWITCH_LID_OPEN_ACPI_SMI
Name(DSLC, 0x3)         // DISPLAY_SWITCH_LID_CLOSE_ACPI_SMI
Name(PITS, 0x10)        // POPUP_ICON_TOGGLE_SMI
Name(SBCS, 0x12)        // SET_BACKLIGHT_CONTROL_SMI
Name(SALS, 0x13)        // SET_ALI_LEVEL_SMI
Name(LSSS, 0x2a)        // LID_STATE_SWITCH_SMI
Name(PSSS, 0x2b)        // POWER_STATE_SWITCH_SMI
Name(SOOT, 0x35)        // SAVE_OSB_OS_TYPE_SMI
Name(ESCS, 0x48)        // ENABLE_SMI_C_STATE_COORDINATION_SMI
Name(SDGV, 0x1c)        // UHCI Controller HOST_ALERT's bit offset within the GPE block. GPIO[0:15] corresponding to GPE[16:31]
Name(ACPH, 0xde)        // North Bridge Scratchpad Data Register for patch ACPI.
Name(ASSB, 0x0)         // ACPI Sleep State Buffer for BIOS Usage.
Name(AOTB, 0x0)         // ACPI OS Type Buffer for BIOS Usage.
Name(AAXB, 0x0)         // ACPI Auxiliary Buffer for BIOS Usage.
Name(PEHP, 0x1)         // _OSC: Pci Express Native Hot Plug Control
Name(SHPC, 0x0)         // _OSC: Standard Hot Plug Controller (SHPC) Native Hot Plug control
Name(PEPM, 0x1)         // _OSC: Pci Express Native Power Management Events control
Name(PEER, 0x1)         // _OSC: Pci Express Advanced Error Reporting control
Name(PECS, 0x1)         // _OSC: Pci Express Capability Structure control
Name(ITKE, 0x0)         // This will be overridden by the ITK module.
Name(FTBL, 0x4)         // Platform Flavor - Tablet flavor for ASL code.
//[-start-160406-IB07400715-modify]//
//#if defined BXTI_PF_ENABLE && BXTI_PF_ENABLE
Name(GRBM, 0xF)         // Board id for Gordon Ridge BMP MRB
Name(MNHL, 0x5)         // Board id for Mineral Hill
Name(OXHL, 0x6)         // Board id for Oxbow Hill
Name(LFHL, 0x7)         // Board id for Leaf Hill
Name(JNHL, 0x8)         // Board id for Juniper Hill
//#endif
//[-end-160406-IB07400715-modify]//