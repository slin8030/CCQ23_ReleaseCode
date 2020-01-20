/** @file
    The header file for variable check service.

;******************************************************************************
;* Copyright (c) 2015 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _INTERNAL_VARIABLE_CHECK_H_
#define _INTERNAL_VARIABLE_CHECK_H_
#include "Variable.h"


typedef struct {
  LIST_ENTRY                    Link;
  EFI_GUID                      Guid;
  VAR_CHECK_VARIABLE_PROPERTY   VariableProperty;
  //CHAR16                        *Name;
} VAR_CHECK_VARIABLE_ENTRY;

typedef
EFI_STATUS
(EFIAPI *INTERNAL_VAR_CHECK_FUNCTION) (
  IN VAR_CHECK_VARIABLE_PROPERTY    *Propery,
  IN UINTN                          DataSize,
  IN VOID                           *Data
  );

typedef struct {
  CHAR16                        *Name;
  VAR_CHECK_VARIABLE_PROPERTY   VariableProperty;
  INTERNAL_VAR_CHECK_FUNCTION   CheckFunction;
} UEFI_DEFINED_VARIABLE_ENTRY;

/**
  Internal SetVariable check.

  @param[in] VariableName       Name of Variable to set.
  @param[in] VendorGuid         Variable vendor GUID.
  @param[in] Attributes         Attribute value of the variable.
  @param[in] DataSize           Size of Data to set.
  @param[in] Data               Data pointer.

  @retval EFI_SUCCESS           The SetVariable check result was success.
  @retval EFI_INVALID_PARAMETER An invalid combination of attribute bits, name, and GUID were supplied,
                                or the DataSize exceeds the minimum or maximum allowed,
                                or the Data value is not following UEFI spec for UEFI defined variables.
  @retval EFI_WRITE_PROTECTED   The variable in question is read-only.
  @retval Others                The return status from check handler.

**/
EFI_STATUS
EFIAPI
InternalVarCheckSetVariableCheck (
  IN CHAR16     *VariableName,
  IN EFI_GUID   *VendorGuid,
  IN UINT32     Attributes,
  IN UINTN      DataSize,
  IN VOID       *Data
  );

/**
  Internal variable property get.

  @param[in]  Name              Pointer to the variable name.
  @param[in]  Guid              Pointer to the vendor GUID.
  @param[out] VariableProperty  Pointer to the output variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was got successfully.
  @retval EFI_NOT_FOUND         The property of variable specified by the Name and Guid was not found.

**/
EFI_STATUS
EFIAPI
InternalVarCheckVariablePropertyGet (
  IN  CHAR16                         *Name,
  IN  EFI_GUID                       *Guid,
  OUT VAR_CHECK_VARIABLE_PROPERTY   *VariableProperty
  );

/**
  Register SetVariable check handler.

  @param[in] Handler            Pointer to check handler.

  @retval EFI_SUCCESS           The SetVariable check handler was registered successfully.
  @retval EFI_INVALID_PARAMETER Handler is NULL.
  @retval EFI_ACCESS_DENIED     EFI_END_OF_DXE_EVENT_GROUP_GUID or EFI_EVENT_GROUP_READY_TO_BOOT has
                                already been signaled.
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource for the SetVariable check handler register request.
  @retval EFI_UNSUPPORTED       This interface is not implemented.
                                For example, it is unsupported in VarCheck protocol if both VarCheck and SmmVarCheck protocols are present.

**/
EFI_STATUS
EFIAPI
VarCheckRegisterSetVariableCheckHandler (
  IN VAR_CHECK_SET_VARIABLE_CHECK_HANDLER   Handler
  );

/**
  Variable property set.

  @param[in] Name               Pointer to the variable name.
  @param[in] Guid               Pointer to the vendor GUID.
  @param[in] VariableProperty   Pointer to the input variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was set successfully.
  @retval EFI_INVALID_PARAMETER Name, Guid or VariableProperty is NULL, or Name is an empty string,
                                or the fields of VariableProperty are not valid.
  @retval EFI_ACCESS_DENIED     EFI_END_OF_DXE_EVENT_GROUP_GUID or EFI_EVENT_GROUP_READY_TO_BOOT has
                                already been signaled.
  @retval EFI_OUT_OF_RESOURCES  There is not enough resource for the variable property set request.

**/
EFI_STATUS
EFIAPI
VarCheckVariablePropertySet (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *Guid,
  IN VAR_CHECK_VARIABLE_PROPERTY    *VariableProperty
  );

/**
  Variable property get.

  @param[in]  Name              Pointer to the variable name.
  @param[in]  Guid              Pointer to the vendor GUID.
  @param[out] VariableProperty  Pointer to the output variable property.

  @retval EFI_SUCCESS           The property of variable specified by the Name and Guid was got successfully.
  @retval EFI_INVALID_PARAMETER Name, Guid or VariableProperty is NULL, or Name is an empty string.
  @retval EFI_NOT_FOUND         The property of variable specified by the Name and Guid was not found.

**/
EFI_STATUS
EFIAPI
VarCheckVariablePropertyGet (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *Guid,
  OUT VAR_CHECK_VARIABLE_PROPERTY   *VariableProperty
  );

/**
  Convert all of pointer to variable check information to virtual address.
**/
VOID
ConvertVariableCheckInfo (
  VOID
  );

/**
  This function uses to send SMI to create vriable check list in SMM mode.
**/
VOID
CreateVariableCheckListThroughSmi (
  VOID
  );

/**
  This function uses to create variable check list.

  @return EFI_SUCCESS    Create variable check list in SMM RAM successfully.
  @return Other          Any error occured while creating variable check list in SMM RAM.
**/
EFI_STATUS
SmmCreateVariableCheckList (
  VOID
  );

/**
  Update the variable check property of AuditMode variable.

  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of AuditMode variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find AuditMode variable in variable check list.
**/
EFI_STATUS
UpdateAuditModeProperty (
  IN CONST UINT8      Property
  );

/**
  Update the variable check property of DeployedMode variable.

  @param[in] Property            Input variable property.

  @return EFI_SUCCESS            Update variable check property of DeployedMode variable successfully.
  @return EFI_INVALID_PARAMETER  Input variable check propery is unsupported.
  @return EFI_NOT_FOUND          Cannot find DeployedMode variable in variable check list.
**/
EFI_STATUS
UpdateDeployedModeProperty (
  IN CONST UINT8      Property
  );

/**
  This fucnitons uses to invoke SMM mode update variable property.

  @retval EFI_SUCCESS     Update variable property in SMM mode successful.
  @return Other           Any erro occured while updating variable property in SMM mode.
**/
EFI_STATUS
SmmUpdateVariablePropertySmi (
  VOID
  );

//[-start-161117-IB07400815-add]//
EFI_STATUS
SmmInternalQueryVariableInfo (
  VOID
  );
//[-end-161117-IB07400815-add]//

extern UEFI_DEFINED_VARIABLE_ENTRY mGlobalVariableList[];
#endif
