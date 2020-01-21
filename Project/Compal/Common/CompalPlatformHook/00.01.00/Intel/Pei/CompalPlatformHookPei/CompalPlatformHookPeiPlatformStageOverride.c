/*
 * (C) Copyright 2011-2012 Compal Electronics, Inc.
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
*/

#include "CompalPlatformHookPei.h"
#include "CompalPlatformHookPeiPlatformStageOverride.h"
#include "CompalGpioTable.h"
#include "CompalAudioVerbTable.h"
#include <CompalEclib.h>


#define UMA_MODE        0x00
#define DIS_MODE        0x01
#define OPTIMUS_MODE    0x02
#define UNDEFINE        0xFF

UINT8
GetVgaMode(
    VOID
)
{
#if 0 //ifndef COMPAL_ORB_SUPPORT  
    UINT16                GpioOffset;
    UINT32                OptimusEnable, DgpuPresent;

    // Check GPIO38 (OPTIMUS_EN#)
    GpioOffset = PCH_GPIO_BASE_ADDRESS + R_PCH_GPIO_LVL2;
    OptimusEnable = IoRead32(GpioOffset) & BIT6;

    // Check GPIO67 (DGPU_HOLD_RST#)
    GpioOffset = PCH_GPIO_BASE_ADDRESS + R_PCH_GPIO_LVL3;
    DgpuPresent = IoRead32(GpioOffset) & BIT3;

    // For UMA Mode(OPTIMUS_EN-HIGH#, DGPU_PRSNT#-HIGH)
    if ((OptimusEnable != 0) && (DgpuPresent != 0)) {
        return UMA_MODE;
    } else if ((OptimusEnable != 0) && (DgpuPresent == 0)) {
        // For DIS Mode (OPTIMUS_EN-HIGH#, DGPU_PRSNT#-LOW)
        return DIS_MODE;
    } else if ((OptimusEnable == 0) && (DgpuPresent == 0)) {
        // For Optimus Mode (OPTIMUS_EN-LOW#, DGPU_PRSNT#-LOW)
        return OPTIMUS_MODE;
    }
#endif
    return UNDEFINE;
}

//
// Modify GPIO50(DGPU_HOLD_RST#) setting for DIS/UMA/OPTIMUS mode
//
EFI_STATUS
Gpio50HookBeforeDgpuHoldRst (
    IN  UINT8                GpioIndex,
    IN  COMPAL_GPIO_TYPE     *GpioType,
    IN  COMPAL_GPIO_LEVEL    *GpioLevel,
    IN  UINT32               *GpioExtData
)
{
    UINT8                 DisplayMode;

    DisplayMode = GetVgaMode();

    // For DIS mode, set DGPU_HOLD_RST# = High, DGPU_PWR_EN = High
    if (DisplayMode == DIS_MODE) {
        *GpioLevel = HIGH;
    } else {
        *GpioLevel = LOW;
    }

    return EFI_SUCCESS;
}

//
// Modify GPIO54(DGPU_PWR_EN) setting for DIS/UMA/OPTIMUS mode
//
EFI_STATUS
Gpio54HookBeforeDgpuPwrEn (
    IN  UINT8                GpioIndex,
    IN  COMPAL_GPIO_TYPE     *GpioType,
    IN  COMPAL_GPIO_LEVEL    *GpioLevel,
    IN  UINT32               *GpioExtData
)
{
    UINT8                 DisplayMode;

    DisplayMode = GetVgaMode();

    // For DIS mode, set DGPU_PWR_EN = High
    if (DisplayMode == DIS_MODE) {
        *GpioLevel = HIGH;
    } else {
        *GpioLevel = LOW;
    }

    return EFI_SUCCESS;
}


EFI_STATUS
ProjectPlatformHookPeiPlatformStage1Override (
    IN CONST EFI_PEI_SERVICES          **PeiServices
)
{
    EFI_STATUS                     Status = EFI_SUCCESS;

    UINT16                         CompalGpioTableCount;
    UINT16                         CompalGpioTableVersion;
    COMPAL_PLATFORM_INIT_PEI_PPI   *CompalPlatformInitPeiPpi;
	UINT8                          B_BIOS_DEC_EN1;

    CompalGpioTableCount = 0;

//__debugbreak():
	
    //
    // Modify Intel EC Index IO to 0xFD60 and 0x68 for EC Lib.
    //
    PciOr32( PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_GEN2_DEC), (EC_INDEXIO_BASE   + (0x04 << 16) + 1));
    PciOr32( PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_GEN3_DEC), (EC_CMD_METHOD_APP + (0x04 << 16) + 1));

	//
	// Set EC Memory Mapping Base Address
	//
	B_BIOS_DEC_EN1 = PciRead8(PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_FWH_BIOS_DEC));
	PciWrite8(PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_FWH_BIOS_DEC),(B_BIOS_DEC_EN1 & (~B_PCH_LPC_FWH_BIOS_DEC_40)));
	
	PciWrite32(PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, R_PCH_LPC_LGMR),((EC_MEMORY_MAPPING_BASE_ADDRESS | B_PCH_LPC_LGMR_LMRD_EN) ));
	

    Status = PeiServicesLocatePpi (&gCompalPlatformInitPeiPpiGuid, 0, NULL, &CompalPlatformInitPeiPpi);
	
    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    CompalGpioTableVersion = COMPAL_GPIO_SETTINGS_VERISON;

#if 1 //ifdef COMPAL_ORB_SUPPORT
	CompalGpioTableCount = sizeof (mGpioDefinition_ORB) / sizeof (COMPAL_GPIO_SETTINGS);
	Status = CompalPlatformInitPeiPpi->CompalPlatformInitPeiGpio ( PeiServices, mGpioDefinition_ORB, CompalGpioTableCount, CompalGpioTableVersion );

//[-start-121001-IB03780462-add]//
  //
  // Lynx Point spec v0.6.1 ch.22.16.5: The System BIOS must program GPIOBASE + 10h [15:0] = FFFFh to enable all
  // supported GPI pins to route to PIRQ[X:I] which then can generate IOxAPIC interrupts[39:24].
  // 
// [PRJ] 20121128 Well.Chang - Disable unused PIRQ.
// Please check GPIO Mode Input to PIRQ Mapping in EDS for detail information.
  IoWrite32 ((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_IOAPIC_SEL, 0xEC32);
//  IoWrite32 ((UINT16)(PciRead32 (LPC_PCI_GPIOBASE) & ACPI_GPIO_BASE_MASK) + R_PCH_GPIO_IOAPIC_SEL, V_PCH_GPIO_IOAPIC_SEL);
// [PRJ] 20121128 Well.Chang - Disable unused PIRQ.
//[-end-121001-IB03780462-add]//

#endif

    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

#ifdef COMPAL_DEBUG_BY_EC_SUPPORT
    CompalDebugMsg ((PeiServices," CompalGpio Table Count %x \n",CompalGpioTableCount));
	CompalDebugMsg ((PeiServices," CompalGpio Table Version %x \n",CompalGpioTableVersion));
#endif

  return Status;
  }

EFI_STATUS
ProjectPlatformHookPeiPlatformStage2Override (
    IN CONST EFI_PEI_SERVICES          **PeiServices
)
{
    EFI_STATUS                     Status = EFI_SUCCESS;
    EFI_BOOT_MODE                  BootMode;
    COMPAL_PLATFORM_INIT_PEI_PPI   *CompalPlatformInitPeiPpi;
    //
    // Intial Audio Verb table for Crisis mode only.
    //
    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);

    Status = PeiServicesLocatePpi (&gCompalPlatformInitPeiPpiGuid, 0, NULL, &CompalPlatformInitPeiPpi);

    if ( EFI_ERROR( Status ) ) {
      return Status;
    }

    Status = CompalPlatformInitPeiPpi->CompalPlatformInitPeiAudioVerbTable ( PeiServices, &CompalAzaliaVerbTable_ORB[0], COMPAL_VERB_TABLE_NUMBER_ORB );
    if ( EFI_ERROR( Status ) ) {
      return Status;
    }

    return Status;
}
