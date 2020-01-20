/** @file
  CpuTraining.h
  Library specific macros and function declarations used within the MMRC.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _CPUTRAINING_H_
#define _CPUTRAINING_H_

#include "Mmrc.h"

#if !CPGC_API

#ifdef MINIBIOS
#define Shr64(Value, Shift) LShiftU64 (Value, Shift)
#define Shl64(Value, Shift) LRightU64 (Value, Shift)
#endif

/**
  Fill cache array

  @param[in,out]   MrcData
  @param[in]       Ptr
  @param[in]       Data
  @param[in]       Offset

  @return     MMRC_STATUS
**/
MMRC_STATUS
FillCacheArray (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         *Ptr,
  IN        UINT8         Data,
  IN        UINT8         *Offset
  );

/**
  Build cache arrays

  @param[in,out]   MrcData
  @param[in]       CL
  @param[in]       Patterns
  @param[in]       NumberPatterns
  @param[in]       NumCL

  @return     MMRC_STATUS
**/
MMRC_STATUS
BuildCacheArrays (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         *CL,
  IN        UINT8         *Patterns,
  IN        UINT32        NumberPatterns,
  IN        UINT32        *NumCL
  );

/**
  Enable 16K MTRR

  @param[in,out]   MrcData
  @param[in]       TargetAddress
  @param[in]       Msr

  @return     MMRC_STATUS
**/
MMRC_STATUS
Enable16KMTRR (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        TargetAddress,
  IN        UINT32        Msr
  );

/**
  Load Xmm03 with pattern

  @param[in,out]   MrcData
  @param[in]       CacheLines

  @return     MMRC_STATUS
**/
MMRC_STATUS
LoadXmm03WithPattern (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         *CacheLines
  );

/**
  Burst out cache lines via Xmm

  @param[in,out]   MrcData
  @param[in]       Address
  @param[in]       CacheLineRepeatCount

  @return     MMRC_STATUS
**/
MMRC_STATUS
BurstOutCachelinesViaXmm (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        Address,
  IN        UINT32        CacheLineRepeatCount
  );

/**
  Compare cache with Xmm47

  @param[in,out]   MrcData
  @param[in]       TargetAddress
  @param[in]       CacheLineRepeatCount
  @param[in]       EncodedFailPtr

  @return     MMRC_STATUS
**/
MMRC_STATUS
CompareCacheWithXmm47 (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        TargetAddress,
  IN        UINT32        CacheLineRepeatCount,
  IN        UINT32        *EncodedFailPtr
  );

/**
  Read ram into cache

  @param[in,out]   MrcData
  @param[in]       TargetAddress
  @param[in]       CacheLineRepeatCount

  @return     MMRC_STATUS
**/
MMRC_STATUS
ReadRamIntoCache (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        TargetAddress,
  IN        UINT32        CacheLineRepeatCount
  );

/**
  Generate pattern

  @param[in,out]   MrcData
  @param[in]       VictimPattern
  @param[in]       AggressorPattern
  @param[in]       ByteShift
  @param[in]       BitShift
  @param[in]       VictimRepeat
  @param[in]       EvenOddFlag
  @param[in]       BufferPv

  @return     MMRC_STATUS
**/
MMRC_STATUS
GeneratePattern (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        VictimPattern,
  IN        UINT32        AggressorPattern,
  IN        UINT8         ByteShift[],
  IN        UINT8         BitShift,
  IN        UINT8         VictimRepeat,
  IN        UINT8         EvenOddFlag,
  IN        VOID          *BufferPv
  );

/**
  Fill golden buffer

  @param[in,out]   MrcData
  @param[in]       PatternIndex
  @param[in]       BufferAddress

  @return     MMRC_STATUS
**/
MMRC_STATUS
FillGoldenBuffer (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        *PatternIndex,
  IN        UINT32        BufferAddress
  );

/**
  Transfer memory

  @param[in,out]   MrcData
  @param[in]       DstAddr
  @param[in]       SrcAddr
  @param[in]       CacheLines
  @param[in]       InvertFlag
  @param[in]       AllOnes

  @return     MMRC_STATUS
**/
MMRC_STATUS
TransferMemory (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         *DstAddr,
  IN        UINT8         *SrcAddr,
  IN        UINT32        CacheLines,
  IN        UINT8         InvertFlag,
  IN        UINT32        *AllOnes
  );

/**
  Compare golden with DRAM patterns

  @param[in,out]   MrcData
  @param[in]       GoldenPatternAddress
  @param[in]       Channel
  @param[in]       Rank
  @param[in]       CompareResults

  @return     MMRC_STATUS
**/
MMRC_STATUS
CompareGoldenWithDRAMPatterns (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        GoldenPatternAddress,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT16        *CompareResults
  );

/**
  CompareMemory

  @param[in,out]   MrcData
  @param[in]       GoldenAddress
  @param[in]       TestAddr
  @param[in]       CacheLines
  @param[in]       AllOnes
  @param[in]       InvertFlag
  @param[in]       Results

  @return     MMRC_STATUS
**/
MMRC_STATUS
CompareMemory (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         *GoldenAddress,
  IN        UINT8         *TestAddr,
  IN        UINT32        CacheLines,
  IN        UINT32        *AllOnes,
  IN        UINT8         InvertFlag,
  IN        UINT16        *Results
  );

/**
  DisableMTRR

  @param[in,out]   MrcData
  @param[in]       Msr

  @return     MMRC_STATUS
**/
MMRC_STATUS
DisableMTRR (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        Msr
  );

#ifndef MINIBIOS
#if !defined SIM && !defined JTAG

/**
  64-bit Shift Right

  @param[in]   Value      Value to be left shifted
  @param[in]   Shift      Number of Shifts to be done to the
                          right
  @return      Value      Shifted
**/
UINT64
Shr64 (
  IN        UINT64        Value,
  IN        UINT8         Shift
  );

/**
  64-bit Shift Left

  @param[in]   Value      Value to be left shifted
  @param[in]   Shift      Number of Shifts to be done to the
                          left
  @retval      Value      Shifted
**/
UINT64
Shl64 (
  IN        UINT64        Value,
  IN        UINT8         Shift
  );

#endif // SIM
#endif // MINIBIOS
#endif // CPGC_API

#endif // _CPUTRAINING_H_
