/** @file
  Header file for the Platform ID code.

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef __PLATFORM_ID_H__
#define __PLATFORM_ID_H__

#include <Library/PssLib.h>

// Strap Fw Cfg ID define
#define IO_EXPANDER_I2C_BUS_NO   0x06
#define IO_EXPANDER_SLAVE_ADDR   0x22
#define IO_EXPANDER_INPUT_REG_0  0x00
#define IO_EXPANDER_INPUT_REG_1  0x01
#define IO_EXPANDER_INPUT_REG_2  0x02

EFI_STATUS
EFIAPI
GetFwCfgId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *FwCfgId
  );

EFI_STATUS
EFIAPI
GetBoardIdFabId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  );

//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
EFI_STATUS
EFIAPI
GetEmbeddedBoardIdFabId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  );

EFI_STATUS
EFIAPI
GetIVIBoardIdFabId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  );
//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//

EFI_STATUS
EFIAPI
GetDockId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *DockId
  );

EFI_STATUS
EFIAPI
GetOsSelPss (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *OsSelPss
  );

EFI_STATUS
EFIAPI
GetBomIdPss (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BomIdPss
  );

#endif
