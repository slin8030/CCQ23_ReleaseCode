/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/
/*++

Copyright (c) 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  BiosIdLib.h

Abstract:

  BIOS ID library definitions.

  This library provides functions to get BIOS ID, VERSION, DATE and TIME

--*/

#ifndef _BIOS_ID_LIB_H_
#define _BIOS_ID_LIB_H_

//
// Include files
//
//#include "Tiano.h"

//
// BIOS ID string format:
//
// $(BOARD_ID)$(BOARD_REV).$(OEM_ID).$(VERSION_MAJOR).$(BUILD_TYPE)$(VERSION_MINOR).YYMMDDHHMM
//
// Example: "TRFTCRB1.86C.0008.D03.0506081529"
//
#pragma pack(1)

typedef struct {
  CHAR16  BoardId[7];               // "TRFTCRB"
  CHAR16  BoardRev;                 // "1"
  CHAR16  Dot1;                     // "."
  CHAR16  OemId[3];                 // "86C"
  CHAR16  Dot2;                     // "."
  CHAR16  VersionMajor[4];          // "0008"
  CHAR16  Dot3;                     // "."
  CHAR16  BuildType;                // "D"
  CHAR16  VersionMinor[2];          // "03"
  CHAR16  Dot4;                     // "."
  CHAR16  TimeStamp[10];            // "YYMMDDHHMM"
  CHAR16  NullTerminator;           // 0x0000
} BIOS_ID_STRING;
//[-start-151228-IB03090424-remove]//
//#if (BIOS_2MB)
//  #define MEM_IFWIVER_START           0x1E0000
//#else
//  #define MEM_IFWIVER_START           0x7E0000
//#endif
//
//#define MEM_IFWIVER_LENGTH          0x1000
//typedef struct _MANIFEST_OEM_DATA{
//  UINT32         Signature;
//  unsigned char  FillNull[0x89];
//  UINT32         IFWIVersionLen;
//  unsigned char  IFWIVersion[32];
//}MANIFEST_OEM_DATA;
//[-end-151228-IB03090424-remove]//

//
// A signature precedes the BIOS ID string in the FV to enable search by external tools.
//
typedef struct {
  UINT8           Signature[8];     // "$IBIOSI$"
  BIOS_ID_STRING  BiosIdString;     // "TRFTCRB1.86C.0008.D03.0506081529"
} BIOS_ID_IMAGE;

#pragma pack()

EFI_STATUS
GetBiosId (
  OUT BIOS_ID_IMAGE     *BiosIdImage
  );
/*++
Description:

  This function returns BIOS ID by searching HOB or FV.

Arguments:

  BiosIdImage - The BIOS ID got from HOB or FV
  
Returns:

  EFI_SUCCESS - All parameters were valid and BIOS ID has been got.

  EFI_NOT_FOUND - BiosId image is not found, and no parameter will be modified.

  EFI_INVALID_PARAMETER - The parameter is NULL
     
--*/

EFI_STATUS
GetBiosVersionDateTime (
  OUT CHAR16    *BiosVersion, OPTIONAL
  OUT CHAR16    *BiosReleaseDate, OPTIONAL
  OUT CHAR16    *BiosReleaseTime OPTIONAL
  );
/*++
Description:

  This function returns the Version & Release Date and Time by getting and converting
  BIOS ID.

Arguments:

  BiosVersion - The Bios Version out of the conversion

  BiosReleaseDate - The Bios Release Date out of the conversion

  BiosReleaseTime - The Bios Release Time out of the conversion
  
Returns:

  EFI_SUCCESS - BIOS Version & Release Date and Time have been got successfully.

  EFI_NOT_FOUND - BiosId image is not found, and no parameter will be modified.

  EFI_INVALID_PARAMETER - All the parameters are NULL.
     
--*/

#endif
