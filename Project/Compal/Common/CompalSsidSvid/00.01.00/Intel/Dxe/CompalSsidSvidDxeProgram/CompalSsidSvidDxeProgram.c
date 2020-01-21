
#include "CompalSsidSvidDxeProgram.h"

COMPAL_SSID_SVID_DXE_PROTOCOL    *CompalSsidSvidDxeProtocol;


EFI_STATUS
CompalSsidSvidDxeWrite (
    IN  UINT32                           SsidSvid,
    IN  COMPAL_SSID_SVID_SPECIAL_TABLE   *CompalSsidSvidSpecialTable
)
/*++
Routine Description:
  PCI SCAN to find and program SSID/SVID for all PCI Device

Arguments:
  SsidSvid                   SSID/SVID you want to program
  CompalSsidSvidSpecialTable     Table defined how to program SSID/SVID for Specifical Device

Returns:
  EFI_SUCCESS        Program SSID/SVID for all PCI Device successfully

--*/
{
    EFI_STATUS       Status = EFI_SUCCESS;
    UINT8            Bus;
    UINT8            Dev;
    UINT8            Func;
    UINT8            Value8;
    UINT16           VendorId;
    UINT16           DeviceId;
    UINT16           ClassCode;
    UINT8            HeaderType;
    UINT8            BusLimit = 1;
    UINT8            FuncLimit;

    // ----------------  Bus Loop  ----------------
    for (Bus = 0; Bus <= BusLimit; Bus++) {
        // ----------------  Device Loop  ----------------
        for (Dev = 0; Dev <= PCI_MAX_DEVICE; Dev++) {
            // Check if device exist to reduce loop
            VendorId =  PciRead16(PCI_LIB_ADDRESS (Bus,Dev,0,PCI_VENDOR_ID_OFFSET));
            if (VendorId == NULL_ID) {
                continue;
            }

            // Check if header type is multi-function to set FuncLimt
            HeaderType =  PciRead8(PCI_LIB_ADDRESS (Bus,Dev,0,PCI_HEADER_TYPE_OFFSET));
            if (HeaderType & HEADER_TYPE_MULTI_FUNCTION) {
                FuncLimit = PCI_MAX_FUNC;
            } else {
                FuncLimit = 0;
            }

            // ----------------  Function Loop  ----------------
            for (Func = 0; Func <= FuncLimit; Func++) {
                // Check if device exist to reduce loop
                VendorId =  PciRead16(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_VENDOR_ID_OFFSET));
                DeviceId =  PciRead16(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_DEVICE_ID_OFFSET));
                if (VendorId == NULL_ID) {
                    continue;
                }

                // Check if class code is P2P Bridge to set BusLimit
                ClassCode =  PciRead16(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_CLASSCODE_OFFSET+1));
                if (ClassCode == ((PCI_CLASS_BRIDGE << 8) | PCI_CLASS_BRIDGE_P2P)) {
                    Value8 =  PciRead8(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET));
                    if (Value8 > BusLimit) {
                        BusLimit = Value8;
                    }
                }

                //
                // 1. Program Specific SSID/SVID
                //
                Status = ProgramSpecificSsidSvid (Bus, Dev, Func, VendorId, DeviceId, SsidSvid, CompalSsidSvidSpecialTable);
                if (!EFI_ERROR(Status)) {
                    continue;
                }

                //
                // 2. Program other SSID/SVID follow Spec
                //
                ProgramSsidSvid (Bus, Dev, Func, VendorId, DeviceId, SsidSvid);
            }
        }
    }

    return EFI_SUCCESS;
}


EFI_STATUS
ProgramSpecificSsidSvid (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT16                           VendorId,
    IN  UINT16                           DeviceId,
    IN  UINT32                           SsidSvid,
    IN  COMPAL_SSID_SVID_SPECIAL_TABLE     *CompalSsidSvidSpecialTable
)
/*++
Routine Description:
  Program SSID/SVID for PCI device follow COMPAL_SSID_SVID_SPECIAL_TABLE defined

Arguments:
  Bus                        PCI Bus Number
  Dev                        PCI Device Number
  Func                       PCI Function Number
  VendorId                   Vendor ID of desired PCI device
  DeviceId                   Device ID of desired PCI device
  SsidSvid                   SSID/SVID you want to program
  CompalSsidSvidSpecialTable     Table defined how to program SSID/SVID for Specifical Device

Returns:
  EFI_SUCCESS        Program SSID/SVID for Specifical Device successfully

--*/
{
    UINT8                                Index;

    for (Index = 0; CompalSsidSvidSpecialTable[Index].VendorId != END_OF_TABLE; Index++ ) {
        // Check if VendorId and DeviceId match
        if (CompalSsidSvidSpecialTable[Index].VendorId == VendorId) {
            if ((CompalSsidSvidSpecialTable[Index].DeviceId == DeviceId) ||
                    (CompalSsidSvidSpecialTable[Index].DeviceId == DEVICE_ID_DONT_CARE)) {

                if (CompalSsidSvidSpecialTable[Index].SsidSvid == DONT_WRITE_SSID) {
                    // Do not write SSID, Skip it
                    return EFI_SUCCESS;
                } else if (CompalSsidSvidSpecialTable[Index].SsidSvid != PLATFORM_SSID) {
                    // Set SSID/SVID defined in COMPAL_SSID_SVID_SPECIAL_TABLE
                    SsidSvid = CompalSsidSvidSpecialTable[Index].SsidSvid;
                }

                // Program SSID/SVID by CompalSsidSvidSpecialFunction
                if (CompalSsidSvidSpecialTable[Index].CompalSsidSvidSpecialFunction != NULL) {
                    CompalSsidSvidSpecialTable[Index].CompalSsidSvidSpecialFunction (Bus, Dev, Func, SsidSvid);
                } else {
                    // Program SSID/SVID by Offset
                    CompalSsidSvidDxePciWrite (Bus, Dev, Func, CompalSsidSvidSpecialTable[Index].Offset, WIDTH32, &SsidSvid);
                }

                return EFI_SUCCESS;
            }
        }
    }

    return EFI_UNSUPPORTED;
}

VOID
ProgramSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT16       VendorId,
    IN  UINT16       DeviceId,
    IN  UINT32       SsidSvid
)
/*++
Routine Description:
  Program SSID/SVID automatically for PCI device by check VenderID, ClassCode, etc.

Arguments:
  Bus                PCI Bus Number
  Dev                PCI Device Number
  Func               PCI Function Number
  VendorId           Vendor ID of desired PCI device
  DeviceId           Device ID of desired PCI device
  SsidSvid           SSID/SVID you want to program

Returns:
  EFI_SUCCESS        Program SSID/SVID successfully

--*/
{
    EFI_STATUS       Status;
    UINT8            PciHeaderType;
    UINT16           ClassCode;
    UINT8            ProgramInterface;
    UINT8            SubsystemCapOffset;
    UINT8            PciSsidOffset;
    UINT16           PegBusValue;

    PciHeaderType = PciRead8(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_HEADER_TYPE_OFFSET));
    ClassCode = PciRead16(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_CLASSCODE_OFFSET+1));
    ProgramInterface = PciRead8(PCI_LIB_ADDRESS (Bus,Dev,Func,PCI_CLASSCODE_OFFSET));
    PegBusValue = PEG_BUS;

#ifdef NO_BRIDGE_DEVICE_SSID
    //
    // According to PCI spec Section "Subsystem Vendor ID and Subsystem ID"
    //   Base class 6 with sub class 0,1,2,3,4, or base class 8 with sub class 0,1,2,3
    //   are excluded from the requirement of programming these registers
    //
    if ((ClassCode >= (PCI_CLASS_BRIDGE << 8) + PCI_CLASS_BRIDGE_HOST)
            && (ClassCode <= (PCI_CLASS_BRIDGE << 8) + PCI_CLASS_BRIDGE_P2P)) {
        return;
    }
    if ((ClassCode >= (PCI_CLASS_SYSTEM_PERIPHERAL << 8) + PCI_SUBCLASS_PIC) &&
            (ClassCode <= (PCI_CLASS_SYSTEM_PERIPHERAL << 8) + PCI_SUBCLASS_RTC)) {
        return;
    }
#endif

    if ((VendorId == ATI_VID || VendorId == AMD_VID) &&
            Bus == 0x00 && Dev == 0x00 && Func == 0x00) {
        // Program AMD/ATI NB SSID/SVID, offset 0x50
        CompalSsidSvidDxePciWrite (Bus, Dev, Func, AMD_NB_SSID_OFFSET, WIDTH32, &SsidSvid);
    } else if (Bus == 0x00 && Dev == 0x01 && Func == 0x00 &&
               ClassCode == P2P_BRIDGE_CLASSCODE) {
        // Program VGA Controller(0x0604) SSID/SVID and External VGA Bus number(0x0101)

        // Set External VGA Bus number = 0x01
        CompalSsidSvidDxePciWrite (0x00, 0x01, 0x00, SECONDARY_BUS_OFFSET, WIDTH16, &PegBusValue);

        // For AMD/ATI, offset 0x4C
        if (VendorId == ATI_VID || VendorId == AMD_VID) {
            CompalSsidSvidDxePciWrite (Bus, Dev, Func, ATI_VGA_SSID_OFFSET, WIDTH32, &SsidSvid);
        } else if (VendorId == INTEL_VID) {
            // For Intel, offset 0x8C
            CompalSsidSvidDxePciWrite (Bus, Dev, Func, INTEL_VGA_CONTROLLER_OFFSET, WIDTH32, &SsidSvid);
        }
    } else if (Bus == 0x00 && ((ClassCode & BASE_CLASS_MASK) == VGA_CLASSCODE || ClassCode == AUDIO_CLASSCODE)) {
        // Program Internal VGA(0x0300)/Audio(0x0403) SSID/SVID

        if (VendorId == ATI_VID || VendorId == AMD_VID) {
            if (Dev == 0x14 && Func == 0x02) {
                // For ATI HD Audio Controller, offset 0x2C
                CompalSsidSvidDxePciWrite (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, WIDTH32, &SsidSvid);
            } else {
                // For else, offset 0x4C
                CompalSsidSvidDxePciWrite (Bus, Dev, Func, ATI_VGA_SSID_OFFSET, WIDTH32, &SsidSvid);
            }
        } else if (VendorId == INTEL_VID) {
            // For Intel, offset 0x2C
            CompalSsidSvidDxePciWrite (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, WIDTH32, &SsidSvid);
        }
    } else if (Bus > 0x00 && ((ClassCode & BASE_CLASS_MASK) == VGA_CLASSCODE || ClassCode == AUDIO_CLASSCODE)) {
        // Program External VGA(0x0300)/Audio(0x0403) SSID/SVID

        if (VendorId == ATI_VID || VendorId == AMD_VID) {
            // For ATI, offset 0x4C
            CompalSsidSvidDxePciWrite (Bus, Dev, Func, ATI_VGA_SSID_OFFSET, WIDTH32, &SsidSvid);
        } else if (VendorId == NV_VID) {
            // For Nvdia, offset 0x40
            CompalSsidSvidDxePciWrite (Bus, Dev, Func, NV_VGA_SSID_OFFSET, WIDTH32, &SsidSvid);
        }
    } else if (VendorId == INTEL_VID && ClassCode == INTEL_EHCI_CLASSCODE && ProgramInterface == INTEL_EHCI_INTERFACE) {
        // Program INTEL EHCI SSID/SVID, offset 0x2C and must enable Access Control
        ProgramEhciSsidSvid(Bus, Dev, Func, SsidSvid);
    } else if (VendorId == INTEL_VID && ClassCode == INTEL_XHCI_CLASSCODE && ProgramInterface == INTEL_XHCI_INTERFACE) {
        // Program INTEL XHCI SSID/SVID, offset 0x2C and follow PCH BIOS Spec Rev 0.7.0 Section 13.2.4 to Lock xHCI Register settings
        ProgramXhciSsidSvid(Bus, Dev, Func, SsidSvid);
    } else if (( PciHeaderType & HEADER_LAYOUT_CODE) != HEADER_TYPE_DEVICE ) {
        // Program Non-Standard PCI SSID/SVID, Check Header Type

        if ((PciHeaderType & HEADER_LAYOUT_CODE) == HEADER_TYPE_PCI_TO_PCI_BRIDGE) {
            // Header Type = 1: PCI-to-PCI Bridge, offset CapID + 0x04

            // Try to find SSID/SVID Capability ID: 0x0D
            Status = PciFindCapId ( Bus, Dev, Func, EFI_PCI_CAPABILITY_ID_SSID, &SubsystemCapOffset );
            if ( !EFI_ERROR ( Status ) ) {
                PciSsidOffset = SubsystemCapOffset + 0x04;
                CompalSsidSvidDxePciWrite (Bus, Dev, Func, PciSsidOffset, WIDTH32, &SsidSvid);
            }
        } else if ((PciHeaderType & HEADER_LAYOUT_CODE) == HEADER_TYPE_CARDBUS_BRIDGE) {
            // Header Type = 2: Cardbus, offset 0x40
            CompalSsidSvidDxePciWrite (Bus, Dev, Func, CARDBUS_SSID_OFFSET, WIDTH32, &SsidSvid);
        }
    } else {
        // Default SSID/SVID, offset 0x2C
        CompalSsidSvidDxePciWrite (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, WIDTH32, &SsidSvid);
    }

    // Program Arrandale(0x0044) & Clarksfield(0xD132) Generic Non-core(Bus: 0x7F) SSID/SVID
    if (VendorId == INTEL_VID && (DeviceId == ARRANDALE_DID || DeviceId == CLARKSFIELD_DID) &&
            Bus == 0x00 && Dev == 0x00 && Func == 0x00) {
        CompalSsidSvidDxePciWrite (GENERIC_BUS, 0x00, 0x00, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, WIDTH32, &SsidSvid);
    }

    return;
}

EFI_STATUS
ProgramEhciSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
)
/*++
Routine Description:
  Program INTEL EHCI SSID/SVID, it will enable Access Control before program SSID/SVID

Arguments:
  Bus                PCI Bus Number
  Dev                PCI Device Number
  Func               PCI Function Number
  SsidSvid           SSID/SVID you want to program

Returns:
  EFI_SUCCESS        Program INTEL EHCI SSID/SVID successfully

--*/
{
    UINT8            EhciAccessCntl = 0;
    UINT8            WrtRdonly = 0;

    //
    // Set EHCI devices WRT_RDONLY bit ( D29:F0 0x80[0], D26:F0 0x80[0] ) to 1, to make SVID and SSID registers are writable
    //
    EhciAccessCntl = PciRead8(PCI_LIB_ADDRESS (Bus,Dev,Func,R_PCH_EHCI_ACCESS_CNTL));
    WrtRdonly = EhciAccessCntl & B_PCH_EHCI_ACCESS_CNTL_ENABLE;
    EhciAccessCntl = EhciAccessCntl | V_PCH_EHCI_ACCESS_CNTL_ENABLE;
    CompalSsidSvidDxePciWrite (Bus, Dev, Func, R_PCH_EHCI_ACCESS_CNTL, WIDTH8, &EhciAccessCntl);

    //
    // Program SSID & SVID as DID & VID.
    //
    CompalSsidSvidDxePciWrite (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, WIDTH32, &SsidSvid);

    //
    // Restore the EHCI devices WRT_RDONLY bit ( D29:F0 0x80[0], D26:F0 0x80[0] ) value
    //
    EhciAccessCntl = PciRead8(PCI_LIB_ADDRESS (Bus,Dev,Func,R_PCH_EHCI_ACCESS_CNTL));
    EhciAccessCntl = EhciAccessCntl & ( ~B_PCH_EHCI_ACCESS_CNTL_ENABLE );
    EhciAccessCntl = EhciAccessCntl | WrtRdonly;
    CompalSsidSvidDxePciWrite (Bus, Dev, Func, R_PCH_EHCI_ACCESS_CNTL, WIDTH8, &EhciAccessCntl);

    return EFI_SUCCESS;
}

EFI_STATUS
ProgramXhciSsidSvid (
    IN  UINT8        Bus,
    IN  UINT8        Dev,
    IN  UINT8        Func,
    IN  UINT32       SsidSvid
)
/*++
Routine Description:
  Program INTEL XHCI SSID/SVID.

Arguments:
  Bus                PCI Bus Number
  Dev                PCI Device Number
  Func               PCI Function Number
  SsidSvid           SSID/SVID you want to program

Returns:
  EFI_SUCCESS        Program INTEL EHCI SSID/SVID successfully

--*/
{
    //
    // Program SSID & SVID
    //
    CompalSsidSvidDxePciWrite (Bus, Dev, Func, PCI_SUBSYSTEM_VENDOR_ID_OFFSET, WIDTH32, &SsidSvid);

    return EFI_SUCCESS;
}

EFI_STATUS
PciFindCapId (
    IN  UINT8        Bus,
    IN  UINT8        Device,
    IN  UINT8        Function,
    IN  UINT8        CapId,
    OUT UINT8        *CapHeader
)
/*++
Routine Description:
  Find the Offset to a given Capabilities ID
  CAPID list:
    0x01 = PCI Power Management Interface
    0x04 = Slot Identification
    0x05 = MSI Capability
    0x0D = SSID / SSVID Capability
    0x10 = PCI Express Capability

Arguments:
  Bus                PCI Bus Number
  Device             PCI Device Number
  Function           PCI Function Number
  CapId              CAPID to search for
  CapHeader          Offset of desired CAPID

Returns:
  EFI_SUCCESS        CAPID found
  EFI_NOT_FOUND      CAPID not found
  EFI_UNSUPPORTED    Capabilities List not supported

--*/
{
    UINT16           PciStatusRegister;
    UINT8            PciHeaderType;
    UINT8            CapPtrOffset;
    UINT8            CurrentCapId;

    //
    // Check Capabilities List bit
    //
    PciStatusRegister = PciRead16(PCI_LIB_ADDRESS (Bus,Device,Function,PCI_PRIMARY_STATUS_OFFSET));
    if ( !( PciStatusRegister & EFI_PCI_STATUS_CAPABILITY ) ) {
        return EFI_UNSUPPORTED;
    }

    //
    // Check the header layout to determine the Offset of Capabilities Pointer Register
    //
    PciHeaderType = PciRead8(PCI_LIB_ADDRESS (Bus,Device,Function,PCI_HEADER_TYPE_OFFSET));
    if ( ( PciHeaderType & HEADER_LAYOUT_CODE ) == HEADER_TYPE_CARDBUS_BRIDGE ) {
        //
        // If CardBus bridge, start at Offset 0x14
        //
        CapPtrOffset = EFI_PCI_CARDBUS_BRIDGE_CAPABILITY_PTR;
    } else {
        //
        // Otherwise, start at Offset 0x34
        //
        CapPtrOffset = EFI_PCI_CAPABILITY_PTR;
    }

    *CapHeader = PciRead8(PCI_LIB_ADDRESS (Bus,Device,Function,CapPtrOffset));
    if ( *CapHeader == 0xFF ) {
        return EFI_UNSUPPORTED;
    }

    while ( *CapHeader != 0x00 ) {
        //
        // Each capability must be DWORD aligned.
        //
        // The bottom two bits of all pointers ( including the initial pointer ) are reserved and must be implemented as 00b although software must mask them to allow for future uses of these bits.
        //
        *CapHeader = *CapHeader & ( ~( BIT1 | BIT0 ) );

        CurrentCapId = PciRead8(PCI_LIB_ADDRESS (Bus,Device,Function,*CapHeader));
        if ( CurrentCapId == CapId ) {
            return EFI_SUCCESS;
        }

        *CapHeader = PciRead8(PCI_LIB_ADDRESS (Bus,Device,Function,( *CapHeader + 0x01 )));
    }

    return EFI_NOT_FOUND;
}

EFI_STATUS
CompalSsidSvidDxePciWrite (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT8                            Offset,
    IN  EFI_BOOT_SCRIPT_WIDTH            DataWidth,
    IN  VOID                             *DataPtr
)
/*++
Routine Description:
  Set PCI register during POST and EFI_ACPI_S3_RESUME_SCRIPT_TABLE

Arguments:
  Bus                PCI Bus Number
  Dev                PCI Device Number
  Func               PCI Function Number
  Offset             Offset of desired register
  DataWidth          Data Size of desired register
  DataPtr            Data of desired register

Returns:
  EFI_SUCCESS        Set PCI register successfully

--*/
{
    UINT64                    PciAddress;
    S3_BOOT_SCRIPT_LIB_WIDTH  S3DataWidth;
    //
    // Write PCI Register
    //
    switch (DataWidth) {
    case EfiBootScriptWidthUint8:
        PciWrite8(PCI_LIB_ADDRESS (Bus,Dev,Func,Offset),*(UINT8*)DataPtr);
        S3DataWidth = S3BootScriptWidthUint8;
        break;
    case EfiBootScriptWidthUint16:
        PciWrite16(PCI_LIB_ADDRESS (Bus,Dev,Func,Offset),*(UINT16*)DataPtr);
		S3DataWidth = S3BootScriptWidthUint16;
        break;
    case EfiBootScriptWidthUint32:
    default:
        PciWrite32(PCI_LIB_ADDRESS (Bus,Dev,Func,Offset),*(UINT32*)DataPtr);
		S3DataWidth = S3BootScriptWidthUint32;
        break;
    }

    PciAddress = EFI_PCI_ADDRESS(Bus, Dev, Func, Offset);

    //
    // Restore the PCI register when S3 resume
    //
    S3BootScriptSavePciCfgWrite (
                                S3DataWidth,
                                (UINTN) (EFI_PCI_ADDRESS (Bus, Dev, Func, Offset)),
                                1,
                                DataPtr
                                );    
#if 0
    SCRIPT_PCI_CFG_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                          DataWidth,
                          PciAddress,
                          1,
                          DataPtr
                         );
#endif

    return EFI_SUCCESS;
}

EFI_STATUS
CompalSsidSvidDxePciRead (
    IN  UINT8                            Bus,
    IN  UINT8                            Dev,
    IN  UINT8                            Func,
    IN  UINT8                            Offset,
    IN  EFI_BOOT_SCRIPT_WIDTH            DataWidth,
    IN  VOID                             *DataPtr
)
/*++
Routine Description:
  Get PCI register

Arguments:
  Bus                PCI Bus Number
  Dev                PCI Device Number
  Func               PCI Function Number
  Offset             Offset of desired register
  DataWidth          Data Size of desired register
  DataPtr            Data of desired register

Returns:
  EFI_SUCCESS        Get PCI register successfully

--*/
{
    //
    // Read PCI Register
    //
    switch (DataWidth) {

    case EfiBootScriptWidthUint8:
        *(UINT8*)DataPtr = PciRead8(PCI_LIB_ADDRESS (Bus,Dev,Func,Offset));
        break;

    case EfiBootScriptWidthUint16:
        *(UINT16*)DataPtr = PciRead16(PCI_LIB_ADDRESS (Bus,Dev,Func,Offset));
        break;

    case EfiBootScriptWidthUint32:

    default:
        *(UINT32*)DataPtr = PciRead32(PCI_LIB_ADDRESS (Bus,Dev,Func,Offset));
        break;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CompalSsidSvidDxeProgramEntryPoint (
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS                            Status;

    //
    // Initial script for S3 resume
    //
//    INITIALIZE_SCRIPT (ImageHandle, SystemTable);

    //
    // Allocate pool for CompalSsidSvidDxeProtocol
    //
    CompalSsidSvidDxeProtocol = AllocateZeroPool (sizeof (COMPAL_SSID_SVID_DXE_PROTOCOL));
    if (CompalSsidSvidDxeProtocol == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Initialize the CompalSsidSvidDxeProtocol
    //
    CompalSsidSvidDxeProtocol->CompalSsidSvidDxeWrite = CompalSsidSvidDxeWrite;
    CompalSsidSvidDxeProtocol->CompalSsidSvidDxePciRead = CompalSsidSvidDxePciRead;
    CompalSsidSvidDxeProtocol->CompalSsidSvidDxePciWrite = CompalSsidSvidDxePciWrite;

    //
    // Install the CompalSsidSvidDxeProtocol
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gCompalSsidSvidDxeProtocolGuid,
                 CompalSsidSvidDxeProtocol,
                 NULL
             );

    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    return Status;
}
