/** @file
  MMRC.h
  The external header file that all projects must include in order to port the MMRC.

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

#ifndef _MMRC_H_
#define _MMRC_H_

#include "MmrcData.h"
#include "Printf.h"
#include "RegAccess.h"
#include "MmrcHooks.h"
#include "MmrcLibraries.h"

#ifndef _H2INC
#ifndef SUSSW
//
// C intrinsic function definitions
//
#define size_t UINT32
#ifndef __GNUC__

void * __cdecl memcpy (void * dst, void *src, UINT32 cnt);
#endif
#endif
#endif


extern MMRC_STATUS MmrcExecuteTask (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex,UINT8 Channel);
extern MMRC_STATUS MmrcExecuteTaskImplementation (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex,UINT8 Channel, BOOLEAN ForceRun, UINT8 MaxChannel);
extern MMRC_STATUS ReceiveEnable (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS VOCTraining (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS FineWriteLeveling (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS CoarseWriteLeveling (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS ReadTraining (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS PeriodicTraining (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS PhyViewTable (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS WriteTraining (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS CommandClockTraining (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex,UINT8 Channel);
extern MMRC_STATUS ScrubMemory (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS MemoryTest (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern void      *MmrcMemset (void *dst, int value, size_t cnt);
extern BOOLEAN    MmrcStringCompare (VOID *str1, VOID *str2, UINT32 cnt);
extern VOID       SaveMrcHostStructureAddress (MMRC_DATA *MrcDataAddress);
extern MMRC_DATA *GetMrcHostStructureAddress ();
extern VOID       MrcDeadLoop ();
extern VOID       GetMaxDq (MMRC_DATA *MrcData);

extern UINT8                                  EarlyCommandWeights[3][3];
extern STROBE_TO_ERROR_MASK                   StrobeToErrorMask[2][MAX_STROBES];
extern TASK_DESCRIPTOR                        InitTasks[];
extern CONST SUBTASK_INDEX_OFFSET             SubtaskOffsetIndex[MAX_SUBTASKS + 1];
extern EXTERNAL_GETSET_FUNCTION               ExternalFunctions[];
extern UINT16                                 GsmGtToMmrcDefine[];
extern UINT16                                 GsmCsnToMmrcDefine[];
extern CONST PFCT_AND_IO_LEVEL_TO_BLUEPRINT   PfctAndIoLevelToBlueprint[MAX_IO_LEVELS];

#if (!defined(MAX_GETSETS) || (defined(MAX_GETSETS) && MAX_GETSETS == MAX_BLUEPRINTS && MAX_BLUEPRINTS > 1))
  extern       UINT16                     HalfClkGenerated[MAX_BLUEPRINTS][MAX_DLL_COUNT][NUM_FREQ];
  extern       UINT8                      ADdllGenerated[MAX_BLUEPRINTS][2];
  extern CONST INDEX_TO_OFFSET_LAYER      DelayIndexToOffsetLayerGenerated[MAX_BLUEPRINTS][MAX_NUM_ALGOS];
  extern CONST INDEX_TO_OFFSET_LAYER      GroupIndexToOffsetLayerGenerated[MAX_BLUEPRINTS][MAX_NUM_GROUP];
  extern CONST INDEX_TO_OFFSET_LAYER      UniqueIndexToOffsetLayerGenerated[MAX_BLUEPRINTS][MAX_NUM_UNIQUE];

  extern CONST GET_SET_ELEMENTS           ElementsGenerated[MAX_BLUEPRINTS][MAX_NUM_ELEMENTS];
  extern CONST UINT8                      NumberAlgosGenerated[MAX_BLUEPRINTS];
  extern CONST UINT8                      NumberOfDependantElementsGenerated[MAX_BLUEPRINTS];
  extern CONST UINT8                      NumberOfDelayElementsPerAlgoGenerated[MAX_BLUEPRINTS];
  extern CONST UINT8                      NumberOfCCElementsGenerated[MAX_BLUEPRINTS];
  extern CONST UINT8                      NumberOfCCRangesGenerated[MAX_BLUEPRINTS];
  extern CONST UINT8                      NumberOfElementsPerAlgoGenerated[MAX_BLUEPRINTS];
  extern CONST UINT8                      NumberOfFrequenciesGenerated[MAX_BLUEPRINTS];
  extern       SIGNAL_INFO                SignalInfoGenerated[MAX_BLUEPRINTS][MAX_SIGNAL_INFO_ELEMENTS];
  extern       UINT16                     GranularityGenerated[MAX_BLUEPRINTS][MAX_DLL_COUNT][NUM_FREQ][MAX_NUM_DELAY_ELEMENTS];
  extern CLOCK_CROSSINGS                  ClockCrossings(UINT8 CcRange);

  #define HalfClk                         HalfClkGenerated [MrcData->CurrentBlueprint]
  #define ADdll                           ADdllGenerated [MrcData->CurrentBlueprint]
  #define DelayIndexToOffsetLayer         DelayIndexToOffsetLayerGenerated [MrcData->CurrentBlueprint]
  #define GroupIndexToOffsetLayer         GroupIndexToOffsetLayerGenerated [MrcData->CurrentBlueprint]
  #define UniqueIndexToOffsetLayer        UniqueIndexToOffsetLayerGenerated [MrcData->CurrentBlueprint]
  #define Elements                        ElementsGenerated [MrcData->CurrentBlueprint]
  #define NumberAlgos                     NumberAlgosGenerated [MrcData->CurrentBlueprint]
  #define NumberOfDependantElements       NumberOfDependantElementsGenerated [MrcData->CurrentBlueprint]
  #define NumberOfDelayElementsPerAlgo    NumberOfDelayElementsPerAlgoGenerated [MrcData->CurrentBlueprint]
  #define NumberOfCCElements              NumberOfCCElementsGenerated [MrcData->CurrentBlueprint]
  #define NumberOfCCRanges                NumberOfCCRangesGenerated [MrcData->CurrentBlueprint]
  #define NumberOfElementsPerAlgo         NumberOfElementsPerAlgoGenerated [MrcData->CurrentBlueprint]
  #define NumberOfFrequencies             NumberOfFrequenciesGenerated [MrcData->CurrentBlueprint]
  #define SignalInfo                      SignalInfoGenerated [MrcData->CurrentBlueprint]
  #define Granularity                     GranularityGenerated [MrcData->CurrentBlueprint]
#else
  extern       UINT16                     HalfClkGenerated[MAX_DLL_COUNT][NUM_FREQ];
  extern       UINT8                      ADdllGenerated[2];
  extern CONST INDEX_TO_OFFSET_LAYER      DelayIndexToOffsetLayerGenerated[MAX_NUM_ALGOS];
  extern CONST INDEX_TO_OFFSET_LAYER      GroupIndexToOffsetLayerGenerated[MAX_NUM_GROUP];
  extern CONST INDEX_TO_OFFSET_LAYER      UniqueIndexToOffsetLayerGenerated[MAX_NUM_UNIQUE];

  extern CONST GET_SET_ELEMENTS           ElementsGenerated[MAX_NUM_ELEMENTS];
  extern CONST UINT8                      NumberAlgosGenerated;
  extern CONST UINT8                      NumberOfDependantElementsGenerated;
  extern CONST UINT8                      NumberOfDelayElementsPerAlgoGenerated;
  extern CONST UINT8                      NumberOfCCElementsGenerated;
  extern CONST UINT8                      NumberOfCCRangesGenerated;
  extern CONST UINT8                      NumberOfElementsPerAlgoGenerated;
  extern CONST UINT8                      NumberOfFrequenciesGenerated;
  extern       SIGNAL_INFO                SignalInfoGenerated[MAX_SIGNAL_INFO_ELEMENTS];
  extern       UINT16                     GranularityGenerated[MAX_DLL_COUNT][NUM_FREQ][MAX_NUM_DELAY_ELEMENTS];
  extern CLOCK_CROSSINGS                  ClockCrossingsGenerated[MAX_NUM_CC_RANGES];

  #define HalfClk                         HalfClkGenerated
  #define ADdll                           ADdllGenerated
  #define DelayIndexToOffsetLayer         DelayIndexToOffsetLayerGenerated
  #define GroupIndexToOffsetLayer         GroupIndexToOffsetLayerGenerated
  #define UniqueIndexToOffsetLayer        UniqueIndexToOffsetLayerGenerated
  #define Elements                        ElementsGenerated
  #define NumberAlgos                     NumberAlgosGenerated
  #define NumberOfDependantElements       NumberOfDependantElementsGenerated
  #define NumberOfDelayElementsPerAlgo    NumberOfDelayElementsPerAlgoGenerated
  #define NumberOfCCElements              NumberOfCCElementsGenerated
  #define NumberOfCCRanges                NumberOfCCRangesGenerated
  #define NumberOfElementsPerAlgo         NumberOfElementsPerAlgoGenerated
  #define NumberOfFrequencies             NumberOfFrequenciesGenerated
  #define SignalInfo                      SignalInfoGenerated
  #define Granularity                     GranularityGenerated
  #define ClockCrossings                  ClockCrossingsGenerated
#endif


#if MAX_BLUEPRINTS > 1
  extern INSTANCE_PORT_MAP                InstancePortMapGenerated[MAX_BLUEPRINTS][MAX_BOXES];
  extern INSTANCE_PORT_OFFSET             InstancePortOffsetGenerated[MAX_BLUEPRINTS][MAX_INSTANCE_PORTS];
  extern UINT8                            ChannelToInstanceMapGenerated[MAX_BLUEPRINTS][MAX_BOXES][MAX_CHANNELS];
  extern CONST FLOORPLAN                  FloorPlanGenerated[MAX_BLUEPRINTS][MAX_CHANNELS][MAX_STROBES];
  extern CONST INT16                      StrobeToRegisterGenerated[MAX_BLUEPRINTS][MAX_STROBES][2];
  extern CONST INT16                      RankToRegisterGenerated[MAX_BLUEPRINTS][MAX_RANKS][2];
  extern CONST UINT8                      UnitStringsGenerated[MAX_BLUEPRINTS][MAX_BOXES][MAX_BOXNAME_LENGTH];

  #define InstancePortMap                 InstancePortMapGenerated [MrcData->CurrentBlueprint]
  #define InstancePortOffset              InstancePortOffsetGenerated [MrcData->CurrentBlueprint]
  #define ChannelToInstanceMap            ChannelToInstanceMapGenerated [MrcData->CurrentBlueprint]
  #define FloorPlan                       FloorPlanGenerated [MrcData->CurrentBlueprint]
  #define StrobeToRegister                StrobeToRegisterGenerated [MrcData->CurrentBlueprint]
  #define RankToRegister                  RankToRegisterGenerated [MrcData->CurrentBlueprint]
  #define UnitStrings                     UnitStringsGenerated [MrcData->CurrentBlueprint]
#else
  extern INSTANCE_PORT_MAP                InstancePortMapGenerated[MAX_BOXES];
  extern INSTANCE_PORT_OFFSET             InstancePortOffsetGenerated[MAX_INSTANCE_PORTS];
  extern UINT8                            ChannelToInstanceMapGenerated[MAX_BOXES][MAX_CHANNELS];
  extern CONST FLOORPLAN                  FloorPlanGenerated[MAX_CHANNELS][MAX_STROBES];
  extern CONST INT16                      StrobeToRegisterGenerated[MAX_STROBES][2];
  extern CONST INT16                      RankToRegisterGenerated[MAX_RANKS][2];
  extern CONST UINT8                      UnitStringsGenerated[MAX_BOXES][MAX_BOXNAME_LENGTH];

  #define InstancePortMap                 InstancePortMapGenerated
  #define InstancePortOffset              InstancePortOffsetGenerated
  #define ChannelToInstanceMap            ChannelToInstanceMapGenerated
  #define FloorPlan                       FloorPlanGenerated
  #define StrobeToRegister                StrobeToRegisterGenerated
  #define RankToRegister                  RankToRegisterGenerated
  #define UnitStrings                     UnitStringsGenerated
#endif

extern       Register                 Registers[];
extern CONST UINT8                    InitData[];
extern       UINT16                   EarlyCommandPatternsRise[];
extern       UINT16                   EarlyCommandPatternsFall[];
extern       UINT16 EarlyCommandPatterns1RiseLP4[];
extern       UINT16 EarlyCommandPatterns1FallLP4[];
extern       UINT16 EarlyCommandPatterns2RiseLP4[];
extern       UINT16 EarlyCommandPatterns2FallLP4[];
extern       UINT32                   EarlyCommandResultsSIM[];
extern       UINT32                   EarlyCommandResultsSIL[MAX_CHANNELS][4];
extern       UINT32                   EarlyCommandResults[];
extern       UINT32                   EarlyCommandResults41[MAX_CHANNELS][4];
extern       UINT32                   EarlyCommandResults48[MAX_CHANNELS][4];
extern       UINT16                   MrsCommandIndex[MAX_RANKS][MR_COUNT];
extern       char                    *OutputStrings[];
extern       UINT8                    NumberEyeMaskRead;
extern       EYEMASK_T                EyeMaskRead[];
extern       UINT8                    NumberEyeMaskWrite;
extern       EYEMASK_T                EyeMaskWrite[];
extern       UINT8                    NumberEyeMaskCmd;
extern       EYEMASK_T                EyeMaskCmd[];
extern       UINT8                    NumberControlKnobsWrite;
#if LPDDR3_SUPPORT==1
extern       CONTROLKNOBS             ControlKnobsWriteLP3[];
extern       CONTROLKNOBS             ControlKnobsReadLP3[];
#endif
#if LPDDR4_SUPPORT==1
extern       CONTROLKNOBS             ControlKnobsWriteLP4[];
extern       CONTROLKNOBS             ControlKnobsReadLP4[];
#endif
#if DDR3_SUPPORT==1
extern       CONTROLKNOBS             ControlKnobsWriteDDR3[];
extern       CONTROLKNOBS             ControlKnobsReadDDR3[];
#endif
extern       UINT8                    NumberControlKnobsRead;
extern       UINT8                    NumberControlKnobsCmdCtl;
extern       CONTROLKNOBS             ControlKnobsCmdCtl[];
extern       CONTROLKNOBS             ControlKnobsCmdCtl_DDR3[];
extern       CONTROLKNOBS             ControlKnobsCmdCtl_LPDDR3[];
extern       UINT8                    ControlVrefsRead[];
extern       UINT8                    ControlVrefsWrite[];
extern       UINT8                    ControlVrefsCmd[];
#endif
