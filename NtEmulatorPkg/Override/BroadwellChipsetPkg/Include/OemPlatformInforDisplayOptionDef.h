/** @file

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _OEM_PLATFORM_INFO_DISPLAY_OPTION_DEF_H_
#define _OEM_PLATFORM_INFO_DISPLAY_OPTION_DEF_H_
#include <Uefi.h>
//#include "UefiIfrLibrary.h"

#define DISPLAY_DISABLE      0
#define DISPLAY_ENABLE       1
#define DISPLAY_INFO_END     2

//
// Function Option
//
#define GET_VBIOS_VERSION_OPTION        DISPLAY_ENABLE , GetVbiosVersion
#define GET_GOP_VERSION_OPTION          DISPLAY_ENABLE , GetGopVersion
#define GET_EC_VERSION_OPTION           DISPLAY_ENABLE , GetEcVersion
#define GET_PCH_REVERSION_OPTION        DISPLAY_ENABLE , GetPchReversionId
#define GET_ME_VERSION_OPTION           DISPLAY_ENABLE , GetMeVersion
#define GET_CPU_MISC_INFOR_OPTION       DISPLAY_ENABLE , GetCpuMiscInfo
#define GET_LAN_PHY_VERSION_OPTION      DISPLAY_DISABLE, GetLanPhyReversion

//
// Get CPU Informaiton Option
//
#define GET_CPU_SPEED_OPTION            DISPLAY_ENABLE , GetCpuSpeedFunc
#define GET_CACHE_INFO_OPTION           DISPLAY_ENABLE , GetCacheInfo
#define GET_L1_DATA_CACHE_OPTION        DISPLAY_ENABLE , GetL1DataCache
#define GET_L1_INSTRUCTION_CACHE_OPTION DISPLAY_ENABLE , GetL1Insruction
#define GET_L2_CACHE_OPTION             DISPLAY_ENABLE , GetL2Cache
#define GET_L3_CACHE_OPTION             DISPLAY_ENABLE , GetL3Cache
#define GET_CPU_ID_OPTION               DISPLAY_ENABLE , GetCpuIdFunc
#define GET_CORE_NUMBER_OPTION          DISPLAY_ENABLE , GetCoreThreadNumFunc
#define GET_CPU_STEPPING_OPTION         DISPLAY_ENABLE , GetCpuSteppingFunc
#define GET_MICROCODE_VERSION_OPTION    DISPLAY_ENABLE , GetMicrocodeVersion
#define GET_TXT_CAPABILITY_OPTION       DISPLAY_ENABLE , GetTxtCapability
#define GET_VTD_CAPABILITY_OPTION       DISPLAY_ENABLE , GetVtdCapability
#define GET_VTX_CAPABILITY_OPTION       DISPLAY_ENABLE , GetVtxCapability
#define GET_GT_INFO_OPTION              DISPLAY_ENABLE , GetGTInfo

//
// Get EC Information Option
//
#define GET_EC_VERSION_ID_OPTION        DISPLAY_ENABLE , GetEcVerIdFunc
#define GET_BOARD_ID_OPTION             DISPLAY_ENABLE , GetBoardFunc
#define GET_FAB_ID_OPTION               DISPLAY_ENABLE , GetFabFunc

//
// Platform Information Display Option Table
//
#define PLATFORM_INFO_DISPLAY_OPTION_TABLE_LIST     {GET_CPU_MISC_INFOR_OPTION}, \
                                                    {GET_PCH_REVERSION_OPTION}, \
                                                    {GET_VBIOS_VERSION_OPTION}, \
                                                    {GET_GOP_VERSION_OPTION}, \
                                                    {GET_EC_VERSION_OPTION}, \
                                                    {GET_ME_VERSION_OPTION}, \
                                                    {GET_LAN_PHY_VERSION_OPTION}, \
                                                    {DISPLAY_INFO_END, NULL}

////
//// SA Information Display Option Table
////
//
// EC Information Display Option Table
//                                                  
#define EC_INFO_DISPLAY_TABLE_LIST                  {GET_EC_VERSION_ID_OPTION}, \
                                                    {GET_BOARD_ID_OPTION}, \
                                                    {GET_FAB_ID_OPTION}, \
                                                    {DISPLAY_INFO_END, NULL}
//[-start-121022-IB10540014-modify]//                                                 
//
// CPU Information Display Option Table
//
#define CPU_INFO_DISPLAY_TABLE_LIST                 {GET_CPU_ID_OPTION}, \
                                                    {GET_CPU_SPEED_OPTION}, \
                                                    {GET_CPU_STEPPING_OPTION}, \
                                                    {GET_CACHE_INFO_OPTION}, \
                                                    {GET_L1_DATA_CACHE_OPTION}, \
                                                    {GET_L1_INSTRUCTION_CACHE_OPTION}, \
                                                    {GET_L2_CACHE_OPTION}, \
                                                    {GET_L3_CACHE_OPTION}, \
                                                    {GET_CORE_NUMBER_OPTION}, \
                                                    {GET_MICROCODE_VERSION_OPTION}, \
                                                    {GET_GT_INFO_OPTION}, \
                                                    {DISPLAY_INFO_END, NULL}
#define CPU_CAPABILITY_DISPLAY_TABLE_LIST           {GET_TXT_CAPABILITY_OPTION}, \
                                                    {DISPLAY_INFO_END, NULL}
#endif
