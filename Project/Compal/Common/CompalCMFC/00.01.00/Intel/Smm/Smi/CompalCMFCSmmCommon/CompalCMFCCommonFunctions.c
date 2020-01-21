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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#include "CompalCMFCCommonFunctions.h"

extern EFI_SMM_SYSTEM_TABLE2             *mSmst;
extern COMPAL_GLOBAL_NVS_AREA            *CompalGlobalNvsArea;
extern COMPAL_CMFC_SMM_HOOK_PROTOCOL     *CompalCMFCSmmHookProtocol;
extern EFI_SMM_VARIABLE_PROTOCOL         *SmmVariable;

#if COMPAL_S35_SUPPORT
EFI_GUID                                 S35VarGuid  = S35_VARIABLE_GUID;
#endif

VOID
CompalCMFCFunctionEntry (
    FunctionBuffer			FB
)
{
    UINTN index;
    BOOLEAN found=FALSE;
    void (*fp)();

    CMFC_FUNCTIONS CMFC_FUNCTIONS_Table[] = {
        {0x11,0x00,ReturnVersion},
        {0x12,0x01,GetManufacturTypeId},
        {0x12,0x02,GetPanelMax},
        {0x12,0x03,GetEDIDFullData},
        {0x13,0x01,SetBootDeviceSequence},
        {0x13,0x02,GetBootDeviceSequence},

        {0x13,0x03,GetBootDeviceGUID},
        {0x13,0x04,GetBootDeviceNAME},
        {0x13,0x05,SetBootDeviceValue},

        {0x14,0x00,OemBIOSLoadDefault},
        {0x14,0x01,OemBIOSDefaultGUID},
        {0x14,0x02,OemBIOSDefaultNAME},

        {0x15,0x00,ECCommunicateInterface},
#if COMPAL_S35_SUPPORT
        {0x16,0x00,S35APInstalled},
        {0x16,0x01,S35Enable},
        {0x16,0x02,S35Disable},
        {0x16,0x03,GetS35Status},
#endif

        {0x17,0x01,ClearSupervisorPassword},
        {0x17,0x02,ClearUserPassword},
        {0x17,0x03,CheckSupervisorPassword},
        {0x17,0x04,CheckUserPassword},
        {0x17,0x05,CheckHDDPassword},

        {0x20,0x01,SetBootTypeCsm},
        {0x20,0x02,SetBootTypeUEFI},
        {0x20,0x03,SetBootTypeWPEAndPXEB},
        {0x20,0x04,SetBootTypeWPEAndNetB},
        {0x20,0x05,SetBootTypeDUAL},
        {0x20,0x06,SetPxeBootToLanEnabled},
        {0x20,0x07,SetSmtDefault},
        {0x20,0x10,GetBootTypeStatus},
        {0x21,0x01,SecureBootDisable},
        {0x21,0x02,SecureBootEnable},

        {0x21,0x03,GetSecureBootGUID},
        {0x21,0x04,GetSecureBootNAME},
        {0x21,0x05,GetSecureBootValue},

        {0x21,0x10,GetSecureBootStatus},
        {0x22,0x01,SetOSTypeNonOS},
        {0x22,0x02,SetOSTypeLinux},
        {0x22,0x03,SetOSTypeWinXP},
        {0x22,0x04,SetOSTypeVista},
        {0x22,0x05,SetOSTypeWin7},
        {0x22,0x06,SetOSTypeWin8},
        {0x22,0x07,SetOSTypeWin8Pro},
        {0x22,0x08,SetOSTypeWin7Win8},
        {0x22,0x10,GetOSTypeStatus},
        {0x23,0x10,GetVendorName},

        {0x26,0x01,GetACConnectStatus},
        {0x26,0x02,GetBatStatus},
        {0x26,0x03,GetBatConnectStatus},
        {0x26,0x04,GetBatDesignCapacity},
        {0x26,0x05,GetBatFullChargeCapacity},
        {0x26,0x06,GetBatRemainingCapacity},
        {0x26,0x07,GetBatChargeCurrent},
        {0x26,0x08,GetBatCurrent},
        {0x26,0x09,GetBatCycleCounter},
        {0x26,0x0A,GetBatAverageTemperature},

        {0x64,0x01,DisableSPIRegionProtect},
        {0x64,0x02,EnableSPIRegionProtect},
        {0xFF,0xFF,NULL}
    };

    index = 0;
    while (CMFC_FUNCTIONS_Table[index].cbFunNo != 0xFF)
    {
        if (FB.cbFunNo == CMFC_FUNCTIONS_Table[index].cbFunNo)
        {
            if (FB.cbFunNo == 0x11 || FB.cbFunNo == 0x15)                         // sub-function did not have cbSubFunNo
            {
                found=TRUE;
                break;
            }
            if (FB.cbSubFunNo == CMFC_FUNCTIONS_Table[index].cbSubFunNo)
            {
                found=TRUE;
                break;
            }
        }
        index++;
    }//end of while

    // Func_Matched
    if (found)
    {
        fp = CMFC_FUNCTIONS_Table[index].FuncPtr;
        (*fp)();
    }
}

//
// Function No 0x11
//
VOID
ReturnVersion (
    VOID
)
{
    UINT8 SmiVersion = 0 ;

    SmiVersion = SSMIVersion;
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        IoWrite8(SW_SMI_IO_DATA, SmiVersion);
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        CompalGlobalNvsArea->BufferData[0] = SmiVersion;
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
    return;
}

//
// Function No 0x12
//
VOID
GetManufacturTypeId (
    VOID
)
{
    UINT8    i;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        for (i=0; i<4; i++) {
            CompalGlobalNvsArea->BufferData[i] =  CompalGlobalNvsArea->EDIDFullData[EDID_MANUFACTURE_NAME_OFFSET+i];
        }
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
}

VOID
GetPanelMax (
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        CompalGlobalNvsArea->BufferData[0] =  CompalGlobalNvsArea->EDIDFullData[EDID_DTD1_HORIZONTAL_ACTIVE_OFFSET];
        CompalGlobalNvsArea->BufferData[1] =  CompalGlobalNvsArea->EDIDFullData[EDID_DTD1_HORIZONTAL_ACTIVE_BLANKING_OFFSET];
        CompalGlobalNvsArea->BufferData[2] =  CompalGlobalNvsArea->EDIDFullData[EDID_DTD1_Vertical_Active_OFFSET];
        CompalGlobalNvsArea->BufferData[3] =  CompalGlobalNvsArea->EDIDFullData[EDID_DTD1_VERTICAL_ACTIVE_BLANKING_OFFSET];
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
}

VOID
GetEDIDFullData (
    VOID
)
{
    UINT8                     i;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        for (i=0; i<128; i++) {
            CompalGlobalNvsArea->BufferData[i] =  CompalGlobalNvsArea->EDIDFullData[i];
        }
        CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    }
}

//
// Function No 0x13
//
VOID
SetBootDeviceSequence (
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_SET_BOOT_DEVICE_SEQUENCE,
            CompalGlobalNvsArea
        );
    }
}

VOID
GetBootDeviceSequence (
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_GET_BOOT_DEVICE_SEQUENCE,
            CompalGlobalNvsArea
        );
    }
}

VOID
GetBootDeviceGUID (
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_GET_BOOT_DEVICE_GUID,
            CompalGlobalNvsArea
        );
    }
}

VOID
GetBootDeviceNAME (
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_GET_BOOT_DEVICE_NAME,
            CompalGlobalNvsArea
        );
    }
}

VOID
SetBootDeviceValue (
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_SET_BOOT_DEVICE_VALUE,
            CompalGlobalNvsArea
        );
    }
}

//
// Function index 0x14
//
EFI_STATUS
OemBIOSLoadDefault(
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_BIOS_LOAD_DEFUALT,
            CompalGlobalNvsArea
        );
    }
    return EFI_SUCCESS;
}

EFI_STATUS
OemBIOSDefaultGUID(
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_BIOS_DEFUALT_GUID,
            CompalGlobalNvsArea
        );
    }
    return EFI_SUCCESS;
}

EFI_STATUS
OemBIOSDefaultNAME(
    VOID
)
{
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
            CompalCMFCSmmHookProtocol,
            CMFC_BIOS_DEFUALT_NAME,
            CompalGlobalNvsArea
        );
    }
    return EFI_SUCCESS;
}

//
// Function index 0x15
//
VOID
ECCommunicateInterface (
    VOID
)
{
    UINT8       EC_CMD,EC_DATA_LEN,EC_RTN_DATA_LEN=0,EC_DATA_OF_CMD_PART[8],BUFFER_INDEX,CLEAR_BUFFER_COUNT=0;
    UINT8       SmiVersion = 0 ;

    SmiVersion = SSMIVersion;
    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    } else {
        EC_CMD = (UINT8)(0x00FF) & (CompalGlobalNvsArea->SubFunctionNo);
        EC_DATA_LEN = CompalGlobalNvsArea->BufferData[0];

        if (((UINT8)((CompalGlobalNvsArea->SubFunctionNo)>>8) & 0xFF) == 0x80)
        {
            EC_RTN_DATA_LEN = CompalGlobalNvsArea->BufferData[1];
            for (BUFFER_INDEX=0 ; BUFFER_INDEX < EC_DATA_LEN ; BUFFER_INDEX++)
            {
                EC_DATA_OF_CMD_PART[BUFFER_INDEX] = CompalGlobalNvsArea->BufferData[2+BUFFER_INDEX];
            }
            while ((CompalGlobalNvsArea->BufferData[2] !=0x00) || CLEAR_BUFFER_COUNT < 128)
            {
                CompalGlobalNvsArea->BufferData[2+CLEAR_BUFFER_COUNT] = 0x00;
                CLEAR_BUFFER_COUNT++;
            }
            CompalECReadCmd(EC_INDEXIO_PORT,EC_CMD,EC_DATA_LEN,EC_RTN_DATA_LEN,&EC_DATA_OF_CMD_PART[0]);
            for (BUFFER_INDEX=0 ; BUFFER_INDEX < EC_RTN_DATA_LEN ; BUFFER_INDEX++)
            {
                CompalGlobalNvsArea->BufferData[2+BUFFER_INDEX] = EC_DATA_OF_CMD_PART[BUFFER_INDEX];
            }

            CompalGlobalNvsArea->StatusData = SwSmiSuccess;
        }
        else if (((UINT8)((CompalGlobalNvsArea->SubFunctionNo)>>8) & 0xFF) == 0x00)
        {
            for (BUFFER_INDEX=0 ; BUFFER_INDEX < EC_DATA_LEN ; BUFFER_INDEX++)
            {
                EC_DATA_OF_CMD_PART[BUFFER_INDEX] = CompalGlobalNvsArea->BufferData[2+BUFFER_INDEX];
            }
            while ((CompalGlobalNvsArea->BufferData[2] !=0x00) || CLEAR_BUFFER_COUNT < 128)
            {
                CompalGlobalNvsArea->BufferData[2+CLEAR_BUFFER_COUNT] = 0x00;
                CLEAR_BUFFER_COUNT++;
            }
            CompalECWriteCmd(EC_INDEXIO_PORT,EC_CMD,EC_DATA_LEN,&EC_DATA_OF_CMD_PART[0]);
            CompalGlobalNvsArea->StatusData = SwSmiSuccess;
        }
    }
}

#if COMPAL_S35_SUPPORT

VOID
S35APInstalled(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x16
;
; Sub function : 0x00
;
; Procedure : S35APInstalled
;
; Description : S3.5 AP Installed
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;
    UINT32                               Attributes;
    UINTN                                BufferSize;
    S35_VARIABLE                         mS35Variable;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if ( CompalGlobalNvsArea->CMFCAPVersion == 0 ) {
        return;
    } else {
        CompalS35Port80( 0x00 );
        BufferSize = sizeof (S35_VARIABLE);
        Status = SmmVariable->GetVariable (
                     S35_VARIABLE_NAME,
                     &S35VarGuid,
                     &Attributes,
                     &BufferSize,
                     &mS35Variable
                 );

        if ( EFI_ERROR(Status) ) {
            return;
        } else {

            mS35Variable.S35APEnableFlag = 0x01;
            Status = SmmVariable->SetVariable (
                         S35_VARIABLE_NAME,
                         &S35VarGuid,
                         EFI_VARIABLE_NON_VOLATILE |
                         EFI_VARIABLE_BOOTSERVICE_ACCESS |
                         EFI_VARIABLE_RUNTIME_ACCESS,
                         sizeof (S35_VARIABLE),
                         &mS35Variable
                     );
            if ( EFI_ERROR(Status) ) {
                return;
            } else {
                CompalGlobalNvsArea->StatusData = SwSmiSuccess;
            }
        }
    }
}

VOID
S35Enable(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x16
;
; Sub function : 0x01
;
; Procedure : S35Enable
;
; Description : Enable S3.5 Function
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;
    UINT8                                Data8;
    UINT32                               Attributes;
    UINTN                                BufferSize;
    S35_VARIABLE                         mS35Variable;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {
        CompalS35Port80( 0x01 );
        BufferSize = sizeof (S35_VARIABLE);

        Status = SmmVariable->GetVariable (
                     S35_VARIABLE_NAME,
                     &S35VarGuid,
                     &Attributes,
                     &BufferSize,
                     &mS35Variable
                 );
        if ( EFI_ERROR(Status) ) {
            return;
        }

        mS35Variable.S35EnableFlag = 0x01;
        Status = SmmVariable->SetVariable (
                     S35_VARIABLE_NAME,
                     &S35VarGuid,
                     EFI_VARIABLE_NON_VOLATILE |
                     EFI_VARIABLE_BOOTSERVICE_ACCESS |
                     EFI_VARIABLE_RUNTIME_ACCESS,
                     sizeof (S35_VARIABLE),
                     &mS35Variable
                 );
        if ( EFI_ERROR(Status) ) {
            return;
        } else {
            CompalECReadENEData( 0xBA, 0xF4, &Data8 );
            Data8 |= (1 << 1);
            CompalECWriteENEData( 0xBA, 0xF4, Data8 );
            CompalGlobalNvsArea->StatusData = SwSmiSuccess;
        }
    }
}

VOID
S35Disable(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x16
;
; Sub function : 0x02
;
; Procedure : S35Disable
;
; Description : Disable S3.5 Function
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;
    UINT8                                Data8;
    UINT32                               Attributes;
    UINTN                                BufferSize;
    S35_VARIABLE                         mS35Variable;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if ( CompalGlobalNvsArea->CMFCAPVersion == 0 ) {
        return;
    } else {
        CompalS35Port80( 0x02 );
        BufferSize = sizeof (S35_VARIABLE);
        Status = SmmVariable->GetVariable (
                     S35_VARIABLE_NAME,
                     &S35VarGuid,
                     &Attributes,
                     &BufferSize,
                     &mS35Variable
                 );

        if ( EFI_ERROR(Status) ) {
            return;
        }

        mS35Variable.S35EnableFlag = 0x00;
        Status = SmmVariable->SetVariable (
                     S35_VARIABLE_NAME,
                     &S35VarGuid,
                     EFI_VARIABLE_NON_VOLATILE |
                     EFI_VARIABLE_BOOTSERVICE_ACCESS |
                     EFI_VARIABLE_RUNTIME_ACCESS,
                     sizeof (S35_VARIABLE),
                     &mS35Variable
                 );

        if ( EFI_ERROR(Status) ) {
            return;
        } else {
            CompalECReadENEData( 0xBA, 0xF4, &Data8 );
            Data8 &= ~(1 << 1);
            CompalECWriteENEData( 0xBA, 0xF4, Data8 );
            CompalGlobalNvsArea->StatusData = SwSmiSuccess;
        }
    }
}

VOID
GetS35Status(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x16
;
; Sub function : 0x03
;
; Procedure : GetS35Status
;
; Description : Get S3.5 Stauts
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;
    UINT8                                Data8;
    UINT32                               Attributes;
    UINTN                                BufferSize;
    S35_VARIABLE                         mS35Variable;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if ( CompalGlobalNvsArea->CMFCAPVersion == 0 ) {
        return;
    } else {
        CompalS35Port80( 0x03 );
        BufferSize = sizeof (S35_VARIABLE);
        Status = SmmVariable->GetVariable (
                     S35_VARIABLE_NAME,
                     &S35VarGuid,
                     &Attributes,
                     &BufferSize,
                     &mS35Variable
                 );

        if ( EFI_ERROR(Status) ) {
            return;
        } else {
            CompalGlobalNvsArea->BufferData[0] = mS35Variable.S35EnableFlag;
            CompalGlobalNvsArea->BufferData[1] = mS35Variable.S35PartitionValid;
            CompalGlobalNvsArea->StatusData = SwSmiSuccess;

            CompalECReadENEData( 0xBA, 0xF4, &Data8 );
            Data8 &= ~(1 << 1);
            Data8 |= mS35Variable.S35EnableFlag << 1;
            CompalECWriteENEData( 0xBA, 0xF4, Data8 );
        }
    }
}
#endif

VOID
ClearSupervisorPassword(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x17
;
; Sub function : 0x01
;
; Procedure : ClearSupervisorpassword
;
; Description : Clear Supervisor password
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_CLEAR_SUPERVISOR_PASSWORD,
                     CompalGlobalNvsArea
                 );


        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
ClearUserPassword(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x17
;
; Sub function : 0x02
;
; Procedure : ClearUserpassword
;
; Description : Clear User password
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_CLEAR_USER_PASSWORD,
                     CompalGlobalNvsArea
                 );


        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
CheckSupervisorPassword(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x17
;
; Sub function : 0x03
;
; Procedure : CheckSupervisorpassword
;
; Description : Check Supervisor password
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {
        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_CHECK_SUPERVISOR_PASSWORD,
                     CompalGlobalNvsArea
                 );
        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }
    }
}

VOID
CheckUserPassword(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x17
;
; Sub function : 0x04
;
; Procedure : CheckUserpassword
;
; Description : Check User password
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {
        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_CHECK_USER_PASSWORD,
                     CompalGlobalNvsArea
                 );
        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }
    }
}

VOID
CheckHDDPassword(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x17
;
; Sub function : 0x05
;
; Procedure : CheckHDDpassword
;
; Description : Check HDD password
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {
        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_CHECK_HD_PASSWORD,
                     CompalGlobalNvsArea
                 );
        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }
    }
}

VOID
SetBootTypeCsm(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x01
;
; Procedure : SetBootTypeCsm
;
; Description : Set Boot Type to Legacy boot
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_BOOT_TYPE_Csm,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetBootTypeUEFI(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x02
;
; Procedure : SetBootTypeUEFI
;
; Description : Set Boot Type to UEFI
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_BOOT_TYPE_UEFI,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetBootTypeWPEAndPXEB(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x03
;
; Procedure : SetBootTypeWPEAndPXEB
;
; Description : Set Boot Type to Legacy boot + PXE boot first
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                 CompalCMFCSmmHookProtocol,
                 CMFC_BOOT_TYPE_WPEAndPXEB,
                 CompalGlobalNvsArea
             );

    if (Status != EFI_SUCCESS) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    }
}

VOID
SetBootTypeWPEAndNetB(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x04
;
; Procedure : SetBootTypeWPEAndNetB
;
; Description : Set Boot Type to UEFI boot + EFI Network boot first
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                 CompalCMFCSmmHookProtocol,
                 CMFC_BOOT_TYPE_WPEAndNetB,
                 CompalGlobalNvsArea
             );

    if (Status != EFI_SUCCESS) {
        CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
        return;
    }
}


VOID
SetBootTypeDUAL(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x05
;
; Procedure : SetBootTypeDual
;
; Description : Set Boot Type to Dual
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = 0x0000;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                 CompalCMFCSmmHookProtocol,
                 CMFC_BOOT_TYPE_DUAL,
                 CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = 0x0000;
            return;
        }
    }
}

VOID
SetPxeBootToLanEnabled(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x06
;
; Procedure : SetPxeBootToLanEnabled
;
; Description : Set PXE BOOT to LAN Enabled
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = 0x0000;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                 CompalCMFCSmmHookProtocol,
                 CMFC_PXE_BOOT_TO_LAN_ENABLED,
                 CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = 0x0000;
            return;
        }
    }
}

VOID
SetSmtDefault(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x07
;
; Procedure : SetSmtDefault
;
; Description : Set SMT Default
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = 0x0000;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                 CompalCMFCSmmHookProtocol,
                 CMFC_SET_SMT_DEFAULT,
                 CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = 0x0000;
            return;
        }
    }
}

VOID
GetBootTypeStatus(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x20
;
; Sub function : 0x10
;
; Procedure : GetBootTypeStatus
;
; Description : Get Boot Type Status
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;


    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BOOT_TYPE,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }
    }
}

VOID
SecureBootDisable(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x21
;
; Sub function : 0x01
;
; Procedure : SecureBootDisable
;
; Description : Set Secure Boot State to "Disable"
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_SECURE_BOOT_DIS,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SecureBootEnable(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x21
;
; Sub function : 0x02
;
; Procedure : SecureBootEnable
;
; Description : Set Secure Boot State to "Enable"
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_SECURE_BOOT_ENA,
                     CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetSecureBootGUID (
    VOID
)
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_SECURE_BOOT_GUID,
                     CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetSecureBootNAME (
    VOID
)
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_SECURE_BOOT_NAME,
                     CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetSecureBootValue (
    VOID
)
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_SECURE_BOOT_VALUE,
                     CompalGlobalNvsArea
             );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetSecureBootStatus(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x21
;
; Sub function : 0x10
;
; Procedure : GetSecureBootStatus
;
; Description : Get Secure Boot Status
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_SECURE_BOOT,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetOSTypeNonOS(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x01
;
; Procedure : SetOSTypeNonOS
;
; Description : Set OS Type to NonOS
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_NonOS,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetOSTypeLinux(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x02
;
; Procedure : SetOSTypeLinux
;
; Description : Set OS Type to Linux
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_Linux,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetOSTypeWinXP(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x03
;
; Procedure : SetOSTypeWinXP
;
; Description : Set OS Type to WinXP
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_WinXP,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}
VOID
SetOSTypeVista(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x04
;
; Procedure : SetOSTypeVista
;
; Description : Set OS Type to Vista
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_Vista,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetOSTypeWin7(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x05
;
; Procedure : SetOSTypeWin7
;
; Description : Set OS Type to Win7
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_Win7,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetOSTypeWin8(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x06
;
; Procedure : SetOSTypeWin8
;
; Description : Set OS Type to Win8
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_Win8,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}


VOID
SetOSTypeWin8Pro(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x07
;
; Procedure : SetOSTypeWin8Pro
;
; Description : Set OS Type to Win8Pro
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_Win8Pro,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
SetOSTypeWin7Win8(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x08
;
; Procedure : SetOSTypeWin7Win8
;
; Description : Set OS Type to Win7Win8
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_OS_TYPE_Win7Win8,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetOSTypeStatus(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x22
;
; Sub function : 0x10
;
; Procedure : GetOSTypeStatus
;
; Description : Get OS Type Status
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_OS_TYPE,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetVendorName(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x23
;
; Sub function : 0x10
;
; Procedure : GetVendorName
;
; Description : Get Vendor Name
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_VENDOR_NAME,
                     CompalGlobalNvsArea
                 );

        if (Status != EFI_SUCCESS) {
            CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
            return;
        }

    }
}

VOID
GetACConnectStatus(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x01
;
; Procedure : GetACConnectStatus
;
; Description : Get AC Connect Status
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_AC_CONNECT_STATUS,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatStatus(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x02
;
; Procedure : GetBatStatus
;
; Description : Get Battery Status
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_STATUS,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatConnectStatus(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x03
;
; Procedure : GetBatConnectStatus
;
; Description : Get Battery Connect Status
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_CONNECT_STATUS,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatDesignCapacity(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x04
;
; Procedure : GetBatDesignCapacity
;
; Description : Get Battery Design Capacity
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_DESIGN_CAPACITY,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatFullChargeCapacity(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x05
;
; Procedure : GetBatDesignCapacity
;
; Description : Get Battery Design Capacity
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_FULL_CHARGE_CAPACITY,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatRemainingCapacity(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x06
;
; Procedure : GetBatRemainingCapacity
;
; Description : Get Battery Remaining Capacity
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_REMAINING_CAPACITY,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatChargeCurrent(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x07
;
; Procedure : GetBatChargeCurrent
;
; Description : Get Battery Charge Current
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_CHARGE_CURRENT,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatCurrent(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x08
;
; Procedure : GetBatCurrent
;
; Description : Get Battery Current
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_CURRENT,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatCycleCounter(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x09
;
; Procedure : GetBatCycleCounter
;
; Description : Get Battery Cycle Counter
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_CYCLE_COUNTER,
                     CompalGlobalNvsArea
                 );

    }
}

VOID
GetBatAverageTemperature(
    VOID
)
/*
;----------------------------------------------------------------------------
;
; Function : 0x26
;
; Sub function : 0x0A
;
; Procedure : GetBatAverageTemperature
;
; Description : Get Battery Average Temperature
;
; Input :
;
; Output :
;
; Modified :
;
; Referrals :
;
; Notes :
;
;----------------------------------------------------------------------------
*/
{
    EFI_STATUS                           Status;

    CompalGlobalNvsArea->StatusData = SwSmiNotSupport;
    CompalGlobalNvsArea->BufferData[0] = 0x0000;
    CompalGlobalNvsArea->BufferData[1] = 0x0000;

    if (CompalGlobalNvsArea->CMFCAPVersion == 0) {
        return;
    } else {

        Status = CompalCMFCSmmHookProtocol->CompalCMFCSMMHookDispatch(
                     CompalCMFCSmmHookProtocol,
                     CMFC_GET_BAT_AVERAGE_TEMPERATURE,
                     CompalGlobalNvsArea
                 );

    }
}

//
// Function index 0x64
//
VOID
DisableSPIRegionProtect(
    VOID
)
{
    UINT32  Data;

    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_HSFS,
                            1,
                            &Data );

    Data &= B_PCH_SPI_HSFS_FLOCKDN;

    if (Data != 0) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;   // Not Support SPI Region Protect
        return;
    }

//Read PR0 ,protect region 0 register(Base address of the SPI host interface registers offset 0x74).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR0,
                            1,
                            &Data );
    Data = Data & (~B_PCH_SPI_PR0_WPE); //Disable SPI Write Protection Bit(Bit31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR0,
                            1,
                            &Data );

//Read PR1,protect region 1 register(Base address+0x78).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR1,
                            1,
                            &Data );
    Data = Data & (~B_PCH_SPI_PR1_WPE);//Disable SPI Write Protection Bit(Bit31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR1,
                            1,
                            &Data );

//Read PR2,protect region 2 register(Base address+0x7C).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR2,
                            1,
                            &Data );
    Data = Data & (~B_PCH_SPI_PR2_WPE);//Disable SPI Write Protection Bit(Bit31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR2,
                            1,
                            &Data );

//Read PR3,protect region 3 register(Base address+0x80).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR3,
                            1,
                            &Data );
    Data = Data & (~B_PCH_SPI_PR3_WPE);//Disable SPI Write Protection Bit(Bit31).


    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR3,
                            1,
                            &Data );

//Read PR4,protect region 4 register(Base address+0x84).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR4,
                            1,
                            &Data );
    Data = Data & (~B_PCH_SPI_PR4_WPE);//Disable SPI Write Protection Bit(Bit31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR4,
                            1,
                            &Data );

    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return;
}

VOID
EnableSPIRegionProtect(
    VOID
)
{
    UINT32  Data;

    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_HSFS,
                            1,
                            &Data );

    Data &= B_PCH_SPI_HSFS_FLOCKDN;

    if (Data != 0) {
        CompalGlobalNvsArea->StatusData = SwSmiFail;   // Not Support SPI Region Protect
        return;
    }

//Read PR0 ,protect region 0 register(Base address of the SPI host interface registers offset 0x74).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR0,
                            1,
                            &Data );
    Data = (Data |B_PCH_SPI_PR0_WPE); //Set Write Protection Enable(BIT31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR0,
                            1,
                            &Data );

//Read PR1,protect region 1 register(Base address+0x78).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR1,
                            1,
                            &Data );
    Data = (Data |B_PCH_SPI_PR1_WPE);//Set Write Protection Enable(BIT31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR1,
                            1,
                            &Data );

//Read PR2,protect region 2 register(Base address+0x7C).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR2,
                            1,
                            &Data );
    Data = (Data |B_PCH_SPI_PR2_WPE);//Set Write Protection Enable(BIT31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR2,
                            1,
                            &Data );

//Read PR3,protect region 3 register(Base address+0x80).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR3,
                            1,
                            &Data );
    Data = (Data |B_PCH_SPI_PR3_WPE); //Set Write Protection Enable(BIT31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR3,
                            1,
                            &Data );

//Read PR4,protect region 4 register(Base address+0x84).
    mSmst->SmmIo.Mem.Read ( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR4,
                            1,
                            &Data );
    Data = (Data |B_PCH_SPI_PR4_WPE);//Set Write Protection Enable(BIT31).

    mSmst->SmmIo.Mem.Write( &mSmst->SmmIo,
                            SMM_IO_UINT32,
                            PCH_RCBA_ADDRESS+R_PCH_SPI_PR4,
                            1,
                            &Data );

    CompalGlobalNvsArea->StatusData = SwSmiSuccess;
    return;
}
