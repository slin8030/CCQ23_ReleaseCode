/** @file
  Dxe driver will register a ready to boot event to program SsidSvid.

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <IndustryStandard/Pci.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/CommonPciLib.h>
#include <Library/PciExpressLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeChipsetSvcLib.h>
#include <Library/PcdLib.h>
#include <Protocol/PciEnumerationComplete.h>


#include <Library/DxeOemSvcKernelLib.h>

#include "ScAccess.h"
#include "SaAccess.h"

//[-start-151220-IB1127138-remove]//
// VOID
// PchMsgBusWriteEx32WithS3Item (
//   IN     UINT8                           PortId,
//   IN     UINT32                          Register,
//   IN     UINT32                          Data32,
//   IN     UINT8                           WriteOpCode,
//   IN     UINT8                           Bar,          OPTIONAL
//   IN     UINT8                           Device,       OPTIONAL
//   IN     UINT8                           Function      OPTIONAL
//   )
// {
//   UINTN   PciExpressBase;
//   UINT32  Buffer32;
// 
//   Buffer32         = 0;
//   PciExpressBase = (UINTN) PcdGet64 (PcdPciExpressBaseAddress);
// 
//   Buffer32 = (UINT32) (((Bar & 0x07) << 8) | (((Device << 3) | (Function)) & 0xFF));
//   MmioWrite32 (
//     (UINTN) (PciExpressBase + MC_MCRXX),
//     (UINT32) Buffer32
//     );
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (PciExpressBase + MC_MCRXX),
//     1,
//     &Buffer32
//     );
// 
//   Buffer32 = (UINT32) (Register & MSGBUS_MASKHI);
//   MmioWrite32 (
//   (UINTN) (PciExpressBase + MC_MCRX),
//   (UINT32) Buffer32
//   );
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (PciExpressBase + MC_MCRX),
//     1,
//     &Buffer32
//     );
// 
//   Buffer32 = (UINT32) Data32;
//   MmioWrite32 (
//     (UINTN) (PciExpressBase + MC_MDR),
//     (UINT32) Buffer32
//     );  
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (PciExpressBase + MC_MDR),
//     1,
//     &Buffer32
//     );
// 
//   Buffer32 = (UINT32) ((WriteOpCode << 24) | (PortId << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN);
//   MmioWrite32 (
//     (UINTN) (PciExpressBase + MC_MCR),
//     (UINT32) Buffer32
//     );
//   S3BootScriptSaveMemWrite (
//     EfiBootScriptWidthUint32,
//     (UINTN) (PciExpressBase + MC_MCR),
//     1,
//     &Buffer32
//     );
//   return;
// }
// 
// 
// VOID
// SetMsgBus32WithS3Item (
//   IN     UINT8                           PortId,
//   IN     UINT32                          Register,
//   IN     UINT32                          Data32,
//   IN     UINT8                           WriteOpCode
//   )
// {
//   PchMsgBusWriteEx32WithS3Item(PortId, Register, Data32, WriteOpCode, 0x00, 0x00, 0x00);
// }
//[-end-151220-IB1127138-remove]//


VOID
EFIAPI
SetSsidSvidCallBack (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

VOID
ProgramSsidSvid (
  IN     UINT8     Bus,
  IN     UINT8     Dev,
  IN     UINT8     Func,
  IN     UINT32    SsidSvid
  );



/**
  The Entry Point of SetSsidSvidDxe. It register a ready to boot event for programming SsidSvid.

  @param ImageHandle    The firmware allocated handle for the EFI image.
  @param SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The entry point is executed successfully.

**/
EFI_STATUS
SetSsidSvidEntryPoint (
  IN     EFI_HANDLE          ImageHandle,
  IN     EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_EVENT                                  SetSsidSvidEvent;
  VOID                                       *Registration; 

  SetSsidSvidEvent = EfiCreateProtocolNotifyEvent (
                       &gEfiPciEnumerationCompleteProtocolGuid,
                       TPL_NOTIFY,
                       SetSsidSvidCallBack,
                       NULL,
                       &Registration
                       );

  ASSERT (SetSsidSvidEvent != NULL);
    
  return EFI_SUCCESS;
}

VOID
EFIAPI
SetSsidSvidCallBack (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS          Status;
  UINT8               Bus;
  UINT8               Dev;
  UINT8               Func;
  UINT32              SsidSvid;
  UINT8               Value8;
  UINT16              VendorId;
  UINT16              DeviceId;
  UINT16              ClassCode;
  UINT8               HeaderType;
  UINT8               BusLimit;
  UINT8               FuncLimit;
  VOID                *ProtocolPointer;

//[-start-151220-IB1127138-remove]//
//   SsidSvid = (UINT32)PcdGet32(PcdDefaultSsidSvid);
//[-end-151220-IB1127138-remove]//

  //
  // Check whether this is real ExitPmAuth notification, or just a SignalEvent
  //
  Status = gBS->LocateProtocol (&gEfiPciEnumerationCompleteProtocolGuid, NULL, (VOID **)&ProtocolPointer);
//[-start-151220-IB1127138-remove]//
//   if (EFI_ERROR (Status)) {
//     if (SocStepping () >= SocC0) {
//       ///
//       /// Cherryview Bug HSD 5239576
//       /// P-Unit fails PCI Compliance due to illegal straps in the Subsystem ID, Subsystem Version ID and Power Management Capabilities ID
//       /// Program SVID/SID to PUNIT_SB + 0x148
//       ///
//       SetMsgBus32WithS3Item (CHV_PUNIT, 0x148, SsidSvid, 0x07); 
//     }
// 	return ;
//   }
//[-end-151220-IB1127138-remove]//

  Status   = EFI_SUCCESS;

  BusLimit = 1;
  SsidSvid = 0xFFFFFFFF;

  for (Bus = 0; Bus <= BusLimit; Bus++) {
    for (Dev = 0; Dev <= 0x1F ; Dev++) {
      ClassCode = PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, 0, PCI_CLASSCODE_OFFSET + 1));
      if (ClassCode == 0xFFFF) {
        continue;
      }
      
      HeaderType = PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, 0, PCI_HEADER_TYPE_OFFSET));
      if (HeaderType & HEADER_TYPE_MULTI_FUNCTION) {
        FuncLimit = 7;
      } else {
        FuncLimit = 0;
      }
      for (Func = 0; Func <= FuncLimit; Func++) {
        ClassCode = PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_CLASSCODE_OFFSET + 1));
        if (ClassCode == 0xFFFF) {
          continue;
        } else if (ClassCode == ((PCI_CLASS_BRIDGE << 8) | PCI_CLASS_BRIDGE_P2P)) {
          Value8 = PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET));
          if (Value8 > BusLimit) {
            BusLimit = Value8;
          }
        }
        VendorId = PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_VENDOR_ID_OFFSET));
        DeviceId = PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_DEVICE_ID_OFFSET));


        if (SsidSvid == 0) {
          SsidSvid = ((UINT32)DeviceId << 16) + VendorId;
        }

        //
        // OemServices
        //
        Status = OemSvcUpdateSsidSvidInfo (Bus, Dev, Func, VendorId, DeviceId, ClassCode, &SsidSvid);
        if (!EFI_ERROR(Status)) {
          continue;
        }
        Status = DxeCsSvcProgramChipsetSsid (Bus, Dev, Func, VendorId, DeviceId, SsidSvid);
        if (!EFI_ERROR(Status)) {
           continue;
        } 
        ProgramSsidSvid (Bus, Dev, Func, SsidSvid);
      }
    } 
  }
  return ;
}

VOID
ProgramSsidSvid (
  IN     UINT8     Bus,
  IN     UINT8     Dev,
  IN     UINT8     Func,
  IN     UINT32    SsidSvid
  )
{
  UINT8           PciHeaderType;
  UINT8           SubsystemCapOffset;
  UINT8           PciSsidOffset;
  EFI_STATUS      Status;
  UINT64          BootScriptPciAddress;


  PciHeaderType = PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_HEADER_TYPE_OFFSET));

  if (FixedPcdGetBool (PcdNoBridgeDeviceSsid)) {
    UINT16          ClassCode;
    //
    // According to PCI spec Section "Subsystem Vendor ID and Subsystem ID"
    //   Base class 6 with sub class 0,1,2,3,4, or base class 8 with sub class 0,1,2,3
    //   are excluded from the requirement of programming these registers
    //
    ClassCode = PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_CLASSCODE_OFFSET + 1));
    if ((ClassCode >= (PCI_CLASS_BRIDGE << 8) + PCI_CLASS_BRIDGE_HOST) 
      && (ClassCode <= (PCI_CLASS_BRIDGE << 8) + PCI_CLASS_BRIDGE_P2P)) {
      return;
    }
    if ((ClassCode >= (PCI_CLASS_SYSTEM_PERIPHERAL << 8) + PCI_SUBCLASS_PIC) && 
      (ClassCode <= (PCI_CLASS_SYSTEM_PERIPHERAL << 8) + PCI_SUBCLASS_RTC)) {
      return;
    }
  }

  if (( PciHeaderType & HEADER_LAYOUT_CODE) != HEADER_TYPE_DEVICE ) {
    Status = PciFindCapId ( Bus, Dev, Func, EFI_PCI_CAPABILITY_ID_SSID, &SubsystemCapOffset );
    if ( !EFI_ERROR ( Status ) ) {
      PciSsidOffset = SubsystemCapOffset + 0x04;
      PciExpressWrite32 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PciSsidOffset), SsidSvid);

      BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Dev, Func, PciSsidOffset);
      S3BootScriptSavePciCfgWrite (
          S3BootScriptWidthUint32, 
          BootScriptPciAddress,
          1, 
          &SsidSvid);

    }
  } else {
    PciExpressWrite32 (PCI_EXPRESS_LIB_ADDRESS (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET), SsidSvid);

    BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET);
    S3BootScriptSavePciCfgWrite (
        S3BootScriptWidthUint32, 
        BootScriptPciAddress,
        1, 
        &SsidSvid);
    
  }
  return;
}

