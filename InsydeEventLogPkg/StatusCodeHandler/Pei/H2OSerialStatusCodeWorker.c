/** @file
  Implementation of H2OSerialStatusCodeWorker module.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/** @file
  Serial I/O status code reporting worker.

  Copyright (c) 2006 - 2012, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include "H2OStatusCodeHandlerPei.h"
#include <Guid/DebugMessageConfigUtilHii.h>

/**
  Convert status code value and extended data to readable ASCII string, send string to serial I/O device.

  @param  PeiServices      An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param  CodeType         Indicates the type of status code being reported.
  @param  Value            Describes the current status of a hardware or
                           software entity. This includes information about the class and
                           subclass that is used to classify the entity as well as an operation.
                           For progress codes, the operation is the current activity.
                           For error codes, it is the exception.For debug codes,it is not defined at this time.
  @param  Instance         The enumeration of a hardware or software entity within
                           the system. A system may contain multiple entities that match a class/subclass
                           pairing. The instance differentiates between them. An instance of 0 indicates
                           that instance information is unavailable, not meaningful, or not relevant.
                           Valid instance numbers start with 1.
  @param  CallerId         This optional parameter may be used to identify the caller.
                           This parameter allows the status code driver to apply different rules to
                           different callers.
  @param  Data             This optional parameter may be used to pass additional data.

  @retval EFI_SUCCESS      Status code reported to serial I/O successfully.

**/
EFI_STATUS
EFIAPI
SerialStatusCodeReportWorker (
  IN CONST  EFI_PEI_SERVICES        **PeiServices,
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          Value,
  IN UINT32                         Instance,
  IN CONST EFI_GUID                 *CallerId,
  IN CONST EFI_STATUS_CODE_DATA     *Data OPTIONAL
  )
{
  CHAR8           *Filename;
  CHAR8           *Description;
  CHAR8           *Format;
  CHAR8           Buffer[MAX_DEBUG_MESSAGE_LENGTH];
  UINT32          ErrorLevel;
  UINT32          LineNumber;
  UINTN           CharCount;
  BASE_LIST       Marker;

  Buffer[0] = '\0';

  if (Data != NULL &&
      ReportStatusCodeExtractAssertInfo (CodeType, Value, Data, &Filename, &Description, &LineNumber)) {
    //
    // Print ASSERT() information into output buffer.
    //
//[-start-171212-IB08400542-modify]//
    if ((PcdGetBool (PcdH2OShowErrorCodeSerialMsgEnable) == FALSE) || (PcdGetBool (PcdH2OShowStatusCodeSerialMsgReady) == FALSE)) {
      return EFI_SUCCESS;
    }
//[-end-171212-IB08400542-modify]//
    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "\n\rPEI_ASSERT!: %a (%d): %a\n\r",
                  Filename,
                  LineNumber,
                  Description
                  );
  } else if (Data != NULL &&
             ReportStatusCodeExtractDebugInfo (Data, &ErrorLevel, &Marker, &Format)) {
    //
    // Print DEBUG() information into output buffer.
    //
    CharCount = AsciiBSPrint (
                  Buffer,
                  sizeof (Buffer),
                  Format,
                  Marker
                  );
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) {
    //
    // Print ERROR information into output buffer.
    //
//[-start-171212-IB08400542-modify]//
    if ((PcdGetBool (PcdH2OShowErrorCodeSerialMsgEnable) == FALSE) || (PcdGetBool (PcdH2OShowStatusCodeSerialMsgReady)== FALSE)) {
      return EFI_SUCCESS;
    }
//[-end-171212-IB08400542-modify]//

    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "ERROR: C%x:V%x I%x",
                  CodeType,
                  Value,
                  Instance
                  );
    
    ASSERT(CharCount > 0);
    
    if (CallerId != NULL) {
      CharCount += AsciiSPrint (
                     &Buffer[CharCount],
                     (sizeof (Buffer) - (sizeof (Buffer[0]) * CharCount)),
                     " %g",
                     CallerId
                     );
    }

    if (Data != NULL) {
      CharCount += AsciiSPrint (
                     &Buffer[CharCount],
                     (sizeof (Buffer) - (sizeof (Buffer[0]) * CharCount)),
                     " %x",
                     Data
                     );
    }

    CharCount += AsciiSPrint (
                   &Buffer[CharCount],
                   (sizeof (Buffer) - (sizeof (Buffer[0]) * CharCount)),
                   "\n\r"
                   );
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_PROGRESS_CODE) {
    //
    // Print PROGRESS information into output buffer.
    //
//[-start-171212-IB08400542-modify]//
    if ((PcdGetBool (PcdH2OShowProgressCodeSerialMsgEnable) == FALSE) || (PcdGetBool (PcdH2OShowStatusCodeSerialMsgReady) == FALSE)) {
      return EFI_SUCCESS;
    }
//[-end-171212-IB08400542-modify]//
    
    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "PROGRESS CODE: V%x I%x\n\r",
                  Value,
                  Instance
                  );
  } else if (Data != NULL &&
             CompareGuid (&Data->Type, &gEfiStatusCodeDataTypeStringGuid) &&
             ((EFI_STATUS_CODE_STRING_DATA *) Data)->StringType == EfiStringAscii) {
    //
    // EFI_STATUS_CODE_STRING_DATA
    //
//[-start-171212-IB08400542-modify]//
    if ((PcdGetBool (PcdH2OShowDebugCodeSerialMsgEnable) == FALSE) || (PcdGetBool (PcdH2OShowStatusCodeSerialMsgReady) == FALSE)) {
      return EFI_SUCCESS;
    }
//[-end-171212-IB08400542-modify]//
    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "%a\n\r",
                  ((EFI_STATUS_CODE_STRING_DATA *) Data)->String.Ascii
                  );
  } else {
    //
    // Code type is not defined.
    //
//[-start-171212-IB08400542-modify]//
    if ((PcdGetBool (PcdH2OShowDebugCodeSerialMsgEnable) == FALSE) || (PcdGetBool (PcdH2OShowStatusCodeSerialMsgReady) == FALSE)) {
      return EFI_SUCCESS;
    }
//[-end-171212-IB08400542-modify]//
    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "Undefined: C%x:V%x I%x\n\r",
                  CodeType,
                  Value,
                  Instance
                  );
  }

  //
  // Call SerialPort Lib function to do print.
  //
  SerialPortWrite ((UINT8 *) Buffer, CharCount);

  return EFI_SUCCESS;
}

/**
 Update Insyde Event Log Policy for related Variables.

 @param[in]  VariablePpi         A pointer to EFI_PEI_READ_ONLY_VARIABLE2_PPI struct pointer.

 @retval EFI_SUCCESS                    Update Policy success.
 @return EFI_ERROR                      Locate gEfiPeiReadOnlyVariable2PpiGuid or execute GetVariable error.
*/
EFI_STATUS
UpdateStatusCodePolicy (
  IN CONST EFI_PEI_READ_ONLY_VARIABLE2_PPI       *VariablePpi
  )
{  
  EFI_STATUS                                Status;  
  UINTN                                     VariableSize;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION   DebugConfig;

//[-start-180724-IB08400617-modify]//
//[-start-180823-IB07400999-modify]//
  Status = PcdSetBool (PcdH2OShowStatusCodeSerialMsgReady, TRUE);
//[-end-180823-IB07400999-modify]//
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-180724-IB08400617-modify]//
  VariableSize = sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION);
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME,
                          &gH2ODebugMessageConfigUtilVarstoreGuid,
                          NULL,
                          &VariableSize,
                          &DebugConfig
                          );
  if (!EFI_ERROR (Status)) {
//[-start-171212-IB08400542-modify]//
//[-start-180718-IB08400617-modify]//
//[-start-180823-IB07400999-modify]//
    Status = PcdSetBool (PcdH2OShowProgressCodeSerialMsgEnable, DebugConfig.ShowProgressCode);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = PcdSetBool (PcdH2OShowErrorCodeSerialMsgEnable,    DebugConfig.ShowErrorCode);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = PcdSetBool (PcdH2OShowDebugCodeSerialMsgEnable,    DebugConfig.ShowDebugCode);
    if (EFI_ERROR (Status)) {
      return Status;
    }
//[-end-180823-IB07400999-modify]//
//[-start-180718-IB08400617-modify]//	
//[-end-171212-IB08400542-modify]//
  } 

  return Status;
}

/**
  Callback function for the notification of ReadOnlyVariable2 PPI.

  @param[in]         PeiServices         A pointer to EFI_PEI_SERVICES struct pointer.

  @retval EFI_SUCCESS                    Update Policy success.
  @return EFI_ERROR                      Locate gEfiPeiReadOnlyVariable2PpiGuid or execute GetVariable error.
*/
EFI_STATUS
ReadOnlyVariable2Callback (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  )
{
  EFI_STATUS                            Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI       *VariablePpi;

  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariablePpi);
  if (EFI_ERROR (Status)) {
    return Status;
  }
 
  Status = UpdateStatusCodePolicy (VariablePpi);
  
  return Status;
}

