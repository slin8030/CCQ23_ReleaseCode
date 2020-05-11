/** @file
  This function provides an interface to hook GenericRouteConfig.
   
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/DxeOemSvcChipsetLib.h>
#include <CompalEcLib.h>
#include <SetupConfig.h>
#include <ProjectDefinition.h>
//
// Fix compiler error
//
#ifdef CHIPSET_CONFIGURATION
  #undef CHIPSET_CONFIGURATION
  #include <ChipsetSetupConfig.h>
#endif
/**
  This function provides an interface to hook GenericRouteConfig.

  @param[in,out]     ScBuffer            A pointer to CHIPSET_CONFIGURATION struct.
  @param[in]         BufferSize          System configuration size.
  @param[in]         ScuRecord           The bit mask of the currently SCU record.
                                           Bit 0 = 1 (SCU_ACTION_LOAD_DEFAULT), It indicates system do load default action.

  @retval            EFI_UNSUPPORTED     This function is a pure hook; Chipset code don't care return status.
  @retval            EFI_SUCCESS         This function is a pure hook; Chipset code don't care return status. 
**/
EFI_STATUS
OemSvcHookRouteConfig (
  IN OUT CHIPSET_CONFIGURATION          *ScBuffer, 
  IN     UINT32                         BufferSize,
  IN     UINT32                         ScuRecord
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/
  CompalECWriteCmdTwoByte(EC_MEMORY_MAPPING_PORT, 0x59, 0xFA, ((SYSTEM_CONFIGURATION *)(UINTN)ScBuffer)->ATX_Flag);

  return EFI_UNSUPPORTED;
}
