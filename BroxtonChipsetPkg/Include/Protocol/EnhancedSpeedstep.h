//
// This file contains 'Framework Code' and is licensed as such 
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.                 
//
/*++

Copyright (c)  2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  EnhancedSpeedstep.h

Abstract:

  Enhanced Speedstep protocol 

  The Enhanced Speedstep Protocol support.   
--*/

#ifndef _ENHANCED_SPEEDSTEP_H_
#define _ENHANCED_SPEEDSTEP_H_

#define ENHANCED_SPEEDSTEP_PROTOCOL_GUID \
  { \
    0x91a1ddcf, 0x5374, 0x4939, 0x89, 0x51, 0xd7, 0x29, 0x3f, 0x1a, 0x78, 0x6f \
  }

typedef struct _ENHANCED_SPEEDSTEP_PROTOCOL ENHANCED_SPEEDSTEP_PROTOCOL;

typedef struct {
  UINT8   Version;      // EIST State format
  UINT8   Size;         // Size of element
  UINT32  RatioStep;    // Step
  UINT32  MinRatio;     // Calculated min ratio
  UINT32  MaxRatio;     // Calculated max ratio
  UINT32  MinCoreFreq;  // Calculated min freq
  UINT32  MaxCoreFreq;  // Calculated max freq
  UINT32  MinPower;     // Calculated min power
  UINT32  MaxPower;     // Calculated max power
  UINT32  NumStates;    // Number of states 
} EIST_INFORMATION;

typedef struct {
  UINT32  CoreFrequency;
  UINT32  Power;
  UINT32  TransitionLatency;
  UINT32  BusMasterLatency;
  UINT32  Control;
  UINT32  Status;
} EFI_ACPI_CPU_PSS_STATE;

typedef
EFI_STATUS
(EFIAPI *GET_EIST_TABLE) (
  IN    ENHANCED_SPEEDSTEP_PROTOCOL    *This,
  OUT   EIST_INFORMATION               **EistInformation,
  OUT   VOID                           **PssStates
  );

struct _ENHANCED_SPEEDSTEP_PROTOCOL {
  UINT32              ProcApicId;
  GET_EIST_TABLE      GetEistTable;
};

//
// There will be an instance of this protocol for every processor
// in the system.  ProcNumber is used to manage all the different
// processors in the system and passed into the MP protocol
// to run code streams on application processors
//
extern EFI_GUID gEnhancedSpeedstepProtocolGuid;

#endif
