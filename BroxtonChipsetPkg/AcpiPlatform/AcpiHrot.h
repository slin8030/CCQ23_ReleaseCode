/*++

Copyright (c)  2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    AcpiHrot.h

Abstract:

    This file describes the contents of the ACPI HROT Table.

 

--*/

#ifndef _ACPI_HROT_UPDATE_H_
#define _ACPI_HROT_UPDATE_H_

#define MAX_NUM_SUB_PAR       0x10

//
// BPDT Table define
//
#define BPDT_SIGNATURE        0x000055AA  // ('B', 'P', 'D', 'T')
#define DIR_HDR_SIGNATURE     SIGNATURE_32('$', 'C', 'P', 'D')

typedef enum {
  bpOemSmip,  ///< 0
  bpCseRbe,   ///< 1
  bpCseBup,   ///< 2
  bpuCode,    ///< 3
  bpIBB,      ///< 4
  bpSBpdt,    ///< 5
  bpObb,      ///< 6 
  bpCseMain,  ///< 7 
  bpIsh,      ///< 8
  bpCseIdlm,  ///< 9
  bpIfpOverride, ///< 10
  bpDebugTokens, ///< 11
  bpUfsPhyConfig,///< 12
  bpUfsGppLunId, ///< 13
  bpPmc,         ///< 14
  bpiUnit,       ///< 15
  bpMaxType = 0x10
} bpdesc_t;

typedef struct {
  UINT32  RegionType;
  UINT32  RegOffset;
  UINT32  RegSize;
} BPDTDSC;

typedef struct {
  UINT32                    Signature;
  UINT16                    DscCount;
  UINT16                    BpdtVer;
  UINT32                    RedundantBlockXorSum;
  UINT32                    IFWIVer;
  UINT64                    Reserved;
  BPDTDSC                   BpdtDscriptor[bpMaxType];
} BPDT_HEADER;

typedef struct {
  CHAR8    EntryName[12];
  UINT32   EntryOffset;
  UINT32   EntrySize;
  UINT32   Reserved;
} PARTDIRENTRY;

typedef struct {
  UINT32                    HeaderMarker;    ///< 0x44504324 ("$CPD")
  UINT32                    NumOfEntries;
  UINT8                     HeaderVersion;
  UINT8                     EntryVersion;
  UINT8                     HeaderLength;
  UINT8                     Checksum;
  UINT32                    SubPartName;
  PARTDIRENTRY              PatDirEntry[MAX_NUM_SUB_PAR];
} DIRECTORY_HEADER;

///
/// Boot Policy Manifest (BPM) data structure
///
typedef struct {
  UINT32                    ExtType;       ///< 0x00000013  (Little-Endian)
  UINT32                    ExtLength;
  UINT32                    NemData;
  UINT32                    IbblHashAlgo;  ///< 0x00000002
  UINT32                    IbblHashSize;  ///< 0x00000020
  UINT8                     IbblHash[0x20];
  UINT32                    IbbmHashAlgo;
  UINT32                    IbbmHashSize;
  UINT8                     IbbmHash[0x20];
  UINT32                    ObbHashAlgo;
  UINT32                    ObbHashSize;
  UINT8                     ObbHash[0x20];
  UINT8                     Reserved[124]; ///< Others data
} BPMDATA;

#endif
