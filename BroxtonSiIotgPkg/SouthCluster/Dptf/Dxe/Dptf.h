/** @file
  Header file for the Dptf driver.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

///
///  Created DPTF Driver to disable DPTF Config Space.
///

#ifndef _DPTF_H
#define _DPTF_H

#include <Library/DebugLib.h>
#include <SaRegs.h>
#include <Library/ScPlatformLib.h>
#include <Library/MmPciLib.h>
#include <Library/S3BootScriptLib.h>

#ifdef FSP_FLAG
#include <Library/FspCommonLib.h>
#include <FspEas.h>
#include <FspsUpd.h>
#endif

#endif //_DPTF_H
