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
           01/05/14  Champion Liu Add CMFC 0x14, sub_fun 0x05 for 00.00.15 spec.
*/

//
// [COMPAL CMFC SMM] Smm Hook using EDK II
//

#include "CompalCMFCSmmHook.h"

//
// Insyde Code
//
EFI_SMM_SYSTEM_TABLE2                   *mSmst;
EFI_SMM_VARIABLE_PROTOCOL               *SmmVariable;
SYSTEM_CONFIGURATION                    *SetupVariable;
EFI_GUID                                GuidId  = SYSTEM_CONFIGURATION_GUID;

const COMPAL_CMFC_SMM_HOOK_PROTOCOL CompalOverrideInstance = {
    CMFCSwSMIHookMain
};

EFI_STATUS
CompalCMFCSmmHookEntryPoint (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
/*++

  Routine Description:

  Arguments:

    ImageHandle     - Handle for the image of this driver
    SystemTable     - Pointer to the EFI System Table

  Returns:

    EFI_STATUS

--*/
{
    EFI_STATUS                    Status;
    EFI_GUID                      gCompalCMFCSmmHookProtocolGuid = COMPAL_CMFC_SMM_HOOK_PROTOCOL_GUID;
    BOOLEAN                       InSmm= FALSE;
    EFI_SMM_BASE2_PROTOCOL        *mSmmBase= NULL;
    EFI_SETUP_UTILITY_PROTOCOL    *EfiSetupUtility;

    Status = gBS->LocateProtocol (
                 &gEfiSmmBase2ProtocolGuid, 
                 NULL,
                 (VOID **) &mSmmBase
             );

    if (!EFI_ERROR (Status)) {
      mSmmBase->InSmm (mSmmBase, &InSmm);
    }

    if (InSmm) {
        //
        // Great!  We're now in SMM!
        //
        //
        mSmmBase->GetSmstLocation (mSmmBase, &mSmst);

        Status = mSmst->SmmLocateProtocol (
                     &gEfiSmmVariableProtocolGuid,
                     NULL,
                     (VOID **) &SmmVariable
                 );

        if (EFI_ERROR(Status)) {
            return Status;
        }
    }

    Status = gBS->LocateProtocol (
                 &gEfiSetupUtilityProtocolGuid,
                 NULL,
                 &EfiSetupUtility
             );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    SetupVariable = (SYSTEM_CONFIGURATION *)EfiSetupUtility->SetupNvData;

    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gCompalCMFCSmmHookProtocolGuid,
                 &CompalOverrideInstance,
                 NULL
             );
    return Status;
}

EFI_STATUS
EFIAPI
CMFCSwSMIHookMain (
    IN     COMPAL_CMFC_SMM_HOOK_PROTOCOL       *This,
    IN     COMPAL_SW_SMI_COMMON_SMM_HOOK_ID    OverrideType,
    ...
)
/*++

Routine Description:

Arguments:

  This                    - Pointer to the protocol instance.
  DispatchItem            - The item to be dispatched.

Returns:

  EFI_STATUS              - Successfully completed.
  EFI_OUT_OF_RESOURCES    - Out of resources.

--*/
{
    EFI_STATUS    Status;
    VA_LIST       Marker;
    COMPAL_GLOBAL_NVS_AREA   *CompalGlobalNvsArea;

    VA_START(Marker, OverrideType);
    CompalGlobalNvsArea = VA_ARG (Marker, COMPAL_GLOBAL_NVS_AREA  *);
    switch (OverrideType) {
    case CMFC_GET_BOOT_DEVICE_SEQUENCE:
        Status = CMFCGetBootDeviceSequence(CompalGlobalNvsArea);
        break;

    case CMFC_SET_BOOT_DEVICE_SEQUENCE:
        Status = CMFCSetBootDeviceSequence(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BOOT_DEVICE_GUID:
        Status = CMFCGetBootDeviceGUID(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BOOT_DEVICE_NAME:
        Status = CMFCGetBootDeviceNAME(CompalGlobalNvsArea);
        break;

    case CMFC_SET_BOOT_DEVICE_VALUE:
        Status = CMFCSetBootDeviceValue(CompalGlobalNvsArea);
        break;

    case CMFC_BIOS_LOAD_DEFUALT:
        Status = CMFCLoadBiosDefualt(CompalGlobalNvsArea);
        break;

    case CMFC_BIOS_DEFUALT_GUID:
        Status = CMFCLoadDefualtGUID(CompalGlobalNvsArea);
        break;

    case CMFC_BIOS_DEFUALT_NAME:
        Status = CMFCLoadDefualtName(CompalGlobalNvsArea);
        break;

    case CMFC_GET_MFG_RESERVED_AREA:
        Status = CMFCGetMFGReservedArea(CompalGlobalNvsArea);
        break;

    case CMFC_BOOT_TYPE_Csm:
        Status = CMFCSetBootTypeCsm(CompalGlobalNvsArea);
        break;

    case CMFC_BOOT_TYPE_UEFI:
        Status = CMFCSetBootTypeUEFI(CompalGlobalNvsArea);
        break;

    case CMFC_BOOT_TYPE_WPEAndPXEB:
        Status = CMFCSetBootTypeWPEAndPXEB(CompalGlobalNvsArea);
        break;

    case CMFC_BOOT_TYPE_WPEAndNetB:
        Status = CMFCSetBootTypeWPEAndNetB(CompalGlobalNvsArea);
        break;

    case CMFC_BOOT_TYPE_DUAL:
        Status = CMFCSetBootTypeDUAL(CompalGlobalNvsArea);
        break;

    case CMFC_PXE_BOOT_TO_LAN_ENABLED:
        Status = CMFCSetPxeBootToLanEnabled(CompalGlobalNvsArea);
        break;

    case CMFC_SET_SMT_DEFAULT:
        Status = CMFCSetSmtDefault(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BOOT_TYPE:
        Status = CMFCGetBootType(CompalGlobalNvsArea);
        break;

    case CMFC_SET_BOOT_TYPE:
        Status = CMFCSetBootType(CompalGlobalNvsArea);
        break;

    case CMFC_SECURE_BOOT_DIS:
        Status = CMFCSetSecureBootDisable(CompalGlobalNvsArea);
        break;

    case CMFC_SECURE_BOOT_ENA:
        Status = CMFCSetSecureBootEnable(CompalGlobalNvsArea);
       break;

    case CMFC_SECURE_BOOT_GUID:
        Status = CMFCGetSecureBootGUID(CompalGlobalNvsArea);
        break;

    case CMFC_SECURE_BOOT_NAME:
        Status = CMFCGetSecureBootNAME(CompalGlobalNvsArea);
        break;

    case CMFC_SECURE_BOOT_VALUE:
        Status = CMFCGetSecureBootValue(CompalGlobalNvsArea);
        break;

    case CMFC_GET_SECURE_BOOT:
        Status = CMFCGetSecureBoot(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_NonOS:
        Status = CMFCSetOSTypeNonOS(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_Linux:
        Status = CMFCSetOSTypeLinux(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_WinXP:
        Status = CMFCSetOSTypeWinXP(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_Vista:
        Status = CMFCSetOSTypeVista(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_Win7:
        Status = CMFCSetOSTypeWin7(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_Win8:
        Status = CMFCSetOSTypeWin8(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_Win8Pro:
        Status = CMFCSetOSTypeWin8Pro(CompalGlobalNvsArea);
        break;

    case CMFC_OS_TYPE_Win7Win8:
        Status = CMFCSetOSTypeWin7Win8(CompalGlobalNvsArea);
        break;

    case CMFC_GET_OS_TYPE:
        Status = CMFCGetOSType(CompalGlobalNvsArea);
        break;

    case CMFC_SET_OS_TYPE:
        Status = CMFCSetOSType(CompalGlobalNvsArea);
        break;

    case CMFC_GET_VENDOR_NAME:
        Status = CMFCGetVendorName(CompalGlobalNvsArea);
        break;

    case CMFC_CLEAR_SUPERVISOR_PASSWORD:
        Status = CMFCClearSupervisorPassword(CompalGlobalNvsArea);
        break;

    case CMFC_CLEAR_USER_PASSWORD:
        Status = CMFCClearUserPassword(CompalGlobalNvsArea);
        break;

    case CMFC_CHECK_SUPERVISOR_PASSWORD:
        Status = CMFCCheckSupervisorPassword(CompalGlobalNvsArea);
        break;

    case CMFC_CHECK_USER_PASSWORD:
        Status = CMFCCheckUserPassword(CompalGlobalNvsArea);
        break;

    case CMFC_CHECK_HD_PASSWORD:
        Status = CMFCCheckHDPassword(CompalGlobalNvsArea);
        break;

    case CMFC_GET_AC_CONNECT_STATUS:
        Status = CMFCGetACConnectStatus(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_STATUS:
        Status = CMFCGetBatStatus(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_CONNECT_STATUS:
        Status = CMFCGetBatConnectStatus(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_DESIGN_CAPACITY:
        Status = CMFCGetBatDesignCapacity(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_FULL_CHARGE_CAPACITY:
        Status = CMFCGetBatFullChargeCapacity(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_REMAINING_CAPACITY:
        Status = CMFCGetBatRemainingCapacity(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_CHARGE_CURRENT:
        Status = CMFCGetBatChargeCurrent(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_CURRENT:
        Status = CMFCGetBatCurrent(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_CYCLE_COUNTER:
        Status = CMFCGetBatCycleCounter(CompalGlobalNvsArea);
        break;

    case CMFC_GET_BAT_AVERAGE_TEMPERATURE:
        Status = CMFCGetBatAverageTemperature(CompalGlobalNvsArea);
        break;

    case CMFC_SET_AC_POWER_SOURCE:
        Status = CMFCSetACPowerSource(CompalGlobalNvsArea);
        break;

    case CMFC_SHIPPING_MODE_ENABLE:
        Status = CMFCShippingModeEnable(CompalGlobalNvsArea);
        break;

    case CMFC_SYSTEM_FULL_CHARGE_LED_ON:
        Status = CMFCSystemFullChargeLEDOn(CompalGlobalNvsArea);
        break;

    case CMFC_SYSTEM_CHARGE_LED_ON:
        Status = CMFCSystemChargeLEDOn(CompalGlobalNvsArea);
        break;

    case CMFC_ALL_LED_ON:
        Status = CMFCAllLEDOn(CompalGlobalNvsArea);
        break;

    case CMFC_LED_OUT_OF_TEST_MODE:
        Status = CMFCLEDOutOfTestMode(CompalGlobalNvsArea);
        break;

    case CMFC_SET_LED_ON_OFF_BLINK:
        Status = CMFCSetLEDOnOffBlink(CompalGlobalNvsArea);
        break;

    case CMFC_GET_LED_STATUS:
        Status = CMFCGetLEDStatus(CompalGlobalNvsArea);
        break;

    case CMFC_A31_FUNC_90:
        Status = CMFCA31Func90(CompalGlobalNvsArea);
        break;

    case CMFC_A31_FUNC_91:
        Status = CMFCA31Func91(CompalGlobalNvsArea);
        break;

    case CMFC_A31_GET_EDID_OR_PN:
        Status = CMFCA31GetEDIDOrPN(CompalGlobalNvsArea);
        break;

    case CMFC_A32_FUNC_92:
        Status = CMFCA32Func92(CompalGlobalNvsArea);
        break;

    case CMFC_A32_FUNC_93:
        Status = CMFCA32Func93(CompalGlobalNvsArea);
        break;

    case CMFC_ABO_FUNC_94:
        Status = CMFCABOFunc94(CompalGlobalNvsArea);
        break;

    case CMFC_ABO_FUNC_95:
        Status = CMFCABOFunc95(CompalGlobalNvsArea);
        break;

    case CMFC_A51_FUNC_96:
        Status = CMFCA51Func96(CompalGlobalNvsArea);
        break;

    case CMFC_A51_FUNC_97:
        Status = CMFCA51Func97(CompalGlobalNvsArea);
        break;

    case CMFC_C38_FUNC_98:
        Status = CMFCC38Func98(CompalGlobalNvsArea);
        break;

    case CMFC_C38_FUNC_99:
        Status = CMFCC38Func99(CompalGlobalNvsArea);
        break;

    default:
        Status = EFI_UNSUPPORTED;
        break;
    }

    VA_END(Marker);

    return Status;
}

EFI_STATUS
CMFCGetBootDeviceSequence(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{

//
// InsydeCode
//
    UINTN                                BufferSize;
    UINT8                                TempBootOrder[MAX_BOOT_ORDER_NUMBER];
    UINT8                                Index, IndexBuffer;

    BufferSize = sizeof (SYSTEM_CONFIGURATION);

    for (Index = 0; Index < MAX_BOOT_ORDER_NUMBER; Index++) {
        TempBootOrder[Index] = SetupVariable->BootTypeOrder[Index];
    }

    IndexBuffer=0;
    for (Index=0; Index<8; Index++) {
        switch (TempBootOrder[Index]) {
        case BBS_HARDDISK:
            CompalGlobalNvsArea->BufferData[IndexBuffer] = HDD0;
            IndexBuffer++;
            break;
        case BBS_CDROM:
            CompalGlobalNvsArea->BufferData[IndexBuffer] = CDROM;
            IndexBuffer++;
            break;
        case BBS_USB:
            CompalGlobalNvsArea->BufferData[IndexBuffer] = USBHDD;
            IndexBuffer++;
            break;
        case BBS_BEV_DEVICE:
            CompalGlobalNvsArea->BufferData[IndexBuffer] = LAN;
            IndexBuffer++;
            break;
        case BBS_FLOPPY:
            CompalGlobalNvsArea->BufferData[IndexBuffer] = USBFloppy;
            IndexBuffer++;
            break;
        default:
            break;
        }
    }

    for ( ; IndexBuffer<MAX_BOOT_ORDER_NUMBER; IndexBuffer++) {
        CompalGlobalNvsArea->BufferData[IndexBuffer] = 0x3F;
    }

    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootDeviceSequence(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
//
// InsydeCode
//
    EFI_STATUS                           Status;
    UINTN                                BufferSize;
    UINT8                                Index, IndexBuffer;

    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    IndexBuffer = 0;
    for (Index=0; Index<MAX_BOOT_ORDER_NUMBER; Index++) {
        switch (CompalGlobalNvsArea->BufferData[Index]) {
        case HDD0:
            CompalGlobalNvsArea->BufferData[IndexBuffer]= HDD0;
            SetupVariable->BootTypeOrder[IndexBuffer] = BBS_HARDDISK;
            IndexBuffer++;
            break;
        case CDROM:
            CompalGlobalNvsArea->BufferData[IndexBuffer]= CDROM;
            SetupVariable->BootTypeOrder[IndexBuffer] = BBS_CDROM;
            IndexBuffer++;
            break;
        case LAN:
            CompalGlobalNvsArea->BufferData[IndexBuffer]= LAN;
            SetupVariable->BootTypeOrder[IndexBuffer] = BBS_BEV_DEVICE;
            IndexBuffer++;
            break;
        case USBHDD:
            CompalGlobalNvsArea->BufferData[IndexBuffer]= USBHDD;
            SetupVariable->BootTypeOrder[IndexBuffer] = BBS_USB;
            IndexBuffer++;
            break;
        case USBFloppy:
            CompalGlobalNvsArea->BufferData[IndexBuffer]= USBFloppy;
            SetupVariable->BootTypeOrder[IndexBuffer] = BBS_FLOPPY;
            IndexBuffer++;
            break;
        default:
            break;
        }
    }

    for ( ; IndexBuffer<MAX_BOOT_ORDER_NUMBER; IndexBuffer++) {
        CompalGlobalNvsArea->BufferData[IndexBuffer] = 0x3F;
    }

    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );
    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
    }
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    return EFI_SUCCESS;
}


EFI_STATUS
CMFCGetBootDeviceGUID(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetBDPVGuid[] = {                            // {20130519-ABCD-1234-0000-201305190317}
    0x7B, 0x32, 0x30, 0x31, 0x33, 0x30, 0x35, 0x31,      // L"SETBDPV" Guid  
    0x39, 0x2D, 0x41, 0x42, 0x43, 0x44, 0x2D, 0x31,
    0x32, 0x33, 0x34, 0x2D, 0x30, 0x30, 0x30, 0x30,
    0x2D, 0x32, 0x30, 0x31, 0x33, 0x30, 0x35, 0x31,
    0x39, 0x30, 0x33, 0x31, 0x37, 0x7D, 0x00, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetBDPVGuid, 40);
  CompalGlobalNvsArea->StatusData = SwSmiSuccess;

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBootDeviceNAME(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetBDPVName[] = {                            // L"SETBDPV"
    0x53, 0x45, 0x54, 0x42, 0x44, 0x50, 0x56, 0x00, 0x00};

  EFI_STATUS    Status = EFI_SUCCESS;
  UINTN         BufferSize;

  BufferSize = sizeof (SYSTEM_CONFIGURATION);
  SetupVariable->Win8FastBoot = 0x01;                    // 0:Enabled, 1=Disabled
  Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetBDPVName, 9);
  CompalGlobalNvsArea->StatusData = SwSmiSuccess;

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootDeviceValue(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetBDPVValue[] = {                            // 0x01 = USB
                                                          // 0x02 = CD
                                                          // 0x03 = PXE
                                                          // 0xFE = Change Boot order to default.

    0xFE, 0x00, 
    0x00, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetBDPVValue, 4);
  CompalGlobalNvsArea->StatusData = SwSmiSuccess;

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCLoadBiosDefualt(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status;

    //
    // Delete Variable
    //
    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 0,
                 NULL
             );
    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCLoadDefualtGUID(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetupGuid[] = {                             // {a04a27f4-df00-4d42-b552-39511302113d}
    0x7B, 0x61, 0x30, 0x34, 0x61, 0x32, 0x37, 0x66,     // L"Setup" Guid
    0x34, 0x2D, 0x64, 0x66, 0x30, 0x30, 0x2D, 0x34,
    0x64, 0x34, 0x32, 0x2D, 0x62, 0x35, 0x35, 0x32,
    0x2D, 0x33, 0x39, 0x35, 0x31, 0x31, 0x33, 0x30,
    0x32, 0x31, 0x31, 0x33, 0x64, 0x7D, 0x00, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetupGuid, 40);
  CompalGlobalNvsArea->StatusData = SwSmiSuccess;

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCLoadDefualtName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetupName[] = {                            // L"Setup"
    0x53, 0x65, 0x74, 0x75, 0x70, 0x00, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetupName, 7);
  CompalGlobalNvsArea->StatusData = SwSmiSuccess;

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetMFGReservedArea(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT16        MFGAreaOffset;
  UINT16        MFGAreaSize;
  UINT8         Index;

  MFGAreaOffset = CompalGlobalNvsArea->BufferData[0] + (CompalGlobalNvsArea->BufferData[1] << 8);
  MFGAreaSize = CompalGlobalNvsArea->BufferData[2] + (CompalGlobalNvsArea->BufferData[3] << 8);

  for ( Index = 0 ; Index < MFGAreaSize ; Index++ ) {
      CompalGlobalNvsArea->BufferData[Index] = *((volatile UINT8*)((UINTN)(FixedPcdGet32 (PcdFlashOemICTProcessBase) + MFGAreaOffset + Index)));
  }

  CompalGlobalNvsArea->StatusData = SwSmiSuccess;

  return EFI_SUCCESS;
}


EFI_STATUS
CMFCSetBootTypeCsm(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status = EFI_SUCCESS;
    UINTN                                BufferSize;

    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    SetupVariable->BootTypeReserved = BootToCSM;
    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );

    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootTypeUEFI(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status = EFI_SUCCESS;
    UINTN                                BufferSize;

    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    SetupVariable->BootTypeReserved = BootToUEFI;
    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );

    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootTypeWPEAndPXEB(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootTypeWPEAndNetB(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootTypeDUAL(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status = EFI_SUCCESS;
    UINTN                                BufferSize;

    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    SetupVariable->BootTypeReserved = BootToDUAL;
    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );

    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetPxeBootToLanEnabled(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{	
    CompalECEepromWriteByte(OEM_EEPROM_BOOT_TYPE_OFFSET, OEM_EEPROM_PXE_BOOT_TO_LAN_ENABLED);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetSmtDefault(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{	
    CompalECEepromWriteByte(OEM_EEPROM_BOOT_TYPE_OFFSET, OEM_EEPROM_SET_SMT_DEFAULT);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetBootType(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{	
    CompalECEepromWriteByte(OEM_EEPROM_BOOT_TYPE_OFFSET, CompalGlobalNvsArea->BufferData[0]);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBootType(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status = EFI_SUCCESS;
    UINTN                                BufferSize;

    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    Status = SmmVariable->SmmGetVariable (
                 L"Setup",
                 &GuidId,
                 NULL,
                 &BufferSize,
                 (VOID *)SetupVariable
             );

    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->BufferData[0] = SetupVariable->BootTypeReserved;
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetSecureBootDisable(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status = EFI_SUCCESS;
    UINTN                                BufferSize;
    UINT8                                SecureBootEnable;

    SecureBootEnable = TRUE;
    Status = SmmVariable->SmmSetVariable (
                 L"AdministerSecureBoot",
                 &gEfiGenericVariableGuid,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 1,
                 &SecureBootEnable
             );
    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    SetupVariable->CompalSecureBoot = 1;
    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );

    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetSecureBootEnable(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status = EFI_SUCCESS;
    UINTN                                BufferSize;
    UINT8                                SecureBootEnable;

    SecureBootEnable = TRUE;
    Status = SmmVariable->SmmSetVariable (
                 L"AdministerSecureBoot",
                 &gEfiGenericVariableGuid,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 1,
                 &SecureBootEnable
             );
    BufferSize = sizeof (SYSTEM_CONFIGURATION);
    SetupVariable->CompalSecureBoot = 2;
    Status = SmmVariable->SmmSetVariable (
                 L"Setup",
                 &GuidId,
                 EFI_VARIABLE_NON_VOLATILE |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS |
                 EFI_VARIABLE_RUNTIME_ACCESS,
                 BufferSize,
                 (VOID *)SetupVariable
             );

    if (EFI_ERROR (Status)) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return EFI_SUCCESS;
}


EFI_STATUS
CMFCGetSecureBootGUID(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetupGuid[] = {                             // {a04a27f4-df00-4d42-b552-39511302113d}
    0x7B, 0x61, 0x30, 0x34, 0x61, 0x32, 0x37, 0x66,     // L"Setup" Guid
    0x34, 0x2D, 0x64, 0x66, 0x30, 0x30, 0x2D, 0x34,
    0x64, 0x34, 0x32, 0x2D, 0x62, 0x35, 0x35, 0x32,
    0x2D, 0x33, 0x39, 0x35, 0x31, 0x31, 0x33, 0x30,
    0x32, 0x31, 0x31, 0x33, 0x64, 0x7D, 0x00, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetupGuid, 40);

//[PRJ] Start - Change to CMFCSetSecureBootEnable, CMFCSetSecureBootDisable function
//CompalGlobalNvsArea->StatusData = SwSmiSuccess;
  CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
//[PRJ] End

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetSecureBootNAME(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetupName[] = {                            // L"Setup"
    0x53, 0x65, 0x74, 0x75, 0x70, 0x00, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetupName, 7);

 //[PRJ] Start - Change to CMFCSetSecureBootEnable, CMFCSetSecureBootDisable function
//CompalGlobalNvsArea->StatusData = SwSmiSuccess;
  CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
//[PRJ] End

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetSecureBootValue(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  UINT8 mEfiSetSecureValue[] = {                          // example: A01SecureBoot offset=535, 0x217
    0x17, 0x02, 
    0x01, 0x00
  };

  Memcpy ( &CompalGlobalNvsArea->BufferData[0], &mEfiSetSecureValue, 4);

//[PRJ] Start - Change to CMFCSetSecureBootEnable, CMFCSetSecureBootDisable function
//CompalGlobalNvsArea->StatusData = SwSmiSuccess;
  CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
//[PRJ] End

  return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetSecureBoot(
IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    EFI_STATUS                           Status;
    UINTN                                BufferSize;
    UINT8                                Data;

    Data = TRUE;
    BufferSize = sizeof (UINT8);
    Status = SmmVariable->SmmGetVariable (

                 L"SecureBoot",
                 &gEfiGlobalVariableGuid,
                 NULL,
                 &BufferSize,
                 &Data
             );

    if ( EFI_ERROR(Status) ) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;
        return Status;
    } else {
        CompalGlobalNvsArea->BufferData[0] = Data;
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeNonOS(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_NonOS);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeLinux(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_Linux);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeWinXP(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_WinXP);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeVista(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_Vista);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeWin7(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_Win7);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeWin8(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_Win8);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}


EFI_STATUS
CMFCSetOSTypeWin8Pro(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_Win8Pro);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSTypeWin7Win8(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, OEM_EEPROM_OSTYPE_Win7Win8);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetOSType(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECEepromReadByte(OEM_EEPROM_OSTYPE_FLAG);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetOSType(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalECEepromWriteByte(OEM_EEPROM_OSTYPE_FLAG, CompalGlobalNvsArea->BufferData[0]);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetVendorName(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
/*++
Returns:

  EFI_STATUS              - Successfully completed.
  Vendor Code             - 
                            COMPAL :0x0001
                            DELL   :0x0002
                            HP     :0x0003
                            TOSHIBA:0x0004
                            ACER   :0x0005
                            LENOVO :0x0006, 0x0007
                            NOKIA  :0x0008
                            NEC:   :0x0009
                            MOTION :0x000A
                            ASUS   :0x000B
                            LG     :0x000C
                            SAMSUNG:0x000D
                            SHARP  :0x000E
                            UNKNOW :0x00FF
--*/
{
    CompalGlobalNvsArea->BufferData[0] = 0xFF;
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCClearSupervisorPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCClearUserPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCCheckSupervisorPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
  CompalGlobalNvsArea->StatusData = SwSmiSuccess;
  return EFI_SUCCESS;
}

EFI_STATUS
CMFCCheckUserPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCCheckHDPassword(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetACConnectStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    UINT8 DCPresent;

    DCPresent=CompalECACPINVSReadByte(EC_NAME_BAT_CONNECT_OFFSET);
    if(DCPresent & 0x01) {
      CompalGlobalNvsArea->BufferData[0] |= 0x01;
    }

    DCPresent=CompalECACPINVSReadByte(EC_NAME_AC_CONNECT_OFFSET);
    if(DCPresent & 0x80) {
      CompalGlobalNvsArea->BufferData[1] |= 0x01;
    }

    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_INFO_OFFSET);

    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatConnectStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    UINT8 DCPresent;

    DCPresent=CompalECACPINVSReadByte(EC_NAME_BAT_CONNECT_OFFSET);
    if(DCPresent & 0x01) {
      CompalGlobalNvsArea->BufferData[0] |= 0x01;
    }

    if(DCPresent & 0x02) {
      CompalGlobalNvsArea->BufferData[0] |= 0x01;
    }
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatDesignCapacity(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_DESIGN_CAPACITY_LOW);
    CompalGlobalNvsArea->BufferData[1] = CompalECACPINVSReadByte(EC_NAME_BAT_DESIGN_CAPACITY_HIGH); 
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatFullChargeCapacity(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_FULL_CHARGE_CAPACITY_LOW);
    CompalGlobalNvsArea->BufferData[1] = CompalECACPINVSReadByte(EC_NAME_BAT_FULL_CHARGE_CAPACITY_HIGH); 
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatRemainingCapacity(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_REMAINING_CAPACITY_LOW);
    CompalGlobalNvsArea->BufferData[1] = CompalECACPINVSReadByte(EC_NAME_BAT_REMAINING_CAPACITY_HIGH);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatChargeCurrent(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_CHARGE_CURRENT_LOW);
    CompalGlobalNvsArea->BufferData[1] = CompalECACPINVSReadByte(EC_NAME_BAT_CHARGE_CURRENT_HIGH);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatCurrent(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_CURRENT_LOW);
    CompalGlobalNvsArea->BufferData[1] = CompalECACPINVSReadByte(EC_NAME_BAT_CURRENT_HIGH);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatCycleCounter(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_CYCLE_COUNTER);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetBatAverageTemperature(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->BufferData[0] = CompalECACPINVSReadByte(EC_NAME_BAT_AVERAGE_TEMPERATURE);
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetACPowerSource(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCShippingModeEnable(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSystemFullChargeLEDOn(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSystemChargeLEDOn(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCAllLEDOn(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCLEDOutOfTestMode(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCSetLEDOnOffBlink(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCGetLEDStatus(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA31Func90(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case A31_SET_COLD_BOOT:
          break;

        case A31_SET_WARM_BOOT:
          break;

        case A31_ERASE_EEPROM:
          break;

        case A31_GET_COMPUTRACE_STATUS:
          break;

        case A31_ENABLE_COMPUTRACE:
          break;

        case A31_DISABLE_COMPUTRACE:
          break;

        case A31_RESTORE_COMPUTRACE:
          break;

        case A31_DETECT_TPM:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA31Func91(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA31GetEDIDOrPN(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCABOFunc94(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case ABO_GET_CPU_VPRO_STATUS:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCABOFunc95(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA51Func96(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA51Func97(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA32Func92(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case A32_WRITE_EEPROM:
          break;

        case A32_READ_EEPROM:
          break;

        case A32_DISABLE_PXE_BOOT:

          break;

        case A32_ENABLE_PXE_BOOT:

          break;

        case A32_DISABLE_ISCT_BOOT:

          break;

        case A32_ENABLE_ISCT_BOOT:

          break;

        case A32_DISABLE_IRST_BOOT:
          break;

        case A32_ENABLE_IRST_BOOT:
          break;

        case A32_DISABLE_LEGACY_SUPPORT:

          break;

        case A32_ENABLE_LEGACY_SUPPORT:

          break;

        case A32_RELAD_DEFAULT:

          break;

        case A32_WRITE_FACTORY_MODE:

          break;

        case A32_SET_ALL_WIRELESS_ON:

          break;

        case A32_HDD_BOOT_FIRST:

          break;

        case A32_LAN_BOOT_FIRST:

          break;

        case A32_CLEAR_ERROR_LOG:

          break;

        case A32_PCID_VERSION:

          break;

        case A32_ERROR_LOG_BASE_ADDRESS:

          break;

        case A32_CMFC_VERSION:

          break;

        case A32_VGA_TYPE:

          break;

        case A32_PSENSOR_STATUS:

          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCA32Func93(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCC38Func98(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case C38_GET_EC_VERSION:
          break;

        case C38_GET_EC_REVISION_NUMBER:
          break;

        case C38_CLEAR_BATT_FIRST_USING_TIME:

          break;

        case C38_CTRL_BATT_CAP_TO_ARG2_DEFINE:

          break;

        case C38_DISABLE_CTRL_BATT_CAPACITY:

          break;

        case C38_ENABLE_VIBRATOR:

          break;

        case C38_DISABLE_VIBRATOR:
          break;

        case C38_ENABLE_USB_20Y_CABLE:
          break;

        case C38_DISABLE_USB_20Y_CABLE:

          break;

        case C38_GET_1ST_PSENSOR_STATUS:

          break;

        case C38_GET_2ND_PSENSOR_STATUS:

          break;

        case C38_GET_DOCKING_STATUS:

          break;

        case C38_GET_PANEL_TYPE:

          break;

        case C38_GET_GUID_OF_NFC:

          break;

        case C38_GET_GUID_OF_ROLLBACK:

          break;

        case C38_GET_GUID_OF_MTM:

          break;

        case C38_GET_GUID_OF_OS_LICENSE:

          break;

        case C38_GET_VARIABLE_NAME_OF_NFC:

          break;

        case C38_GET_VARIABLE_NAME_OF_ROLLBACK:

          break;

        case C38_GET_VARIABLE_NAME_OF_MTM:

          break;

        case C38_GET_VARIABLE_NAME_OF_OS_LICENSE:

          break;

        case C38_RET_BIT_OFFSET_OF_DIGITIZER:

          break;

        case C38_RET_BIT_OFFSET_OF_NFC:

          break;

        case C38_RET_BIT_OFFSET_OF_FINGERPRINT:

          break;

        case C38_RET_BIT_OFFSET_OF_AUTO_DETECT:

          break;

        case C38_RET_BYTE_OFFSET_OF_ROLLBACK:

          break;

        case C38_DISABLE_OS_OPTIMIZED_DEFAULT:

          break;

        case C38_ENABLE_OS_OPTIMIZED_DEFAULT:

          break;

        case C38_GET_OS_OPTIMIZED_DEFAULT:

          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

EFI_STATUS
CMFCC38Func99(
    IN OUT COMPAL_GLOBAL_NVS_AREA *CompalGlobalNvsArea
)
{
    CompalGlobalNvsArea->StatusData = SwSmiSuccess;

    switch(CompalGlobalNvsArea->SubFunctionNo) {
        case 0x01:
          break;

        default:
          CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
          break;
    } 

    return EFI_SUCCESS;
}

VOID
Memcpy (
  IN VOID   *Destination,
  IN VOID   *Source,
  IN UINTN  Length
  )

{
  CHAR8 *Destination8;
  CHAR8 *Source8;

  Destination8 = Destination;
  Source8 = Source;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }
}