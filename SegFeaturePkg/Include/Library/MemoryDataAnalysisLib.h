/** @file
  Memory Data Analysis library header file.

;******************************************************************************
;* Copyright (c) 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __MEMORY_DATA_ANALYSIS_LIB_H__
#define __MEMORY_DATA_ANALYSIS_LIB_H__

/**
  Analyze memory data and print them on serial port

  If each memory data(sizeof (UINTN)) is an address in the code section of a loaded image, 
  it will show the image information. 
  The actual analysis length will be aligned to sizeof (UINTN).

  Note: If the StartAddress is not aligned to sizeof (UINTN), 
        it will get incorrect image information.

  @param [in]  StartAddress     The start address of memory 
  @param [in]  Length           The length, in bytes, of memory 
  
  @retval EFI_SUCCESS           Analyze memory data successfully.
  
**/
EFI_STATUS
MemoryDataAnalysis (
  IN  EFI_PHYSICAL_ADDRESS    StartAddress,
  IN  UINTN                   Length
  );

#endif
