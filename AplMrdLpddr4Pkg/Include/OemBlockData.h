/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _OEM_BLOCK_DATA_H
#define _OEM_BLOCK_DATA_H

#include <ChipsetOemBlockData.h>


#pragma pack(1)

//
// Size of OEM_BLOCK_DATA is fixed 400 bytes.
//
typedef struct {  
  CHIPSET_OEM_BLOCK_DATA   ChipsetOemBlockData;   //352 bytes
  //
  // Reserve for OEM
  //
  UINT8                     Reserved[48];

} OEM_BLOCK_DATA;

#pragma pack()

#endif

