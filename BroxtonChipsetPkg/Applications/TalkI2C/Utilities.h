//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <Library/UefiLib.h>
#include <Library/BaseLib.h>


BOOLEAN
StringToUint32(
  CONST CHAR16 *pString,
  UINTN * pValue
  );


BOOLEAN
StringToUint16(
  CONST CHAR16 *pString,
  UINT16 * pValue
  );

BOOLEAN
StringToUint8(
  CONST CHAR16 *pString,
  UINT8 * pValue
  )
;


BOOLEAN 
GetWriteArray(
  IN CHAR16 *WriteString,
  OUT UINT8 *CmdWriteBufferLength,
  OUT UINT8 *CmdWriteBuffer
  );

#endif
