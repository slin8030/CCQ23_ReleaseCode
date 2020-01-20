/** @file
  CommonPciPlatformDxe driver will produces the PciPlatform Protocol for Dxe driver.
  If Platform layer already install this protocol, This driver will hook platform protocol.

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
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
#include <Protocol/Bds.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciPolicy.h>
#include <Protocol/PciPlatform.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Library/DebugLib.h>
#include <Library/CommonPciLib.h>
#include <Library/PciExpressLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/DxeChipsetSvcLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

#include <Guid/AutoGenPciOptionRomList.h>

#define PCCARD_CARDBUS_LATENCY_TIMER_OFFSET 0x1B

EFI_STATUS
EFIAPI
CommonPlatformNotify (
  IN EFI_PCI_PLATFORM_PROTOCOL                        *This,
  IN EFI_HANDLE                                       HostBridge,
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE    Phase,
  IN EFI_PCI_CHIPSET_EXECUTION_PHASE                  ChipsetPhase
  );

EFI_STATUS
EFIAPI
CommonPlatformPrepController (
  IN EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN EFI_HANDLE                                     HostBridge,
  IN EFI_HANDLE                                     RootBridge,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS    PciAddress,
  IN EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE   Phase,
  IN EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  );

EFI_STATUS
EFIAPI
CommonGetPlatformPolicy (
  IN  CONST EFI_PCI_PLATFORM_PROTOCOL               *This,
  OUT       EFI_PCI_PLATFORM_POLICY                 *PciPolicy
  );

EFI_STATUS
EFIAPI
CommonGetPciRom (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL                 *This,
  IN       EFI_HANDLE                                PciHandle,
  OUT      VOID                                      **RomImage,
  OUT      UINTN                                     *RomSize
  );

EFI_STATUS
ProgramLatencyTime (
  IN     UINT8                                       Bus,
  IN     UINT8                                       Device,
  IN     UINT8                                       Func,
  IN     PCI_POLICY                                  *PciPolicy
);

EFI_STATUS
ProgramPciePayLoad(
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS PciAddress,
  IN     UINT8                                       CapIdOffset,
  IN     PCI_POLICY                                  *PciPolicy
);

EFI_STATUS
GetSmallestMaxPayLoad(
  IN     UINT8                                       Bus,
  IN OUT UINT8                                       *SmallestPayLoadValue
);

EFI_STATUS
SetSmallestPayLoadValue (
  IN     UINT8                                       Bus,
  IN     UINT8                                       SmallestPayLoadValue
);

VOID
EFIAPI
CommonPciPlatformHookBdsCallback (
  IN EFI_EVENT                                       Event,
  IN VOID                                            *Context
);

VOID
EFIAPI
CommonPciPlatformBds (
  IN EFI_BDS_ARCH_PROTOCOL                           *Bds
);

VOID
CheckOpromId (
  IN  OUT VOID                                    **RomImage,
  IN      UINTN                                   *RomSize,
  IN      UINT16                                  VendorId,
  IN      UINT16                                  DeviceId
);

EFI_PCI_PLATFORM_PROTOCOL mCommonPciPlatform = {
  CommonPlatformNotify,
  CommonPlatformPrepController,
  CommonGetPlatformPolicy,
  CommonGetPciRom
};

EFI_PCI_PLATFORM_PROTOCOL mPciPlatformHook = {
  NULL,
  NULL,
  NULL,
  NULL
};

EFI_HANDLE                mCommonPciPlatformHandle = NULL;
EFI_BDS_ENTRY             mOriginalBdsEntry = NULL;

PCI_OPTION_ROM_TABLE      *mPciOpRomTable;
UINTN                     mPciOpRomTableCnt;
UINTN                     mMaxOpRomTableCnt;

EFI_STATUS
CommonPciInstallOptionRomTable (
  IN    PCI_OPTION_ROM_TABLE           *PciOpRom
  )
{
  UINTN                                TableIndex;

  //
  // If find VID and DID is exist, return success.
  // Let first install can select option rom from OemSvc.
  // The VID and DID is only installed one set.
  //
  for (TableIndex = 0; TableIndex < mPciOpRomTableCnt; TableIndex++) {
    if ((mPciOpRomTable[mPciOpRomTableCnt].VendorId == PciOpRom->VendorId) &&
        (mPciOpRomTable[mPciOpRomTableCnt].DeviceId == PciOpRom->DeviceId)) {
      return EFI_SUCCESS;
    }
  }

  //
  // Check Database size
  //
  if (mPciOpRomTableCnt == mMaxOpRomTableCnt) {
    mPciOpRomTable = ReallocatePool (
                       sizeof (PCI_OPTION_ROM_TABLE) * mMaxOpRomTableCnt,
                       sizeof (PCI_OPTION_ROM_TABLE) * (mMaxOpRomTableCnt + 0x200),
                       mPciOpRomTable
                       );
    if (mPciOpRomTable == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    mMaxOpRomTableCnt += 0x200;
  }

  //
  // Put Item
  //
  CopyGuid (&(mPciOpRomTable[TableIndex].FileName), &(PciOpRom->FileName));
  mPciOpRomTable[TableIndex].VendorId = PciOpRom->VendorId;
  mPciOpRomTable[TableIndex].DeviceId = PciOpRom->DeviceId;
  mPciOpRomTableCnt++;
  return EFI_SUCCESS;
}

/**
  Install Option rom to table from OemSvc

  @retval EFI_SUCCESS                  Success
  @retval other                        Get OemService table failed

**/
EFI_STATUS
CommonPciInstallOptionRomFromOemService (
  )
{
  EFI_STATUS                           Status;
  PCI_OPTION_ROM_TABLE                 *PciOptionRomTable;
  UINTN                                TableIndex;

  PciOptionRomTable = NULL;
  Status = OemSvcInstallOptionRomTable (
             PCI_OPROM,
             (VOID **)&PciOptionRomTable
             );
  if (Status != EFI_MEDIA_CHANGED) {
    return Status;
  }
  if (PciOptionRomTable == NULL) {
    return EFI_NOT_READY;
  }
  for (TableIndex = 0; PciOptionRomTable[TableIndex].VendorId != 0xFFFF; TableIndex++) {
    CommonPciInstallOptionRomTable (PciOptionRomTable + TableIndex);
  }
  return EFI_SUCCESS;
}

/**
  Install Option rom to table from AutoGenList

  @retval EFI_SUCCESS                  Success
  @retval other                        Section not found
  @retval other                        Memory allocation failed
  @retval other                        Content invalid

**/
EFI_STATUS
CommonPciInstallOptionRomFromAutoGenList (
  )
{
  EFI_STATUS                                Status;
  UINTN                                     SectionSize;
  UINTN                                     Offset;
  PCI_OPTION_ROM_TABLE                      PciOptionRomTable;
  AUTO_GEN_PCI_OPTION_ROM_TABLE             *AutoGenOpRomList;
  AUTO_GEN_PCI_OPTION_ROM_TABLE             *Ptr;
  UINTN                                     Index;

  Status = GetSectionFromAnyFv (
             &gAutoGenPciOptionRomListGuid,
             EFI_SECTION_RAW,
             0,
             &AutoGenOpRomList,
             &SectionSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Check data vaildation
  //
  Offset = 0;
  Ptr = AutoGenOpRomList;
  while (Offset < SectionSize) {
    Offset += sizeof (AUTO_GEN_PCI_OPTION_ROM_TABLE) + sizeof (UINT16) * (Ptr->DeviceIdCount - 1);
    if ((Ptr->DeviceIdCount == 0) || (Offset > SectionSize)) {
      Status = EFI_INVALID_PARAMETER;
      goto DONE;
    }
    Ptr = (AUTO_GEN_PCI_OPTION_ROM_TABLE *)(((UINTN)AutoGenOpRomList) + Offset);
  }

  //
  // Put Item
  //
  Offset = 0;
  Ptr = AutoGenOpRomList;
  while (Offset < SectionSize) {
    Offset += sizeof (AUTO_GEN_PCI_OPTION_ROM_TABLE) + sizeof (UINT16) * (Ptr->DeviceIdCount - 1);
    CopyGuid (&(PciOptionRomTable.FileName), &(Ptr->FileName));
    PciOptionRomTable.VendorId = Ptr->VendorId;
    for (Index = 0; Index < Ptr->DeviceIdCount; Index++) {
      PciOptionRomTable.DeviceId = Ptr->DeviceId[Index];
      CommonPciInstallOptionRomTable (&PciOptionRomTable);
    }
    Ptr = (AUTO_GEN_PCI_OPTION_ROM_TABLE *)(((UINTN)AutoGenOpRomList) + Offset);
  }

DONE:
  if (AutoGenOpRomList != NULL) {
    gBS->FreePool (AutoGenOpRomList);
  }
  return Status;
}

/**

 @param [in]   ImageHandle
 @param [in]   SystemTable

 @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
CommonPciPlatformDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_EVENT                                 PciPlatformBdsInstalledEvent;
  VOID                                      *Registration;
  EFI_BDS_ARCH_PROTOCOL                     *Bds;

  Status = gBS->LocateProtocol (
                  &gEfiBdsArchProtocolGuid,
                  NULL,
                  (VOID **)&Bds
                  );
  if (EFI_ERROR (Status)) {
    //
    // gEfiBdsArchProtocolGuid is not installed, register a protocolnotify event.
    // Once gEfiBdsArchProtocolGuid is installed, callback to CommonPciPlatformBdsCallback to Hook Bds->Entry
    //
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    CommonPciPlatformHookBdsCallback,
                    NULL,
                    &PciPlatformBdsInstalledEvent
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gEfiBdsArchProtocolGuid,
                      PciPlatformBdsInstalledEvent,
                      &Registration
                      );
    }
    ASSERT_EFI_ERROR (Status);

  } else {
    //
    // gEfiBdsArchProtocolGuid is installed, Hook Bds->Entry to CommonPciPlatformBds
    //
    mOriginalBdsEntry = Bds->Entry;
    Bds->Entry = (EFI_BDS_ENTRY) CommonPciPlatformBds;
  }

  //
  // Initial PCI Option Rom database.
  // Integrate AutoGen Oprom List with OpRomTable from OemService.
  //
  mPciOpRomTable    = NULL;
  mPciOpRomTableCnt = 0;
  mMaxOpRomTableCnt = 0;
  CommonPciInstallOptionRomFromOemService ();
  CommonPciInstallOptionRomFromAutoGenList ();

  return Status;
}

/**
  This is the callback function after gEfiBdsArchProtocolGuid is installed.
  Hook Bds->Entry to CommonPciPlatformBds.

  @param    EFI_EVENT   Event,
  @param    VOID        *Context

  @retval   None.

**/
VOID
EFIAPI
CommonPciPlatformHookBdsCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  EFI_BDS_ARCH_PROTOCOL     *Bds;
  EFI_STATUS                Status;
  Status = gBS->LocateProtocol (
                  &gEfiBdsArchProtocolGuid,
                  NULL,
                  (VOID **)&Bds
                  );
  //
  // gEfiBdsArchProtocolGuid should be installed at this moment.
  //
  ASSERT_EFI_ERROR (Status);

  mOriginalBdsEntry = Bds->Entry;
  Bds->Entry = (EFI_BDS_ENTRY) CommonPciPlatformBds;
}

/**
  This is the Hook function of Bds->Entry. After dxemain called gBds->Entry, code should come here.
  The purpose of this function is to hook PciPlatform.
  if gEfiPciPlatformProtocolGuid is installed by PciPlatform driver, Hook the original protocols.
  else, install gEfiPciPlatformProtocolGuid protocol.

  @param      Bds          - Bds architechtural protocol passed from Dxemain

  @retval   None.

**/
VOID
EFIAPI
CommonPciPlatformBds (
  IN EFI_BDS_ARCH_PROTOCOL  *Bds
)
{
  EFI_STATUS                                Status;
  EFI_PCI_PLATFORM_PROTOCOL                 *PciPlatform;

  Status = gBS->LocateProtocol (
                  &gEfiPciPlatformProtocolGuid,
                  NULL,
                  (VOID **)&PciPlatform
                  );
  if (!EFI_ERROR(Status)) {
    mPciPlatformHook.PlatformNotify         = PciPlatform->PlatformNotify;
    mPciPlatformHook.PlatformPrepController = PciPlatform->PlatformPrepController;
    mPciPlatformHook.GetPlatformPolicy      = PciPlatform->GetPlatformPolicy;
    mPciPlatformHook.GetPciRom              = PciPlatform->GetPciRom;
    //
    // Hook original PciPlatform protocols
    //
    PciPlatform->PlatformNotify         = CommonPlatformNotify;
    PciPlatform->PlatformPrepController = CommonPlatformPrepController;
    PciPlatform->GetPlatformPolicy      = CommonGetPlatformPolicy;
    PciPlatform->GetPciRom              = CommonGetPciRom;
  } else {
    //
    // gEfiPciPlatformProtocol is not installed. Install it on a new handle
    //
    Status = gBS->InstallProtocolInterface (
                    &mCommonPciPlatformHandle,
                    &gEfiPciPlatformProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &mCommonPciPlatform
                    );
  }
  //
  // call to the original bdsentry
  //
  Bds->Entry = mOriginalBdsEntry;
  Bds->Entry (Bds);
}

EFI_STATUS
EFIAPI
CommonPlatformNotify (
  IN  EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE  Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ExecPhase
  )
{
  if (mPciPlatformHook.PlatformNotify != NULL) {
        mPciPlatformHook.PlatformNotify (
                                This,
                                HostBridge,
                                Phase,
                                ExecPhase
                                );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CommonPlatformPrepController (
  IN  EFI_PCI_PLATFORM_PROTOCOL                      *This,
  IN  EFI_HANDLE                                     HostBridge,
  IN  EFI_HANDLE                                     RootBridge,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS    PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE   Phase,
  IN  EFI_PCI_CHIPSET_EXECUTION_PHASE                ChipsetPhase
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;
  PCI_POLICY  *PciPolicy;
  UINT8       CapIdOffset;

  if ((Phase == EfiPciBeforeResourceCollection) && (ChipsetPhase == ChipsetEntry)) {

    Status = gBS->LocateProtocol (
                            &gPciPolicyProtocolGuid,
                            NULL,
                            (VOID **)&PciPolicy
                            );

    if (!EFI_ERROR(Status)) {
      Status = PciFindCapId (PciAddress.Bus, PciAddress.Device, PciAddress.Function, EFI_PCI_CAPABILITY_ID_PCIEXP, &CapIdOffset);

      if (!EFI_ERROR (Status)) {
        //
        // Only PCIE need to be prgorammed Max_Payload_Size register.
        //
        if (PciAddress.Bus == 0) {
          //
          // Program this root port and all device behind this root port with the same smallest Max_Payload_Size register.
          //
          ProgramPciePayLoad (PciAddress, CapIdOffset, PciPolicy);
        }
      }

      if (EFI_ERROR (Status)) {
        //
        // Only PCI need to be prgorammed Latency Time register.
        //
        ProgramLatencyTime (PciAddress.Bus, PciAddress.Device, PciAddress.Function, PciPolicy);
      }
    }

  }

  if (mPciPlatformHook.PlatformPrepController != NULL) {
          mPciPlatformHook.PlatformPrepController (
                                This,
                                HostBridge,
                                RootBridge,
                                PciAddress,
                                Phase,
                                ChipsetPhase
                                );
  }

  return EFI_SUCCESS;
}

/**
  GC_TODO: Add function description

  @param     This        add argument description
  @param     PciPolicy   add argument description.

  @retval    EFI_INVALID_PARAMETER  It mean there is no any platform policy.
  @retval    EFI_SUCCESS            Get the platform policy successfully.

**/
EFI_STATUS
EFIAPI
CommonGetPlatformPolicy (
  IN CONST EFI_PCI_PLATFORM_PROTOCOL               *This,
  OUT EFI_PCI_PLATFORM_POLICY                      *PciPolicy
  )
{
  if (PciPolicy == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Always reserved VGA IO range.
  //
  *PciPolicy |= EFI_RESERVE_VGA_IO_ALIAS;

  if (mPciPlatformHook.GetPlatformPolicy != NULL) {
    mPciPlatformHook.GetPlatformPolicy (
                       This,
                       PciPolicy
                       );
  }

  return EFI_SUCCESS;
}


/**
  Return a PCI ROM image for the onboard device represented by PciHandle

  @param    This       Protocol instance pointer.
  @param    PciHandle  PCI device to return the ROM image for.
  @param    RomImage   PCI Rom Image for onboard device
  @param    RomSize    Size of RomImage in bytes

  @retval   EFI_NOT_FOUND  No RomImage.
  @retval   EFI_SUCCESS    RomImage is valid.

**/
EFI_STATUS
EFIAPI
CommonGetPciRom (
  IN  CONST EFI_PCI_PLATFORM_PROTOCOL             *This,
  IN  EFI_HANDLE                                  PciHandle,
  OUT VOID                                        **RomImage,
  OUT UINTN                                       *RomSize
  )
{
  EFI_STATUS            Status;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  UINTN                 Segment;
  UINTN                 Bus;
  UINTN                 Device;
  UINTN                 Function;
  UINT16                VendorId;
  UINT16                DeviceId;
  UINTN                 TableIndex;
  BOOLEAN               SkipGetPciRom;
  EFI_STATUS            PciPlatformHookStatus = EFI_SUCCESS;
  UINT64                Result;

  *RomImage = NULL;
  *RomSize = 0;

  Status = gBS->HandleProtocol (
                  PciHandle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 0, 1, &VendorId);

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint16, 2, 1, &DeviceId);

  PciIo->Attributes(
         PciIo,
         EfiPciIoAttributeOperationGet,
         0,
         &Result
         );
  

  SkipGetPciRom = FALSE;
  DxeCsSvcSkipGetPciRom (Segment, Bus, Device, Function, VendorId, DeviceId, &SkipGetPciRom);
  if (SkipGetPciRom) {
    return EFI_NOT_FOUND;
  }
  //
  // OemServices
  //
  Status = OemSvcSkipLoadPciOptionRom (
                          Segment,
                          Bus,
                          Device,
                          Function,
                          VendorId,
                          DeviceId,
                          &SkipGetPciRom
                          );
  if ((Status == EFI_SUCCESS) || (Status == EFI_MEDIA_CHANGED)) {
    Status = EFI_NOT_FOUND;

      //
      // Per UEFI spec 2.6
      // If EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM bit is set, then the PCI option ROM described by the RomImage and RomSize
      // fields is not from ROM BAR of the PCI controller. If this bit is clear, then the
      // RomImage and RomSize fields were initialized based on the PCI option ROM
      // found through the ROM BAR of the PCI controller. Below condition is to skip ROM that found from ROMBAR.    
      //

//[-start-180821-IB16530003-modify]//
    //
    // fix the bug of conditional operator which cause pci controller can't find 
    //
	//if (SkipGetPciRom && (!(Result && EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM))) {
    if (SkipGetPciRom && (!(Result & EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM))) {
//[-end-180821-IB16530003-modify]//
      PciIo->RomSize = 0;
      PciIo->RomImage = NULL;
    }
  } else {
    Status = EFI_NOT_FOUND;
    for (TableIndex = 0; TableIndex < mPciOpRomTableCnt; TableIndex++) {
      //
      // See if the PCI device specified by PciHandle matches at device in PciOptionRomTable
      //
      if (VendorId != mPciOpRomTable[TableIndex].VendorId ||
        DeviceId != mPciOpRomTable[TableIndex].DeviceId) {
        continue;
      }

      Status = GetSectionFromAnyFv (
                &mPciOpRomTable[TableIndex].FileName,
                EFI_SECTION_RAW,
                0,
                RomImage,
                RomSize);

      if (EFI_ERROR (Status)) {
        continue;
      }
      CheckOpromId (RomImage, RomSize, VendorId, DeviceId);
      Status = EFI_SUCCESS;
      break;
    }
  }
  
  if (mPciPlatformHook.GetPciRom != NULL) {
      PciPlatformHookStatus = mPciPlatformHook.GetPciRom (
                                  This,
                                  PciHandle,
                                  RomImage,
                                  RomSize
                                  );
  }

  if (EFI_ERROR (PciPlatformHookStatus)) {
    return PciPlatformHookStatus;
  }
  return Status;
}

/**
  Check Vendor ID and Device ID in Option rom, if not equal, make them the same.

  @param    RomImage - Oprom Image Offset
  @param    VendorId - Pci Vendor Id
  @param    DeviceId - Pci Device Id

  @retval

**/
VOID
CheckOpromId (
  IN  OUT VOID                                    **RomImage,
  IN      UINTN                                   *RomSize,
  IN      UINT16                                  VendorId,
  IN      UINT16                                  DeviceId
)
{
  EFI_PCI_ROM_HEADER  RomHeader;
  PCI_DATA_STRUCTURE  *Pcir;
  BOOLEAN             Done;

  if (*RomSize < sizeof (EFI_PCI_ROM_HEADER)) {
    return ;
  }

  RomHeader.Raw = *RomImage;
  Done          = FALSE;
  while (!Done) {
    if (RomHeader.Generic->Signature == PCI_EXPANSION_ROM_HEADER_SIGNATURE) {

      if (*RomSize < RomHeader.Generic->PcirOffset + sizeof (PCI_DATA_STRUCTURE)) {
        return ;
      }

      Pcir = (PCI_DATA_STRUCTURE *) (RomHeader.Raw + RomHeader.Generic->PcirOffset);

      if (Pcir->CodeType == PCI_CODE_TYPE_PCAT_IMAGE) {
        //
        // We have found a PC-AT ROM
        //
        Pcir->VendorId = VendorId;
        Pcir->DeviceId = DeviceId;
        return ;

      }

      if ((Pcir->Indicator & 0x80) == 0x80) {
        Done = TRUE;
      } else {
        RomHeader.Raw += 512 * Pcir->ImageLength;
      }
    } else {
      Done = TRUE;
    }
  }
}

/**
  Program Pci latency time register as SCU value.

  @param    BUS           - Pointer to the  EFI_PCI_PLATFORM_PROTOCOL  instance.
  @param    Device        - The associated PCI host bridge handle.
  @param    Func          - The associated PCI root bridge handle.

  @retval   EFI_UNSUPPORTED
  @retval   EFI_SUCCESS
**/
EFI_STATUS
ProgramLatencyTime (
  IN  UINT8       Bus,
  IN  UINT8       Device,
  IN  UINT8       Func,
  IN  PCI_POLICY  *PciPolicy
)
{
  UINT8       LatTime;
  UINT64      BootScriptPciAddress;
  UINT8       HeaderType;

  if(PciPolicy->LatencyTimePolicy.Enable == FALSE) {
    return EFI_UNSUPPORTED;
  }

  //
  // Program SCU value into Latency Time register.
  //
  LatTime = PciPolicy->LatencyTimePolicy.LatencyTime;

  PciExpressAndThenOr8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCI_LATENCY_TIMER_OFFSET), 0x00, LatTime);

  BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Device, Func, PCI_LATENCY_TIMER_OFFSET);
  S3BootScriptSavePciCfgWrite (
      S3BootScriptWidthUint8,
      BootScriptPciAddress,
      1,
      &LatTime);

  //
  // Program Sec. Latency Time register of the pci Bridge.
  //
  HeaderType = PciExpressAnd8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCI_HEADER_TYPE_OFFSET), HEADER_LAYOUT_CODE);

  if (HeaderType == HEADER_TYPE_PCI_TO_PCI_BRIDGE) {

    PciExpressAndThenOr8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCCARD_CARDBUS_LATENCY_TIMER_OFFSET), 0x00, LatTime);

    BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Device, Func, PCCARD_CARDBUS_LATENCY_TIMER_OFFSET);
    S3BootScriptSavePciCfgWrite (
        S3BootScriptWidthUint8,
        BootScriptPciAddress,
        1,
        &LatTime);

  }

  return EFI_SUCCESS;
}

/**
  Program PCIE Root port bridge and all endpoints with the same smallest Max_PayLoad value.

  @param    PciAddress     The address of the PCI device on the PCI bus.
  @param    CapIdOffset    This CapId indicate this device is a PCIE device.

  @retval   EFI_SUCCESS

**/
EFI_STATUS
ProgramPciePayLoad(
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS    PciAddress,
  IN     UINT8                                          CapIdOffset,
  IN     PCI_POLICY                                     *PciPolicy
)
{
  UINT8       SmallestMaxPayLoad;
  UINT8       SecondBus;
  UINT8       HeaderType;

  if(PciPolicy->ProgramPciePayLoadPolicy.Enable == FALSE) {
    return EFI_UNSUPPORTED;
  }

  HeaderType = (PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, PCI_HEADER_TYPE_OFFSET)) & (HEADER_LAYOUT_CODE));

  if (HeaderType == HEADER_TYPE_PCI_TO_PCI_BRIDGE) {

    SecondBus = PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));

    SmallestMaxPayLoad = (PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, CapIdOffset + 0x04)) & (BIT2 | BIT1 | BIT0)) << 5;

    //
    //  Scan all device behind this root port to get the Smallest Max_Pay_Load.
    //
    GetSmallestMaxPayLoad (SecondBus, &SmallestMaxPayLoad);

    //
    //  Program all device behind this root port with the same Smallest Max_Pay_Load.
    //
    SetSmallestPayLoadValue (SecondBus, SmallestMaxPayLoad);

    //
    //  Program this root port with smallest Max_Pay_Load.
    //
    PciExpressAndThenOr8 (PCI_EXPRESS_LIB_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, CapIdOffset + 0x08), 0x1F, SmallestMaxPayLoad) ;

  }
  return EFI_SUCCESS;
}

/**
  Scan all endpoints behind this root port bridge and find the smallest Max_PayLoad Value.

  @param    Bus                    Bus number which will be recursived to scan all the bus behine this bridge.
  @param    SmallestPayLoadValue   This value point to the smallest payload value.

  @retval   EFI_SUCCESS

**/
EFI_STATUS
GetSmallestMaxPayLoad(
  IN     UINT8   Bus,
  IN OUT UINT8   *SmallestPayLoadValue
)
{
  EFI_STATUS      Status;
  UINT8           SecondBus;
  UINT8           Device;
  UINT8           Func;
  UINT8           HeaderType;
  UINT8           EndPointCapIdOffset;
  UINT8           TempPayLoad;

  for (Device = 0; Device <= PCI_MAX_DEVICE; Device++){
    for (Func = 0; Func <= PCI_MAX_FUNC; Func++){

      //
      // Check whether a pci device is present or not.
      //
      if ( PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, 0)) == 0xffff) {
        if (Func == 0) {
          Func = PCI_MAX_FUNC;
        }
        continue;
      }

      //
      // Check whether a PCIE or not.
      //
      EndPointCapIdOffset = 0;
      Status = PciFindCapId (Bus, Device, Func, EFI_PCI_CAPABILITY_ID_PCIEXP, &EndPointCapIdOffset);

      if (EFI_ERROR (Status)){
        continue;
      }

      TempPayLoad = (PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, EndPointCapIdOffset + 0x04)) & (BIT2 | BIT1 | BIT0)) << 5;

      if (*SmallestPayLoadValue > TempPayLoad ) {
        *SmallestPayLoadValue = TempPayLoad;
      }

      HeaderType = (PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCI_HEADER_TYPE_OFFSET)) & HEADER_LAYOUT_CODE);

      if (HeaderType == HEADER_TYPE_PCI_TO_PCI_BRIDGE) {

        SecondBus = PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));

        GetSmallestMaxPayLoad (SecondBus, SmallestPayLoadValue);

      }
    }
  }
  return EFI_SUCCESS;
}

/**
  Program all endpoints behind this root port bridge with the same smallest Max_PayLoad Value.

  @param    Bus                    Bus number which will be recursived to scan all the bus behine this bridge.
  @param    SmallestPayLoadValue   This value indicate smallest payload value.

  @retval   EFI_SUCCESS
**/
EFI_STATUS
SetSmallestPayLoadValue (
  IN     UINT8   Bus,
  IN     UINT8   SmallestPayLoadValue
)
{
  EFI_STATUS      Status;
  UINT8           Device;
  UINT8           Func;
  UINT8           HeaderType;
  UINT8           SecondBus;
  UINT8           EndPointCapIdOffset;

  //
  // Program minimal payload value into all PCIE devices below this bridge.
  //
  for (Device = 0; Device <= PCI_MAX_DEVICE; Device++){
    for (Func = 0; Func <= PCI_MAX_FUNC; Func++){
      //
      // Check whether a pci device is present or not.
      //
      if ( PciExpressRead16 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, 0)) == 0xffff) {
        if (Func == 0) {
          Func = PCI_MAX_FUNC;
        }
        continue;
      }

      //
      // Check whether a PCIE or not.
      //
      EndPointCapIdOffset = 0;
      Status = PciFindCapId (Bus, Device, Func, EFI_PCI_CAPABILITY_ID_PCIEXP, &EndPointCapIdOffset);

      if (EFI_ERROR (Status)){
        continue;
      }

      PciExpressAndThenOr8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, EndPointCapIdOffset + 0x08), 0x1F, SmallestPayLoadValue);

      HeaderType = (PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCI_HEADER_TYPE_OFFSET)) & HEADER_LAYOUT_CODE);

      if (HeaderType == HEADER_TYPE_PCI_TO_PCI_BRIDGE) {

        SecondBus = PciExpressRead8 (PCI_EXPRESS_LIB_ADDRESS (Bus, Device, Func, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET));

        SetSmallestPayLoadValue (SecondBus, SmallestPayLoadValue);

      }

    }
  }

  return EFI_SUCCESS;
}

