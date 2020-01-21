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
 1.01      12/26/14  Champion Liu Add CMFC new function for CMFC spec 00.00.13.
           01/05/15  Champion Liu Add CMFC 0x14, sub_fun 0x05 for 00.00.15 spec.
 1.02      01/08/15  Champion Liu Merge CMFC 0x24/0x25 function.
           01/15/15  Champion Liu Add CMFC 0x14, sub_fun 0x03/0x04 for 00.00.14 spec.
           01/21/15  Champion Liu Update CMFC SMI version to 0x15 for 00.00.15 spec.
           02/02/15  Champion Liu Add CMFC 0x14, sub_fun 0x06 for 00.00.16 spec.
*/

#ifndef _COMPAL_CMFC_COMMON_FUNCTIONS_
#define _COMPAL_CMFC_COMMON_FUNCTIONS_

#include "CompalCMFCCommonSwSmi.h"
#include <CompalGlobalNvsArea.h>
#include <CompalEclib.h>
#include <CompalCMFCSmmHookProtocol.h>
#include <CompalGlobalNvsProtocol.h>
#include <SetupConfig.h>
#include <EdidDefinition.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/SmmSwDispatch.h>

extern EFI_BOOT_SERVICES  *gBS;

#if COMPAL_S35_SUPPORT
#include "CompalS35.h"
#include "CompalS35Lib.h"
#endif

#define PCH_RCBA_ADDRESS        0xFED1C000 // Root Complex Base Address
#define R_PCH_RCRB_SPI_BASE     0x3800 
#define R_PCH_SPI_HSFS          (R_PCH_RCRB_SPI_BASE + 0x04)
#define B_PCH_SPI_HSFS_FLOCKDN  BIT1

#define R_PCH_SPI_PR0           0x84       ///< Protected Region 0 Register
#define R_PCH_SPI_PR1           0x88       ///< Protected Region 1 Register
#define R_PCH_SPI_PR2           0x8C       ///< Protected Region 2 Register
#define R_PCH_SPI_PR3           0x90       ///< Protected Region 3 Register
#define R_PCH_SPI_PR4           0x94       ///< Protected Region 4 Register

#define B_PCH_SPI_PR0_WPE       BIT31      ///< Write Protection Enable
#define B_PCH_SPI_PR1_WPE       BIT31      ///< Write Protection Enable
#define B_PCH_SPI_PR2_WPE       BIT31      ///< Write Protection Enable
#define B_PCH_SPI_PR3_WPE       BIT31      ///< Write Protection Enable
#define B_PCH_SPI_PR4_WPE       BIT31      ///< Write Protection Enable

//
// Define SMI Status
//
#define SwSmiFail         0x0000
#define SwSmiSuccess      0x0001
#define SwSmiNotSupport   0x8000

#define SSMIVersion       0x16       // Must update Verion when anyone

// Function Prototype
VOID
CompalCMFCFunctionEntry (
    FunctionBuffer FB
);

//
// FunNo 0x10
//
VOID
SetAPVersion (
    VOID
);

VOID
ReturnVersion (
    VOID
);

VOID
GetFWVersion (
    VOID
);
//
// FunNo 0x12
//
VOID
GetManufacturTypeId (
    VOID
);

VOID
GetPanelMax (
    VOID
);

VOID
GetEDIDFullData (
    VOID
);

//
// FunNo 0x13
//
VOID
SetBootDeviceSequence (
    VOID
);

VOID
GetBootDeviceSequence (
    VOID
);

VOID
GetBootDeviceGUID (
    VOID
);

VOID
GetBootDeviceNAME (
    VOID
);

VOID
SetBootDeviceValue (
    VOID
);

//
// FunNo 0x14
//
EFI_STATUS
OemBIOSLoadDefault(
    VOID
);

EFI_STATUS
OemBIOSDefaultGUID(
    VOID
);

EFI_STATUS
OemBIOSDefaultNAME(
    VOID
);


EFI_STATUS
OemBIOSEepromGUID(
    VOID
);

EFI_STATUS
OemBIOSEepromNAME(
    VOID
);

EFI_STATUS
OemGetMFGReservedArea(
    VOID
);

EFI_STATUS
OemGetOnBoardSPDdata(
    VOID
);
//
// FunNo 0x15
//
VOID
ECCommunicateInterface (
    VOID
);

//
// FunNo 0x16 for S3.5
//
#if COMPAL_S35_SUPPORT
VOID
S35APInstalled(
    VOID
);

VOID
S35Enable(
    VOID
);

VOID
S35Disable(
    VOID
);

VOID
GetS35Status(
    VOID
);
#endif

VOID
ClearSupervisorPassword(
    VOID
);

VOID
ClearUserPassword(
    VOID
);

VOID
CheckSupervisorPassword(
    VOID
);

VOID
CheckUserPassword(
    VOID
);

VOID
CheckHDDPassword(
    VOID
);

VOID
SetBootTypeCsm(
    VOID
);

VOID
SetBootTypeUEFI(
    VOID
);

VOID
SetBootTypeWPEAndPXEB(
    VOID
);

VOID
SetBootTypeWPEAndNetB(
    VOID
);

VOID
SetBootTypeDUAL(
    VOID
);

VOID
SetPxeBootToLanEnabled(
    VOID
);

VOID
SetSmtDefault(
    VOID
);

VOID
GetBootTypeStatus(
    VOID
);

VOID
SetBootType(
    VOID
);

VOID
SecureBootDisable(
    VOID
);

VOID
SecureBootEnable(
    VOID
);

VOID
GetSecureBootGUID (
    VOID
);

VOID
GetSecureBootNAME (
    VOID
);

VOID
GetSecureBootValue (
    VOID
);

VOID
GetSecureBootStatus(
    VOID
);

VOID
SetOSTypeNonOS(
    VOID
);

VOID
SetOSTypeLinux(
    VOID
);

VOID
SetOSTypeWinXP(
    VOID
);

VOID
SetOSTypeVista(
    VOID
);

VOID
SetOSTypeWin7(
    VOID
);

VOID
SetOSTypeWin8(
    VOID
);

VOID
SetOSTypeWin8Pro(
    VOID
);

VOID
SetOSTypeWin7Win8(
    VOID
);

VOID
GetOSTypeStatus(
    VOID
);

VOID
SetOSType(
    VOID
);

VOID
GetVendorName(
    VOID
);

VOID
GetUUIDGuid (
    VOID
);

VOID
GetSerNumGuid (
    VOID
);

VOID
GetProductNameGuid (
    VOID
);

VOID
GetMBSerNumGuid (
    VOID
);

VOID
GetOSTypeGuid (
    VOID
);

VOID
GetUUIDName (
    VOID
);

VOID
GetSerNumName (
    VOID
);

VOID
GetProductNameName (
    VOID
);

VOID
GetMBSerNumName (
    VOID
);

VOID
GetOSTypeName (
    VOID
);

VOID
GetACConnectStatus(
    VOID
);

VOID
GetBatStatus(
    VOID
);

VOID
GetBatConnectStatus(
    VOID
);

VOID
GetBatDesignCapacity(
    VOID
);

VOID
GetBatFullChargeCapacity(
    VOID
);

VOID
GetBatRemainingCapacity(
    VOID
);

VOID
GetBatChargeCurrent(
    VOID
);

VOID
GetBatCurrent(
    VOID
);

VOID
GetBatCycleCounter(
    VOID
);

VOID
GetBatAverageTemperature(
    VOID
);

VOID
SetACPowerSource(
    VOID
);

VOID
ShippingModeEnable(
    VOID
);

VOID
SystemFullChargeLEDOn(
    VOID
);

VOID
SystemChargeLEDOn(
    VOID
);

VOID
AllLEDOn(
    VOID
);

VOID
LEDOutOfTestMode(
    VOID
);

VOID
SetLEDOnOffBlink(
    VOID
);

VOID
GetLEDStatus(
    VOID
);
//
// FunNo 0x28
//
VOID
GetRegionProtectStatus(
    VOID
);

VOID
SetRegionProtect(
    VOID
);

VOID
DisableSPIRegionProtect(
    VOID
);

VOID
EnableSPIRegionProtect(
    VOID
);

VOID
A31Func90(
    VOID
);

VOID
A31GetEDIDOrPN(
    VOID
);

VOID
A31Func91(
    VOID
);

VOID
Func92(
    VOID
);

VOID
Func93(
    VOID
);

VOID
Func94(
    VOID
);

VOID
Func95(
    VOID
);

VOID
Func96(
    VOID
);

VOID
Func97(
    VOID
);

VOID
Func98(
    VOID
);

VOID
Func99(
    VOID
);

#endif
