//
// This file contains an 'Intel Pre-EFI Module' and is licensed
// for Intel CPUs and Chipsets under the terms of your license 
// agreement with Intel or your vendor.  This file may be      
// modified by the user, subject to additional terms of the    
// license agreement                                           
//
/** @file
  Define private data structure for UHCI and EHCI.
  
Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#ifndef _PEI_ICH_USB_H
#define _PEI_ICH_USB_H

#include <PiPei.h>
#include <ScAccess.h>
#include <Ppi/UsbController.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/Stall.h>
//#include <PchCommonDefinitions.h>

#define PEI_ICH_UHCI_SIGNATURE          SIGNATURE_32 ('U', 'H', 'C', '7')
#define PEI_ICH_EHCI_SIGNATURE          SIGNATURE_32 ('E', 'H', 'C', 'I')

#define USB_CONTROLLER_IO_RANGE         32

#define PCI_MAX_EHCI_USB_CONTROLLERS    1

typedef struct {
  UINTN                   Signature;
  PEI_USB_CONTROLLER_PPI  UsbControllerPpi;
  EFI_PEI_PPI_DESCRIPTOR  PpiList;
  UINTN                   MmioBase[PCI_MAX_EHCI_USB_CONTROLLERS];
} ICH_EHCI_DEVICE;

#define ICH_EHCI_DEVICE_FROM_THIS(a) \
  CR (a, ICH_EHCI_DEVICE, UsbControllerPpi, PEI_ICH_EHCI_SIGNATURE)

#endif
