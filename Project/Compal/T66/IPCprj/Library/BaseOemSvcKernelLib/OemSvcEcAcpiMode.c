/** @file
  Provide hook function for OEM to implement EC ACPI Mode. 

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/BaseOemSvcKernelLib.h>
#include <CompalECLib.h>

/**
  Platform specific function to enable/disable EC ACPI Mode

  @param[in]      EnableEcAcpiMode      Boolean to enable/disable ACPI mode.

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
  @retval EFI_MEDIA_CHANGED  The value of IN OUT parameter is changed. 
**/
EFI_STATUS
OemSvcEcAcpiMode (
  IN  BOOLEAN         EnableEcAcpiMode
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  UINT8         EcCommand;

  if (EnableEcAcpiMode) {
    EcCommand = EC_CMD_SYSTEM_NOTIFICATION_ACPI_ENABLE;
  } else {
    EcCommand = EC_CMD_SYSTEM_NOTIFICATION_ACPI_DISABLE;
  }
  CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_SYSTEM_NOTIFICATION,EcCommand);

  return EFI_UNSUPPORTED;
}
