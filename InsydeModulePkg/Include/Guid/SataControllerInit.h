//;******************************************************************************
//;* Copyright (c) 1983-2011, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//
// This file contains 'Framework Code' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.
//
/*++

Copyright (c) 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    SataControllerInit.h

Abstract:

    EFI Controller Init Guid

Revision History

--*/

#ifndef _EFI_SATA_CONTROLLER_INIT_H_
#define _EFI_SATA_CONTROLLER_INIT_H_

//
// Global ID for the ControllerHandle Driver Protocol
//
#define EFI_SATA_CONTROLLER_INIT_GUID \
  { \
    0x62463c80, 0x1549, 0x4304, 0x90, 0xd2, 0xe, 0xf4, 0x2e, 0xae, 0x3a, 0x24 \
  }


extern EFI_GUID gEfiSataControllerInitGuid;

#endif
