//;******************************************************************************
//;* Copyright (c) 1983-2015, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************

#ifndef _SIO_GET_PCD_H_
#define _SIO_GET_PCD_H_

/**
  Retrieves a size to a PCD token buffer based on a token name.

  Returns a size to the buffer for the token specified by TokenName.
  If TokenName is not a valid token in the token space, then the module will not build.

  @param   TokenName  The name of the PCD token to retrieve a current value for.

  @return  A size of the buffer.

**/
#ifndef PcdGetSize
#define PcdGetSize(TokenName)                _PCD_PATCHABLE_##TokenName##_SIZE
#endif

//
// Feature Pcd
//
#define PCD_SIO_SETUP_SUPPORT   FeaturePcdGet (PcdSioNct5104dSetup)
#define PCD_SIO_UPDATE_ASL      FeaturePcdGet (PcdSioNct5104dUpdateAsl)
#define PCD_SIO_ENABLE_GPIO_PEI FeaturePcdGet (PcdSioNct5104dEnableGpioPei)

//
// Pointer Pcd
//
#define PCD_SIO_CONFIG_TABLE                PcdGetPtr (PcdSioNct5104dCfg)
#define PCD_SIO_EXTENSIVE_CONFIG_TABLE      PcdGetPtr (PcdSioNct5104dExtensiveCfg)
#define PCD_SIO_VARIABLE_NAME               PcdGetPtr (PcdSioNct5104dSetupStr)
#define PCD_SIO_PEI_CUSTOMIZE_SETTING_TABLE PcdGetPtr (PcdSioNct5104dPeiCustomizeSettingTable)
#define PCD_SIO_DXE_CUSTOMIZE_SETTING_TABLE PcdGetPtr (PcdSioNct5104dDxeCustomizeSettingTable)
#define PCD_SIO_PORT80_TO_UART              PcdGetPtr (PcdSioNct5104dPort80toUart)

//
// Size Pcd
//
#define PCD_SIO_PEI_CUSTOMIZE_SETTING_TABLE_SIZE PcdGetSize (PcdSioNct5104dPeiCustomizeSettingTable)
#define PCD_SIO_DXE_CUSTOMIZE_SETTING_TABLE_SIZE PcdGetSize (PcdSioNct5104dDxeCustomizeSettingTable)

#endif
