/** @file
  Base TraceHub Lib Null.

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

#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>

/**
  Calculate TraceHub Trace Address.
  Dummy function call, should be implemented by Platform

  @param [in] Thread          Master is being used.
  @param [in] Channel         Channel is being used.
  @param [out] TraceAddress   TraceHub Address for the Master/Channel
                              is being used.

  @retval                     RETURN_UNSUPPORTED  Not implemented.

**/

#ifndef MmPciAddress
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)0xE0000000 + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#endif

RETURN_STATUS
EFIAPI
TraceHubMmioTraceAddress (
  IN  UINT16    Master,
  IN  UINT16    Channel,
  OUT UINT32    *TraceAddress
  )
{
  UINTN   NpkSwBar;
  
  NpkSwBar = MmioRead32(MmPciAddress (0, 0, 0, 2, 0x18));
  
  
  NpkSwBar &= 0xFFFFFFF0;
//[-start-151204-IB02950555-add]/  
  if((NpkSwBar == 0)||(NpkSwBar == 0xFFFFFFF0)){
    *TraceAddress = 0;
    return EFI_UNSUPPORTED;
  }
//[-end-151204-IB02950555-add]/  
  //
  // SW master is 256 by default. Just need to calculate channel.
  //
  NpkSwBar += 13 * 0x40;

  *TraceAddress = (UINT32)NpkSwBar;
  return 0;
}