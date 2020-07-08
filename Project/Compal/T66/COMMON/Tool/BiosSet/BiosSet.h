//;******************************************************************************
//;* Copyright (c) 1983-2006, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;
//; Abstract:  ver 2
//;

#ifndef _UTILITY_BIOS_SET_H_
#define _UTILITY_BIOS_SET_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/HobLib.h>
#include <T66CommonDefinition.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <ScuAllPage.h>
#include <Sample.h>

/**
  Remove data in mLinkEntry by signature

  @param[In] Signatrue        The Signature date which will remove

  @return EFI_SUCCESS          Function execute successful.
**/
EFI_STATUS
Remove_BiosSetting_Data (
  IN UINT32                         Signature
  );

/**
  Add data to mLinkEntry
  @param[In] DataStru             Date which add to mLinkEntry

  @return EFI_OUT_OF_RESOURCES    Allocate memroy fail.
  @return EFI_SUCCESS             Function execute successful.
**/
EFI_STATUS
Add_BiosSetting_Data (
  IN BIOS_SETTING_STRUCT    *DataStru
  );

#define c_PARM              L"-c"
#define c_HELP_STRING       L"    -c    :Clean all bios setting.\n"
#define c_HELP_EXAMPLE      L"           Example: BiosSet.efi -c \n"

#define dp_PARM             L"-dp"
#define dp_HELP_STRING      L"    -dp   :Dump all bios setting.\n"
#define dp_HELP_EXAMPLE     L"           Example: BiosSet.efi -dp \n"

#define h_PARM              L"-h"
#define h_HELP_STRING       L""
#define h_HELP_EXAMPLE      L""

#define NULL_PARM  NULL
#define NULL_HELP_STRING    L""
#define NULL_HELP_EXAMPLE   L""

EFI_STATUS
Dump_ALL (
  LIST_ENTRY                *List
  );

EFI_STATUS
Clean_ALL (
  LIST_ENTRY                *List
  );

EFI_STATUS
HelpInfo(
  LIST_ENTRY                *List
  );

//
// Please add support command / function in here directly:
//
// xxx_PARM: command string
// XXX_HELP_STRING: Command help string
// XXX_HELP_STRING: Command help example
// XXX should same as command
// REGISTER_PARAMETER_FUNCTION (command , SHELL_PARAM_TYPE, Parameter_Count (include command), Function)
//
#define REGISTER_PARAMETER_FUNCTION  do {                             \
  ADD_BIOS_STTING_CONFING (scu,   TypeFlag,   2,  SCU_AllPage );      \
  ADD_BIOS_STTING_CONFING (c,     TypeFlag,   1,  Clean_ALL   );      \
  ADD_BIOS_STTING_CONFING (dp,    TypeFlag,   1,  Dump_ALL    );      \
  ADD_BIOS_STTING_CONFING (h,     TypeFlag,   1,  HelpInfo    );      \
} while(0)
  //  ADD_BIOS_STTING_CONFING (Samp,  TypeFlag,   2,  Sample   );         \  // Sample code

#endif
