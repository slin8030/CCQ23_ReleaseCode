/*
 * (C) Copyright 2016 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2016 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y

 Ver       Date       Who          Change           Support Chipset
 --------  --------   ------------ ---------------- -----------------------------------
 1.00      16/08/08    Daiven_Chen   Init version.
*/
#include "Library/DxeProjectSvcLib.h"
#include <BiosSet.h>

typedef struct {
  CHAR16             *Parameter;
  UINTN               ParmCount;
  CHAR16             *HelpString;
  CHAR16             *HelpExample;
  EFI_STATUS         (*fptr)(LIST_ENTRY*);
} BIOS_SETTING_PARAMETER_ENTRY;

BIOS_SETTING_ENTRY      *mLinkEntry;

#define BIOS_SETTING_ENTRY_FROM_LINK(a)       BASE_CR (a, BIOS_SETTING_ENTRY, Link)

/**
  Set default mParamList/mDefaultBiosSet
  mParamList/mDefaultBiosSet array size will patch by compiler.
**/
STATIC
EFI_STATUS
SetDefaultFunc (
  )
{
  UINTN   Counter;
  UINTN   Index;
  extern BIOS_SETTING_PARAMETER_ENTRY mDefaultBiosSet[];
  extern SHELL_PARAM_ITEM mParamList[];

  Index = 0;
  #define ADD_PARAM_LIST(ParamName, ParamType)              \
          {                                                 \
            mParamList[Index].Name = ParamName;             \
            mParamList[Index].Type = ParamType;             \
          }

  #define ADD_BIOS_STTING_CONFING(ParamName, ParamType, ParamCount, Function)           \
          {                                                                 \
            ADD_PARAM_LIST (ParamName##_PARM, ParamType);                   \
            mDefaultBiosSet[Index].Parameter = ParamName##_PARM;            \
            mDefaultBiosSet[Index].ParmCount = ParamCount;                  \
            mDefaultBiosSet[Index].HelpString =  ParamName##_HELP_STRING;   \
            mDefaultBiosSet[Index].HelpExample = ParamName##_HELP_EXAMPLE;  \
            mDefaultBiosSet[Index].fptr = Function;                         \
            Index++;                                                        \
            Counter = __COUNTER__;                                          \
          }
  REGISTER_PARAMETER_FUNCTION;

  return EFI_SUCCESS;
}

BIOS_SETTING_PARAMETER_ENTRY mDefaultBiosSet[__COUNTER__];
SHELL_PARAM_ITEM             mParamList[__COUNTER__];

/**
  Print help information
  @param[In] List             Not used

  @return EFI_SUCCESS          Function execute successful.
**/
EFI_STATUS
HelpInfo(
  IN LIST_ENTRY                *List
  )
{
  UINTN                     Index;

  for (Index = 0; Index < sizeof(mDefaultBiosSet)/sizeof(mDefaultBiosSet[0]); Index++) {
    Print(mDefaultBiosSet[Index].HelpString);
    Print(mDefaultBiosSet[Index].HelpExample);
    if (mDefaultBiosSet[Index].Parameter == NULL) {
      continue;
    }
  }
  return EFI_SUCCESS;
}

/**
  Search Signature in BiosSetting

  @param[In] Signature        Signature to search

  @return  NULL               Not found data
  @return  Address            return data address which match the Signature
**/
BIOS_SETTING_ENTRY*
Search_Setting_Data (
  IN UINT32                    Signature
  )
{
  BIOS_SETTING_ENTRY         *SetEntry;
  LIST_ENTRY                 *Link;

  SetEntry = NULL;
  Link = GetFirstNode (&mLinkEntry->Link);
  while (!IsNull (&mLinkEntry->Link, Link)) {
    SetEntry = BIOS_SETTING_ENTRY_FROM_LINK (Link);
    if (SetEntry->Header.Signature == Signature) {
      return SetEntry;
    }
    Link = GetNextNode (&mLinkEntry->Link, Link);
  }
  return NULL;
}
/**
  Dump all date in mLinkEntry;

  @param[In] List             The list include utility input parameter.

  @return EFI_SUCCESS         Function execute successful.
**/
EFI_STATUS
Dump_ALL (
  LIST_ENTRY                *List
  )
{
  BIOS_SETTING_ENTRY                *BiosDataEntry;
  LIST_ENTRY                        *Link;
  UINTN                              Index;
  CHAR16                              SignatureStrUni[5];
  UINT8                               SignatureStrAsii[5];

  if (mLinkEntry == NULL) {
    return EFI_SUCCESS;
  }

  Link = GetFirstNode (&mLinkEntry->Link);
  while (!IsNull (&mLinkEntry->Link, Link)) {
    BiosDataEntry = BIOS_SETTING_ENTRY_FROM_LINK (Link);
    ZeroMem (SignatureStrAsii, sizeof(SignatureStrAsii));
    SignatureStrAsii[4] = '\0';
    CopyMem(SignatureStrAsii, &BiosDataEntry->Header.Signature,sizeof(BiosDataEntry->Header.Signature));
    AsciiStrToUnicodeStr(SignatureStrAsii, SignatureStrUni);
    Print(L"Signature:%s\n", SignatureStrUni);
    Print(L" DataSize:0x%x\n", BiosDataEntry->Header.DataSize);
    Print(L" Data:");
    for (Index = 0 ; Index < BiosDataEntry->Header.DataSize; Index++) {
      Print(L" 0x%02x", BiosDataEntry->Data[Index]);
      Index++;
      if ((Index%16) ==0) {
        Print(L"\n");
        Print(L"     :");
      }
    }
    Print(L" \n");
    Link = GetNextNode (&mLinkEntry->Link, Link);
  }

  return EFI_SUCCESS;
}

/**
  Clean all date in mLinkEntry;

  @param[In] List             The list include utility input parameter.

  @return EFI_SUCCESS         Function execute successful.
**/
EFI_STATUS
Clean_ALL (
  LIST_ENTRY                *List
  )
{
  BIOS_SETTING_ENTRY                *BiosDataEntry;
  LIST_ENTRY                        *Link;

  if (mLinkEntry == NULL) {
    return EFI_SUCCESS;
  }

  BiosDataEntry = NULL;
  Link = GetFirstNode (&mLinkEntry->Link);
  while (!IsNull (&mLinkEntry->Link, Link)) {
    BiosDataEntry = BIOS_SETTING_ENTRY_FROM_LINK (Link);
    Link = GetNextNode (&mLinkEntry->Link, Link);
    RemoveEntryList(&BiosDataEntry->Link);
    mLinkEntry->Header.DataSize -= (sizeof (BIOS_SETTING_HEADER) + BiosDataEntry->Header.DataSize);
    FreePool(BiosDataEntry);
  }

  return EFI_SUCCESS;
}

/**
  Remove data in mLinkEntry by signature

  @param[In] Signatrue        The Signature date which will remove

  @return EFI_SUCCESS          Function execute successful.
**/
EFI_STATUS
Remove_BiosSetting_Data (
  IN UINT32                         Signature
  )
{
  BIOS_SETTING_ENTRY      *BiosDataEntry;

  if (mLinkEntry == NULL) {
    return EFI_SUCCESS;
  }

  BiosDataEntry = NULL;
  BiosDataEntry = Search_Setting_Data(Signature);
  if (BiosDataEntry != NULL) {
    RemoveEntryList(&BiosDataEntry->Link);
    mLinkEntry->Header.DataSize -= (sizeof (BIOS_SETTING_HEADER) + BiosDataEntry->Header.DataSize);
    FreePool(BiosDataEntry);
  }
  return EFI_SUCCESS;
}

/**
  Add data to mLinkEntry
  @param[In] DataStru             Date which add to mLinkEntry

  @return EFI_OUT_OF_RESOURCES    Allocate memroy fail.
  @return EFI_SUCCESS             Function execute successful.
**/
EFI_STATUS
Add_BiosSetting_Data (
  IN BIOS_SETTING_STRUCT    *DataStru
  )
{
  BIOS_SETTING_ENTRY      *BiosEntry;

  BiosEntry = Search_Setting_Data(DataStru->Header.Signature);
  //
  // Overwrite already exist data directly
  //
  if (BiosEntry != NULL) {
    // Data same as old, update directly
    if (BiosEntry->Header.DataSize == DataStru->Header.DataSize) {
      CopyMem (&BiosEntry->Data, &DataStru->Data, DataStru->Header.DataSize);
      Print (L"Find same update date signature , update directly \n");
      return EFI_SUCCESS;
    } else {
      // Signature already exist, but data size doesn't the same , remove old data
      Print (L"Signature : %s already exist, but data size is not the same\n", BiosEntry->Header.Signature);
      RemoveEntryList(&BiosEntry->Link);
      mLinkEntry->Header.DataSize -= (sizeof (BIOS_SETTING_HEADER) + BiosEntry->Header.DataSize);
      FreePool (BiosEntry);
    }
  }

  // Add new data in link list.
  BiosEntry = NULL;
  BiosEntry = AllocateZeroPool(sizeof (BIOS_SETTING_ENTRY) + DataStru->Header.DataSize);
  if (BiosEntry == NULL) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
    Print(L"ERROR allocate Memory error\n");
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem(&BiosEntry->Header, DataStru, sizeof (BIOS_SETTING_HEADER) + DataStru->Header.DataSize);
  InsertTailList(&mLinkEntry->Link, &BiosEntry->Link);
  mLinkEntry->Header.DataSize += sizeof (BIOS_SETTING_HEADER) + DataStru->Header.DataSize;
  return EFI_SUCCESS;
}

/**
  Search the input parametr (in List) which match the mDefaultBiosSet.Parameter/ParmCount define

  @param[In] List             The list include utility input parameter.

  @return EFI_SUCCESS         Execute function success;
  @return EFI_NOT_FOUND       Not fond any parameter which match in  mDefaultBiosSet function define
  @return others              Execute function return status
**/
STATIC
EFI_STATUS
ExecBiosSetFunc (
  IN LIST_ENTRY                *List
  )
{
  EFI_STATUS                Status;
  UINTN                     Index;

  //
  // Check the parameter and paramter count , execute function if match the function define
  // else print help string.
  //
  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < sizeof(mDefaultBiosSet)/sizeof(mDefaultBiosSet[0]); Index++) {
    if (ShellCommandLineGetFlag(List, mDefaultBiosSet[Index].Parameter)) {
      if (ShellCommandLineGetCount(List) == mDefaultBiosSet[Index].ParmCount) {
        //
        // Parameter, ParmCount match defien , run function.
        //
        Status =  mDefaultBiosSet[Index].fptr(List);
      } else {
        //
        // ParmCount doesn't match defien ,print help example.
        //
        Print(mDefaultBiosSet[Index].HelpString);
        Print(mDefaultBiosSet[Index].HelpExample);
        Status = EFI_INVALID_PARAMETER;
      }
      if (EFI_ERROR(Status)) {
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
      }
      Print (L"%s status: %r \n", mDefaultBiosSet[Index].Parameter, Status);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
      return Status;
    }
  }

  HelpInfo(NULL);
  return Status;
}

INTN
EFIAPI
ShellAppMain (
  IN UINTN    Argc,
  IN CHAR16   **Argv
  )
{
  EFI_STATUS                        Status;
  BIOS_SETTING_STRUCT               *DefaultBiosSetting;
  LIST_ENTRY                        *List = NULL;
  UINTN                             BufferSize;
  UINTN                             EndAddress;
  BIOS_SETTING_ENTRY                *SubSetEntry;
  BIOS_SETTING_STRUCT               *HeaderEntry;
  LIST_ENTRY                        *Link;
  Print (L"Bios Setting utility : V0.03 \n");
/****************************************************
  Signature (BIOS_SETTING_SIGNATURE_$BST)
   DataSize (Total DataSize)
   Data---Signature                               <-----
           DataSize                                     :
           Data---                                      :
                |                                       :
                | DataSize                              :
                |                                       :
               --- Signature                            :
                    DataSize
                    Data---                        (Total DataSize)
                         |
                         | DataSize                     :
                         |                              :
                        --- Signature                   :
                             DataSize                   :
                             Data---                    :
                                  |                     :
                                  | DataSize            :
                                  |                     :
                                 ---                    :
                                  .                     :
                                  .                     :
                                 ....             <-----
  **************************************************/

  DefaultBiosSetting = NULL;
  mLinkEntry = NULL;
  //
  // SetDefaultFunc use for Compiler to patch mDefaultBiosSet array size.
  //
  SetDefaultFunc();

  //
  // Get input command and parameter
  //
  Status = ShellCommandLineParse(mParamList, &List, NULL, FALSE);

  //
  // Get BiosSetting variable title entry
  //
  Status = GetBiosSettingData (&DefaultBiosSetting, BIOS_SETTING_SIGNATURE);
  if (EFI_ERROR (Status)) {
    BufferSize = sizeof (BIOS_SETTING_HEADER);
    DefaultBiosSetting = AllocatePool (BufferSize);
    DefaultBiosSetting->Header.Signature = BIOS_SETTING_SIGNATURE;
    DefaultBiosSetting->Header.DataSize = 0;
  }

  //
  // Init mLinkEntry , copy BiosSetting subdata to mLinkEntry link list
  //
  mLinkEntry = AllocateZeroPool (sizeof (BIOS_SETTING_ENTRY));
  if (mLinkEntry == NULL) {
    Status =  EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  InitializeListHead (&mLinkEntry->Link);
  mLinkEntry->Header.Signature = BIOS_SETTING_SIGNATURE;
  mLinkEntry->Header.DataSize = 0;
  EndAddress = (UINTN)(DefaultBiosSetting->Data + DefaultBiosSetting->Header.DataSize);
  HeaderEntry =(BIOS_SETTING_STRUCT *)DefaultBiosSetting->Data;

  while ((UINTN)HeaderEntry < EndAddress){
    SubSetEntry = AllocateZeroPool (sizeof (BIOS_SETTING_ENTRY) + HeaderEntry->Header.DataSize);
    if (SubSetEntry == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
    CopyMem(&SubSetEntry->Header, HeaderEntry, sizeof(BIOS_SETTING_HEADER) + HeaderEntry->Header.DataSize);
    InsertTailList (&mLinkEntry->Link, &SubSetEntry->Link);
    mLinkEntry->Header.DataSize += sizeof(BIOS_SETTING_HEADER) + SubSetEntry->Header.DataSize;
    HeaderEntry = (BIOS_SETTING_STRUCT *)(HeaderEntry->Data + HeaderEntry->Header.DataSize);
  }

  if (mLinkEntry->Header.DataSize != DefaultBiosSetting->Header.DataSize) {
    Print(L"Load bios setting error\n");
    Status = EFI_LOAD_ERROR;
    goto Exit;
  }
  FreePool(DefaultBiosSetting);
  DefaultBiosSetting = NULL;

  //
  // Execute command function
  //
  Status = ExecBiosSetFunc (List);

  //
  // Update mLinkEntry link list data to BiosSetting variable
  //
  if (!EFI_ERROR(Status)) {
    BufferSize = sizeof(BIOS_SETTING_HEADER) + mLinkEntry->Header.DataSize;
    DefaultBiosSetting = AllocateZeroPool (BufferSize);
    DefaultBiosSetting->Header.Signature = BIOS_SETTING_SIGNATURE;
    DefaultBiosSetting->Header.DataSize = 0;
    HeaderEntry =(BIOS_SETTING_STRUCT *)DefaultBiosSetting->Data;
    Link = GetFirstNode (&mLinkEntry->Link);
    while (!IsNull (&mLinkEntry->Link, Link)) {
      SubSetEntry = BIOS_SETTING_ENTRY_FROM_LINK (Link);
      CopyMem(&HeaderEntry->Header, &SubSetEntry->Header, sizeof (BIOS_SETTING_HEADER) + SubSetEntry->Header.DataSize);
      DefaultBiosSetting->Header.DataSize += sizeof (BIOS_SETTING_HEADER) + HeaderEntry->Header.DataSize;
      HeaderEntry = (BIOS_SETTING_STRUCT *)(HeaderEntry->Data + HeaderEntry->Header.DataSize);
      Link = GetNextNode (&mLinkEntry->Link, Link);
    }

    if (mLinkEntry->Header.DataSize == 0) {
      //
      // No any data , clean all variable directly.
      //
      BufferSize = 0;
    }

    gRT->SetVariable (
           BIOS_SETTING_VARIABLE_NAME,
           &gEfiBIOSSettingGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           BufferSize,
           DefaultBiosSetting
           );
    FreePool (DefaultBiosSetting);
  }

Exit:
  if (DefaultBiosSetting != NULL) {
    FreePool(DefaultBiosSetting);
  }

  if (mLinkEntry != NULL) {
    SubSetEntry = NULL;
    Link = GetFirstNode (&mLinkEntry->Link);
    while (!IsNull (&mLinkEntry->Link, Link)) {
      SubSetEntry = BIOS_SETTING_ENTRY_FROM_LINK (Link);
      Link = GetNextNode (&mLinkEntry->Link, Link);
      RemoveEntryList(&SubSetEntry->Link);
      FreePool(SubSetEntry);
    }
    FreePool(mLinkEntry);
  }
  return Status;
}
