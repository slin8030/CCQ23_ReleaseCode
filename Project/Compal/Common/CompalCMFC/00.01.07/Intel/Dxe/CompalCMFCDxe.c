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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#include "CompalCMFCDxe.h"

extern EFI_GUID gEfiGlobalNvsAreaProtocolGuid;

UINT32
GetAcpiAdr (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_HANDLE        DeviceHandle
  )
/*++
Routine Description:
  Get ACPI Address

Arguments:
    ImageHandle     - Handle for the image of this driver
    DeviceHandle    - Pointer to the device handle

Returns:
  ACPI Address

Note:
  None
		 
--*/
{
    EFI_STATUS                    Status;
    EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
    ACPI_ADR_DEVICE_PATH          *AcpiAdr;
    UINT8                         *Ptr;

    //
    // Open Device Path Protocol
    //
    Status = gBS->OpenProtocol (
                 DeviceHandle,
                 &gEfiDevicePathProtocolGuid,
                 (VOID **) &DevicePath,
                 ImageHandle,
                 DeviceHandle,
                 EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
                 );
    if (EFI_ERROR (Status)) {
      return 0;
    }
    while (DevicePath->Type != END_DEVICE_PATH_TYPE) {
      if ((DevicePath->Type == ACPI_DEVICE_PATH) && (DevicePath->SubType == ACPI_ADR_DP))
        break;
      Ptr = (UINT8 *) DevicePath;
      Ptr += DevicePath->Length[0];
      DevicePath = (EFI_DEVICE_PATH_PROTOCOL *)Ptr;
    }
    AcpiAdr = (ACPI_ADR_DEVICE_PATH *) DevicePath;

    return AcpiAdr->ADR;
}

EFI_STATUS
CompalCMFCDxeEntry (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS                        Status;
    EFI_EVENT                         Event;
    COMPAL_GLOBAL_NVS_AREA            *CompalGlobalNvsArea;
    COMPAL_GLOBAL_NVS_PROTOCOL        *CompalGlobalNvsAreaProtocol;
    UINTN                             EdidSize;
    UINTN                             Index;
    UINTN                             Offset;
    UINTN                             NumHandles;
    EFI_HANDLE                        *Handles;
    UINT32                            AcpiAdress;
    EFI_EDID_DISCOVERED_PROTOCOL      *EdidData;

//__debugbreak();

    Status = gBS->LocateProtocol (&gCompalGlobalNvsProtocolGuid, NULL, &CompalGlobalNvsAreaProtocol);
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    CompalGlobalNvsArea = CompalGlobalNvsAreaProtocol->CompalArea;

    //
    // Local Handle Buffer of EDID Discovered Protocol
    //
    Status = gBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiEdidDiscoveredProtocolGuid,
                 NULL,
                 &NumHandles,
                 &Handles
                 );
    if (!EFI_ERROR (Status)) {
      for (Index = 0; Index < NumHandles; Index++) {
        //
        // Open EDID Discovered Protocol
        //
        Status = gBS->OpenProtocol (
                 Handles[Index],
                 &gEfiEdidDiscoveredProtocolGuid,
                 (VOID **) &EdidData,
                 ImageHandle,
                 Handles[Index],
                 EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
                 );
        if (EFI_ERROR (Status)) {
          return Status;
        }

        EdidSize = EdidData->SizeOfEdid;
        if (EdidSize) {
          AcpiAdress = GetAcpiAdr (ImageHandle, Handles[Index]);
          if (AcpiAdress & LCD_ACPI_ADR) {
            for (Offset = 0; Offset < EdidSize; Offset++) {
              //
              // Get the EDID from Panel and save to Compal Global NVS
              //
              CompalGlobalNvsArea->EDIDFullData[Offset] = EdidData->Edid[Offset];
            } //end of for (Offset = 0; Offset < EdidSize; Offset++)
          } //end of if (AcpiAdress & LCD_ACPI_ADR)
          else {
            goto LegacyBIOS;
          }
        } //end of if (EdidSize)
        else {
          goto LegacyBIOS;
        }

      } // end of for (Index = 0; Index < NumHandles; Index++)

      return Status;

    } else {

LegacyBIOS:

      Status = gBS->CreateEventEx (
                 EVT_NOTIFY_SIGNAL,
                 TPL_NOTIFY,
                 CompalCMFCEventFunction,
                 NULL,
                 &gEfiEventReadyToBootGuid,
                 &Event
             );

      return EFI_SUCCESS;
    }
}

VOID
EFIAPI
CompalCMFCEventFunction (
    IN EFI_EVENT        Event,
    IN VOID             *Context
)
{

    UINT16                            InternalVgaVid;
    UINT16                            ExternalVgaVid;

    InternalVgaVid = PciRead16(PCI_LIB_ADDRESS (SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, 0x00));
    ExternalVgaVid = 0xFFFF;

    if ( (InternalVgaVid == IGD_VID )|| ( (InternalVgaVid == IGD_VID) && (ExternalVgaVid != 0xFFFF) )) {
        CompalGetEDID(IGD_VID,OEM_Intel_EDID_Controller_Number);
    } else if ( ExternalVgaVid == ATI_VID ) {
        CompalGetEDID(ATI_VID,OEM_ATI_EDID_Controller_Number);
    } else if ( ExternalVgaVid == Nvidia_VID ) {
        CompalGetEDID(Nvidia_VID,OEM_Nvidia_EDID_Controller_Number);
    }

}

EFI_STATUS
CompalGetEDID (
    IN UINT16            VgaVid,
    IN UINT8             EDID_Controller_Number
)
{
    EFI_STATUS                        Status = EFI_SUCCESS;
    UINT16                            i;
    UINTN                             NumberOfPages;              // Number of 4KB pages in EdidDataBuffer
    EFI_PHYSICAL_ADDRESS              EdidDataBufferAddr;         // Buffer for all VBE Information Blocks
    EFI_IA32_REGISTER_SET             Regs;
    EFI_LEGACY_BIOS_PROTOCOL          *LegacyBios;
    COMPAL_GLOBAL_NVS_AREA            *CompalGlobalNvsArea;
    COMPAL_GLOBAL_NVS_PROTOCOL        *CompalGlobalNvsAreaProtocol;

    Status = gBS->LocateProtocol (&gCompalGlobalNvsProtocolGuid, NULL, &CompalGlobalNvsAreaProtocol);
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    CompalGlobalNvsArea = CompalGlobalNvsAreaProtocol->CompalArea;

    Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **) &LegacyBios);
    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    //
    // Allocate buffer under 1MB for VBE data structures
    //
    NumberOfPages = EFI_SIZE_TO_PAGES ( sizeof (VESA_BIOS_EXTENSIONS_EDID_DATA_BLOCK) );

    EdidDataBufferAddr = 0x00100000 - 1;
    Status = (gBS->AllocatePages) (
                 AllocateMaxAddress,
                 EfiBootServicesData,
                 NumberOfPages,
                 &EdidDataBufferAddr
             );

    if (EFI_ERROR (Status)) {
        return Status;
    }

    //
    // Read EDID information
    //
    ZeroMem(&Regs, sizeof (EFI_IA32_REGISTER_SET));

    // Intel
    if ( VgaVid == IGD_VID ) {
        Regs.X.AX = VESA_BIOS_EXTENSIONS_EDID;
        Regs.X.BX = 0x0001;
        Regs.X.CX = EDID_Controller_Number;
        Regs.X.DX = 0x0000;
        Regs.X.ES = EFI_SEGMENT ((UINTN) EdidDataBufferAddr);
        Regs.X.DI = EFI_OFFSET ((UINTN) EdidDataBufferAddr);

        // ATI
    } else if ( VgaVid == ATI_VID ) {
        Regs.H.AH = ATI_INT10_AH;
        Regs.H.AL = ATI_INT10_DISPLAY_DATA_CHANNEL_SERVICE;
        Regs.H.BL = ATI_DDC_FORMAT_SUPPORT;
        Regs.H.BH = EDID_Controller_Number;
        LegacyBios->Int86 (LegacyBios, 0x10, &Regs);

        if ( Regs.H.AH == ATI_SUCCESS ) {
            Regs.H.AH = ATI_INT10_AH;
            Regs.H.AL = ATI_INT10_DISPLAY_DATA_CHANNEL_SERVICE;
            Regs.H.BL = ATI_READ_EDID;
            Regs.H.BH = EDID_Controller_Number;
            Regs.X.ES = EFI_SEGMENT ((UINTN) EdidDataBufferAddr);
            Regs.X.DI = EFI_OFFSET ((UINTN) EdidDataBufferAddr);
        }

        // nVidia
    } else if ( VgaVid == Nvidia_VID ) {
        Regs.X.AX = VESA_BIOS_EXTENSIONS_EDID;
        Regs.X.BX = 0x0001;
        Regs.X.CX = EDID_Controller_Number;
        Regs.X.DX = 0x0000;
        Regs.X.ES = EFI_SEGMENT ((UINTN) EdidDataBufferAddr);
        Regs.X.DI = EFI_OFFSET ((UINTN) EdidDataBufferAddr);
    }

    // Invoke INT 10h
    LegacyBios->Int86 (LegacyBios, 0x10, &Regs);

    //
    // See if the VESA call succeeded
    //
    if ( ((VgaVid == IGD_VID || VgaVid == Nvidia_VID) && (Regs.X.AX == VESA_BIOS_EXTENSIONS_STATUS_SUCCESS)) ||
            ((VgaVid == ATI_VID) && (Regs.H.AH == ATI_SUCCESS)) ) {
        for (i=0; i<sizeof (VESA_BIOS_EXTENSIONS_EDID_DATA_BLOCK); i++) {
            CompalGlobalNvsArea->EDIDFullData[i] = *((UINT8*) (UINTN) (EdidDataBufferAddr + i));
        }
    }

    //
    // Free memory allocated below 1MB
    //
    if (EdidDataBufferAddr != 0) {
        (gBS->FreePages) (EdidDataBufferAddr, NumberOfPages);
    }

    return Status;
}



