/** @file
  Initial and callback functions for Security page

;******************************************************************************
;* Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Security.h"
#include <Protocol/TcgService.h>
#include <Protocol/Tcg2Protocol.h>
#include <Guid/H2OTpm20DtpmPublishAcpiTableDone.h>
#include <IndustryStandard/TcgPhysicalPresence.h>

#pragma pack(1)
typedef struct {
  TPM_RQU_COMMAND_HDR       Header;
  UINT32                    capArea;
  UINT32                    subCapSize;
  UINT32                    subCap;
} TPM_GET_PROP_OWNER_COMMAND;

typedef struct {
  TPM_RSP_COMMAND_HDR       Header;
  UINT32                    respSize;
  UINT8                     resp;
} TPM_GET_PROP_OWNER_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPM_CAP                   Capability;
  UINT32                    Property;
  UINT32                    PropertyCount;
} TPM2_GET_CAPABILITY_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER      Header;
  TPMI_YES_NO               MoreData;
  TPMS_CAPABILITY_DATA      CapabilityData;
} TPM2_GET_CAPABILITY_RESPONSE;
#pragma pack()

typedef struct {
  UINT8                     OperationValue;
  EFI_STRING_ID             OperationStringId;
} TPM_PP_OPERATION_TABLE;

#define TPMA_PERMANENT_OWNER_AUTH_SET       BIT0
#define TPMA_PERMANENT_LOCKOUT_AUTH_SET     BIT2

STATIC CHAR16                      mTcg2VarStoreName[] = L"Tcg2ConfigInfo";
STATIC EFI_GUID                    mTcg2VarStoreGuid = TCG2_CONFIGURATION_INFO_GUID;
STATIC TCG2_CONFIGURATION_INFO     mTcg2ConfigInfo = {0};

STATIC BOOLEAN                     mSupervisorStartFlag = TRUE;
EFI_CALLBACK_INFO                  *mSecurityCallBackInfo;
STATIC TPM_PP_OPERATION_TABLE      mTpm2OperationTable[] = {
                                     TCG2_PHYSICAL_PRESENCE_NO_ACTION,                                    STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_NO_ACTION_STRING),
                                     TCG2_PHYSICAL_PRESENCE_ENABLE,                                       STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_ENABLE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_DISABLE,                                      STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_DISABLE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_DISABLE_ENDORSEMENT_ENABLE_STORAGE_HIERARCHY, STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_DISABLE_ENDORSEMENT_ENABLE_STORAGE_HIERARCHY_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS,                                STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PCR_BANKS_STRING),
                                     TCG2_PHYSICAL_PRESENCE_LOG_ALL_DIGESTS,                              STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_LOG_ALL_DIGESTS_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_TRUE,               STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_TRUE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_FALSE,              STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CLEAR_FALSE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_FALSE,            STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_FALSE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_TRUE,             STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_ON_TRUE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_FALSE,           STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_FALSE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_TRUE,            STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_TURN_OFF_TRUE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_FALSE,        STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_FALSE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_TRUE,         STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_PCRS_TRUE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_FALSE,         STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_FALSE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_TRUE,          STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_TRUE_STRING),
                                     TCG2_PHYSICAL_PRESENCE_CHANGE_EPS,                                   STRING_TOKEN (STR_TCG2_PHYSICAL_PRESENCE_CHANGE_EPS_STRING)
                                   };

STATIC
EFI_STATUS
ValidatePassword (
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL *SysPasswordService,
  IN  BOOLEAN                           SupervisorOrUser,
  IN  SYS_PASSWORD_INFO_DATA            *SysPasswordDataInfo,
  IN  CHAR16                            *Password
  )
{
  EFI_STATUS                         Status;

  if (SysPasswordService == NULL) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //  If callback function return other status, the password flow won't terminate.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  if (SysPasswordDataInfo->Flag == DISABLE_PASSWORD) {
    //
    //  return EFI_SUCCESS to display two password items for new password and confirm new password
    //
    return EFI_SUCCESS;
  }
  //
  // Check Password exists or not.
  //
  Status = SysPasswordService->GetStatus (
                                 SysPasswordService,
                                 SupervisorOrUser
                                 );

  SysPasswordDataInfo->SystemPasswordStatus = Status;

  //
  // Old password already exist
  //
  if (SysPasswordDataInfo->SystemPasswordStatus == EFI_SUCCESS) {
    //
    // Enter old password to confirmation.
    //
    if (SysPasswordDataInfo->Flag == CHANGE_PASSWORD) {
      if (StrCmp(Password, SysPasswordDataInfo->InputString) != 0x00) {
        //
        //  Input password is  NOT  match current set system password
        //  Return EFI_NOT_READY to clean dialog and let browser terminate password flow
        //
        return EFI_NOT_READY;
      } else {
        //
        //  Input password is match current set system password
        //  Return EFI_SUCCESS to display another two password items for new password and confirm new password
        //
        return EFI_SUCCESS;
      }
    }

    //
    //  This check has two purpose:
    //  1. NULL string: indicating that there is a preexisting password
    //  2. User input string: to check current system password
    //
    Status = SysPasswordService->CheckPassword (
                                   SysPasswordService,
                                   Password,
                                   (SysPasswordMaxNumber (SysPasswordService) + 1) * sizeof(UINT16),
                                   SupervisorOrUser
                                   );

    if ((Status != EFI_SUCCESS)) {
      //
      //  1. Check for an existing password by sending a NULL string value.
      //     Returns an error, indicating that there is a preexisting password.
      //

      //
      //  2. Input password is  NOT  match current set system password
      //     Return EFI_NOT_READY to clean dialog and let browser terminate password flow
      //
      return EFI_NOT_READY;
    }
  } else if (SysPasswordDataInfo->Flag == CHANGE_PASSWORD) {
    if (StrCmp(Password, SysPasswordDataInfo->InputString) != 0x00) {
      //
      //  input password is  NOT  match current set system password
      //  return EFI_NOT_READY to clean dialog and let browser terminate password flow
      //
      return EFI_NOT_READY;
    } else {
      //
      //  input password is match current set system password
      //  return EFI_SUCCESS to display another two password items for new password and confirm new password
      //
      return EFI_SUCCESS;
    }
  }

  //
  //  return EFI_SUCCESS to display two password items for new password and confirm new password
  //
  return  EFI_SUCCESS;
}

STATIC
EFI_STATUS
SetPassword (
  IN  BOOLEAN                           SupervisorOrUser,
  IN  SYS_PASSWORD_INFO_DATA            *SysPasswordDataInfo,
  IN  CHAR16                            *Password
  )
{
  EFI_STATUS                            Status;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    //
    //  Return EFI_NOT_AVAILABLE_YET or EFI_UNSUPPORTED to let browser terminate password flow.
    //
    return EFI_NOT_AVAILABLE_YET;
  }

  if (Password[0] != 0x00) {
    if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      if (SuBrowser->SUCInfo->SupervisorPwdFlag == 0 || SuBrowser->SUCInfo->UserPwdFlag == 0) {
        mSupervisorStartFlag = FALSE;
      }
    } else {
      if (SuBrowser->SUCInfo->SupervisorPwdFlag == 0) {
        mSupervisorStartFlag = FALSE;
      }
    }

    SysPasswordDataInfo->StringLength = (SysPasswordMaxNumber (SuBrowser->SUCInfo->SysPasswordService) + 1) * sizeof(UINT16);
    ZeroMem(SysPasswordDataInfo->InputString, SysPasswordDataInfo->StringLength);
    StrCpy(SysPasswordDataInfo->InputString, Password);
    SysPasswordDataInfo->Flag = CHANGE_PASSWORD;

    if (SupervisorOrUser == SUPERVISOR_FLAG) {
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->SupervisorFlag = 1;
    } else if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = 1;
    }
  } else {
    mSupervisorStartFlag = FALSE;
    SysPasswordDataInfo->Flag = DISABLE_PASSWORD;
    if (SupervisorOrUser == SUPERVISOR_FLAG) {
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->SupervisorFlag = 0;
      if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
        ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = 0;
        SuBrowser->SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
      }
    } else if (PcdGetBool (PcdSysPasswordSupportUserPswd))  {
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = 0;
    }
  }

  //
  //  Return EFI_SUCCESS if password was accepted and saved.
  //
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
CheckSupervisorPassword (
  IN  SETUP_UTILITY_BROWSER_DATA        *SuBrowser,
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL *SysPasswordService,
  IN  CHAR16                            *Password,
  IN OUT UINTN                          *PasswordState
  )
{
  EFI_STATUS                     Status;

  if (SysPasswordService == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (SuBrowser->SUCInfo->SupervisorPassword->Flag != CHANGE_PASSWORD) {

    Status = SysPasswordService->GetStatus (
                                   SysPasswordService,
                                   SystemSupervisor
                                   );
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }

    Status = SysPasswordService->CheckPassword (
                                   SysPasswordService,
                                   Password,
                                   SysPasswordMaxNumber (SysPasswordService) * sizeof(UINT16),
                                   SystemSupervisor
                                   );

    if (!EFI_ERROR(Status)) {
      *PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
      return EFI_UNSUPPORTED;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
CheckUserPassword (
  IN  SETUP_UTILITY_BROWSER_DATA        *SuBrowser,
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL *SysPasswordService,
  IN  CHAR16                            *Password,
  IN OUT UINTN                          *PasswordState
  )
{
  EFI_STATUS                     Status;

  if (SysPasswordService == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((SuBrowser->SUCInfo->UserPassword->Flag != DISABLE_PASSWORD) && (Password[0] != 0x00)) {
    Status = SysPasswordService->CheckPassword (
                                   SysPasswordService,
                                   Password,
                                   SysPasswordMaxNumber (SysPasswordService) * sizeof(UINT16),
                                   SystemUser
                                   );
    if ((!EFI_ERROR(Status)) ||
        (StrCmp (Password, SuBrowser->SUCInfo->UserPassword->InputString) == 0)) {
      *PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
      return EFI_UNSUPPORTED;
    }
  }

  return EFI_SUCCESS;
}

/**

  @param SupervisorOrUser
  @param Type
  @param Value
  @param ActionRequest
  @param PState
  @param SysPasswordService      System Password service ptr
  @param SysPasswordDataInfo     Security information

  @retval EFI_SUCCESS            Password callback success.
  @retval EFI_ALREADY_STARTED    Already password in SetupUtility.
  @retval EFI_NOT_READY          Password confirm error.
  @return EFI_INVALID_PARAMETER  Input parameter is invalid

**/
STATIC
EFI_STATUS
PasswordCallback (
  IN  BOOLEAN                           SupervisorOrUser,
  IN  UINT8                             Type,
  IN  EFI_IFR_TYPE_VALUE                *Value,
  OUT EFI_BROWSER_ACTION_REQUEST        *ActionRequest,
  OUT BOOLEAN                           *PState,
  IN  EFI_SYS_PASSWORD_SERVICE_PROTOCOL *SysPasswordService,
  IN  SYS_PASSWORD_INFO_DATA            *SysPasswordDataInfo
  )
{
  STATIC UINTN                   PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
  EFI_STATUS                     Status;
  CHAR16                         *Password;
  SETUP_UTILITY_BROWSER_DATA     *SuBrowser;

  *PState = FALSE;
  if (Type != EFI_IFR_TYPE_STRING) {
    return  EFI_INVALID_PARAMETER;
  }
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Value->string == 0) {
    //
    // If Value->String == 0, only need reset the state machine to BROWSER_STATE_VALIDATE_PASSWORD
    //
    PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
    return EFI_SUCCESS;
  }

  Password = HiiGetString (
               SuBrowser->SUCInfo->MapTable[SecurityHiiHandle].HiiHandle,
               Value->string,
               NULL
               );
  //
  // When try to set a new password, user will be chanlleged with old password.
  // The Callback is responsible for validating old password input by user,
  // If Callback return EFI_SUCCESS, it indicates validation pass.
  //
  switch (PasswordState) {

  case BROWSER_STATE_VALIDATE_PASSWORD:
    Status = ValidatePassword (SysPasswordService, SupervisorOrUser, SysPasswordDataInfo, Password);
    if (Status == EFI_SUCCESS) {
      PasswordState = BROWSER_STATE_SET_PASSWORD;
    }
    break;

  case BROWSER_STATE_SET_PASSWORD:
    if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      if ((SupervisorOrUser == USERPASSWORD_FLAG) &&
          (Password[0] != 0x00) &&
          StrCmp (SuBrowser->SUCInfo->SupervisorPassword->InputString, Password) == 0) {
        PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
        return EFI_UNSUPPORTED;
      } else {
        if (SupervisorOrUser == USERPASSWORD_FLAG) {
          Status = CheckSupervisorPassword (SuBrowser, SysPasswordService, Password, &PasswordState);
        } else {
          Status = CheckUserPassword (SuBrowser, SysPasswordService, Password, &PasswordState);
        }
        if (EFI_ERROR(Status)) {
          return EFI_UNSUPPORTED;
        }
      }
    }
    Status = SetPassword (SupervisorOrUser, SysPasswordDataInfo, Password);
    *PState = TRUE;
    PasswordState = BROWSER_STATE_VALIDATE_PASSWORD;
    break;

  default:
    Status = EFI_NOT_AVAILABLE_YET;
    break;
  }

  if (Password != NULL) {
    gBS->FreePool (Password);
  }

  return Status;
}

/**
  This command returns various information regarding the TPM and its current state.

  @param[in]  Capability         Group selection; determines the format of the response.
  @param[in]  Property           Further definition of information. 
  @param[in]  PropertyCount      Number of properties of the indicated type to return.
  @param[out] MoreData           Flag to indicate if there are more values of this type.
  @param[out] CapabilityData     The capability data.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
STATIC
EFI_STATUS
Tpm2GetCapability (
  IN      TPM_CAP                   Capability,
  IN      UINT32                    Property,
  IN      UINT32                    PropertyCount,
  OUT     TPMI_YES_NO               *MoreData,
  OUT     TPMS_CAPABILITY_DATA      *CapabilityData
  )
{
  EFI_STATUS                        Status;
  EFI_TCG2_PROTOCOL                 *Tcg2Protocol;
  TPM2_GET_CAPABILITY_COMMAND       SendBuffer;
  TPM2_GET_CAPABILITY_RESPONSE      RecvBuffer;
  UINT32                            SendBufferSize;
  UINT32                            RecvBufferSize;

  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID**)&Tcg2Protocol);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Construct command
  //
  SendBuffer.Header.tag = SwapBytes16 (TPM_ST_NO_SESSIONS);
  SendBuffer.Header.commandCode = SwapBytes32 (TPM_CC_GetCapability);

  SendBuffer.Capability = SwapBytes32 (Capability);
  SendBuffer.Property = SwapBytes32 (Property);
  SendBuffer.PropertyCount = SwapBytes32 (PropertyCount);
 
  SendBufferSize = (UINT32) sizeof (SendBuffer);
  SendBuffer.Header.paramSize = SwapBytes32 (SendBufferSize);
    
  //
  // send Tpm command
  //
  RecvBufferSize = sizeof(RecvBuffer);
  Status = Tcg2Protocol->SubmitCommand (
                           Tcg2Protocol,
                           SendBufferSize,
                           (UINT8 *)&SendBuffer,
                           RecvBufferSize,
                           (UINT8 *)&RecvBuffer
                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (RecvBufferSize <= sizeof (TPM2_RESPONSE_HEADER) + sizeof (UINT8)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Return the response
  //
  *MoreData = RecvBuffer.MoreData;
  //
  // Does not unpack all possiable property here, the caller should unpack it and note the byte order.
  //
  CopyMem (CapabilityData, &RecvBuffer.CapabilityData, RecvBufferSize - sizeof (TPM2_RESPONSE_HEADER) - sizeof (UINT8));
  
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPMA_STARTUP_CLEAR.

  This function parse the value got from TPM2_GetCapability and return the StartupClear value.

  @param[out] Permanent          TPM Permanent.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
STATIC
EFI_STATUS
Tpm2GetCapabilityStartupClear (
  OUT UINT32              *StartupClear
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status;
 
  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_STARTUP_CLEAR, 
             MAX_TPM_PROPERTIES, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
 *StartupClear = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}

/**
  This command returns the information of TPMA_PERMANENT.

  This function parse the value got from TPM2_GetCapability and return the Permanent value.

  @param[out] Permanent          TPM Permanent.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
STATIC
EFI_STATUS
Tpm2GetCapabilityPermanent (
  OUT UINT32              *Permanent
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status;

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_PERMANENT, 
             1, 
             &MoreData, 
             &TpmCap
             );
  
 *Permanent = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}

/**
  This command returns Returns a list of TPM_CAP_COMMANDS.
  This function parse the value got from TPM2_GetCapability and return the list.

  @param[out] CmdList            List of supported commands.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
STATIC
EFI_STATUS
Tpm2GetCapabilityCommandList (
  OUT TPML_CCA            *CmdList
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  UINTN                   Index;
  EFI_STATUS              Status;
 
  Status = Tpm2GetCapability (
             TPM_CAP_COMMANDS, 
             TPM_PT_NONE, 
             MAX_CAP_CC, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  CopyMem (CmdList, &TpmCap.data.command , sizeof (TPML_CCA));

  CmdList->count = SwapBytes32 (CmdList->count);
  for (Index = 0; Index < CmdList->count; Index++) {
    WriteUnaligned32 ((UINT32 *)&CmdList->commandAttributes[Index], SwapBytes32 (ReadUnaligned32 ((UINT32 *)&CmdList->commandAttributes[Index])));
  }

  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM PCRs.

  This function parse the value got from TPM2_GetCapability and return the PcrSelection.

  @param[out] Pcrs    The Pcr Selection
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  
**/
STATIC
EFI_STATUS
Tpm2GetCapabilityPcrs (
  OUT TPML_PCR_SELECTION      *Pcrs
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status;
  UINTN                   Index;

  Status = Tpm2GetCapability (
             TPM_CAP_PCRS, 
             0, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Pcrs->count = SwapBytes32 (TpmCap.data.assignedPCR.count);
  for (Index = 0; Index < Pcrs->count; Index++) {
    Pcrs->pcrSelections[Index].hash = SwapBytes16 (TpmCap.data.assignedPCR.pcrSelections[Index].hash);
    Pcrs->pcrSelections[Index].sizeofSelect = TpmCap.data.assignedPCR.pcrSelections[Index].sizeofSelect;
    CopyMem (Pcrs->pcrSelections[Index].pcrSelect, TpmCap.data.assignedPCR.pcrSelections[Index].pcrSelect, Pcrs->pcrSelections[Index].sizeofSelect);
  }

  return EFI_SUCCESS;
}

/**
  Get current state of TPM device.

  @param[in]   TcgProtocol          Point to EFI_TCG_PROTOCOL instance.
  @param[out]  TpmDisabled          Flag to indicate TPM is disenabled or not.
  @param[out]  TpmDeactivated       Flag to indicate TPM is deactivated or not.

  @retval EFI_SUCCESS               State is successfully returned.
  @retval EFI_INVALID_PARAMETER     Invalid input.
  @retval EFI_DEVICE_ERROR          Failed to get TPM response.
**/
EFI_STATUS
GetTpm12State (
  IN  EFI_TCG_PROTOCOL          *TcgProtocol,
  OUT BOOLEAN                   *TpmDisabled,
  OUT BOOLEAN                   *TpmDeactivated
  )
{
  EFI_STATUS                    Status;
  TPM_RSP_COMMAND_HDR           *TpmRsp;
  UINT32                        TpmSendSize;
  TPM_PERMANENT_FLAGS           *TpmPermanentFlags;
  UINT8                         CmdBuf[64];

  if (TcgProtocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((TpmDisabled == NULL) && (TpmDeactivated == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get TPM Permanent flags
  //
  TpmSendSize           = sizeof (TPM_RQU_COMMAND_HDR) + sizeof (UINT32) * 3;
  *(UINT16*)&CmdBuf[0]  = SwapBytes16 (TPM_TAG_RQU_COMMAND);
  *(UINT32*)&CmdBuf[2]  = SwapBytes32 (TpmSendSize);
  *(UINT32*)&CmdBuf[6]  = SwapBytes32 (TPM_ORD_GetCapability);

  *(UINT32*)&CmdBuf[10] = SwapBytes32 (TPM_CAP_FLAG);
  *(UINT32*)&CmdBuf[14] = SwapBytes32 (sizeof (TPM_CAP_FLAG_PERMANENT));
  *(UINT32*)&CmdBuf[18] = SwapBytes32 (TPM_CAP_FLAG_PERMANENT);

  Status = TcgProtocol->PassThroughToTpm (
                          TcgProtocol,
                          TpmSendSize,
                          CmdBuf,
                          sizeof (CmdBuf),
                          CmdBuf
                          );
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  TpmRsp = (TPM_RSP_COMMAND_HDR *) &CmdBuf[0];
  if ((TpmRsp->tag != SwapBytes16 (TPM_TAG_RSP_COMMAND)) || (TpmRsp->returnCode != 0)) {
    return EFI_DEVICE_ERROR;
  }

  TpmPermanentFlags = (TPM_PERMANENT_FLAGS *) &CmdBuf[sizeof (TPM_RSP_COMMAND_HDR) + sizeof (UINT32)];

  if (TpmDisabled != NULL) {
    *TpmDisabled = (BOOLEAN) TpmPermanentFlags->disable;
  }

  if (TpmDeactivated != NULL) {
    *TpmDeactivated = (BOOLEAN) TpmPermanentFlags->deactivated;
  }

  return EFI_SUCCESS;
}

/**
  Get TPM owner status string.

  @param[in] HiiHandle               Hii hanlde for security page.
  @param[in] TpmDevice               Supported version of the TPM.

  @return TPM Onwer Status token.

**/
STATIC
EFI_STRING_ID
GetTpmOwnerStatusString (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  UINT8                             TpmDevice
  )
{
  EFI_STATUS                            Status;
  EFI_TCG_PROTOCOL                      *TcgProtocol;
  EFI_TCG2_PROTOCOL                     *Tcg2Protocol;
  EFI_STRING_ID                         OwnerStatusToken;
  UINT8                                 OwnerStatus;
  UINT32                                Permanent;
  TPM_GET_PROP_OWNER_COMMAND            TcgSendBuffer;
  TPM_GET_PROP_OWNER_RESPONSE           TcgRecvBuffer;

  switch (TpmDevice) {

  case TPM_DEVICE_NULL:
    OwnerStatusToken = 0;
    break;

  case TPM_DEVICE_1_2:
    Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID**)&TcgProtocol);
    if (EFI_ERROR (Status)) {
      return 0;
    }
    //
    // Construct command
    //
    TcgSendBuffer.Header.tag       = SwapBytes16 (TPM_TAG_RQU_COMMAND);
    TcgSendBuffer.Header.paramSize = SwapBytes32 (sizeof (TcgSendBuffer));
    TcgSendBuffer.Header.ordinal   = SwapBytes32 (TPM_ORD_GetCapability);
    TcgSendBuffer.capArea          = SwapBytes32 (TPM_CAP_PROPERTY);
    TcgSendBuffer.subCap           = SwapBytes32 (TPM_CAP_PROP_OWNER);
    TcgSendBuffer.subCapSize       = SwapBytes32 (sizeof (TPM_CAP_PROP_OWNER));

    Status = TcgProtocol->PassThroughToTpm (
                            TcgProtocol,
                            (UINT32) sizeof (TcgSendBuffer),
                            (UINT8 *) &TcgSendBuffer,
                            (UINT32) sizeof (TcgRecvBuffer),
                            (UINT8 *) &TcgRecvBuffer
                            );
    if ((EFI_ERROR (Status)) || (TcgRecvBuffer.Header.returnCode != TPM_SUCCESS)) {
      return 0;
    }

    OwnerStatus = (TcgRecvBuffer.resp);
    OwnerStatusToken = (OwnerStatus == 0) ? STRING_TOKEN (STR_TPM_UNOWNED_STRING) : STRING_TOKEN (STR_TPM_OWNED_STRING);
    break;

  case TPM_DEVICE_2_0:
    Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID**)&Tcg2Protocol);
    if (EFI_ERROR (Status)) {
      return 0;
    }
    Status = Tpm2GetCapabilityPermanent (&Permanent);
    if (EFI_ERROR (Status)) {
      return 0;
    }
    OwnerStatus = (UINT8)(Permanent & (TPMA_PERMANENT_OWNER_AUTH_SET | TPMA_PERMANENT_LOCKOUT_AUTH_SET));
    OwnerStatusToken = (OwnerStatus == 0) ? STRING_TOKEN (STR_TPM_UNOWNED_STRING) : STRING_TOKEN (STR_TPM_OWNED_STRING);
    break;

  default:
    OwnerStatusToken = 0;
    break;
  }

  return OwnerStatusToken;
}

/**
  Get the TPM status string.

  @param[in] HiiHandle               Hii hanlde for security page.
  @param[in] TpmDevice               Supported version of the TPM.

  @return TPM Status token.

**/
STATIC
EFI_STRING_ID
GetTpmStatusString (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  UINT8                             TpmDevice
  )
{
  EFI_STATUS                            Status;
  EFI_TCG_PROTOCOL                      *TcgProtocol;
  EFI_TCG2_PROTOCOL                     *Tcg2Protocol;
  BOOLEAN                               TpmDisabled;
  BOOLEAN                               TpmDeactivated;
  EFI_STRING_ID                         TpmStatusToken;
  TPMA_STARTUP_CLEAR                    StartupClear;

  switch (TpmDevice) {

  case TPM_DEVICE_NULL:
    TpmStatusToken = STRING_TOKEN (STR_NOT_INSTALLED_TEXT);
    break;

  case TPM_DEVICE_1_2:
    Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID**)&TcgProtocol);
    if (EFI_ERROR (Status)) {
      return 0;
    }

    Status = GetTpm12State (TcgProtocol, &TpmDisabled, &TpmDeactivated);
    if (EFI_ERROR (Status)) {
      return 0;
    }

    if (TpmDeactivated) {
      TpmStatusToken = (TpmDisabled) ? STRING_TOKEN (STR_TPM_DISABLED_DEACTIVATED_STRING) : STRING_TOKEN (STR_TPM_ENABLED_DEACTIVATED_STRING);
    } else {
      TpmStatusToken = (TpmDisabled) ? STRING_TOKEN (STR_TPM_DISABLED_ACTIVATED_STRING) : STRING_TOKEN (STR_TPM_ENABLED_ACTIVATED_STRING);
    }
    break;

  case TPM_DEVICE_2_0:
    Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID**)&Tcg2Protocol);
    if (EFI_ERROR (Status)) {
      return 0;
    }

    Status = Tpm2GetCapabilityStartupClear ((UINT32 *)&StartupClear);
    if (EFI_ERROR (Status)) {
      return 0;
    }
    
    if (StartupClear.ehEnable && StartupClear.phEnable && StartupClear.shEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"All Hierarchies Enabled", NULL);
    } else if (StartupClear.ehEnable && StartupClear.shEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"Platform Hierarchy Disabled", NULL);
    } else if (StartupClear.phEnable && StartupClear.shEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"Endorsement Hierarchy Disabled", NULL);
    } else if (StartupClear.phEnable && StartupClear.ehEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"Storage Hierarchy Disabled", NULL);
    } else if (StartupClear.phEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"Endorsement & Storage Hierarchies Disabled", NULL);
    } else if (StartupClear.ehEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"Platform & Storage Hierarchies Disabled", NULL);
    } else if (StartupClear.shEnable) {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"Platform & Endorsement Hierarchies Disabled", NULL);
    } else {
      HiiSetString (HiiHandle, STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT), L"All Hierarchies Disabled", NULL);
    }
    
    TpmStatusToken = STRING_TOKEN (STR_TCG2_DEVICE_STATE_CONTENT);
    break;

  default:
    TpmStatusToken = 0;
    break;
  }

  return TpmStatusToken;
}

/**
  Check if buffer is all zero.

  @param[in] Buffer      Buffer to be checked.
  @param[in] BufferSize  Size of buffer to be checked.

  @retval TRUE  Buffer is all zero.
  @retval FALSE Buffer is not all zero.
**/
STATIC
BOOLEAN
IsZeroBuffer (
  IN VOID      *Buffer,
  IN UINTN     BufferSize
  )
{
  UINT8        *BufferData;
  UINTN        Index;

  BufferData = Buffer;
  for (Index = 0; Index < BufferSize; Index++) {
    if (BufferData[Index] != 0) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
  Set Config info according to TpmAlgHash.

  @param[in,out] Tcg2ConfigInfo       TCG2 config.
  @param[in]     TpmAlgHash           TpmAlgHash.

**/
STATIC
VOID
SetTcg2ConfigInfo (
     OUT TCG2_CONFIGURATION_INFO    *Tcg2ConfigInfo,
  IN     TPMS_PCR_SELECTION         *PcrSelections
  )
{
  switch (PcrSelections->hash) {
  
  case TPM_ALG_SHA1:
    Tcg2ConfigInfo->Sha1Supported = TRUE;
    if (!IsZeroBuffer (PcrSelections->pcrSelect, PcrSelections->sizeofSelect)) {
      Tcg2ConfigInfo->Sha1Activated= TRUE;
    }
    break;
  
  case TPM_ALG_SHA256:
    Tcg2ConfigInfo->Sha256Supported = TRUE;
    if (!IsZeroBuffer (PcrSelections->pcrSelect, PcrSelections->sizeofSelect)) {
      Tcg2ConfigInfo->Sha256Activated= TRUE;
    }
    break;
  
  case TPM_ALG_SHA384:
    Tcg2ConfigInfo->Sha384Supported = TRUE;
    if (!IsZeroBuffer (PcrSelections->pcrSelect, PcrSelections->sizeofSelect)) {
      Tcg2ConfigInfo->Sha384Activated= TRUE;
    }
    break;
  
  case TPM_ALG_SHA512:
    Tcg2ConfigInfo->Sha512Supported = TRUE;
    if (!IsZeroBuffer (PcrSelections->pcrSelect, PcrSelections->sizeofSelect)) {
      Tcg2ConfigInfo->Sha512Activated= TRUE;
    }
    break;
  
  case TPM_ALG_SM3_256:
    Tcg2ConfigInfo->Sm3Supported = TRUE;
    if (!IsZeroBuffer (PcrSelections->pcrSelect, PcrSelections->sizeofSelect)) {
      Tcg2ConfigInfo->Sm3Activated= TRUE;
    }
    break;
  }
}

/**
  Append Buffer With TpmAlgHash.

  @param[in] Buffer               Buffer to be appended.
  @param[in] BufferSize           Size of buffer.
  @param[in] TpmAlgHash           TpmAlgHash.

**/
STATIC
VOID
AppendBufferWithTpmAlgHash (
  IN UINT16  *Buffer,
  IN UINTN   BufferSize,
  IN UINT32  TpmAlgHash
  )
{
  switch (TpmAlgHash) {
  case TPM_ALG_SHA1:
    if (Buffer[0] != 0) {
      StrCatS (Buffer, BufferSize / sizeof(CHAR16), L", ");
    }
    StrCatS (Buffer, BufferSize / sizeof(CHAR16), L"SHA1");
    break;
  case TPM_ALG_SHA256:
    if (Buffer[0] != 0) {
      StrCatS (Buffer, BufferSize / sizeof(CHAR16), L", ");
    }
    StrCatS (Buffer, BufferSize / sizeof(CHAR16), L"SHA256");
    break;
  case TPM_ALG_SHA384:
    if (Buffer[0] != 0) {
      StrCatS (Buffer, BufferSize / sizeof(CHAR16), L", ");
    }
    StrCatS (Buffer, BufferSize / sizeof(CHAR16), L"SHA384");
    break;
  case TPM_ALG_SHA512:
    if (Buffer[0] != 0) {
      StrCatS (Buffer, BufferSize / sizeof(CHAR16), L", ");
    }
    StrCatS (Buffer, BufferSize / sizeof(CHAR16), L"SHA512");
    break;
  case TPM_ALG_SM3_256:
    if (Buffer[0] != 0) {
      StrCatS (Buffer, BufferSize / sizeof(CHAR16), L", ");
    }
    StrCatS (Buffer, BufferSize / sizeof(CHAR16), L"SM3_256");
    break;
  }
}

/**
  Set Config according to TpmAlgHash.

  @param[in,out] Tcg2ConfigInfo       TCG2 config info.
  @param[in]     TpmAlgHash           TpmAlgHash.

**/
STATIC
VOID
SetActivePcrBanks (
  IN     TCG2_CONFIGURATION_INFO    *Tcg2ConfigInfo,
     OUT UINT8                      *ActivePcrBanks
  )
{
  if (Tcg2ConfigInfo->Sha1Activated) {
    *ActivePcrBanks |= EFI_TCG2_BOOT_HASH_ALG_SHA1;
  } else {
    *ActivePcrBanks &= ~EFI_TCG2_BOOT_HASH_ALG_SHA1;
  }
  
  if (Tcg2ConfigInfo->Sha256Activated) {
    *ActivePcrBanks |= EFI_TCG2_BOOT_HASH_ALG_SHA256;
  } else {
    *ActivePcrBanks &= ~EFI_TCG2_BOOT_HASH_ALG_SHA256;
  }

  if (Tcg2ConfigInfo->Sha384Activated) {
    *ActivePcrBanks |= EFI_TCG2_BOOT_HASH_ALG_SHA384;
  } else {
    *ActivePcrBanks &= ~EFI_TCG2_BOOT_HASH_ALG_SHA384;
  }
  
  if (Tcg2ConfigInfo->Sha512Activated) {
    *ActivePcrBanks |= EFI_TCG2_BOOT_HASH_ALG_SHA512;
  } else {
    *ActivePcrBanks &= ~EFI_TCG2_BOOT_HASH_ALG_SHA512;
  }
  
  if (Tcg2ConfigInfo->Sm3Activated) {
    *ActivePcrBanks |= EFI_TCG2_BOOT_HASH_ALG_SM3_256;
  } else {
    *ActivePcrBanks &= ~EFI_TCG2_BOOT_HASH_ALG_SM3_256;
  }
}

/**
  Update the TPM2 PCR banks information and algorithms for hash.

  @param[in] HiiHandle               Hii hanlde for security page.
  @param[in] KernelConfig            Pointer to KERNEL_CONFIGURATION sturct.

**/
STATIC
VOID
UpdateTpm2AlgHashInfo (
  IN     EFI_HII_HANDLE                    HiiHandle
  )
{
  EFI_STATUS                      Status;
  UINTN                           Index;
  TPML_PCR_SELECTION              Pcrs;
  CHAR16                          TempBuffer[1024];
  
  Status = Tpm2GetCapabilityPcrs (&Pcrs);
  if (EFI_ERROR (Status)) {
    HiiSetString (HiiHandle, STRING_TOKEN (STR_TPM2_ACTIVE_HASH_ALGO_CONTENT), L"[Unknown]", NULL);
    HiiSetString (HiiHandle, STRING_TOKEN (STR_TPM2_SUPPORTED_HASH_ALGO_CONTENT), L"[Unknown]", NULL);
  } else {
    TempBuffer[0] = 0;
    for (Index = 0; Index < Pcrs.count; Index++) {
      if (!IsZeroBuffer (Pcrs.pcrSelections[Index].pcrSelect, Pcrs.pcrSelections[Index].sizeofSelect)) {
        AppendBufferWithTpmAlgHash (TempBuffer, sizeof(TempBuffer), Pcrs.pcrSelections[Index].hash);
      }
    }
    HiiSetString (HiiHandle, STRING_TOKEN (STR_TPM2_ACTIVE_HASH_ALGO_CONTENT), TempBuffer, NULL);

    TempBuffer[0] = 0;
    for (Index = 0; Index < Pcrs.count; Index++) {
      AppendBufferWithTpmAlgHash (TempBuffer, sizeof(TempBuffer), Pcrs.pcrSelections[Index].hash);
      SetTcg2ConfigInfo (&mTcg2ConfigInfo, &Pcrs.pcrSelections[Index]);
    }
    HiiSetString (HiiHandle, STRING_TOKEN (STR_TPM2_SUPPORTED_HASH_ALGO_CONTENT), TempBuffer, NULL);
  }
}

/**
  Update the TPM2 operation options in Security page.

  @param[in] HiiHandle               Hii hanlde for security page.
  @param[in] KernelConfig            Pointer to KERNEL_CONFIGURATION sturct.

**/
STATIC
VOID
UpdateTpm2OperationLabel (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  )
{
  EFI_STATUS                            Status;
  VOID                                  *StartOpCodeHandle;
  VOID                                  *EndOpCodeHandle;
  VOID                                  *OptionsOpCodeHandle;
  EFI_IFR_GUID_LABEL                    *StartLabel;
  EFI_IFR_GUID_LABEL                    *EndLabel;
  UINTN                                 Index;
  TPML_CCA                              CmdList;
  BOOLEAN                               ChangeESPSupported;
  UINT16                                VarOffset;  
  UINTN                                 OptionNum;

  //
  // Check whether ChangeESP command supported
  //
  ChangeESPSupported = FALSE;
  Status = Tpm2GetCapabilityCommandList (&CmdList);
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < CmdList.count; Index++) {
      if (CmdList.commandAttributes[Index].commandIndex == (UINT32)TPM_CC_ChangeEPS) {
        ChangeESPSupported = TRUE;
        break;
      }
    }
  }

  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);
  
  OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (OptionsOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = TPM_OPERATION_START_LABEL;
  
  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode   = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number         = TPM_OPERATION_END_LABEL;

  OptionNum = sizeof(mTpm2OperationTable) / sizeof(TPM_PP_OPERATION_TABLE);
  for (Index = 0; Index < OptionNum; Index++) {
    if (!ChangeESPSupported &&
        (mTpm2OperationTable[Index].OperationValue == TCG2_PHYSICAL_PRESENCE_CHANGE_EPS ||
         mTpm2OperationTable[Index].OperationValue == TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_FALSE ||
         mTpm2OperationTable[Index].OperationValue == TCG2_PHYSICAL_PRESENCE_SET_PP_REQUIRED_FOR_CHANGE_EPS_TRUE)) {
      continue;
    }
    if (!PcdGetBool (PcdH2OTpm2DisableOperationSupported) && 
        (mTpm2OperationTable[Index].OperationValue == TCG2_PHYSICAL_PRESENCE_DISABLE ||
         mTpm2OperationTable[Index].OperationValue == TCG2_PHYSICAL_PRESENCE_DISABLE_ENDORSEMENT_ENABLE_STORAGE_HIERARCHY)) {
      continue;
    }
    HiiCreateOneOfOptionOpCode (
      OptionsOpCodeHandle,
      mTpm2OperationTable[Index].OperationStringId,
      mTpm2OperationTable[Index].OperationValue == 0 ? EFI_IFR_OPTION_DEFAULT : 0,
      EFI_IFR_NUMERIC_SIZE_1,
      (UINT8)mTpm2OperationTable[Index].OperationValue
      );
  }
  
  VarOffset = (UINT16)((UINTN)(&KernelConfig->TpmOperation) - (UINTN)(KernelConfig));

  HiiCreateOneOfOpCode (
    StartOpCodeHandle,
    (EFI_QUESTION_ID)KEY_TPM2_OPERATION,
    CONFIGURATION_VARSTORE_ID,
    VarOffset,
    STRING_TOKEN (STR_TPM_OPERATION_STRING),
    STRING_TOKEN (STR_TCG2_OPERATION_HELP),
    0,
    EFI_IFR_NUMERIC_SIZE_1,
    OptionsOpCodeHandle,
    NULL
    );

  HiiUpdateForm (
    HiiHandle,
    NULL,
    ROOT_FORM_ID,
    StartOpCodeHandle,
    EndOpCodeHandle
    );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);
  HiiFreeOpCodeHandle (OptionsOpCodeHandle);
}

/**
  Update the TPM State in Security page.

  @param[in]     HiiHandle               Hii hanlde for security page.
  @param[in]     TpmDevice               Supported version of the TPM.
  @param[in]     TokenToUpdate           The token updated.

**/
STATIC
VOID
UpdateTpmStateToken (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN     UINT8                          TpmDevice,
  IN     EFI_STRING_ID                  TokenToUpdate
  )
{
  CHAR16                                *TpmStateString;
  CHAR16                                *OwnerStateString;
  CHAR16                                *UpdatedString;
  UINTN                                 UpdatedStringSize;
  UINTN                                 TotalLanguageCount;
  UINTN                                 LanguageCount;
  CHAR8                                 *LanguageString;
  CHAR8                                 *Lang;
  EFI_STRING_ID                         TpmStatusToken;
  EFI_STRING_ID                         OwnerStatusToken;

  TpmStatusToken = GetTpmStatusString (HiiHandle, TpmDevice);
  OwnerStatusToken = GetTpmOwnerStatusString (HiiHandle, TpmDevice);

  TpmStateString = NULL;
  OwnerStateString = NULL;
  GetLangDatabase (&TotalLanguageCount, (UINT8**)&LanguageString);
  for (LanguageCount = 0; LanguageCount < TotalLanguageCount; LanguageCount++) {
    Lang = &LanguageString[LanguageCount * RFC_3066_ENTRY_SIZE];
    if ((TpmStatusToken != 0) && (OwnerStatusToken != 0)) {
      TpmStateString = HiiGetString (HiiHandle, TpmStatusToken, Lang);
      OwnerStateString = HiiGetString (HiiHandle, OwnerStatusToken, Lang);
      UpdatedStringSize = StrSize (TpmStateString) + StrSize (OwnerStateString) + sizeof (CHAR16) * 2;
      UpdatedString = AllocateZeroPool (UpdatedStringSize);
      UnicodeSPrint (UpdatedString, UpdatedStringSize, L"%s%s%s", TpmStateString, L", ", OwnerStateString);
    } else if (TpmStatusToken != 0) {
      UpdatedString = HiiGetString (HiiHandle, TpmStatusToken, Lang);
    } else {
      UpdatedString = HiiGetString (HiiHandle, STRING_TOKEN (STR_TPM_UNKNOWN_STRING), Lang);
    }

    if (TpmStateString != NULL) {
      gBS->FreePool (TpmStateString);
    }

    if (OwnerStateString != NULL) {
      gBS->FreePool (OwnerStateString);
    }

    HiiSetString (HiiHandle, TokenToUpdate, UpdatedString, Lang);
    gBS->FreePool (UpdatedString);
    UpdatedString = NULL;
  }
}

/**
  Update the TPM2 device token string in Security page.

  @param[in]     HiiHandle               Hii hanlde for security page.
  @param[in]     TpmDevice               Supported version of the TPM.
  @param[in]     TokenToUpdate           The token updated.

**/
STATIC
VOID
UpdateTpm2DeviceToken (
  IN     EFI_HII_HANDLE                 HiiHandle,
  IN     UINT8                          TpmDevice,
  IN     EFI_STRING_ID                  TokenToUpdate
  )
{
  EFI_STATUS                            Status;
  CHAR16                                *UpdatedString;
  UINTN                                 TotalLanguageCount;
  UINTN                                 LanguageCount;
  CHAR8                                 *LanguageString;
  CHAR8                                 *Lang;
  BOOLEAN                               Dtpm2Selected;
  VOID                                  *Interface;

  Status = gBS->LocateProtocol (&gH2OTpm20DtpmPublishAcpiTableDoneGuid, NULL, (VOID**)&Interface);
  Dtpm2Selected = (EFI_ERROR (Status)) ? FALSE : TRUE;

  UpdatedString = NULL;
  GetLangDatabase (&TotalLanguageCount, (UINT8**)&LanguageString);
  for (LanguageCount = 0; LanguageCount < TotalLanguageCount; LanguageCount++) {
    Lang = &LanguageString[LanguageCount * RFC_3066_ENTRY_SIZE];
    if (Dtpm2Selected) {
      UpdatedString = HiiGetString (HiiHandle, STRING_TOKEN (STR_TPM2_DTPM_TEXT_STRING), Lang);
    } else {
      UpdatedString = HiiGetString (HiiHandle, STRING_TOKEN (STR_TPM2_FTPM_TEXT_STRING), Lang);
    }

    HiiSetString (HiiHandle, TokenToUpdate, UpdatedString, Lang);
    gBS->FreePool (UpdatedString);
    UpdatedString = NULL;
  }
}


/**
  Update TPM items for SCU.

  @param HiiHandle               Hii hanlde for security page.
  @param KernelConfig            Pointer to KERNEL_CONFIGURATION sturct.

  @retval None.

**/
STATIC
VOID
UpdateTpmItemsForSCU (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  KERNEL_CONFIGURATION              *KernelConfig
  )
{
  EFI_STATUS                            Status;
  EFI_TCG_PROTOCOL                      *TcgProtocol;
  EFI_TCG2_PROTOCOL                     *Tcg2Protocol;
  BOOLEAN                               TpmDisabled;
  BOOLEAN                               TpmDeactivated;
  UINT8                                 TpmDevice;
  VOID                                  *StartOpCodeHandle;
  EFI_IFR_GUID_LABEL                    *StartLabel;

  TpmDisabled     = FALSE;
  TpmDeactivated  = FALSE;
  TpmDevice       = KernelConfig->TpmDevice;
  switch (TpmDevice) {

  case TPM_DEVICE_1_2:
    //
    // Verify if TPM1.2 is present
    //
    Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID**)&TcgProtocol);
    if (EFI_ERROR (Status)) {
      TpmDevice = TPM_DEVICE_NULL;
      break;
    }

    Status = GetTpm12State (TcgProtocol, &TpmDisabled, &TpmDeactivated);
    if (EFI_ERROR (Status)) {
      TpmDevice = TPM_DEVICE_NULL;
      break;
    }

    KernelConfig->GrayoutTpmClear = (TpmDeactivated | TpmDisabled);
    break;

  case TPM_DEVICE_2_0:
    //
    // Verify if TPM2.0 is present
    //
    Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID**)&Tcg2Protocol);
    if (EFI_ERROR (Status)) {
      TpmDevice = TPM_DEVICE_NULL;
      break;
    }

    if (PcdGetBool (PcdTpmAutoDetection)) {
      UpdateTpm2DeviceToken (HiiHandle, TpmDevice, STRING_TOKEN (STR_TPM2_TEXT_STRING));
    }

    //
    // Update TPM operation options
    //
    UpdateTpm2OperationLabel (HiiHandle, KernelConfig);

    //
    // Update PCR banks info and hash algorithm
    //
    UpdateTpm2AlgHashInfo (HiiHandle);

    SetActivePcrBanks (&mTcg2ConfigInfo, &KernelConfig->ActivePcrBanks);
    break;

  default:
    TpmDevice = TPM_DEVICE_NULL;
    break;
  }

  //
  // Don't update TPM state string if TPM is hidden
  //
  if (KernelConfig->TpmHide == 1 || PcdGetBool (PcdTpmHide)) {
    return;
  }

  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = TPM_STATE_LABEL;
  //
  // Update TPM_STATE_LABEL
  //
  UpdateTpmStateToken (HiiHandle, TpmDevice, STRING_TOKEN (STR_TPM_STATUS_STRING2));

  HiiCreateTextOpCode (StartOpCodeHandle, STRING_TOKEN (STR_TPM_STATUS_STRING), 0, STRING_TOKEN (STR_TPM_STATUS_STRING2));

  HiiUpdateForm (
    HiiHandle,
    NULL,
    ROOT_FORM_ID,
    StartOpCodeHandle,
    NULL
    );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  return;
}

/**
 This function allows a caller to extract TCG2 configuration.

 @param [in]   This             Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
 @param [in]   Request          A null-terminated Unicode string in <ConfigRequest> format.
 @param [out]  Progress         On return, points to a character in the Request string.
                                Points to the string's null terminator if request was successful.
                                Points to the most recent '&' before the first failing name/value
                                pair (or the beginning of the string if the failure is in the
                                first name/value pair) if the request was not successful.
 @param [out]  Results          A null-terminated Unicode string in <ConfigAltResp> format which
                                has all values filled in for the names in the Request string.
                                String to be allocated by the called function.

 @retval EFI_SUCCESS            The Results is filled with the requested values.
 @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
 @retval EFI_INVALID_PARAMETER  Request is NULL, illegal syntax, or unknown name.
 @retval EFI_UNSUPPORTED        Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
Tcg2ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                    Status;
  UINTN                         BufferSize;
  EFI_STRING                    ConfigRequestHdr;
  EFI_STRING                    ConfigRequest;
  BOOLEAN                       AllocatedRequest;
  UINTN                         Size;
  CHAR16                        *StrPointer;
  EFI_CALLBACK_INFO             *CallbackInfo;
  EFI_HANDLE                    DriverHandle;
  SETUP_UTILITY_BROWSER_DATA    *SuBrowser;


  if (This == NULL || Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check routing data in <ConfigHdr>.
  // Note: if only one Storage is used, then this checking could be skipped.
  //
  if (!HiiIsConfigHdrMatch (Request, &mTcg2VarStoreGuid, mTcg2VarStoreName)) {
    return EFI_UNSUPPORTED;
  }

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  *Progress        = Request;
  CallbackInfo     = EFI_CALLBACK_INFO_FROM_THIS (This);
  BufferSize       = sizeof (mTcg2ConfigInfo);
  ConfigRequestHdr = NULL;
  ConfigRequest    = NULL;
  AllocatedRequest = FALSE;
  Size             = 0;

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //
    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    Status = SuBrowser->HiiDatabase->GetPackageListHandle (SuBrowser->HiiDatabase, CallbackInfo->HiiHandle, &DriverHandle);
    if (EFI_ERROR (Status)) {
      return EFI_NOT_FOUND;
    }
    ConfigRequestHdr = HiiConstructConfigHdr (&mTcg2VarStoreGuid, mTcg2VarStoreName, DriverHandle);
    if (ConfigRequestHdr == NULL) {
      return EFI_UNSUPPORTED;
    }
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    if (ConfigRequest == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64) BufferSize);
    FreePool (ConfigRequestHdr);
    ConfigRequestHdr = NULL;
  } else {
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
    //
    // Check whether Request includes Request Element.
    //
    if (StrStr (Request, L"OFFSET") == NULL) {
      //
      // Check Request Element does exist in Reques String
      //
      StrPointer = StrStr (Request, L"PATH");
      if (StrPointer == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (StrStr (StrPointer, L"&") == NULL) {
        Size = (StrLen (Request) + 32 + 1) * sizeof (CHAR16);
        ConfigRequest = AllocateZeroPool (Size);
        ASSERT (ConfigRequest != NULL);
        if (ConfigRequest == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        AllocatedRequest = TRUE;
        UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", Request, (UINT64) BufferSize);
      }
    }
  }

  if (StrStr (ConfigRequest, L"OFFSET") == NULL) {
    //
    // If requesting Name/Value storage, return not found.
    //
    return EFI_NOT_FOUND;
  }

  //
  // Convert buffer data to <ConfigResp> by helper function BlockToConfig()
  //
  Status = SuBrowser->HiiConfigRouting->BlockToConfig (
                                           SuBrowser->HiiConfigRouting,
                                           ConfigRequest,
                                           (UINT8 *) &mTcg2ConfigInfo,
                                           BufferSize,
                                           Results,
                                           Progress
                                           );

  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    gBS->FreePool (ConfigRequest);
    ConfigRequest = NULL;
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return Status;
}

/**
 This function allows a caller to extract the current configuration for one
 or more named elements from the target driver.

 @param [in]   This             Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
 @param [in]   Request          A null-terminated Unicode string in <ConfigRequest> format.
 @param [out]  Progress         On return, points to a character in the Request string.
                                Points to the string's null terminator if request was successful.
                                Points to the most recent '&' before the first failing name/value
                                pair (or the beginning of the string if the failure is in the
                                first name/value pair) if the request was not successful.
 @param [out]  Results          A null-terminated Unicode string in <ConfigAltResp> format which
                                has all values filled in for the names in the Request string.
                                String to be allocated by the called function.

 @retval EFI_SUCCESS            The Results is filled with the requested values.
 @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
 @retval EFI_INVALID_PARAMETER  Request is NULL, illegal syntax, or unknown name.
 @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
SecurityExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                                Status;
  SETUP_UTILITY_BROWSER_DATA                *SuBrowser;

  if (HiiIsConfigHdrMatch (Request, &mTcg2VarStoreGuid, mTcg2VarStoreName)) {
    return Tcg2ExtractConfig (This, Request, Progress, Results);
  } else {
    Status = GetSetupUtilityBrowserData (&SuBrowser);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    return SuBrowser->ExtractConfig (This, Request, Progress, Results);
  }
}

/**
  Initialize security menu for setuputility use

  @param HiiHandle               Hii hanlde for the call back routine

  @retval EFI_SUCCESS            Function has completed successfully.
  @retval Others                 Error occurred during execution.

**/
EFI_STATUS
InitSecurityMenu (
  IN EFI_HII_HANDLE       HiiHandle
  )
{
  EFI_STATUS                                Status;
  SETUP_UTILITY_CONFIGURATION               *SUCInfo;
  UINT8                                     *SCBuffer;
  SETUP_UTILITY_BROWSER_DATA                *SuBrowser;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SUCInfo  = SuBrowser->SUCInfo;
  SCBuffer = SuBrowser->SCBuffer;

  if (PcdGetBool (PcdH2OTpmSupported) || PcdGetBool (PcdH2OTpm2Supported)) {
    UpdateTpmItemsForSCU (HiiHandle, (KERNEL_CONFIGURATION *) SCBuffer);
  }

  SUCInfo->SupervisorPassword = NULL;
  SUCInfo->UserPassword       = NULL;

  Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **) &SUCInfo->SysPasswordService
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Check password
    //
    Status = PasswordCheck (
               SUCInfo,
               (KERNEL_CONFIGURATION *) SCBuffer
               );
    ASSERT_EFI_ERROR (Status);
  }

  Status = gBS->LocateProtocol (
                  &gEfiHddPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **) &SUCInfo->HddPasswordService
                  );
  if (!EFI_ERROR (Status)) {
    Status = FrozenHddResetStatus (
               );

    InitHddPasswordScuData ();

    UpdateAllHddPasswordFlag (
      HiiHandle,
      SUCInfo->HddPasswordScuData,
      SUCInfo->NumOfHdd
      );

    Status = InitStoragePasswordForSCU (HiiHandle);
  }

  return EFI_SUCCESS;
}

/**
  Update the string token for password state.

  @param HiiHandle               Hii hanlde for security page.

  @retval EFI_SUCCESS            The updating of password state is successful.
  @retval Other                  Error occurred during execution.

**/
EFI_STATUS
UpdatePasswordState (
  EFI_HII_HANDLE                        HiiHandle
  )
{
  CHAR16                                *NewString;
  STRING_REF                            TokenToUpdate;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_STATUS                            Status;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!(SuBrowser->SUCInfo->SupervisorPwdFlag)) {
    NewString = HiiGetString (
                  HiiHandle,
                  STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                  NULL
                  );

    ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->SupervisorFlag = FALSE;
  } else {
    NewString = HiiGetString (
                  HiiHandle,
                  STRING_TOKEN (STR_INSTALLED_TEXT),
                  NULL
                  );

    ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->SupervisorFlag = TRUE;
  }

  TokenToUpdate = STRING_TOKEN (STR_SUPERVISOR_PASSWORD_STRING2);
  HiiSetString (HiiHandle, TokenToUpdate, NewString, NULL);

  if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
    if (!SuBrowser->SUCInfo->UserPwdFlag) {
      gBS->FreePool (NewString);

      NewString = HiiGetString (
                    HiiHandle,
                    STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                    NULL
                    );

      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->UserFlag = FALSE;
    }

    TokenToUpdate = STRING_TOKEN (STR_USER_PASSWORD_STRING2);
    HiiSetString (HiiHandle, TokenToUpdate, NewString, NULL);
  }

  gBS->FreePool (NewString);

  return EFI_SUCCESS;
}

/**
  This function is called by the forms browser in response to a user action on a question which has the
  EFI_IFR_FLAG_CALLBACK bit set in the EFI_IFR_QUESTION_HEADER. The user action is specified by Action.
  Depending on the action, the browser may also pass the question value using Type and Value. Upon return,
  the callback function may specify the desired browser action. Callback functions should return
  EFI_UNSUPPORTEDfor all values of Action that they do not support.

  @param This                    Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action                  Specifies the type of action taken by the browser.
  @param QuestionId              A unique value which is sent to the original exporting driver so that it can identify the
                                 type of data to expect. The format of the data tends to vary based on the opcode that
                                 generated the callback.
  @param Type                    The type of value for the question.
  @param Value                   A pointer to the data being sent to the original exporting driver. The type is specified
                                 by Type. Type EFI_IFR_TYPE_VALUE is defined in EFI_IFR_ONE_OF_OPTION.
  @param ActionRequest           On return, points to the action requested by the callback function. Type
                                 EFI_BROWSER_ACTION_REQUEST is specified in SendForm() in the Form Browser Protocol.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the callback.

**/
EFI_STATUS
EFIAPI
SecurityCallbackRoutine (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                            Status;
  BOOLEAN                               PState;
  CHAR16                                *NewString;
  CHAR16                                *StringPtr;
  KERNEL_CONFIGURATION                  *MyIfrNVData;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_CALLBACK_INFO                     *CallbackInfo;
  EFI_INPUT_KEY                         Key;
  SETUP_UTILITY_CONFIGURATION           *SUCInfo;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  UINTN                                 BufferSize;
  EFI_GUID                              VarStoreGuid = SYSTEM_CONFIGURATION_GUID;
  EFI_GUID                              PasswordConfigGuid = PASSWORD_CONFIGURATION_GUID;
  UINTN                                 PasswordConfigBufferSize;
  UINT16                                HddIndex;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL    *Interface;

  HddIndex = 0;

  if (!FeaturePcdGet (PcdH2OFormBrowserSupported) && Action == EFI_BROWSER_ACTION_CHANGING) {
    //
    // For compatible to old form browser which only use EFI_BROWSER_ACTION_CHANGING action,
    // change action to EFI_BROWSER_ACTION_CHANGED to make it workable.
    //
    Action = EFI_BROWSER_ACTION_CHANGED;
  }

  if (FeaturePcdGet (PcdH2OFormBrowserSupported) && Action == EFI_BROWSER_ACTION_CHANGING) {
    //
    // Advance Setup only send CHANGING in "goto" opcode, doesn't send CHANGED
    //
    if (QuestionId >= LABEL_STORAGE_PASSWORD_OPTION && QuestionId <= (LABEL_STORAGE_PASSWORD_OPTION + 16)) {
      Action = EFI_BROWSER_ACTION_CHANGED;
    }
  }


  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    return SecurityCallbackRoutineByAction (This, Action, QuestionId, Type, Value, ActionRequest);
  }

  *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  CallbackInfo = EFI_CALLBACK_INFO_FROM_THIS (This);
  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Interface  = NULL;
  BufferSize = GetVarStoreSize (CallbackInfo->HiiHandle, &CallbackInfo->FormsetGuid, &VarStoreGuid, "SystemConfig");
  PasswordConfigBufferSize = sizeof (PASSWORD_CONFIGURATION);
  if (!FeaturePcdGet (PcdH2OFormBrowserSupported)) {
    if (QuestionId == GET_SETUP_CONFIG || QuestionId == SET_SETUP_CONFIG) {
      Status = SetupVariableConfig (
                 &VarStoreGuid,
                 L"SystemConfig",
                 BufferSize,
                 (UINT8 *) SuBrowser->SCBuffer,
                 (BOOLEAN) (QuestionId == GET_SETUP_CONFIG)
                 );
      Status = SetupVariableConfig (
                 &PasswordConfigGuid,
                 L"PasswordConfig",
                 PasswordConfigBufferSize,
                 (UINT8 *) &mPasswordConfig,
                 (BOOLEAN) (QuestionId == GET_SETUP_CONFIG)
                 );
      return Status;
    }
    Status = gBS->LocateProtocol (
                   &gEfiSetupUtilityBrowserProtocolGuid,
                   NULL,
                   (VOID **) &Interface
                   );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = SetupVariableConfig (
             &VarStoreGuid,
             L"SystemConfig",
             BufferSize,
             (UINT8 *) SuBrowser->SCBuffer,
             TRUE
             );
  Status = SetupVariableConfig (
             &PasswordConfigGuid,
             L"PasswordConfig",
             PasswordConfigBufferSize,
             (UINT8 *) &mPasswordConfig,
             TRUE
             );
  Status = SetupVariableConfig (
             &mTcg2VarStoreGuid,
             mTcg2VarStoreName,
             sizeof (TCG2_CONFIGURATION_INFO),
             (UINT8 *) &mTcg2ConfigInfo,
             TRUE
             );

  MyIfrNVData = (KERNEL_CONFIGURATION *) SuBrowser->SCBuffer;
  Status = EFI_SUCCESS;
  StringPtr = NULL;
  HiiHandle = CallbackInfo->HiiHandle;
  SUCInfo   = SuBrowser->SUCInfo;

  if (QuestionId >= LABEL_STORAGE_PASSWORD_OPTION && QuestionId <= (LABEL_STORAGE_PASSWORD_OPTION + SUCInfo->NumOfHdd)) {

    HddIndex = QuestionId - LABEL_STORAGE_PASSWORD_OPTION;
    QuestionId = LABEL_STORAGE_PASSWORD_OPTION;
  }

  switch (QuestionId) {

  case KEY_SUPERVISOR_PASSWORD:
    Status = PasswordCallback (
               SUPERVISOR_FLAG,
               Type,
               Value,
               ActionRequest,
               &PState,
               SUCInfo->SysPasswordService,
               SUCInfo->SupervisorPassword
               );
    if (PState) {
        StringPtr = HiiGetString (
                      HiiHandle,
                      STRING_TOKEN (STR_SUPERVISOR_PASSWORD_STRING2),
                      NULL
                      );
      if (SUCInfo->SupervisorPassword->Flag == DISABLE_PASSWORD) {
        NewString = HiiGetString (
                      HiiHandle,
                      STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                      NULL
                      );
        PState = FALSE;
        if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
          if (SUCInfo->UserPassword->SystemPasswordStatus == EFI_SUCCESS) {
            SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
          }
        }
      } else {
        NewString = HiiGetString (
                      HiiHandle,
                      STRING_TOKEN (STR_INSTALLED_TEXT),
                      NULL
                      );
        PState = TRUE;
        if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
          //
          // If the new supervisor password is the same as user password, then clear user password.
          //
          if (SUCInfo->UserPassword->Flag == NO_ACCESS_PASSWORD) {
            if (SUCInfo->UserPassword->SystemPasswordStatus == EFI_SUCCESS) {
              Status = SUCInfo->SysPasswordService->CheckPassword (
                                                      SUCInfo->SysPasswordService,
                                                      SUCInfo->SupervisorPassword->InputString,
                                                      SUCInfo->SupervisorPassword->StringLength,
                                                      SystemUser
                                                      );
              if (Status == EFI_SUCCESS) {
                SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
                StringPtr = HiiGetString (
                              HiiHandle,
                              STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                              NULL
                              );
                UpdateUserTags (
                  HiiHandle,
                  FALSE,
                  StringPtr,
                  MyIfrNVData
                  );
                SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
              }
            }
          } else if (SUCInfo->UserPassword->Flag == CHANGE_PASSWORD) {
            if (StrCmp (SUCInfo->SupervisorPassword->InputString,
                   SUCInfo->UserPassword->InputString) == 0) {
              SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
              StringPtr = HiiGetString (
                            HiiHandle,
                            STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                            NULL
                            );
              UpdateUserTags (HiiHandle, FALSE, StringPtr, MyIfrNVData);
              SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
            }
          }
        }
      }

      //
      // If Supervisor Password text op-code is not the same as new state, update the string
      //
      if (StrCmp (StringPtr, NewString) != 0) {
        UpdateSupervisorTags (HiiHandle, PState, NewString, MyIfrNVData);
        SuBrowser->SUCInfo->SupervisorPwdFlag = !(SuBrowser->SUCInfo->SupervisorPwdFlag);

        if (SuBrowser->SUCInfo->SupervisorPwdFlag == FALSE) {
          SuBrowser->SUCInfo->UserPwdFlag = FALSE;
        }
      }

      gBS->FreePool (NewString);
      gBS->FreePool (StringPtr);
      Status = EFI_SUCCESS;
    }
    SUCInfo->DoRefresh = TRUE;
    break;
  case KEY_USER_PASSWORD:
    if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      Status = PasswordCallback (
                 USERPASSWORD_FLAG,
                 Type,
                 Value,
                 ActionRequest,
                 &PState,
                 SUCInfo->SysPasswordService,
                 SUCInfo->UserPassword
                 );

      if (PState) {
        StringPtr = HiiGetString (
                      HiiHandle,
                      STRING_TOKEN (STR_USER_PASSWORD_STRING2),
                      NULL
                      );
        if (SUCInfo->UserPassword->Flag == DISABLE_PASSWORD) {
          NewString = HiiGetString (
                        HiiHandle,
                        STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                        NULL
                        );
          PState = FALSE;
          SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
        } else {
          if (SUCInfo->SupervisorPassword->Flag == NO_ACCESS_PASSWORD) {
            if (SUCInfo->SupervisorPassword->SystemPasswordStatus == EFI_SUCCESS) {
              Status = SUCInfo->SysPasswordService->CheckPassword (
                                                      SUCInfo->SysPasswordService,
                                                      SUCInfo->UserPassword->InputString,
                                                      SUCInfo->UserPassword->StringLength,
                                                      SystemSupervisor
                                                      );

              if (Status == EFI_SUCCESS) {
                SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
                Status = EFI_SECURITY_VIOLATION;
                SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
                break;
              }
            }
          } else if (SUCInfo->SupervisorPassword->Flag == CHANGE_PASSWORD)  {
            if (StrCmp (SUCInfo->SupervisorPassword->InputString,
                   SUCInfo->UserPassword->InputString) == 0) {
              SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
              Status = EFI_SECURITY_VIOLATION;
              SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
              break;
            }
          }
          NewString = HiiGetString (
                        HiiHandle,
                        STRING_TOKEN (STR_INSTALLED_TEXT),
                        NULL
                        );
          PState = TRUE;
          SuBrowser->SUCInfo->UserPwdFlag       = TRUE;
        }

        //
        // If User Password text op-code is not the same as new state, update the string
        //
        if (StrCmp (StringPtr, NewString) != 0) {
          UpdateUserTags (HiiHandle, PState, NewString, MyIfrNVData);
        }

        gBS->FreePool (NewString);
        gBS->FreePool (StringPtr);
        Status = EFI_SUCCESS;
      }
      SUCInfo->DoRefresh = TRUE;
    }
    break;

  case KEY_CLEAR_USER_PASSWORD:
    if (PcdGetBool (PcdSysPasswordSupportUserPswd)) {
      StringPtr = HiiGetString (
                    HiiHandle,
                    STRING_TOKEN (STR_CLEAR_USER_PASSWORD_STRING),
                    NULL
                    );
      SuBrowser->H2ODialog->ConfirmDialog (
                                 0,
                                 FALSE,
                                 0,
                                 NULL,
                                 &Key,
                                 StringPtr
                                 );
      if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        SUCInfo->UserPassword->Flag = DISABLE_PASSWORD;
        NewString = HiiGetString (
                      HiiHandle,
                      STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                      NULL
                      );
        UpdateUserTags (HiiHandle, FALSE, NewString, MyIfrNVData);
        gBS->FreePool (NewString);
        SuBrowser->SUCInfo->UserPwdFlag       = FALSE;
      }

      gBS->FreePool (StringPtr);
    }
    break;
  case KEY_SET_ALL_HDD_PASSWORD:
    Status = AllHddPasswordCallback (
               This,
               USER_PSW,
               HiiHandle,
               Type,
               Value,
               ActionRequest,
               &PState,
               SUCInfo->HddPasswordScuData,
               SUCInfo->NumOfHdd
               );
    break;

  case KEY_SET_ALL_MASTER_HDD_PASSWORD:
    Status = AllHddPasswordCallback (
               This,
               MASTER_PSW,
               HiiHandle,
               Type,
               Value,
               ActionRequest,
               &PState,
               SUCInfo->HddPasswordScuData,
               SUCInfo->NumOfHdd
               );
    break;

    case LABEL_STORAGE_PASSWORD_OPTION:
      Status = StoragePasswordUpdateForm (
                 HiiHandle,
                 HddIndex
                 );
      break;

    case KEY_SET_STORAGE_PASSWORD:
    case KEY_CHECK_STORAGE_PASSWORD:
      Status = StoragePasswordCallback (
                 HiiHandle,
                 Type,
                 Value,
                 mPasswordConfig.SelectedScuDataIndex,
                 USER_PSW
                 );
      if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
        BrowserRefreshFormSet ();
      } else {
        Interface->Firstin = TRUE;
        Interface->JumpToFirstOption = FALSE;
      }
      break;

    case KEY_MASTER_STORAGE_PASSWORD:
      Status = StoragePasswordCallback (
                 HiiHandle,
                 Type,
                 Value,
                 mPasswordConfig.SelectedScuDataIndex,
                 MASTER_PSW
                 );
      if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
        BrowserRefreshFormSet ();
      } else {
        Interface->Firstin = TRUE;
        Interface->JumpToFirstOption = FALSE;
      }
      break;

    case KEY_TPM_HIDE:
    case KEY_TPM2_HIDE:
      //
      // Do nothing if TPM is hidden
      //
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->TpmClear      = 0;
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->TpmOperation  = 0;
      ((KERNEL_CONFIGURATION *) SuBrowser->SCBuffer)->Tpm2Operation = 0;
      break;

    case KEY_TPM2_PCR_BANKS_REQUEST_0:
    case KEY_TPM2_PCR_BANKS_REQUEST_1:
    case KEY_TPM2_PCR_BANKS_REQUEST_2:
    case KEY_TPM2_PCR_BANKS_REQUEST_3:
      SetActivePcrBanks (&mTcg2ConfigInfo, &(((KERNEL_CONFIGURATION *)SuBrowser->SCBuffer)->ActivePcrBanks));
      break;
    
    case KEY_TREE_PROTOCOL_VERSION:
      if ((((KERNEL_CONFIGURATION *)SuBrowser->SCBuffer)->TrEEVersion) != 0) {
        if (!mTcg2ConfigInfo.Sha256Activated) {
          mTcg2ConfigInfo.Sha256Activated = TRUE;
          SetActivePcrBanks (&mTcg2ConfigInfo, &(((KERNEL_CONFIGURATION *)SuBrowser->SCBuffer)->ActivePcrBanks));
        }
      }
      break;
    
    default:
      Status = SuBrowser->HotKeyCallback (
                            This,
                            Action,
                            QuestionId,
                            Type,
                            Value,
                            ActionRequest
                            );
      break;
  }

  SetupVariableConfig (
    &VarStoreGuid,
    L"SystemConfig",
    BufferSize,
    (UINT8 *) SuBrowser->SCBuffer,
    FALSE
    );

  SetupVariableConfig (
    &PasswordConfigGuid,
    L"PasswordConfig",
    PasswordConfigBufferSize,
    (UINT8 *) &mPasswordConfig,
    FALSE
    );

  SetupVariableConfig (
    &mTcg2VarStoreGuid,
    mTcg2VarStoreName,
    sizeof (TCG2_CONFIGURATION_INFO),
    (UINT8 *) &mTcg2ConfigInfo,
    FALSE
    );

  return Status;
}

EFI_STATUS
SecurityCallbackRoutineByAction (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                            Status;
  SETUP_UTILITY_BROWSER_DATA            *SuBrowser;
  EFI_CALLBACK_INFO                     *CallbackInfo;
  UINTN                                 BufferSize;
  EFI_GUID                              VarStoreGuid = SYSTEM_CONFIGURATION_GUID;
  EFI_GUID                              PasswordConfigGuid = PASSWORD_CONFIGURATION_GUID;
  UINTN                                 PasswordConfigBufferSize;

  if ((This == NULL) ||
      ((Value == NULL) &&
       (Action != EFI_BROWSER_ACTION_FORM_OPEN) &&
       (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
      (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PasswordConfigBufferSize = sizeof (PASSWORD_CONFIGURATION);
  *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  CallbackInfo   = EFI_CALLBACK_INFO_FROM_THIS (This);
  BufferSize     = GetVarStoreSize (CallbackInfo->HiiHandle, &CallbackInfo->FormsetGuid, &VarStoreGuid, "SystemConfig");
  Status         = EFI_UNSUPPORTED;

  switch (Action) {

  case EFI_BROWSER_ACTION_FORM_OPEN:
    if (QuestionId == 0) {
      Status = SetupVariableConfig (
                 &VarStoreGuid,
                 L"SystemConfig",
                 BufferSize,
                 (UINT8 *) SuBrowser->SCBuffer,
                 FALSE
                 );
      Status = SetupVariableConfig (
                 &PasswordConfigGuid,
                 L"PasswordConfig",
                 PasswordConfigBufferSize,
                 (UINT8 *) &mPasswordConfig,
                 FALSE
                 );
    }
    break;

  case EFI_BROWSER_ACTION_FORM_CLOSE:
    if (QuestionId == 0) {
      Status = SetupVariableConfig (
                 &VarStoreGuid,
                 L"SystemConfig",
                 BufferSize,
                 (UINT8 *) SuBrowser->SCBuffer,
                 TRUE
                 );
      Status = SetupVariableConfig (
                 &PasswordConfigGuid,
                 L"PasswordConfig",
                 PasswordConfigBufferSize,
                 (UINT8 *) &mPasswordConfig,
                 TRUE
                 );
    }
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    Status = EFI_SUCCESS;
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    if (QuestionId == KEY_SCAN_F9) {
      Status = SuBrowser->HotKeyCallback (
                            This,
                            Action,
                            QuestionId,
                            Type,
                            Value,
                            ActionRequest
                            );
      SetupVariableConfig (
        &VarStoreGuid,
        L"SystemConfig",
        sizeof(KERNEL_CONFIGURATION),
        (UINT8 *) SuBrowser->SCBuffer,
        FALSE
        );
    }
    //
    // avoid GetQuestionDefault execute ExtractConfig
    //
    return EFI_SUCCESS;

  default:
    break;
  }

  return Status;
}

/**
  Install Security Callback routine.

  @param DriverHandle            Specific driver handle for the call back routine
  @param HiiHandle               Hii hanlde for security page.

  @retval EFI_SUCCESS            Function has completed successfully.
  @retval Other                  Error occurred during execution.

**/
EFI_STATUS
EFIAPI
InstallSecurityCallbackRoutine (
  IN EFI_HANDLE                             DriverHandle,
  IN EFI_HII_HANDLE                         HiiHandle
  )
{
  EFI_STATUS                                Status;
  SETUP_UTILITY_BROWSER_DATA                *SuBrowser;
  EFI_GUID                                  FormsetGuid = FORMSET_ID_GUID_SECURITY;
  EFI_GUID                                  OldFormsetGuid = SYSTEM_CONFIGURATION_GUID;

  Status = GetSetupUtilityBrowserData (&SuBrowser);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mSecurityCallBackInfo = AllocatePool (sizeof (EFI_CALLBACK_INFO));
  if (mSecurityCallBackInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mSecurityCallBackInfo->Signature                    = EFI_CALLBACK_INFO_SIGNATURE;
  mSecurityCallBackInfo->DriverCallback.ExtractConfig = SecurityExtractConfig;
  mSecurityCallBackInfo->DriverCallback.RouteConfig   = SuBrowser->RouteConfig;
  mSecurityCallBackInfo->DriverCallback.Callback      = SecurityCallbackRoutine;
  mSecurityCallBackInfo->HiiHandle                    = HiiHandle;
  if (FeaturePcdGet (PcdH2OFormBrowserSupported)) {
    CopyGuid (&mSecurityCallBackInfo->FormsetGuid, &FormsetGuid);
  } else {
    CopyGuid (&mSecurityCallBackInfo->FormsetGuid, &OldFormsetGuid);
  }

  Status = gBS->InstallProtocolInterface (
                  &DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSecurityCallBackInfo->DriverCallback
                  );
  ASSERT_EFI_ERROR (Status);

  Status = InitSecurityMenu (HiiHandle);

  return Status;

}

/**
  Uninstall Security Callback routine.

  @param DriverHandle            Specific driver handle for the call back routine

  @retval EFI_SUCCESS            Function has completed successfully.
  @return Other                  Error occurred during execution.

**/
EFI_STATUS
EFIAPI
UninstallSecurityCallbackRoutine (
  IN EFI_HANDLE                             DriverHandle
  )
{
  EFI_STATUS       Status;

  if (mSecurityCallBackInfo == NULL) {
    return EFI_SUCCESS;
  }
  Status = gBS->UninstallProtocolInterface (
                  DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mSecurityCallBackInfo->DriverCallback
                  );
  ASSERT_EFI_ERROR (Status);

  gBS->FreePool (mSecurityCallBackInfo);
  mSecurityCallBackInfo = NULL;

  return Status;
}

