/** @file
  Header file for the SMIP parser code.

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

#ifndef __SMIP_HEADER_H__
#define __SMIP_HEADER_H__

//[-start-160216-IB03090424-modify]//
EFI_STATUS
SmipInit(
  IN VOID*  FitHeaderPtr,
  IN UINT8  BoardId
);
//[-end-160216-IB03090424-modify]//

#endif
