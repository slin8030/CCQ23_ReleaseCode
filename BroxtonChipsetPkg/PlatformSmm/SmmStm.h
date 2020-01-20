/*++

Copyright (c)  1999 - 2005 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Platform.h

Abstract:

  Header file for 

 

++*/

#ifndef _SMM_STM_H
#define _SMM_STM_H


#define MAX_MSEG_SIZE                   0x80000

#define EFI_MSR_IA32_MSEG                   0x9B


// SMI interface return codes     
#define LT_SMM_SUCCESS                      0
#define ERROR_LT_SMM_UNSUPPORTED            0x80000001
#define ERROR_LT_SMM_MSEG_SIZE_TOO_LARGE    0x80000002
#define ERROR_LT_SMM_VMX_ENABLED            0x80000003
#define ERROR_LT_SMM_STM_ALREADY_LOADED     0x80000004
#define ERROR_LT_SMM_STM_SIZE_TOO_LARGE     0x80000005  
#define ERROR_LT_SMM_STM_REJECTED           0x80000006

#define LT_STM_METADATA_GUID_SIZE           16

typedef struct {
  UINT64                        RangeBase;
  UINT32                        RangeSize;
  UINT32                        Reserved;
} LT_SMRAM_RANGE;


typedef struct {
  UINT8                         VendorId[16];
  UINT8                         StmId[16];
  UINT64                        Version;
  CHAR8                         StmName[64];
  UINT8                         Hash[20];
  UINT32                        Reserved;
} STM_INFO;


typedef struct {
  UINT64                        MsegBase;
  UINT32                        MsegSize;
  UINT32                        MaxMsegSize;
  UINT32                        Reserved;
  UINT32                        RebootRequired:1;
  UINT32                        StmIsPresent:1;
  UINT32                        ReservedFlags:30;
  STM_INFO                      StmInfo;
} LT_MSEG_STATUS;


typedef struct {
  UINT8                         Guid[LT_STM_METADATA_GUID_SIZE];
  UINT32                        Signature;
  UINT32                        MetaDataLength;
} LT_STM_METADATA_ENTRY;


typedef struct {
  STM_INFO                      StmInfo;
  UINT64                        StmOsSectionPtr;
  UINT32                        Reserved;
  UINT32                        StmPackageSize;
} LT_SMM_STM_DESCRIPTOR;


typedef struct {
  UINT64                        BaseAddress;
  UINT32                        StmSectionLength;
  UINT32                        TerminalSection:1;
  UINT32                        NextArrayPointer:1;
  UINT32                        ReservedFlags:30;
} LT_STM_OS_SECTION;

#define MC_MMIO_POC             0xF4    //Power-On Configuration
#define MC_MMIO_POCRL           0xFA    //Power-On Configuration Reset and Lock

#endif

