/** @file
  EC hardware library functions and definitions.

  This library provides basic Embedded Controller Hardware interface.
  It is keeped simple enough and used by other BaseEc libraries to abstacrt the usage

@copyright
 Copyright (c) 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains 'Framework Code' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may not be modified, except as allowed by
 additional terms of your license agreement.

@par Specification Reference:
**/


#ifndef _BASE_EC_HW_LIB_H_
#define _BASE_EC_HW_LIB_H_

#include <Base.h>
#include <Uefi.h>
#include <Library/EcCommands.h>

typedef enum {
  LpcInterface = 0,
  SpiInterface,
  EspiInterface,
  I2cInterface,
  EcInterfaceMax
} EC_INTERFACE_TYPE;

typedef enum {
  EcId0 = 0,
  EcIdMax
} EC_ID;

#endif
