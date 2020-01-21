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

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Guid/EventGroup.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <VesaBiosExtensions.H>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/LegacyBios.h>
#include <CompalGlobalNvsArea.h>
#include <CompalGlobalNvsProtocol.h>

extern EFI_BOOT_SERVICES  *gBS;

#define SA_IGD_BUS           0x00
#define SA_IGD_DEV           0x02
#define SA_IGD_FUN_0         0x00

#define SA_PEG_BUS_NUM     0x00
#define SA_PEG_DEV_NUM     0x01
#define SA_PEG10_DEV_NUM   SA_PEG_DEV_NUM
#define SA_PEG10_FUN_NUM   0x00
#define SA_PEG11_DEV_NUM   SA_PEG_DEV_NUM
#define SA_PEG11_FUN_NUM   0x01
#define SA_PEG12_DEV_NUM   SA_PEG_DEV_NUM
#define SA_PEG12_FUN_NUM   0x02
#define SA_PEG_MAX_FUN     0x03
#define SA_PEG_MAX_LANE    0x10
#define SA_PEG_MAX_BUNDLE  0x08

typedef struct {
    UINT8 Bus;
    UINT8 Device;
    UINT8 Function;
} PEG_PORT;

#define   OEM_Intel_EDID_Controller_Number        0x0002
#define   OEM_ATI_EDID_Controller_Number          0x0001
#define   OEM_Nvidia_EDID_Controller_Number       0x0002

#ifndef IGD_VID
#define IGD_VID                                   0x8086
#endif

#define   ATI_VID	                          0x1002
#define   Nvidia_VID                              0x10DE

//
// ATI VBIOS
//
#define   ATI_INT10_AH                            0xA0

//
// ATI return code
//
#define   ATI_SUCCESS                             0
#define   ATI_ERROR                               1
#define   ATI_FUNCTION_NOT_SUPPORT                2

//
// ATI Sub-function: 0x0B
//
#define   ATI_INT10_DISPLAY_DATA_CHANNEL_SERVICE  0x0B
#define   ATI_DDC_FORMAT_SUPPORT                  0
#define   ATI_READ_EDID                           1

// INTEL Chipset offset define.
#define PCH_PCIE_SCBN                           0x19
#define PCH_PCIE_SBBN                           0x1A

// Get EDID from GOP function.
#define CRT_ACPI_ADR                            0x100
#define HDMI_ACPI_ADR                           0x300
#define LCD_ACPI_ADR                            0x400

VOID
EFIAPI
CompalCMFCEventFunction (
    IN    EFI_EVENT     Event,
    IN    VOID          *Context
);

EFI_STATUS
CompalGetEDID (
    IN    UINT16     VgaVid,
    IN    UINT8      EDID_Controller_Number
);

UINT16
ExternalVGACheck (
    VOID
);

