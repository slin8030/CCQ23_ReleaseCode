/** @file
  File to contain all the hardware specific stuff for the Smm USB dispatch protocol.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/
#include "ScSmmHelpers.h"
#include <Protocol/PciRootBridgeIo.h>

GLOBAL_REMOVE_IF_UNREFERENCED SC_SMM_SOURCE_DESC mUSB1_LEGACY = {
  SC_SMM_NO_FLAGS,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_EN
      },
      S_SMI_EN,
      N_SMI_EN_LEGACY_USB
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_STS
      },
      S_SMI_STS,
      N_SMI_STS_LEGACY_USB
    }
  }
};

GLOBAL_REMOVE_IF_UNREFERENCED SC_SMM_SOURCE_DESC mUSB3_LEGACY = {
  SC_SMM_NO_FLAGS,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_EN
      },
      S_SMI_EN,
      N_SMI_EN_LEGACY_USB3
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_SMI_STS
      },
      S_SMI_STS,
      N_SMI_STS_LEGACY_USB3
    }
  }
};

typedef enum {
  ScUsbControllerLpc0 = 0,
  ScUsbControllerXhci,
  ScUsbControllerTypeMax
} SC_USB_CONTROLLER_TYPE;

typedef struct {
  UINT8                   Function;
  UINT8                   Device;
  SC_USB_CONTROLLER_TYPE UsbConType;
} USB_CONTROLLER;

GLOBAL_REMOVE_IF_UNREFERENCED USB_CONTROLLER  mUsbControllersMap[] = {
  {
    PCI_FUNCTION_NUMBER_PCH_LPC,
    PCI_DEVICE_NUMBER_PCH_LPC,
    ScUsbControllerLpc0
  },
  {
    PCI_FUNCTION_NUMBER_XHCI,
    PCI_DEVICE_NUMBER_XHCI,
    ScUsbControllerXhci
  }
};
/**

  Find the handle that best matches the input Device Path and return the USB controller type

  @param[in]  DevicePath           Pointer to the device Path table
  @param[out] Controller           Returned with the USB controller type of the input device path

  @retval EFI_SUCCESS             Find the handle that best matches the input Device Path
  @exception EFI_UNSUPPORTED      Invalid device Path table or can't find any match USB device path
                                  SC_USB_CONTROLLER_TYPE The USB controller type of the input
                                  device path

**/
EFI_STATUS
DevicePathToSupportedController (
  IN  EFI_DEVICE_PATH_PROTOCOL   *DevicePath,
  OUT SC_USB_CONTROLLER_TYPE    *Controller
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                DeviceHandle;
  ACPI_HID_DEVICE_PATH      *AcpiNode;
  PCI_DEVICE_PATH           *PciNode;
  EFI_DEVICE_PATH_PROTOCOL  *RemaingDevicePath;
  UINT8                     UsbIndex;
  //
  // Find the handle that best matches the Device Path. If it is only a
  // partial match the remaining part of the device path is returned in
  // RemainingDevicePath.
  //
  RemaingDevicePath = DevicePath;
  Status = gBS->LocateDevicePath (
                  &gEfiPciRootBridgeIoProtocolGuid,
                  &DevicePath,
                  &DeviceHandle
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  DevicePath = RemaingDevicePath;

  //
  // Get first node: Acpi Node
  //
  AcpiNode = (ACPI_HID_DEVICE_PATH *) RemaingDevicePath;

  if (AcpiNode->Header.Type != ACPI_DEVICE_PATH ||
      AcpiNode->Header.SubType != ACPI_DP ||
      DevicePathNodeLength (&AcpiNode->Header) != sizeof (ACPI_HID_DEVICE_PATH) ||
      AcpiNode->HID != EISA_PNP_ID (0x0A03) ||
      AcpiNode->UID != 0
      ) {
    return EFI_UNSUPPORTED;
  } else {
    //
    // Get the next node: Pci Node
    //
    RemaingDevicePath = NextDevicePathNode (RemaingDevicePath);
    PciNode           = (PCI_DEVICE_PATH *) RemaingDevicePath;
    if (PciNode->Header.Type != HARDWARE_DEVICE_PATH ||
        PciNode->Header.SubType != HW_PCI_DP ||
        DevicePathNodeLength (&PciNode->Header) != sizeof (PCI_DEVICE_PATH)
        ) {
      return EFI_UNSUPPORTED;
    }

    for (UsbIndex = 0; UsbIndex < sizeof (mUsbControllersMap) / sizeof (USB_CONTROLLER); UsbIndex++) {
      if ((PciNode->Device == mUsbControllersMap[UsbIndex].Device) &&
          (PciNode->Function == mUsbControllersMap[UsbIndex].Function)) {
        *Controller = mUsbControllersMap[UsbIndex].UsbConType;
        return EFI_SUCCESS;
      }
    }

    return EFI_UNSUPPORTED;
  }
}

//[-start-160924-IB07400789-add]//
BOOLEAN
IsSupportedUsbController (
  IN  SC_SMM_CONTEXT         *Context
  )
{
  SC_USB_CONTROLLER_TYPE    Controller;
  EFI_STATUS                Status;
  
  Status = DevicePathToSupportedController (Context->Usb.Device, &Controller);
  if (EFI_ERROR (Status)) {
    return FALSE;
  } else {
    return TRUE;
  }
}
//[-end-160924-IB07400789-add]//


/**

  Maps a USB context to a source description.

  @param[in]  Context              The context we need to map.  Type must be USB.
  @param[out] SrcDesc              The source description that corresponds to the given context.

  @retval None.

**/
VOID
MapUsbToSrcDesc (
  IN  SC_SMM_CONTEXT         *Context,
  OUT SC_SMM_SOURCE_DESC     *SrcDesc
  )
{
  SC_USB_CONTROLLER_TYPE Controller;
  EFI_STATUS              Status;

  Status = DevicePathToSupportedController (Context->Usb.Device, &Controller);
  //
  // Either the device path passed in by the child is incorrect or
  // the ones stored here internally are incorrect.
  //
  ASSERT_EFI_ERROR (Status);

  switch (Context->Usb.Type) {
    case UsbLegacy:
      switch (Controller) {
        case ScUsbControllerLpc0:
          CopyMem ((VOID *) SrcDesc, (VOID *) (&mUSB1_LEGACY), sizeof (SC_SMM_SOURCE_DESC));
          break;

        case ScUsbControllerXhci:
          CopyMem ((VOID *) SrcDesc, (VOID *) (&mUSB3_LEGACY), sizeof (SC_SMM_SOURCE_DESC));
          break;

        default:
          ASSERT (FALSE);
          break;
      }
      break;

    case UsbWake:
      ASSERT (FALSE);
      break;

    default:
      ASSERT (FALSE);
      break;
  }
}
//[-start-170627-IB07400880-add]//
BOOLEAN     
IsXhciPendingSmi (
  IN  UINT8 SmiSts 
  )
{
  UINT32    XhciPciBase;
  UINT32    UsbMmio;
  UINT32    Data32;
  
  if (SmiSts == N_SMI_STS_LEGACY_USB3) {
    //
    // Get XHCI PCIe base address
    //
    XhciPciBase = (UINT32)MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_XHCI, PCI_FUNCTION_NUMBER_XHCI, 0);
//    DEBUG ((EFI_D_INFO, "XhciPciBase = %x\n", XhciPciBase));

    //
    // Get XHCI MMIO
    //
    UsbMmio = MmioRead32 (XhciPciBase + R_XHCI_MEM_BASE) & B_XHCI_MEM_BASE_BA;
    if (UsbMmio == 0) {
      return FALSE;
    }

    //
    // Check MMIO exist or not
    //
    Data32 = MmioRead32 (UsbMmio);
    if (Data32 == 0xFFFFFFFF) {
      return FALSE;
    }

    //
    // If SMI ON Event interrupt bit is set, XHCI pending SMI occur
    //
    Data32 = MmioRead32 (UsbMmio + R_XHCI_USB_LEGACY_CTRL_STS);
    if ((Data32 & (B_XHCI_SMI_ON_EVENT_INT_STS | B_XHCI_USB_SMI_ENABLE)) == (B_XHCI_SMI_ON_EVENT_INT_STS | B_XHCI_USB_SMI_ENABLE)) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Clear the SMI status bit after the SMI handling is done

  @param[in] SrcDesc              Pointer to the SC SMI source description table

  @retval None
**/
VOID
EFIAPI
ScSmmUsbClearSource (
  IN SC_SMM_SOURCE_DESC *SrcDesc
  )
{
  UINTN DescIndex;

  for (DescIndex = 0; DescIndex < NUM_STS_BITS; DescIndex++) {
    if (!IS_BIT_DESC_NULL (SrcDesc->Sts[DescIndex])) {
      if (IsXhciPendingSmi(SrcDesc->Sts[DescIndex].Bit)) {
        continue;
      }
      WriteBitDesc (&SrcDesc->Sts[DescIndex], 1, TRUE);
    }
  }
}
//[-end-170627-IB07400880-add]//
