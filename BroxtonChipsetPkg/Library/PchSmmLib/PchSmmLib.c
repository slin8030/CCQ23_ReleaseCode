/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  PCH Smm Library Services that implements both S/W SMI generation and detection. 

  Copyright (c) 2007 - 2010, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/


#include "CommonHeader.h"



/**
  Triggers a run time or boot time SMI.  

  This function triggers a software SMM interrupt and set the APMC status with an 8-bit Data.

  @param  Data                 The value to set the APMC status.

**/
VOID
InternalTriggerSmi (
  IN UINT8                     Data
  )
{
  ASSERT(FALSE);
}


/**
  Triggers an SMI at boot time.  

  This function triggers a software SMM interrupt at boot time.

**/
VOID
EFIAPI
TriggerBootServiceSoftwareSmi (
  VOID
  )
{
  ASSERT(FALSE);
}


/**
  Triggers an SMI at run time.  

  This function triggers a software SMM interrupt at run time.

**/
VOID
EFIAPI
TriggerRuntimeSoftwareSmi (
  VOID
  )
{
  ASSERT(FALSE);
}


/**
  Gets the software SMI data.  

  This function tests if a software SMM interrupt happens. If a software SMI happens, 
  it retrieves the SMM data and returns it as a non-negative value; otherwise a negative
  value is returned. 

  @return Data                 The data retrieved from SMM data port in case of a software SMI;
                               otherwise a negative value.

**/
INTN
InternalGetSwSmiData (
  VOID
  )
{
  ASSERT(FALSE);
  return -1;
}  


/**
  Test if a boot time software SMI happened.  

  This function tests if a software SMM interrupt happened. If a software SMM interrupt happened and
  it was triggered at boot time, it returns TRUE. Otherwise, it returns FALSE.

  @retval TRUE   A software SMI triggered at boot time happened.
  @retval FLASE  No software SMI happened or the software SMI was triggered at run time.

**/
BOOLEAN
EFIAPI
IsBootServiceSoftwareSmi (
  VOID
  )
{
  ASSERT(FALSE);
  return FALSE;
}


/**
  Test if a run time software SMI happened.  

  This function tests if a software SMM interrupt happened. If a software SMM interrupt happened and
  it was triggered at run time, it returns TRUE. Otherwise, it returns FALSE.

  @retval TRUE   A software SMI triggered at run time happened.
  @retval FLASE  No software SMI happened or the software SMI was triggered at boot time.

**/
BOOLEAN
EFIAPI
IsRuntimeSoftwareSmi (
  VOID
  )
{
  ASSERT(FALSE);
  return FALSE;
}



/**

  Clear APM SMI Status Bit; Set the EOS bit. 
  
**/
VOID
EFIAPI
ClearSmi (
  VOID
  )
{

  UINT16                       PmBase;

  //
  // Get PMBase
  //
  PmBase = PcdGet16 (PcdPchAcpiIoPortBaseAddress);

  //
  // Clear the APM SMI Status Bit
  //
  IoWrite16 (PmBase + R_ACPI_SMI_STS, B_ACPI_APM_STS);

  //
  // Set the EOS Bit
  //
  IoOr32 (PmBase + R_ACPI_SMI_EN, B_ACPI_EOS);
}

